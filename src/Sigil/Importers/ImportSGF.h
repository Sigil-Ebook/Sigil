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
#ifndef IMPORTSGF_H
#define IMPORTSGF_H

#include "ImportOEBPS.h"
#include "../BookManipulation/XHTMLDoc.h"

class Resource;

class ImportSGF : public ImportOEBPS
{

public:

    // Constructor;
    // The parameter is the file to be imported
    ImportSGF( const QString &fullfilepath );

    // Reads and parses the file 
    // and returns the created Book;
    // Overrides;
    QSharedPointer< Book > GetBook();

private:

    // Loads the source code into the Book
    QString LoadSource();

    // Creates style files from the style tags in the source
    // and returns a list of their file paths relative 
    // to the OEBPS folder in the FolderKeeper
    QList< Resource* > CreateStyleResources( const QString &source );

    Resource* CreateOneStyleFile( const XHTMLDoc::XMLElement &element, 
                                  const QString &folderpath, 
                                  int index );

    // Strips CDATA declarations from the provided source
    static QString StripCDATA( const QString &style_source );

    // Removes Sigil styles from the provided source
    static QString RemoveSigilStyles( const QString &style_source );

    // Takes a list of style sheet file names 
    // and returns the header for XHTML files
    static QString CreateHeader( const QList< Resource* > &style_resources );

    // Creates XHTML files from the book source;
    // the provided header is used as the header of the created files
    void CreateXHTMLFiles( const QString &source, 
                           const QString &header,
                           const QHash< QString, QString > &html_updates );

    void CreateOneXHTMLFile( QString source, 
                             int reading_order, 
                             const QString &folderpath,
                             const QHash< QString, QString > &html_updates );


};

#endif // IMPORTSGF_H

