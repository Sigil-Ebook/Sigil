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
#ifndef IMPORTHTML_H
#define IMPORTHTML_H

#include "Importer.h"
#include "BookManipulation/FolderKeeper.h"
#include "BookManipulation/XercesHUse.h"

class HTMLResource;
class CSSResource;
class QDomDocument;

class ImportHTML : public Importer
{

public:

    // Constructor;
    // The parameter is the file to be imported
    ImportHTML( const QString &fullfilepath );

    // Needed so that we can use an existing Book
    // in which to load HTML files (and their dependencies).
    void SetBook( QSharedPointer< Book > book, bool ignore_duplicates );

    // Reads and parses the file 
    // and returns the created Book.
    virtual QSharedPointer< Book > GetBook();

private:

    // Loads the source code into the Book
    QString LoadSource();      

    // Strips the file specifier on all the href attributes 
    // of anchor tags with filesystem links with fragment identifiers;
    // thus something like <a href="chapter01.html#firstheading" />
    // becomes just <a href="#firstheading" />
    void StripFilesFromAnchors( xc::DOMDocument &document );

    // Searches for meta information in the HTML file
    // and tries to convert it to Dublin Core
    void LoadMetadata( const xc::DOMDocument &document ); 

    HTMLResource& CreateHTMLResource();

    void UpdateFiles( HTMLResource &html_resource, 
                      xc::DOMDocument &document,
                      const QHash< QString, QString > &updates );

    // Loads the referenced files into the main folder of the book;
    // as the files get a new name, the references are updated 
    QHash< QString, QString > LoadFolderStructure( const xc::DOMDocument &document );

    // Returns a hash with keys being old references (URLs) to resources,
    // and values being the new references to those resources.
    QHash< QString, QString > LoadImages( const xc::DOMDocument *document );

    QHash< QString, QString > LoadStyleFiles( const xc::DOMDocument *document );


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    bool m_IgnoreDuplicates;
};

#endif // IMPORTHTML_H

