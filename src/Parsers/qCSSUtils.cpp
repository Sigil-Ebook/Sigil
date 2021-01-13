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
 
#include "Parsers/qCSSUtils.h"

QString CSSUtils::strtolower(QString istring)
{
    int str_size = istring.length();
    for(int i = 0; i < str_size; i++)
    {
        istring[i] = chartolower(istring[i]);
    }
    return istring;
}


QChar CSSUtils::chartolower(const QChar c)
{
    if ((c.unicode() >= 'A') && (c.unicode() <= 'Z')) return c.toLower();
    return c;
}


QString CSSUtils::strtoupper(QString istring)
{
    int str_size = istring.length();
    for(int i = 0; i < str_size; i++)
    {
        istring[i] = chartoupper(istring[i]);
    }
    return istring;
}


QChar CSSUtils::chartoupper(const QChar c)
{
    if ((c.unicode() >= 'a') && (c.unicode() <= 'z')) return c.toUpper();
    return c;
}


unsigned int CSSUtils::hexdec(QString istring)
{
    unsigned int res = 0;
    bool okay = false;
    istring = trim(istring);
    res = istring.toUInt(&okay, 16);
    if (okay) return res;
    return 0;
}

 
bool CSSUtils::escaped(const QString &istring, const int pos) 
{
    return !(s_at(istring,pos-1) != '\\' || escaped(istring,pos-1));
}


// Safe replacement for .at()
QChar CSSUtils::s_at(const QString &istring, const int pos)
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


QVector<QString> CSSUtils::explode(const QString e, QString s, const bool check)
{
    QVector<QString> ret;
    int iPos = s.indexOf(e, 0);
    int iPit = e.length();
        
    while(iPos > -1)
    {
        if(iPos != 0 || check)
        {
            ret.push_back(s.mid(0,iPos));
        }
        s = s.remove(0,iPos+iPit);
        iPos = s.indexOf(e, 0);
    }
        
    if(s != "" || check)
    {
        ret.push_back(s);
    }
    return ret;
}


QString CSSUtils::implode(const QString e, const QVector<QString> s)
{
    QString ret;
    for(int i = 0; i < s.size(); i++)
    {
        ret += s[i];
        if(i != (s.size()-1)) ret += e;
    }
    return ret;
}


QString CSSUtils::build_value(const QVector<QString> subvalues)
{
    QString ret;
    for(int i = 0; i < subvalues.size(); i++)
    {
        ret += subvalues[i];
        if(i != (subvalues.size()-1))
        {
            QChar last = s_at(subvalues[i], subvalues[i].length()-1);
            QChar next = s_at(subvalues[i+1], 0);
            // allow a space after a comma in a values list.  next line was:
            // if (QString("(,=:").contains(last) || QString("),=:").contains(next))
            if (QString("(=:").contains(last) || QString("),=:").contains(next))
            {
                continue;
            }
            ret += " ";
        }
    }
    return ret;
}


bool CSSUtils::ctype_space(const QChar c)
{
    return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == 11);
}


bool CSSUtils::ctype_digit(const QChar c)
{
    if ((c.unicode() >= '0') && (c.unicode() <= '9')) return true;
    return false;
}


bool CSSUtils::ctype_xdigit(QChar c)
{
    c = chartolower(c);
    return (ctype_digit(c) || c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f');
}


bool CSSUtils::ctype_alpha(QChar c)
{
    c = chartolower(c);
    if ((c.unicode() >= 'a') && (c.unicode() <= 'z')) return true;
    return false;
}


const QString CSSUtils::trim(const QString istring)
{
    int first = find_first_not_of(istring, " \n\t\r\0xb");
    if (first == -1) {
        return QString();
    }
    else
    {
        int last = find_last_not_of(istring, " \n\t\r\0xb");
        return istring.mid( first, last - first + 1);
    }
}


const QString CSSUtils::ltrim(const QString istring)
{
    int first = find_first_not_of(istring, " \n\t\r\0xb");
    if (first == -1) {
        return QString();
    }
    else 
    {
        return istring.mid(first);
    }
}


const QString CSSUtils::rtrim(const QString istring)
{
    int last = find_last_not_of(istring, " \n\t\r\0xb"); /// must succeed
    return istring.mid(0, last + 1);
}


const QString CSSUtils::rtrim(const QString istring, const QString chars)
{
    int last = find_last_not_of(istring, chars); /// must succeed
    return istring.mid( 0, last + 1);
}


const QString CSSUtils::indent(int lvl, const QString &base)
{
    QString ind = "";
    for (int i = 0; i < lvl; i++) {
        ind = ind + base;
    }
    return ind;
}


int CSSUtils::find_first_of(const QString &tgt, const QString& stopchars, int p)
{
    while((p < tgt.length()) && !stopchars.contains(tgt.at(p))) p++;
    if (p < tgt.length()) return p;
    return -1;
}


int CSSUtils::find_first_not_of(const QString &tgt, const QString& skipchars, int p)
{
    while((p < tgt.length()) && skipchars.contains(tgt.at(p))) p++;
    if (p < tgt.length()) return p;
    return -1;
}


int CSSUtils::find_last_not_of(const QString &tgt, const QString& skipchars)
{
    int p = tgt.length() - 1;
    while((p >= 0) && skipchars.contains(tgt.at(p))) p--;
    if (p < 0) return -1;
    return p;
}
