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
#include "SearchOperations.h"
#include "../ResourceObjects/TextResource.h"
#include "../ResourceObjects/HTMLResource.h"
#include "../BookManipulation/XHTMLDoc.h"
#include "../BookManipulation/CleanSource.h"



int SearchOperations::CountInFiles( const QRegExp &search_regex,
                                    QList< Resource* > resources,
                                    SearchType search_type )
{
    QProgressDialog progress( QObject::tr( "Counting occurrences.." ), QString(), 0, resources.count() );
    progress.setMinimumDuration( PROGRESS_BAR_MINIMUM_DURATION );

    QFutureWatcher<int> watcher;
    QObject::connect( &watcher, SIGNAL( progressValueChanged( int ) ), &progress, SLOT( setValue( int ) ) );

    watcher.setFuture( QtConcurrent::mappedReduced( resources, 
                                                    boost::bind( CountInFile, search_regex, _1, search_type ),
                                                    Accumulate ) );

    return watcher.result();
}


int SearchOperations::CountInFile( const QRegExp &search_regex, 
                                   Resource* resource, 
                                   SearchType search_type )
{
    HTMLResource *html_resource = qobject_cast< HTMLResource* >( resource );

    if ( html_resource )
    {
        return CountInHTMLFile( search_regex, html_resource, search_type );
    }

    TextResource *text_resource = qobject_cast< TextResource* >( resource );
    
    if ( text_resource )
    {
        return CountInTextFile( search_regex, text_resource );
    }

    // We should never get here.
    Q_ASSERT( false );
    return 0;
}


int SearchOperations::CountInHTMLFile( const QRegExp &search_regex, 
                                       HTMLResource* html_resource, 
                                       SearchType search_type )
{
    if ( search_type == SearchOperations::CodeViewSearch )
    {
        const QDomDocument &document = html_resource->GetDomDocumentForReading();
        const QString &text          = CleanSource::PrettyPrint( XHTMLDoc::GetQDomNodeAsString( document ) );

        return text.count( search_regex );
    }

    //TODO: BookViewSearch
    return 0;
}


int SearchOperations::CountInTextFile( const QRegExp &search_regex, TextResource* text_resource )
{
    // TODO
    return 0;
}


void SearchOperations::Accumulate( int &first, const int &second )
{
    first += second;
}