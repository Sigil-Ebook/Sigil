/************************************************************************
 **
 **  Copyright (C) 2021-2022 Kevin B. Hendricks, Stratford, Ontario, Canada
 **
 **  This file is part of Sigil.
 **
 **  Sigil is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Sigil is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
 **
 ** Extracted and modified from:
 ** CSSTidy (https://github.com/csstidy-c/csstidy)
 **
 ** CSSTidy Portions Copyright:
 **   Florian Schmitz <floele@gmail.com>
 **   Thierry Charbonnel
 **   Will Mitchell <aethon@gmail.com>
 **   Brett Zamir <brettz9@yahoo.com>
 **   sined_ <sined_@users.sourceforge.net>
 **   Dmitry Leskov <git@dmitryleskov.com>
 **   Kevin Coyner <kcoyner@debian.org>
 **   Tuukka Pasanen <pasanen.tuukka@gmail.com>
 **   Frank W. Bergmann <csstidy-c@tuxad.com>
 **   Frank Dana <ferdnyc@gmail.com>
 **
 ** CSSTidy us Available under the LGPL 2.1
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include <QChar>
#include <QString>
#include <QTextStream>
#include "Parsers/qCSSProperties.h"
#include "Parsers/qCSSUtils.h"
#include "Parsers/qCSSParser.h"

/* PIS = in selector
 * PIP = in property
 * PIV = in value
 * PINSTR = in string (-> ",',( => ignore } and ; etc.)
 * PIC = in comment (ignore everything)
 * PAT = in @-block
 */

CSSParser::CSSParser()
{ 
    tokens = "{};:()@='\"/,\\!$%&*+.<>?[]^`|~";

    // Used for serializing parsed css (multiline format)
    csstemplateM.push_back("  ");      //  0 - standard indentation
    csstemplateM.push_back(" {\n");    //  1 - bracket after @-rule
    csstemplateM.push_back("");        //  2 - unused
    csstemplateM.push_back(" {\n");    //  3 - bracket after selector was "\n{\n"
    csstemplateM.push_back("");        //  4 - unused
    csstemplateM.push_back(" ");       //  5 - string after property before value
    csstemplateM.push_back(";\n");     //  6 - string after value
    csstemplateM.push_back("}");       //  7 - closing bracket - selector
    csstemplateM.push_back("\n\n");    //  8 - space between blocks {...}
    csstemplateM.push_back("}\n\n");   //  9 - closing bracket @-rule
    csstemplateM.push_back("");        // 10 - unused
    csstemplateM.push_back("");        // 11 - before comment
    csstemplateM.push_back("\n");      // 12 - after comment
    csstemplateM.push_back("\n");      // 13 - after last line @-rule

    // Used for serializing parsed css (single line format)
    csstemplate1.push_back("");        //  0 - standard indentation
    csstemplate1.push_back("{");       //  1 - bracket after @-rule
    csstemplate1.push_back("");        //  2 - unused
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


    // at_rule to parser state map
    at_rules["page"] = PIS;
    at_rules["font-face"] = PIS;
    at_rules["charset"] = PIV;
    at_rules["import"] = PIV;
    at_rules["namespace"] = PIV;
    at_rules["media"] = PAT;
    at_rules["keyframes"] = PAT;
    at_rules["supports"] = PAT;
    at_rules["-moz-keyframes"] = PAT;
    at_rules["-ms-keyframes"] = PAT;
    at_rules["-o-keyframes"] = PAT;
    at_rules["-webkit-keyframes"] = PAT;

    // descriptive names for each token type
    token_type_names.push_back("CHARSET");
    token_type_names.push_back("IMPORT");
    token_type_names.push_back("NAMESP");
    token_type_names.push_back("AT_START");
    token_type_names.push_back("AT_END");
    token_type_names.push_back("SEL_START");
    token_type_names.push_back("SEL_END");
    token_type_names.push_back("PROPERTY");
    token_type_names.push_back("VALUE");
    token_type_names.push_back("COMMENT");
    token_type_names.push_back("CSS_END");

    css_level = "CSS3.0";
} 


void CSSParser::set_level(QString level)
{
    if ((level == "CSS1.0") || (level == "CSS2.0") ||
        (level == "CSS2.1") || (level == "CSS3.0"))
    {
        css_level = level;
    }
}


void CSSParser::reset_parser()
{
    token_ptr = 0;
    charset = "";
    namesp = "";
    line = 1;
    selector_nest_level = 0;
    import.clear();
    csstokens.clear();
    cur_selector.clear();
    cur_at.clear();
    cur_property.clear();
    cur_function.clear();
    cur_sub_value.clear();
    cur_value.clear();
    cur_string.clear();
    cur_selector.clear();
    sel_separate.clear();
}


QString CSSParser::get_charset()
{
    return charset;
}

QString CSSParser::get_namespace()
{
    return namesp;
}

QVector<QString> CSSParser::get_import()
{
    return import;
}


CSSParser::token CSSParser::get_next_token(int start_ptr)
{
    if ((start_ptr >= 0) && (start_ptr < csstokens.size()))
    {
        token_ptr = start_ptr;
    }

    token atoken;
    atoken.type = CSS_END;
    atoken.data = "";

    if (token_ptr < csstokens.size())
    {
        atoken = csstokens[token_ptr];
        token_ptr++;
    }
    return atoken;
}


QString CSSParser::get_type_name(CSSParser::token_type t)
{
    return token_type_names[t];
}


void CSSParser::add_token(const token_type ttype, const QString data)
{
    token temp;
    temp.type = ttype;
    temp.pos = spos;
    temp.line = sline;
    temp.data = (ttype == COMMENT) ? data : CSSUtils::trim(data);
    csstokens.push_back(temp);
    if (ttype == SEL_START) selector_nest_level++;
    if (ttype == SEL_END) selector_nest_level--;
}

void CSSParser::log(const QString msg, const message_type type, int iline)
{
    message new_msg;
    new_msg.m = msg;
    new_msg.t = type;
    if(iline == 0)
    {
        iline = line;
    }
    if(logs.count(line) > 0)
    {
        for(int i = 0; i < logs[line].size(); ++i)
        {
            if(logs[line][i].m == new_msg.m && logs[line][i].t == new_msg.t)
            {
                return;
            }
        }
    }
    logs[line].push_back(new_msg);
}

QString CSSParser::unicode(QString& istring, int& i)
{
    ++i;
    QString add = "";
    bool replaced = false;
    
    while(i < istring.length() && (CSSUtils::ctype_xdigit(istring[i]) ||
                                   CSSUtils::ctype_space(istring[i])) && add.length()< 6)
    {
        add += istring[i];

        if(CSSUtils::ctype_space(istring[i])) {
            break;
        }
        i++;
    }

    if ((CSSUtils::hexdec(add) > 47 && CSSUtils::hexdec(add) < 58) ||
        (CSSUtils::hexdec(add) > 64 && CSSUtils::hexdec(add) < 91) ||
        (CSSUtils::hexdec(add) > 96 && CSSUtils::hexdec(add) < 123))
    {
        QString msg = "Replaced unicode notation: Changed \\" + CSSUtils::rtrim(add) + " to ";
        add = static_cast<int>(CSSUtils::hexdec(add));
        msg += add;
        log(msg,Information);
        replaced = true;
    } else {
        add = CSSUtils::trim("\\" + add);
    }

    if ((CSSUtils::ctype_xdigit(istring[i+1]) && CSSUtils::ctype_space(istring[i])
         && !replaced) || !CSSUtils::ctype_space(istring[i]))
    {
        i--;
    }
    
    if (1) {
        return add;
    }
    return "";
}


bool CSSParser::is_token(QString& istring, const int i)
{
    return (tokens.contains(istring[i]) && !CSSUtils::escaped(istring,i));
}


void CSSParser::explode_selectors()
{
    sel_separate = QVector<int>();
}


int CSSParser::_seeknocomment(const int key, int move)
{
    int go = (move > 0) ? 1 : -1;
    for (int i = key + 1; abs(key-i)-1 < abs(move); i += go)
    {
        if (i < 0 || i >= csstokens.size()) {
            return -1;
        }
        if (csstokens[i].type == COMMENT) {
            move += 1;
            continue;
        }
        return csstokens[i].type;
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

    for (int i = 0; i < csstokens.size(); ++i)
    {
        switch (csstokens[i].type)
        {
            case CHARSET:
                output << "@charset " << csstemplate[5] << csstokens[i].data << csstemplate[6];
                break;

            case IMPORT:
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent << "@import " << csstemplate[5] << csstokens[i].data << csstemplate[6];
                break;

            case NAMESP:
                output << "@namespace " << csstemplate[5] << csstokens[i].data << csstemplate[6];
                break;

            case AT_START:
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent << csstokens[i].data << csstemplate[1];
                lvl++;
                break;

            case SEL_START:
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent << csstokens[i].data << csstemplate[3];
                lvl++;
                break;

            case PROPERTY:
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent << csstokens[i].data << ":" << csstemplate[5];
                break;

            case VALUE:
                output << csstokens[i].data << csstemplate[6];
                break;

            case SEL_END:
                lvl--; if (lvl < 0) lvl = 0;
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << indent << csstemplate[7];
                if (multiline) {
                    if(_seeknocomment(i, 1) != AT_END) output << csstemplate[8];
                } else {
                    if (lvl == 0) {
                        output << csstemplate[8];
                    }
                }
                break;

            case AT_END:
                lvl--; if (lvl < 0) lvl = 0;
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << csstemplate[13] << indent << csstemplate[9];
                break;

            case COMMENT:
                if (multiline || (lvl == 0)) {
                    output << csstemplate[11] <<  "/*" << csstokens[i].data << "*/" << csstemplate[12];
                } else {
                    output << csstemplate[11] <<  "/*" << csstokens[i].data << "*/";
                }
                break;

            case CSS_END:
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


void CSSParser::parseInAtBlock(QString& css_input, int& i, parse_status& astatus, parse_status& afrom)
{
    if(is_token(css_input,i))
    {
        if(css_input[i] == '/' && CSSUtils::s_at(css_input,i+1) == '*')
        {
            astatus = PIC; ++i;
            afrom = PAT;
        }
        else if(css_input[i] == '{')
        {
            astatus = PIS;
            add_token(AT_START, cur_at);
        }
        else if(css_input[i] == ',')
        {
            cur_at = CSSUtils::trim(cur_at) + ",";
        }
        else if(css_input[i] == '\\')
        {
            cur_at += unicode(css_input,i);
        }
        else /*if((css_input[i] == '(') || (css_input[i] == ':') || (css_input[i] == ')') || (css_input[i] == '.'))*/
        {
            if(!QString("():/.").contains(css_input[i]))
            {
                // Strictly speaking, these are only permitted in @media rules 
                log("Unexpected symbol '" + css_input[i] + "' in @-rule", Warning);
            }
            cur_at += css_input[i];  /* append tokens after media selector */
        }
    }
    else
    {
        // Skip excess whitespace
        int lastpos = cur_at.length()-1;
        if(lastpos == -1 || !( (CSSUtils::ctype_space(cur_at[lastpos]) ||
                               (is_token(cur_at,lastpos) && cur_at[lastpos] == ',')) && 
                               CSSUtils::ctype_space(css_input[i])))
        {
            cur_at += css_input[i];
        }
    }
}


void CSSParser::parseInSelector(QString& css_input, int& i, parse_status& astatus, parse_status& afrom,
                                bool& invalid_at, QChar& str_char, int str_size)
{
    if(is_token(css_input,i))
    {
        if(css_input[i] == '/' && CSSUtils::s_at(css_input,i+1) == '*')
        {
            astatus = PIC; ++i;
            afrom = PIS;
        }
        else if(css_input[i] == '@' && CSSUtils::trim(cur_selector) == "")
        {
            // Check for at-rule
            invalid_at = true;
            for(QMap<QString,parse_status>::iterator j = at_rules.begin(); j != at_rules.end(); ++j )
            {
                if(CSSUtils::strtolower(css_input.mid(i+1,j.key().length())) == j.key())
                {
                    (j.value() == PAT) ? cur_at = "@" + j.key() : cur_selector = "@" + j.key();
                    astatus = j.value();
                    i += j.key().length();
                    invalid_at = false;
                }
            }
            if (invalid_at)
            {
                cur_selector = "@";
                QString invalid_at_name = "";
                for(int j = i+1; j < str_size; ++j)
                {
                    if(!CSSUtils::ctype_alpha(css_input[j]))
                    {
                        return;
                    }
                    invalid_at_name += css_input[j];
                }
                log("Invalid @-rule: " + invalid_at_name + " (removed)",Warning);
            }
        }
        else if(css_input[i] == '"' || css_input[i] == '\'')
        {
            cur_string = css_input[i];
            astatus = PINSTR;
            str_char = css_input[i];
            afrom = PIS;
        }
        else if(invalid_at && css_input[i] == ';')
        {
            invalid_at = false;
            astatus = PIS;
        }
        else if (css_input[i] == '{')
        {
            astatus = PIP;
            add_token(SEL_START, cur_selector);
        }
        else if (css_input[i] == '}')
        {
            add_token(AT_END, cur_at);
            cur_at = "";
            cur_selector = "";
            sel_separate = QVector<int>();
        }
        else if(css_input[i] == ',')
        {
            cur_selector = CSSUtils::trim(cur_selector) + ",";
            sel_separate.push_back(cur_selector.length());
        }
        else if (css_input[i] == '\\')
        {
            cur_selector += unicode(css_input,i);
        }
        // remove unnecessary universal selector,  FS#147
        else if(!(css_input[i] == '*' && (CSSUtils::s_at(css_input,i+1) == '.' ||
                                          CSSUtils::s_at(css_input,i+1) == '[' ||
                                          CSSUtils::s_at(css_input,i+1) == ':' ||
                                          CSSUtils::s_at(css_input,i+1) == '#')))
        {
            cur_selector += css_input[i];
        }
    }
    else
    {
        int lastpos = cur_selector.length()-1;
        if( (lastpos == -1) || !( (CSSUtils::ctype_space(cur_selector[lastpos]) ||
                                   (is_token(cur_selector,lastpos) && cur_selector[lastpos] == ',')) &&
                                  CSSUtils::ctype_space(css_input[i])))
        {
            cur_selector += css_input[i];
        }
    }
}


void CSSParser::parseInProperty(QString& css_input, int& i, parse_status& astatus, parse_status& afrom,
                                bool& invalid_at)
{

    if (is_token(css_input,i))
    {
        if (css_input[i] == ':' || (css_input[i] == '=' && cur_property != ""))
        {
            astatus = PIV;
            bool valid = true || (CSSProperties::instance()->contains(cur_property) && 
                                  CSSProperties::instance()->levels(cur_property).indexOf(css_level,0) != -1);
            if(valid) {
                add_token(PROPERTY, cur_property);
            }
        }
        else if(css_input[i] == '/' && CSSUtils::s_at(css_input,i+1) == '*' && cur_property == "")
        {
            astatus = PIC; ++i;
            afrom = PIP;
        }
        else if(css_input[i] == '}')
        {
            explode_selectors();
            astatus = PIS;
            invalid_at = false;
            add_token(SEL_END, cur_selector);
            cur_selector = "";
            cur_property = "";
        }
        else if(css_input[i] == ';')
        {
            cur_property = "";
        }
        else if(css_input[i] == '\\')
        {
            cur_property += unicode(css_input,i);
        }
        else if(css_input[i] == '*') 
        {
            // IE7 and below recognize properties that begin with '*'
            if (cur_property == "")
            {
                cur_property += css_input[i];
                log("IE7- hack detected: property name begins with '*'", Warning);
            } 
        }
        else
        {
            log("Unexpected character '" + QString(1, css_input[i]) + "'in property name", Error);
        }
    }
    else if(!CSSUtils::ctype_space(css_input[i]))
    {
        if(css_input[i] == '_' && cur_property == "")
        {
            // IE6 and below recognize properties that begin with '_'
            log("IE6 hack detected: property name begins with '_'", Warning);
        }
        // TODO: Check for invalid characters
        cur_property += css_input[i];
    }
    // TODO: Check for whitespace inside property names
}


void CSSParser::parseInValue(QString& css_input, int& i, parse_status& astatus, parse_status& afrom,
                             bool& invalid_at, QChar& str_char, bool& pn, int str_size)
{
    pn = (((css_input[i] == '\n' || css_input[i] == '\r') && property_is_next(css_input,i+1)) || i == str_size-1);
    if(pn)
    {
        log("Added semicolon to the end of declaration",Warning);
    }
    if(is_token(css_input,i) || pn)
    {
        if((css_input[i] == '{') && (cur_selector == "@import" ||
                                     cur_selector == "@charset" ||
                                     cur_selector == "@namespace"))
        {
            log("Unexpected character '" + QString(1, css_input[i]) + "' in " + cur_selector, Error);
        }
        if(css_input[i] == '/' && CSSUtils::s_at(css_input,i+1) == '*')
        {
            astatus = PIC; ++i;
            afrom = PIV;
        }
        else if(css_input[i] == '"' || css_input[i] == '\'' ||
                (css_input[i] == '(' && cur_sub_value == "url") )
        {
            str_char = (css_input[i] == QChar('(')) ? QChar(')') : css_input[i];
            cur_string = css_input[i];
            astatus = PINSTR;
            afrom = PIV;
        }
        else if(css_input[i] == '(')
        {
            // function call or an open parenthesis in a calc() expression
            // url() is a special case that should have been handled above
            assert(cur_sub_value != "url");
                    
            // cur_sub_value should contain the name of the function, if any
            cur_sub_value = CSSUtils::trim(cur_sub_value + "(");
            // set current function name and push it onto the stack
            cur_function = cur_sub_value;
            cur_function_arr.push_back(cur_sub_value);
            cur_sub_value_arr.push_back(cur_sub_value);
            cur_sub_value = "";
        }
        else if(css_input[i] == '\\')
        {
            cur_sub_value += unicode(css_input,i);
        }
        else if(css_input[i] == ';' || pn)
        {
            if(cur_selector.mid(0,1) == "@" && 
               at_rules.count(cur_selector.mid(1)) > 0 &&
               at_rules[cur_selector.mid(1)] == PIV)
            {
                cur_sub_value_arr.push_back(CSSUtils::trim(cur_sub_value));
                astatus = PIS;

                if(cur_selector == "@charset")
                {
                     charset = cur_sub_value_arr[0];
                     add_token(CHARSET, charset);
                }
                else if(cur_selector == "@import")
                {
                     QString aimport = CSSUtils::build_value(cur_sub_value_arr);
                     add_token(IMPORT, aimport);
                     import.push_back(aimport);
                }
                else if(cur_selector == "@namespace")
                {
                    namesp = CSSUtils::implode(" ",cur_sub_value_arr);
                    add_token(NAMESP, namesp);
                }
                cur_sub_value_arr.clear();
                cur_sub_value = "";
                cur_selector = "";
                sel_separate = QVector<int>();
            }
            else
            {
                astatus = PIP;
            }
        }
        else if (css_input[i] == '!') 
        {
            cur_sub_value_arr.push_back(CSSUtils::trim(cur_sub_value));
            cur_sub_value = "!";
        }
        else if (css_input[i] == ',' || css_input[i] == ')') 
        {
            // store the current subvalue, if any
            cur_sub_value = CSSUtils::trim(cur_sub_value);
            if(cur_sub_value != "")
            {
                cur_sub_value_arr.push_back(cur_sub_value);
                cur_sub_value = "";
            }
            bool drop = false;
            if (css_input[i] == ')')
            {
                if (cur_function_arr.isEmpty())
                {
                    // No matching open parenthesis, drop this closing one
                    log("Unexpected closing parenthesis, dropping", Warning);
                    drop = true;
                }
                else
                {
                    // Pop function from the stack
                    cur_function_arr.pop_back();
                    cur_function = cur_function_arr.isEmpty() ? "" : cur_function_arr.back();
                            
                }
            }
            if (!drop) {
                cur_sub_value_arr.push_back(QString(1,css_input[i]));
            }
        }
        else if(css_input[i] != '}')
        {
            cur_sub_value += css_input[i];
        }
        if( (css_input[i] == '}' || css_input[i] == ';' || pn) && !cur_selector.isEmpty())
        {
            // End of value: normalize, optimize and store property
            if(cur_at == "")
            {
                cur_at = "standard";
            }

            // Kill all whitespace
            cur_at = CSSUtils::trim(cur_at);
            cur_selector = CSSUtils::trim(cur_selector);
            cur_value = CSSUtils::trim(cur_value);
            cur_property = CSSUtils::trim(cur_property);
            cur_sub_value = CSSUtils::trim(cur_sub_value);

            cur_property = CSSUtils::strtolower(cur_property);

            if(cur_sub_value != "")
            {
                cur_sub_value_arr.push_back(cur_sub_value);
                cur_sub_value = "";
            }

            // Check for leftover open parentheses
            if (!cur_function_arr.isEmpty())
            {
                QVector<QString>::reverse_iterator rit;
                for (rit = cur_function_arr.rbegin(); rit != cur_function_arr.rend(); ++rit)
                {
                    log("Closing parenthesis missing for '" + *rit + "', inserting", Warning);
                    cur_sub_value_arr.push_back(")");
                }
            }

            cur_value = CSSUtils::build_value(cur_sub_value_arr);


            bool valid = (CSSProperties::instance()->contains(cur_property) &&
                          CSSProperties::instance()->levels(cur_property).indexOf(css_level,0) != -1);
            if(1)
            {
                //add(cur_at,cur_selector,cur_property,cur_value);
                add_token(VALUE, cur_value);
            }
            if(!valid)
            {
                log("Invalid property in " + CSSUtils::strtoupper(css_level) + ": " + cur_property,Warning);
            }

            //Split multiple selectors here if necessary
            cur_property = "";
            cur_sub_value_arr.clear();
            cur_value = "";
        }
        if(css_input[i] == '}')
        {
            explode_selectors();
            add_token(SEL_END, cur_selector);
            astatus = PIS;
            invalid_at = false;
            cur_selector = "";
        }
    }
    else if(!pn)
    {
        cur_sub_value += css_input[i];

        if(CSSUtils::ctype_space(css_input[i]))
        {
            if(CSSUtils::trim(cur_sub_value) != "")
            {
                cur_sub_value_arr.push_back(CSSUtils::trim(cur_sub_value));
            }
            cur_sub_value = "";
        }
    }
}


void CSSParser::parseInComment(QString& css_input, int& i, parse_status& astatus, parse_status& afrom,
                               QString& cur_comment)
{

    if(css_input[i] == '*' && CSSUtils::s_at(css_input,i+1) == '/')
    {
        astatus = afrom;
        ++i;
        add_token(COMMENT, cur_comment);
        cur_comment = "";
    }
    else
    {
        cur_comment += css_input[i];
    }
}


void CSSParser::parseInString(QString& css_input, int& i, parse_status& astatus, parse_status& afrom,
                              QChar& str_char, bool& str_in_str)
{

    if(str_char == ')' && (css_input[i] == '"' || css_input[i] == '\'') &&
       str_in_str == false && !CSSUtils::escaped(css_input,i))
    {
        str_in_str = true;
    }
    else if(str_char == ')' && (css_input[i] == '"' || css_input[i] == '\'') &&
            str_in_str == true && !CSSUtils::escaped(css_input,i))
    {
        str_in_str = false;
    }
    QString temp_add = ""; temp_add += css_input[i];
    if( (css_input[i] == '\n' || css_input[i] == '\r') &&
        !(css_input[i-1] == '\\' && !CSSUtils::escaped(css_input,i-1)) )
    {
        temp_add = "\\A ";
        log("Fixed incorrect newline in string",Warning);
    }
    if (!(str_char == ')' && QString(" \n\t\r\0xb").contains(css_input[i]) && !str_in_str))
    {
        cur_string += temp_add;
    }
    if(css_input[i] == str_char && !CSSUtils::escaped(css_input,i) && str_in_str == false)
    {
        astatus = afrom;
        // Quotes are optional so leave them alone (ie. act like a parser not an optimizer)
        if(afrom == PIV)
        {
            cur_sub_value += cur_string;
        }
        else if(afrom == PIS)
        {
            cur_selector += cur_string;
        }
    }
}


void CSSParser::parse_css(QString css_input)
{
    reset_parser();
    css_input = css_input.replace("\r\n","\n"); // Replace newlines
    css_input += "\n";
    parse_status astatus = PIS, afrom;
    parse_status old_status = PIS;
    record_position(PIS, PIS, css_input, 0, true);
    QString cur_comment;

    cur_sub_value_arr.clear();
    cur_function_arr.clear(); // Stack of nested function calls
    QChar str_char;
    bool str_in_str = false;
    bool invalid_at = false;
    bool pn = false;

    int str_size = css_input.length();
    for(int i = 0; i < str_size; ++i)
    {
        // track line number
        if(css_input[i] == '\n') ++line;

        // record current position for selected state transitions
        if (old_status != astatus)
        {
            record_position(old_status, astatus, css_input, i);
        }
        old_status = astatus;


        switch(astatus)
        {
            /* Case in-at-block */
            case PAT:
                parseInAtBlock(css_input, i, astatus, afrom);
                break;

            /* Case in-selector */
            case PIS:
                parseInSelector(css_input, i, astatus, afrom, invalid_at, str_char, str_size);
                break;

            /* Case in-property */
            case PIP:
                parseInProperty(css_input, i, astatus, afrom, invalid_at);
                break;

            /* Case in-value */
            case PIV:
                parseInValue(css_input, i, astatus, afrom, invalid_at, str_char, pn, str_size);
                break;

            /* Case in-string */
            case PINSTR:
                parseInString(css_input, i, astatus, afrom, str_char, str_in_str);
                break;

            /* Case in-comment */
            case PIC:
                parseInComment(css_input, i, astatus, afrom, cur_comment);
                break;
        }
    }
    // validate that every selector start had its own selector end
    if (selector_nest_level != 0)
    {
        log("Unbalanced selector braces in style sheet", Error, line);
    }
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


bool CSSParser::property_is_next(QString istring, int pos)
{
    istring = istring.mid(pos,istring.length()-pos);
    pos = CSSUtils::find_first_of(istring, ":");
    if(pos == -1)
    {
        return false;
    }
    istring = CSSUtils::strtolower(CSSUtils::trim(istring.mid(0,pos)));
    return CSSProperties::instance()->contains(istring);
}


QVector<QString> CSSParser::get_parse_errors()
{ 
    return get_logs(CSSParser::Error);
}


QVector<QString> CSSParser::get_parse_warnings()
{
    return get_logs(CSSParser::Warning);
}


QVector<QString> CSSParser::get_parse_info()
{
    return get_logs(CSSParser::Information);
}


QVector<QString> CSSParser::get_logs(CSSParser::message_type mt)
{
    QVector<QString> res;
    if(logs.size() > 0)
    {
        for(QMap<int, QVector<message> >::iterator j = logs.begin(); j != logs.end(); j++ )
        {
            for(int i = 0; i < j.value().size(); ++i)
            {
                if (j.value()[i].t == mt) {
                    res.push_back(QString::number(j.key()) + ": " + j.value()[i].m);
                }
            }
        }
    }
    return res;
}


// allow a new set of csstokens to be loaded and then serialized
void CSSParser::set_csstokens(const QVector<CSSParser::token> &ntokens)
{
    csstokens.clear();
    for(int i = 0; i < ntokens.size(); i++)
    {
        token atemp = ntokens[i];
        csstokens.push_back(atemp);
    }
}


// update the position only for selected state transitions
void CSSParser::record_position(parse_status old_status, parse_status new_status,
                                QString &css_input, int i, bool force)
{
    // to reach here old_status must be != new_status
    bool record = false;

    // any state into a  comment
    if (new_status == PIC) record = true;

    // start of a property
    if ((old_status == PIS) && (new_status == PIP)) record = true;
    if ((old_status == PIV) && (new_status == PIP)) record = true;

    // from properties to values or from @charset, @namespace, and @import into values
    if ((old_status == PAT) && (new_status == PIV)) record = true;
    if ((old_status == PIP) && (new_status == PIV)) record = true;

    // starting a new selector
    if ((old_status == PAT) && (new_status == PIS)) record = true;
    if ((old_status == PIV) && (new_status == PIS)) record = true;
    if ((old_status == PIP) && (new_status == PIS)) record = true;
    if ((old_status == PIC) && (new_status == PIS)) record = true; 
    
    if (record || force) {
        spos = CSSUtils::find_first_not_of(css_input, " \n\t\r\0xb", i);
        sline = line;
        for(int j = i+1; j <= spos; j++) {
            if (css_input[j] == '\n') sline++;
        }
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
