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
#ifndef OPFRESOURCE_H
#define OPFRESOURCE_H

#include "XMLResource.h"
#include "BookManipulation/GuideSemantics.h"

// Needed because the moc_* version of this file doesn't include stdafx.h
#include <boost/shared_ptr.hpp>

class HTMLResource;
class ImageResource;
class NCXResource;


class OPFResource : public XMLResource 
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param fullfilepath The full path to the file that this
     *                     resource is representing.
     * @param parent The object's parent.
     */
    OPFResource( const QString &fullfilepath, QObject *parent = NULL );

    // inherited

    virtual bool RenameTo( const QString &new_filename );
    
    virtual ResourceType Type() const;

    GuideSemantics::GuideSemanticType GetGuideSemanticTypeForResource( const Resource &resource ) const;

    int GetReadingOrder( const ::HTMLResource &html_resource ) const;

    QString GetCoverPageOEBPSPath() const;

    QString GetMainIdentifierValue() const;

    // Also creates such an ident if none was found
    QString GetUUIDIdentifierValue();

    void EnsureUUIDIdentifierPresent();

    void UpdateNCXLocationInManifest( const ::NCXResource &ncx );

    void AddSigilVersionMeta();

    bool IsCoverImage( const ::ImageResource &image_resource ) const;

    /**
     * Determines if a cover image exists.
     *
     * @return \c true if a cover image exists.
     */
    bool CoverImageExists() const;    

    void AutoFixWellFormedErrors();

    QStringList GetSpineOrderFilenames() const;

    /**
     * Returns the book's Dublin Core metadata. Note that metadata from
     * <meta> elements is not included.
     *
     * @return The DC metadata, in the same format as the SetDCMetadata metadata parameter.
     */
    QHash< QString, QList< QVariant > > GetDCMetadata() const;

public slots:

    /**
     * Writes metadata to the OPF <metadata> element.
     *
     * @param metadata A hash with meta information about the book.
     *                 The keys are the metadata names, and the values
     *                 are the lists of metadata values for that metadata name.
     */
    void SetDCMetadata( const QHash< QString, QList< QVariant > > &metadata );

    void AddResource( const Resource &resource );

    void RemoveResource( const Resource &resource );

    void AddGuideSemanticType( const ::HTMLResource &html_resource, GuideSemantics::GuideSemanticType new_type );

    void SetResourceAsCoverImage( const ::ImageResource &image_resource );

    void UpdateSpineOrder( const QList< ::HTMLResource* > html_files );

    void ResourceRenamed( Resource *resource, QString old_full_path );

private:

    static void AppendToSpine( const QString &id, xc::DOMDocument &document );

    static void RemoveFromSpine( const QString &id, xc::DOMDocument &document );

    static void UpdateItemrefID( const QString &old_id, const QString &new_id, xc::DOMDocument &document );

    boost::shared_ptr< xc::DOMDocument > GetDocument() const;

    static xc::DOMElement& GetPackageElement( const xc::DOMDocument &document );

    static xc::DOMElement& GetMetadataElement( const xc::DOMDocument &document );

    static xc::DOMElement& GetManifestElement( const xc::DOMDocument &document );
    
    static xc::DOMElement& GetSpineElement( const xc::DOMDocument &document );

    static xc::DOMElement& GetGuideElement( xc::DOMDocument &document );

    // CAN BE NULL! NULL means no reference for resource
    static xc::DOMElement* GetGuideReferenceForResource( 
        const Resource &resource, 
        const xc::DOMDocument &document );

    static void RemoveGuideReferenceForResource( 
        const Resource &resource, 
        xc::DOMDocument &document );

    static GuideSemantics::GuideSemanticType GetGuideSemanticTypeForResource( 
        const Resource &resource, 
        xc::DOMDocument &document );

    static void SetGuideSemanticTypeForResource(
        GuideSemantics::GuideSemanticType type,
        const Resource &resource, 
        xc::DOMDocument &document );

    static void RemoveDuplicateGuideTypes(
        GuideSemantics::GuideSemanticType new_type, 
        xc::DOMDocument &document );

    static QHash< ::HTMLResource*, xc::DOMElement* > GetItemrefsForHTMLResources( 
        const QList< ::HTMLResource* > html_files,
        xc::DOMDocument &document );

    // CAN BE NULL! NULL means no cover meta element
    static xc::DOMElement* GetCoverMeta( const xc::DOMDocument &document );

    static xc::DOMElement& GetMainIdentifier( const xc::DOMDocument &document );

    static xc::DOMElement* GetMainIdentifierUnsafe( const xc::DOMDocument &document );

    static QString GetResourceManifestID( const Resource &resource, const xc::DOMDocument &document );

    static QHash< Resource*, QString > GetResourceManifestIDMapping( 
        const QList< Resource* > resources,
        const xc::DOMDocument &document );

    static void SetMetaElementsLast( xc::DOMDocument &document );

    static void RemoveDCElements( xc::DOMDocument &document );

    /**
     * Dispatches each metadata entry based on its type. 
     * The specialized Write* functions write the elements.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written. 
     * @param document The OPF DOM document.
     */
    static void MetadataDispatcher( const QString &metaname, const QVariant &metavalue, xc::DOMDocument &document );

    /**
     * Writes <creator> and <contributor> metadata elements.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written. 
     * @param document The OPF DOM document.
     */
    static void WriteCreatorOrContributor( const QString &metaname, const QString &metavalue, xc::DOMDocument &document );

    /**
     * Writes simple metadata. 
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written. 
     * @param document The OPF DOM document.
     */
    static void WriteSimpleMetadata( const QString &metaname, const QString &metavalue, xc::DOMDocument &document );

    /**
     * Writes the <identifier> elements.
     * The metaname will be used for the scheme.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written. 
     * @param document The OPF DOM document.
     */
    static void WriteIdentifier( const QString &metaname, const QString &metavalue, xc::DOMDocument &document );

    /**
     * Writes the <date> elements.
     * The metaname will be used for the event.
     *
     * @param metaname The name of the metadata to be written.
     * @param metavalue The value of the metadata to be written. 
     * @param document The OPF DOM document.
     */
    static void WriteDate( const QString &metaname, const QVariant &metavalue, xc::DOMDocument &document );

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

    static bool BasicStructurePresent( const xc::DOMDocument &document );

    boost::shared_ptr< xc::DOMDocument > CreateOPFFromScratch() const;

    QStringList GetRelativePathsToAllFilesInOEPBS() const;

    static QString GetOPFDefaultText();

    void FillWithDefaultText();

    QString GetUniqueID( const QString &preferred_id, const xc::DOMDocument &document ) const;

    QString GetResourceMimetype( const Resource &resource ) const;

    QString GetFileMimetype( const QString &filepath ) const;

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
    QHash< QString, QString > m_Mimetypes;

};

#endif // OPFRESOURCE_H
