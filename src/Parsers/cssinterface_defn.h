#ifndef CSSTOKEN_DEFN_H
#define CSSTOKEN_DEFN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
        TKN_CHARSET_AT = 0 ,
        TKN_IMPORT_AT      , /* 1 */
        TKN_NAMESPACE_AT   , /* 2 */
        TKN_LAYER_AT       , /* 3 */
        TKN_AT_RULE_BEGIN  , /* 4 */
        TKN_AT_BLOCK_BEGIN , /* 5 */
        TKN_AT_BLOCK_END   , /* 6 */
        TKN_AT_RULE_UNKNOWN, /* 7 */
        TKN_SELECTOR       , /* 8 */
        TKN_INVALID_RULE   , /* 9 */
        TKN_SEL_BLOCK_BEGIN, /* 10 */
        TKN_SEL_BLOCK_END  , /* 11 */
        TKN_PROPERTY       , /* 12 */
        TKN_PROPERTY_VALUE , /* 13 */
        TKN_COMMENT        , /* 14 */
        TKN_CSS_END          /* 15 */
    } csstoken_type;

typedef void (*sigil_add_csstoken_callback_f)(csstoken_type st, size_t pos, size_t line, size_t col, const char* data, void* context);

typedef void (*sigil_add_error_callback_f)(const char* emessage, void* context);

extern unsigned int parse_css_structure_with_user_context(const char* data, size_t len,  void* cpp_this, 
                                                   sigil_add_csstoken_callback_f add_csstoken_method, 
                                                   sigil_add_error_callback_f add_error_method);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CSSTOKEN_DEFN_H */
