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

#ifdef _WIN32
#define NOMINMAX
#endif

#include <string.h>
#include <zip.h>
#ifdef _WIN32
#include <iowin32.h>
#endif

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTemporaryFile>
#include <QtCore/QTextStream>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/FolderKeeper.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Exporters/EncryptionXmlWriter.h"
#include "Exporters/ExportEPUB.h"
#include "Misc/Utility.h"
#include "Misc/TempFolder.h"
#include "Misc/FontObfuscation.h"
#include "ResourceObjects/FontResource.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

#define BUFF_SIZE 8192

const QString BODY_START = "<\\s*body[^>]*>";
const QString BODY_END   = "</\\s*body\\s*>";

const QString OPF_FILE_NAME            = "content.opf";
const QString NCX_FILE_NAME            = "toc.ncx";
const QString CONTAINER_XML_FILE_NAME  = "container.xml";
const QString ENCRYPTION_XML_FILE_NAME = "encryption.xml";

static const QString METAINF_FOLDER_SUFFIX = "/META-INF";
static const QString OEBPS_FOLDER_SUFFIX   = "/OEBPS";

static const char * EPUB_MIME_DATA = "application/epub+zip";


// Constructor;
// the first parameter is the location where the book
// should be save to, and the second is the book to be saved
ExportEPUB::ExportEPUB(const QString &fullfilepath, QSharedPointer<Book> book)
    :
    m_FullFilePath(fullfilepath),
    m_Book(book)
{
}


// Destructor
ExportEPUB::~ExportEPUB()
{
}


// Writes the book to the path
// specified in the constructor
void ExportEPUB::WriteBook()
{
    // Obfuscating fonts needs an UUID ident
    if (m_Book->HasObfuscatedFonts()) {
        m_Book->GetOPF().EnsureUUIDIdentifierPresent();
    }

    m_Book->GetOPF().AddSigilVersionMeta();
    m_Book->GetOPF().AddModificationDateMeta();
    m_Book->SaveAllResourcesToDisk();
    TempFolder tempfolder;
    CreatePublication(tempfolder.GetPath());

    if (m_Book->HasObfuscatedFonts()) {
        ObfuscateFonts(tempfolder.GetPath());
    }

    SaveFolderAsEpubToLocation(tempfolder.GetPath(), m_FullFilePath);
}


// Creates the publication from the Book
// (creates XHTML, CSS, OPF, NCX files etc.)
void ExportEPUB::CreatePublication(const QString &fullfolderpath)
{
    Utility::CopyFiles(m_Book->GetFolderKeeper().GetFullPathToMainFolder(), fullfolderpath);

    if (m_Book->HasObfuscatedFonts()) {
        CreateEncryptionXML(fullfolderpath + METAINF_FOLDER_SUFFIX);
    }
}

void ExportEPUB::SaveFolderAsEpubToLocation(const QString &fullfolderpath, const QString &fullfilepath)
{
    QString tempFile = fullfolderpath + "-tmp.epub";
    QDateTime timeNow = QDateTime::currentDateTime();
    zip_fileinfo fileInfo;
#ifdef Q_OS_WIN32
    zlib_filefunc64_def ffunc;
    fill_win32_filefunc64W(&ffunc);
    zipFile zfile = zipOpen2_64(Utility::QStringToStdWString(QDir::toNativeSeparators(tempFile)).c_str(), APPEND_STATUS_CREATE, NULL, &ffunc);
#else
    zipFile zfile = zipOpen64(QDir::toNativeSeparators(tempFile).toUtf8().constData(), APPEND_STATUS_CREATE);
#endif

    if (zfile == NULL) {
        boost_throw(CannotOpenFile() << errinfo_file_fullpath(tempFile.toStdString()));
    }

    memset(&fileInfo, 0, sizeof(fileInfo));
    fileInfo.tmz_date.tm_sec = timeNow.time().second();
    fileInfo.tmz_date.tm_min = timeNow.time().minute();
    fileInfo.tmz_date.tm_hour = timeNow.time().hour();
    fileInfo.tmz_date.tm_mday = timeNow.date().day();
    fileInfo.tmz_date.tm_mon = timeNow.date().month() - 1;
    fileInfo.tmz_date.tm_year = timeNow.date().year();

    // Write the mimetype. This must be uncompressed and the first entry in the archive.
    if (zipOpenNewFileInZip64(zfile, "mimetype", &fileInfo, NULL, 0, NULL, 0, NULL, Z_NO_COMPRESSION, 0, 0) != Z_OK) {
        zipClose(zfile, NULL);
        QFile::remove(tempFile);
        boost_throw(CannotStoreFile() << errinfo_file_fullpath("mimetype"));
    }

    if (zipWriteInFileInZip(zfile, EPUB_MIME_DATA, (unsigned int)strlen(EPUB_MIME_DATA)) != Z_OK) {
        zipCloseFileInZip(zfile);
        zipClose(zfile, NULL);
        QFile::remove(tempFile);
        boost_throw(CannotStoreFile() << errinfo_file_fullpath("mimetype"));
    }

    zipCloseFileInZip(zfile);
    // Write all the files in our directory path to the archive.
    QDirIterator it(fullfolderpath, QDir::Files | QDir::NoDotAndDotDot | QDir::Readable | QDir::Hidden, QDirIterator::Subdirectories);

    while (it.hasNext()) {
        it.next();
        QString relpath = it.filePath().remove(fullfolderpath);

        while (relpath.startsWith("/")) {
            relpath = relpath.remove(0, 1);
        }

        // Add the file entry to the archive.
        // We should check the uncompressed file size. If it's over >= 0xffffffff the last parameter (zip64) should be 1.
        if (zipOpenNewFileInZip4_64(zfile, relpath.toUtf8().constData(), &fileInfo, NULL, 0, NULL, 0, NULL, Z_DEFLATED, 8, 0, 15, 8, Z_DEFAULT_STRATEGY, NULL, 0, 0x0b00, 1<<11, 0) != Z_OK) {
            zipClose(zfile, NULL);
            QFile::remove(tempFile);
            boost_throw(CannotStoreFile() << errinfo_file_fullpath(relpath.toStdString()));
        }

        // Open the file on disk. We will read this and write what we read into
        // the archive.
        QFile dfile(it.filePath());

        if (!dfile.open(QIODevice::ReadOnly)) {
            zipCloseFileInZip(zfile);
            zipClose(zfile, NULL);
            QFile::remove(tempFile);
            boost_throw(CannotOpenFile() << errinfo_file_fullpath(it.fileName().toStdString()));
        }

        // Write the data from the file on disk into the archive.
        char buff[BUFF_SIZE] = {0};
        qint64 read = 0;

        while ((read = dfile.read(buff, BUFF_SIZE)) > 0) {
            if (zipWriteInFileInZip(zfile, buff, read) != Z_OK) {
                dfile.close();
                zipCloseFileInZip(zfile);
                zipClose(zfile, NULL);
                QFile::remove(tempFile);
                boost_throw(CannotStoreFile() << errinfo_file_fullpath(relpath.toStdString()));
            }
        }

        dfile.close();

        // There was an error reading the file on disk.
        if (read < 0) {
            zipCloseFileInZip(zfile);
            zipClose(zfile, NULL);
            QFile::remove(tempFile);
            boost_throw(CannotStoreFile() << errinfo_file_fullpath(relpath.toStdString()));
        }

        if (zipCloseFileInZip(zfile) != Z_OK) {
            zipClose(zfile, NULL);
            QFile::remove(tempFile);
            boost_throw(CannotStoreFile() << errinfo_file_fullpath(relpath.toStdString()));
        }
    }

    zipClose(zfile, NULL);
    // Overwrite the contents of the real file with the contents from the temp
    // file we saved the data do. We do this instead of simply copying the file
    // because a file copy will lose extended attributes such as labels on OS X.
    QFile temp_epub(tempFile);

    if (!temp_epub.open(QFile::ReadOnly)) {
        boost_throw(CannotOpenFile() << errinfo_file_fullpath(tempFile.toStdString()));
    }

    QFile real_epub(fullfilepath);

    if (!real_epub.open(QFile::WriteOnly | QFile::Truncate)) {
        temp_epub.close();
        boost_throw(CannotWriteFile() << errinfo_file_fullpath(fullfilepath.toStdString()));
    }

    // Copy the contents from the temp file to the real file.
    char buff[BUFF_SIZE] = {0};
    qint64 read = 0;
    qint64 written = 0;

    while ((read = temp_epub.read(buff, BUFF_SIZE)) > 0) {
        written = real_epub.write(buff, read);

        if (written != read) {
            temp_epub.close();
            real_epub.close();
            QFile::remove(tempFile);
            boost_throw(CannotCopyFile() << errinfo_file_fullpath(fullfilepath.toStdString()));
        }
    }

    if (read == -1) {
        temp_epub.close();
        real_epub.close();
        QFile::remove(tempFile);
        boost_throw(CannotCopyFile() << errinfo_file_fullpath(fullfilepath.toStdString()));
    }

    temp_epub.close();
    real_epub.close();
    QFile::remove(tempFile);
}


void ExportEPUB::CreateEncryptionXML(const QString &fullfolderpath)
{
    QTemporaryFile file;

    if (!file.open()) {
        boost_throw(CannotOpenFile()
                    << errinfo_file_fullpath(file.fileName().toStdString())
                    << errinfo_file_errorstring(file.errorString().toStdString())
                   );
    }

    EncryptionXmlWriter enc(*m_Book, file);
    enc.WriteXML();
    // Write to disk immediately
    file.flush();
    QFile::copy(file.fileName(), fullfolderpath + "/" + ENCRYPTION_XML_FILE_NAME);
}


void ExportEPUB::ObfuscateFonts(const QString &fullfolderpath)
{
    QString uuid_id = m_Book->GetOPF().GetUUIDIdentifierValue();
    QString main_id = m_Book->GetPublicationIdentifier();
    QList<FontResource *> font_resources = m_Book->GetFolderKeeper().GetResourceTypeList<FontResource>();
    foreach(FontResource * font_resource, font_resources) {
        QString algorithm = font_resource->GetObfuscationAlgorithm();

        if (algorithm.isEmpty()) {
            continue;
        }

        QString font_path = fullfolderpath + "/" + font_resource->GetRelativePathToRoot();

        if (algorithm == ADOBE_FONT_ALGO_ID) {
            FontObfuscation::ObfuscateFile(font_path, algorithm, uuid_id);
        } else {
            FontObfuscation::ObfuscateFile(font_path, algorithm, main_id);
        }
    }
}






