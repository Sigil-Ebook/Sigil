/************************************************************************
 **
 **  Copyright (C) 2021  Kevin B. Hendricks, Stratford, Ontario, Canada
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
 ** CSSTidy Copyright 2005-2007 Florian Schmitz
 ** Available under the LGPL 2.1
 ** You should have received a copy of the GNU Lesser General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/
 
#include "Misc/CSSProperties.h"
#include "Misc/CSSUtils.h"
#include "Misc/CSSParser.h"

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

    // Used for serializing parsed css
    csstemplate.push_back("    ");    //  0 - standard indentation
    csstemplate.push_back(" {\n");    //  1 - bracket after @-rule
    csstemplate.push_back("");        //  2 - unused
    csstemplate.push_back(" {\n");    //  3 - bracket after selector was "\n{\n"
    csstemplate.push_back("");        //  4 - unused
    csstemplate.push_back("");        //  5 - string after property before value
    csstemplate.push_back(";\n");     //  6 - string after value
    csstemplate.push_back("}");       //  7 - closing bracket - selector
    csstemplate.push_back("\n\n");    //  8 - space between blocks {...}
    csstemplate.push_back("}\n\n");   //  9 - closing bracket @-rule
    csstemplate.push_back("");        // 10 - unused
    csstemplate.push_back("");        // 11 - before comment
    csstemplate.push_back("\n");      // 12 - after comment
    csstemplate.push_back("\n");      // 13 - after last line @-rule

    // at_rule to parser state map
    at_rules["page"] = PIS;
    at_rules["font-face"] = PIS;
    at_rules["charset"] = PIV;
    at_rules["import"] = PIV;
    at_rules["namespace"] = PIV;
    at_rules["media"] = PAT;
    at_rules["keyframes"] = PAT;
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


void CSSParser::set_level(std::string level)
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


std::string CSSParser::get_charset()
{
    return charset;
}

std::string CSSParser::get_namespace()
{
    return namesp;
}

std::vector<std::string> CSSParser::get_import()
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


std::string CSSParser::get_type_name(CSSParser::token_type t)
{
    return token_type_names[t];
}


void CSSParser::add_token(const token_type ttype, const std::string data, const bool force)
{
    token temp;
    temp.type = ttype;
    temp.pos = spos;
    temp.data = (ttype == COMMENT) ? data : CSSUtils::trim(data);
    csstokens.push_back(temp);
}

void CSSParser::log(const std::string msg, const message_type type, int iline)
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

std::string CSSParser::unicode(std::string& istring, int& i)
{
    ++i;
    std::string add = "";
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
        std::string msg = "Replaced unicode notation: Changed \\" + CSSUtils::rtrim(add) + " to ";
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


bool CSSParser::is_token(std::string& istring, const int i)
{
    return (CSSUtils::in_str_array(tokens, istring[i]) && !CSSUtils::escaped(istring,i));
}


void CSSParser::explode_selectors()
{
    sel_separate = std::vector<int>();
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
    // FIXME: control can reach end of non-void function
    return -1;
}


std::string CSSParser::serialize_css(std::string filename, bool tostdout)
{
    if(charset == "" && namesp == "" && import.empty() && csstokens.empty())
    {
        std::cout << "Warning: empty CSS output!" << std::endl;
    }

    std::ofstream file_output;
    if(filename != "")
    {
         file_output.open(filename.c_str(),std::ios::binary);
         if(file_output.bad())
         {
             std::cout << "Error when trying to save the output file!" << std::endl;
             return "";
         }
    }

    std::stringstream output;
    int lvl = 0;
    std::string indent = "";

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
                output << indent + csstemplate[7];
                if(_seeknocomment(i, 1) != AT_END) output << csstemplate[8];
                break;

            case AT_END:
                lvl--; if (lvl < 0) lvl = 0;
                indent = CSSUtils::indent(lvl, csstemplate[0]);
                output << csstemplate[13] << indent << csstemplate[9];
                break;

            case COMMENT:
                output << csstemplate[11] <<  "/*" << csstokens[i].data << "*/" << csstemplate[12];
                break;

            case CSS_END:
                break;
        }
    }

    std::string output_string = CSSUtils::trim(output.str());

    if(tostdout)
    {
        std::cout << output_string << "\n";
    }
    if(filename != "")
    {
        file_output << output_string;
        file_output.close();
    }
    return output_string;
}


void CSSParser::parseInAtBlock(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom)
{
    if(is_token(css_input,i))
    {
        if(css_input[i] == '/' && CSSUtils::s_at(css_input,i+1) == '*')
        {
            astatus = PIC; i += 2;
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
            if(!CSSUtils::in_char_arr("():/.", css_input[i]))
            {
                // Strictly speaking, these are only permitted in @media rules 
                log("Unexpected symbol '" + std::string(css_input, i, 1) + "' in @-rule", Warning);
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


void CSSParser::parseInSelector(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom,
                                bool& invalid_at, char& str_char, int str_size)
{
    if(is_token(css_input,i))
    {
        // if(css_input[i] == '/' && CSSUtils::s_at(css_input,i+1) == '*' && 
        // trim(cur_selector) == "") selector as dep doesn't make any sense here, huh?
        if(css_input[i] == '/' && CSSUtils::s_at(css_input,i+1) == '*')
        {
            astatus = PIC; ++i;
            afrom = PIS;
        }
        else if(css_input[i] == '@' && CSSUtils::trim(cur_selector) == "")
        {
            // Check for at-rule
            invalid_at = true;
            for(std::map<std::string,parse_status>::iterator j = at_rules.begin(); j != at_rules.end(); ++j )
            {
                if(CSSUtils::strtolower(css_input.substr(i+1,j->first.length())) == j->first)
                {
                    (j->second == PAT) ? cur_at = "@" + j->first : cur_selector = "@" + j->first;
                    astatus = j->second;
                    i += j->first.length();
                    invalid_at = false;
                }
            }
            if (invalid_at)
            {
                cur_selector = "@";
                std::string invalid_at_name = "";
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
            sel_separate = std::vector<int>();
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
        else if(!(css_input[i] == '*' && (CSSUtils::s_at(css_input,i+1) == '.' || CSSUtils::s_at(css_input,i+1) == '[' || CSSUtils::s_at(css_input,i+1) == ':' || CSSUtils::s_at(css_input,i+1) == '#')))
        {
            cur_selector += css_input[i];
        }
    }
    else
    {
        int lastpos = cur_selector.length()-1;
        if((lastpos == -1) || !( (CSSUtils::ctype_space(cur_selector[lastpos]) ||
                                  (is_token(cur_selector,lastpos) && cur_selector[lastpos] == ',')) &&
                                 CSSUtils::ctype_space(css_input[i])))
        {
            cur_selector += css_input[i];
        }
    }
}


void CSSParser::parseInProperty(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom,
                                bool& invalid_at)
{

    if (is_token(css_input,i))
    {
        if (css_input[i] == ':' || (css_input[i] == '=' && cur_property != ""))
        {
            astatus = PIV;
            bool valid = true || (CSSProperties::instance()->contains(cur_property) && 
                                  CSSProperties::instance()->levels(cur_property).find(css_level,0) != std::string::npos);
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
            log("Unexpected character '" + std::string(1, css_input[i]) + "'in property name", Error);
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


void CSSParser::parseInValue(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom,
                             bool& invalid_at, char& str_char, bool& pn, int str_size)
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
            log("Unexpected character '" + std::string(1, css_input[i]) + "' in " + cur_selector, Error);
        }
        if(css_input[i] == '/' && CSSUtils::s_at(css_input,i+1) == '*')
        {
            astatus = PIC; ++i;
            afrom = PIV;
        }
        else if(css_input[i] == '"' || css_input[i] == '\'' ||
                (css_input[i] == '(' && cur_sub_value == "url") )
        {
            str_char = (css_input[i] == '(') ? ')' : css_input[i];
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
            if(cur_selector.substr(0,1) == "@" && 
               at_rules.count(cur_selector.substr(1)) > 0 &&
               at_rules[cur_selector.substr(1)] == PIV)
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
                     std::string aimport = CSSUtils::build_value(cur_sub_value_arr);
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
                sel_separate = std::vector<int>();
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
                if (cur_function_arr.empty())
                {
                    // No matching open parenthesis, drop this closing one
                    log("Unexpected closing parenthesis, dropping", Warning);
                    drop = true;
                }
                else
                {
                    // Pop function from the stack
                    cur_function_arr.pop_back();
                    cur_function = cur_function_arr.empty() ? "" : cur_function_arr.back();
                            
                }
            }
            if (!drop) {
                cur_sub_value_arr.push_back(std::string(1,css_input[i]));
            }
        }
        else if(css_input[i] != '}')
        {
            cur_sub_value += css_input[i];
        }
        if( (css_input[i] == '}' || css_input[i] == ';' || pn) && !cur_selector.empty())
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
            if (!cur_function_arr.empty())
            {
                std::vector<std::string>::reverse_iterator rit;
                for (rit = cur_function_arr.rbegin(); rit != cur_function_arr.rend(); ++rit)
                {
                    log("Closing parenthesis missing for '" + *rit + "', inserting", Warning);
                    cur_sub_value_arr.push_back(")");
                }
            }

            cur_value = CSSUtils::build_value(cur_sub_value_arr);


            bool valid = (CSSProperties::instance()->contains(cur_property) &&
                          CSSProperties::instance()->levels(cur_property).find(css_level,0) != std::string::npos);
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


void CSSParser::parseInComment(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom,
                               std::string& cur_comment)
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


void CSSParser::parseInString(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom,
                              char& str_char, bool& str_in_str)
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
    std::string temp_add = ""; temp_add += css_input[i];
    if( (css_input[i] == '\n' || css_input[i] == '\r') &&
        !(css_input[i-1] == '\\' && !CSSUtils::escaped(css_input,i-1)) )
    {
        temp_add = "\\A ";
        log("Fixed incorrect newline in string",Warning);
    }
    if (!(str_char == ')' && 
          CSSUtils::char2str(css_input[i]).find_first_of(" \n\t\r\0xb") != std::string::npos && !str_in_str))
    {
        cur_string += temp_add;
    }
    if(css_input[i] == str_char && !CSSUtils::escaped(css_input,i) && str_in_str == false)
    {
        astatus = afrom;
        if (cur_function == "" && 
            cur_string.find_first_of(" \n\t\r\0xb") == std::string::npos &&
            cur_property != "content" && cur_sub_value != "format")
        {
            // If the string is not inside a function call, contains no whitespace, 
            // and the current property is not 'content', it may be safe to remove quotes.
            // TODO: Are there any properties other than 'content' where this is unsafe?
            // TODO: What if the string contains a comma or slash, and the property is a list or shorthand?
            if (str_char == '"' || str_char == '\'')
            {
                // If the string is in double or single quotes, remove them
                // FIXME: once url() is handled separately, this may always be the case.
                cur_string = cur_string.substr(1, cur_string.length() - 2);
            } else if (cur_string.length() > 3 && (cur_string[1] == '"' || cur_string[1] == '\'')) /* () */ 
            {
                cur_string = cur_string[0] + cur_string.substr(2, cur_string.length() - 4) + cur_string[cur_string.length()-1];
            }
        }
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


void CSSParser::parse_css(std::string css_input)
{
    reset_parser();
    css_input = CSSUtils::str_replace("\r\n","\n",css_input); // Replace newlines
    css_input += "\n";
    parse_status astatus = PIS, afrom;
    parse_status old_status = PIS;
    record_position(PIS, PIS, css_input, 0, true);
    std::string cur_comment;

    cur_sub_value_arr.clear();
    cur_function_arr.clear(); // Stack of nested function calls
    char str_char;
    bool str_in_str = false;
    bool invalid_at = false;
    bool pn = false;

    int str_size = css_input.length();
    for(int i = 0; i < str_size; ++i)
    {
        if(css_input[i] == '\n' || css_input[i] == '\r')
        {
            ++line;
        }


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
}


bool CSSParser::property_is_next(std::string istring, int pos)
{
    istring = istring.substr(pos,istring.length()-pos);
    pos = istring.find_first_of(':',0);
    if(pos == std::string::npos)
    {
        return false;
    }
    istring = CSSUtils::strtolower(CSSUtils::trim(istring.substr(0,pos)));
    return CSSProperties::instance()->contains(istring);
}


std::vector<std::string> CSSParser::get_parse_errors()
{ 
    return get_logs(CSSParser::Error);
}


std::vector<std::string> CSSParser::get_parse_warnings()
{
    return get_logs(CSSParser::Warning);
}


std::vector<std::string> CSSParser::get_parse_info()
{
    return get_logs(CSSParser::Information);
}


std::vector<std::string> CSSParser::get_logs(CSSParser::message_type mt)
{
    std::vector<std::string> res;
    if(logs.size() > 0)
    {
        for(std::map<int, std::vector<message> >::iterator j = logs.begin(); j != logs.end(); j++ )
        {
            for(int i = 0; i < j->second.size(); ++i)
            {
                if (j->second[i].t == mt) {
                    res.push_back(std::to_string(j->first) + ": " + j->second[i].m);
                }
            }
        }
    }
    return res;
}


// allow a new set of csstokens to be loaded and then serialized
void CSSParser::set_csstokens(const std::vector<CSSParser::token> &ntokens)
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
                                std::string &css_input, int i, bool force)
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

    if (record || force) {
        spos = css_input.find_first_not_of(" \n\t\r\0xb", i);
    }
}
