/*
 * Copyright (C) 2018 Alexander Borisov
 *
 * Author: Vincent Torri <vincent.torri@gmail.com>
 */

#include "lexbor/core/perf.h"

#include <windows.h>


typedef struct lexbor_perf {
    LARGE_INTEGER start;
    LARGE_INTEGER end;
    LARGE_INTEGER freq;
}
lexbor_perf_t;


void *
lexbor_perf_create(void)
{
    lexbor_perf_t *perf = lexbor_calloc(1, sizeof(lexbor_perf_t));
    if (perf == NULL) {
        return NULL;
    }

    /*
     * According to MSDN, QueryPerformanceFrequency() never fails
     * on Windows XP or later
     */
    QueryPerformanceFrequency(&perf->freq);
    return perf;
}

void
lexbor_perf_clean(void *perf)
{
    memset(perf, 0, sizeof(lexbor_perf_t));
}

void
lexbor_perf_destroy(void *perf)
{
    if (perf != NULL) {
        lexbor_free(perf);
    }
}

lxb_status_t
lexbor_perf_begin(void *perf)
{
    /*
     * According to MSDN, QueryPerformanceCounter() never fails
     * on Windows XP or later
     */
    QueryPerformanceCounter(&(((lexbor_perf_t *) (perf))->start));

    return LXB_STATUS_OK;
}

lxb_status_t
lexbor_perf_end(void *perf)
{
    /*
     * According to MSDN, QueryPerformanceCounter() never fails
     * on Windows XP or later
     */
    QueryPerformanceCounter(&(((lexbor_perf_t *) (perf))->end));

    return LXB_STATUS_OK;
}

double
lexbor_perf_in_sec(void *perf)
{
    lexbor_perf_t *obj_perf = (lexbor_perf_t *) perf;

    return ((double) (obj_perf->end.QuadPart - obj_perf->start.QuadPart)
            / (double)obj_perf->freq.QuadPart);
}
