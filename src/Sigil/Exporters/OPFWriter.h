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
#ifndef OPFWRITER_H
#define OPFWRITER_H

#include "XMLWriter.h"

class HTMLResource;

class OPFWriter : private XMLWriter
{

public:

    // Constructor;
    // The first parameter is the book being exported,
    // and the second is the FolderKeeper object representing
    // the folder where the book will be exported
    OPFWriter( QSharedPointer< Book > book, QIODevice &device );

    // Returns the created XML file
    void WriteXML();
    
private:

    // Writes the <metadata> element
    void WriteMetadata();

    // Dispatches each metadata entry based on its type;
    // the specialized Write* functions write the elements
    void MetadataDispatcher(        const QString &metaname, const QVariant &metavalue );

    // Write <creator> and <contributor> metadata elements
    void WriteCreatorOrContributor( const QString &metaname, const QString &metavalue );

    // Writes simple metadata; the metaname will be the element name
    // and the metavalue will be written as the value
    void WriteSimpleMetadata(       const QString &metaname, const QString &metavalue );

    // Writes the <identifier> elements;
    // the metaname will be used for the scheme
    // and the metavalue for the value
    void WriteIdentifier(           const QString &metaname, const QString &metavalue );

    // Writes the <date> elements;
    // the metaname will be used for the event
    // and the metavalue for the value 
    void WriteDate(                 const QString &metaname, const QVariant &metavalue );

    // Takes the reversed form of a name ("Doe, John")
    // and returns the normal form ("John Doe"); if the
    // provided name is already normal, returns an empty string
    static QString GetNormalName( const QString &name );

    /**
     * Creates a valid ID from the requested value.
     * 
     * @param value What the caller wants the ID value to be.
     * @return The potentially modified value to make the ID valid.
     */
    static QString GetValidID( const QString &value );

    // Writes the <manifest> element
    void WriteManifest();

    // Writes the <spine> element
    void WriteSpine();	

    // Writes the <guide> element
    void WriteGuide();

    // Returns true if the text of the HTML resource specified
    // has fewer characters than 'threshold' number. 
    bool IsFlowUnderThreshold( HTMLResource *resource, int threshold ) const;

    // Initializes m_Mimetypes
    void CreateMimetypes();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    QHash<QString, QString> m_Mimetypes;
};

#endif // OPFWRITER_H

