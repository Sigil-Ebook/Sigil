#ifndef DBUF_H
#define DBUF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { char *d; size_t l; size_t c; } DBuf;

static DBuf dbuf_new(size_t cap) {
    DBuf str;
    str.d = (char *)malloc(cap);
    if (str.d == NULL) { fprintf(stderr, "Allocation failed!\n"); exit(1); }
    str.d[0] = '\0'; str.l = 0; str.c = cap;
    return str;
}

/* text need not be null terminated, but buf always is null terminated */
static void dbuf_cat(DBuf *str, const char *text, size_t tl) {
    if (tl == 0) return;
    size_t rc = str->l + tl + 1; 
    if (rc > str->c) {
        size_t nc = str->c * 2; if (nc < rc) nc = rc;
        char *nd = (char *)realloc(str->d, nc);
        if (nd == NULL) { fprintf(stderr, "Reallocation failed!\n"); return; }
        str->d = nd; str->c = nc;
    } 
    memcpy(str->d + str->l, text, tl); str->l += tl; 
    str->d[str->l] = '\0';
}

static char* dbuf_cstr(DBuf* src) {
    char *cstr = (char*)malloc(src->l + 1);
    if (cstr == NULL) { fprintf(stderr, "Allocation failed!\n"); exit(1); }
    memcpy(cstr, (char*) src->d, src->l);
    cstr[src->l] = '\0';
    return cstr;
}

static void dbuf_free(DBuf *str) { if (str->d != NULL) free(str->d); str->d = NULL; str->l = 0; str->c = 0; }
static void dbuf_null(DBuf* str) { str->d[0] = '\0'; str->l = 0; }
static void dbuf_set(DBuf* str, char * text, size_t len) { dbuf_null(str); dbuf_cat(str, text, len); }
static void dbuf_cpy(DBuf* dst, DBuf* src) { dbuf_null(dst); dbuf_cat(dst, src->d, src->l); }

#if 0 /* currently not needed */
static int  dbuf_cmp(DBuf *s1, DBuf *s2) { return strcmp(s1->d, s2->d); }

/* text need not be null terminated */
static int dbuf_txtcmp(DBuf* str, char *text, size_t tl) {
     DBuf tmp = dbuf_new(tl + 1); dbuf_set(&tmp, text, tl);
     int rv = dbuf_cmp(str, &tmp); dbuf_free(&tmp);
     return rv;
}
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* DBUF_H */
