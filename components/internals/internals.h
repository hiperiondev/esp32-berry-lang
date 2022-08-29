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

#ifndef INTERNALS_H_
#define INTERNALS_H_

#include "littlefs.h"

#define MOUNT_POINT           "/littlefs"
#define PARTITION_LABEL       "littlefs"

#define FS_INIT               littlefs_init()
#define F_OPEN(FN, OT)        littlefs_fopen(FN, OT)
#define F_REOPEN(FN, OT, ST)  littlefs_freopen(FN, OT, ST)
#define F_REMOVE(FN)          littlefs_remove(FN)
#define F_RENAME(ON, NN)      littlefs_rename(ON, NN)
#define F_LS                  littlefs_ls()

#endif /* INTERNALS_H_ */
