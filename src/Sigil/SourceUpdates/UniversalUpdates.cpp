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
#include "UniversalUpdates.h"
#include "PerformHTMLUpdates.h"
#include "PerformCSSUpdates.h"
#include "../ResourceObjects/HTMLResource.h"
#include "../ResourceObjects/CSSResource.h"
#include "../Misc/HTMLEncodingResolver.h"
#include "../BookManipulation/CleanSource.h"
#include "../Misc/Utility.h"
#include "CustomSyncs/SGReadLocker.h"
#include "CustomSyncs/SGWriteLocker.h"


void UniversalUpdates::PerformUniversalUpdates( bool resources_already_loaded,
                                                const QList< Resource* > &resources,
                                                const QHash< QString, QString > &updates )
{
    QHash< QString, QString > html_updates;
    QHash< QString, QString > css_updates;
    tie( html_updates, css_updates ) = SeparateHTMLAndCSSUpdates( updates );

    QList< HTMLResource* > html_resources;
    QList< CSSResource* > css_resources;

    int num_files = resources.count();

    for ( int i = 0; i < num_files; ++i )
    {
        Resource *resource = resources.at( i );

        if ( resource->Type() == Resource::HTMLResource )

            html_resources.append( qobject_cast< HTMLResource* >( resource ) );

        else if ( resource->Type() == Resource::CSSResource )   

            css_resources.append( qobject_cast< CSSResource* >( resource ) );
    }

    QFutureSynchronizer<void> sync;

    if ( resources_already_loaded )
    {
        sync.addFuture( QtConcurrent::map( html_resources, boost::bind( UpdateOneHTMLFile, _1, html_updates, css_updates ) ) );
        sync.addFuture( QtConcurrent::map( css_resources,  boost::bind( UpdateOneCSSFile,  _1, css_updates ) ) );
    }

    else
    {
        sync.addFuture( QtConcurrent::map( html_resources, boost::bind( LoadAndUpdateOneHTMLFile, _1, html_updates, css_updates ) ) );
        sync.addFuture( QtConcurrent::map( css_resources,  boost::bind( LoadAndUpdateOneCSSFile,  _1, css_updates ) ) );
    }

    sync.waitForFinished();
}


tuple< QHash< QString, QString >, 
QHash< QString, QString > > UniversalUpdates::SeparateHTMLAndCSSUpdates( const QHash< QString, QString > &updates )
{
    QHash< QString, QString > html_updates = updates;
    QHash< QString, QString > css_updates;

    QList< QString > keys = updates.keys();
    int num_keys = keys.count();

    for ( int i = 0; i < num_keys; ++i )
    {
        QString key_path = keys.at( i );
        QString extension = QFileInfo( key_path ).suffix().toLower();

        // Font file updates are CSS updates, not HTML updates
        if ( extension == "ttf" || extension == "otf" )
        {
            css_updates[ key_path ] = html_updates.value( key_path );
            html_updates.remove( key_path );
        }

        if ( extension == "css" )
        {
            // Needed for CSS updates because of @import rules
            css_updates[ key_path ] = html_updates.value( key_path );
        }
    }

    return make_tuple( html_updates, css_updates );
}


void UniversalUpdates::UpdateOneHTMLFile( HTMLResource* html_resource, 
                                          const QHash< QString, QString > &html_updates, 
                                          const QHash< QString, QString > &css_updates )
{
    SGWriteLocker locker( &html_resource->GetLock() );
    const QDomDocument &document = html_resource->GetDomDocumentForWriting();
    html_resource->SetDomDocument( PerformHTMLUpdates( document, html_updates, css_updates )() );
}


void UniversalUpdates::UpdateOneCSSFile( CSSResource* css_resource, 
                                         const QHash< QString, QString > &css_updates )
{
    SGWriteLocker locker( &css_resource->GetLock() );
    const QString &source = css_resource->GetTextDocumentForWriting().toPlainText();
    css_resource->SetText( PerformCSSUpdates( source, css_updates )() );
}


void UniversalUpdates::LoadAndUpdateOneHTMLFile( HTMLResource* html_resource,
                                                 const QHash< QString, QString > &html_updates,
                                                 const QHash< QString, QString > &css_updates )
{
    const QString &source = CleanSource::Clean( HTMLEncodingResolver::ReadHTMLFile( html_resource->GetFullPath() ) );
    html_resource->SetDomDocument( PerformHTMLUpdates( source, html_updates, css_updates )() );
}


void UniversalUpdates::LoadAndUpdateOneCSSFile( CSSResource* css_resource, 
                                                const QHash< QString, QString > &css_updates )
{
    const QString &source = Utility::ReadUnicodeTextFile( css_resource->GetFullPath() );
    css_resource->SetText( PerformCSSUpdates( source, css_updates )() );
}