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

#include "berry.h"

/* this file contains the declaration of the module table. */

/* default modules declare */
be_extern_native_module(string);
be_extern_native_module(json);
be_extern_native_module(math);
be_extern_native_module(time);
be_extern_native_module(os);
be_extern_native_module(global);
be_extern_native_module(sys);
be_extern_native_module(debug);
be_extern_native_module(gc);
be_extern_native_module(solidify);
be_extern_native_module(introspect);
be_extern_native_module(strict);
be_extern_native_module(undefined);

/* user-defined modules declare start */
/* Tasmota Berry extensions */
#include "be_mapping.h"
be_extern_native_module(cb);
//be_extern_native_class(int64);

/* user-defined modules declare end */

/* module list declaration */
BERRY_LOCAL const bntvmodule *const be_module_table[] = {
/* default modules register */
#if BE_USE_STRING_MODULE
        &be_native_module(string),
#endif
#if BE_USE_JSON_MODULE
        &be_native_module(json),
#endif
#if BE_USE_MATH_MODULE
        &be_native_module(math),
#endif
#if BE_USE_TIME_MODULE
        &be_native_module(time),
#endif
#if BE_USE_OS_MODULE
        &be_native_module(os),
#endif
#if BE_USE_GLOBAL_MODULE
        &be_native_module(global),
#endif
#if BE_USE_SYS_MODULE
        &be_native_module(sys),
#endif
#if BE_USE_DEBUG_MODULE
        &be_native_module(debug),
#endif
#if BE_USE_GC_MODULE
        &be_native_module(gc),
#endif
#if BE_USE_SOLIDIFY_MODULE
        &be_native_module(solidify),
#endif
#if BE_USE_INTROSPECT_MODULE
        &be_native_module(introspect),
#endif
#if BE_USE_STRICT_MODULE
        &be_native_module(strict),
#endif
        &be_native_module(undefined),
        /* user-defined modules register start */
        /* Tasmota Berry extensions */
        &be_native_module(cb),

        /* user-defined modules register end */
        NULL /* do not remove */
};

/* user-defined classes declare start */
/* be_extern_native_class(my_class); */
/* user-defined classes declare end */

BERRY_LOCAL bclass_array be_class_table = {
/* first list are direct classes */
/* &be_native_class(my_class), */
        //&be_native_class(int64),
        NULL, /* do not remove */
};

/* this code loads the native class definitions */
BERRY_API void be_load_custom_libs(bvm *vm)
{
    (void)vm;   /* prevent a compiler warning */

    /* add here custom libs */
#if !BE_USE_PRECOMPILED_OBJECT
    /* be_load_xxxlib(vm); */
#endif
}
