/************************************************************************
**
**  Copyright (C) 2022 Kevin B. Hendricks, Stratford Ontario Canada
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
*************************************************************************/

#include <QString>
#include <QStringList>
#include <QHash>
#include <QSet>
#include <QFileInfo>
#include <QDebug>

#include "Parsers/qCSSParser.h"  // css parser
#include "Parsers/QuickParser.h" // xhtml parser
#include "Misc/Utility.h"
#include "Parsers/TagAtts.h"  // simple ordered QHash class
#include "Parsers/CSSToolbox.h"


const QString SIGIL_LINK_STYLESHEET_PLACEHOLDER = "<SIGIL_LINK_STYLESHEET_PLACEHOLDER/>\n";


// generate file nameset from list of bookpaths
QSet<QString> CSSToolbox::generate_name_set(const QStringList& bookpaths)
{
    QSet<QString> nameset;
    foreach(const QString &bkpath, bookpaths) {
        QFileInfo fi(bkpath);
        nameset.insert(fi.baseName());
    }
    return nameset;
}


// generate class set from css file contents
QSet<QString> CSSToolbox::generate_class_set(const QString& cssdata)
{
    QSet<QString> classset;
    CSSParser cp;
    cp.set_level("CSS3.0");
    cp.parse_css(cssdata);
    QVector<QString> errors = cp.get_parse_errors();
    for(int i = 0; i < errors.size(); i++) {
        qDebug() << "  CSS Parser Error: " << errors[i] << "\n";
    }
    // now identify all class names
    CSSParser::token atoken = cp.get_next_token();
    while(atoken.type != CSSParser::CSS_END) {
        if (atoken.type == CSSParser::SEL_START && !atoken.data.startsWith('@')) {
            QStringList sels = CSSParser::splitGroupSelector(atoken.data);
            foreach(QString asel, sels) {
                std::pair<int, QString> res = CSSParser::findNextClassInSelector(asel, 0);
                while (res.first != -1) {
                    if (!res.second.isEmpty()) classset.insert(res.second);
                    int p = res.first + res.second.length() + 1;
                    res = CSSParser::findNextClassInSelector(asel, p);
                }
            }
        }
        atoken = cp.get_next_token();
    }
    return classset;
}


// generate a unique filename from a basename and count if needed
QString CSSToolbox::unique_filename(const QString& name, const QSet<QString>& nameset)
{    
    QString bname = name;
    QString ext;
    if (name.contains('.')) {
        QStringList pieces = name.split('.');
        ext = "." + pieces.last();
        pieces.removeLast();
        bname = pieces.join('.');
    }
    int cnt = 0;
    QString uname = bname + ext;
    while(nameset.contains(uname)) {
        uname = bname + QStringLiteral("%1").arg(cnt, 4, 10, QLatin1Char('0')) + ext;
        cnt++;
    }
    return uname;
}


// generate a unique base name (prefix) for classes
QString CSSToolbox::unique_classbase(const QString& name, const QSet<QString>& classset)
{
    QString uname = name;
    int cnt = 0;
    bool unique = false;
    while(!unique) {
        unique = !classset.contains(uname);
        if (unique) {
            // verify it is not an existing prefix of class names
            foreach(const QString& cname, classset) {
                unique = unique & !cname.startsWith(uname);
                if (!unique) break;
            }
            
        }
        if (!unique) {
            uname = "sgc" + QString::number(cnt) + name;
            cnt++;
        }
    }
    return uname;
}


// compare property list tgt to sel
int CSSToolbox::compare_properties(const QHash<QString, QString> &tgt, const QHash<QString, QString> &sel)
{
    int score = 0;
    // merge key lists to get full differences
    QStringList sel_keys = sel.keys();
    QStringList tgt_keys = tgt.keys();
    foreach(QString prop, sel_keys) {
       if (!tgt_keys.contains(prop)) tgt_keys << prop;
    }
    // FIXME handle ignores here or before invoking this routine
    foreach(QString prop, tgt_keys) {
        if (!sel.contains(prop)) score = score + 10;
        if (!tgt.contains(prop)) score = score + 10;
        QString sel_val = sel.value(prop, "");
        QString tgt_val = tgt.value(prop, "");
        if (sel_val != tgt_val) score = score + 1;
    }
    return score;
}


// copy selector from css
QString CSSToolbox::copy_selector_from_css(const QString& sel, const QString &cssdata)
{
    QVector<CSSParser::token> csstokens;
    bool in_selector = false;
    bool in_group = false;
    CSSParser cp;
    cp.set_level("CSS3.0");
    cp.parse_css(cssdata);
    QVector<QString> errors = cp.get_parse_errors();
    for(int i = 0; i < errors.size(); i++) {
        qDebug() << "  CSS Parser Error: " << errors[i] << "\n";
    }
    // now store the sequence of parsed tokens for selector sel
    CSSParser::token atoken = cp.get_next_token();
    while(atoken.type != CSSParser::CSS_END) {
        if (atoken.type == CSSParser::SEL_START && !atoken.data.startsWith('@')) {
            // if matches entire selector or one of the selector group
            if (atoken.data == sel) {
                in_selector = true;
                in_group = false;
            } else {
                QStringList sels = CSSParser::splitGroupSelector(atoken.data);
                if (sels.contains(sel)) { 
                    in_selector = true;
                    in_group = true;
                }
            }
        }
        if (in_selector) {
            CSSParser::token temp;
            temp.pos = atoken.pos;
            temp.line = atoken.line;
            temp.type = atoken.type;
            if (in_group) {
                temp.data = sel;
            } else {
                temp.data = atoken.data;
            }
            csstokens.append(temp);        
        }
        if (atoken.type == CSSParser::SEL_END && !atoken.data.startsWith('@')) {
            if (atoken.data == sel || CSSParser::splitGroupSelector(atoken.data).contains(sel)) {
                in_selector = false;
                in_group = false;
            }
        }
        atoken = cp.get_next_token();
    }
    CSSParser::token temp;
    temp.pos = -1;
    temp.line = -1;
    temp.type = CSSParser::CSS_END;
    temp.data = "";
    csstokens.append(temp);
    CSSParser np;
    np.set_level("CSS3.0");
    np.set_csstokens(csstokens);
    QString ncssdata = np.serialize_css(false);
    return ncssdata;
}


// remove properties from css
QString CSSToolbox::remove_properties_from_css(const QStringList& proplist, const QString &cssdata)
{
    QVector<CSSParser::token> csstokens;
    bool in_selector = false;
    bool remove_property = false;
    CSSParser cp;
    cp.set_level("CSS3.0");
    cp.parse_css(cssdata);
    QVector<QString> errors = cp.get_parse_errors();
    for(int i = 0; i < errors.size(); i++) {
        qDebug() << "  CSS Parser Error: " << errors[i] << "\n";
    }
    // store the sequence of parsed tokens but skip the selector in question
    CSSParser::token atoken = cp.get_next_token();
    while(atoken.type != CSSParser::CSS_END) {
        if (atoken.type == CSSParser::SEL_START && !atoken.data.startsWith('@')) in_selector = true;
        if (atoken.type == CSSParser::SEL_END && !atoken.data.startsWith('@')) in_selector = false;
        if (in_selector && atoken.type == CSSParser::PROPERTY) {
            if (proplist.contains(atoken.data)) remove_property = true;
        }
        if (!remove_property) {
            CSSParser::token temp;
            temp.pos = atoken.pos;
            temp.line = atoken.line;
            temp.type = atoken.type;
            temp.data = atoken.data;
            csstokens.append(temp);        
        }
        if (remove_property && atoken.type == CSSParser::VALUE) {
            remove_property = false;
        }
        atoken = cp.get_next_token();
    }
    CSSParser::token temp;
    temp.pos = -1;
    temp.line = -1;
    temp.type = CSSParser::CSS_END;
    temp.data = "";
    csstokens.append(temp);
    // now recreate the new cssdata
    CSSParser np;
    np.set_level("CSS3.0");
    np.set_csstokens(csstokens);
    QString ncssdata = np.serialize_css(false);
    return ncssdata;
}


// erase selector from css
QString CSSToolbox::erase_selector_from_css(const QString& sel, const QString &cssdata)
{
    QVector<CSSParser::token> csstokens;
    bool in_selector = false;
    bool in_group = false;
    CSSParser cp;
    cp.set_level("CSS3.0");
    cp.parse_css(cssdata);
    QVector<QString> errors = cp.get_parse_errors();
    for(int i = 0; i < errors.size(); i++) {
        qDebug() << "  CSS Parser Error: " << errors[i] << "\n";
    }
    // store the sequence of parsed tokens but skip the selector in question
    // if it is not part of a group
    CSSParser::token atoken = cp.get_next_token();
    while(atoken.type != CSSParser::CSS_END) {
        if (atoken.type == CSSParser::SEL_START && !atoken.data.startsWith('@')) {
            if (atoken.data == sel) {
                in_selector = true;
            } else {
                QStringList sels = CSSParser::splitGroupSelector(atoken.data);
                if (sels.contains(sel)) {
                    sels.removeOne(sel);
                    if (sels.isEmpty()) {
                        in_selector = true;
                    } else {
                        atoken.data = sels.join(",");
                        in_group = true;
                    }
                }
            }
        }
        
        // update atoken.data for changed selectors of a group if needed
        if (atoken.type == CSSParser::SEL_END && !atoken.data.startsWith('@')) {
            if (in_group) {
                QStringList sels = CSSParser::splitGroupSelector(atoken.data);
                sels.removeOne(sel);
                atoken.data = sels.join(",");
            }
        }
        if (!in_selector) {
            CSSParser::token temp;
            temp.pos = atoken.pos;
            temp.line = atoken.line;
            temp.type = atoken.type;
            temp.data = atoken.data;
            csstokens.append(temp);        
        }
        if (atoken.type == CSSParser::SEL_END && !atoken.data.startsWith('@')) {
            in_selector = false;
            in_group = false;
        }
        atoken = cp.get_next_token();
    }
    CSSParser::token temp;
    temp.pos = -1;
    temp.line = -1;
    temp.type = CSSParser::CSS_END;
    temp.data = "";
    csstokens.append(temp);
    // now recreate the new cssdata
    CSSParser np;
    np.set_level("CSS3.0");
    np.set_csstokens(csstokens);
    QString ncssdata = np.serialize_css(false);
    return ncssdata;
}


// replace all occurences of oldclass in selectors with newclass in cssdata
QString CSSToolbox::rename_class_in_css(const QString &oldclass, const QString &newclass, const QString &cssdata)
{
    QVector<CSSParser::token> csstokens;
    CSSParser cp;
    cp.set_level("CSS3.0");
    cp.parse_css(cssdata);
    QVector<QString> errors = cp.get_parse_errors();
    for(int i = 0; i < errors.size(); i++) {
        qDebug() << "  CSS Parser Error: " << errors[i] << "\n";
    }
    // now store the sequence of parsed tokens after making any modifications
    CSSParser::token atoken = cp.get_next_token();
    while(atoken.type != CSSParser::CSS_END) {
        if (atoken.type == CSSParser::SEL_START && !atoken.data.startsWith('@')) {
            QStringList sels = CSSParser::splitGroupSelector(atoken.data);
            QStringList nsels;
            foreach(QString asel, sels) {
                std::pair<int, QString> res = CSSParser::findNextClassInSelector(asel);
                int p = res.first;
                while (res.first != -1) {
                    if (res.second == oldclass) {
                        // replace oldclass with newclass
                        asel = asel.replace(res.first+1, oldclass.length(), newclass);
                        p = res.first + newclass.length() + 1;
                    } else {
                        p = res.first + res.second.length() + 1;
                    }
                    res = CSSParser::findNextClassInSelector(asel, p);
                }
                nsels << asel;
            }
            atoken.data = nsels.join(",");
        }
        CSSParser::token temp;
        temp.pos = atoken.pos;
        temp.line = atoken.line;
        temp.type = atoken.type;
        temp.data = atoken.data;
        csstokens.append(temp);        
        atoken = cp.get_next_token();
    }
    CSSParser::token temp;
    temp.pos = -1;
    temp.line = -1;
    temp.type = CSSParser::CSS_END;
    temp.data = "";
    csstokens.append(temp);
    // now recreate the new cssdata
    CSSParser np;
    np.set_level("CSS3.0");
    np.set_csstokens(csstokens);
    QString ncssdata = np.serialize_css(false);
    return ncssdata;
}


// get linked css hrefs (raw relative hrefs NOT bookpaths)
QStringList CSSToolbox::get_linked_css_hrefs(const QString &source)
{
    QuickParser qp(source);
    QStringList css_hrefs;
    while(true) {
        QuickParser::MarkupInfo mi = qp.parse_next();
        if (mi.pos < 0) break;
        if (mi.text.isEmpty()) {
            if (mi.tname == "link" && mi.tpath.contains(".head")) {
                if (mi.tattr.value("rel", "") == "stylesheet") css_hrefs << mi.tattr.value("href","");
            }
            if ((mi.tname == "head") && (mi.ttype == "end")) break;
        }
    }
    return css_hrefs;
}


// rename class in body of html
QString CSSToolbox::rename_class_in_text(const QString &oldclass, const QString &newclass, const QString &source)
{
    QuickParser qp(source);
    QStringList ndata;
    bool modified = false;
    while(true) {
        QuickParser::MarkupInfo mi = qp.parse_next();
        if (mi.pos < 0) break;
        if (mi.text.isEmpty()) {
            if (mi.ttype == "begin" || mi.ttype == "single") {
                if (mi.tattr.contains("class")) {
                    bool class_changed = false;
                    QStringList cvals = mi.tattr.value("class", "").split(' ');
                    for (int i = 0; i < cvals.size(); i++) {
                        if (cvals[i] == oldclass) {
                            cvals[i] = newclass;
                            class_changed = true;
                        }
                    }
                    if (class_changed) {
                        mi.tattr["class"] = cvals.join(" ");
                        modified = true;
                    }
                }
            }
        }
        ndata << qp.serialize_markup(mi);
    }
    if (modified) return ndata.join("");
    return source;
}


// extract contents of style tags and add placeholder for link
// res.first is extracted stylesheet contents
// res.second is source with style tags removed and link placeholder added
std::pair<QString, QString> CSSToolbox::extract_style_tags(const QString &textdata)
{
    QStringList ndata;
    QStringList stylesheet;
    std::pair<QString, QString> res(QString(), textdata);

    QuickParser qp(textdata);
    bool in_style_tag = false;
    bool used_style_tags = false;
    QString style_indent = "";

    while(true) {
        QuickParser::MarkupInfo mi = qp.parse_next();
        if (mi.pos < 0) break;
        if (mi.text.isEmpty()) {
            if (mi.tname == "style" && mi.ttype == "begin") {
                in_style_tag = true;
                used_style_tags = true;
                style_indent = ndata.last();
            }
            if (mi.tname == "head" && mi.ttype == "end") {
                // inject placeholder for style link
                ndata << style_indent << SIGIL_LINK_STYLESHEET_PLACEHOLDER;
            }
        }     
        if (in_style_tag) {
            // no nesting of html tags allowed inside style tags
            // but do we need to handle cdata here? It should be within a css comment string
            if (!mi.text.isEmpty()) stylesheet << mi.text;
        } else {
            ndata << qp.serialize_markup(mi);
        }

        if (mi.text.isEmpty() && mi.tname == "style" && mi.ttype == "end") {
            in_style_tag = false;
            stylesheet << "\n";
        }
    }

    if (used_style_tags) {
        res.first = Utility::DecodeXML(stylesheet.join(""));
        res.second = ndata.join("");
    }

    return res;
}


// convert inline styles into new class attributes stored in new_rules
// returns modified textdata with css link placeholder injected if inline styles exist
// otherwise returns an empty string;
QString CSSToolbox::extract_inline_styles(const QString& textdata, 
                                        const QString& class_base,
                                        TagAtts& new_rules)
{
    QString rdata;
    int rule_cnt = new_rules.size();
    QuickParser qp(textdata);
    QStringList res;
    bool has_inline_styles = false;
    bool in_svg = false;
    bool in_mathml = false;
    QString link_indent = "";
    while(true) {
        QuickParser::MarkupInfo mi = qp.parse_next();
        if (mi.pos < 0) break;
        if (mi.text.isEmpty()) {
            if (mi.tname == "title" && mi.ttype == "begin") link_indent = res.last();
            // inject a link to the new stylesheet at the head closing tag
            if (mi.tname == "head" && mi.ttype == "end") {
                res << link_indent << SIGIL_LINK_STYLESHEET_PLACEHOLDER;
            }                
            if ((mi.tname == "svg" || mi.tname == "svg:svg") && (mi.ttype == "single" || mi.ttype == "begin")) {
                in_svg = true;
            }
            if ((mi.tname == "math" || mi.tname == "m:math" || mi.tname == "mml:math") &&
                (mi.ttype == "single" || mi.ttype == "begin")) {
                in_mathml = true;
            }
            if ((mi.ttype == "single" || mi.ttype == "begin") && !in_mathml && !in_svg) {
                // strip out any existing style attributes and their values
                QString astyle = mi.tattr.value("style", "");
                if (!astyle.isEmpty()) {
                    has_inline_styles = true;
                    mi.tattr.remove("style");
                    astyle = astyle.trimmed();
                    QString ncls;
                    foreach(QString acls, new_rules.keys()) {
                        QString arule = new_rules.value(acls, "");
                        if (arule == astyle) {
                            ncls = acls;
                            break;
                        }
                    }
                    if (ncls.isEmpty()) {
                        rule_cnt++;
                        ncls = class_base + "_" + QString::number(rule_cnt);
                    }
                    new_rules[ncls] = astyle;

                    // now we need to add in or extend any existing class attribute
                    QString aclass = mi.tattr.value("class", "");
                    if (aclass.isEmpty()) {
                        mi.tattr["class"] = ncls;
                    } else {
                        mi.tattr["class"] = aclass + " " + ncls;
                    }
                }
            }
                
            if ((mi.tname == "svg" || mi.tname == "svg:svg") && (mi.ttype == "single" || mi.ttype == "end")) {
                in_svg = false;
            }
            if ((mi.tname == "math" || mi.tname == "m:math" || mi.tname == "mml:math") &&
                (mi.ttype == "single" || mi.ttype == "end")) {
                in_mathml = false;
            }
        }
        res << qp.serialize_markup(mi);
    }

    // rebuild the xhtml text
    // only output the revised data if inline styles found and removed
    if (has_inline_styles) rdata = res.join("");
    return rdata;

}
