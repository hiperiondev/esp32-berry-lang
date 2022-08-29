/*
 * Copyright 2022 Emiliano Gonzalez (egonzalez . hiperion @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/esp32-berry-lang *
 *
 * This is based on other projects:
 *    Berry (https://github.com/berry-lang/berry)
 *    LittleFS port for ESP-IDF (https://github.com/joltwallet/esp_littlefs)
 *    Lightweight TFTP server library (https://github.com/lexus2k/libtftp)
 *    esp32 run berry language (https://github.com/HoGC/esp32_berry)
 *    Tasmota (https://github.com/arendst/Tasmota)
 *    Others (see individual files)
 *
 *    please contact their authors for more information.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "esp_heap_caps.h"
#include "esp_chip_info.h"
#include "soc/efuse_reg.h"
#include "spiram.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "berry.h"
#include "be_vm.h"
#include "be_mapping.h"
#include "be_gc.h"
#include "berry_conf.h"
#include "be_portutils.h"

static const char *TAG = "be_portutils";

extern void be_load_custom_libs(bvm *vm);
extern void be_tracestack(bvm *vm);

//
// Sanity Check for be_top()
// Checks that the Berry stack is empty, if not print a Warning and empty it
//
void checkBeTop(bvm *vm, char *str) {
    int32_t top = be_top(vm);
    if (top != 0) {
        be_pop(vm, top); // TODO should not be there
        ESP_LOGI(TAG, "Error be_top is non zero=%d (%s)", top, str);
    }
}

size_t callBerryGC(bvm *vm) {
    return callBerryEventDispatcher(vm, "gc", NULL, 0, NULL, 0);
}

// call the event dispatcher from custom object
// if data_len is non-zero, the event is also sent as raw `bytes()` object because the string may lose data
int32_t callBerryEventDispatcher(bvm *vm, const char *type, const char *cmd, int32_t idx, const char *payload, uint32_t data_len) {
    int32_t ret = 0;

    if (NULL == vm) {
        return ret;
    }
    checkBeTop(vm, "callBerryEventDispatcher IN");
    be_getglobal(vm, "custom");
    if (!be_isnil(vm, -1)) {
        be_getmethod(vm, -1, "event");
        if (!be_isnil(vm, -1)) {
            be_pushvalue(vm, -2); // add instance as first arg
            be_pushstring(vm, type != NULL ? type : "");
            be_pushstring(vm, cmd != NULL ? cmd : "");
            be_pushint(vm, idx);
            be_pushstring(vm, payload != NULL ? payload : "");  // empty json
            //BrTimeoutStart();
            if (data_len > 0) {
                be_pushbytes(vm, payload, data_len); // if data_len is set, we also push raw bytes
                ret = be_pcall(vm, 6); // 6 arguments
                be_pop(vm, 1);
            } else {
                ret = be_pcall(vm, 5); // 5 arguments
            }
            //BrTimeoutReset();
            if (ret != 0) {
                be_error_pop_all(vm); // clear Berry stack
                return ret;
            }
            be_pop(vm, 5);
            if (be_isint(vm, -1) || be_isbool(vm, -1)) {
                if (be_isint(vm, -1)) {
                    ret = be_toint(vm, -1);
                }
                if (be_isbool(vm, -1)) {
                    ret = be_tobool(vm, -1);
                }
            }
        }
        be_pop(vm, 1);  // remove method
    }
    be_pop(vm, 1);  // remove instance object
    checkBeTop(vm, "callBerryEventDispatcher OUT");
    return ret;
}

// Simplified version of event loop. Just call `tasmota.fast_loop()`
void callBerryFastLoop(bvm *vm) {

    if (NULL == vm) {
        return;
    }

    if (be_getglobal(vm, "custom")) {
        if (be_getmethod(vm, -1, "fast_loop")) {
            be_pushvalue(vm, -2); // add instance as first arg
            //BrTimeoutStart();
            int32_t ret = be_pcall(vm, 1);
            if (ret != 0) {
                be_error_pop_all(vm); // clear Berry stack
            }
            //BrTimeoutReset();
            be_pop(vm, 1);
        }
        be_pop(vm, 1); // remove method
    }
    be_pop(vm, 1); // remove instance object
    be_pop(vm, be_top(vm));   // clean
}

/*********************************************************************************************\
 * VM Observability
 \********************************************************************************************/

void be_dumpstack(bvm *vm) {
    int32_t top = be_top(vm);
    ESP_LOGI(TAG, "BRY: top=%d", top);
    be_tracestack(vm);
    for (uint32_t i = 1; i <= top; i++) {
        const char *tname = be_typename(vm, i);
        const char *cname = be_classname(vm, i);
        if (be_isstring(vm, i)) {
            cname = be_tostring(vm, i);
        }
        ESP_LOGI(TAG, "BRY: stack[%d] = type='%s' (%s)", i, (tname != NULL) ? tname : "", (cname != NULL) ? cname : "");
    }
}

void BerryObservability(bvm *vm, int event, ...) {
    va_list param;
    va_start(param, event);
    static int32_t vm_usage = 0;
    static uint32_t gc_time = 0;

    switch (event) {
        case BE_OBS_PCALL_ERROR: {  // error after be_pcall
            int32_t top = be_top(vm);
            // check if we have two strings for an Exception
            if (top >= 2 && be_isstring(vm, -1) && be_isstring(vm, -2)) {
                ESP_LOGI(TAG, "Exception> '%s' - %s", be_tostring(vm, -2), be_tostring(vm, -1));
                be_tracestack(vm);
            } else {
                be_dumpstack(vm);
            }
        }
        __attribute__ ((fallthrough));
        //no break
        case BE_OBS_GC_START: {
            gc_time = esp_timer_get_time() / 1000;
            vm_usage = va_arg(param, int32_t);
        }
            break;
        case BE_OBS_GC_END: {
            int32_t vm_usage2 = va_arg(param, int32_t);
            uint32_t gc_elapsed = (esp_timer_get_time() / 1000) - gc_time;
            uint32_t vm_scanned = va_arg(param, uint32_t);
            uint32_t vm_freed = va_arg(param, uint32_t);
            size_t slots_used_before_gc = va_arg(param, size_t);
            size_t slots_allocated_before_gc = va_arg(param, size_t);
            size_t slots_used_after_gc = va_arg(param, size_t);
            size_t slots_allocated_after_gc = va_arg(param, size_t);
            ESP_LOGI(TAG, "GC from %i to %i bytes, objects freed %i/%i (in %d ms) - slots from %i/%i to %i/%i", vm_usage, vm_usage2, vm_freed, vm_scanned,
                    gc_elapsed, slots_used_before_gc, slots_allocated_before_gc, slots_used_after_gc, slots_allocated_after_gc);

            // make new threshold tighter when we reach high memory usage
            if (vm->gc.threshold > 20 * 1024) {
                vm->gc.threshold = vm->gc.usage + 10 * 1024; // increase by only 10 KB
            }
        }
            break;
        case BE_OBS_STACK_RESIZE_START: {
            int32_t stack_before = va_arg(param, int32_t);
            int32_t stack_after = va_arg(param, int32_t);
            ESP_LOGI(TAG, "Stack resized from %i to %i bytes", stack_before, stack_after);
        }
            break;
        case BE_OBS_VM_HEARTBEAT: {
            ESP_LOGI(TAG, ">>>: Heartbeat now = %i", (int )esp_timer_get_time() / 1000);
        }
            break;
        default:
            break;
    }
    va_end(param);
}

/*********************************************************************************************\
 * VM Init
 \*********************************************************************************************/

void be_error_pop_all(bvm *vm) {
    if (vm->obshook != NULL)
        (*vm->obshook)(vm, BE_OBS_PCALL_ERROR);
    be_pop(vm, be_top(vm)); // clear Berry stack
}

bool BerryInit(bvm **vm, char *berry_prog) {
    // clean previous VM if any
    if (*vm != NULL) {
        be_vm_delete(*vm);
        *vm = NULL;
    }

    int32_t ret_code1 = 0, ret_code2 = 0;
    bool berry_init_ok = false;

    *vm = be_vm_new(); // create a virtual machine instance
    be_set_obs_hook(*vm, &BerryObservability); // attach observability hook
    comp_set_named_gbl(*vm); // Enable named globals in Berry compiler
    comp_set_strict(*vm); // Enable strict mode in Berry compiler, equivalent of `import strict`
    be_set_ctype_func_hanlder(*vm, be_call_ctype_func);

    be_load_custom_libs(*vm); // load classes and modules

    // Set the GC threshold to 3584 bytes to avoid the first useless GC
    (*vm)->gc.threshold = 3584;

    berry_init_ok = true;

    if (berry_prog == NULL)
        goto cont;
    ret_code1 = be_loadstring(*vm, berry_prog);
    if (ret_code1 != 0) {
        be_error_pop_all(*vm); // clear Berry stack
        goto cont;
    }
    ESP_LOGI(TAG, "Berry code loaded, RAM used=%u", be_gc_memcount(*vm));

    ret_code2 = be_pcall(*vm, 0);
    if (ret_code2 != 0) {
        be_error_pop_all(*vm); // clear Berry stack
        goto cont;
    }
    ESP_LOGI(TAG, "Berry code ran, RAM used=%u", be_gc_memcount(*vm));

    if (be_top(*vm) > 1) {
        be_error_pop_all(*vm); // clear Berry stack
    } else {
        be_pop(*vm, 1);
    }
    ESP_LOGI(TAG, "Berry initialized, RAM used=%u bytes", callBerryGC(*vm));

    cont:
    // we generate a synthetic event `autoexec`
    callBerryEventDispatcher(*vm, "preinit", NULL, 0, NULL, 0);

    // TODO: Run pre-init
    // BrLoad(vm, "preinit.be"); // run 'preinit.be' if present

    if (!berry_init_ok) {
        // free resources
        if (*vm != NULL) {
            be_vm_delete(*vm);
            *vm = NULL;
        }
    }

    return berry_init_ok;
}

/*********************************************************************************************\
 * Execute a script in Flash file-system
 \*********************************************************************************************/
void BrLoad(bvm *vm, const char *script_name) {
    if (vm == NULL) {
        return;
    }   // abort is berry is not running, or bootloop prevention kicked in

    be_getglobal(vm, "load");
    if (!be_isnil(vm, -1)) {
        be_pushstring(vm, script_name);

        //BrTimeoutStart();
        if (be_pcall(vm, 1) != 0) {
            be_error_pop_all(vm); // clear Berry stack
            return;
        }
        //BrTimeoutReset();
        bool loaded = be_tobool(vm, -2);  // did it succeed?
        be_pop(vm, 2);
        if (loaded) {
            ESP_LOGI(TAG, "Successfully loaded '%s'", script_name);
        } else {
            ESP_LOGI(TAG, "No '%s'", script_name);
        }
    }
}
