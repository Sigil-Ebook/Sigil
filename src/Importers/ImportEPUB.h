/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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
#ifndef IMPORTEPUB_H
#define IMPORTEPUB_H

#include <QCoreApplication>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QStringList>

#include "Importers/Importer.h"
#include "Misc/TempFolder.h"

class HTMLResource;
class CSSResource;
class QXmlStreamReader;

class ImportEPUB : public Importer
{
    Q_DECLARE_TR_FUNCTIONS(ImportEPUB)

public:
    // The parameter is the file to be imported
    ImportEPUB(const QString &fullfilepath);

    // Reads and parses the file
    // and returns the created Book
    virtual QSharedPointer<Book> GetBook();

private:
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
    void ReadIdentifierElement(QXmlStreamReader *opf_reader);

    /**
     * Reads a manifest <item> element.
     *
     * @param opf_reader The OPF reader positioned to read
     *                   the required element type.
     */
    void ReadManifestItemElement(QXmlStreamReader *opf_reader);

    /**
     * Locate or create an NCX file if missing or not correctly specified.
     *
     * @param ncx_id_on_spine - The toc attribute value from the <spine>
     */
    void LocateOrCreateNCX(const QString &ncx_id_on_spine);

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
    QHash<QString, QString> LoadFolderStructure();

    /**
     * Loads a single file.
     *
     * @param path A full path to the file to load.
     * @param mimetype The mimetype of the file to load.
     * @return A tuple where the first member is the old path to the file,
     *         and the new member is the new, OEBPS-relative path to it.
     */
    std::tuple<QString, QString> LoadOneFile(const QString &path,
                                        const QString &mimetype = QString());

    /**
     * Performs the necessary modifications to the OPF
     * source so that it can be read.
     *
     * @param source The XML source of the OPF.
     * @return The prepared source.
     */
    QString PrepareOPFForReading(const QString &source);

    /**
     * Parses the "encryption.xml" file in the META-INF folder.
     * We return the list of file paths and the algorithms used
     * to encrypt them.
     *
     * @return The list of encrypted fsiles. The keys are the
     *         absolute paths to the files and the values are the
     *         encryption algorithm IDs.
     */
    QHash<QString, QString> ParseEncryptionXml();

    bool BookContentEncrypted(const QHash<QString, QString> &encrypted_files);

    void AddObfuscatedButUndeclaredFonts(const QHash<QString, QString> &encrypted_files);

    /**
     * Another workaround function to handle com.apple.ibooks.display-options.xml
     * and any future non-standard Apple xml. If additional files need to
     * be excluded from being handled as proper ePub content, you will also
     * need to alter FILE_EXCEPTIONS at the top of FolderKeeper.cpp so the regex
     * will detect them.
     * This is not added to the manifest, but epubcheck uses a similar exception
     * and accepts ePubs containing an unmanifested file of this name.
     */
    void AddNonStandardAppleXML();

    void ProcessFontFiles(const QList<Resource *> &resources,
                          const QHash<QString, QString> &updates,
                          const QHash<QString, QString> &encrypted_files);

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
    QMap<QString, QString> m_Files;

    /**
     * The map of all files in the publication's manifest;
     * The keys are the element ID's, the vaules are the
     * mimetype of the file.
     */
    QMap<QString, QString> m_FileMimetypes;

    /**
     * InDesign likes listing several files multiple times in the manifest,
     * even though that's explicitly forbidden by the spec. So we use this
     * to make sure we don't load such files multiple times.
     */
    QSet<QString> m_MainfestFilePaths;

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
    QHash<QString, QString> m_NcxCandidates;

    bool m_HasSpineItems;
    bool m_NCXNotInManifest;
    QString m_NCXId;

    /**
     * The value of the opf package version tag
     */
    QString m_PackageVersion;

};

#endif // IMPORTEPUB_H

