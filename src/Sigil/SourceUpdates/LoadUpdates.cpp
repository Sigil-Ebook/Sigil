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
#include "LoadUpdates.h"
#include <QDomDocument>
#include "../BookManipulation/XHTMLDoc.h"


LoadUpdates::LoadUpdates( const QString &source, const QHash< QString, QString > &updates )
    :
    m_Source( source ) 
{
    m_HTMLUpdates = updates;

    foreach( QString old_path, m_HTMLUpdates.keys() )
    {
        QString extension = QFileInfo( old_path ).suffix().toLower();

        // Font file updates are CSS updates, not HTML updates
        if ( extension == "ttf" || extension == "otf" )
        {
            m_CSSUpdates[ old_path ] = m_HTMLUpdates[ old_path ];
            m_HTMLUpdates.remove( old_path );
        }
    }
}


QString LoadUpdates::operator()()
{
    UpdateHTMLReferences();
    UpdateCSSReferences();

    return m_Source;
}


void LoadUpdates::UpdateHTMLReferences()
{
    QDomDocument document;
    document.setContent( m_Source );

    UpdateReferenceInNode( document.documentElement() );

    // We wait until all the nodes are updated
    m_NodeUpdateSynchronizer.waitForFinished();

    m_Source = XHTMLDoc::GetQDomNodeAsString( document );
}


void LoadUpdates::UpdateReferenceInNode( QDomNode node )
{
    QDomNamedNodeMap attributes = node.attributes();

    for ( int i = 0; i < attributes.count(); ++i )
    {
        QDomAttr attribute = attributes.item( i ).toAttr();

        if ( !attribute.isNull() )
        {
            foreach ( QString old_path, m_HTMLUpdates.keys() )
            {
                QString filename = QFileInfo( old_path ).fileName();

                QRegExp file_match( ".*/" + QRegExp::escape( filename ) + "|" + QRegExp::escape( filename ) );

                if ( file_match.exactMatch( QUrl::fromPercentEncoding( attribute.value().toUtf8() ) ) )
                {
                    QByteArray encoded_url = QUrl::toPercentEncoding( m_HTMLUpdates[ old_path ], QByteArray( "/" ) );

                    attribute.setValue( QString::fromUtf8( encoded_url.constData(), encoded_url.count() ) );
                }
            }            
        }
    }

    QDomNodeList children = node.childNodes();

    // We used to have a new synchronizer here that would monitor
    // the calls on its children, but that proved inefficient.
    // So we use a class global sync that waits for all nodes.

    QMutexLocker locker( &m_SynchronizerMutex );

    for ( int i = 0; i < children.count(); ++i )
    {        
        m_NodeUpdateSynchronizer.addFuture(
            QtConcurrent::run( this, &LoadUpdates::UpdateReferenceInNode, children.at( i ) ) );
    }
}


void LoadUpdates::UpdateCSSReferences()
{
    foreach( QString old_path, m_CSSUpdates.keys() )
    {
        QString filename  = QFileInfo( old_path ).fileName();

        QRegExp reference = QRegExp( "src:\\s*\\w+\\([\"']*([^\\)]*/" + QRegExp::escape( filename ) + "|"
            + QRegExp::escape( filename ) + ")[\"']*\\)" );

        int index = -1;

        while ( true )
        {
            int newindex = m_Source.indexOf( reference );

            // We need to make sure we don't end up
            // replacing the same thing over and over again
            if ( ( index == newindex ) || ( newindex == -1 ) )

                break;

            m_Source.replace( reference.cap( 1 ), m_CSSUpdates[ old_path ] );

            index = newindex;
        }
    }  
}

