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
#include "SearchOperations.h"
#include "ResourceObjects/TextResource.h"
#include "ResourceObjects/HTMLResource.h"
#include "BookManipulation/XhtmlDoc.h"
#include "BookManipulation/CleanSource.h"
#include "ViewEditors/Searchable.h"
#include "BookManipulation/CleanSource.h"
#include "BookManipulation/XercesCppUse.h"
#include "PCRE/PCRECache.h"
#include "Utility.h"



int SearchOperations::CountInFiles( const QString &search_regex,
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


int SearchOperations::ReplaceInAllFIles( const QString &search_regex,
                                         const QString &replacement,
                                         QList< Resource* > resources, 
                                         SearchType search_type )
{
    QProgressDialog progress( QObject::tr( "Replacing search term..." ), QString(), 0, resources.count() );
    progress.setMinimumDuration( PROGRESS_BAR_MINIMUM_DURATION );

    QFutureWatcher<int> watcher;
    QObject::connect( &watcher, SIGNAL( progressValueChanged( int ) ), &progress, SLOT( setValue( int ) ) );

    watcher.setFuture( QtConcurrent::mappedReduced( resources, 
                                                    boost::bind( ReplaceInFile, search_regex, replacement, _1, search_type ),
                                                    Accumulate ) );

    return watcher.result();
}


int SearchOperations::CountInFile( const QString &search_regex,
                                   Resource* resource, 
                                   SearchType search_type )
{
    QReadLocker locker( &resource->GetLock() );

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
    return 0;
}


int SearchOperations::CountInHTMLFile( const QString &search_regex,
                                       HTMLResource* html_resource, 
                                       SearchType search_type )
{
    if ( search_type == SearchOperations::CodeViewSearch )
    {
        const xc::DOMDocument &document = html_resource->GetDomDocumentForReading();
        const QString &text = CleanSource::PrettyPrint( XhtmlDoc::GetDomDocumentAsString( document ) );

        //return Searchable::Count( search_regex, text );
        return PCRECache::instance()->getObject( search_regex )->getMatchOffsets( text ).count();
    }

    //TODO: BookViewSearch
    return 0;
}


int SearchOperations::CountInTextFile( const QString &search_regex, TextResource* text_resource )
{
    // TODO
    return 0;
}


int SearchOperations::ReplaceInFile( const QString &search_regex,
                                     const QString &replacement, 
                                     Resource* resource, 
                                     SearchType search_type )
{
    QWriteLocker locker( &resource->GetLock() );

    HTMLResource *html_resource = qobject_cast< HTMLResource* >( resource );

    if ( html_resource )
    {
        return ReplaceHTMLInFile( search_regex, replacement, html_resource, search_type );
    }

    TextResource *text_resource = qobject_cast< TextResource* >( resource );

    if ( text_resource )
    {
        return ReplaceTextInFile( search_regex, replacement, text_resource );
    }

    // We should never get here.
    return 0;
}


int SearchOperations::ReplaceHTMLInFile( const QString &search_regex,
                                         const QString &replacement, 
                                         HTMLResource* html_resource, 
                                         SearchType search_type )
{
    if ( search_type == SearchOperations::CodeViewSearch )
    {
        const QString &text = CleanSource::PrettyPrint( 
            XhtmlDoc::GetDomDocumentAsString( html_resource->GetDomDocumentForReading() ) );
    
        QString new_text;
        int count;
        tie( new_text, count ) = PerformGlobalReplace( text, search_regex, replacement );

        html_resource->SetDomDocument( 
            XhtmlDoc::LoadTextIntoDocument( CleanSource::ToValidXHTML( new_text ) ) );

        return count;
    }

    //TODO: BookViewSearch
    return 0;
}


int SearchOperations::ReplaceTextInFile( const QString &search_regex,
                                         const QString &replacement, 
                                         TextResource* text_resource )
{
    // TODO
    return 0;
}


// We don't use QString.replace(QRegExp, QString) because
// 1. we want the same behavior across all search algos;
// 2. it doesn't return replace count;
// 3. it would have to be replaced with this code either way
//    when we integrate PCRE. 
tuple< QString, int > SearchOperations::PerformGlobalReplace( const QString &text, 
                                                              const QString &search_regex,
                                                              const QString &replacement )
{
    QString new_text = text;
    int count = 0;

    /*
    int start = 0;
    int end = 0;

    tie( start, end ) = Searchable::RunSearchRegex( search_regex, text, start, Searchable::Direction_Down );
    while ( start != end )
    {
        QString match_segement = Utility::Substring( start, end, new_text );
        QString replaced_segement;

        Searchable::FillWithCapturedTexts( search_regex, match_segement, replacement, replaced_segement );
        new_text = Utility::Substring( 0, start, new_text ) % replaced_segement % Utility::Substring( end, new_text.length(), new_text );

        tie( start, end ) = Searchable::RunSearchRegex( search_regex, text, start + replaced_segement.length(), Searchable::Direction_Down );
        count++;
    }
*/
    return make_tuple( new_text, count );
}


void SearchOperations::Accumulate( int &first, const int &second )
{
    first += second;
}
