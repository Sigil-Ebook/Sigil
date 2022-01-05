/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford Ontario Canada
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

#pragma once
#ifndef CSSTOOLBOX_H
#define CSSTOOLBOX_H

#include <QString>
#include <QSet>
#include <QHash>
#include <QStringList>

class TagAtts;

class CSSToolbox
{
    Q_DECLARE_TR_FUNCTIONS(CSSToolbox)

public:


    // const QString SIGIL_LINK_STYLESHEET_PLACEHOLDER = "<SIGIL_LINK_STYLESHEET_PLACEHOLDER/>\n";

    // generate file nameset from list of bookpaths
    static QSet<QString> generate_name_set(const QStringList& bookpaths);


    // generate set of class names from css file contents
    static QSet<QString>generate_class_set(const QString& cssdata);


    // generate a unique filename from a basename and count if needed
    static QString unique_filename(const QString& name, const QSet<QString>& nameset);


    // generate a unique base name (prefix) for classes
    static QString unique_classbase(const QString& name, const QSet<QString>& classset);


    // compare property list tgt selector to sel selector and return difference score
    static int compare_properties(const QHash<QString, QString> &tgt, const QHash<QString, QString> &sel);


    // copy selector from css
    static QString copy_selector_from_css(const QString& sel, const QString &cssdata);


    // remove properties named in proplist from all selectors in css
    static QString remove_properties_from_css(const QStringList& proplist, const QString &cssdata);


    // erase selector sel from css - must match full selector exactly
    static QString erase_selector_from_css(const QString& sel, const QString &cssdata);


    // replace all occurences of oldclass in selectors with newclass in cssdata
    static QString rename_class_in_css(const QString &oldclass, const QString &newclass, const QString &cssdata);


    // get linked css *hrefs* from xhtml source
    static QStringList get_linked_css_hrefs(const QString &source);


    // rename class in body of html source from oldclass to newclass
    static QString rename_class_in_text(const QString &oldclass, const QString &newclass, const QString &source);


    // extract contents of style tags and add placeholder for link
    // res.first is extracted stylesheet contents
    // res.second is source with style tags removed and link placeholder added
    static std::pair<QString, QString> extract_style_tags(const QString &textdata);


    // convert inline style attributes from xhtml source to classes
    // returns the new textdata with injected css stylesheet placeholder if inline styles exist
    // otherwise returns an empty string
    static QString extract_inline_styles(const QString& textdata, 
                                         const QString& class_base,
                                         TagAtts& new_rules);

};
#endif // CSSTOOLBOX_H
