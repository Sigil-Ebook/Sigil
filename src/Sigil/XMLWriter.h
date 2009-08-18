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
#ifndef XMLWRITER_H
#define XMLWRITER_H

#include "FolderKeeper.h"

class Book;

class XMLWriter
{

public:

    // Constructor;
    // The first parameter is the book being exported,
    // and the second is the FolderKeeper object representing
    // the folder where the book will be exported
    XMLWriter( const Book &book, const FolderKeeper &fkeeper );

    // Destructor
    virtual ~XMLWriter();
    
    // Returns the created XML file
    virtual QString GetXML() = 0;
    
protected:

    // The book being exported
    const Book &m_Book;

    // Represents the folder where the book will be exported
    const FolderKeeper &m_Folder;

    // list of files in the folder
    // that will become the exported book
    const QStringList m_Files;	

    // The XML source that is being written
    QString m_Source;

    // The XML writer used to write XML
    QXmlStreamWriter *m_Writer;
};

#endif // XMLWRITER_H

