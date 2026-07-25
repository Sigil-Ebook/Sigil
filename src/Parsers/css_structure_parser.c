/*
 * Copyright (C) 2026 Kevin B. Hendricks, Stratford, ON, Canada
 *
 * Based on example/lexbor/css/syntax/structure_parse_file.c
 * which is Copyright (C) 2022-2025 Alexander Borisov <borisov@lexbor.com>
 *              
 */

#include "lexbor/core/array.h"
#include "lexbor/css/css.h"
#include "lexbor/css/syntax/token.h"
#include "Parsers/css_structure_parser.h"

/* used to validate @ rule names */
static const char * ATRULE_SET1 = "@charset|@container|@counter-style|@font-face|@font-feature-values|"
                                  "@font-palette-values|@function|@import|@keyframes|@layer|@media|"
                                  "@namespace|@page|@position-try|@property|@scope|@starting-style|@supports|";

static const char * ATRULE_SET2 = "@top-left|@top-left-corner|@top-center|@top-right|@top-right-corner|"
                                  "@right-top|@right-middle|@right-bottom|@left-bottom|@left-middle|@left-top|"
                                  "@bottom-right|@bottom-right-corner|@bottom-center|@bottom-left|"
                                  "@bottom-left-corner|";

static const char * PROP_BAD_START   = " !\"#$%&'()*+,./0123456789:;<=>?~{|}^[]`";
static const char * PROP_BAD_CHARS   = " !\"#$%&'()*+,./:;<=>?~{|}^[]`";
static const char * SEL_BAD_START   = "0123456789";


/* forward definitions */


/* accessor functions for user context */
static char* get_token_data(void *uctx);

static void set_token_data(char* data, size_t len, void *uctx);

static void append_to_token_data(char * data, size_t len, void *uctx);

static void replace_newlines_in_token_data(void *uctx);

static void set_tokenstate(csstoken_type tt, void *uctx);

static void set_curblk(void* uctx); 

static void append_to_curblk(char* data, size_t len, void* uctx);

static void push_curblk_to_blkstack(void *uctx);

static char* pop_curblk_from_blkstack(void *uctx);

static char* get_attype(void *uctx);

static void set_attype(void *uctx); 

static void set_atargs(void *uctx);

static char* get_atargs(void *uctx);

static void process_single_token(const lxb_css_syntax_token_t* token, void *uctx);

static void add_syntax_error(char *message, char* addendum, void *uctx);

static void add_other_error(char* message, void *uctx);

static void build_position_tables(const lxb_char_t *data, size_t length, void *uctx);

static size_t convert_col8_to_col16(size_t line, size_t col, void *uctx);

static size_t convert_line_col16_to_pos16(size_t line, size_t col16, void *uctx);

static void position_from_pos8(size_t pos, size_t *aline, size_t *acol, size_t* apos, void *uctx);

static void position_from_token(const lxb_css_syntax_token_t * token, size_t *aline,
                                size_t *acol, size_t * apos, void *uctx);

static void position_info(const lxb_css_syntax_token_t * token, void *uctx);

static void check_token(const lxb_css_syntax_token_t * token, void *uctx);

static void check_value(const char *data, size_t len, void *uctx);

static void check_atrule(const char *data, size_t len, void *uctx);

static void check_pname(const char *data, size_t len, void *uctx);

static void check_selector(const char *data, size_t len, void *uctx);

static bool check_for_invalid_rule(const lxb_css_syntax_token_t* token, void *uctx);

static void css_consule_tokens(lxb_css_parser_t *parser, 
                               const lxb_css_syntax_token_t *token, bool cws, void *uctx);

static lxb_status_t
css_parse(lxb_css_parser_t *parser, const lxb_char_t *data, size_t length, void *ctx);


/* call back target forward declarations */

static void comment_info(const lxb_css_syntax_token_t * token, void *uctx);

static lxb_status_t token_cb_f(const lxb_char_t *data, size_t len, void *uctx);


static bool
css_blank_list_rules_next(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_blank_list_rules_end(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token,
                         void *ctx, bool failed);

static const lxb_css_syntax_cb_at_rule_t *
css_blank_at_rule_begin(lxb_css_parser_t *parser,
                        const lxb_css_syntax_token_t *token, void *ctx,
                        void **out_rule);

static bool
css_blank_at_rule_prelude(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token,
                          void *ctx);

static lxb_status_t
css_blank_at_rule_prelude_end(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              void *ctx, bool failed);

static const lxb_css_syntax_cb_block_t *
css_blank_at_rule_block_begin(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              void *ctx, void **out_rule);
static bool
css_blank_at_rule_prelude_failed(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 void *ctx);
static lxb_status_t
css_blank_at_rule_end(lxb_css_parser_t *parser,
                      const lxb_css_syntax_token_t *token,
                      void *ctx, bool failed);

static const lxb_css_syntax_cb_qualified_rule_t *
css_blank_qualified_rule_begin(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token,
                               void *ctx, void **out_rule);

static bool
css_blank_qualified_rule_prelude(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 void *ctx);

static lxb_status_t
css_blank_qualified_rule_prelude_end(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token,
                                     void *ctx, bool failed);

static const lxb_css_syntax_cb_block_t *
css_blank_qualified_rule_block_begin(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token,
                                     void *ctx, void **out_rule);

static bool
css_blank_qualified_rule_prelude_failed(lxb_css_parser_t *parser,
                                        const lxb_css_syntax_token_t *token,
                                        void *ctx);
static lxb_status_t
css_blank_qualified_rule_end(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, bool failed);
static bool
css_blank_block_next(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_blank_block_end(lxb_css_parser_t *parser,
                    const lxb_css_syntax_token_t *token,
                    void *ctx, bool failed);

static const lxb_css_syntax_cb_declarations_t *
css_blank_declarations_begin(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, void **out_rule);

static lxb_css_parser_state_f
css_blank_declaration_name(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token,
                           void *ctx, void **out_rule);
static bool
css_blank_declaration_value(lxb_css_parser_t *parser,
                            const lxb_css_syntax_token_t *token, void *ctx);

static lxb_status_t
css_blank_declaration_end(lxb_css_parser_t *parser,
                          void *declarations, void *ctx,
                          const lxb_css_syntax_token_t *token,
                          lxb_css_syntax_declaration_offset_t *offset,
                          bool important, bool failed);

static lxb_status_t
css_blank_declarations_end(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token,
                           void *ctx, bool failed);

static bool
css_blank_declarations_bad(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token, void *ctx);

/* end of forward definitions */

/* static initializers for register parsing callbacks for syntax */

static const lxb_css_syntax_cb_list_rules_t lxb_css_blank_list_rules = {
    .at_rule = css_blank_at_rule_begin,
    .qualified_rule = css_blank_qualified_rule_begin,
    .next = css_blank_list_rules_next,
    .cb.failed = lxb_css_state_failed,
    .cb.end = css_blank_list_rules_end
};

static const lxb_css_syntax_cb_at_rule_t lxb_css_blank_at_rule = {
    .prelude = css_blank_at_rule_prelude,
    .prelude_end = css_blank_at_rule_prelude_end,
    .block = css_blank_at_rule_block_begin,
    .cb.failed = css_blank_at_rule_prelude_failed,
    .cb.end = css_blank_at_rule_end
};

static const lxb_css_syntax_cb_qualified_rule_t lxb_css_blank_qualified_rule = {
    .prelude = css_blank_qualified_rule_prelude,
    .prelude_end = css_blank_qualified_rule_prelude_end,
    .block = css_blank_qualified_rule_block_begin,
    .cb.failed = css_blank_qualified_rule_prelude_failed,
    .cb.end = css_blank_qualified_rule_end
};

static const lxb_css_syntax_cb_block_t lxb_css_blank_block = {
    .at_rule = css_blank_at_rule_begin,
    .declarations = css_blank_declarations_begin,
    .qualified_rule = css_blank_qualified_rule_begin,
    .next = css_blank_block_next,
    .cb.failed = lxb_css_state_failed,
    .cb.end = css_blank_block_end,
};

static const lxb_css_syntax_cb_declarations_t lxb_css_blank_declaration = {
    .name = css_blank_declaration_name,
    .end = css_blank_declaration_end,
    .cb.failed = css_blank_declarations_bad,
    .cb.end = css_blank_declarations_end
};

/* end registering callbacks */


static void check_token(const lxb_css_syntax_token_t * token, void* uctx)
{
    /* CtxData* actx = (CtxData*)(uctx); */
    lxb_char_t* begin = (lxb_char_t *)lxb_css_syntax_token_base(token)->begin;
    size_t len = lxb_css_syntax_token_base(token)->length;
    if (begin == NULL) {
        add_syntax_error("Illegal Empty Token data", NULL, uctx);    
    }
    char* name = (char *) lxb_css_syntax_token_type_name_by_id(token->type);
    if (strcmp(name, "bad-string") == 0) {
        char abuf[51];
        if (len > 50) len = 50;
        (void) snprintf(abuf, len, "%s", begin);
        for(size_t i=0; i < len; i++) { if (abuf[i] == '\n') abuf[i] = ' '; }
        add_syntax_error("Bad String", abuf, uctx);    
    }   
    if (strcmp(name, "bad-url") == 0) {
        char abuf[51];
        if (len > 50) len = 50;
        (void) snprintf(abuf,len, "%s",begin);
        for(size_t i=0; i < len; i++) { if (abuf[i] == '\n') abuf[i] = ' '; }
        add_syntax_error("Bad Url",abuf, uctx);    
    }
}

static void check_value(const char* data, size_t len, void *uctx)
{
    /* CtxData* actx = (CtxData*)(uctx); */
    bool in_q1 = false;
    bool in_q2 = false;
    char pc = ' ';
    char* p = (char *) data;
    for (size_t i=0; i < len; i++) {
        if ((*p == '"') && (pc != '\\')) in_q2 = !in_q2;
        if ((*p == '\'') && (pc != '\\')) in_q1 = !in_q1;
        if ((*p == ':') && (pc != '\\') && !in_q2 && !in_q1) {
            add_syntax_error("Missing semicolon after value", NULL, uctx);
            break;
        }
        pc = *p++;
    }
}

static void check_atrule(const char *data, size_t len, void *uctx)
{
    /* CtxData* actx = (CtxData*)(uctx); */
    DBuf search_term = dbuf_new(40);
    dbuf_null(&search_term);
    dbuf_cat(&search_term, data, len);
    dbuf_cat(&search_term, "|", 1);
    bool found = (strstr(ATRULE_SET1, search_term.d) != NULL);
    if (!found) found = (strstr(ATRULE_SET2, search_term.d) != NULL);
    if (!found) add_syntax_error("Unrecognized At-rule", (char *)data, uctx);
    dbuf_free(&search_term);
}

static void check_pname(const char *data, size_t len, void *uctx)
{
    /* CtxData* actx = (CtxData*)(uctx); */
    DBuf pname = dbuf_new(128);
    dbuf_set(&pname, (char *)data, len);
    uint8_t bval = *data;
    if ((bval < 32) || (strchr(PROP_BAD_START, bval) != NULL)) {
        add_syntax_error("Bad initial character in property name", pname.d, uctx);
    }
    uint8_t pc = (uint8_t)*data;
    char* p = (char *)data;
    for (size_t i=0; i < len; i++) {
        bval = *p;
        if ((strchr(PROP_BAD_CHARS, bval) != NULL) && (pc != '\\')) {
            add_syntax_error("Bad unescaped char in property name", pname.d, uctx);
        }
        pc = *p++;
    }
    dbuf_free(&pname);
}

static void check_selector(const char *data, size_t len, void* uctx)
{
    /* CtxData* actx = (CtxData*)(uctx); */
    if ((data == NULL) || (len == 0) ) {
        add_syntax_error("Illegal Empty Selector", NULL, uctx);
        return;
    }
    DBuf sel_ident = dbuf_new(128);
    dbuf_set(&sel_ident, (char *)data, len);
    uint8_t bval = *data;
    if ((bval < 32) || (strchr(SEL_BAD_START, bval) != NULL)) {
        add_syntax_error("Bad initial character in selector", sel_ident.d, uctx);
    }
    if ((bval == '-') && (len > 1)) {
        uint8_t nc = data[1];
        if (strchr(SEL_BAD_START, nc) != NULL) {
            add_syntax_error("Bad initial characters in selector", sel_ident.d, uctx);
        }
    }
    /* FIX ME: add more extensive tests here */
    dbuf_free(&sel_ident);
}

/* look forward for first unquoted and unescaped ; or { to check validity */
static bool check_for_invalid_rule(const lxb_css_syntax_token_t* token, void *uctx)
{
    /* null token already caught in check_token */
    if (token == NULL) return false; 
    CtxData* actx = (CtxData*)(uctx);    
    lxb_char_t* begin = (lxb_char_t *)lxb_css_syntax_token_base(token)->begin;
    size_t len = lxb_css_syntax_token_base(token)->length;
    
    /* empty selector will be caught in check_selector() */
    if (begin == NULL) return false; 
    
    DBuf sel_ident = dbuf_new(128);
    dbuf_set(&sel_ident, (char *) begin, len);
    bool in_q1 = false;
    bool in_q2 = false;
    char pc = ' ';
    char* p = (char *) begin;
    while(p < (char *) (actx->startptr + actx->srclen)) {
        if ((*p == '"') && (pc != '\\')) in_q2 = !in_q2;
        if ((*p == '\'') && (pc != '\\')) in_q1 = !in_q1;
        if (((*p == ';') || (*p == '{')) && pc != '\\' && !in_q2 && !in_q1) {
            if (*p == ';') {
                add_syntax_error("Invalid Rule", sel_ident.d, uctx);
                dbuf_free(&sel_ident);
                return true;
            } else {
                dbuf_free(&sel_ident);
                return false;
            }
        }
        pc = *p++;
    }
    dbuf_free(&sel_ident);
    return false;
} 

static void add_syntax_error(char* message, char* addendum, void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    DBuf msgbuffer = dbuf_new(128);
    dbuf_set(&msgbuffer, "CSS Syntax Error near: ", 23);
    dbuf_cat(&msgbuffer, actx->tkpos.d, actx->tkpos.l);
    dbuf_cat(&msgbuffer, " > ", 3);
    size_t len = strlen(message);
    dbuf_cat(&msgbuffer, message, len);
    if (addendum != NULL) {
        dbuf_cat(&msgbuffer, "(", 1);
        dbuf_cat(&msgbuffer, addendum, strlen(addendum));
        dbuf_cat(&msgbuffer, ")", 1);
    }    
    lexbor_array_push(actx->emessages,(void *)dbuf_cstr(&msgbuffer));
    if ((actx->add_error_cb != NULL) && (actx->cpp_instance != NULL)) {
        (*actx->add_error_cb)(msgbuffer.d, actx->cpp_instance);
    }
    dbuf_free(&msgbuffer);
}


static void add_other_error(char* message, void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    DBuf msgbuffer = dbuf_new(128);
    size_t len = strlen(message);
    dbuf_set(&msgbuffer, message, len);
    lexbor_array_push(actx->emessages,(void *)dbuf_cstr(&msgbuffer));
    if ((actx->add_error_cb != NULL) && (actx->cpp_instance != NULL)) {
        (*actx->add_error_cb)(msgbuffer.d, actx->cpp_instance);
    }
    dbuf_free(&msgbuffer);
}


static void push_csstoken(void* uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    /* callback into c++ code */
    if ((actx->cpp_instance != NULL) && (actx->add_csstoken_cb != NULL)) {
        (*actx->add_csstoken_cb)(actx->atk_kind, 
                                actx->pos, 
                                actx->line, 
                                actx->col, 
                                actx->tkdata.d, 
                                actx->cpp_instance);
    }
}


static void
build_position_tables(const lxb_char_t *data, size_t length, void *uctx)
{
    /* initialize the array of newline offsets in the data for both 
     * utf-8 encoding and utf-16 encoding 
     */
    CtxData* actx = (CtxData*)(uctx);
    if (!actx->startptr) {
        actx->startptr = (lxb_char_t *)data;
        actx->srclen = length;
    }
    lxb_char_t* p = (lxb_char_t*)data;
    actx->newlines = lexbor_array_create();
    lexbor_array_init(actx->newlines, 100);
    actx->newlines16 = lexbor_array_create();
    lexbor_array_init(actx->newlines16, 100);
    size_t pos8 = 0;
    size_t pos16 = 0;
    while (p < data + length) {
        uint8_t bval = *p; 
        if (bval == 10) {
            lexbor_array_push(actx->newlines,(void *) (pos8));
            lexbor_array_push(actx->newlines16,(void *)(pos16));
        }
        pos8++;
        if ((bval & 0xC0) != 0x80) {
            pos16++;
            if (bval >= 0xF0) pos16++;
        }
        p++;
    }
    lexbor_array_push(actx->newlines, (void *) (pos8));
    lexbor_array_push(actx->newlines16,(void *)(pos16+1));
}

static size_t convert_col8_to_col16(size_t line, size_t col, void *uctx) 
{
    CtxData* actx = (CtxData*)(uctx);
    size_t sp8 = 0;
    size_t sl = line - 1;
    if (sl > 0) {
        /* start point is char after the previous new line (sl -1) or 0 */ 
        sp8 = (size_t) (lexbor_array_get_noi(actx->newlines, sl-1)) + 1;
    } else {
        /* start point is char after the previous new line or 0 if first line */ 
        sp8 = 0;
    }
    size_t col8 = 1;
    size_t col16 = 1;
    lxb_char_t* p = actx->startptr + sp8;
    while (col8 < col) {
        uint8_t bval = *p;
        if ((bval & 0xC0) != 0x80) {
            col16++;
            if (bval >= 0xF0) col16++;
        }
        col8++;
        p++;
    }
    return col16;
}

static size_t convert_line_col16_to_pos16(size_t line, size_t col16, void *uctx) 
{
    CtxData* actx = (CtxData*)(uctx);
    size_t pos16;
    /* start point is previous new line  or 0 */
    size_t sl = line - 1;
    if (sl > 0) {
        pos16 = (size_t) (lexbor_array_get_noi(actx->newlines16, sl - 1)) + col16;
    } else {
        pos16 = col16 - 1;
    }
    return pos16;
}

static void position_from_pos8(const size_t pos, size_t *aline, size_t *acol, size_t* apos, void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    size_t line = 0;
    size_t col = 0;
    size_t p = pos;
    size_t sl = 0;
    size_t n = lexbor_array_length_noi(actx->newlines);
    while ((sl < n) && p > (size_t) (lexbor_array_get_noi(actx->newlines, sl))) {
        sl++;
    }
    line = sl + 1;
    /* we want position of previous line end or 0 if in the first line */
    if (sl > 0) {
        col = pos - (size_t)(lexbor_array_get_noi(actx->newlines, sl-1));
    } else {
        col = pos + 1;
    }
    /* now convert all to their utf-16 equivalents */
    size_t line16 = line;
    size_t col16 = convert_col8_to_col16(line, col, uctx);
    size_t pos16 = convert_line_col16_to_pos16(line16, col16, uctx);
    *aline = line16;
    *acol = col16;
    *apos = pos16;
}

static void position_from_token(const lxb_css_syntax_token_t * token, size_t* aline,
                                size_t* acol, size_t* apos, void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    if (token == NULL) return;
    lxb_char_t* begin = (lxb_char_t *)lxb_css_syntax_token_base(token)->begin;
    /* size_t len = lxb_css_syntax_token_base(token)->length; */
    if (begin == NULL) {
        check_token(token, uctx);
        return;
    }
    size_t pos = begin - actx->startptr;
    size_t line16 = 0;
    size_t col16 = 0;
    size_t pos16 = 0;
    position_from_pos8(pos, &line16, &col16, &pos16, uctx);
    *aline = line16;
    *acol = col16;
    *apos = pos16;
}


static void position_info(const lxb_css_syntax_token_t * token, void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    if (token == NULL) return;
    size_t line16 = 0;
    size_t col16 = 0;
    size_t pos16 = 0;
    position_from_token(token, &line16, &col16, &pos16, uctx);
    /* if invalid position info do not update from last position for better error reporting */
    if ((line16 >= 1) && (col16 >= 1)) {
        dbuf_null(&actx->tkpos);
        char abuf[256];
        int ol = snprintf(abuf,255,"[line:%5zu col:%4zu]", line16, col16);
        dbuf_cat(&actx->tkpos, abuf, ol);
        actx->pos = pos16;
        actx->line = line16;
        actx->col = col16;
    }
}


unsigned int parse_css_structure_with_user_context(const char* data, size_t len, 
                     void* cpp_this, 
                     sigil_add_csstoken_callback_f add_csstoken_method, 
                     sigil_add_error_callback_f add_error_method)
{
    lxb_css_syntax_tokenizer_t *tkz;
    lxb_status_t status;
    lxb_css_parser_t *parser;
    unsigned int rv = 0;

    /* initialize context */
    CtxData actx = {
       .startptr   = NULL,
       .srclen     = 0,
       .newlines   = NULL,
       .newlines16 = NULL,
       .emessages  = NULL,
       .blkstack   = NULL,
       .atk_kind   = TKN_CSS_END,
       .pos        = 0,
       .line       = 0,
       .col        = 0,
       .tkinfo     = {NULL, 0, 0},
       .tkdata     = {NULL, 0, 0},
       .tkpos      = {NULL, 0, 0},
       .curblk     = {NULL, 0, 0},
       .at_type    = {NULL, 0, 0},
       .at_args    = {NULL, 0, 0},
       .cpp_instance = cpp_this,
       .add_csstoken_cb = add_csstoken_method,
       .add_error_cb = add_error_method
    };
    actx.tkinfo = dbuf_new(64); 
    actx.tkdata = dbuf_new(64);
    actx.tkpos = dbuf_new(40);
    actx.curblk = dbuf_new(64);
    actx.at_type = dbuf_new(32);
    actx.at_args = dbuf_new(64);

    actx.newlines = lexbor_array_create();
    lexbor_array_init(actx.newlines, 100);

    actx.newlines16 = lexbor_array_create();
    lexbor_array_init(actx.newlines, 100);

    actx.emessages = lexbor_array_create();
    lexbor_array_init(actx.emessages, 30);

    actx.blkstack = lexbor_array_create();
    lexbor_array_init(actx.blkstack, 30);

    tkz = lxb_css_syntax_tokenizer_create();
    status = lxb_css_syntax_tokenizer_init(tkz);
    if (status != LXB_STATUS_OK) {
        add_other_error("Failed to create CSS:Syntax tokenizer", (void*)&actx);
        rv = 1; /* failure */
        goto cleanup_user_context;
    }
    /* register callback for comments from tokenizer with our user context */
    tkz->with_comment = true;
    tkz->comment_cb = comment_info;
    tkz->uctx = (void*)&actx; /* user context */

    parser = lxb_css_parser_create();
    status = lxb_css_parser_init(parser, tkz);
    if (status != LXB_STATUS_OK) {
        add_other_error("Failed to create CSS Parser", (void*)&actx);
        rv = 1; /* failure */
        goto cleanup_tokenizer;
    }
    
    /* include our user context */
    parser->uctx = (void*) &actx; /* user context */

    /* do the actual parsing */
    status = css_parse(parser, (const lxb_char_t*)data, len, NULL);

    if (status != LXB_STATUS_OK) {
        add_other_error("Failed to parse CSS", (void*)&actx);
        rv = 1; /* failure */
    }

    /* post tokenizer errors */
    size_t error_count = lexbor_array_obj_length_noi(tkz->parse_errors);
    if (error_count > 0) {
        lxb_css_syntax_tokenizer_error_t* err;
        for (size_t i=0; i < error_count; i++) {
            char* errormsg = NULL;
            err = (lxb_css_syntax_tokenizer_error_t*) lexbor_array_obj_get_noi(tkz->parse_errors, i);
            if (err->id == 0) errormsg = "Unexpected EOF";
            if (err->id == 1) errormsg = "EOF in Comment";
            if (err->id == 2) errormsg = "EOF in String";
            if (err->id == 3) errormsg = "EOF in Url";
            if (err->id == 4) errormsg = "EOF in Escaped";
            if (err->id == 5) errormsg = "QO in Url";
            if (err->id == 6) errormsg = "Wrong Escape In Url";
            if (err->id == 7) errormsg = "Newline in String";
            if (err->id == 8) errormsg = "Bad Character";
            if (err->id == 9) errormsg = "Bad Codepoint";
            size_t line16 = 0;
            size_t col16 = 0;
            size_t pos16 = 0;
            size_t pos = err->pos - actx.startptr;
            position_from_pos8(pos, &line16, &col16, &pos16, (void*) &actx);
            char abuf[128];
            size_t olen = snprintf(abuf, 127, "Tokenizer  Error near: [line:%5zu col:%4zu] > %s\n",
                                   line16, col16, errormsg);
            abuf[olen] = '\0';
            add_other_error(abuf, &actx);
            /* tokenizer errors are cleaned up during tokenizer destroy */
        }
    }

    /* post parser errors */
    if (parser != NULL) {
        size_t parser_error_count = lexbor_array_obj_length_noi(&parser->log->messages);
        if (parser_error_count > 0) {
            DBuf ebuf = dbuf_new(100);
            for (size_t i=0; i < parser_error_count; i++) {
                lxb_css_log_message_t* msg = lexbor_array_obj_get(&parser->log->messages, i);
                size_t length = 0;
                lxb_char_t* type_name = (lxb_char_t*) lxb_css_log_type_by_id(msg->type, &length);
                dbuf_null(&ebuf);
                dbuf_set(&ebuf, "Parser ",7); 
                dbuf_cat(&ebuf, (char*) type_name, length);
                dbuf_cat(&ebuf, ". ", 2);
                dbuf_cat(&ebuf, (char*)msg->text.data, msg->text.length);
                add_other_error(ebuf.d, &actx);
            }
            dbuf_free(&ebuf);
            /* parser error log is cleaned up during parser destroy */
        }
    }

    /* do clean ups */

 cleanup_tokenizer:

    (void) lxb_css_syntax_tokenizer_destroy(tkz);

    (void) lxb_css_parser_destroy(parser, true);

cleanup_user_context:

    /* arrays used to map line, column pos information for utf-8 and utf-16 */
    lexbor_array_destroy(actx.newlines, true);
    lexbor_array_destroy(actx.newlines16, true);

    /* arrays used to store our own error messages */
    size_t cnt = lexbor_array_length_noi(actx.emessages);
    for (size_t i=0; i < cnt; i++) {
       char* msg = (char *)lexbor_array_get_noi(actx.emessages, i);
       free(msg);
    }
    lexbor_array_destroy(actx.emessages, true);

    /* array to store our block stack strings */
    size_t acnt = lexbor_array_length_noi(actx.blkstack);
    for (size_t i=0; i < acnt; i++) {
       char* sitem = (char *)lexbor_array_get_noi(actx.blkstack, i);
       free(sitem);
    }
    lexbor_array_destroy(actx.blkstack, true);

    /* free up the DBuf buffers */
    (void) dbuf_free(&actx.tkinfo);
    (void) dbuf_free(&actx.tkdata);
    (void) dbuf_free(&actx.tkpos);
    (void) dbuf_free(&actx.at_type);
    (void) dbuf_free(&actx.at_args);
    (void) dbuf_free(&actx.curblk);

    return rv;
}


static lxb_status_t
css_parse(lxb_css_parser_t *parser, const lxb_char_t *data, size_t length, void *ctx)
{
    (void)ctx;
    
    CtxData* actx = (CtxData*)(parser->uctx);
    lxb_css_syntax_rule_t *rule;
    lxb_css_parser_buffer_set(parser, data, length);
    build_position_tables(data, length, (void*)actx);
    rule = lxb_css_syntax_parser_list_rules_push(parser, &lxb_css_blank_list_rules,
                                                 NULL, NULL, LXB_CSS_SYNTAX_TOKEN_UNDEF);
    if (rule == NULL) {
        return LXB_STATUS_ERROR;
    }
    return lxb_css_syntax_parser_run(parser);
}

lxb_status_t token_cb_f(const lxb_char_t *data, size_t len, void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    dbuf_cat(&actx->tkdata, (char *)data, len);
    return LXB_STATUS_OK;
}

static void append_to_token_data(char * data, size_t len, void *uctx) 
{
    CtxData* actx = (CtxData*)(uctx);
    dbuf_cat(&actx->tkdata, (char*) data, len);
}

static char* get_token_data(void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    return actx->tkdata.d;
}

static void replace_newlines_in_token_data(void *uctx)
{
    (void)uctx;
    /* CtxData* actx = (CtxData*)(uctx); */
    /* replace newlines to get proper value string */
    /* for (size_t i=0; i < actx->tkdata.l; i++) { if (actx->tkdata.d[i] == '\n') actx->tkdata.d[i] = ' '; } */
}

static void set_token_data(char* data, size_t len, void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    dbuf_set(&actx->tkdata, data, len);
}

static void set_tokenstate(csstoken_type tt, void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    actx->atk_kind = tt;
}

static void set_curblk(void* uctx) 
{
    CtxData* actx = (CtxData*)(uctx);
    dbuf_null(&actx->curblk);
    dbuf_cpy(&actx->curblk, &actx->tkdata);
}

static void append_to_curblk(char* data, size_t len, void* uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    dbuf_cat(&actx->curblk, data, len);
}

static void push_curblk_to_blkstack(void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    lexbor_array_push(actx->blkstack,(void *) dbuf_cstr(&actx->curblk));
}

static char* pop_curblk_from_blkstack(void * uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    return (char *)lexbor_array_pop(actx->blkstack);
}

static void set_attype(void *uctx) 
{
    CtxData* actx = (CtxData*)(uctx);
    dbuf_null(&actx->at_type);
    dbuf_cpy(&actx->at_type, &actx->tkdata);
}

static char* get_attype(void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    return actx->at_type.d;
}

static void set_atargs(void* uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    dbuf_null(&actx->at_args);
    dbuf_cpy(&actx->at_args, &actx->tkdata);
}

static char* get_atargs(void* uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    return actx->at_args.d;
}

static void process_single_token(const lxb_css_syntax_token_t* token, void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    dbuf_null(&actx->tkdata);
    if (token == NULL) return;
    if ((token->type == LXB_CSS_SYNTAX_TOKEN_STRING) ||
        (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) ) {
        /* need to use raw token data here as internal string data has been unescaped! */
        lxb_char_t* begin = (lxb_char_t *)lxb_css_syntax_token_base(token)->begin;
        size_t len = lxb_css_syntax_token_base(token)->length;
        dbuf_cat(&actx->tkdata, (char*)begin, len); /* use base raw data to save escapes */
    } else {
        (void) lxb_css_syntax_token_serialize(token, token_cb_f, uctx);
    }
    check_token(token, uctx);
}

static void
css_consule_tokens(lxb_css_parser_t *parser,
                   const lxb_css_syntax_token_t *token, bool cws, void *uctx)
{
    CtxData* actx = (CtxData*)(uctx);
    dbuf_null(&actx->tkdata);
    if (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
        position_info(token, uctx);
        while (token != NULL && token->type != LXB_CSS_SYNTAX_TOKEN__END) {
            if (token->type == LXB_CSS_SYNTAX_TOKEN_WHITESPACE && cws) {
                 dbuf_cat(&actx->tkdata, " ", 1);
            } else if ((token->type == LXB_CSS_SYNTAX_TOKEN_STRING) ||
                       (token->type == LXB_CSS_SYNTAX_TOKEN_IDENT) ) {
                /* need to use raw token data here as internal string data has been unescaped! */
                lxb_char_t* begin = (lxb_char_t *)lxb_css_syntax_token_base(token)->begin;
                size_t len = lxb_css_syntax_token_base(token)->length;
                dbuf_cat(&actx->tkdata, (char*)begin, len); /* use base raw data to save escapes */
            } else {
                (void) lxb_css_syntax_token_serialize(token, token_cb_f, uctx);
            }
            check_token(token, uctx);
            lxb_css_syntax_parser_consume(parser);
            token = lxb_css_syntax_parser_token(parser);
        }
    }
}

static void comment_info(const lxb_css_syntax_token_t * token, void *uctx)
{
    /* Comment */
    position_info(token, uctx);
    set_tokenstate(TKN_COMMENT, uctx);
    process_single_token(token, uctx);
    push_csstoken(uctx);
}


static bool
css_blank_list_rules_next(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token, void *ctx)
{
    (void)token; (void)ctx;
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_list_rules_end(lxb_css_parser_t *parser,
                         const lxb_css_syntax_token_t *token,
                         void *ctx, bool failed)
{
    (void)parser; (void)token; (void)ctx; (void)failed;
    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_at_rule_t *
css_blank_at_rule_begin(lxb_css_parser_t *parser,
                        const lxb_css_syntax_token_t *token, void *ctx,
                        void **out_rule)
{
    /* At-Rule Begin */
    (void)ctx; (void)out_rule;
    void *uctx = parser->uctx;
    position_info(token, uctx);
    set_tokenstate(TKN_AT_RULE_BEGIN, uctx);
    process_single_token(token, uctx);
    char* td = get_token_data(uctx);
    check_atrule(td, strlen(td), uctx);
    set_curblk(uctx);
    set_attype(uctx);
    /* Do not push_csstoken(); until after At-Rule Prelude End */
    return &lxb_css_blank_at_rule;
}

static bool
css_blank_at_rule_prelude(lxb_css_parser_t *parser,
                          const lxb_css_syntax_token_t *token, void *ctx)
{
    /* At-Rule Prelude Begin */
    (void)ctx;
    void *uctx = parser->uctx;
    css_consule_tokens(parser, token, false, uctx);
    char * td = get_token_data(uctx);
    if (strlen(td) > 0) { 
        append_to_curblk(" ", 1, uctx);
        append_to_curblk(td, strlen(td), uctx);
    }
    set_atargs(uctx);
    /* Do not push_csstoken(); until after At-Rule Prelude End */
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_at_rule_prelude_end(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              void *ctx, bool failed)
{
    /* At-Rule Prelude End */
    (void)ctx; (void)failed;
    void *uctx = parser->uctx;
    set_tokenstate(TKN_AT_RULE_UNKNOWN, uctx);
    process_single_token(token, uctx);
    char* td = get_token_data(uctx);
    char* attype = get_attype(uctx);
    /*  handle statement at rules: such as namespace, charset, import, layer */  
    if (td[0] == ';') {
        if (!strcmp(attype, "@charset"))    set_tokenstate(TKN_CHARSET_AT  , uctx); 
        if (!strcmp(attype, "@import"))     set_tokenstate(TKN_IMPORT_AT   , uctx); 
        if (!strcmp(attype, "@namespace"))  set_tokenstate(TKN_NAMESPACE_AT, uctx); 
        if (!strcmp(attype, "@layer"))      set_tokenstate(TKN_LAYER_AT    , uctx);
        char* atargs = get_atargs(uctx);
        set_token_data(atargs, strlen(atargs), uctx);
    } else {
        set_tokenstate(TKN_AT_RULE_BEGIN, uctx);
        set_token_data(attype, strlen(attype), uctx);
        char* atargs = get_atargs(uctx);
        if (strlen(atargs) > 0) {
            append_to_token_data(" ", 1, uctx);
            append_to_token_data(atargs, strlen(atargs), uctx);
        }
    }
    push_csstoken(uctx);
    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_block_t *
css_blank_at_rule_block_begin(lxb_css_parser_t *parser,
                              const lxb_css_syntax_token_t *token,
                              void *ctx, void **out_rule)
{
    /* At-Rule Block Begin */
    (void)ctx; (void)out_rule;
    void *uctx = parser->uctx;
    position_info(token, uctx);
    set_tokenstate(TKN_AT_BLOCK_BEGIN, uctx);
    process_single_token(token, uctx);
    push_csstoken(uctx);
    push_curblk_to_blkstack(uctx);
    return &lxb_css_blank_block;
}

static bool
css_blank_at_rule_prelude_failed(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 void *ctx)
{
    /* We won't be able to get in here, it's just a formality. */
    (void)token; (void)ctx;
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_at_rule_end(lxb_css_parser_t *parser,
                      const lxb_css_syntax_token_t *token,
                      void *ctx, bool failed)
{
    /* At-Rule End */
    (void)parser; (void)token; (void)ctx; (void)failed;
    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_qualified_rule_t *
css_blank_qualified_rule_begin(lxb_css_parser_t *parser,
                               const lxb_css_syntax_token_t *token,
                               void *ctx, void **out_rule)
{
    /* Qualified Rule Begin */
    (void)parser; (void)token; (void)ctx; (void)out_rule;
    return &lxb_css_blank_qualified_rule;
}

static bool
css_blank_qualified_rule_prelude(lxb_css_parser_t *parser,
                                 const lxb_css_syntax_token_t *token,
                                 void *ctx)
{
    /* Qualified Rule Prelude Begin */
    (void)ctx;
    void *uctx = parser->uctx;
    position_info(token, uctx);
    bool invalid = check_for_invalid_rule(token, uctx);
    if (invalid) {
        set_tokenstate(TKN_INVALID_RULE, uctx);
    } else {
        set_tokenstate(TKN_SELECTOR, uctx);
    }
    css_consule_tokens(parser, token, false, uctx);
    char* td = get_token_data(uctx);
    check_selector(td, strlen(td), uctx);
    push_csstoken(uctx);
    set_curblk(uctx);
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_qualified_rule_prelude_end(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token,
                                     void *ctx, bool failed)
{
    /* Qualified Rule Prelude End */
    (void)parser; (void)token; (void)ctx; (void)failed;
    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_block_t *
css_blank_qualified_rule_block_begin(lxb_css_parser_t *parser,
                                     const lxb_css_syntax_token_t *token,
                                     void *ctx, void **out_rule)
{
    /* Qualified Rule Block Begin */
    (void)ctx; (void)out_rule;
    void *uctx = parser->uctx;
    position_info(token, uctx);
    set_tokenstate(TKN_SEL_BLOCK_BEGIN, uctx);
    process_single_token(token, uctx);
    push_csstoken(uctx);
    push_curblk_to_blkstack(uctx);
    return &lxb_css_blank_block;
}

static bool
css_blank_qualified_rule_prelude_failed(lxb_css_parser_t *parser,
                                        const lxb_css_syntax_token_t *token,
                                        void *ctx)
{
    /* We won't be able to get in here, it's just a formality. */
    (void)token; (void)ctx;
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_qualified_rule_end(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, bool failed)
{
    /* Qualified Rule End */
    (void)parser; (void)token; (void)ctx; (void)failed;
    return LXB_STATUS_OK;
}

static bool
css_blank_block_next(lxb_css_parser_t *parser,
                     const lxb_css_syntax_token_t *token, void *ctx)
{
    /* Block Next" */
    (void)token; (void)ctx;
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_block_end(lxb_css_parser_t *parser,
                    const lxb_css_syntax_token_t *token,
                    void *ctx, bool failed)
{
    /* Block End */
    (void)ctx; (void)failed;
    void *uctx = parser->uctx;
    char * ablock = pop_curblk_from_blkstack(uctx);
    position_info(token, uctx);
    if (ablock[0] == '@') {
        set_tokenstate(TKN_AT_BLOCK_END, uctx);
    } else {
        set_tokenstate(TKN_SEL_BLOCK_END, uctx);
    }
    process_single_token(token, uctx);
    append_to_token_data(" ", 1, uctx);
    append_to_token_data(ablock, strlen(ablock), uctx);
    free(ablock);
    push_csstoken(uctx);
    return LXB_STATUS_OK;
}

static const lxb_css_syntax_cb_declarations_t *
css_blank_declarations_begin(lxb_css_parser_t *parser,
                             const lxb_css_syntax_token_t *token,
                             void *ctx, void **out_rule)
{
    /* Declarations Begin */
    (void)parser; (void)token; (void)ctx; (void)out_rule;
    return &lxb_css_blank_declaration;
}

static lxb_css_parser_state_f
css_blank_declaration_name(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token,
                           void *ctx, void **out_rule)
{
    /* Declaration Name */
    (void)ctx; (void)out_rule;
    void *uctx = parser->uctx;
    set_tokenstate(TKN_PROPERTY, uctx);
    position_info(token, uctx);
    process_single_token(token, uctx);
    push_csstoken(uctx);
    char* td = get_token_data(uctx);
    check_pname(td, strlen(td), uctx);
    return css_blank_declaration_value;
}

static bool
css_blank_declaration_value(lxb_css_parser_t *parser,
                            const lxb_css_syntax_token_t *token, void *ctx)
{
    /* Declaration Value */
    (void)ctx;
    void *uctx = parser->uctx;
    set_tokenstate(TKN_PROPERTY_VALUE, uctx);
    css_consule_tokens(parser, token, true, uctx);
    replace_newlines_in_token_data(uctx);
    char * td = get_token_data(uctx);
    check_value(td, strlen(td), uctx);
    push_csstoken(uctx);
    return lxb_css_parser_success(parser);
}

static lxb_status_t
css_blank_declaration_end(lxb_css_parser_t *parser,
                          void *declarations, void *ctx,
                          const lxb_css_syntax_token_t *token,
                          lxb_css_syntax_declaration_offset_t *offset,
                          bool important, bool failed)
{
    /* Declaration End */
    (void)parser; (void)declarations; (void)token; (void)ctx; (void)failed;
    (void)offset; (void)important; 
    return LXB_STATUS_OK;
}

static lxb_status_t
css_blank_declarations_end(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token,
                           void *ctx, bool failed)
{
    (void)parser; (void)token; (void)ctx; (void)failed; 
    /* Declarations End */
    return LXB_STATUS_OK;
}

static bool
css_blank_declarations_bad(lxb_css_parser_t *parser,
                           const lxb_css_syntax_token_t *token, void *ctx)
{
    (void)token; (void)ctx;
    /* We won't be able to get in here, it's just a formality. */
    return lxb_css_parser_success(parser);
}
