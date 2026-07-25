/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif


#include <string.h>
#include <sys/stat.h>

#include <windows.h>

#include "lexbor/core/fs.h"


lxb_status_t
lexbor_fs_dir_read(const lxb_char_t *dirpath, lexbor_fs_dir_opt_t opt,
                   lexbor_fs_dir_file_f callback, void *ctx)
{
    WIN32_FIND_DATA data;
    HANDLE h;
    size_t path_len, free_len, d_namlen;
    lexbor_action_t action;
    lexbor_fs_file_type_t f_type;

    char *file_begin;
    char full_path[4096];

    path_len = strlen((const char *) dirpath);
    if (path_len == 0 || path_len >= (sizeof(full_path) - 1)) {
        return LXB_STATUS_ERROR;
    }

    memcpy(full_path, dirpath, path_len);

    /* Check for a separating character at the end dirpath */
    if (full_path[(path_len - 1)] != '/') {
        path_len++;

        if (path_len >= (sizeof(full_path) - 1)) {
            return LXB_STATUS_ERROR;
        }

        full_path[(path_len - 1)] = '/';
    }

    file_begin = &full_path[path_len];
    free_len = (sizeof(full_path) - 1) - path_len;

    if (opt == LEXBOR_FS_DIR_OPT_UNDEF)
    {
        h = FindFirstFile((LPCSTR)dirpath, &data);
        if (h == INVALID_HANDLE_VALUE) {
            return LXB_STATUS_ERROR;
        }

        do {
            if (opt & LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN
                && *data.cFileName == '.')
            {
                continue;
            }

            d_namlen = strlen(data.cFileName);

            if (d_namlen >= free_len) {
                goto error;
            }

            /* +1 copy terminating null byte '\0' */
            memcpy(file_begin, data.cFileName, (d_namlen + 1));

            action = callback((const lxb_char_t *) full_path,
                              (path_len + d_namlen),
                              (const lxb_char_t *) data.cFileName,
                              d_namlen, ctx);
            if (action == LEXBOR_ACTION_STOP) {
                break;
            }
        }
        while (FindNextFile(h, &data));

        goto done;
    }

    h = FindFirstFile((LPCSTR)dirpath, &data);
    if (h == INVALID_HANDLE_VALUE) {
        return LXB_STATUS_ERROR;
    }

    do {
        if (opt & LEXBOR_FS_DIR_OPT_WITHOUT_HIDDEN
            && *data.cFileName == '.') {
            continue;
        }

        d_namlen = strlen(data.cFileName);

        if (d_namlen >= free_len) {
            goto error;
        }

        if ((data.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_NORMAL)) != 0) {
            f_type = lexbor_fs_file_type((const lxb_char_t *) data.cFileName);

            if (opt & LEXBOR_FS_DIR_OPT_WITHOUT_DIR
                && f_type == LEXBOR_FS_FILE_TYPE_DIRECTORY) {
                continue;
            }

            if (opt & LEXBOR_FS_DIR_OPT_WITHOUT_FILE
                && f_type == LEXBOR_FS_FILE_TYPE_FILE)
            {
                continue;
            }
        }
        else {
            if (opt & LEXBOR_FS_DIR_OPT_WITHOUT_DIR
                && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
            {
                continue;
            }

            if (opt & LEXBOR_FS_DIR_OPT_WITHOUT_FILE
                && (data.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) == FILE_ATTRIBUTE_NORMAL)
            {
                continue;
            }
        }

        /* +1 copy terminating null byte '\0' */
        memcpy(file_begin, data.cFileName, (d_namlen + 1));

        action = callback((const lxb_char_t *) full_path,
                          (path_len + d_namlen),
                          (const lxb_char_t *) data.cFileName,
                          d_namlen, ctx);
        if (action == LEXBOR_ACTION_STOP) {
            break;
        }
    }
    while (FindNextFile(h, &data));

done:

    FindClose(h);

    return LXB_STATUS_OK;

error:

    FindClose(h);

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
        case S_IFCHR:
            return LEXBOR_FS_FILE_TYPE_CHARACTER_DEVICE;

        case S_IFDIR:
            return LEXBOR_FS_FILE_TYPE_DIRECTORY;

        case S_IFREG:
            return LEXBOR_FS_FILE_TYPE_FILE;

        default:
            return LEXBOR_FS_FILE_TYPE_UNDEF;
    }

    return LEXBOR_FS_FILE_TYPE_UNDEF;
}

lxb_char_t *
lexbor_fs_file_easy_read(const lxb_char_t *full_path, size_t *len)
{
    LARGE_INTEGER size;
    HANDLE fh;
    char *data;
    DWORD nread = 0;

    fh = CreateFile((LPCSTR)full_path, GENERIC_READ, FILE_SHARE_READ,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fh == INVALID_HANDLE_VALUE) {
        goto error;
    }

    if (GetFileSizeEx(fh, &size) == FALSE) {
        goto error_close;
    }

    data = lexbor_malloc(size.QuadPart + 1);
    if (data == NULL) {
        goto error_close;
    }

    if (ReadFile(fh, data, (DWORD) size.QuadPart, &nread, NULL) != TRUE) {
        goto error_close;
    }

    CloseHandle(fh);

    if ((LONGLONG) nread != size.QuadPart) {
            goto error;
    }

    if (len != NULL) {
        *len = (size_t)size.QuadPart;
    }

    data[size.QuadPart] = '\0';

    return (lxb_char_t *)data;

error_close:

    CloseHandle(fh);

error:

    if (len != NULL) {
        *len = 0;
    }

    return NULL;
}
