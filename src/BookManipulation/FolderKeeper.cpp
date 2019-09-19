/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford, Ontario, Canada
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

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QTime>
#include <QtWidgets/QApplication>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "BookManipulation/FolderKeeper.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "ResourceObjects/AudioResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/Resource.h"
#include "ResourceObjects/VideoResource.h"
#include "Misc/Utility.h"
#include "Misc/OpenExternally.h"
#include "Misc/SettingsStore.h"

const QStringList IMAGE_EXTENSIONS = QStringList() << "jpg"   << "jpeg"  << "png"
                                     << "gif"   << "tif"   << "tiff"
                                     << "bm"    << "bmp";
const QStringList SVG_EXTENSIONS = QStringList() << "svg";
const QStringList SMIL_EXTENSIONS = QStringList() << "smil";
const QStringList JPG_EXTENSIONS = QStringList()   << "jpg"   << "jpeg";
const QStringList TIFF_EXTENSIONS = QStringList()  << "tif"  << "tiff";

// Exception for non-standard Apple files in META-INF.
// container.xml and encryption.xml will be rewritten
// on export. Other files in this directory are passed
// through untouched.
const QRegularExpression FILE_EXCEPTIONS("META-INF");

const QStringList MISC_TEXT_EXTENSIONS = QStringList()  << "txt"  << "js";
const QStringList MISC_XML_EXTENSIONS  = QStringList() << "smil" << "xpgt" << "pls";
const QStringList FONT_EXTENSIONS      = QStringList() << "ttf"   << "ttc"   << "otf" << "woff" << "woff2";
const QStringList TEXT_EXTENSIONS      = QStringList() << "xhtml" << "html"  << "htm";
const QStringList STYLE_EXTENSIONS     = QStringList() << "css";
const QStringList AUDIO_EXTENSIONS     = QStringList() << "aac" << "m4a" << "mp3" << "mpeg" << "mpg" << "oga" << "ogg";
const QStringList VIDEO_EXTENSIONS     = QStringList() << "m4v" << "mp4" << "mov" << "ogv" << "webm" << "vtt" << "ttml";

const QString IMAGE_FOLDER_NAME = "Images";
const QString FONT_FOLDER_NAME  = "Fonts";
const QString TEXT_FOLDER_NAME  = "Text";
const QString STYLE_FOLDER_NAME = "Styles";
const QString AUDIO_FOLDER_NAME = "Audio";
const QString VIDEO_FOLDER_NAME = "Video";
const QString MISC_FOLDER_NAME  = "Misc";

const QStringList IMAGE_MIMEYPES     = QStringList() << "image/gif" << "image/jpeg" << "image/png";
const QStringList SVG_MIMETYPES      = QStringList() << "image/svg+xml";
const QStringList TEXT_MIMETYPES     = QStringList() << "application/xhtml+xml" << "application/x-dtbook+xml";
const QStringList STYLE_MIMETYPES    = QStringList() << "text/css";
const QStringList FONT_MIMETYPES     = QStringList() << "application/x-font-ttf" << "application/x-font-opentype" << 
                                                        "application/vnd.ms-opentype" << "application/font-woff" << 
                                                        "application/font-sfnt" << "font/woff2";
const QStringList AUDIO_MIMETYPES    = QStringList() << "audio/mpeg" << "audio/mp3" << "audio/ogg" << "audio/mp4";
const QStringList VIDEO_MIMETYPES    = QStringList() << "video/mp4" << "video/ogg" << "video/webm" << 
                                                        "text/vtt" << "application/ttml+xml" ;
const QStringList MISC_XML_MIMETYPES = QStringList() << "application/oebps-page-map+xml" <<  "application/smil+xml" <<
                                                        "application/adobe-page-template+xml" <<
                                                        "application/vnd.adobe-page-template+xml" << "application/pls+xml";

static const QString CONTAINER_XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                                     "<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
                                     "    <rootfiles>\n"
                                     "        <rootfile full-path=\"OEBPS/content.opf\" media-type=\"application/oebps-package+xml\"/>\n"
                                     "   </rootfiles>\n"
                                     "</container>\n";


FolderKeeper::FolderKeeper(QObject *parent)
    :
    QObject(parent),
    m_OPF(NULL),
    m_NCX(NULL),
    m_FSWatcher(new QFileSystemWatcher()),
    m_FullPathToMainFolder(m_TempFolder.GetPath())
{
    CreateExtensionToMediaTypeMap();
    CreateKeyToLCPMap();
    CreateFolderStructure();
    CreateInfrastructureFiles();
}


FolderKeeper::~FolderKeeper()
{
    if (m_FullPathToMainFolder.isEmpty()) {
        return;
    }

    if (m_FSWatcher) {
        delete m_FSWatcher;
        m_FSWatcher = 0;
    }

    foreach(Resource *resource, m_Resources.values()) {
        // We disconnect the Deleted signal, since if we don't
        // the OPF will try to update itself on every resource
        // removal (and there's no point to that, FolderKeeper is dying).
        // Disconnecting this speeds up FolderKeeper destruction.
        disconnect(resource, SIGNAL(Deleted(const Resource *)), this, SLOT(RemoveResource(const Resource *)));
        resource->Delete();
    }
}

Resource *FolderKeeper::AddContentFileToFolder(const QString &fullfilepath, bool update_opf, const QString &mimetype)
{
    if (!QFileInfo(fullfilepath).exists()) {
        throw(FileDoesNotExist(fullfilepath.toStdString()));
    }

    QString new_file_path;
    QString normalised_file_path = fullfilepath;
    Resource *resource = NULL;
    // Rename files that start with a '.'
    // These merely introduce needless difficulties
    QFileInfo fileInformation(normalised_file_path);
    QString fileName = fileInformation.fileName();

    if (fileName.left(1) == ".") {
        normalised_file_path = fileInformation.absolutePath() % "/" % fileName.right(fileName.size() - 1);
    }

    // We need to lock here because otherwise
    // several threads can get the same "unique" name.
    // After we deal with the resource hash, other threads can continue.
    {
        QMutexLocker locker(&m_AccessMutex);
        QString filename  = GetUniqueFilenameVersion(QFileInfo(normalised_file_path).fileName());
        QString extension = QFileInfo(normalised_file_path).suffix().toLower();
	QString lcppath;

        if (fullfilepath.contains(FILE_EXCEPTIONS)) {
            // This is a big hack that assumes the new and old filepaths use root paths
            // of the same length. I can't see how to fix this without refactoring
            // a lot of the code to provide a more generalised interface.
            new_file_path = m_FullPathToMainFolder % fullfilepath.right(fullfilepath.size() - m_FullPathToMainFolder.size());
            resource = new Resource(m_FullPathToMainFolder, new_file_path);
	    lcppath = m_KeyToLCP.value("other", "");
        } else if (MISC_TEXT_EXTENSIONS.contains(extension)) {
            new_file_path = m_FullPathToMiscFolder + "/" + filename;
            resource = new MiscTextResource(m_FullPathToMainFolder, new_file_path);
	    lcppath = m_KeyToLCP.value("misc", "");
        } else if (AUDIO_EXTENSIONS.contains(extension) || AUDIO_MIMETYPES.contains(mimetype)) {
            new_file_path = m_FullPathToAudioFolder + "/" + filename;
            resource = new AudioResource(m_FullPathToMainFolder, new_file_path);
	    lcppath = m_KeyToLCP.value("audio","");
        } else if (VIDEO_EXTENSIONS.contains(extension) || VIDEO_MIMETYPES.contains(mimetype)) {
            new_file_path = m_FullPathToVideoFolder + "/" + filename;
            resource = new VideoResource(m_FullPathToMainFolder, new_file_path);
	    lcppath = m_KeyToLCP.value("video","");
        } else if (IMAGE_EXTENSIONS.contains(extension) || IMAGE_MIMEYPES.contains(mimetype)) {
            new_file_path = m_FullPathToImagesFolder + "/" + filename;
            resource = new ImageResource(m_FullPathToMainFolder, new_file_path);
	    lcppath = m_KeyToLCP.value("images","");
        } else if (SVG_EXTENSIONS.contains(extension) || SVG_MIMETYPES.contains(mimetype)) {
            new_file_path = m_FullPathToImagesFolder + "/" + filename;
            resource = new SVGResource(m_FullPathToMainFolder, new_file_path);
	    lcppath = m_KeyToLCP.value("images","");
        } else if (FONT_EXTENSIONS.contains(extension) || FONT_MIMETYPES.contains(mimetype)) {
            new_file_path = m_FullPathToFontsFolder + "/" + filename;
            resource = new FontResource(m_FullPathToMainFolder, new_file_path);
	    lcppath = m_KeyToLCP.value("fonts","");
        } else if (TEXT_EXTENSIONS.contains(extension) || TEXT_MIMETYPES.contains(mimetype)) {
            new_file_path = m_FullPathToTextFolder + "/" + filename;
            resource = new HTMLResource(m_FullPathToMainFolder, new_file_path, m_Resources);
	    lcppath = m_KeyToLCP.value("text","");
        } else if (STYLE_EXTENSIONS.contains(extension) || STYLE_MIMETYPES.contains(mimetype)) {
            new_file_path = m_FullPathToStylesFolder + "/" + filename;
            resource = new CSSResource(m_FullPathToMainFolder, new_file_path);
	    lcppath = m_KeyToLCP.value("styles","");
        } else if (MISC_XML_EXTENSIONS.contains(extension) || MISC_XML_MIMETYPES.contains(mimetype)) {
            new_file_path = m_FullPathToMiscFolder + "/" + filename;
            resource = new XMLResource(m_FullPathToMainFolder, new_file_path);
	    lcppath = m_KeyToLCP.value("misc","");
        } else {
            // Fallback mechanism
            new_file_path = m_FullPathToMiscFolder + "/" + filename;
            resource = new Resource(m_FullPathToMainFolder, new_file_path);
	    lcppath = m_KeyToLCP.value("misc","");
        }

        m_Resources[ resource->GetIdentifier() ] = resource;

	// Note:  m_FullPathToMainFolder **never** ends with a "/"
	QString book_path = new_file_path.right(new_file_path.length() - m_FullPathToMainFolder.length() - 1);
	m_Path2Resource[ book_path ] = resource;

        resource->SetEpubVersion(m_OPF->GetEpubVersion());
        if (!mimetype.isEmpty()) {
            resource->SetMediaType(mimetype);
        } else {
           resource->SetMediaType(m_ExtToMType.value(extension));
        }
        resource->SetLCP(lcppath);
    }
    QFile::copy(fullfilepath, new_file_path);

    if (QThread::currentThread() != QApplication::instance()->thread()) {
        resource->moveToThread(QApplication::instance()->thread());
    }

    connect(resource, SIGNAL(Deleted(const Resource *)),
            this,     SLOT(RemoveResource(const Resource *)), Qt::DirectConnection);
    connect(resource, SIGNAL(Renamed(const Resource *, QString)),
            this,     SLOT(ResourceRenamed(const Resource *, QString)), Qt::DirectConnection);

    if (update_opf) {
        emit ResourceAdded(resource);
    }

    return resource;
}


int FolderKeeper::GetHighestReadingOrder() const
{
    int count_of_html_resources = 0;
    foreach(Resource * resource, m_Resources.values()) {
        if (resource->Type() == Resource::HTMLResourceType) {
            ++count_of_html_resources;
        }
    }
    return count_of_html_resources - 1;
}


QString FolderKeeper::GetUniqueFilenameVersion(const QString &filename) const
{
    const QStringList &filenames = GetAllFilenames();

    if (!filenames.contains(filename, Qt::CaseInsensitive)) {
        return filename;
    }

    // name_prefix is part of the name without the number suffix.
    // So for "Section0001.xhtml", it is "Section"
    QString name_prefix = QFileInfo(filename).baseName().remove(QRegularExpression("\\d+$"));
    QString extension   = QFileInfo(filename).completeSuffix();
    // Used to search for the filename number suffixes.
    QString search_string = QRegularExpression::escape(name_prefix).prepend("^") +
                            "(\\d*)" +
                            (!extension.isEmpty() ? ("\\." + QRegularExpression::escape(extension)) : QString()) +
                            "$";

    QRegularExpression filename_search(search_string);
    filename_search.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
    int max_num_length = -1;
    int max_num = -1;
    foreach(QString existing_file, filenames) {
        QRegularExpressionMatch match = filename_search.match(existing_file);
        if (!match.hasMatch()) {
            continue;
        }

        bool conversion_successful = false;
        int number_suffix = match.captured(1).toInt(&conversion_successful);
        
        if (conversion_successful && number_suffix > max_num) {
            max_num = number_suffix;
            max_num_length = match.capturedLength(1);
        }
    }

    if (max_num == -1) {
        max_num = 0;
        max_num_length = 4;
    }

    const int conversion_base = 10;
    QString new_name = name_prefix + QString("%1").arg(max_num + 1,
                       max_num_length,
                       conversion_base,
                       QChar('0'));
    return new_name + (!extension.isEmpty() ? ("." + extension) : QString());
}

QList<Resource *> FolderKeeper::GetResourceList() const
{
    return m_Resources.values();
}

QList<Resource *> FolderKeeper::GetResourceListByType(Resource::ResourceType type) const
{
    QList <Resource *> resources;
    foreach (Resource *resource, m_Resources.values()) {
        if (resource->Type() == type) {
            resources.append(resource);
        }
    }
    return resources;
}

Resource *FolderKeeper::GetResourceByIdentifier(const QString &identifier) const
{
    return m_Resources[ identifier ];
}


Resource *FolderKeeper::GetResourceByFilename(const QString &filename) const
{
    foreach(Resource *resource, m_Resources.values()) {
        if (resource->Filename() == filename) {
            return resource;
        }
    }
    throw(ResourceDoesNotExist(filename.toStdString()));
}


Resource *FolderKeeper::GetResourceByShortPathName(const QString &shortpathname) const
{
    foreach(Resource *resource, m_Resources.values()) {
        if (resource->ShortPathName() == shortpathname) {
            return resource;
        }
    }
    throw(ResourceDoesNotExist(shortpathname.toStdString()));
}

// Not guaranteed to be unique or to be found
// if not found returns an empty string
QString FolderKeeper::GetBookPathByPathEnd(const QString& path_end) const
{
    foreach(Resource *resource, m_Resources.values()) {
        QString bookpath = resource->GetRelativePath();
        if (bookpath.endsWith(path_end)) {
            return bookpath ;
        }
    }
    return "";
}


// a Book path is the path from the m_MainFolder to that file
Resource *FolderKeeper::GetResourceByBookPath(const QString &bookpath) const
{
    Resource * resource = m_Path2Resource.value(bookpath, NULL);
    if (resource) {
        return resource;
    }
    throw(ResourceDoesNotExist(bookpath.toStdString()));
}


OPFResource *FolderKeeper::GetOPF() const
{
    return m_OPF;
}


// Note this routine can now return nullptr on epub3
NCXResource *FolderKeeper::GetNCX() const
{
    return m_NCX;
}


NCXResource*FolderKeeper::AddNCXToFolder(const QString & version)
{
    m_NCX = new NCXResource(m_FullPathToMainFolder, m_FullPathToOEBPSFolder + "/" + NCX_FILE_NAME, this);
    m_NCX->SetMainID(m_OPF->GetMainIdentifierValue());
    m_NCX->SetEpubVersion(version);
    m_NCX->SetLCP(m_KeyToLCP[ "ncx"]);
    m_Resources[ m_NCX->GetIdentifier() ] = m_NCX;
    m_Path2Resource[ m_NCX->GetRelativePath() ] = m_NCX;

    // TODO: change from Resource* to const Resource&
    connect(m_NCX, SIGNAL(Deleted(const Resource *)), this, SLOT(RemoveResource(const Resource *)));
    return m_NCX;
}


void FolderKeeper::RemoveNCXFromFolder()
{
    if (!m_NCX) {
        return;
    }
    disconnect(m_NCX, SIGNAL(Deleted(const Resource *)), this, SLOT(RemoveResource(const Resource *)));
    RemoveResource(m_NCX);    
    m_NCX = NULL;
    return;
}


QString FolderKeeper::GetFullPathToMainFolder() const
{
    return m_FullPathToMainFolder;
}


QString FolderKeeper::GetFullPathToOEBPSFolder() const
{
    return m_FullPathToOEBPSFolder;
}


QString FolderKeeper::GetFullPathToTextFolder() const
{
    return m_FullPathToTextFolder;
}


QString FolderKeeper::GetFullPathToImageFolder() const
{
    return m_FullPathToImagesFolder;
}

QString FolderKeeper::GetFullPathToAudioFolder() const
{
    return m_FullPathToAudioFolder;
}

QString FolderKeeper::GetFullPathToVideoFolder() const
{
    return m_FullPathToVideoFolder;
}


QStringList FolderKeeper::GetAllFilenames() const
{
    QStringList filelist;
    foreach(Resource *resource, m_Resources.values()) {
        filelist.append(resource->Filename());
    }
    return filelist;
}


QStringList FolderKeeper::GetAllBookPaths() const
{
    QStringList bookpaths;
    foreach(Resource *resource, m_Resources.values()) {
        bookpaths.append(resource->GetRelativePath());
    }
    return bookpaths;
}


void FolderKeeper::RemoveResource(const Resource *resource)
{
    m_Resources.remove(resource->GetIdentifier());
    m_Path2Resource.remove(resource->GetRelativePath());

    if (m_FSWatcher->files().contains(resource->GetFullPath())) {
        m_FSWatcher->removePath(resource->GetFullPath());
    }

    m_SuspendedWatchedFiles.removeAll(resource->GetFullPath());
    emit ResourceRemoved(resource);
}

void FolderKeeper::ResourceRenamed(const Resource *resource, const QString &old_full_path)
{
    // Renaming means the resource book path has changed and so we need to update it
    // Note:  m_FullPathToMainFolder **never** ends with a "/"                                                        
    QString book_path = old_full_path.right(old_full_path.length() - m_FullPathToMainFolder.length() - 1);
    Resource * res = m_Path2Resource[book_path];
    m_Path2Resource.remove(book_path);
    m_Path2Resource[resource->GetRelativePath()] = res;
    m_OPF->ResourceRenamed(resource, old_full_path);
}

void FolderKeeper::ResourceFileChanged(const QString &path) const
{
    // The file may have been deleted prior to writing a new version - give it a chance to write.
    QTime wake_time = QTime::currentTime().addMSecs(1000);

    while (!QFile::exists(path) && QTime::currentTime() < wake_time) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    // The signal is also received after resource files are removed / renamed,
    // but it can be safely ignored because QFileSystemWatcher automatically stops watching them.
    if (QFile::exists(path)) {
        // Some editors write the updated contents to a temporary file
        // and then atomically move it over the watched file.
        // In this case QFileSystemWatcher loses track of the file, so we have to add it again.
        if (!m_FSWatcher->files().contains(path)) {
            m_FSWatcher->addPath(path);
        }

        foreach(Resource *resource, m_Resources.values()) {
            if (resource->GetFullPath() == path) {
                resource->FileChangedOnDisk();
                return;
            }
        }
    }
}

void FolderKeeper::WatchResourceFile(const Resource *resource)
{
    if (OpenExternally::mayOpen(resource->Type())) {
        if (!m_FSWatcher->files().contains(resource->GetFullPath())) {
            m_FSWatcher->addPath(resource->GetFullPath());
        }

        // when the file is changed externally, mark the owning Book as modified
        // parent() is the Book object
        connect(resource,  SIGNAL(ResourceUpdatedFromDisk(Resource *)),
                parent(),   SLOT(ResourceUpdatedFromDisk(Resource *)), Qt::UniqueConnection);
    }
}

void FolderKeeper::SuspendWatchingResources()
{
    if (m_SuspendedWatchedFiles.isEmpty() && !m_FSWatcher->files().isEmpty()) {
        m_SuspendedWatchedFiles.append(m_FSWatcher->files());
        m_FSWatcher->removePaths(m_SuspendedWatchedFiles);
    }
}

void FolderKeeper::ResumeWatchingResources()
{
    if (!m_SuspendedWatchedFiles.isEmpty()) {
        foreach(QString path, m_SuspendedWatchedFiles) {
            if (QFile::exists(path)) {
                m_FSWatcher->addPath(path);
            }
        }
        m_SuspendedWatchedFiles.clear();
    }
}

// The required folder structure is this:
//	 META-INF
//	 OEBPS
//	    Images
//	    Fonts
//	    Text
//          Styles
//          Misc
void FolderKeeper::CreateFolderStructure()
{
    QDir folder(m_FullPathToMainFolder);
    folder.mkdir("META-INF");
    folder.mkdir("OEBPS");
    folder.mkpath("OEBPS/" + AUDIO_FOLDER_NAME);
    folder.mkpath("OEBPS/" + VIDEO_FOLDER_NAME);
    folder.mkpath("OEBPS/" + IMAGE_FOLDER_NAME);
    folder.mkpath("OEBPS/" + FONT_FOLDER_NAME);
    folder.mkpath("OEBPS/" + TEXT_FOLDER_NAME);
    folder.mkpath("OEBPS/" + STYLE_FOLDER_NAME);
    folder.mkpath("OEBPS/" + MISC_FOLDER_NAME);
    m_FullPathToMetaInfFolder = m_FullPathToMainFolder + "/META-INF";
    m_FullPathToOEBPSFolder   = m_FullPathToMainFolder + "/OEBPS";
    m_FullPathToAudioFolder   = m_FullPathToOEBPSFolder + "/" + AUDIO_FOLDER_NAME;
    m_FullPathToVideoFolder   = m_FullPathToOEBPSFolder + "/" + VIDEO_FOLDER_NAME;
    m_FullPathToImagesFolder  = m_FullPathToOEBPSFolder + "/" + IMAGE_FOLDER_NAME;
    m_FullPathToFontsFolder   = m_FullPathToOEBPSFolder + "/" + FONT_FOLDER_NAME;
    m_FullPathToTextFolder    = m_FullPathToOEBPSFolder + "/" + TEXT_FOLDER_NAME;
    m_FullPathToStylesFolder  = m_FullPathToOEBPSFolder + "/" + STYLE_FOLDER_NAME;
    m_FullPathToMiscFolder    = m_FullPathToOEBPSFolder + "/" + MISC_FOLDER_NAME;
}


void FolderKeeper::CreateInfrastructureFiles()
{
    SettingsStore ss;
    QString version = ss.defaultVersion();
    m_OPF = new OPFResource(m_FullPathToMainFolder, m_FullPathToOEBPSFolder + "/" + OPF_FILE_NAME, this);
    m_OPF->SetEpubVersion(version);
    m_OPF->SetLCP(m_KeyToLCP[ "opf" ]);
    m_Resources[ m_OPF->GetIdentifier() ] = m_OPF;
    m_Path2Resource[ m_OPF->GetRelativePath() ] = m_OPF;
    // note - ncx is optional on epub3 so do not create an ncx here
    // instead add it ony when needed

    // TODO: change from Resource* to const Resource&
    connect(m_OPF, SIGNAL(Deleted(const Resource *)), this, SLOT(RemoveResource(const Resource *)));
    // For ResourceAdded, the connection has to be DirectConnection,
    // otherwise the default of AutoConnection screws us when
    // AddContentFileToFolder is called from multiple threads.
    connect(this,  SIGNAL(ResourceAdded(const Resource *)),
            m_OPF, SLOT(AddResource(const Resource *)), Qt::DirectConnection);
    connect(this,  SIGNAL(ResourceRemoved(const Resource *)),
            m_OPF, SLOT(RemoveResource(const Resource *)));
    connect(m_FSWatcher, SIGNAL(fileChanged(const QString &)),
            this,        SLOT(ResourceFileChanged(const QString &)), Qt::DirectConnection);
    Utility::WriteUnicodeTextFile(CONTAINER_XML, m_FullPathToMetaInfFolder + "/container.xml");
}



// Initializes m_Mimetypes
void FolderKeeper::CreateExtensionToMediaTypeMap()
{
  m_ExtToMType[ "bm"    ] = "image/bmp";
  m_ExtToMType[ "bmp"   ] = "image/bmp";
  m_ExtToMType[ "css"   ] = "text/css";
  m_ExtToMType[ "gif"   ] = "image/gif";
  m_ExtToMType[ "htm"   ] = "application/xhtml+xml";
  m_ExtToMType[ "html"  ] = "application/xhtml+xml";
  m_ExtToMType[ "jpeg"  ] = "image/jpeg";
  m_ExtToMType[ "jpg"   ] = "image/jpeg";
  m_ExtToMType[ "js"    ] = "application/javascript";
  // m_ExtToMType[ "js"    ] = "text/javascript";
  m_ExtToMType[ "mp3"   ] = "audio/mpeg";
  m_ExtToMType[ "m4a"   ] = "audio/mp4";
  m_ExtToMType[ "mp4"   ] = "video/mp4";
  m_ExtToMType[ "m4v"   ] = "video/mp4";
  m_ExtToMType[ "ncx"   ] = "application/x-dtbncx+xml";
  m_ExtToMType[ "oga"   ] = "audio/ogg";
  m_ExtToMType[ "ogg"   ] = "audio/ogg";
  m_ExtToMType[ "ogv"   ] = "video/ogg";
  m_ExtToMType[ "opf"   ] = "application/oebps-package+xml";
  m_ExtToMType[ "otf"   ] = "application/vnd.ms-opentype";
  // m_ExtToMType[ "otf"   ] = "application/font-sfnt";
  m_ExtToMType[ "pls"   ] = "application/pls+xml";
  m_ExtToMType[ "png"   ] = "image/png";
  m_ExtToMType[ "smil"  ] = "application/smil+xml";
  m_ExtToMType[ "svg"   ] = "image/svg+xml";
  m_ExtToMType[ "tif"   ] = "image/tiff";
  m_ExtToMType[ "tiff"  ] = "image/tiff";
  m_ExtToMType[ "ttf"   ] = "application/x-font-ttf";
  m_ExtToMType[ "ttc"   ] = "application/x-font-truetype-collection";
  m_ExtToMType[ "ttml"  ] = "application/ttml+xml";
  m_ExtToMType[ "vtt"   ] = "text/vtt";
  m_ExtToMType[ "webm"  ] = "video/webm";
  m_ExtToMType[ "woff"  ] = "application/font-woff";
  m_ExtToMType[ "woff2"  ] = "font/woff2";
  m_ExtToMType[ "xpgt"  ] = "application/adobe-page-template+xml";
  m_ExtToMType[ "xhtml" ] = "application/xhtml+xml";
}

// Hard codes Longest Common Path for the time being
// Note all LCP paths **must** end with "/"
void FolderKeeper::CreateKeyToLCPMap()
{
    // Note: m_FullPathToMainFolder **never** ends with a "/" see Misc/TempFolder.cpp
    // Note all LCP paths **must** end with "/"
    m_KeyToLCP[ "text"   ] = m_FullPathToMainFolder + "/OEBPS/Text/";
    m_KeyToLCP[ "styles" ] = m_FullPathToMainFolder + "/OEBPS/Styles/";
    m_KeyToLCP[ "images" ] = m_FullPathToMainFolder + "/OEBPS/Images/";
    m_KeyToLCP[ "fonts"  ] = m_FullPathToMainFolder + "/OEBPS/Fonts/";
    m_KeyToLCP[ "audio"  ] = m_FullPathToMainFolder + "/OEBPS/Audio/";
    m_KeyToLCP[ "video"  ] = m_FullPathToMainFolder + "/OEBPS/Video/";
    m_KeyToLCP[ "misc"   ] = m_FullPathToMainFolder + "/OEBPS/Misc/";
    m_KeyToLCP[ "other"  ] = m_FullPathToMainFolder + "/";
    m_KeyToLCP[ "opf"    ] = m_FullPathToMainFolder + "/OEBPS/";
    m_KeyToLCP[ "ncx"    ] = m_FullPathToMainFolder + "/OEBPS/";
}

QString FolderKeeper::GetLongestCommonPathForKey(const QString &key)
{
    return m_KeyToLCP.value(key, "");
}
