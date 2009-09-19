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

#pragma once
#ifndef HEADINGS_H
#define HEADINGS_H

#include <QString>

class Headings
{

public:

    // Describes a heading element found 
    // in the XHTML source code
    struct Heading
    {
        // The whole source of the element; for instance:
        // <h1 class="something">I'm a heading!</h1>
        QString element_source;

        // The value of the ID attribute of the heading;
        // NULL if heading doesn't have the attribute set
        QString id;

        // The text value of the heading; for instance:
        // I'm a heading!
        QString text;

        // The level of the heading, from 1 to 6
        // (lower number means 'bigger' heading )
        int level;

        // True if the heading appears within
        // 1000 chars after a chapter break
        bool after_chapter_break;

        // Should the heading be included in the TOC or not
        bool include_in_toc;

        // The headings 'below' this one
        // (appear after it in the source and
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

    // Constructs the new heading element source
    // created from the provided heading struct
    static QString GetNewHeadingSource( const Heading &heading );

    // Returns a list of headings from the provided XHTML source;
    // the list is flat, the headings are *not* in a hierarchy tree
    static QList< Heading > GetHeadingList( const QString &source ); 

    // Takes a flat list of headings and returns a list with those
    // headings sorted into a hierarchy
    static QList< Heading > MakeHeadingHeirarchy( const QList< Heading > &headings );

    // Takes a hierarchical list of headings and returns a flat list of them
    static QList< Heading > GetFlattenedHeadings( const QList< Heading > &headings );       

private:

    // Flattens the provided heading node and its children
    // into a list and returns it
    static QList< Heading > FlattenHeadingNode( Heading heading );

    // Returns true if the provided heading location 
    // appears within 1000 chars after a chapter break
    static bool IsAfterChapterBreak( const QString &source, int heading_location );     

    // Adds the new_child heading to the parent heading;
    // the new_child is propagated down the tree if necessary
    static void AddChildHeading( Heading &parent, Heading new_child );
};


// Enables us to store instances of the
// HeadingPointer struct inside of QVariants
Q_DECLARE_METATYPE( Headings::HeadingPointer );


#endif // HEADINGS_H


