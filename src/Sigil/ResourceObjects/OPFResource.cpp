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


void OPFResource::AddResource( const Resource &resource )
{
    QWriteLocker locker( &m_ReadWriteLock );

    shared_ptr< xc::DOMDocument > document = GetDocument();
    xc::DOMElement &manifest = GetManifestElement( *document );

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


void OPFResource::AppendToSpine( const QString &id, xc::DOMDocument &document )
{
    xc::DOMElement &spine = GetSpineElement( document );

    QHash< QString, QString > attributes;
    attributes[ "idref" ] = id;

    xc::DOMElement *new_item = XhtmlDoc::CreateElementInDocument(
        "itemref", OPF_XML_NAMESPACE, document, attributes );
    spine.appendChild( new_item );
}


shared_ptr< xc::DOMDocument > OPFResource::GetDocument()
{
    // TODO: make sure that the basic elements (package, metadata, manifest, spine) 
    // are present, otherwise rebuild the opf (and add a comment to the opf about this)
    return XhtmlDoc::LoadTextIntoDocument( m_TextDocument->toPlainText() );
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



