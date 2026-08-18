#ifndef CSS_STRUCTURE_PARSER_H
#define CSS_STRUCTURE_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lexbor/core/fs.h>
#include <lexbor/core/array.h>
#include <lexbor/css/css.h>
#include "Parsers/DBuf.h"
#include "Parsers/cssinterface_defn.h"


typedef struct {
    lxb_char_t * startptr;
    size_t srclen;
    lexbor_array_t *newlines;
    lexbor_array_t *newlines16;
    lexbor_array_t *emessages;
    lexbor_array_t *blkstack;
    csstoken_type  atk_kind;
    size_t         pos;
    size_t         line;
    size_t         col;
    DBuf           tkinfo;
    DBuf           tkdata;
    DBuf           tkpos;
    DBuf           curblk;
    DBuf           at_type;
    DBuf           at_args;
    void *cpp_instance;
    sigil_add_csstoken_callback_f add_csstoken_cb;
    sigil_add_error_callback_f    add_error_cb;
} CtxData;


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CSS_STRUCTURE_PARSER_H */
