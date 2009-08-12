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
#ifndef IMPORTHTML_H
#define IMPORTHTML_H

#include "ImportTXT.h"
#include "FolderKeeper.h"

class ImportHTML : public ImportTXT
{

public:

    // Constructor;
    // The parameter is the file to be imported
    ImportHTML( const QString &fullfilepath );

    // Reads and parses the file 
    // and returns the created Book
    virtual Book GetBook();

protected:

    // Returns a style tag created 
    // from the provided path to a CSS file
    QString CreateStyleTag( const QString &fullfilepath );

    // Updates all references to the resource specified with oldpath
    // to the path of the new resource specified with newpath
    void UpdateReferences( const QString &oldpath, const QString &newpath );

    // Resolves custom ENTITY declarations
    QString ResolveCustomEntities( const QString &html_source ); 

    // Strips the file specifier on all the href attributes 
    // of anchor tags with filesystem links with fragment identifiers;
    // thus something like <a href="chapter01.html#firstheading" />
    // becomes just <a href="#firstheading" />
    void StripFilesFromAnchors();

private:

    // Loads the source code into the Book
    void LoadSource();    

    // Loads the referenced files into the main folder of the book;
    // as the files get a new name, the references are updated 
    void LoadFolderStructure();
};

#endif // IMPORTHTML_H

