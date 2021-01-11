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
 
#include <cstring> 
#include "Misc/CSSUtils.h"

std::string CSSUtils::strtolower(std::string istring)
{
    int str_size = istring.length();
    for(int i = 0; i < str_size; i++)
    {
        istring[i] = chartolower(istring[i]);
    }
    return istring;
}


char CSSUtils::chartolower(const char c)
{
    switch(c)
    {
        case 'A': return 'a';
        case 'B': return 'b';
        case 'C': return 'c';
        case 'D': return 'd';
        case 'E': return 'e';
        case 'F': return 'f';
        case 'G': return 'g';
        case 'H': return 'h';
        case 'I': return 'i';
        case 'J': return 'j';
        case 'K': return 'k';
        case 'L': return 'l';
        case 'M': return 'm';
        case 'N': return 'n';
        case 'O': return 'o';
        case 'P': return 'p';
        case 'Q': return 'q';
        case 'R': return 'r';
        case 'S': return 's';
        case 'T': return 't';
        case 'U': return 'u';
        case 'V': return 'v';
        case 'W': return 'w';
        case 'X': return 'x';
        case 'Y': return 'y';
        case 'Z': return 'z';
        default: return c;
    }
}


std::string CSSUtils::strtoupper(std::string istring)
{
    int str_size = istring.length();
    for(int i = 0; i < str_size; i++)
    {
        istring[i] = chartoupper(istring[i]);
    }
    return istring;
}


char CSSUtils::chartoupper(const char c)
{
    switch(c)
    {
        case 'a': return 'A';
        case 'b': return 'B';
        case 'c': return 'C';
        case 'd': return 'D';
        case 'e': return 'E';
        case 'f': return 'F';
        case 'g': return 'G';
        case 'h': return 'H';
        case 'i': return 'I';
        case 'j': return 'J';
        case 'k': return 'K';
        case 'l': return 'L';
        case 'm': return 'M';
        case 'n': return 'N';
        case 'o': return 'O';
        case 'p': return 'P';
        case 'q': return 'Q';
        case 'r': return 'R';
        case 's': return 'S';
        case 't': return 'T';
        case 'u': return 'U';
        case 'v': return 'V';
        case 'w': return 'W';
        case 'x': return 'X';
        case 'y': return 'Y';
        case 'z': return 'Z';
        default: return c;
    }
}


double CSSUtils::hexdec(std::string istring)
{
    double ret = 0;
    istring = trim(istring);
    for(int i = istring.length()-1; i >= 0; --i)
    {
        int num = 0;
        switch(tolower(istring[i]))
        {
            case 'a': num = 10; break;
            case 'b': num = 11; break;
            case 'c': num = 12; break;
            case 'd': num = 13; break;
            case 'e': num = 14; break;
            case 'f': num = 15; break;
            case '1': num = 1; break;
            case '2': num = 2; break;
            case '3': num = 3; break;
            case '4': num = 4; break;
            case '5': num = 5; break;
            case '6': num = 6; break;
            case '7': num = 7; break;
            case '8': num = 8; break;
            case '9': num = 9; break;
            case '0': num = 0; break;
        }
        ret += num*pow((double) 16, (double) istring.length()-i-1);
    }
    return ret;
}


std::string CSSUtils::char2str(const char c)
{
    std::string ret = "";
    ret += c;
    return ret;
}


std::string CSSUtils::char2str(const char *c)
{
    std::stringstream sstream;
    sstream << c;
    return sstream.str();
}


std::string CSSUtils::file_get_contents(const std::string filename)
{
    std::ifstream file_input(filename.c_str(),std::ios::binary);
    std::string line, file_contents = "";
    
    if(file_input.bad())
    {
        return "";
    }
    else
    {
        while(file_input.good())
        {
            getline(file_input,line);
            file_contents += (line + "\n");
        }       
    }
    file_input.close();

    return file_contents;
}


bool CSSUtils::file_exists(const char *filename)
{
    std::ifstream file_input(filename);

    if(file_input.is_open())
    {
        file_input.close();
        return true;
    }
    
    file_input.close();
    return false;
}


bool CSSUtils::escaped(const std::string &istring, const int pos) 
{
    return !(s_at(istring,pos-1) != '\\' || escaped(istring,pos-1));
}


// Safe replacement for .at()
char CSSUtils::s_at(const std::string &istring, const int pos)
{
    if(pos > (istring.length()-1) || pos < 0)
    {
        return 0;
    } 
    else 
    {
        return istring[pos];
    }
}


std::vector<std::string> CSSUtils::explode(const std::string e, std::string s, const bool check)
{
    std::vector<std::string> ret;
    int iPos = s.find(e, 0);
    int iPit = e.length();
        
    while(iPos > -1)
    {
        if(iPos != 0 || check)
        {
            ret.push_back(s.substr(0,iPos));
        }
        s.erase(0,iPos+iPit);
        iPos = s.find(e, 0);
    }
        
    if(s != "" || check)
    {
        ret.push_back(s);
    }
    return ret;
}


std::string CSSUtils::implode(const std::string e, const std::vector<std::string> s)
{
    std::string ret;
    for(int i = 0; i < s.size(); i++)
    {
        ret += s[i];
        if(i != (s.size()-1)) ret += e;
    }
    return ret;
}


std::string CSSUtils::build_value(const std::vector<std::string> subvalues)
{
    std::string ret;
    for(int i = 0; i < subvalues.size(); i++)
    {
        ret += subvalues[i];
        if(i != (subvalues.size()-1))
        {
            char last = s_at(subvalues[i], subvalues[i].length()-1);
            char next = s_at(subvalues[i+1], 0);
            if (strchr("(,=:", last) != NULL || strchr("),=:", next) != NULL)
            {
                continue;
            }
            ret += " ";
        }
    }
    return ret;
}


std::string CSSUtils::str_replace(const std::string find, const std::string replace, std::string str)
{
    int len = find.length();
    int replace_len = replace.length();
    int pos = str.find(find);

    while(pos != std::string::npos)
    {  
        str.replace(pos, len, replace);
        pos = str.find(find, pos + replace_len);
    }
    return str;
}



bool CSSUtils::in_char_arr(const char* haystack, const char needle)
{
    for(int i = 0; i < strlen(haystack); ++i)
    {
        if(haystack[i] == needle)
        {
            return true;
        }
    }
    return false;
}


bool CSSUtils::in_str_array(const std::string& haystack, const char needle)
{
    return (haystack.find_first_of(needle,0) != std::string::npos);
}


bool CSSUtils::ctype_space(const char c)
{
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == 11);
}


bool CSSUtils::ctype_digit(const char c)
{
    return (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9');
}


bool CSSUtils::ctype_xdigit(char c)
{
    c = chartolower(c);
    return (ctype_digit(c) || c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f');
}

bool CSSUtils::ctype_alpha(char c)
{
    c = chartolower(c);
    return (c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' || 
            c == 'g' || c == 'h' || c == 'i' || c == 'j' || c == 'k' || c == 'l' ||
            c == 'm' || c == 'n' || c == 'o' || c == 'p' || c == 'q' || c == 'r' ||
            c == 's' || c == 't' || c == 'u' || c == 'v' || c == 'w' || c == 'x' || 
            c == 'y' || c == 'z');
}


const std::string CSSUtils::trim(const std::string istring)
{
    std::string::size_type first = istring.find_first_not_of(" \n\t\r\0xb");
    if (first == std::string::npos) {
        return std::string();
    }
    else
    {
        std::string::size_type last = istring.find_last_not_of(" \n\t\r\0xb");
        return istring.substr( first, last - first + 1);
    }
}


const std::string CSSUtils::ltrim(const std::string istring)
{
    std::string::size_type first = istring.find_first_not_of(" \n\t\r\0xb");
    if (first == std::string::npos) {
        return std::string();
    }
    else 
    {
        return istring.substr( first );
    }
}


const std::string CSSUtils::rtrim(const std::string istring)
{
    std::string::size_type last = istring.find_last_not_of(" \n\t\r\0xb"); /// must succeed
    return istring.substr( 0, last + 1);
}


const std::string CSSUtils::rtrim(const std::string istring, const std::string chars)
{
    std::string::size_type last = istring.find_last_not_of(chars); /// must succeed
    return istring.substr( 0, last + 1);
}

const std::string CSSUtils::indent(int lvl, const std::string &base)
{
    std::string ind = "";
    for (int i = 0; i < lvl; i++) {
        ind = ind + base;
    }
    return ind;
}
