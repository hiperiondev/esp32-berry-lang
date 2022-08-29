/********************************************************************
 ** Copyright (c) 2018-2020 Guan Wenliang
 ** This file is part of the Berry default interpreter.
 ** skiars@qq.com, https://github.com/Skiars/berry
 ** See Copyright Notice in the LICENSE file or at
 ** https://github.com/Skiars/berry/blob/master/LICENSE
 ********************************************************************/

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
