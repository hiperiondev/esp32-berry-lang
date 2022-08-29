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

#include <stdio.h>
#include <string.h>

#include "berry.h"
#include "be_mem.h"
#include "be_sys.h"

#include "internals.h"

/* this file contains configuration for the file system. */

/* standard input and output */

BERRY_API void be_writebuffer(const char *buffer, size_t length) {
    be_fwrite(stdout, buffer, length);
}

BERRY_API char* be_readstring(char *buffer, size_t size) {
    return be_fgets(stdin, buffer, (int) size);
}

void* be_fopen(const char *filename, const char *modes) {
    return F_OPEN(filename, modes);
}

int be_fclose(void *hfile) {
    return fclose(hfile);
}

size_t be_fwrite(void *hfile, const void *buffer, size_t length) {
    return fwrite(buffer, 1, length, hfile);
}

size_t be_fread(void *hfile, void *buffer, size_t length) {
    return fread(buffer, 1, length, hfile);
}

char* be_fgets(void *hfile, void *buffer, int size) {
    return fgets(buffer, size, hfile);
}

int be_fseek(void *hfile, long offset) {
    return fseek(hfile, offset, SEEK_SET);
}

long int be_ftell(void *hfile) {
    return ftell(hfile);
}

long int be_fflush(void *hfile) {
    return fflush(hfile);
}

size_t be_fsize(void *hfile) {
    long int size, offset = be_ftell(hfile);
    fseek(hfile, 0L, SEEK_END);
    size = ftell(hfile);
    fseek(hfile, offset, SEEK_SET);
    return size;
}

#if BE_USE_FILE_SYSTEM

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

int be_isdir(const char *path) {
    struct stat path_stat;
    int res = stat(path, &path_stat);
    return res == 0 && S_ISDIR(path_stat.st_mode);
}

int be_isfile(const char *path) {
    struct stat path_stat;
    int res = stat(path, &path_stat);
    return res == 0 && !S_ISDIR(path_stat.st_mode);
}

int be_isexist(const char *path) {
    struct stat path_stat;
    return stat(path, &path_stat) == 0;
}

char* be_getcwd(char *buf, size_t size) {
    return getcwd(buf, size);
}

int be_chdir(const char *path) {
    return chdir(path);
}

int be_mkdir(const char *path) {
    return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

int be_unlink(const char *filename) {
    return F_REMOVE(filename);
}

int be_dirfirst(bdirinfo *info, const char *path_) {
    char path[512];
    sprintf(path, "%s/%s", MOUNT_POINT, path_);
    info->dir = opendir(path);
    if (info->dir) {
        return be_dirnext(info);
    }
    return 1;
}

int be_dirnext(bdirinfo *info) {
    struct dirent *file;
    info->file = file = readdir(info->dir);
    if (file) {
        info->name = file->d_name;
        return 0;
    }
    return 1;
}

int be_dirclose(bdirinfo *info) {
    return closedir(info->dir) != 0;
}
#endif /* BE_USE_OS_MODULE || BE_USE_FILE_SYSTEM */
