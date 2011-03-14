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

#include <stdafx.h>
#include "OPFResource.h"
#include "HTMLResource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/Metadata.h"
#include "Misc/Utility.h"
#include <XmlUtils.h>

namespace xe = XercesExt;

static const QString OPF_XML_NAMESPACE        = "http://www.idpf.org/2007/opf"; 
static const QString FALLBACK_MIMETYPE        = "text/plain";
static const QString ITEM_ELEMENT_TEMPLATE    = "<item id=\"%1\" href=\"%2\" media-type=\"%3\"/>";
static const QString ITEMREF_ELEMENT_TEMPLATE = "<itemref idref=\"%1\"/>";
static const QString OPF_REWRITTEN_COMMENT    = "<!-- Your OPF file was broken so Sigil "
                                                "was forced to create a new one from scratch. -->";

static const QString TEMPLATE_TEXT = 
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<package version=\"2.0\" xmlns=\"http://www.idpf.org/2007/opf\" unique-identifier=\"BookId\">\n\n"
    "  <metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:opf=\"http://www.idpf.org/2007/opf\">\n"
    "    <dc:identifier opf:scheme=\"UUID\" id=\"BookId\">urn:uuid:%1</dc:identifier>\n"
    "  </metadata>\n\n"
    "  <manifest>\n"
    "    <item id=\"ncx\" href=\"toc.ncx\" media-type=\"application/x-dtbncx+xml\"/>\n"    
    "  </manifest>\n\n"
    "  <spine toc=\"ncx\">\n"
    "  </spine>\n\n"
    "</package>";


OPFResource::OPFResource( const QString &fullfilepath, QObject *parent )
    : XMLResource( fullfilepath, parent )
{
    CreateMimetypes();
    FillWithDefaultText();

    // Make sure the file exists on disk.
    // Among many reasons, this also solves the problem
    // with the Book Browser not displaying an icon for this resource.
    SaveToDisk();
}


bool OPFResource::RenameTo( const QString &new_filename )
{
    // The user is not allowed to rename the OPF file.
    return false;
}


Resource::ResourceType OPFResource::Type() const
{
    return Resource::OPFResource;
}


GuideSemantics::GuideSemanticType OPFResource::GetGuideSemanticTypeForResource( const Resource &resource ) const
{
    QReadLocker locker( &GetLock() );
    shared_ptr< xc::DOMDocument > document = GetDocument();
    return GetGuideSemanticTypeForResource( resource, *document );
}


int OPFResource::GetReadingOrder( const ::HTMLResource &html_resource ) const
{
    QReadLocker locker( &GetLock() );
    shared_ptr< xc::DOMDocument > document = GetDocument();

    const Resource &resource = *static_cast< const Resource* >( &html_resource );
    QString resource_id = GetResourceManifestID( resource, *document );

    QList< xc::DOMElement* > itemrefs = 
        XhtmlDoc::GetTagMatchingDescendants( *document, "itemref", OPF_XML_NAMESPACE );

    for ( int i = 0; i < itemrefs.count(); ++i )
    {
        QString idref = XtoQ( itemrefs[ i ]->getAttribute( QtoX( "idref" ) ) );

        if ( resource_id == idref )

            return i;
    }

    return -1;
}


QString OPFResource::GetCoverPageOEBPSPath() const
{
    QReadLocker locker( &GetLock() );
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QList< xc::DOMElement* > references = 
        XhtmlDoc::GetTagMatchingDescendants( *document, "reference", OPF_XML_NAMESPACE );

    foreach( xc::DOMElement* reference, references )
    {
        QString type_text = XtoQ( reference->getAttribute( QtoX( "type" ) ) );
        GuideSemantics::GuideSemanticType current_type =
            GuideSemantics::Instance().MapReferenceTypeToGuideEnum( type_text );

        if ( current_type == GuideSemantics::Cover )
        {
            XtoQ( reference->getAttribute( QtoX( "href" ) ) );              
        }        
    }

    return QString();
}


QString OPFResource::GetMainIdentifierValue() const
{
    QReadLocker locker( &GetLock() );
    shared_ptr< xc::DOMDocument > document = GetDocument();
    return XtoQ( GetMainIdentifier( *document ).getTextContent() ).remove( "urn:uuid:" );
}


QString OPFResource::GetUUIDIdentifierValue()
{
    EnsureUUIDIdentifierPresent();

    QReadLocker locker( &GetLock() );
    shared_ptr< xc::DOMDocument > document = GetDocument();

    QList< xc::DOMElement* > identifiers = 
        XhtmlDoc::GetTagMatchingDescendants( *document, "identifier", DUBLIN_CORE_NS );

    foreach( xc::DOMElement *identifier, identifiers )
    {
        QString value = XtoQ( identifier->getTextContent() ).remove( "urn:uuid:" );

        if ( !QUuid( value ).isNull() )
        {
            return value;
        }
    }

    // EnsureUUIDIdentifierPresent should ensure we 
    // never reach here.
    Q_ASSERT( false );
    return QString();
}


void OPFResource::EnsureUUIDIdentifierPresent()
{
    QWriteLocker locker( &GetLock() );
    shared_ptr< xc::DOMDocument > document = GetDocument();

    QList< xc::DOMElement* > identifiers = 
        XhtmlDoc::GetTagMatchingDescendants( *document, "identifier", DUBLIN_CORE_NS );

    foreach( xc::DOMElement *identifier, identifiers )
    {
        QString value = XtoQ( identifier->getTextContent() ).remove( "urn:uuid:" );

        if ( !QUuid( value ).isNull() )
        {
            return;
        }
    }
    
    QString uuid = Utility::CreateUUID();

    WriteIdentifier( "UUID", uuid, *document );
    UpdateTextFromDom( *document );    
}


// TODO: only accept ImageResource
bool OPFResource::IsCoverImage( const Resource &resource ) const
{
    if ( resource.Type() != ImageResource )

        return false;

    QReadLocker locker( &GetLock() );

    shared_ptr< xc::DOMDocument > document = GetDocument();
    xc::DOMElement* meta                   = GetCoverMeta( *document );    
    
    if ( meta )
    {
        QString resource_id = GetResourceManifestID( resource, *document );

        return XtoQ( meta->getAttribute( QtoX( "content" ) ) ) == resource_id;
    }

    return false;
}


bool OPFResource::CoverImageExists() const
{
    QReadLocker locker( &GetLock() );

    shared_ptr< xc::DOMDocument > document = GetDocument();
    
    return GetCoverMeta( *document ) != NULL;
}


void OPFResource::AutoFixWellFormedErrors()
{
    QWriteLocker locker( &GetLock() );

    UpdateTextFromDom( *CreateOPFFromScratch() );
}


QStringList OPFResource::GetSpineOrderFilenames() const
{
    QReadLocker locker( &GetLock() );
    shared_ptr< xc::DOMDocument > document = GetDocument();

    QList< xc::DOMElement* > items = 
        XhtmlDoc::GetTagMatchingDescendants( *document, "item", OPF_XML_NAMESPACE );

    QHash< QString, QString > id_to_filename_mapping;

    foreach( xc::DOMElement* item, items )
    {
        QString id   = XtoQ( item->getAttribute( QtoX( "id" ) ) );
        QString href = XtoQ( item->getAttribute( QtoX( "href" ) ) );

        id_to_filename_mapping[ id ] = QFileInfo( href ).fileName();
    }

    QList< xc::DOMElement* > itemrefs = 
        XhtmlDoc::GetTagMatchingDescendants( *document, "itemref", OPF_XML_NAMESPACE );

    QStringList filenames_in_reading_order;
    
    foreach( xc::DOMElement* itemref, itemrefs )
    {
        QString idref = XtoQ( itemref->getAttribute( QtoX( "idref" ) ) );

        if ( id_to_filename_mapping.contains( idref ) )

           filenames_in_reading_order.append( id_to_filename_mapping[ idref ] );
    }

    return filenames_in_reading_order;
}



QHash< QString, QList< QVariant > > OPFResource::GetDCMetadata() const
{
    QReadLocker locker( &GetLock() );
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QList< xc::DOMElement* > dc_elements = 
        XhtmlDoc::GetTagMatchingDescendants( *document, "*", DUBLIN_CORE_NS );

    QHash< QString, QList< QVariant > > metadata;

    foreach( xc::DOMElement *dc_element, dc_elements )
    {
        Metadata::MetaElement book_meta = Metadata::Instance().MapToBookMetadata( *dc_element );

        if ( !book_meta.name.isEmpty() && !book_meta.value.toString().isEmpty() )
        {
            metadata[ book_meta.name ].append( book_meta.value );
        }
    }

    return metadata;
}


void OPFResource::SetDCMetadata( const QHash< QString, QList< QVariant > > &metadata )
{
    QWriteLocker locker( &GetLock() );
    shared_ptr< xc::DOMDocument > document = GetDocument();

    RemoveDCElements( *document );

    foreach ( QString name, metadata.keys() )
    {
        foreach ( QVariant single_value, metadata[ name ] )
        {
            MetadataDispatcher( name, single_value, *document );
        }
    }

    SetMetaElementsLast( *document );

    UpdateTextFromDom( *document );
}


void OPFResource::AddResource( const Resource &resource )
{
    QWriteLocker locker( &GetLock() );

    QHash< QString, QString > attributes;
    attributes[ "id"         ] = GetValidID( resource.Filename() );
    attributes[ "href"       ] = resource.GetRelativePathToOEBPS();
    attributes[ "media-type" ] = GetResourceMimetype( resource );

    shared_ptr< xc::DOMDocument > document = GetDocument();

    xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument( 
        "item", OPF_XML_NAMESPACE, *document, attributes );

    xc::DOMElement &manifest = GetManifestElement( *document );
    manifest.appendChild( new_item );

    if ( resource.Type() == Resource::HTMLResource )

        AppendToSpine( attributes[ "id" ], *document );

    UpdateTextFromDom( *document );
}


void OPFResource::RemoveResource( const Resource &resource )
{
    QWriteLocker locker( &GetLock() );

    shared_ptr< xc::DOMDocument > document  = GetDocument();
    xc::DOMElement &manifest                = GetManifestElement( *document );
    std::vector< xc::DOMElement* > children = xe::GetElementChildren( manifest );
    QString resource_oebps_path             = resource.GetRelativePathToOEBPS();
    QString item_id;

    foreach( xc::DOMElement *child, children )
    {
        QString href = XtoQ( child->getAttribute( QtoX( "href" ) ) );
        
        if ( href == resource_oebps_path )
        {
            item_id = XtoQ( child->getAttribute( QtoX( "id" ) ) );
            manifest.removeChild( child );
            break;
        }
    }

    if ( resource.Type() == Resource::HTMLResource )

        RemoveFromSpine( item_id, *document );

    UpdateTextFromDom( *document );
}


// TODO: only accept HTMLResources
void OPFResource::AddGuideSemanticType( const Resource &resource, GuideSemantics::GuideSemanticType new_type )
{
    QWriteLocker locker( &GetLock() );

    shared_ptr< xc::DOMDocument > document         = GetDocument();
    GuideSemantics::GuideSemanticType current_type = GetGuideSemanticTypeForResource( resource, *document );
       
    if ( current_type != new_type )
    {
        RemoveDuplicateGuideTypes( new_type, *document );
        SetGuideSemanticTypeForResource( new_type, resource, *document );
    }

    // If the current type is the same as the new one,
    // we toggle it off.
    else 
    {
        RemoveGuideReferenceForResource( resource, *document );
    }

    UpdateTextFromDom( *document );
}

// TODO: only accept ImageResources
void OPFResource::SetResourceAsCoverImage( const Resource &resource )
{
    QWriteLocker locker( &GetLock() );

    shared_ptr< xc::DOMDocument > document = GetDocument();
    xc::DOMElement* meta = GetCoverMeta( *document );
    QString resource_id = GetResourceManifestID( resource, *document );
    
    if ( meta )
    {
        // If the image is already set as the cover, then we toggle it off
        if ( XtoQ( meta->getAttribute( QtoX( "content" ) ) ) == resource_id )
        
            GetMetadataElement( *document ).removeChild( meta );
        
        else
        
            meta->setAttribute( QtoX( "content" ), QtoX( resource_id ) );        
    }

    else
    {
        QHash< QString, QString > attributes;
        attributes[ "name"    ] = "cover";
        attributes[ "content" ] = resource_id;

        xc::DOMElement *new_meta = XhtmlDoc::CreateElementInDocument( 
            "meta", OPF_XML_NAMESPACE, *document, attributes );

        xc::DOMElement &metadata = GetMetadataElement( *document );
        metadata.appendChild( new_meta );
    }

    UpdateTextFromDom( *document );
}


void OPFResource::UpdateSpineOrder( const QList< ::HTMLResource* > html_files )
{
    QWriteLocker locker( &GetLock() );
    shared_ptr< xc::DOMDocument > document = GetDocument();

    QHash< ::HTMLResource*, xc::DOMElement* > itemref_mapping =
        GetItemrefsForHTMLResources( html_files, *document );

    xc::DOMElement &spine = GetSpineElement( *document );

    XhtmlDoc::RemoveChildren( spine );

    foreach( ::HTMLResource* resource, html_files )
    {
        xc::DOMElement* itemref = itemref_mapping.value( resource, NULL );

        if ( itemref )

            spine.appendChild( itemref );
    }

    UpdateTextFromDom( *document );
}


void OPFResource::AppendToSpine( const QString &id, xc::DOMDocument &document )
{
    QHash< QString, QString > attributes;
    attributes[ "idref" ] = id;

    xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument(
        "itemref", OPF_XML_NAMESPACE, document, attributes );

    xc::DOMElement &spine = GetSpineElement( document );
    spine.appendChild( new_item );
}


void OPFResource::RemoveFromSpine( const QString &id, xc::DOMDocument &document )
{
    xc::DOMElement &spine = GetSpineElement( document );
    std::vector< xc::DOMElement* > children = xe::GetElementChildren( spine );

    foreach( xc::DOMElement *child, children )
    {
        QString idref = XtoQ( child->getAttribute( QtoX( "idref" ) ) );
        
        if ( idref == id )
        {
            spine.removeChild( child );
            break;
        }
    }
}


shared_ptr< xc::DOMDocument > OPFResource::GetDocument() const
{
    shared_ptr< xc::DOMDocument > document = XhtmlDoc::LoadTextIntoDocument( m_TextDocument->toPlainText() );

    if ( !BasicStructurePresent( *document ) )

        document = CreateOPFFromScratch();

    // For NCX files, the default of standalone == false should remain
    document->setXmlStandalone( true );
    return document;
}


xc::DOMElement& OPFResource::GetPackageElement( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > packages =
        XhtmlDoc::GetTagMatchingDescendants( document, "package", OPF_XML_NAMESPACE );
    Q_ASSERT( !packages.isEmpty() );
    
    return *packages[ 0 ];
}


xc::DOMElement& OPFResource::GetMetadataElement( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > metadatas =
        XhtmlDoc::GetTagMatchingDescendants( document, "metadata", OPF_XML_NAMESPACE );
    Q_ASSERT( !metadatas.isEmpty() );

    return *metadatas[ 0 ];
}


xc::DOMElement& OPFResource::GetManifestElement( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > manifests =
        XhtmlDoc::GetTagMatchingDescendants( document, "manifest", OPF_XML_NAMESPACE );
    Q_ASSERT( !manifests.isEmpty() );

    return *manifests[ 0 ];    
}


xc::DOMElement& OPFResource::GetSpineElement( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > spines = 
        XhtmlDoc::GetTagMatchingDescendants( document, "spine", OPF_XML_NAMESPACE );
    Q_ASSERT( !spines.isEmpty() );

    return *spines[ 0 ];    
}


xc::DOMElement& OPFResource::GetGuideElement( xc::DOMDocument &document )
{
    QList< xc::DOMElement* > guides = 
        XhtmlDoc::GetTagMatchingDescendants( document, "guide", OPF_XML_NAMESPACE );
    
    if ( !guides.isEmpty() )

        return *guides[ 0 ];

    xc::DOMElement *guide = XhtmlDoc::CreateElementInDocument(
        "guide", OPF_XML_NAMESPACE, document, QHash< QString, QString >() );

    xc::DOMElement &package = *document.getDocumentElement();
    package.appendChild( guide );

    return *guide;
}


xc::DOMElement* OPFResource::GetGuideReferenceForResource( const Resource &resource, const xc::DOMDocument &document )
{
    QString resource_oebps_path         = resource.GetRelativePathToOEBPS();
    QList< xc::DOMElement* > references = 
        XhtmlDoc::GetTagMatchingDescendants( document, "reference", OPF_XML_NAMESPACE );

    foreach( xc::DOMElement* reference, references )
    {
        QString href = XtoQ( reference->getAttribute( QtoX( "href" ) ) );

        if ( href == resource_oebps_path )
        {
            return reference;              
        }        
    }

    return NULL;
}


void OPFResource::RemoveGuideReferenceForResource( const Resource &resource, xc::DOMDocument &document )
{
    xc::DOMElement &guide = GetGuideElement( document );
    guide.removeChild( GetGuideReferenceForResource( resource, document ) );
}


GuideSemantics::GuideSemanticType OPFResource::GetGuideSemanticTypeForResource( 
    const Resource &resource,
    xc::DOMDocument &document )
{
    xc::DOMElement* reference = GetGuideReferenceForResource( resource, document );
    
    if ( reference )
    {
        QString type = XtoQ( reference->getAttribute( QtoX( "type" ) ) );
        return GuideSemantics::Instance().MapReferenceTypeToGuideEnum( type );  
    }

    return GuideSemantics::NoType;
}


void OPFResource::SetGuideSemanticTypeForResource( 
    GuideSemantics::GuideSemanticType type, 
    const Resource &resource, 
    xc::DOMDocument &document )
{
    xc::DOMElement* reference = GetGuideReferenceForResource( resource, document );
    QString type_attribute;
    QString title_attribute;
    tie( type_attribute, title_attribute ) = GuideSemantics::Instance().GetGuideTypeMapping()[ type ];
    
    if ( reference )
    {
        reference->setAttribute( QtoX( "type" ), QtoX( type_attribute ) ); 
    }

    else
    {

        QHash< QString, QString > attributes;
        attributes[ "type"  ] = type_attribute;
        attributes[ "title" ] = title_attribute;
        attributes[ "href"  ] = resource.GetRelativePathToOEBPS();

        xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument( 
            "reference", OPF_XML_NAMESPACE, document, attributes );

        xc::DOMElement &guide = GetGuideElement( document );
        guide.appendChild( new_item );
    }
}


void OPFResource::RemoveDuplicateGuideTypes( 
    GuideSemantics::GuideSemanticType new_type, 
    xc::DOMDocument &document )
{
    // Industry best practice is to have only one 
    // <guide> reference type instance per book.
    // The only exception is the Text type, of which  
    // we customarily have more than one instance.
    // For NoType, there is nothing to remove.
    if ( new_type == GuideSemantics::Text || new_type == GuideSemantics::NoType )

        return;

    xc::DOMElement &guide               = GetGuideElement( document );
    QList< xc::DOMElement* > references = 
        XhtmlDoc::GetTagMatchingDescendants( document, "reference", OPF_XML_NAMESPACE );

    foreach( xc::DOMElement* reference, references )
    {
        QString type_text = XtoQ( reference->getAttribute( QtoX( "type" ) ) );
        GuideSemantics::GuideSemanticType current_type =
            GuideSemantics::Instance().MapReferenceTypeToGuideEnum( type_text );   

        if ( current_type == new_type )
        {
            guide.removeChild( reference );

            // There is no "break" statement here because we might
            // load an epub that has several instance of one guide type.
            // We preserve them on load, but if the user is intent on
            // changing them, then we enforce "one type instance per book".
        }        
    }
}

// If there is no itemref for the resource, one will be created (but NOT
// attached to the spine element!).
// Also, it's possible that a NULL will be set as an itemref for a resource
// if that resource doesns't have an entry in the manifest.
QHash< ::HTMLResource*, xc::DOMElement* > OPFResource::GetItemrefsForHTMLResources( 
    const QList< ::HTMLResource* > html_files, 
    xc::DOMDocument &document )
{
    QList< xc::DOMElement* > itemrefs = 
        XhtmlDoc::GetTagMatchingDescendants( document, "itemref", OPF_XML_NAMESPACE );

    QList< Resource* > resource_list;
    foreach( ::HTMLResource* html_resource, html_files )
    {
        resource_list.append( static_cast< Resource* >( html_resource ) );
    }

    QHash< Resource*, QString > id_mapping = GetResourceManifestIDMapping( resource_list, document );

    QList< Resource* > htmls_without_itemrefs;
    QHash< ::HTMLResource*, xc::DOMElement* > itmeref_mapping;

    foreach( Resource* resource, resource_list )
    {
        ::HTMLResource* html_resource = qobject_cast< ::HTMLResource* >( resource );
        QString resource_id = id_mapping.value( resource, "" );

        foreach( xc::DOMElement* itemref, itemrefs )
        {
            QString idref = XtoQ( itemref->getAttribute( QtoX( "idref" ) ) );

            if ( idref == resource_id )
            {
                itmeref_mapping[ html_resource ] = itemref;
                break;
            }
        }

        if ( !itmeref_mapping.contains( html_resource ) )

            htmls_without_itemrefs.append( resource );
    }

    foreach( Resource* resource, htmls_without_itemrefs )
    {
        QHash< QString, QString > attributes;
        QString resource_id = id_mapping.value( resource, "" );
        ::HTMLResource* html_resource = qobject_cast< ::HTMLResource* >( resource );

        if ( resource_id.isEmpty() )

            itmeref_mapping[ html_resource ] = NULL;

        attributes[ "idref" ] = resource_id;

        xc::DOMElement *new_itemref = XhtmlDoc::CreateElementInDocument(
            "itemref", OPF_XML_NAMESPACE, document, attributes );

        itmeref_mapping[ html_resource ] = new_itemref;
    }

    return itmeref_mapping;
}


xc::DOMElement* OPFResource::GetCoverMeta( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > metas =
        XhtmlDoc::GetTagMatchingDescendants( document, "meta", OPF_XML_NAMESPACE );

    foreach( xc::DOMElement* meta, metas )
    {
        QString name = XtoQ( meta->getAttribute( QtoX( "name" ) ) );
        
        if ( name == "cover" )
        {
            return meta;
        }
    }

    return NULL;
}


xc::DOMElement& OPFResource::GetMainIdentifier( const xc::DOMDocument &document )
{
    xc::DOMElement* identifier = GetMainIdentifierUnsafe( document );
    Q_ASSERT( identifier );

    return *identifier;
}


// This is here because we use it to check for the presence of the main identifier
// during DOM validation. But after that, the GetMainIdentifier func should be used.
xc::DOMElement* OPFResource::GetMainIdentifierUnsafe( const xc::DOMDocument &document )
{
    xc::DOMElement &package = GetPackageElement( document );
    QString unique_identifier = XtoQ( package.getAttribute( QtoX( "unique-identifier" ) ) );

    QList< xc::DOMElement* > identifiers = 
        XhtmlDoc::GetTagMatchingDescendants( document, "identifier", DUBLIN_CORE_NS );

    foreach( xc::DOMElement *identifier, identifiers )
    {
        QString id = XtoQ( identifier->getAttribute( QtoX( "id" ) ) );

        if ( id == unique_identifier )

            return identifier;
    }

    return NULL;
}

QString OPFResource::GetResourceManifestID( const Resource &resource, const xc::DOMDocument &document )
{
    QString oebps_path = resource.GetRelativePathToOEBPS();
    QList< xc::DOMElement* > items = 
        XhtmlDoc::GetTagMatchingDescendants( document, "item", OPF_XML_NAMESPACE );

    foreach( xc::DOMElement* item, items )
    {
        QString href = XtoQ( item->getAttribute( QtoX( "href" ) ) );

        if ( href == oebps_path )

            return XtoQ( item->getAttribute( QtoX( "id" ) ) );
    }

    return QString();
}


QHash< Resource*, QString > OPFResource::GetResourceManifestIDMapping( 
    const QList< Resource* > resources, 
    const xc::DOMDocument &document )
{
    QHash< Resource*, QString > id_mapping;

    QList< xc::DOMElement* > items = 
        XhtmlDoc::GetTagMatchingDescendants( document, "item", OPF_XML_NAMESPACE );

    foreach( Resource* resource, resources )
    {
        QString oebps_path = resource->GetRelativePathToOEBPS();        

        foreach( xc::DOMElement* item, items )
        {
            QString href = XtoQ( item->getAttribute( QtoX( "href" ) ) );

            if ( href == oebps_path )
            {
                id_mapping[ resource ] = XtoQ( item->getAttribute( QtoX( "id" ) ) );
                break;
            }
        }
    }

    return id_mapping;
}


void OPFResource::SetMetaElementsLast( xc::DOMDocument &document )
{
    // TODO: this should probably be SetNonDCElementsLast,
    // and then work that way too.

    QList< xc::DOMElement* > metas = 
        XhtmlDoc::GetTagMatchingDescendants( document, "meta", OPF_XML_NAMESPACE );
    xc::DOMElement &metadata = GetMetadataElement( document );

    foreach( xc::DOMElement* meta, metas )
    {
        // This makes sure that the <meta> elements come last
        metadata.removeChild( meta );
        metadata.appendChild( meta );
    }
}


void OPFResource::RemoveDCElements( xc::DOMDocument &document )
{
    QList< xc::DOMElement* > dc_elements = XhtmlDoc::GetTagMatchingDescendants( document, "*", DUBLIN_CORE_NS );
    xc::DOMElement &main_identifier = GetMainIdentifier( document );

    foreach( xc::DOMElement *dc_element, dc_elements )
    {
        // We preserve the original main identifier. Users
        // complain when we don't.
        if ( dc_element->isSameNode( &main_identifier ) )

            continue;

        xc::DOMNode *parent = dc_element->getParentNode();

        if ( parent )

            parent->removeChild( dc_element );
    }
}


void OPFResource::MetadataDispatcher(
    const QString &metaname, 
    const QVariant &metavalue,
    xc::DOMDocument &document )
{
    // We ignore badly formed meta elements.
    if ( metaname.isEmpty() || metavalue.isNull() )

        return;

    // There is a relator for the publisher, but there is
    // also a special publisher element that we would rather use
    if (  Metadata::Instance().GetRelatorMap().contains( metaname ) &&
          metaname != QObject::tr( "Publisher" )
       )
    {
        WriteCreatorOrContributor( metaname, metavalue.toString(), document );
    }

    else if ( metaname == QObject::tr( "Language" ) )
    {
        WriteSimpleMetadata( metaname.toLower(), 
                             Metadata::Instance().GetLanguageMap()[ metavalue.toString() ],
                             document );
    }

    else if ( ( metaname == QObject::tr( "ISBN" ) ) || 
              ( metaname == QObject::tr( "ISSN" ) ) ||
              ( metaname == QObject::tr( "DOI" ) )
            )
    {
        WriteIdentifier( metaname, metavalue.toString(), document );
    }

    else if ( metaname == QObject::tr( "CustomID" ) )
    {
        // FIXME: this should work in opfresource
        // Don't write the CustomID, it is used as the
        // main identifier if present
    }

    else if ( metaname.contains( QObject::tr( "Date" ) ) )
    {
        WriteDate( metaname, metavalue, document );		
    }
    
    // Everything else should be simple
    else
    {
        WriteSimpleMetadata( metaname.toLower(), metavalue.toString(), document );
    }
}


void OPFResource::WriteCreatorOrContributor( 
    const QString &metaname, 
    const QString &metavalue, 
    xc::DOMDocument &document )
{
    // Authors get written as creators, all other relators
    // are written as contributors
    QString element_name = metaname == QObject::tr( "Author" ) ? "creator" : "contributor";
    QString role = Metadata::Instance().GetRelatorMap()[ metaname ].relator_code;
    QString value;
    QString file_as;

    // if the name is written in standard form 
    // ("John Doe"), just write it out
    if ( GetNormalName( metavalue ) == metavalue )
    {
        value = metavalue;
    }

    // Otherwise it is written in reversed form
    // ("Doe, John") and we write the reversed form
    // to the "file-as" attribute and the normal form as the value
    else
    {
        file_as = metavalue;
        value = GetNormalName( metavalue );
    }   

    // This assumes that the "dc" prefix has been declared for the DC namespace
    xc::DOMElement *element = document.createElementNS( QtoX( DUBLIN_CORE_NS ), QtoX( "dc:" + element_name ) );

    element->setAttributeNS( QtoX( OPF_XML_NAMESPACE ), QtoX( "role" ), QtoX( role ) );

    if ( !file_as.isEmpty() )

        element->setAttributeNS( QtoX( OPF_XML_NAMESPACE ), QtoX( "file-as" ), QtoX( file_as ) );

    element->setTextContent( QtoX( value ) );

    xc::DOMElement &metadata = GetMetadataElement( document );
    metadata.appendChild( element );
}


void OPFResource::WriteSimpleMetadata( 
    const QString &metaname, 
    const QString &metavalue, 
    xc::DOMDocument &document )
{
    // This assumes that the "dc" prefix has been declared for the DC namespace
    xc::DOMElement *element = document.createElementNS( QtoX( DUBLIN_CORE_NS ), QtoX( "dc:" + metaname ) );
    element->setTextContent( QtoX( metavalue ) );

    xc::DOMElement &metadata = GetMetadataElement( document );
    metadata.appendChild( element );
}


void OPFResource::WriteIdentifier( 
    const QString &metaname, 
    const QString &metavalue, 
    xc::DOMDocument &document )
{
    xc::DOMElement &main_identifier = GetMainIdentifier( document );

    // There's a possibility that this identifier is a duplicate
    // of the main identifier that we preserved, so we don't write
    // it out if it is.
    if ( metavalue == XtoQ( main_identifier.getTextContent() ) &&
         metaname == XtoQ( main_identifier.getAttributeNS( QtoX( OPF_XML_NAMESPACE ), QtoX( "scheme" ) ) ) )
    {
        return;
    }

    // This assumes that the "dc" prefix has been declared for the DC namespace
    xc::DOMElement *element = document.createElementNS( QtoX( DUBLIN_CORE_NS ), QtoX( "dc:identifier" ) );
    element->setAttributeNS( QtoX( OPF_XML_NAMESPACE ), QtoX( "scheme" ), QtoX( metaname ) );

    if ( metaname.toLower() == "uuid" && !metavalue.contains( "urn:uuid:" ) )

        element->setTextContent( QtoX( "urn:uuid:" + metavalue ) );

    else

        element->setTextContent( QtoX( metavalue ) );

    xc::DOMElement &metadata = GetMetadataElement( document );
    metadata.appendChild( element );
}


void OPFResource::WriteDate( 
    const QString &metaname, 
    const QVariant &metavalue,
    xc::DOMDocument &document )
{
    QString date = metavalue.toDate().toString( "yyyy-MM-dd" );
    
    // The metaname should be "Date of X", where X is
    // "publication", "creation" etc.
    QStringList metaname_words = metaname.split( " " );
    QString event_type = metaname_words.count() == 3          ? 
                         metaname.split( " " )[ 2 ].toLower() :
                         "publication";

    // This assumes that the "dc" prefix has been declared for the DC namespace
    xc::DOMElement *element = document.createElementNS( QtoX( DUBLIN_CORE_NS ), QtoX( "dc:date" ) );
    element->setAttributeNS( QtoX( OPF_XML_NAMESPACE ), QtoX( "event" ), QtoX( event_type ) );
    element->setTextContent( QtoX( date ) );

    xc::DOMElement &metadata = GetMetadataElement( document );
    metadata.appendChild( element );
}


QString OPFResource::GetNormalName( const QString &name )
{
    if ( !name.contains( "," ) )

        return name;

    QStringList splits = name.split( "," );

    return splits[ 1 ].trimmed() + " " + splits[ 0 ].trimmed();
}


bool OPFResource::BasicStructurePresent( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > packages = 
        XhtmlDoc::GetTagMatchingDescendants( document, "package", OPF_XML_NAMESPACE );

    if ( packages.count() != 1 )

        return false;

    QList< xc::DOMElement* > metadatas = 
        XhtmlDoc::GetTagMatchingDescendants( document, "metadata", OPF_XML_NAMESPACE );

    if ( metadatas.count() != 1 )

        return false;

    QList< xc::DOMElement* > manifests = 
        XhtmlDoc::GetTagMatchingDescendants( document, "manifest", OPF_XML_NAMESPACE );

    if ( manifests.count() != 1 )

        return false;

    QList< xc::DOMElement* > spines = 
        XhtmlDoc::GetTagMatchingDescendants( document, "spine", OPF_XML_NAMESPACE );

    if ( spines.count() != 1 )

        return false;

    xc::DOMElement* identifier = GetMainIdentifierUnsafe( document );
    if ( !identifier )

        return false;

    return true;
}


shared_ptr< xc::DOMDocument > OPFResource::CreateOPFFromScratch() const
{
    QString xml_source = GetOPFDefaultText();

    QString manifest_content;
    QString spine_content;
    QStringList relative_oebps_paths = GetRelativePathsToAllFilesInOEPBS();

    foreach( QString path, relative_oebps_paths )
    {
        // The OPF is not allowed to be in the manifest and the NCX
        // is already in the template.
        if ( path.contains( OPF_FILE_NAME ) || path.contains( NCX_FILE_NAME ) )

            continue;

        QString item_id = GetValidID( QFileInfo( path ).fileName() );
        QString item = ITEM_ELEMENT_TEMPLATE
                       .arg( item_id )
                       .arg( path )
                       .arg( GetFileMimetype( path ) );

        manifest_content.append( item );
        
        if ( TEXT_EXTENSIONS.contains( QFileInfo( path ).suffix().toLower() ) )
        {
            spine_content.append( ITEMREF_ELEMENT_TEMPLATE.arg( item_id ) );
        }
    }

    xml_source.replace( "</manifest>", manifest_content + "</manifest>" )
              .replace( "</spine>", spine_content + "</spine>" )
              .replace( "<metadata", OPF_REWRITTEN_COMMENT + "<metadata" );
                

    shared_ptr< xc::DOMDocument > document = 
        XhtmlDoc::LoadTextIntoDocument( xml_source );

    document->setXmlStandalone( true );
    return document;
}


// Yeah, we could get this list of paths with the GetSortedContentFilesList()
// func from FolderKeeper, but let's not create a strong coupling from
// the opf to the FK just yet. If we can work without that dependency,
// then let's do so.
QStringList OPFResource::GetRelativePathsToAllFilesInOEPBS() const
{
    // The parent folder of the OPF will always be the OEBPS folder.
    QString path_to_oebps_folder = QFileInfo( GetFullPath() ).absolutePath();
    QStringList paths = Utility::GetAbsolutePathsToFolderDescendantFiles( path_to_oebps_folder );
    paths.replaceInStrings( path_to_oebps_folder + "/", "" );

    paths.sort();
    return paths;
}


QString OPFResource::GetOPFDefaultText()
{
    // FIXME: This should use the Book's identifier... actually the Book's identifier 
    // should become what is in the OPF, not the other way around.
    return TEMPLATE_TEXT.arg( Utility::CreateUUID() );
}


void OPFResource::FillWithDefaultText()
{
    SetText( GetOPFDefaultText() );
}


QString OPFResource::GetResourceMimetype( const Resource &resource ) const
{
    return GetFileMimetype( resource.Filename() );
}


QString OPFResource::GetFileMimetype( const QString &filepath ) const
{
    return m_Mimetypes.value( QFileInfo( filepath ).suffix().toLower(), FALLBACK_MIMETYPE );
}


// Initializes m_Mimetypes
void OPFResource::CreateMimetypes()
{
    m_Mimetypes[ "jpg"   ] = "image/jpeg"; 
    m_Mimetypes[ "jpeg"  ] = "image/jpeg"; 
    m_Mimetypes[ "png"   ] = "image/png";
    m_Mimetypes[ "gif"   ] = "image/gif";
    m_Mimetypes[ "tif"   ] = "image/tiff";
    m_Mimetypes[ "tiff"  ] = "image/tiff";
    m_Mimetypes[ "bm"    ] = "image/bmp";
    m_Mimetypes[ "bmp"   ] = "image/bmp";
    m_Mimetypes[ "svg"   ] = "image/svg+xml";	

    m_Mimetypes[ "ncx"   ] = NCX_MIMETYPE; 

    // We convert all HTML document types to XHTML
    m_Mimetypes[ "xml"   ] = "application/xhtml+xml"; 
    m_Mimetypes[ "xhtml" ] = "application/xhtml+xml"; 
    m_Mimetypes[ "html"  ] = "application/xhtml+xml"; 
    m_Mimetypes[ "htm"   ] = "application/xhtml+xml"; 
    m_Mimetypes[ "css"   ] = "text/css"; 

    // Hopefully we won't get a lot of these
    m_Mimetypes[ "xpgt"  ] = "application/vnd.adobe-page-template+xml"; 

    // Until the standards gods grace us with font mimetypes,
    // these will have to do
    m_Mimetypes[ "otf"   ] = "application/vnd.ms-opentype"; 
    m_Mimetypes[ "ttf"   ] = "application/x-font-ttf";
    m_Mimetypes[ "ttc"   ] = "application/x-font-truetype-collection";
}














