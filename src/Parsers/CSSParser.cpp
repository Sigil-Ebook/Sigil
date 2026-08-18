#include <QString>
#include <QStringList>
#include <QVector>
#include <QTextStream>
#include <QDebug>
#include "Parsers/CSSUtils.h"
#include "Parsers/CSSDeNest.h"
#include "Parsers/CSSParser.h"

// This source buffer is provided by the m_utf8src std::string
// which should always exist unchanged while the parser is active

// Do NOT change or delete m_utf8src once set

CSSParser::CSSParser()
    : m_source(""),
      m_utf8src("")
{
    reset_parser();
    
    // always keep these in sync with csstoken_defn.h
    m_csstoken_type_names.clear();
    m_csstoken_type_names.append("TKN_CHARSET_AT");
    m_csstoken_type_names.append("TKN_IMPORT_AT");
    m_csstoken_type_names.append("TKN_NAMESPACE_AT");
    m_csstoken_type_names.append("TKN_LAYER_AT");
    m_csstoken_type_names.append("TKN_AT_RULE_BEGIN");
    m_csstoken_type_names.append("TKN_AT_BLOCK_BEGIN");
    m_csstoken_type_names.append("TKN_AT_BLOCK_END");
    m_csstoken_type_names.append("TKN_AT_RULE_UNKNOWN");
    m_csstoken_type_names.append("TKN_SELECTOR");
    m_csstoken_type_names.append("TKN_INVALID_RULE");
    m_csstoken_type_names.append("TKN_SEL_BLOCK_BEGIN");
    m_csstoken_type_names.append("TKN_SEL_BLOCK_END");
    m_csstoken_type_names.append("TKN_PROPERTY");
    m_csstoken_type_names.append("TKN_PROPERTY_VALUE");
    m_csstoken_type_names.append("TKN_COMMENT");
    m_csstoken_type_names.append("TKN_CSS_END");

    // Used for serializing parsed css (multiline format)
    csstemplateM.push_back("  ");      //  0 - standard indentation
    csstemplateM.push_back(" {\n");    //  1 - bracket after @-rule
    csstemplateM.push_back("");        //  2 - space after "," in selector
    csstemplateM.push_back(" {\n");    //  3 - bracket after selector was "\n{\n"
    csstemplateM.push_back("");        //  4 - unused
    csstemplateM.push_back(" ");       //  5 - string after property before value
    csstemplateM.push_back(";\n");     //  6 - string after value
    csstemplateM.push_back("}\n");     //  7 - closing bracket - selector
    csstemplateM.push_back("\n");      //  8 - space between blocks {...}
    csstemplateM.push_back("}\n\n");   //  9 - closing bracket @-rule
    csstemplateM.push_back("");        // 10 - unused
    csstemplateM.push_back("");        // 11 - before comment
    csstemplateM.push_back("\n");      // 12 - after comment
    csstemplateM.push_back("");        // 13 - after last line @-rule

    // Used for serializing parsed css (single line format)
    csstemplate1.push_back("");        //  0 - standard indentation
    csstemplate1.push_back("{");       //  1 - bracket after @-rule
    csstemplate1.push_back("");        //  2 - space after "," in selector
    csstemplate1.push_back("{");       //  3 - bracket after selector was "\n{\n"
    csstemplate1.push_back("");        //  4 - unused
    csstemplate1.push_back("");        //  5 - string after property before value
    csstemplate1.push_back(";");       //  6 - string after value
    csstemplate1.push_back("}");       //  7 - closing bracket - selector
    csstemplate1.push_back("\n");      //  8 - space between blocks {...}
    csstemplate1.push_back("}\n");     //  9 - closing bracket @-rule
    csstemplate1.push_back("");        // 10 - unused
    csstemplate1.push_back("");        // 11 - before comment
    csstemplate1.push_back("\n");      // 12 - after comment
    csstemplate1.push_back("");        // 13 - after last line @-rule
}

CSSParser::~CSSParser()
{
    m_utf8src = "";
}

void CSSParser::parse_css(const QString &source)
{
    reset_parser();
    m_source = source;
    QString nsource = source;
    nsource.replace("\r\n","\n");
    nsource += "\n";
    m_utf8src = nsource.toStdString();
    // m_utf8src = CSSDeNest::denest_css(nsource.toStdString());
    if (m_utf8src.empty()) return;

    parse_css_structure_with_user_context(m_utf8src.data(), m_utf8src.length(), 
                                          ((void*) (this)),
                                          &CSSParser::add_csstoken,
                                          &CSSParser::add_error);

    // parsing complete
    // add an "end" token to complete the csstokens list
    csstoken atok;
    atok.type = TKN_CSS_END;
    atok.pos = 0;
    atok.line = 0;
    atok.col = 0;
    atok.data = QString("EOF");
    m_csstokens.append(atok);
        
#if 0
    // validate csstokens and error messages
    foreach(csstoken atk, m_csstokens) {
        size_t idx = (size_t) atk.type;
        QString name = m_csstoken_type_names.at(idx);
        qDebug() << atk.line << atk.col << atk.pos << name << atk.data;
    }
#endif

#if 0
    QString output = serialize_css(true, true);

    QTextStream out(stdout, QIODevice::WriteOnly);
    foreach(QString anerror, m_errors) {
        out << anerror << "\n";
    }
#endif    
}

void CSSParser::add_csstoken_to_csstokens(csstoken_type st, size_t pos,
                                              size_t line, size_t col, const char* data)
{
    csstoken atok;
    atok.type = st;
    atok.pos = pos;
    atok.line = line;
    atok.col = col;
    atok.data = QString::fromUtf8(data);
    m_csstokens.append(atok);
    if (atok.type == TKN_CHARSET_AT) m_charset = atok.data;
    if (atok.type == TKN_IMPORT_AT) m_imports.append(atok.data);
    if (atok.type == TKN_NAMESPACE_AT) m_namesp = atok.data;
    if (atok.type == TKN_LAYER_AT) m_layer = atok.data;
}

void CSSParser::add_error_to_errors(const char* msg)
{
    QString emsg = QString::fromUtf8(msg);
    m_errors.append(emsg);
}


// static method
void CSSParser::add_csstoken(csstoken_type st, size_t pos, size_t line,
                                  size_t col, const char* data, void* context)
{
    CSSParser* instance = static_cast<CSSParser*>(context);
    instance->add_csstoken_to_csstokens(st, pos, line, col, data);
}

// static method
void CSSParser::add_error(const char* data, void* context)
{
    CSSParser* instance = static_cast<CSSParser*>(context);
    instance->add_error_to_errors(data);
}


// --------------------------------------------


int CSSParser::_seeknocomment(const int key, int move)
{
    int go = (move > 0) ? 1 : -1;
    for (int i = key + 1; abs(key-i)-1 < abs(move); i += go)
    {
        if (i < 0 || i >= m_csstokens.size()) {
            return -1;
        }
        if (m_csstokens[i].type == TKN_COMMENT) {
            move += 1;
            continue;
        }
        return m_csstokens[i].type;
    }
    return -1;
}


QString CSSParser::serialize_css(bool tostdout, bool multiline)
{
    QString output_string;
    QTextStream output(&output_string);

    int lvl = 0;
    QString indent = "";

    QVector<QString> csstemplate;

    if (multiline) {
        csstemplate = csstemplateM;
    } else {
        csstemplate = csstemplate1;
    }

    bool tail_comment = false;
    for (size_t i = 0; i < (size_t) m_csstokens.size(); ++i)
    {
        switch (m_csstokens[i].type)
        {
            case TKN_CHARSET_AT:
                output << "@charset " << m_csstokens[i].data << csstemplate[6];
                break;

            case TKN_IMPORT_AT:
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent << "@import " << m_csstokens[i].data << csstemplate[6];
                break;

            case TKN_NAMESPACE_AT:
                output << "@namespace " << m_csstokens[i].data << csstemplate[6];
                break;

            case TKN_LAYER_AT:
                output << "@layer " << m_csstokens[i].data << csstemplate[6];
                break;

            case TKN_AT_RULE_BEGIN:
            case TKN_AT_RULE_UNKNOWN:
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent << m_csstokens[i].data << csstemplate[1];
                break;

            case TKN_SELECTOR:
            case TKN_INVALID_RULE:
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent
                       << CSSUtils::implode("," + csstemplate[2], CSSUtils::explode(",",
                          m_csstokens[i].data, false)) << csstemplate[3];
                break;

            case TKN_PROPERTY:
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent << m_csstokens[i].data << ":" << csstemplate[5];
                break;

            case TKN_PROPERTY_VALUE:
                /* allow for tail comments after property values */
                if ((m_csstokens[i+1].type == TKN_COMMENT) && (m_csstokens[i].line == m_csstokens[i+1].line)) {
                    tail_comment = true;
                    output << m_csstokens[i].data << ";";
                } else {
                    output << m_csstokens[i].data << csstemplate[6];
                }
                break;

            case TKN_SEL_BLOCK_BEGIN:
            case TKN_AT_BLOCK_BEGIN:
                lvl++;
                break;

            case TKN_SEL_BLOCK_END:
                lvl--; if (lvl < 0) lvl = 0;
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent << csstemplate[7];
                if (multiline) {
                    if (lvl == 0) output << csstemplate[8];
#if 0                    
                    csstoken_type next_token = (csstoken_type) _seeknocomment(i, 1);
                    if ((next_token != TKN_AT_BLOCK_END) && (next_token != TKN_SEL_BLOCK_END)) output << csstemplate[8];
#endif
                } else {
                    if (lvl == 0) {
                        output << csstemplate[8];
                    }
                }
                break;

            case TKN_AT_BLOCK_END:
                lvl--; if (lvl < 0) lvl = 0;
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent << csstemplate[9];
                break;

            case TKN_COMMENT:
                if (multiline || (lvl == 0)) {
                    if (tail_comment) {
                        indent = " ";
                    } else {
                        indent = CSSUtils::indent(lvl, csstemplate[0]);
                    }
                    output << indent << csstemplate[11] << m_csstokens[i].data << csstemplate[12];
                } else {
                    output << csstemplate[11] << m_csstokens[i].data;
                }
                tail_comment = false;
                break;

            case TKN_CSS_END:
                break;
        }
    }

    output_string = CSSUtils::trim(output_string);

    if(tostdout)
    {
        QTextStream out(stdout, QIODevice::WriteOnly);
        out << output_string << "\n";
    }
    return output_string;
}


void CSSParser::reset_parser()
{
    m_token_ptr = 0;
    m_charset = "";
    m_namesp = "";
    m_layer = "";
    m_imports.clear();
    m_csstokens.clear();
    m_errors.clear();
    m_utf8src = "";
}

CSSParser::csstoken CSSParser::get_next_token(int start_ptr)
{
    if ((start_ptr >= 0) && (start_ptr < m_csstokens.size()))
    {
        m_token_ptr = start_ptr;
    }

    csstoken atoken;
    atoken.type = TKN_CSS_END;
    atoken.data = "";

    if (m_token_ptr < m_csstokens.size())
    {
        atoken = m_csstokens[m_token_ptr];
        m_token_ptr++;
    }
    return atoken;
}

QString CSSParser::get_type_name(csstoken_type t)
{
    return m_csstoken_type_names[t];
}


// We want to split a possible group selector on commas.
// The issue is that attribute values may legally contain commas
// and selector functions (ie.  contains()) may legally contain commas.
// To make it worse, attribute values can contain [,],(,) as well
// in unmatched sets in any order as they may be in quoted strings.
// This is true for selector functions as well.
// So we need to keep track of brackets and parens.
// And we need to ignore any spurious [, ], (, ), ', or " in any quoted strings.
// Luckily AFAIK  no nesting of [] or () allowed (yet) in the selector.
QStringList CSSParser::splitGroupSelector(const QString& sel)
{
    QStringList res;
    int pos = 0;
    bool insquote = false;
    bool indquote = false;
    bool inbracket = false;
    bool inparen = false;
    for (int i = 0; i < sel.length(); i++)
    {
        QChar c = sel.at(i);
        // keep track of current state
        if (c == '[' && !insquote && !indquote)  inbracket = true;
        if (c == ']' && !insquote && !indquote)  inbracket = false;

        if (c == '(' && !insquote && !indquote)  inparen = true;
        if (c == ')' && !insquote && !indquote)  inparen = false;

        if (c == '\'' && insquote && !indquote)  insquote = false;
        if (c == '\'' && !insquote && !indquote) insquote = true;

        if (c == '"' && !insquote && !indquote)  indquote = true;
        if (c == '"' && !insquote && indquote)   indquote = false;

        if (c == ',' && !inbracket && !inparen) 
        {
            // found split point
            res << sel.mid(pos, i-pos).trimmed();
            pos = i + 1;
        }
        else if (i == sel.length() - 1)
        {
            // we reached the end of the selector
            res << sel.mid(pos, sel.length()-pos).trimmed();
            pos = sel.length();
        }
    }
    return res;
}


// allow a new set of csstokens to be loaded and then serialized
void CSSParser::set_csstokens(const QVector<csstoken> &ntokens)
{
    m_csstokens.clear();
    for(size_t i = 0; i < (size_t)ntokens.size(); i++)
    {
        csstoken atemp = ntokens[i];
        m_csstokens.push_back(atemp);
    }
}


// We want to split a selector on periods to find class selectors
// The issue is that attribute values may legally contain periods
// and selector functions (ie.  contains()) may legally contain periods.
// To make it worse, attribute values can contain [,],(,) as well
// in unmatched sets in any order as they may be in quoted strings.
// This is true for selector functions as well.
// So we need to keep track of brackets and parens.
// And we need to ignore any spurious [, ], (, ), ', or " in any quoted strings.
// Luckily AFAIK  no nesting of [] or () allowed (yet) in the selector.
std::pair<int, QString> CSSParser::findNextClassInSelector(const QString &sel, int p)
{
    std::pair<int, QString> res;
    res.first = -1;
    res.second = QString();
    bool insquote = false;
    bool indquote = false;
    bool inbracket = false;
    for (int i = p; i < sel.length(); i++)
    {
        QChar c = sel.at(i);
        // keep track of current state
        if (c == '[' && !insquote && !indquote)  inbracket = true;
        if (c == ']' && !insquote && !indquote)  inbracket = false;

        if (c == '\'' && insquote && !indquote)  insquote = false;
        if (c == '\'' && !insquote && !indquote) insquote = true;

        if (c == '"' && !insquote && !indquote)  indquote = true;
        if (c == '"' && !insquote && indquote)   indquote = false;

        if (c == '.' && !inbracket  & !indquote & !insquote) {
            // found a class name start
            res.first = i;
            QString cname;
            bool in_escape = false;
            // now walk forward to find next delimiter (skipping the ".")
            for (int j=i+1; j < sel.length(); j++) {
                 QChar d = sel.at(j);
                 if ((d == '\\') && !in_escape) {
                     in_escape = true;
                     cname.append(d);
                     continue;
                 }
                 if (in_escape && !CSSUtils::ctype_xdigit(d)) {
                     in_escape = false;
                     cname.append(d);
                     continue;
                 }
                 if ( CSSUtils::ctype_alpha(d) || CSSUtils::ctype_digit(d) ||
                      d.unicode() >= 160 || d == '-' || d == '_' ) {
                      cname.append(d);
                 } else {
                      break;
                 }

            }
            res.second = cname;
            return res;
        }
    }
    return res;
}
