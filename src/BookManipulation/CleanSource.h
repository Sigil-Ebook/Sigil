/************************************************************************
**
**  Copyright (C) 2015 Kevin B. Hendricks Stratford, ON, Canada 
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

#include <QtCore/QList>

#include "ResourceObjects/HTMLResource.h"

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

    static QString CleanGumbo(const QString &source);

    static QString PrettyPrintGumbo(const QString &source);

    static QString XMLPrettyPrintBS4(const QString &source);

    static QString PrettifyDOCTYPEHeader(const QString &source);

    static QString CharToEntity(const QString &source);

    static void ReformatAll(QList <HTMLResource *> resources, QString(clean_fun)(const QString &source));

private:

    static QString PrettyPrint(const QString &source);

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


