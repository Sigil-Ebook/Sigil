/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Alexander Borisov <borisov@lexbor.com>
 */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include <string.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "lexbor/core/fs.h"


lxb_status_t
lexbor_fs_dir_read(const lxb_char_t *dirpath, lexbor_fs_dir_opt_t opt,
                   lexbor_fs_dir_file_f callback, void *ctx)
{
    DIR *dir;
    size_t path_len, free_len, d_namlen;
    struct dirent *entry;
    lexbor_action_t action;
    lexbor_fs_file_type_t f_type;

    char *file_begin;
    char full_path[4096];

    path_len = strlen((const char *) dirpath);
    if (path_len == 0 || path_len >= (sizeof(full_path) - 1)) {
        return LXB_STATUS_ERROR;
    }

    dir = opendir((const char *) dirpath);
    if (dir == NULL) {
        return LXB_STATUS_ERROR;
    }

    memcpy(full_path, dirpath, path_len);

    /* Check for a separating character at the end dirpath */
    if (full_path[(path_len - 1)] != '/') {
        path_len++;

        if (path_len >= (sizeof(full_path) - 1)) {
            goto error;
        }

        full_path[(path_len - 1)] = '/';
    }

    file_begin = &full_path[path_len];
    free_len = (sizeof(full_path) - 1) - path_len;

    if (opt == LEXBOR_FS_DIR_OPT_UNDEF)
    {
        while ((entry = readdir(dir)) != NULL) {
            d_namlen = strlen(entry->d_name);

            if (d_namlen >= free_len) {
                goto error;
            }

            /* +1 copy terminating null byte '\0' */
            memcpy(file_begin, entry->d_name, (d_namlen + 1));

            action = callback((const lxb_char_t *) full_path,
                              (path_len + d_namlen),
                              (const lxb_char_t *) entry->d_name,
                              d_namlen, ctx);
            if (action == LEXBOR_ACTION_STOP) {
                break;
            }
        }

        goto done;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (opt & LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN
            && *entry->d_name == '.')
        {
            continue;
        }

        d_namlen = strlen(entry->d_name);

        if (d_namlen >= free_len) {
            goto error;
        }

        f_type = lexbor_fs_file_type((const lxb_char_t *) entry->d_name);

        if (opt & LEXBOR_FS_DIR_OPT_WITHOUT_DIR
            && f_type == LEXBOR_FS_FILE_TYPE_DIRECTORY)
        {
            continue;
        }

        if (opt & LEXBOR_FS_DIR_OPT_WITHOUT_FILE
            && f_type == LEXBOR_FS_FILE_TYPE_FILE)
        {
            continue;
        }

        /* +1 copy terminating null byte '\0' */
        memcpy(file_begin, entry->d_name, (d_namlen + 1));

        action = callback((const lxb_char_t *) full_path,
                          (path_len + d_namlen),
                          (const lxb_char_t *) entry->d_name,
                          d_namlen, ctx);
        if (action == LEXBOR_ACTION_STOP) {
            break;
        }
    }

done:

    closedir(dir);

    return LXB_STATUS_OK;

error:

    closedir(dir);

    return LXB_STATUS_ERROR;
}

lexbor_fs_file_type_t
lexbor_fs_file_type(const lxb_char_t *full_path)
{
    struct stat sb;

    if (stat((const char *) full_path, &sb) == -1) {
        return LEXBOR_FS_FILE_TYPE_UNDEF;
    }

    switch (sb.st_mode & S_IFMT) {
        case S_IFBLK:
            return LEXBOR_FS_FILE_TYPE_BLOCK_DEVICE;

        case S_IFCHR:
            return LEXBOR_FS_FILE_TYPE_CHARACTER_DEVICE;

        case S_IFDIR:
            return LEXBOR_FS_FILE_TYPE_DIRECTORY;

        case S_IFIFO:
            return LEXBOR_FS_FILE_TYPE_PIPE;

        case S_IFLNK:
            return LEXBOR_FS_FILE_TYPE_SYMLINK;

        case S_IFREG:
            return LEXBOR_FS_FILE_TYPE_FILE;

        case S_IFSOCK:
            return LEXBOR_FS_FILE_TYPE_SOCKET;

        default:
            return LEXBOR_FS_FILE_TYPE_UNDEF;
    }

    return LEXBOR_FS_FILE_TYPE_UNDEF;
}

lxb_char_t *
lexbor_fs_file_easy_read(const lxb_char_t *full_path, size_t *len)
{
    FILE *fh;
    long size;
    size_t nread;
    lxb_char_t *data;

    fh = fopen((const char *) full_path, "rb");
    if (fh == NULL) {
        goto error;
    }

    if (fseek(fh, 0L, SEEK_END) != 0) {
        goto error_close;
    }

    size = ftell(fh);
    if (size < 0) {
        goto error_close;
    }

    if (fseek(fh, 0L, SEEK_SET) != 0) {
        goto error_close;
    }

    data = lexbor_malloc(size + 1);
    if (data == NULL) {
        goto error_close;
    }

    nread = fread(data, 1, size, fh);
    if (nread != (size_t) size) {
        lexbor_free(data);

        goto error_close;
    }

    fclose(fh);

    data[size] = '\0';

    if (len != NULL) {
        *len = nread;
    }

    return data;

error_close:

    fclose(fh);

error:

    if (len != NULL) {
        *len = 0;
    }

    return NULL;
}
