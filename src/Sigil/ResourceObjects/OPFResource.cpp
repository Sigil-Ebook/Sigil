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
#include "BookManipulation/XhtmlDoc.h"
#include "BookManipulation/XercesCppUse.h"
#include "Misc/Utility.h"
#include <XmlUtils.h>

namespace xe = XercesExt;

static const QString OPF_XML_NAMESPACE = "http://www.idpf.org/2007/opf"; 
static const QString FALLBACK_MIMETYPE = "text/plain";
static const QString TEMPLATE_TEXT     = 
    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<package version=\"2.0\" xmlns=\"http://www.idpf.org/2007/opf\" unique-identifier=\"BookId\">\n\n"
    "  <metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:opf=\"http://www.idpf.org/2007/opf\">\n"
    "    <dc:identifier opf:scheme=\"UUID\" id=\"BookId\">%1</dc:identifier>\n"
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


GuideSemantics::GuideSemanticType OPFResource::GetGuideSemanticTypeForResource( const Resource &resource )
{
    QReadLocker locker( &m_ReadWriteLock );
    shared_ptr< xc::DOMDocument > document = GetDocument();
    return GetGuideSemanticTypeForResource( resource, *document );
}


bool OPFResource::IsCoverImage( const Resource &resource )
{
    if ( resource.Type() != ImageResource )

        return false;

    QReadLocker locker( &m_ReadWriteLock );

    shared_ptr< xc::DOMDocument > document = GetDocument();
    xc::DOMElement* meta                   = GetCoverMeta( *document );    
    
    if ( meta )
    {
        QString resource_id = GetResourceManifestID( resource, *document );

        return XtoQ( meta->getAttribute( QtoX( "content" ) ) ) == resource_id;
    }

    return false;
}


bool OPFResource::CoverImageExists()
{
    QReadLocker locker( &m_ReadWriteLock );

    shared_ptr< xc::DOMDocument > document = GetDocument();
    
    return GetCoverMeta( *document ) != NULL;
}


QString OPFResource::GetCoverPageOEBPSPath()
{
    QReadLocker locker( &m_ReadWriteLock );
    shared_ptr< xc::DOMDocument > document = GetDocument();
    QList< xc::DOMElement* > references = XhtmlDoc::GetTagMatchingDescendants( *document, "reference" );

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


void OPFResource::AddResource( const Resource &resource )
{
    QWriteLocker locker( &m_ReadWriteLock );

    shared_ptr< xc::DOMDocument > document = GetDocument();
    xc::DOMElement &manifest               = GetManifestElement( *document );

    QHash< QString, QString > attributes;
    attributes[ "id"         ] = GetValidID( resource.Filename() );
    attributes[ "href"       ] = resource.GetRelativePathToOEBPS();
    attributes[ "media-type" ] = GetResourceMimetype( resource );

    xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument( 
        "item", OPF_XML_NAMESPACE, *document, attributes );
    manifest.appendChild( new_item );

    if ( resource.Type() == Resource::HTMLResource )

        AppendToSpine( attributes[ "id" ], *document );

    UpdateTextFromDom( *document );
}


void OPFResource::RemoveResource( const Resource &resource )
{
    QWriteLocker locker( &m_ReadWriteLock );

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


void OPFResource::AddGuideSemanticType( const Resource &resource, GuideSemantics::GuideSemanticType new_type )
{
    QWriteLocker locker( &m_ReadWriteLock );

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


void OPFResource::SetResourceAsCoverImage( const Resource &resource )
{
    QWriteLocker locker( &m_ReadWriteLock );

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
        xc::DOMElement &metadata = GetMetadataElement( *document );

        QHash< QString, QString > attributes;
        attributes[ "name"    ] = "cover";
        attributes[ "content" ] = resource_id;

        xc::DOMElement *new_meta = XhtmlDoc::CreateElementInDocument( 
            "meta", OPF_XML_NAMESPACE, *document, attributes );

        metadata.appendChild( new_meta );
    }

    UpdateTextFromDom( *document );
}


void OPFResource::AppendToSpine( const QString &id, xc::DOMDocument &document )
{
    xc::DOMElement &spine = GetSpineElement( document );

    QHash< QString, QString > attributes;
    attributes[ "idref" ] = id;

    xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument(
        "itemref", OPF_XML_NAMESPACE, document, attributes );
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


shared_ptr< xc::DOMDocument > OPFResource::GetDocument()
{
    // TODO: make sure that the basic elements (package, metadata, manifest, spine) 
    // are present, otherwise rebuild the opf (and add a comment to the opf about this)
    shared_ptr< xc::DOMDocument > document = XhtmlDoc::LoadTextIntoDocument( m_TextDocument->toPlainText() );

    // For NCX files, the default of standalone == false should remain
    document->setXmlStandalone( true );
    return document;
}


xc::DOMElement& OPFResource::GetMetadataElement( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > metadatas = XhtmlDoc::GetTagMatchingDescendants( document, "metadata" );
    Q_ASSERT( !metadatas.isEmpty() );

    return *metadatas[ 0 ];  
}


xc::DOMElement& OPFResource::GetManifestElement( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > manifests = XhtmlDoc::GetTagMatchingDescendants( document, "manifest" );
    Q_ASSERT( !manifests.isEmpty() );

    return *manifests[ 0 ];    
}


xc::DOMElement& OPFResource::GetSpineElement( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > spines = XhtmlDoc::GetTagMatchingDescendants( document, "spine" );
    Q_ASSERT( !spines.isEmpty() );

    return *spines[ 0 ];    
}


xc::DOMElement& OPFResource::GetGuideElement( xc::DOMDocument &document )
{
    QList< xc::DOMElement* > guides = XhtmlDoc::GetTagMatchingDescendants( document, "guide" );
    
    if ( !guides.isEmpty() )

        return *guides[ 0 ];

    xc::DOMElement &package = *document.getDocumentElement();
    xc::DOMElement *guide = XhtmlDoc::CreateElementInDocument(
        "guide", OPF_XML_NAMESPACE, document, QHash< QString, QString >() );

    package.appendChild( guide );

    return *guide;
}


xc::DOMElement* OPFResource::GetGuideReferenceForResource( const Resource &resource, const xc::DOMDocument &document )
{
    QString resource_oebps_path         = resource.GetRelativePathToOEBPS();
    QList< xc::DOMElement* > references = XhtmlDoc::GetTagMatchingDescendants( document, "reference" );

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
        xc::DOMElement &guide = GetGuideElement( document );

        QHash< QString, QString > attributes;
        attributes[ "type"  ] = type_attribute;
        attributes[ "title" ] = title_attribute;
        attributes[ "href"  ] = resource.GetRelativePathToOEBPS();

        xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument( 
            "reference", OPF_XML_NAMESPACE, document, attributes );

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
    QList< xc::DOMElement* > references = XhtmlDoc::GetTagMatchingDescendants( document, "reference" );

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


xc::DOMElement* OPFResource::GetCoverMeta( const xc::DOMDocument &document )
{
    QList< xc::DOMElement* > metas = XhtmlDoc::GetTagMatchingDescendants( document, "meta" );

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


QString OPFResource::GetResourceManifestID( const Resource &resource, const xc::DOMDocument &document )
{
    QString oebps_path = resource.GetRelativePathToOEBPS();
    QList< xc::DOMElement* > items = XhtmlDoc::GetTagMatchingDescendants( document, "item" );

    foreach( xc::DOMElement* item, items )
    {
        QString href = XtoQ( item->getAttribute( QtoX( "href" ) ) );

        if ( href == oebps_path )

            return XtoQ( item->getAttribute( QtoX( "id" ) ) );
    }

    return QString();
}


void OPFResource::FillWithDefaultText()
{
    // FIXME: This should use the Book's identifier... actually the Book's identifier 
    // should become what is in the OPF, not the other way around.
    SetText( TEMPLATE_TEXT.arg( Utility::CreateUUID() ) );
}


QString OPFResource::GetResourceMimetype( const Resource &resource )
{
    return m_Mimetypes.value( QFileInfo( resource.Filename() ).suffix().toLower(), FALLBACK_MIMETYPE );
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








