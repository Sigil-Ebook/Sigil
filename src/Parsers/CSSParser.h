#ifndef CSSPARSER_H
#define CSSPARSER_H

#include <stdlib.h>
#include <string>

#include <QString>
#include <QList>
#include <QVector>

#include "Parsers/cssinterface_defn.h"


class CSSParser
{
public:

    struct csstoken
    {
        csstoken_type type;
        size_t pos;
        size_t line;
        size_t col;
        QString data;
    };

    CSSParser();
    ~CSSParser();

    void reset_parser();

    void parse_css(const QString &source);

    // serialize the current list of csstokens back to css
    QString serialize_css(bool tostdout = true, bool multiline = true);

    // access charset, namespace, imports, and layer without having to walk csstokens
    QString          get_charset()   { return m_charset; };
    QVector<QString> get_imports()   { return m_imports; };
    QString          get_namespace() { return m_namesp;  };
    QString          get_layer()     { return m_layer;   };

    // walk the csstokens list, token by token
    // set start_ptr to the position you woud like to start at in the list
    // leaving it as -1 will simply start at 0 and increment
    // last token is a dummy token with type set to CSS_END 
    csstoken get_next_token(int start_ptr = -1);

    // covert token type enum value to a descriptive string
    QString get_type_name(csstoken_type t);

    // this routine allows external modifications to the csstokens
    // to be brought back in to serialize them with serialize_css
    void set_csstokens(const QVector<csstoken> &ntokens);

    // get errors, warnings, and information
    QVector<QString> get_parse_errors() { return m_errors; };

    // QVector<QString> get_denest_errors();

    // instance routine invoked from static callback (add_csstoken)    
    void add_csstoken_to_csstokens(csstoken_type st, size_t pos, size_t Line, size_t col, const char* data);

    // instance routine invoked from static callback (add_error)   
    void add_error_to_errors(const char* data);
    
     // callbacks from lxbcss interface C code
    static void add_csstoken(csstoken_type t, size_t pos, size_t line, size_t col, 
                             const char* data, void* context);

     // callbacks from lxbcss interface C code
    static void add_error(const char* emessage, void* context);

    static QStringList splitGroupSelector(const QString &sel);

    static std::pair<int, QString> findNextClassInSelector(const QString &sel, int p = 0);


private:

    int _seeknocomment(const int key, int move);

    QString           m_source;
    std::string       m_utf8src;
    QVector<QString>  m_csstoken_type_names;
    QVector<csstoken> m_csstokens;
    QVector<QString>  m_errors;
    QVector<QString>  csstemplateM;
    QVector<QString>  csstemplate1;
    QString           m_charset;
    QString           m_namesp;
    QVector<QString>  m_imports;
    QString           m_layer;
    int               m_token_ptr;
};

#endif
