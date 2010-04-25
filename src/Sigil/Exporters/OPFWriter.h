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

/**
 * Writes the OPF file of the EPUB publication.
 */
class OPFWriter : private XMLWriter
{

public:

    /**
     * Constructor.
     *
     * @param book The book for which we're writing the OPF.
     * @param device The IODevice into which we should write the XML.
     */
    OPFWriter( QSharedPointer< Book > book, QIODevice &device );

    void WriteXML();
    
private:

    /**
     *  Writes the <metadata> element.
     */
    void WriteMetadata();

    /**
     * Dispatches each metadata entry based on its type. 
     * The specialized Write* functions write the elements.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written. 
     */
    void MetadataDispatcher(        const QString &metaname, const QVariant &metavalue );

    /**
     * Writes <creator> and <contributor> metadata elements.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written. 
     */
    void WriteCreatorOrContributor( const QString &metaname, const QString &metavalue );

    /**
     * Writes simple metadata. 
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written. 
     */
    void WriteSimpleMetadata(       const QString &metaname, const QString &metavalue );

    /**
     * Writes the <identifier> elements.
     * The metaname will be used for the scheme.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written. 
     */
    void WriteIdentifier(           const QString &metaname, const QString &metavalue );

    /**
     * Writes the <date> elements.
     * The metaname will be used for the event.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written. 
     */
    void WriteDate(                 const QString &metaname, const QVariant &metavalue );

    void WriteCoverImageMeta();

    /**
     * Writes a meta element with the version of Sigil
     * that wrote this OPF file.
     */
    void WriteSigilVersionMeta();

    /**
     * Takes the reversed form of a name ("Doe, John")
     * and returns the normal form ("John Doe"). If the
     * provided name is already normal, returns an empty string
     *
     * @param name The name in reversed form.
     * @return The normalized name, or an empty string if the name 
     *         was already normalized.
     */
    static QString GetNormalName( const QString &name );

    /**
     * Creates a valid ID from the requested value.
     * 
     * @param value What the caller wants the ID value to be.
     * @return The potentially modified value to make the ID valid.
     */
    static QString GetValidID( const QString &value );

    /**
     * Determines if the provided character can appear
     * in an XML ID attribute.
     *
     * @param character The character that needs to be checked.
     * @return True if the character is valid.
     */
    static bool IsValidIDCharacter( const QChar &character );

    /**
     * Writes the <manifest> element.
     */
    void WriteManifest();

    /**
     * Writes the <spine> element.
     */
    void WriteSpine();	

    /**
     * Writes the <guide> element.
     */
    void WriteGuide();

    /**
     * Determines the presence of <guide> semantic information
     * in the HTMLResources.
     *
     * @return \c true if the information is present.
     */
    bool GuideTypesPresent();

    /**
     * Initializes m_Mimetypes.
     */
    void CreateMimetypes();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * A mapping between file extensions
     * and appropriate MIME types.
     */
    QHash<QString, QString> m_Mimetypes;
};

#endif // OPFWRITER_H

