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
#ifndef IMPORTHTML_H
#define IMPORTHTML_H

#include "Importer.h"
#include "../BookManipulation/FolderKeeper.h"

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
    void SetBook( QSharedPointer< Book > book );

    // Reads and parses the file 
    // and returns the created Book.
    virtual QSharedPointer< Book > GetBook();

private:

    // Loads the source code into the Book
    QString LoadSource();  

    // Resolves custom ENTITY declarations
    QString ResolveCustomEntities( const QString &html_source ) const;

    // Strips the file specifier on all the href attributes 
    // of anchor tags with filesystem links with fragment identifiers;
    // thus something like <a href="chapter01.html#firstheading" />
    // becomes just <a href="#firstheading" />
    void StripFilesFromAnchors( QDomDocument &document );

    // Searches for meta information in the HTML file
    // and tries to convert it to Dublin Core
    void LoadMetadata( const QDomDocument &document ); 

    HTMLResource& CreateHTMLResource();

    void UpdateFiles( HTMLResource &html_resource, 
                      QDomDocument &document,
                      const QHash< QString, QString > &updates );

    // Loads the referenced files into the main folder of the book;
    // as the files get a new name, the references are updated 
    QHash< QString, QString > LoadFolderStructure( const QDomDocument &document );

    // Loads CSS files from link tags to style tags.
    // Returns a hash with keys being old references (URLs) to resources,
    // and values being the new references to those resources.
    QHash< QString, QString > LoadImages( const QDomDocument &document );

    // Loads style files from link tags to style tags
    QHash< QString, QString > LoadStyleFiles( const QDomDocument &document );
};

#endif // IMPORTHTML_H

