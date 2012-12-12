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

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtXml/QXmlStreamReader>

#include "BookManipulation/Metadata.h"
#include "BookManipulation/FolderKeeper.h"
#include "Importers/ImportEPUB.h"
#include "Misc/FontObfuscation.h"
#include "Misc/Utility.h"
#include "ResourceObjects/CSSResource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "SourceUpdates/UniversalUpdates.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

const QString ADOBE_FONT_ALGO_ID = "http://ns.adobe.com/pdf/enc#RC";
const QString IDPF_FONT_ALGO_ID  = "http://www.idpf.org/2008/embedding";

// Constructor;
// The parameter is the file to be imported
ImportEPUB::ImportEPUB(const QString &fullfilepath)
    : ImportOEBPS(fullfilepath)
{
}


// Reads and parses the file
// and returns the created Book
QSharedPointer< Book > ImportEPUB::GetBook()
{
    if (!Utility::IsFileReadable(m_FullFilePath)) {
        boost_throw(EPUBLoadParseError() << errinfo_epub_load_parse_errors(QString(QObject::tr("Cannot read EPUB: %1")).arg(QDir::toNativeSeparators(m_FullFilePath)).toStdString()));
    }

    // These read the EPUB file
    ExtractContainer();
    QHash< QString, QString > encrypted_files = ParseEncryptionXml();

    if (BookContentEncrypted(encrypted_files)) {
        boost_throw(FileEncryptedWithDrm());
    }

    // These mutate the m_Book object
    LocateOPF();
    // These mutate the m_Book object
    ReadOPF();
    AddObfuscatedButUndeclaredFonts(encrypted_files);
    AddNonStandardAppleXML();
    LoadInfrastructureFiles();
    const QHash< QString, QString > &updates = LoadFolderStructure();
    const QList< Resource * > &resources      = m_Book->GetFolderKeeper().GetResourceList();
    const QStringList &load_errors = UniversalUpdates::PerformUniversalUpdates(false, resources, updates);

    Q_FOREACH (QString err, load_errors) {
        AddLoadWarning(QString("%1").arg(err));
    }

    ProcessFontFiles(resources, updates, encrypted_files);

    if (m_NCXNotInManifest) {
        // We manually created an NCX file because there wasn't one in the manifest.
        // Need to create a new manifest id for it.
        m_NCXId = m_Book->GetOPF().AddNCXItem(m_NCXFilePath);
    }

    // Ensure that our spine has a <spine toc="ncx"> element on it now in case it was missing.
    m_Book->GetOPF().UpdateNCXOnSpine(m_NCXId);
    // Make sure the <item> for the NCX in the manifest reflects correct href path
    m_Book->GetOPF().UpdateNCXLocationInManifest(m_Book->GetNCX());

    // If spine was not present or did not contain any items, recreate the OPF from scratch
    // preserving any important metadata elements and making a new reading order.
    if (!m_HasSpineItems) {
        QList< Metadata::MetaElement > originalMetadata = m_Book->GetOPF().GetDCMetadata();
        m_Book->GetOPF().AutoFixWellFormedErrors();
        m_Book->GetOPF().SetDCMetadata(originalMetadata);
        AddLoadWarning(QObject::tr("The OPF file does not contain a valid spine.") % "\n" %
                       QObject::tr("Sigil has created a new one for you."));
    }

    // If we have modified the book to add spine attribute, manifest item or NCX mark as changed.
    m_Book->SetModified(GetLoadWarnings().count() > 0);
    return m_Book;
}


QHash< QString, QString > ImportEPUB::ParseEncryptionXml()
{
    QString encrpytion_xml_path = m_ExtractedFolderPath + "/META-INF/encryption.xml";

    if (!QFileInfo(encrpytion_xml_path).exists()) {
        return QHash< QString, QString >();
    }

    QXmlStreamReader encryption(Utility::ReadUnicodeTextFile(encrpytion_xml_path));
    QHash< QString, QString > encrypted_files;
    QString encryption_algo;
    QString uri;

    while (!encryption.atEnd()) {
        encryption.readNext();

        if (encryption.isStartElement()) {
            if (encryption.name() == "EncryptionMethod") {
                encryption_algo = encryption.attributes().value("", "Algorithm").toString();
            } else if (encryption.name() == "CipherReference") {
                uri = m_ExtractedFolderPath + "/" +
                      Utility::URLDecodePath(encryption.attributes().value("", "URI").toString());
                encrypted_files[ uri ] = encryption_algo;
            }
        }
    }

    if (encryption.hasError()) {
        const QString error = QString(QObject::tr("Error parsing encryption xml.\nLine: %1 Column %2 - %3"))
                              .arg(encryption.lineNumber())
                              .arg(encryption.columnNumber())
                              .arg(encryption.errorString());
        boost_throw(EPUBLoadParseError() << errinfo_epub_load_parse_errors(error.toStdString()));
    }

    return encrypted_files;
}


bool ImportEPUB::BookContentEncrypted(const QHash< QString, QString > &encrypted_files)
{
    foreach(QString algorithm, encrypted_files.values()) {
        if (algorithm != ADOBE_FONT_ALGO_ID &&
            algorithm != IDPF_FONT_ALGO_ID) {
            return true;
        }
    }
    return false;
}


// This is basically a workaround for old versions of InDesign not listing the fonts it
// embedded in the OPF manifest, even though the specs say it has to.
// It does list them in the encryption.xml, so we use that.
void ImportEPUB::AddObfuscatedButUndeclaredFonts(const QHash< QString, QString > &encrypted_files)
{
    if (encrypted_files.empty()) {
        return;
    }

    QDir opf_dir = QFileInfo(m_OPFFilePath).dir();
    foreach(QString filepath, encrypted_files.keys()) {
        if (!FONT_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower())) {
            continue;
        }

        // Only add the path to the manifest if it is not already included.
        QMapIterator< QString, QString > valueSearch(m_Files);

        if (!valueSearch.findNext(opf_dir.relativeFilePath(filepath))) {
            m_Files[ Utility::CreateUUID() ] = opf_dir.relativeFilePath(filepath);
        }
    }
}


// Another workaround for non-standard Apple files
// At present it only handles com.apple.ibooks.display-options.xml, but any
// further iBooks aberrations should be handled here as well.
void ImportEPUB::AddNonStandardAppleXML()
{
    QDir opf_dir = QFileInfo(m_OPFFilePath).dir();
    QStringList aberrant_Apple_filenames;
    aberrant_Apple_filenames.append(m_ExtractedFolderPath + "/META-INF/com.apple.ibooks.display-options.xml");

    for (int i = 0 ; i < aberrant_Apple_filenames.size() ; ++i) {
        if (QFile::exists(aberrant_Apple_filenames.at(i))) {
            m_Files[ Utility::CreateUUID() ]  = opf_dir.relativeFilePath(aberrant_Apple_filenames.at(i));
        }
    }
}


// Each resource can provide us with its new path. encrypted_files provides
// a mapping from old resource paths to the obfuscation algorithms.
// So we use the updates hash which provides a mapping from old paths to new
// paths to match the resources to their algorithms.
void ImportEPUB::ProcessFontFiles(const QList< Resource * > &resources,
                                  const QHash< QString, QString > &updates,
                                  const QHash< QString, QString > &encrypted_files)
{
    if (encrypted_files.empty()) {
        return;
    }

    QList< FontResource * > font_resources = m_Book->GetFolderKeeper().GetResourceTypeList< FontResource >();

    if (font_resources.empty()) {
        return;
    }

    QHash< QString, QString > new_font_paths_to_algorithms;
    foreach(QString old_update_path, updates.keys()) {
        if (!FONT_EXTENSIONS.contains(QFileInfo(old_update_path).suffix().toLower())) {
            continue;
        }

        QString new_update_path = updates[ old_update_path ];
        foreach(QString old_encrypted_path, encrypted_files.keys()) {
            if (old_update_path == old_encrypted_path) {
                new_font_paths_to_algorithms[ new_update_path ] = encrypted_files[ old_encrypted_path ];
            }
        }
    }
    foreach(FontResource * font_resource, font_resources) {
        QString match_path = "../" + font_resource->GetRelativePathToOEBPS();
        QString algorithm  = new_font_paths_to_algorithms.value(match_path);

        if (algorithm.isEmpty()) {
            continue;
        }

        font_resource->SetObfuscationAlgorithm(algorithm);

        // Actually we are de-obfuscating, but the inverse operations of the obfuscation methods
        // are the obfuscation methods themselves. For the math oriented, the obfuscation methods
        // are involutary [ f( f( x ) ) = x ].
        if (algorithm == ADOBE_FONT_ALGO_ID) {
            FontObfuscation::ObfuscateFile(font_resource->GetFullPath(), algorithm, m_UuidIdentifierValue);
        } else {
            FontObfuscation::ObfuscateFile(font_resource->GetFullPath(), algorithm, m_UniqueIdentifierValue);
        }
    }
}







