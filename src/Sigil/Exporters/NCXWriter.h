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

#pragma once
#ifndef NCXWRITER_H
#define NCXWRITER_H

#include "XMLWriter.h"
#include "BookManipulation/Headings.h"

/**
 * Writes the NCX file of the EPUB publication.
 */
class NCXWriter : private XMLWriter
{

public:

    /**
     * Constructor.
     *
     * @param book The book for which we're writing the NCX.
     * @param device The IODevice into which we should write the XML.
     */
    NCXWriter( QSharedPointer< Book > book, QIODevice &device );

    void WriteXML();
    
private:

    /**
     *  Writes the <head> element.
     */
    void WriteHead();

    /**
     * Writes the <docTitle> element.
     */
    void WriteDocTitle();

    /**
     * Writes the <navMap> element.
     */
    void WriteNavMap();

    /**
     * Writes a fallback <navPoint> for when the book has no headings.
     */
    void WriteFallbackNavPoint();

    /**
     * Writes a <navPoint>; called recursively to write the TOC tree.
     *
     * @param heading The heading being written.
     * @param play_order A reference to the general <navPoints> playorder. 
     */
    void WriteNavPoint( const Headings::Heading &heading, int &play_order );

    /**
     * Returns the depth of the headings tree 
     * specified in m_Headings.
     *
     * @return The heading hierarchy depth.
     */
    int GetHeadingsDepth() const;

    /**
     * Used to walk through the headings tree and
     * search for the its maximum depth.
     *
     * @param heading The current heading we are walking through.
     * @param current_depth A reference to the depth of the current sub-tree.
     * @param max_depth A reference to the current maximum depth.
     */
    void DepthWalker( const Headings::Heading &heading, int &current_depth, int &max_depth ) const;


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * A hierarchical tree of all the headings in the book.
     */
    const QList< Headings::Heading > m_Headings;
};

#endif // NCXWRITER_H

