/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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
#ifndef CLEANSOURCE_H
#define CLEANSOURCE_H

#include <boost/tuple/tuple.hpp>

#include <QtCore/QList>

#include "ResourceObjects/HTMLResource.h"

using boost::tuple;

class QStringList;

class CleanSource
{

public:
    // Performs general cleaning (and improving)
    // of provided book XHTML source code
    static QString Clean(const QString &source);

    // Convert to valid XHTML with mild cleaning
    static QString ToValidXHTML(const QString &source);

    static QString ProcessXML(const QString &source);

    static QString CleanBS4(const QString &source);

    static QString PrettyPrintBS4(const QString &source);

    static QString XMLPrettyPrintBS4(const QString &source);

    /**
     * Xerces when serializing into Sigil will always mess with the line feeds and
     * spacing of the DOCTYPE and html declarations at top of the document. This is
     * noticeable with Tidy turned off. This is a workaround to put document back
     * as though that top portion had been run with Prettify turned on.
     */
    static QString PrettifyDOCTYPEHeader(const QString &source);

    static QString CharToEntity(const QString &source);

    static void ReformatAll(QList <HTMLResource *> resources, QString(clean_fun)(const QString &source));

private:


    static QString PrettyPrint(const QString &source);

    static int RobustCSSStyleTagCount(const QString &source);

    // Cleans CSS; currently it removes the redundant CSS classes
    // that Tidy sometimes adds because it doesn't parse existing
    // CSS classes, it only adds new ones; this also merges smaller
    // style tags into larger ones
    static QString CleanCSS(const QString &source, int old_num_styles);

    // Returns the content of all CSS style tags in a list,
    // where each element is a QString representing the content
    // of a single CSS style tag
    static QStringList CSSStyleTags(const QString &source);

    // Removes empty comments that are
    // sometimes left after CDATA comments
    static QStringList RemoveEmptyComments(const QStringList &css_style_tags);

    // Merges smaller styles into bigger ones
    static QStringList MergeSmallerStyles(const QStringList &css_style_tags);

    // Removes blank lines at the top of style tag added by Tidy
    static QString RemoveBlankStyleLines(const QString &source);

    // Returns the largest index of all the Sigil CSS classes
    static int MaxSigilCSSClassIndex(const QStringList &css_style_tags);

    // Writes the new CSS style tags to the source, replacing the old ones
    static QString WriteNewCSSStyleTags(const QString &source, const QStringList &css_style_tags);

    // Removes redundant CSS classes from the style tags and source code;
    // Calls more specific version.
    static tuple<QString, QStringList> RemoveRedundantClasses(const QString &source,
            const QStringList &css_style_tags);

    // Removes redundant CSS classes from the provided CSS style tags
    static QStringList RemoveRedundantClassesTags(const QStringList &css_style_tags,
            const QHash<QString, QString> redundant_classes);

    // Removes redundant CSS classes from the provided XHTML source code;
    // Updates references to older classes that do the same thing
    static QString RemoveRedundantClassesSource(const QString &source,
            const QHash<QString, QString> redundant_classes);

    // Returns a QHash with keys being the new redundant CSS classes,
    // and the values the old classes that already do the job of the new ones.
    static QHash<QString, QString> GetRedundantClasses(const QStringList &css_style_tags);

    /**
     * Removes HTML meta tags with charset declarations.
     * Some applications leave a faulty charset encoding
     * even when the XML encoding is different. This makes
     * the epub invalid since no HTML file can have 2 encodings.
     * Sigil will specify UTF-8 in the XML declaration on export,
     * so this meta tag is useless anyway.
     */
    static QString RemoveMetaCharset(const QString &source);

    /**
     * Additional pre-processing for special cases that aren't handled properly
     * by HTML Tidy.
     *
     * @param The source code to be cleaned
     * @return The processed code to be fed to HTML Tidy.
     */
    static QString PreprocessSpecialCases(const QString &source);
};


#endif // CLEANSOURCE_H


