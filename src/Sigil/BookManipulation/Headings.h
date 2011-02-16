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

#pragma once
#ifndef HEADINGS_H
#define HEADINGS_H

#include "BookManipulation/XercesHUse.h"
#include <QString>

class HTMLResource;

class Headings
{

public:

    // Describes a heading element found 
    // in the XHTML source code
    struct Heading
    {
        // The HTMLResource file the heading belongs to
        HTMLResource* resource_file;

        // The DomElement of the heading in the file's DomDocument
        xc::DOMElement *element;

        // Represents what the heading should
        // look like in the TOC.
        QString text;

        // The level of the heading, from 1 to 6
        // (lower number means 'bigger' heading )
        int level;

        // True if the heading appears within
        // 20 lines after the body tag 
        // (only one heading per file)
        bool at_file_start;

        // Should the heading be included in the TOC or not
        bool include_in_toc;

        // The headings 'below' this one
        // (those that appear after it in the source and
        // are of higher level/smaller size)
        QList< Heading > children;
    };

    // A wrapper struct for the pointer
    // because of restrictions on what 
    // can be stored inside a QVariant
    struct HeadingPointer
    {
        Heading *heading;
    }; 


    // Returns a list of headings from the provided XHTML source;
    // the list is flat, the headings are *not* in a hierarchy tree.
    // Set include_unwanted_headings to true to get headings that the 
    // user has marked as unwanted.
    static QList< Heading > GetHeadingList( QList< HTMLResource* > html_resources,
                                            bool include_unwanted_headings = false ); 

    static QList< Heading > GetHeadingListForOneFile( HTMLResource* html_resource,
                                                      bool include_unwanted_headings = false );

    // Takes a flat list of headings and returns a list with those
    // headings sorted into a hierarchy
    static QList< Heading > MakeHeadingHeirarchy( const QList< Heading > &headings );

    // Takes a hierarchical list of headings and converts it into a flat list
    static QList< Heading > GetFlattenedHeadings( const QList< Heading > &headings );       

private:

    // Flattens the provided heading node and its children
    // into a list and returns it
    static QList< Heading > FlattenHeadingNode( Heading heading );   

    // Adds the new_child heading to the parent heading;
    // the new_child is propagated down the tree if necessary
    static void AddChildHeading( Heading &parent, Heading new_child );
};


// Enables us to store instances of the
// HeadingPointer struct inside of QVariants
Q_DECLARE_METATYPE( Headings::HeadingPointer );


#endif // HEADINGS_H


