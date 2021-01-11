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
 
#ifndef HEADER_CSSPARSER
#define HEADER_CSSPARSER 

#include <cstdlib>
#include <utility>
#include <string> 
#include <iterator>
#include <vector>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>

class CSSParser 
{ 

public:

    enum parse_status
    {
        PIS, PIP, PIV, PINSTR, PIC, PAT
    };

    enum message_type
    {
        Information,Warning,Error
    };

    enum token_type
    {
        CHARSET, IMPORT, NAMESP, AT_START, AT_END, SEL_START, SEL_END, PROPERTY, VALUE, COMMENT, CSS_END
    };

    struct token
    {
        token_type type;
        int pos;
        std::string data;
    };

    struct message
    {
        std::string m;
        message_type t;
    };

    CSSParser();

    // valid levels are "CSS1.0", "CSS2.0", "CSS2.1", "CSS3.0"
    void set_level(std::string level = "CSS3.0");

    void parse_css(std::string css_input);

    void reset_parser();        

    // serialize the current list of csstokens back to css
    std::string serialize_css(std::string filename = "", bool tostdout = true);
        
    // access charset, namespace and imports without having to walk csstokens
    std::string get_charset();
    std::vector<std::string> get_import();
    std::string get_namespace();

    // walk the csstokens list, token by token
    // set start_ptr to the position you woud like to start at in the list
    // leaving it as -1 will simply start at 0 and increment
    // last token is a dummy token with type set to CSS_END 
    token get_next_token(int start_ptr = -1);

    // covert token type enum value to a descriptive string
    std::string get_type_name(token_type t);

    // this routine allows external modifications to the csstokens
    // to be brought back into CSSParser in order to serialize them
    // with serialize_css
    void set_csstokens(const std::vector<token> &ntokens);

    // get errors, warnings, and information
    std::vector<std::string> get_parse_errors();
    std::vector<std::string> get_parse_warnings();
    std::vector<std::string> get_parse_info();


private:

    void parseInAtBlock(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom);

    void parseInSelector(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom,
                         bool& invalid_at, char& str_char, int str_size);

    void parseInProperty(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom,
                         bool& invalid_at);

    void parseInValue(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom,
                      bool& invalid_at, char& str_char, bool& pn, int str_size );

    void parseInComment(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom,
                        std::string& cur_comment);

    void parseInString(std::string& css_input, int& i, parse_status& astatus, parse_status& afrom,
                       char&  str_char, bool& str_in_str);


    // filter the logs by message type
    std::vector<std::string> get_logs(message_type t);

    // parses unicode notations
    std::string unicode(std::string& istring, int& i);
        
    // checks if the chat in istring at i is a token
    bool is_token(std::string& istring, const int i);
                        
    void add_token(const token_type ttype, const std::string data, const bool force = false);
        
    // Add a message to the message log
    void log(const std::string msg, const message_type type, int iline = 0);

    // records token position information
    void record_position(parse_status old_status, parse_status new_status,
                         std::string &css_input, int i, bool force=false);
        
    int _seeknocomment(const int key, const int move);

    void explode_selectors();

    static bool property_is_next(std::string istring, int pos);

    // private member variables
    std::vector<std::string> token_type_names;
    std::map<std::string, parse_status>  at_rules;
    std::vector<std::string> csstemplate;
    std::string css_level;
    std::string tokens;
    int token_ptr;

    std::map<int, std::vector<message> > logs;
    int  line;
    int  spos;

    std::string charset;
    std::string namesp;
    std::vector<std::string> import;
    std::vector<token> csstokens;
    std::string cur_selector;
    std::string cur_at;
    std::string cur_property;
    std::string cur_function;
    std::string cur_sub_value;
    std::string cur_value; 
    std::string cur_string;
    std::vector<int>   sel_separate;
    std::vector<std::string> cur_sub_value_arr;
    std::vector<std::string> cur_function_arr;

};
        
#endif // HEADER_CSSPARSER
