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
#ifndef EXPORTEPUB_H
#define EXPORTEPUB_H

#include "Exporter.h"
#include "FolderKeeper.h"
#include "Book.h"

class ExportEPUB : public Exporter
{

public:

    // Constructor;
    // the first parameter is the location where the book 
    // should be save to, and the second is the book to be saved
    ExportEPUB( const QString &fullfilepath, const Book &book );

    // Destructor
    virtual ~ExportEPUB();

    // Writes the book to the path 
    // specified in the constructor
    virtual void WriteBook();

protected:

    // Creates the publication from the Book
    // (creates XHTML, CSS, OPF, NCX files etc.)
    void virtual CreatePublication();

    // Saves the publication to the specified path
    void SaveTo( const QString &fullfilepath );

    // Creates style files from the style tags in the source
    // and returns a list of their file paths relative 
    // to the OEBPS folder in the FolderKeeper
    QStringList CreateStyleFiles();

    // Takes a list of style sheet file names 
    // and returns the header for XHTML files
    QString CreateHeader( const QStringList &cssfiles );

    // Creates XHTML files from the book source;
    // the provided header is used as the header of the created files
    void CreateXHTMLFiles( const QString &header );

    // Creates one text file from the provided source
    // and adds it to the FolderKeeper object with
    // the provided extension; returns the file path
    // relative to the OEBPS folder
    QString CreateOneTextFile( const QString &source, const QString &extension );

    // Strips CDATA declarations from the provided source
    QString StripCDATA( const QString &style_source );

    // Removes Sigil styles from the provided source
    QString RemoveSigilStyles( const QString &style_source );

    // Creates the publication's container.xml file
    void CreateContainerXML();

    // Creates the publication's content.opf file
    void CreateContentOPF();

    // Creates the publication's toc.ncx file
    void CreateTocNCX();


    ///////////////////////////////
    // PROTECTED MEMBER VARIABLES
    ///////////////////////////////

    // The location where the publication
    // should be exported to
    QString m_FullFilePath;

    // The book being exported
    Book m_Book;

    // The folder which contains all the files
    // that will end up in the final exported file
    FolderKeeper fkFolder;

};

#endif // EXPORTEPUB_H