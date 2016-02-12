/************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks Stratford, ON, Canada 
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
#ifndef HTMLMETADATA_H
#define HTMLMETADATA_H

#include <QHash>
#include <QObject>

#include "Misc/GumboInterface.h"
#include "ResourceObjects/OPFParser.h"

class QMutex;
class QString;

class HTMLMetadata : public QObject
{
    Q_OBJECT

public:

    static HTMLMetadata *Instance();

    /**
     * Maps DC and <meta> metadata elements to "internal" MetaElements.
     * Accepts both DublinCore metadata elements like one would find in an
     * OPF and custom <meta> elements like one would find in an HTML file.
     *
     * @param element The element to convert.
     * @return The converted MetaElement.
     */
    MetaEntry MapHTMLToOPFMetadata(GumboNode *node, GumboInterface &gi);

private:

    // Constructor is private because
    // this is a singleton class
    HTMLMetadata(){};

    // Converts HTML sourced Dublin Core metadata to OPF style metadata
    MetaEntry HtmlToOpfDC(QString mname, QString mvalue, const QHash<QString,QString>& matts);

    // Converts free form metadata into internal book metadata
    MetaEntry FixupHTMLMetadata(QString name, QString value, const QHash<QString,QString>& matts);


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    static QMutex s_AccessMutex;

    static HTMLMetadata *m_Instance;

};

#endif // HTMLMETADATA_H

