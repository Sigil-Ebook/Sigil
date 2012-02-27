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
#include <boost/tuple/tuple.hpp>

#include <QtCore/QtCore>
#include <QtCore/QString>
#include <QtCore/QHash>

#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "sigil_constants.h"
#include "SourceUpdates/AnchorUpdates.h"

using boost::make_tuple;
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
    Q_ASSERT( html_resource );

    QReadLocker locker( &html_resource->GetLock() );

    QList< QString >ids = XhtmlDoc::GetAllDescendantIDs( *html_resource->GetDomDocumentForReading().getDocumentElement() );

    return make_tuple( html_resource->Filename(), ids );
}


void AnchorUpdates::UpdateAnchorsInOneFile( HTMLResource *html_resource, 
                                            const QHash< QString, QString > ID_locations )
{
    Q_ASSERT( html_resource );

    QWriteLocker locker( &html_resource->GetLock() );

    xc::DOMDocument &document = html_resource->GetDomDocumentForWriting();
    xc::DOMNodeList *anchors  = document.getElementsByTagName( QtoX( "a" ) );

    const QString &resource_filename = html_resource->Filename();

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
                html_resource->MarkSecondaryCachesAsOld();
            }
        } 
    }
}


void AnchorUpdates::UpdateExternalAnchorsInOneFile( HTMLResource *html_resource, const QString &originating_filename, const QHash< QString, QString > ID_locations )
{
    Q_ASSERT( html_resource );

    QWriteLocker locker( &html_resource->GetLock() );

    xc::DOMDocument &document = html_resource->GetDomDocumentForWriting();
    xc::DOMNodeList *anchors  = document.getElementsByTagName( QtoX( "a" ) );

    QString original_filename_with_relative_path = "../" % TEXT_FOLDER_NAME % "/" % originating_filename;

    // const QString &resource_filename = html_resource->Filename();

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
                html_resource->MarkSecondaryCachesAsOld();
            }
        }
    }
}
