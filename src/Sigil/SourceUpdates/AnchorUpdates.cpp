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

#include <stdafx.h>
#include "AnchorUpdates.h"
#include "ResourceObjects/HTMLResource.h"
#include "../BookManipulation/XHTMLDoc.h"
#include "../Misc/Utility.h"


void AnchorUpdates::UpdateAllAnchorsWithIDs( const QList< HTMLResource* > &html_resources )
{
     const QHash< QString, QString > &ID_locations = GetIDLocations( html_resources );

     QtConcurrent::blockingMap( html_resources, boost::bind( UpdateAnchorsInOneFile, _1, ID_locations ) );
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

    return make_tuple( html_resource->Filename(),
        XHTMLDoc::GetAllChildIDs( html_resource->GetDomDocumentForReading().documentElement() ) );
}


void AnchorUpdates::UpdateAnchorsInOneFile( HTMLResource *html_resource, 
                                            const QHash< QString, QString > ID_locations )
{
    Q_ASSERT( html_resource );

    QWriteLocker locker( &html_resource->GetLock() );

    QDomDocument document = html_resource->GetDomDocumentForWriting();
    QDomNodeList anchors  = document.elementsByTagName( "a" );

    const QString &resource_filename = html_resource->Filename();

    for ( int i = 0; i < anchors.count(); ++i )
    {
        QDomElement element = anchors.at( i ).toElement();

        Q_ASSERT( !element.isNull() );

        if ( element.hasAttribute( "href" ) &&
             QUrl( element.attribute( "href" ) ).isRelative() &&
             element.attribute( "href" ).contains( "#" )
            )
        {
            const QString &href = element.attribute( "href" );
            const QString &id   = href.right( href.size() - ( href.indexOf( QChar( '#' ) ) + 1 ) );

            // If the ID is in a different file, update the link
            if ( ID_locations.value( id ) != resource_filename )
            {
                const QString &attribute_value = QString( "../" )
                                                 .append( TEXT_FOLDER_NAME )
                                                 .append( "/" )
                                                 .append( Utility::URLEncodePath( ID_locations.value( id ) ) )
                                                 .append( "#" )
                                                 .append( id );

                element.setAttribute( "href", attribute_value ); 
                html_resource->MarkSecondaryCachesAsOld();
            }
        } 
    }
}

