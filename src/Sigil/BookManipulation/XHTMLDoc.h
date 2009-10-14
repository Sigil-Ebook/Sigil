/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#ifndef XHTMLDOC_H
#define XHTMLDOC_H

#include <QDomNodeList>
#include <QString>


class XHTMLDoc
{

public:

    struct XMLElement
    {
        // The name of the element
        QString name;

        // The text of the element
        QString text;

        // The attributes of the element;
        // the keys are the attribute names,
        // the values are the attribute values
        QHash< QString, QString > attributes;
    };

    // Returns a list of XMLElements representing all
    // the elements of the specified tag name
    // in the head section of the provided XHTML source code
    static QList< XMLElement > GetTagsInHead( const QString &source, const QString &tag_name  );

    // Returns a list of XMLElements representing all
    // the elements of the specified tag name
    // in the entire document of the provided XHTML source code
    static QList< XMLElement > GetTagsInDocument( const QString &source, const QString &tag_name );

    // We need to remove the XML carriage returns ("&#xD" sequences)
    // that the default toString() method creates so we wrap it in this function
    static QString GetQDomNodeAsString( const QDomNode &node );

    // Removes all the children of a node and
    // returns that same modified node back.
    // (QDomNodes objects are internally references)
    static QDomNode RemoveChildren( QDomNode node );

private:

    // Accepts a reference to an XML stream reader positioned on an XML element.
    // Returns an XMLElement struct with the data in the stream.
    static XMLElement CreateXMLElement( QXmlStreamReader &reader );
};

#endif // XHTMLDOC_H

