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
#ifndef IMPORTOEBPS_H
#define IMPORTOEBPS_H

#include "Importer.h"
#include "Misc/TempFolder.h"
#include <QHash>
#include <QStringList>

class HTMLResource;
class CSSResource;
class QDomDocument;
class QXmlStreamReader;

/**
 * The abstract base class for importers of OEBPS-like documents.
 */
class ImportOEBPS : public Importer
{

public:

    /**
     * Constructor.
     * 
     * @param fullfilepath The path to the file to be imported.
     */
    ImportOEBPS( const QString &fullfilepath );

    virtual QSharedPointer< Book > GetBook() = 0;

protected:
    
    /**
     * Extracts the EPUB file to a temporary folder.
     * The path to the the temp folder with the extracted files
     * is stored in m_ExtractedFolderPath.
     */
    void ExtractContainer();

    /**
     * Locates the OPF file in the extracted folder.
     * The path to the OPF is then stored in m_OPFFilePath.
     */
    void LocateOPF();
    
    /**
     * Parses the OPF file and stores the parsed information 
     * inside m_MetaElements, m_Files and m_ReadingOrderIds 
     */
    void ReadOPF();

    /**
     * Reads an <identifier> element.
     *
     * @param opf_reader The OPF reader positioned to read 
     *                   the required element type.
     */
    void ReadIdentifierElement( QXmlStreamReader &opf_reader );

    /**
     * Reads a manifest <item> element.
     *
     * @param opf_reader The OPF reader positioned to read 
     *                   the required element type.
     */
    void ReadManifestItemElement( QXmlStreamReader &opf_reader );

    /**
     * Reads a <spine> element.
     *
     * @param opf_reader The OPF reader positioned to read 
     *                   the required element type.
     */
    void ReadSpineElement( QXmlStreamReader &opf_reader );

    /**
     * Loads the book's infrastructure files, like 
     * the NCX and the OPF.
     */
    void LoadInfrastructureFiles();

    /**
     * Loads the referenced files into the main folder of the book. 
     *
     * @return A hash with keys being old references (URLs) to resources,
     *         and values being the new references to those resources.
     */ 
    QHash< QString, QString > LoadFolderStructure();

    /**
     * Loads a single file.
     * 
     * @param path A full path to the file to load.
     * @return A tuple where the first member is the old path to the file,
     *         and the new member is the new, OEBPS-relative path to it.
     */
    tuple< QString, QString > LoadOneFile( const QString &path );

    /**
     * Performs the necessary modifications to the OPF
     * source so that it can be read.
     *
     * @param source The XML source of the OPF.
     * @return The prepared source.
     */
    QString PrepareOPFForReading( const QString &source );
    

    ///////////////////////////////
    // PROTECTED MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The main temp folder where files are stored.
     */
    TempFolder m_TempFolder;
    
    /**
     * The full path to the folder where the 
     * EPUB was extracted to.
     */
    QString m_ExtractedFolderPath;

    /**
     * The full path to the OPF file 
     * of the publication.
     */ 
    QString m_OPFFilePath;

    /**
     * The full path to the NCX file 
     * of the publication.
     */ 
    QString m_NCXFilePath;

    /**
     * The map of all the files in the publication's 
     * manifest; The keys are the element ID's, 
     * the values are stored paths to the files.
     */ 
    QMap< QString, QString > m_Files;

    /**
     * InDesign likes listing several files multiple times in the manifest, 
     * even though that's explicitly forbidden by the spec. So we use this
     * to make sure we don't load such files multiple times.
     */
    QSet< QString > m_MainfestFilePaths;

    /**
     * The identifier of the book's unique identifier.
     */
    QString m_UniqueIdentifierId;

    /**
     * The value of the book's unique identifier.
     */
    QString m_UniqueIdentifierValue;

    /**
     * The value of the book's first UUID-based identifier.
     */
    QString m_UuidIdentifierValue;

    /**
     * It's theoretically possible (although unlikely) that an epub
     * will have more than one file listed in the OPF manifest with
     * an NCX mimetype. Only one of them will be the actual NCX though.
     * This hash stores all the candidates, as an ID-to-href mapping.
     */
    QHash< QString, QString > m_NcxCandidates;
};


#endif // IMPORTOEBPS_H
