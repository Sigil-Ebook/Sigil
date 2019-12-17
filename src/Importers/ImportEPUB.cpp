/************************************************************************
**
**  Copyright (C) 2016-2019 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#ifdef _WIN32
#define NOMINMAX
#endif

#include "unzip.h"
#ifdef _WIN32
#include "iowin32.h"
#endif

#include <string>

#include <QApplication>
#include <QtCore/QtCore>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QFutureSynchronizer>
#include <QtConcurrent/QtConcurrent>
#include <QtCore/QXmlStreamReader>
#include <QDirIterator>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStringList>
#include <QMessageBox>
#include <QUrl>
#include <QDebug>

#include "BookManipulation/FolderKeeper.h"
#include "BookManipulation/CleanSource.h"
#include "Importers/ImportEPUB.h"
#include "Misc/MediaTypes.h"
#include "Misc/FontObfuscation.h"
#include "Misc/HTMLEncodingResolver.h"
#include "Misc/QCodePage437Codec.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/CSSResource.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/OPFParser.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

#ifndef MAX_PATH
// Set Max length to 256 because that's the max path size on many systems.
#define MAX_PATH 256
#endif
// This is the same read buffer size used by Java and Perl.
#define BUFF_SIZE 8192

const QString DUBLIN_CORE_NS             = "http://purl.org/dc/elements/1.1/";
static const QString OEBPS_MIMETYPE      = "application/oebps-package+xml";
static const QString UPDATE_ERROR_STRING = "SG_ERROR";
const QString NCX_MIMETYPE               = "application/x-dtbncx+xml";
static const QString NCX_EXTENSION       = "ncx";
const QString ADOBE_FONT_ALGO_ID         = "http://ns.adobe.com/pdf/enc#RC";
const QString IDPF_FONT_ALGO_ID          = "http://www.idpf.org/2008/embedding";
static const QString CONTAINER_XML       = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
        "    <rootfiles>\n"
        "        <rootfile full-path=\"%1\" media-type=\"application/oebps-package+xml\"/>\n"
        "   </rootfiles>\n"
        "</container>\n";

static QCodePage437Codec *cp437 = 0;

// Constructor;
// The parameter is the file to be imported
ImportEPUB::ImportEPUB(const QString &fullfilepath)
    : Importer(fullfilepath),
      m_ExtractedFolderPath(m_TempFolder.GetPath()),
      m_HasSpineItems(false),
      m_NCXNotInManifest(false),
      m_NavResource(NULL)
{
}

// Reads and parses the file
// and returns the created Book
QSharedPointer<Book> ImportEPUB::GetBook(bool extract_metadata)
{
    QList<HTMLResource *> non_well_formed;
    SettingsStore ss;

    if (!Utility::IsFileReadable(m_FullFilePath)) {
        throw (EPUBLoadParseError(QString(QObject::tr("Cannot read EPUB: %1")).arg(QDir::toNativeSeparators(m_FullFilePath)).toStdString()));
    }

    // These read the EPUB file
    ExtractContainer();
    QHash<QString, QString> encrypted_files = ParseEncryptionXml();

    if (BookContentEncrypted(encrypted_files)) {
        throw (FileEncryptedWithDrm(""));
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    LocateOPF();
    m_opfDir = QFileInfo(m_OPFFilePath).dir();
    // These mutate the m_Book object
    ReadOPF();
    AddObfuscatedButUndeclaredFonts(encrypted_files);
    AddNonStandardAppleXML();

    m_Book->GetFolderKeeper()->SetGroupFolders(m_ManifestFilePaths, m_ManifestMediaTypes);

    LoadInfrastructureFiles();

    // Check for files missing in the Manifest and create warning
    QStringList notInManifest;
    foreach(QString file_path, m_ZipFilePaths) {
        // skip mimetype and anything in META-INF and the opf itself
        if (file_path == "mimetype") continue;
        if (file_path.startsWith("META-INF")) continue;
	if (m_OPFFilePath.contains(file_path)) continue;
	if (!m_ManifestFilePaths.contains(file_path)) {
	    notInManifest << file_path;
	}
    }
    
    if (!notInManifest.isEmpty()) {
        Utility::DisplayStdWarningDialog(tr("Files exist in epub that are not listed in the manifest, they will be ignored"), notInManifest.join("\n"));
    }

    LoadFolderStructure();

    const QList<Resource *> resources = m_Book->GetFolderKeeper()->GetResourceList();

    // We're going to check all html files until we find one that isn't well formed then we'll prompt
    // the user if they want to auto fix or not.
    //
    // If we have non-well formed content and they shouldn't be auto fixed we'll pass that on to
    // the universal update function so it knows to skip them. Otherwise we won't include them and
    // let it modify the file.
    for (int i=0; i<resources.count(); ++i) {
        if (resources.at(i)->Type() == Resource::HTMLResourceType) {
            HTMLResource *hresource = qobject_cast<HTMLResource *>(resources.at(i));
            if (!hresource) {
                continue;
            }
            // Load the content into the HTMLResource so we can perform a well formed check.
            try {
                hresource->SetText(HTMLEncodingResolver::ReadHTMLFile(hresource->GetFullPath()));
            } catch (...) {
                if (ss.cleanOn() & CLEANON_OPEN) {
                    non_well_formed << hresource;
                    continue;
                }
            }
            if (ss.cleanOn() & CLEANON_OPEN) {
                if (!XhtmlDoc::IsDataWellFormed(hresource->GetText(),hresource->GetEpubVersion())) {
                    non_well_formed << hresource;
                }
            }
        }
    }
    if (!non_well_formed.isEmpty()) {
        QApplication::restoreOverrideCursor();
        if (QMessageBox::Yes == QMessageBox::warning(QApplication::activeWindow(),
                tr("Sigil"),
                tr("This EPUB has HTML files that are not well formed. "
                   "Sigil can attempt to automatically fix these files, although this "
                   "can result in minor data loss.\n\n"
                   "Do you want to automatically fix the files?"),
                QMessageBox::Yes|QMessageBox::No)) 
        {
	    foreach(HTMLResource* htmlres, non_well_formed) {
		QString fixed_text = CleanSource::Mend(htmlres->GetText(),htmlres->GetEpubVersion());
                htmlres->SetText(fixed_text);
	    }
            non_well_formed.clear();
        }
        QApplication::setOverrideCursor(Qt::WaitCursor);
    }

    ProcessFontFiles(resources, encrypted_files);

    if (m_PackageVersion.startsWith('3')) {
        HTMLResource * nav_resource = NULL;
        if (m_NavResource) {
            if (m_NavResource->Type() == Resource::HTMLResourceType) {
                nav_resource = qobject_cast<HTMLResource*>(m_NavResource);
            }
        }
        if (!nav_resource) { 
            // we need to create a nav file here because one was not found
            // it will automatically be added to the content.opf
            nav_resource = m_Book->CreateEmptyNavFile(true);
            Resource * res = qobject_cast<Resource *>(nav_resource);
            m_Book->GetOPF()->SetItemRefLinear(res, false);
        }
        m_Book->GetOPF()->SetNavResource(nav_resource);
    }

    
    if (m_NCXNotInManifest && m_PackageVersion.startsWith('2')) {
        // We manually created an NCX file because there wasn't one in the manifest.
        // Need to create a new manifest id for it.
        m_NCXId = m_Book->GetOPF()->AddNCXItem(m_NCXFilePath);
    }

    NCXResource * ncxresource = m_Book->GetNCX();
 
    if (ncxresource) {
        // Ensure that our spine has a <spine toc="ncx"> element on it now in case it was missing.
        m_Book->GetOPF()->UpdateNCXOnSpine(m_NCXId);
        // Make sure the <item> for the NCX in the manifest reflects correct href path
        m_Book->GetOPF()->UpdateNCXLocationInManifest(ncxresource);
    }

    // If spine was not present or did not contain any items, recreate the OPF from scratch
    // preserving any important metadata elements and making a new reading order.
    if (!m_HasSpineItems) {
        QList<MetaEntry> originalMetadata = m_Book->GetOPF()->GetDCMetadata();
        m_Book->GetOPF()->AutoFixWellFormedErrors();
        if (extract_metadata) {
            m_Book->GetOPF()->SetDCMetadata(originalMetadata);
        }
        AddLoadWarning(QObject::tr("The OPF file does not contain a valid spine.") % "\n" %
                       QObject::tr("Sigil has created a new one for you."));
    }

    // update the ShortPathNames to reflect any name duplication
    m_Book->GetFolderKeeper()->updateShortPathNames();

    // since we no longer run universal updates we should run 
    // InitialLoad on all TextResources to make sure everything gets loaded
    m_Book->GetFolderKeeper()->PerformInitialLoads();

    // If we have modified the book to add spine attribute, manifest item or NCX mark as changed.
    m_Book->SetModified(GetLoadWarnings().count() > 0);
    QApplication::restoreOverrideCursor();
    return m_Book;
}


QHash<QString, QString> ImportEPUB::ParseEncryptionXml()
{
    QString encrpytion_xml_path = m_ExtractedFolderPath + "/META-INF/encryption.xml";

    if (!QFileInfo(encrpytion_xml_path).exists()) {
        return QHash<QString, QString>();
    }

    QXmlStreamReader encryption(Utility::ReadUnicodeTextFile(encrpytion_xml_path));
    QHash<QString, QString> encrypted_files;
    QString encryption_algo;
    QString uri;

    while (!encryption.atEnd()) {
        encryption.readNext();

        if (encryption.isStartElement()) {
            if (encryption.name() == "EncryptionMethod") {
                encryption_algo = encryption.attributes().value("", "Algorithm").toString();
            } else if (encryption.name() == "CipherReference") {
                uri = Utility::URLDecodePath(encryption.attributes().value("", "URI").toString());
                encrypted_files[ uri ] = encryption_algo;
            }
        }
    }

    if (encryption.hasError()) {
        const QString error = QString(QObject::tr("Error parsing encryption xml.\nLine: %1 Column %2 - %3"))
                              .arg(encryption.lineNumber())
                              .arg(encryption.columnNumber())
                              .arg(encryption.errorString());
        throw (EPUBLoadParseError(error.toStdString()));
    }

    return encrypted_files;
}


bool ImportEPUB::BookContentEncrypted(const QHash<QString, QString> &encrypted_files)
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
void ImportEPUB::AddObfuscatedButUndeclaredFonts(const QHash<QString, QString> &encrypted_files)
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
        QMapIterator<QString, QString> valueSearch(m_Files);

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

    for (int i = 0; i < aberrant_Apple_filenames.size(); ++i) {
        if (QFile::exists(aberrant_Apple_filenames.at(i))) {
            m_Files[ Utility::CreateUUID() ]  = opf_dir.relativeFilePath(aberrant_Apple_filenames.at(i));
        }
    }
}


// Each resource can provide us with its new path. encrypted_files provides
// a mapping from the resource paths to the obfuscation algorithms.
void ImportEPUB::ProcessFontFiles(const QList<Resource *> &resources,
                                  const QHash<QString, QString> &encrypted_files)
{
    if (encrypted_files.empty()) {
        return;
    }

    QList<FontResource *> font_resources = m_Book->GetFolderKeeper()->GetResourceTypeList<FontResource>();

    if (font_resources.empty()) {
        return;
    }

    foreach(FontResource * font_resource, font_resources) {
        QString match_path = font_resource->GetRelativePath();
        QString algorithm  = encrypted_files.value(match_path);

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

void ImportEPUB::ExtractContainer()
{
    int res = 0;
    if (!cp437) {
        cp437 = new QCodePage437Codec();
    }
#ifdef Q_OS_WIN32
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64W(&ffunc);
    unzFile zfile = unzOpen2_64(Utility::QStringToStdWString(QDir::toNativeSeparators(m_FullFilePath)).c_str(), &ffunc);
#else
    unzFile zfile = unzOpen64(QDir::toNativeSeparators(m_FullFilePath).toUtf8().constData());
#endif

    if (zfile == NULL) {
        throw (EPUBLoadParseError(QString(QObject::tr("Cannot unzip EPUB: %1")).arg(QDir::toNativeSeparators(m_FullFilePath)).toStdString()));
    }

    res = unzGoToFirstFile(zfile);

    if (res == UNZ_OK) {
        do {
            // Get the name of the file in the archive.
            char file_name[MAX_PATH] = {0};
            unz_file_info64 file_info;
            unzGetCurrentFileInfo64(zfile, &file_info, file_name, MAX_PATH, NULL, 0, NULL, 0);
            QString qfile_name;
            QString cp437_file_name;
            qfile_name = QString::fromUtf8(file_name);
            if (!(file_info.flag & (1<<11))) {
                // General purpose bit 11 says the filename is utf-8 encoded. If not set then
                // IBM 437 encoding might be used.
                cp437_file_name = cp437->toUnicode(file_name);
            }

            // If there is no file name then we can't do anything with it.
            if (!qfile_name.isEmpty()) {

	        // for security reasons against maliciously crafted zip archives
                // we need the file path to always be inside the target folder 
                // and not outside, so we will remove all illegal backslashes
                // and all relative upward paths segments "/../" from the zip's local 
                // file name/path before prepending the target folder to create 
                // the final path

	        QString original_path = qfile_name;
	        bool evil_or_corrupt_epub = false;

                if (qfile_name.contains("\\")) evil_or_corrupt_epub = true; 
	        qfile_name = "/" + qfile_name.replace("\\","");

                if (qfile_name.contains("/../")) evil_or_corrupt_epub = true;
	        qfile_name = qfile_name.replace("/../","/");

                while(qfile_name.startsWith("/")) { 
		    qfile_name = qfile_name.remove(0,1);
		}

                if (cp437_file_name.contains("\\")) evil_or_corrupt_epub = true; 
                cp437_file_name = "/" + cp437_file_name.replace("\\","");

                if (cp437_file_name.contains("/../")) evil_or_corrupt_epub = true;
	        cp437_file_name = cp437_file_name.replace("/../","/");

                while(cp437_file_name.startsWith("/")) { 
		    cp437_file_name = cp437_file_name.remove(0,1);
		}

                if (evil_or_corrupt_epub) {
                    unzCloseCurrentFile(zfile);
                    unzClose(zfile);
                    throw (EPUBLoadParseError(QString(QObject::tr("Possible evil or corrupt epub file name: %1")).arg(original_path).toStdString()));
                }

                // We use the dir object to create the path in the temporary directory.
                // Unfortunately, we need a dir object to do this as it's not a static function.
                QDir dir(m_ExtractedFolderPath);
                // Full file path in the temporary directory.
                QString file_path = m_ExtractedFolderPath + "/" + qfile_name;
                QFileInfo qfile_info(file_path);

                // Is this entry a directory?
                if (file_info.uncompressed_size == 0 && qfile_name.endsWith('/')) {
                    dir.mkpath(qfile_name);
                    continue;
                } else {
		    if (!qfile_info.path().isEmpty()) dir.mkpath(qfile_info.path());
		    // add it to the list of files found inside the zip
		    if (cp437_file_name.isEmpty()) {
		        m_ZipFilePaths << qfile_name;
		    } else {
                        m_ZipFilePaths << cp437_file_name;
		    }
                }

                // Open the file entry in the archive for reading.
                if (unzOpenCurrentFile(zfile) != UNZ_OK) {
                    unzClose(zfile);
                    throw (EPUBLoadParseError(QString(QObject::tr("Cannot extract file: %1")).arg(qfile_name).toStdString()));
                }

                // Open the file on disk to write the entry in the archive to.
                QFile entry(file_path);

                if (!entry.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    unzCloseCurrentFile(zfile);
                    unzClose(zfile);
                    throw (EPUBLoadParseError(QString(QObject::tr("Cannot extract file: %1")).arg(qfile_name).toStdString()));
                }

                // Buffered reading and writing.
                char buff[BUFF_SIZE] = {0};
                int read = 0;

                while ((read = unzReadCurrentFile(zfile, buff, BUFF_SIZE)) > 0) {
                    entry.write(buff, read);
                }

                entry.close();

                // Read errors are marked by a negative read amount.
                if (read < 0) {
                    unzCloseCurrentFile(zfile);
                    unzClose(zfile);
                    throw (EPUBLoadParseError(QString(QObject::tr("Cannot extract file: %1")).arg(qfile_name).toStdString()));
                }

                // The file was read but the CRC did not match.
                // We don't check the read file size vs the uncompressed file size
                // because if they're different there should be a CRC error.
                if (unzCloseCurrentFile(zfile) == UNZ_CRCERROR) {
                    unzClose(zfile);
                    throw (EPUBLoadParseError(QString(QObject::tr("Cannot extract file: %1")).arg(qfile_name).toStdString()));
                }
                if (!cp437_file_name.isEmpty() && cp437_file_name != qfile_name) {
                    QString cp437_file_path = m_ExtractedFolderPath + "/" + cp437_file_name;
                    QFile::copy(file_path, cp437_file_path);
                }
            }
        } while ((res = unzGoToNextFile(zfile)) == UNZ_OK);
    }

    if (res != UNZ_END_OF_LIST_OF_FILE) {
        unzClose(zfile);
        throw (EPUBLoadParseError(QString(QObject::tr("Cannot open EPUB: %1")).arg(QDir::toNativeSeparators(m_FullFilePath)).toStdString()));
    }

    unzClose(zfile);
}

void ImportEPUB::LocateOPF()
{
    QString fullpath = m_ExtractedFolderPath + "/META-INF/container.xml";
    QXmlStreamReader container;
    try {
        container.addData(Utility::ReadUnicodeTextFile(fullpath));
    } catch (CannotOpenFile) {
        // Find the first OPF file.
        QString OPFfile;
        QDirIterator files(m_ExtractedFolderPath, QStringList() << "*.opf", QDir::NoFilter, QDirIterator::Subdirectories);
        while (files.hasNext()) {
            OPFfile = QDir(m_ExtractedFolderPath).relativeFilePath(files.next());
            break;
        }

        if (OPFfile.isEmpty()) {
            std::string msg = fullpath.toStdString() + ": " + tr("Epub has missing or improperly specified OPF.").toStdString();
            throw (CannotOpenFile(msg));
        }

        // Create a default container.xml.
        QDir folder(m_ExtractedFolderPath);
        folder.mkdir("META-INF");
        Utility::WriteUnicodeTextFile(CONTAINER_XML.arg(OPFfile), fullpath);
        container.addData(Utility::ReadUnicodeTextFile(fullpath));
    }

    int num_opf = 0;

    while (!container.atEnd()) {
        container.readNext();

        if (container.isStartElement() && container.name() == "rootfile") {
            if (container.attributes().hasAttribute("media-type") &&
                container.attributes().value("", "media-type") == OEBPS_MIMETYPE) {
                // As per OCF spec, the first rootfile element
                // with the OEBPS mimetype is considered the "main" one.
	        if (m_OPFFilePath.isEmpty()) {
                    m_OPFFilePath = m_ExtractedFolderPath + "/" + container.attributes().value("", "full-path").toString();
		}
		num_opf++;

            }
        }
    }

    if (container.hasError()) {
        const QString error = QString(
                                  QObject::tr("Unable to parse container.xml file.\nLine: %1 Column %2 - %3"))
                              .arg(container.lineNumber())
                              .arg(container.columnNumber())
                              .arg(container.errorString());
        throw (EPUBLoadParseError(error.toStdString()));
    }

    if (num_opf > 1) {
        Utility::DisplayStdWarningDialog(tr("This epub has multiple renditions (multiple OPF files). Editing this epub in Sigil will produce a normal single rendition epub using only the main (first) OPF file found."),"");
    }

    if (m_OPFFilePath.isEmpty() || !QFile::exists(m_OPFFilePath)) {
        throw (EPUBLoadParseError(QString(QObject::tr("No appropriate OPF file found")).toStdString()));
    }
}


void ImportEPUB::ReadOPF()
{
    QString opf_text = CleanSource::ProcessXML(PrepareOPFForReading(Utility::ReadUnicodeTextFile(m_OPFFilePath)),OEBPS_MIMETYPE);
    QXmlStreamReader opf_reader(opf_text);
    QString ncx_id_on_spine;

    while (!opf_reader.atEnd()) {
        opf_reader.readNext();

        if (!opf_reader.isStartElement()) {
            continue;
        }

        if (opf_reader.name() == "package") {
            m_UniqueIdentifierId = opf_reader.attributes().value("", "unique-identifier").toString();
            m_PackageVersion = opf_reader.attributes().value("", "version").toString();
            if (m_PackageVersion == "1.0") m_PackageVersion = "2.0";
	}

        else if (opf_reader.name() == "identifier") {
            ReadIdentifierElement(&opf_reader);
	}

        // epub3 look for linked metadata resources that are included inside the epub 
        // but that are not and must not be included in the manifest
        else if (opf_reader.name() == "link") {
            ReadMetadataLinkElement(&opf_reader);
	}

        // Get the list of content files that
        // make up the publication
        else if (opf_reader.name() == "item") {
            ReadManifestItemElement(&opf_reader);
        }

        // We read this just to get the NCX id
        else if (opf_reader.name() == "spine") {
            ncx_id_on_spine = opf_reader.attributes().value("", "toc").toString();
        } 

        else if (opf_reader.name() == "itemref") {
            m_HasSpineItems = true;
        }
    }

    if (opf_reader.hasError()) {
        const QString error = QString(QObject::tr("Unable to read OPF file.\nLine: %1 Column %2 - %3"))
                              .arg(opf_reader.lineNumber())
                              .arg(opf_reader.columnNumber())
                              .arg(opf_reader.errorString());
        throw (EPUBLoadParseError(error.toStdString()));
    }

    
    //Important!  The OPF Resource in the new book must be created now before adding to it in any way
    QString bookpath;
    bookpath = m_OPFFilePath.right(m_OPFFilePath.length() - m_ExtractedFolderPath.length() - 1);
    m_Book->GetFolderKeeper()->AddOPFToFolder(m_PackageVersion, bookpath);

    // Ensure we have an NCX available
    LocateOrCreateNCX(ncx_id_on_spine);

}


void ImportEPUB::ReadIdentifierElement(QXmlStreamReader *opf_reader)
{
    QString id     = opf_reader->attributes().value("", "id").toString();
    QString scheme = opf_reader->attributes().value("", "scheme").toString();
    QString value  = opf_reader->readElementText();

    if (id == m_UniqueIdentifierId) {
        m_UniqueIdentifierValue = value;
    }

    if (m_UuidIdentifierValue.isEmpty() &&
        (value.contains("urn:uuid:") || scheme.toLower() == "uuid")) {
        m_UuidIdentifierValue = value;
    }
}

void ImportEPUB::ReadMetadataLinkElement(QXmlStreamReader *opf_reader)
{
    QString relation = opf_reader->attributes().value("", "rel").toString();
    QString mtype = opf_reader->attributes().value("", "media-type").toString();
    QString props = opf_reader->attributes().value("", "properties").toString();
    QString href = opf_reader->attributes().value("", "href").toString();
    if (!href.isEmpty()) {
        QUrl url = QUrl(href);
        if (url.isRelative()) {
	    // we have a local unmanifested metadata file to handle
	    // attempt to map deprecated record types into proper media-types
	    if (relation == "marc21xml-record") {
                mtype = "application/marcxml+xml";
	    }
	    else if (relation == "mods-record") {
                mtype = "application/mods+xml";
	    }
	    else if (relation == "onix-record") {
                mtype = "application/xml;onix";
            }
	    else if (relation == "xmp-record") {
                mtype = "application/xml;xmp";
	    }
            else if (relation == "record") {
                if (props == "onix") mtype = "application/xml;onix";
                if (props == "xmp") mtype = "application/xml;xmp";
	    }
            QDir opf_dir = QFileInfo(m_OPFFilePath).dir();
	    QString path = opf_dir.absolutePath() + "/" + url.path();
	    if (QFile::exists(path)) {
	        QString id = Utility::CreateUUID();
		m_Files[ id ]  = opf_dir.relativeFilePath(path);
		m_FileMimetypes[ id ] = mtype;
	    }
	}
    }
}

void ImportEPUB::ReadManifestItemElement(QXmlStreamReader *opf_reader)
{
    QString id   = opf_reader->attributes().value("", "id").toString();
    QString href = opf_reader->attributes().value("", "href").toString();
    QString type = opf_reader->attributes().value("", "media-type").toString();
    QString properties = opf_reader->attributes().value("", "properties").toString();
    // Paths are percent encoded in the OPF, we use "normal" paths internally.
    href = Utility::URLDecodePath(href);
    QString extension = QFileInfo(href).suffix().toLower();

    // validate the media type if we can, and warn otherwise
    QString group = MediaTypes::instance()->GetGroupFromMediaType(type,"");
    QString ext_mtype = MediaTypes::instance()->GetMediaTypeFromExtension(extension, "");
    if (type.isEmpty() || group.isEmpty()) {
	const QString load_warning = QObject::tr("The OPF uses an unrecognized media type \"%1\" for file \"%2\"").arg(type).arg(QFileInfo(href).fileName()) +
	    " - " + QObject::tr("A temporary media type of \"%1\" has been assigned. You should edit your OPF file to fix this problem.").arg(ext_mtype);
        AddLoadWarning(load_warning);
    }

    // find the epub root relative file path from the opf location and the item href
    QString file_path = m_opfDir.absolutePath() + "/" + href;
    file_path = Utility::resolveRelativeSegmentsInFilePath(file_path,"/");
    file_path = file_path.remove(0, m_ExtractedFolderPath.length() + 1); 
    
    if (type != NCX_MIMETYPE && extension != NCX_EXTENSION) {
        if (!m_ManifestFilePaths.contains(file_path)) {
            if (m_Files.contains(id)) {
                // We have an error situation with a duplicate id in the epub.
                // We must warn the user, but attempt to use another id so the epub can still be loaded.
                QString base_id = QFileInfo(href).fileName();
                QString new_id(base_id);
                int duplicate_index = 0;

                while (m_Files.contains(new_id)) {
                    duplicate_index++;
                    new_id = QString("%1%2").arg(base_id).arg(duplicate_index);
                }

                const QString load_warning = QObject::tr("The OPF manifest contains duplicate ids for: %1").arg(id) +
                  " - " + QObject::tr("A temporary id has been assigned to load this EPUB. You should edit your OPF file to remove the duplication.");
                id = new_id;
                AddLoadWarning(load_warning);
            }

            m_Files[ id ] = href;
            m_FileMimetypes[ id ] = type;
            m_ManifestFilePaths << file_path;
	    m_ManifestMediaTypes << type;

            // store information about any nav document
            if (properties.contains("nav")) {
                m_NavId = id;
                m_NavHref = href;
            }
        }
    } else {
        m_NcxCandidates[ id ] = href;
	m_ManifestFilePaths << file_path;
	m_ManifestMediaTypes << type;
    }
}


void ImportEPUB::LocateOrCreateNCX(const QString &ncx_id_on_spine)
{
    QString load_warning;
    QString ncx_href = "";
    m_NCXId = ncx_id_on_spine;

    // handle the normal/proper case of an ncx id on the spine matching an ncx candidate that exists
    if (!m_NCXId.isEmpty() && m_NcxCandidates.contains(m_NCXId)) {
        QString bookpath;
        ncx_href = m_NcxCandidates[ m_NCXId ];
        m_NCXFilePath = QFileInfo(m_OPFFilePath).absolutePath() % "/" % ncx_href;
	m_NCXFilePath = Utility::resolveRelativeSegmentsInFilePath(m_NCXFilePath, "/");
	bookpath = m_NCXFilePath.right(m_NCXFilePath.length() - m_ExtractedFolderPath.length() - 1);
        m_Book->GetFolderKeeper()->AddNCXToFolder(m_PackageVersion, bookpath);
        m_NCXNotInManifest = false;
        return;
    }

    bool found = false;

    // now handle ncx not specified in spine but file with ncx extension exists in manifest
    // Search for the ncx in the manifest by looking for files with
   // a .ncx extension.
    if (m_NCXId.isEmpty()) {

        QHashIterator<QString, QString> ncxSearch(m_NcxCandidates);
        while (ncxSearch.hasNext()) {
            ncxSearch.next();

            if (QFileInfo(ncxSearch.value()).suffix().toLower() == NCX_EXTENSION) {
                // we found a file with an ncx extension
                m_NCXId = ncxSearch.key();
                found = true;
		break;
            }
        }
    }

    if (found) {
        // m_NCXId has been properly set
        ncx_href = m_NcxCandidates[ m_NCXId ];
        m_NCXFilePath = QFileInfo(m_OPFFilePath).absolutePath() % "/" % ncx_href;
	m_NCXFilePath = Utility::resolveRelativeSegmentsInFilePath(m_NCXFilePath, "/");

	QString bookpath = m_NCXFilePath.right(m_NCXFilePath.length() - m_ExtractedFolderPath.length() - 1);
        m_Book->GetFolderKeeper()->AddNCXToFolder(m_PackageVersion, bookpath);
        m_NCXNotInManifest = false;
        load_warning = QObject::tr("The OPF file did not identify the NCX file correctly.") + "\n" + 
                               " - "  +  QObject::tr("Sigil has used the following file as the NCX:") + 
                               QString(" %1").arg(m_NcxCandidates[ m_NCXId ]);

        AddLoadWarning(load_warning);
        return;
    }

    // An NCX is only required in epub2 so punt here if epub3
    if ( m_PackageVersion.startsWith('3') ) return;

    // epub2 only here

    // If we reached here there is no file with an ncx file extension in the manifest
    // There might be a file with an ncx extension inside the epub zip folder but 
    // since it was unmanifested, we will not use it anyway.
    // So we need to create a new one and thereby handle the following 
    // failure conditions:
    //     - ncx specified in spine, but no matching manifest item entry
    //     - ncx file not physically present
    //     - ncx not in spine or manifest item

    m_NCXNotInManifest = true;

    load_warning = QObject::tr("The OPF file does not contain an NCX file.") + "\n" + 
                               " - " +  QObject::tr("Sigil has created a new one for you.");

    m_NCXFilePath = QFileInfo(m_OPFFilePath).absolutePath() + "/toc.ncx";

    // Create a new file for the NCX in the *Extracted Folder* Path
    // We are relying on an identifier being set from the metadata.
    // It might not have one if the book does not have the urn:uuid: format.
    NCXResource ncx_resource(m_ExtractedFolderPath, m_NCXFilePath, m_PackageVersion, NULL);
    ncx_resource.SetEpubVersion(m_PackageVersion);
    // put it beside the OPF file
    ncx_resource.FillWithDefaultText(m_PackageVersion, QFileInfo(m_OPFFilePath).absolutePath());
    if (!m_UuidIdentifierValue.isEmpty()) {
        ncx_resource.SetMainID(m_UuidIdentifierValue);
    }
    ncx_resource.SaveToDisk();

    // now add the NCX to our folder
    QString bookpath = m_NCXFilePath.right(m_NCXFilePath.length() - m_ExtractedFolderPath.length() - 1);
    m_Book->GetFolderKeeper()->AddNCXToFolder(m_PackageVersion, bookpath);

    if (!load_warning.isEmpty()) {
        AddLoadWarning(load_warning);
    }
}


void ImportEPUB::LoadInfrastructureFiles()
{
    // always SetEpubVersion before SetText in OPF as SetText will validate with it
    m_Book->GetOPF()->SetEpubVersion(m_PackageVersion);
    m_Book->GetOPF()->SetText(CleanSource::ProcessXML(PrepareOPFForReading(Utility::ReadUnicodeTextFile(m_OPFFilePath)),OEBPS_MIMETYPE));
    QString OPFBookRelPath = m_OPFFilePath;
    OPFBookRelPath = OPFBookRelPath.remove(0,m_ExtractedFolderPath.length()+1);
    m_Book->GetOPF()->SetCurrentBookRelPath(OPFBookRelPath);
    NCXResource * ncxresource = m_Book->GetNCX();
    if (ncxresource) {
        ncxresource->SetEpubVersion(m_PackageVersion);
        ncxresource->SetText(CleanSource::ProcessXML(Utility::ReadUnicodeTextFile(m_NCXFilePath),"application/x-dtbncx+xml"));
        QString NCXBookRelPath = m_NCXFilePath;
        NCXBookRelPath = NCXBookRelPath.remove(0,m_ExtractedFolderPath.length()+1);
        ncxresource->SetCurrentBookRelPath(NCXBookRelPath);
    }
}


bool ImportEPUB::LoadFolderStructure()
{
    QList<QString> keys = m_Files.keys();
    int num_files = keys.count();
    bool success = true;

    QFutureSynchronizer<std::tuple<QString, QString>> sync;

    for (int i = 0; i < num_files; ++i) {
        QString id = keys.at(i);
        sync.addFuture(QtConcurrent::run(
                           this,
                           &ImportEPUB::LoadOneFile,
                           m_Files.value(id),
                           m_FileMimetypes.value(id)));
    }

    sync.waitForFinished();
    QList<QFuture<std::tuple<QString, QString>>> futures = sync.futures();
    int num_futures = futures.count();

    for (int i = 0; i < num_futures; ++i) {
        std::tuple<QString, QString> result = futures.at(i).result();
	if (std::get<0>(result) != std::get<1>(result)) {
	    qDebug() << "LoadFolderStructure Issue: " << std::get<0>(result) << std::get<1>(result);
	    success = false;
	}
    }

    return success;
}


std::tuple<QString, QString> ImportEPUB::LoadOneFile(const QString &path, const QString &mimetype)
{
    // Use opf relative href to create the book path (currentpath) for this file
    QString fullfilepath = QDir::cleanPath(QFileInfo(m_OPFFilePath).absolutePath() + "/" + path);
    QString currentpath = fullfilepath;
    currentpath = currentpath.remove(0,m_ExtractedFolderPath.length()+1);
    try {
        QString bookpath = currentpath;
        Resource *resource = m_Book->GetFolderKeeper()->AddContentFileToFolder(fullfilepath, false, mimetype, bookpath);
        if (path == m_NavHref) {
            m_NavResource = resource;
        }
        QString newpath = resource->GetRelativePath();
        return std::make_tuple(currentpath, newpath);
    } catch (FileDoesNotExist) {
        return std::make_tuple(UPDATE_ERROR_STRING, UPDATE_ERROR_STRING);
    }
}


QString ImportEPUB::PrepareOPFForReading(const QString &source)
{
    QString source_copy(source);
    QString prefix = source_copy.left(XML_DECLARATION_SEARCH_PREFIX_SIZE);
    QRegularExpression version(VERSION_ATTRIBUTE);
    QRegularExpressionMatch mo = version.match(prefix);
    if (mo.hasMatch()) {
        // MASSIVE hack for XML 1.1 "support";
        // this is only for people who specify
        // XML 1.1 when they actually only use XML 1.0
        source_copy.replace(mo.capturedStart(1), mo.capturedLength(1), "1.0");
    }
    return source_copy;
}
