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
#ifndef IMPORTOEBPS_H
#define IMPORTOEBPS_H

#include "Importer.h"
#include "../BookManipulation/Metadata.h"
#include <QHash>
#include <QStringList>

class HTMLResource;
class CSSResource;
class QDomDocument;

class ImportOEBPS : public Importer
{

public:

    // Constructor;
    // The parameter is the file to be imported
    ImportOEBPS( const QString &fullfilepath );

    // Destructor
    virtual ~ImportOEBPS();

    virtual QSharedPointer< Book > GetBook() = 0;

protected:

    // Extracts the EPUB file to a temporary folder;
    // the path to this folder is stored in m_ExtractedFolderPath
    void ExtractContainer();

    // Locates the OPF file in the extracted folder;
    // the path to the OPF is stored in m_OPFFilePath
    void LocateOPF();

    // Parses the OPF file and stores the parsed information
    // inside m_MetaElements, m_Files and m_ReadingOrderIds 
    void ReadOPF();

    // Loads the metadata from the m_MetaElements list
    // (filled by reading the OPF) into the book
    void LoadMetadata();

    // Loads the referenced files into the main folder of the book.
    // Returns a hash with keys being old references (URLs) to resources,
    // and values being the new references to those resources.
    QHash< QString, QString > LoadFolderStructure();

    tuple< QString, QString > LoadOneFile( const QString &key );


    ///////////////////////////////
    // PROTECTED MEMBER VARIABLES
    ///////////////////////////////

    // The full path to the folder where the
    // EPUB was extracted to
    QString m_ExtractedFolderPath;

    // The full path to the OPF file
    // of the publication
    QString m_OPFFilePath;

    // The map of all the files in the publication's
    // manifest; The keys are the element ID's, 
    // the values are stored paths to the files
    QMap< QString, QString > m_Files;

    // The list of ID's to the files in the manifest
    // that represent the reading order of the publication
    QStringList m_ReadingOrderIds;

    // The list of metadata elements in the OPF
    QList< Metadata::MetaElement > m_MetaElements;


};


#endif // IMPORTOEBPS_H
