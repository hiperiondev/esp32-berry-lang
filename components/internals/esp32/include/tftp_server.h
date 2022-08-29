/*
 * Copyright 2022 Emiliano Gonzalez (egonzalez . hiperion @ gmail . com))
 * * Project Site: https://github.com/hiperiondev/esp32-berry-lang *
 *
 * This is based on other projects:
 *    Berry (https://github.com/berry-lang/berry)
 *    LittleFS port for ESP-IDF (https://github.com/joltwallet/esp_littlefs)
 *    Lightweight TFTP server library (https://github.com/lexus2k/libtftp)
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

#ifndef TFTP_SERVER_H_
#define TFTP_SERVER_H_

#define TFTP_DEFAULT_PORT (69)

#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>

void TFTP_init(uint16_t port);

/**
 * Starts server.
 * Returns 0 if server is started successfully
 */
int TFTP_start(void);

/**
 * Listens for incoming requests. Can be executed in blocking and non-blocking mode
 * @param wait_for true if blocking mode is required, false if non-blocking mode is required
 */
int TFTP_run(bool wait_for);

/**
 * Stops server
 */
void TFTP_stop(void);

void TFTP_task_start(void);
void TFTP_task_stop(void);

#endif
