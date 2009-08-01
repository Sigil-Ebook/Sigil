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

#include "stdafx.h"
#include "Headings.h"

// The maximum allowed distance (in characters) that a heading
// can be located from a chapter break (or body tag) and still
// be detected as the "name" for that chapter;
// The value was picked arbitrarily
static const int CHAPTER_TO_HEADING_DIST = 1000;

// Use with <QRegExp>.setMinimal( true )
const QString HEADING                    = "<\\s*(?:h|H)\\d[^>]*>(.*)</\\s*(?:h|H)\\d[^>]*>";

static const QString NOT_IN_TOC_CLASS    = "sigilNotInTOC";
static const QString TOC_CLASS_PRESENT   = "(<[^>]*)" + NOT_IN_TOC_CLASS + "([^>]*>)";
static const QString CLASS_ATTRIBUTE     = "<[^>]*class\\s*=\\s*\"([^\"]+)\"[^>]*>";
static const QString ID_ATTRIBUTE        = "<[^>]*id\\s*=\\s*\"([^\"]+)\"[^>]*>";
static const QString ELEMENT_BODY        = "<([^/>]+)>";
static const QString HEADING_LEVEL       = "(?:h|H)(\\d)";


// Constructs the new heading element source
// created from the provided heading struct
QString Headings::GetNewHeadingSource( const Heading &heading )
{
    QString source = heading.element_source;

    // We first remove our TOC class if it's present
    QRegExp sigil_class( TOC_CLASS_PRESENT );
    source.replace( sigil_class, "\\1\\2" );

    // If the heading should be included, 
    // that's it, return what we have
    if ( heading.include_in_toc == true )

        return source;

    // Otherwise add our not-in-TOC CSS class

    QRegExp class_reg( CLASS_ATTRIBUTE );

    // If the element already has a class attribute,
    // we add our TOC class to the list of classes
    if ( source.indexOf( class_reg ) != -1 )
    {
        QString new_class_value  = class_reg.cap( 1 ) + " " + NOT_IN_TOC_CLASS;
        QString new_class        = class_reg.cap( 0 ).replace( class_reg.cap( 1 ), new_class_value );

        source.replace( class_reg, new_class );
    }

    // Otherwise we create the class attribute too
    else
    {
        QRegExp element( ELEMENT_BODY );

        source.indexOf( element );

        QString new_element_body = element.cap( 1 ) + " class=\"" + NOT_IN_TOC_CLASS + "\"";
        QString new_element      = element.cap( 0 ).replace( element.cap( 1 ), new_element_body );

        source.replace( element, new_element );
    }

    return source;
}


// Returns a list of headings from the provided XHTML source;
// the list is flat, the headings are *not* in a hierarchy tree
QList< Headings::Heading > Headings::GetHeadingList( const QString &source )
{
    QList< Heading > heading_list;

    QRegExp heading_regex( HEADING );
    heading_regex.setMinimal( true );

    int main_index = 0;

    while ( true )
    {
        main_index = source.indexOf( heading_regex, main_index );

        if ( main_index == -1 )

            break;

        Heading heading;

        heading.element_source      = heading_regex.cap( 0 );
        heading.text                = heading_regex.cap( 1 );
        heading.after_chapter_break = IsAfterChapterBreak( source, main_index );         

        QRegExp level( HEADING_LEVEL );
        heading_regex.cap( 0 ).indexOf( level );

        heading.level               = level.cap( 1 ).toInt();  

        QRegExp id( ID_ATTRIBUTE );
        heading_regex.cap( 0 ).indexOf( id );

        heading.id                  = id.cap( 1 );

        QRegExp sigil_class( TOC_CLASS_PRESENT );

        if ( heading_regex.cap( 0 ).indexOf( sigil_class ) != -1 )

            heading.include_in_toc = false;

        else

            heading.include_in_toc = true;              

        heading_list.append( heading );

        main_index += heading_regex.matchedLength();
    }

    return heading_list;
}


// Takes a flat list of headings and returns a list with those
// headings sorted into a hierarchy
QList< Headings::Heading > Headings::MakeHeadingHeirarchy( const QList< Heading > &headings )
{
    QList< Heading > ordered_headings = ExcludeHeadingsAboveRoot( headings, GetRootHeadingLevel( headings ) );

    for ( int i = 0; i < ordered_headings.size(); i++ )
    {
        // As long as the headings after this one are
        // higher in level (smaller in size), we continue
        // adding them as this heading's children
        while ( true )
        {
            if (    ( i == ordered_headings.size() - 1 ) ||
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


// Takes a hierarchical list of headings and returns a flat list of them
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


// Returns true if the provided heading location 
// appears within 1000 chars after a chapter break
bool Headings::IsAfterChapterBreak( const QString &source, int heading_location )
{
    // Our search is between search_start and heading_location
    int search_start = heading_location - CHAPTER_TO_HEADING_DIST;

    if ( search_start < 0 )

        search_start = 0;

    QRegExp chapter_break_tag( BREAK_TAG );
    int chapter_location        = source.lastIndexOf( chapter_break_tag, heading_location - 1 );

    // We look for a heading that appears before "this one"
    // because we need to make sure we don't "assign"
    // a chapter break that is already "taken"
    QRegExp heading_regex( HEADING );
    heading_regex.setMinimal( true );
    int previous_heading_location = source.lastIndexOf( heading_regex, heading_location - 1 );

    QRegExp body_tag( BODY_START );
    int body_tag_location         = source.lastIndexOf( body_tag, heading_location - 1 );

    // The tags are considered "found" if they appear within our search interval
    bool chapter_tag_found      = ( chapter_location          != -1 ) && ( chapter_location          > search_start );
    bool body_tag_found         = ( body_tag_location         != -1 ) && ( body_tag_location         > search_start );

    // There is no need to check that chapter_location and body_tag_location
    // are greater than -1 because if they're both aren't, the next IF is false anyway
    bool previous_heading_found = ( previous_heading_location != -1 ) && ( previous_heading_location > search_start ) &&
        ( previous_heading_location > chapter_location ) && ( previous_heading_location > body_tag_location );

    if ( ( chapter_tag_found || body_tag_found ) && !previous_heading_found )

        return true;

    else

        return false;
}


// Adds the new_child heading to the parent heading;
// the new_child is propagated down the tree if necessary
void Headings::AddChildHeading( Heading &parent, Heading new_child )
{
    if (    ( !parent.children.isEmpty() ) && 
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


// Returns the root heading level; the root level is
// considered to be the level with the max number of 
// headings that appear after a chapter break
int Headings::GetRootHeadingLevel( const QList< Heading > &headings )
{
    // Used to count the number of 
    // chapter breaks per level;
    // the keys are levels, the values 
    // are the number of chapter breaks per that level
    QHash< int, int > chapters_per_level;

    foreach( Heading heading, headings )
    {
        if ( heading.after_chapter_break == true )

            chapters_per_level[ heading.level ] = chapters_per_level.value( heading.level, 0 ) + 1 ;
    }

    int maxvalue = 0;
    int maxvalue_key = 0;

    foreach( int key, chapters_per_level.keys() )
    {
        if ( chapters_per_level[ key ] > maxvalue )
        {
            maxvalue     = chapters_per_level[ key ];
            maxvalue_key = key;
        }
    }

    return maxvalue_key;
}


// Sets include_in_toc to false to those 
// headings that appear above the root
QList< Headings::Heading > Headings::ExcludeHeadingsAboveRoot( const QList< Heading > &headings, int root_level )
{
    QList< Heading > included_headings = headings;

    for ( int i = 0; i < included_headings.count(); i++  )
    {
        if ( included_headings[ i ].level < root_level )

            included_headings[ i ].include_in_toc = false;
    }

    return included_headings;
}
