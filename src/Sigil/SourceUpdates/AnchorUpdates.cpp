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

#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>

#include <QtCore/QtCore>
#include <QtCore/QString>
#include <QtCore/QHash>

#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "sigil_constants.h"
#include "SourceUpdates/AnchorUpdates.h"

using boost::make_tuple;
using boost::shared_ptr;
using boost::tie;
using boost::tuple;

void AnchorUpdates::UpdateAllAnchorsWithIDs( const QList< HTMLResource* > &html_resources )
{
     const QHash< QString, QString > &ID_locations = GetIDLocations( html_resources );

     QtConcurrent::blockingMap( html_resources, boost::bind( UpdateAnchorsInOneFile, _1, ID_locations ) );
}


void AnchorUpdates::UpdateExternalAnchors( const QList< HTMLResource* > &html_resources, const QString &originating_filename, const QList< HTMLResource* > new_files )
{
    const QHash< QString, QString > &ID_locations = GetIDLocations( new_files );

    QtConcurrent::blockingMap( html_resources, boost::bind( UpdateExternalAnchorsInOneFile, _1, originating_filename, ID_locations ) );
}


void AnchorUpdates::UpdateAllAnchors( const QList< HTMLResource* > &html_resources, const QStringList &originating_filenames, HTMLResource* new_file )
{
    QList< HTMLResource* > new_files;
    new_files.append(new_file);
    const QHash< QString, QString > &ID_locations = GetIDLocations( new_files );

    QList< QString > originating_filename_links;
    foreach( QString originating_filename, originating_filenames) {
        originating_filename_links.append("../" % TEXT_FOLDER_NAME % "/" % originating_filename);
    }

    const QString &new_filename_with_relative_path = "../" % TEXT_FOLDER_NAME % "/" % Utility::URLEncodePath(new_file->Filename());

    QtConcurrent::blockingMap( html_resources, boost::bind( UpdateAllAnchorsInOneFile, _1, originating_filename_links, ID_locations, new_filename_with_relative_path ) );
}


QHash< QString, QString > AnchorUpdates::GetIDLocations( const QList< HTMLResource* > &html_resources )
{
    const QList< tuple< QString, QList< QString > > > &IDs_in_files = QtConcurrent::blockingMapped( html_resources, GetOneFileIDs );

    QHash< QString, QString > ID_locations;

    for ( int i = 0; i < IDs_in_files.count(); ++i )
    {
        QList< QString > file_element_IDs;
        QString resource_filename;

        tie( resource_filename, file_element_IDs ) = IDs_in_files.at( i );

        for ( int j = 0; j < file_element_IDs.count(); ++j )
        {
            ID_locations[ file_element_IDs.at( j ) ] = resource_filename;
        }
    }

    return ID_locations;
}


tuple< QString, QList< QString > > AnchorUpdates::GetOneFileIDs( HTMLResource* html_resource )
{
    Q_ASSERT(html_resource);

    QReadLocker locker(&html_resource->GetLock());

    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(html_resource->GetText());
    QList<QString> ids = XhtmlDoc::GetAllDescendantIDs(*d.get()->getDocumentElement());
    return make_tuple(html_resource->Filename(), ids);
}


void AnchorUpdates::UpdateAnchorsInOneFile( HTMLResource *html_resource, 
                                            const QHash< QString, QString > ID_locations )
{
    Q_ASSERT( html_resource );

    QWriteLocker locker( &html_resource->GetLock() );

    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(html_resource->GetText());
    xc::DOMDocument &document = *d.get();
    xc::DOMNodeList *anchors  = document.getElementsByTagName( QtoX( "a" ) );

    const QString &resource_filename = html_resource->Filename();

    bool is_changed = false;
    for ( uint i = 0; i < anchors->getLength(); ++i )
    {
        xc::DOMElement &element = *static_cast< xc::DOMElement* >( anchors->item( i ) );

        Q_ASSERT( &element );

        if ( element.hasAttribute( QtoX( "href" ) ) &&
             QUrl( XtoQ( element.getAttribute( QtoX(  "href" ) ) ) ).isRelative() &&
             XtoQ( element.getAttribute( QtoX(  "href" ) ) ).contains( "#" )
            )
        {
            QString href = XtoQ( element.getAttribute( QtoX( "href" ) ) );
            QString id   = href.right( href.size() - ( href.indexOf( QChar( '#' ) ) + 1 ) );

            QString file_id = ID_locations.value( id );
            // If the ID is in a different file, update the link
            if ( file_id != resource_filename && !file_id.isEmpty() )
            {
                QString attribute_value = QString( "../" )
                                          .append( TEXT_FOLDER_NAME )
                                          .append( "/" )
                                          .append( Utility::URLEncodePath( file_id ) )
                                          .append( "#" )
                                          .append( id );

                element.setAttribute( QtoX( "href" ), QtoX( attribute_value ) );
                is_changed = true;
            }
        }
    }
    if (is_changed) {
        html_resource->SetText(XhtmlDoc::GetDomDocumentAsString(document));
    }
}


void AnchorUpdates::UpdateExternalAnchorsInOneFile( HTMLResource *html_resource, const QString &originating_filename, const QHash< QString, QString > ID_locations )
{
    Q_ASSERT( html_resource );

    QWriteLocker locker( &html_resource->GetLock() );

    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(html_resource->GetText());
    xc::DOMDocument &document = *d.get();
    xc::DOMNodeList *anchors  = document.getElementsByTagName( QtoX( "a" ) );

    QString original_filename_with_relative_path = "../" % TEXT_FOLDER_NAME % "/" % originating_filename;

    bool is_changed = false;
    for ( uint i = 0; i < anchors->getLength(); ++i )
    {
        xc::DOMElement &element = *static_cast< xc::DOMElement* >( anchors->item( i ) );

        Q_ASSERT( &element );

        // We're only interested in hrefs of the form "originating_filename#fragment_id".
        // First, we find the hrefs that are relative and contain a fragment id.
        if ( element.hasAttribute( QtoX( "href" ) ) &&
            QUrl( XtoQ( element.getAttribute( QtoX(  "href" ) ) ) ).isRelative() &&
            XtoQ( element.getAttribute( QtoX(  "href" ) ) ).contains( "#" )
            )
        {
            QString href = XtoQ( element.getAttribute( QtoX( "href" ) ) );
            QString file_id       = href.left(  href.indexOf( QChar( '#' ) ) );
            QString fragment_id   = href.right( href.size() - ( href.indexOf( QChar( '#' ) ) + 1 ) );

            // If the href pointed to the original file then update the file_id.
            if ( file_id == original_filename_with_relative_path )
            {
                QString attribute_value = QString( "../" )
                                          .append( TEXT_FOLDER_NAME )
                                          .append( "/" )
                                          .append( Utility::URLEncodePath( ID_locations.value( fragment_id ) ) )
                                          .append( "#" )
                                          .append( fragment_id );

                element.setAttribute( QtoX( "href" ), QtoX( attribute_value ) );
                is_changed = true;
            }
        }
    }
    if (is_changed) {
        html_resource->SetText(XhtmlDoc::GetDomDocumentAsString(document));
    }
}


void AnchorUpdates::UpdateAllAnchorsInOneFile( HTMLResource *html_resource, 
                                               const QList< QString > &originating_filename_links, 
                                               const QHash< QString, QString > ID_locations, 
                                               const QString &new_filename )
{
    Q_ASSERT( html_resource );

    QWriteLocker locker( &html_resource->GetLock() );

    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(html_resource->GetText());
    xc::DOMDocument &document = *d.get();
    xc::DOMNodeList *anchors  = document.getElementsByTagName( QtoX( "a" ) );

    bool is_changed = false;
    for ( uint i = 0; i < anchors->getLength(); ++i )
    {
        xc::DOMElement &element = *static_cast< xc::DOMElement* >( anchors->item( i ) );

        Q_ASSERT( &element );

        // We find the hrefs that are relative and contain an href.
        if ( element.hasAttribute( QtoX( "href" ) ) &&
            QUrl( XtoQ( element.getAttribute( QtoX(  "href" ) ) ) ).isRelative()
            )
        {
            // Is this href in the form "originating_filename#fragment_id" or "originating_filename"?
            QString href = XtoQ( element.getAttribute( QtoX( "href" ) ) );
            if (href.contains('#')) {
                QString file_id = href.left( href.indexOf( QChar( '#' ) ) );
                QString fragment_id = href.right( href.size() - ( href.indexOf( QChar( '#' ) ) + 1 ) );

                // If the href pointed to the original file then update the file_id.
                if ( originating_filename_links.contains(file_id) )
                {
                    QString attribute_value = QString( "../" )
                                              .append( TEXT_FOLDER_NAME )
                                              .append( "/" )
                                              .append( Utility::URLEncodePath( ID_locations.value( fragment_id ) ) )
                                              .append( "#" )
                                              .append( fragment_id );

                    element.setAttribute( QtoX( "href" ), QtoX( attribute_value ) );
                    is_changed = true;
                }
            }
            else {
                // This is a straight href with no anchor fragment
                if ( originating_filename_links.contains(href) ) {
                    element.setAttribute( QtoX( "href" ), QtoX( new_filename ) );
                    is_changed = true;
                }
            }
        }
    }
    if (is_changed) {
        html_resource->SetText(XhtmlDoc::GetDomDocumentAsString(document));
    }
}


void AnchorUpdates::UpdateTOCEntries(NCXResource *ncx_resource, const QString &originating_filename, const QList< HTMLResource* > new_files)
{
    Q_ASSERT( ncx_resource );

    const QHash< QString, QString > &ID_locations = GetIDLocations(new_files);

    QWriteLocker locker(&ncx_resource->GetLock());

    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(ncx_resource->GetText());
    xc::DOMDocument &document = *d.get();
    xc::DOMNodeList *anchors  = document.getElementsByTagName(QtoX("content"));

    QString original_filename_with_relative_path = TEXT_FOLDER_NAME % "/" % originating_filename;

    for (uint i = 0; i < anchors->getLength(); ++i) {
        xc::DOMElement &element = *static_cast< xc::DOMElement* >(anchors->item(i));

        Q_ASSERT(&element);

        // We're only interested in src links of the form "originating_filename#fragment_id".
        // First, we find the hrefs that are relative and contain a fragment id.
        if (element.hasAttribute(QtoX("src")) &&
            QUrl(XtoQ( element.getAttribute(QtoX("src")))).isRelative() &&
            XtoQ(element.getAttribute(QtoX("src"))).contains("#")) {
            QString src = XtoQ(element.getAttribute(QtoX("src")));
            QString file_id = src.left(src.indexOf(QChar( '#' )));
            QString fragment_id = src.right(src.size() - (src.indexOf(QChar('#')) + 1));

            // If the src pointed to the original file then update the file_id.
            if (file_id == original_filename_with_relative_path) {
                QString attribute_value = QString("%1").arg(TEXT_FOLDER_NAME)
                                          .append("/")
                                          .append(Utility::URLEncodePath(ID_locations.value(fragment_id)))
                                          .append("#")
                                          .append(fragment_id);

                element.setAttribute(QtoX("src"), QtoX(attribute_value));
            }
        }
    }
    ncx_resource->SetText(XhtmlDoc::GetDomDocumentAsString(document));
}
