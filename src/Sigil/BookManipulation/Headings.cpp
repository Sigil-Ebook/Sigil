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
#include "../BookManipulation/Headings.h"
#include "../BookManipulation/XHTMLDoc.h"
#include "../Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include <QDomDocument>

// The maximum allowed distance (in lines) that a heading
// can be located from a body tag and still
// be detected as the "name" for that chapter.
// The value was picked arbitrarily.
static const int ALLOWED_HEADING_DISTANCE = 20;
static const QStringList HEADING_TAGS = QStringList() << "h1" << "h2" << "h3" << "h4" << "h5" << "h6";

const QString NOT_IN_TOC_CLASS = "sigilNotInTOC";


// Returns a list of headings from the provided XHTML source;
// the list is flat, the headings are *not* in a hierarchy tree
QList< Headings::Heading > Headings::GetHeadingList( QList< HTMLResource* > html_resources )
{
    QList< Headings::Heading > heading_list;
    
    QList< QList< Headings::Heading > > per_file_headings =
        QtConcurrent::blockingMapped( html_resources, GetHeadingListForOneFile );

    for ( int i = 0; i < per_file_headings.count(); ++i )
    {
        heading_list.append( per_file_headings.at( i ) );
    }

    return heading_list;
}


QList< Headings::Heading > Headings::GetHeadingListForOneFile( HTMLResource* html_resource )
{
    Q_ASSERT( html_resource );

    QDomDocument document    = html_resource->GetDomDocumentForReading();
    QDomElement body_element = document.documentElement().firstChildElement( "body" );

    QList< QDomNode > heading_nodes = XHTMLDoc::GetTagMatchingChildren( document, HEADING_TAGS );
    int num_heading_nodes = heading_nodes.count();

    QList< Headings::Heading > headings;

    for ( int i = 0; i < num_heading_nodes; ++i )
    {
        QDomElement element = heading_nodes.at( i ).toElement();

        Q_ASSERT( !element.isNull() );

        Heading heading;
        heading.resource_file  = html_resource;
        heading.element        = element;
        heading.text           = ( element.hasAttribute( "title" ) ? 
                                   element.attribute( "title" )    :
                                   element.text()
                                 ).simplified(); 

        heading.level          = QString( element.tagName()[ 1 ] ).toInt();
        heading.include_in_toc = !element.attribute( "class", "" ).contains( NOT_IN_TOC_CLASS );
        heading.text_changed   = false;
        heading.at_file_start  = i == 0 && 
            element.lineNumber() - body_element.lineNumber() < ALLOWED_HEADING_DISTANCE;       

        headings.append( heading );
    }

    return headings;
}


// Takes a flat list of headings and returns a list with those
// headings sorted into a hierarchy
QList< Headings::Heading > Headings::MakeHeadingHeirarchy( const QList< Heading > &headings )
{
    QList< Heading > ordered_headings = headings;

    for ( int i = 0; i < ordered_headings.size(); ++i )
    {
        // As long as the headings after this one are
        // higher in level (smaller in size), we continue
        // adding them as this heading's children
        while ( true )
        {
            if ( ( i == ordered_headings.size() - 1 ) ||
                 ( ordered_headings[ i + 1 ].level <= ordered_headings[ i ].level ) 
               )
            {
                break;
            }
       
            AddChildHeading( ordered_headings[ i ], ordered_headings[ i + 1 ] );

            // The removeAt function will "push down" the rest
            // of the elements in the list by one after
            // it removes this element
            ordered_headings.removeAt( i + 1 );
        }
    }

    return ordered_headings;
}


QList< Headings::Heading > Headings::GetFlattenedHeadings( const QList< Heading > &headings )
{
    QList< Heading > flat_headings;

    foreach( Heading heading, headings )
    {
        flat_headings.append( FlattenHeadingNode( heading ) );
    }

    return flat_headings;
}


// Flattens the provided heading node and its children
// into a list and returns it
QList< Headings::Heading > Headings::FlattenHeadingNode( Heading heading )
{
    QList< Heading > my_headings;

    my_headings.append( heading );

    foreach( Heading child_heading, heading.children )
    {
        my_headings.append( FlattenHeadingNode( child_heading ) );
    }

    return my_headings;     
}


// Adds the new_child heading to the parent heading;
// the new_child is propagated down the tree if necessary
void Headings::AddChildHeading( Heading &parent, Heading new_child )
{
    if ( ( !parent.children.isEmpty() ) && 
         ( parent.children.last().level < new_child.level )
       )
    {
        AddChildHeading( parent.children.last(), new_child );
    }

    else
    {
        parent.children.append( new_child );
    }
}

