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
#ifndef EXPORTEPUB_H
#define EXPORTEPUB_H

#include "Exporter.h"
#include "../BookManipulation/FolderKeeper.h"
#include "../BookManipulation/Book.h"



class ExportEPUB : public Exporter
{

public:

    // Constructor;
    // the first parameter is the location where the book 
    // should be save to, and the second is the book to be saved
    ExportEPUB( const QString &fullfilepath, QSharedPointer< Book > book );

    // Destructor
    virtual ~ExportEPUB();

    // Writes the book to the path 
    // specified in the constructor
    virtual void WriteBook();

private:

    // Creates the publication from the Book
    // (creates XHTML, CSS, OPF, NCX files etc.)
    void virtual CreatePublication( const QString &fullfolderpath );

    // Saves the publication in the specified folder 
    // to the specified file path as an epub;
    // the second optional parameter specifies the
    // mimetype to write to the special "mimetype" file
    void SaveFolderAsEpubToLocation( const QString &fullfolderpath, const QString &fullfilepath );

    // Creates the publication's container.xml file
    void CreateContainerXML( const QString &fullfolderpath );

    // Creates the publication's content.opf file
    void CreateContentOPF( const QString &fullfolderpath  );

    // Creates the publication's toc.ncx file
    void CreateTocNCX( const QString &fullfolderpath );


    ///////////////////////////////
    // PROTECTED MEMBER VARIABLES
    ///////////////////////////////

    // The location where the publication
    // should be exported to
    QString m_FullFilePath;

    // The book being exported
    QSharedPointer< Book > m_Book;

};

#endif // EXPORTEPUB_H

