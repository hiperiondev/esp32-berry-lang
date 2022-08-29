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

#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "esp_err.h"
#include "esp_log.h"

#include "internals.h"
#include "esp_littlefs.h"

static const char *TAG = "littlefs";

#define FORMAT_IF_MOUNT_FAILED  1

static bool littlefs_initialized = false;

int littlefs_init(void) {
    ESP_LOGI(TAG, "Initializing LittleFS");

    esp_vfs_littlefs_conf_t conf = {
    		.base_path = MOUNT_POINT,
            .partition_label = PARTITION_LABEL,
			.format_if_mount_failed = FORMAT_IF_MOUNT_FAILED,
            .dont_mount = false
			};

    // Use settings defined above to initialize and mount LittleFS filesystem.
    // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    littlefs_initialized = true;
    return ESP_OK;
}

void littlefs_deinit(void) {
    if (!littlefs_initialized)
        return;
    esp_vfs_littlefs_unregister("littlefs");
    ESP_LOGI(TAG, "LittleFS unmounted");
}

FILE* littlefs_fopen(const char *file, const char *mode) {
    if (!littlefs_initialized)
            return NULL;
    char route[512];
    sprintf(route, "%s/%s", MOUNT_POINT, file);
    return fopen(route, mode);
}

FILE* littlefs_freopen(const char *filename, const char *opentype, FILE *stream) {
    if (!littlefs_initialized)
        return NULL;
    char route[512];

    sprintf(route, "%s/%s", MOUNT_POINT, filename);
    fclose(stream);
    return littlefs_fopen((char *)filename, (char *)opentype);
}

int littlefs_test(char *file) {
    char route[512];
    struct stat st;
    sprintf(route, "%s/%s", MOUNT_POINT, file);
    return stat(route, &st);
}

int littlefs_remove(const char *file) {
    if (!littlefs_initialized)
            return -1;
    char route[512];
    struct stat st;
    sprintf(route, "%s/%s", MOUNT_POINT, file);
    if (stat(route, &st) == 0) {
            unlink(route);
            return 0;
    }

    return -1;
}

int littlefs_rename(const char *file, char *newname) {
    if (!littlefs_initialized)
            return -1;
    char route_old[512];
    char route_new[512];
    sprintf(route_old, "%s/%s", MOUNT_POINT, file);
    sprintf(route_new, "%s/%s", MOUNT_POINT, newname);
    return rename(route_old, route_new);
}
