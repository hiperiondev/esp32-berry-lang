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

#ifndef BE_PORTUTILS_H_
#define BE_PORTUTILS_H_

#include "berry.h"

   bool BerryInit(bvm **vm, char *berry_prog);
   void BrLoad(bvm *vm, const char *script_name);

   void be_error_pop_all(bvm *vm);
   void be_dumpstack(bvm *vm);
   void checkBeTop(bvm *vm, char *str);

int32_t callBerryEventDispatcher(bvm *vm, const char *type, const char *cmd, int32_t idx, const char *payload, uint32_t data_len);

#endif /* BE_PORTUTILS_H_ */
