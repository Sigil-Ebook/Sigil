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
#ifndef NCXWRITER_H
#define NCXWRITER_H

#include "XMLWriter.h"
#include "../BookManipulation/Headings.h"

class NCXWriter : private XMLWriter
{

public:

    // Constructor;
    // The first parameter is the book being exported,
    // and the second is the FolderKeeper object representing
    // the folder where the book will be exported
    NCXWriter( const Book &book, const FolderKeeper &fkeeper );

    // Returns the created XML file
    QString GetXML();
    
private:

    // Writes the <head> element
    void WriteHead();

    // Writes the <docTitle> element
    void WriteDocTitle();

    // Writes the <navMap> element
    void WriteNavMap();

    // Writes a fallback NavPoint for when the book has no headings
    void WriteFallbackNavPoint();

    // Called recursively to write the TOC tree;
    // the first parameter is the heading being written,
    // the second is a reference to the playorder NavPoints
    void WriteNavPoint( const Headings::Heading &heading, int &play_order );

    // Returns a hash that lists all the heading ID's
    // in a particular file
    QHash< QString, QStringList > GetHeadingIDsPerFile() const;

    // Returns a hash that lists all the heading sources
    // in a particular file. "Source" means full element HTML.
    QHash< QString, QStringList > GetHeadingSourcesPerFile() const;

    // Returns the relative path to the XHTML file
    // the provided heading is located in.
    QString GetHeadingFile( const Headings::Heading &heading ) const;

    // Returns the depth of the headings tree
    // specified in m_Headings
    int GetHeadingsDepth() const;

    // Used to walk through the headings tree and 
    // search for the it's maximum depth
    void DepthWalker( const Headings::Heading &heading, int &current_depth, int &max_depth ) const;

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // Lists all the heading ID's in a particular file;
    // the keys are the relative path names, and the values
    // are the ID's of all the headings in it
    const QHash< QString, QStringList > m_HeadingIDsPerFile;

    // Lists all the heading sources in a particular file;
    // the keys are the relative path names, and the values
    // are the HTML element sources of all the headings in it
    // USED FOR A MASSIVE HACK!
    const QHash< QString, QStringList > m_HeadingSourcesPerFile;

    // A hierarchical tree of all the headings in the book
    const QList< Headings::Heading > m_Headings;
};

#endif // NCXWRITER_H

