/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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
#include "PerformHTMLUpdates.h"
#include "PerformCSSUpdates.h"
#include "../BookManipulation/XHTMLDoc.h"
#include "../Misc/Utility.h"
#include <QDomDocument>

static const QStringList PATH_TAGS       = QStringList() << "link" << "a" << "img" << "image";
static const QStringList PATH_ATTRIBUTES = QStringList() << "href" << "src";


PerformHTMLUpdates::PerformHTMLUpdates( const QString &source,
                                        const QHash< QString, QString > &html_updates,
                                        const QHash< QString, QString > &css_updates )
    : 
    m_HTMLUpdates( html_updates ),
    m_CSSUpdates( css_updates )
{   
    m_Document.setContent( source );
}


PerformHTMLUpdates::PerformHTMLUpdates( const QDomDocument &document, 
                                        const QHash< QString, QString > &html_updates, 
                                        const QHash< QString, QString > &css_updates )
    : 
    m_HTMLUpdates( html_updates ),
    m_CSSUpdates( css_updates )
{
    m_Document = document;
}


QDomDocument PerformHTMLUpdates::operator()()
{
    UpdateHTMLReferences();

    if ( !m_CSSUpdates.isEmpty() )
    {
        m_Document.setContent(
            PerformCSSUpdates( XHTMLDoc::GetQDomNodeAsString( m_Document ), m_CSSUpdates )() );
    }

    return m_Document;
}


void PerformHTMLUpdates::UpdateHTMLReferences()
{
    QList< QDomNode > nodes = XHTMLDoc::GetTagMatchingChildren( m_Document.documentElement(), PATH_TAGS );

    int node_count = nodes.count();

    QFutureSynchronizer< void > sync;

    for ( int i = 0; i < node_count; ++i )
    {
        sync.addFuture( QtConcurrent::run( this, &PerformHTMLUpdates::UpdateReferenceInNode, nodes.at( i ) ) );
    }

    // We wait until all the nodes are updated
    sync.waitForFinished();
}


// This function has been brutally optimized since it is the main
// bottleneck during loading (well, not anymore :) ).
// Be vewy, vewy careful when editing it.
void PerformHTMLUpdates::UpdateReferenceInNode( QDomNode node )
{
    QDomNamedNodeMap attributes = node.attributes();
    int num_attributes = attributes.count();

    const QList< QString > &keys = m_HTMLUpdates.keys();
    int num_keys = keys.count();

    for ( int i = 0; i < num_attributes; ++i )
    {
        QDomAttr attribute = attributes.item( i ).toAttr();

        if ( !PATH_ATTRIBUTES.contains( XHTMLDoc::GetAttributeName( attribute ), Qt::CaseInsensitive ) )

             continue;

        for ( int j = 0; j < num_keys; ++j )
        {
            const QString &key_path  = keys.at( j );
            const QString &filename  = QFileInfo( key_path ).fileName();
            const QString &atr_value = Utility::URLDecodePath( attribute.value() );

            int name_index = atr_value.lastIndexOf( filename );

            if ( name_index == -1 )

                continue;
            
            int filename_length  = filename.length();
            int atr_value_length = atr_value.length();

            QString new_path;

            // First we look at whether the filename matches the attribute value,
            // and then we determine whether it's actually a path that ends with the filename
            if ( filename_length == atr_value_length || 
                 ( ( name_index + filename_length == atr_value_length ) &&
                   ( atr_value.at( name_index - 1 ) == QChar::fromAscii( '/' ) )
                 )
               )
            {
                new_path = m_HTMLUpdates.value( key_path );
            }

            // This checks for when the path has a fragment ID (anchor reference)
            else if ( atr_value.at( name_index + filename_length ) == QChar::fromAscii( '#' ) )
            {
                new_path = atr_value.mid( name_index + filename_length ).prepend( m_HTMLUpdates.value( key_path ) );
            }

            if ( !new_path.isEmpty() )
            {
                attribute.setValue( Utility::URLEncodePath( new_path ) );

                // We assign to "i" to break the outer loop
                i = num_attributes;
                break;
            }
        }
    }
}
