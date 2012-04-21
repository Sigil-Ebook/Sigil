/************************************************************************
**
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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

#include <QtCore/QtCore>
#include <QtCore/QString>

#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "ResourceObjects/HTMLResource.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"
#include "SourceUpdates/LinkUpdates.h"

using boost::shared_ptr;

static QString HTML_XML_NAMESPACE = "http://www.w3.org/1999/xhtml";


void LinkUpdates::UpdateLinksInAllFiles( const QList< HTMLResource* > &html_resources, const QList<QString> new_stylesheets )
{
    QtConcurrent::blockingMap( html_resources, boost::bind( UpdateLinksInOneFile, _1, new_stylesheets ) );
}

void LinkUpdates::UpdateLinksInOneFile( HTMLResource *html_resource, QList<QString> new_stylesheets )
{
    Q_ASSERT( html_resource );

    QWriteLocker locker( &html_resource->GetLock() );

    shared_ptr<xc::DOMDocument> d = XhtmlDoc::LoadTextIntoDocument(html_resource->GetText());
    xc::DOMDocument &document = *d.get();

    // head should only appear once
    xc::DOMNodeList *heads = document.getElementsByTagName( QtoX( "head" ) );
    xc::DOMElement &head_element = *static_cast< xc::DOMElement* >( heads->item( 0 ) ); 

    // We only want links in the head
    xc::DOMNodeList *links = head_element.getElementsByTagName( QtoX( "link" ) );

    // Remove the old stylesheet links
    // Link count is dynamic
    uint links_count = links->getLength();
    for ( uint i = 0; i < links_count; i++ )
    {
        // Always delete the top element since list is dynamic
        xc::DOMElement &element = *static_cast< xc::DOMElement* >( links->item( 0 ) );

        Q_ASSERT( &element );

       if ( element.hasAttribute( QtoX( "type" ) ) &&
             XtoQ( element.getAttribute( QtoX(  "type" ) ) ) == "text/css" &&
            element.hasAttribute( QtoX( "rel" ) ) &&
              XtoQ( element.getAttribute( QtoX(  "rel" ) ) ) == "stylesheet")
        {
            head_element.removeChild( &element );
        }
    }

    // Add the new stylesheet links
    foreach ( QString stylesheet, new_stylesheets )
    {
        xc::DOMElement *element = document.createElementNS( QtoX( HTML_XML_NAMESPACE ), QtoX( "link" ) );
        element->setAttribute( QtoX( "href" ), QtoX( stylesheet ) );
        element->setAttribute( QtoX( "type" ), QtoX( "text/css" ) );
        element->setAttribute( QtoX( "rel" ),  QtoX( "stylesheet" ) );

        head_element.appendChild( element );
    }

    html_resource->SetText(XhtmlDoc::GetDomDocumentAsString(document));
}
