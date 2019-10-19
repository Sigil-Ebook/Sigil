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
#include "Misc/MediaTypes.h"


const QStringList groupA = QStringList() << "Text"<<"Styles"<<"Images"<<"Audio"<<"Fonts"<<"Video"<<"Misc";


// Exception for non-standard Apple files in META-INF.
// container.xml and encryption.xml will be rewritten
// on export. Other files in this directory are passed
// through untouched.
const QRegularExpression FILE_EXCEPTIONS("META-INF");


static const QString CONTAINER_XML       = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<container version=\"1.0\" xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
        "    <rootfiles>\n"
        "        <rootfile full-path=\"%1\" media-type=\"application/oebps-package+xml\"/>\n"
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
    CreateGroupToFoldersMap();
    connect(m_FSWatcher, SIGNAL(fileChanged(const QString &)),
            this,        SLOT(ResourceFileChanged(const QString &)), Qt::DirectConnection);
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

QString FolderKeeper::DetermineFileGroup(const QString &filepath, const QString &mimetype)
{
    QFileInfo fi(filepath);
    QString fileName = fi.fileName();
    QString extension = fi.suffix().toLower();
    QString mt = mimetype;

    if (filepath.contains(FILE_EXCEPTIONS)) return "other";
    if (mt.isEmpty()) {
        mt = MediaTypes::instance()->GetMediaTypeFromExtension(extension, "");
        if (mt.isEmpty()) return "other";
    }
    QString group = MediaTypes::instance()->GetGroupFromMediaType(mt, "");
    if (group.isEmpty()) {
        // try again just in case provided mediatype is wrong and use the one based
        // on the file extension only this time
        mt = MediaTypes::instance()->GetMediaTypeFromExtension(extension, "");
	if (!mt.isEmpty()) {
            group = MediaTypes::instance()->GetGroupFromMediaType(mt, "");
	}
    }
    if (group.isEmpty()) group = "other";
    return group;
}

// This routine should never process the opf or the ncx as they 
// are special cased elsewhere in FolderKeeper
Resource *FolderKeeper::AddContentFileToFolder(const QString &fullfilepath, 
					       bool update_opf, 
					       const QString &mimetype, 
					       const QString &bookpath,
					       const QString &folderpath)
{
    if (!QFileInfo(fullfilepath).exists()) {
        throw(FileDoesNotExist(fullfilepath.toStdString()));
    }

    // initialize base file information
    QString norm_file_path = fullfilepath;
    QFileInfo fi(norm_file_path);
    QString filename = fi.fileName();

    // make sure mediatype is properly assigned
    QString mt = mimetype;
    if (mt.isEmpty()) {
        QString extension = fi.suffix().toLower();
	mt = MediaTypes::instance()->GetMediaTypeFromExtension(extension, "");
    }

    QString group = DetermineFileGroup(norm_file_path, mt);
    QString resdesc = MediaTypes::instance()->GetResourceDescFromMediaType(mt, "Resource");
    QDir folder(m_FullPathToMainFolder);

    Resource *resource = NULL;
    QString new_file_path;


    // lock for GetUniqueFilenameVersion() until the 
    // resource with that file name has been created
    // and added to the list of all resources so it
    // will return that this filename is now taken
    {

        QMutexLocker locker(&m_AccessMutex);

        if (!bookpath.isEmpty()) {
            // use the specified bookpath to determine both file name and location
	    if (!Utility::startingDir(bookpath).isEmpty()) {
	        folder.mkpath(Utility::startingDir(bookpath));
	    }
	    new_file_path = m_FullPathToMainFolder + "/" + bookpath;
        } else {
	    // Use either the provided folder path or the default folder to store the file
 
            // Rename files that start with a '.'
            // These merely introduce needless difficulties
            if (filename.left(1) == ".") {
                norm_file_path = fi.absolutePath() % "/" % filename.right(filename.size() - 1);
            }
            filename  = GetUniqueFilenameVersion(QFileInfo(norm_file_path).fileName());
	    QString folder_to_use = folderpath;
	    if (folder_to_use == "\\") folder_to_use = GetDefaultFolderForGroup(group);
	    if (!folder_to_use.isEmpty()) {
	        folder.mkpath(folder_to_use);
	        new_file_path = m_FullPathToMainFolder + "/" + folder_to_use + "/" + filename;
	    } else {
	        new_file_path = m_FullPathToMainFolder + "/" + filename;
	    }
        }

        if (fullfilepath.contains(FILE_EXCEPTIONS)) {
	    // This is used for all files inside the META-INF directory
            // This is a big hack that assumes the new and old filepaths use root paths
            // of the same length. I can't see how to fix this without refactoring
            // a lot of the code to provide a more generalised interface.
            new_file_path = m_FullPathToMainFolder % fullfilepath.right(fullfilepath.size() - m_FullPathToMainFolder.size());
            resource = new Resource(m_FullPathToMainFolder, new_file_path);
        } else if (resdesc == "MiscTextResource") {
            resource = new MiscTextResource(m_FullPathToMainFolder, new_file_path);
        } else if (resdesc == "AudioResource") {
            resource = new AudioResource(m_FullPathToMainFolder, new_file_path);
        } else if (resdesc == "VideoResource") {
            resource = new VideoResource(m_FullPathToMainFolder, new_file_path);
        } else if (resdesc == "ImageResource") {
            resource = new ImageResource(m_FullPathToMainFolder, new_file_path);
        } else if (resdesc == "SVGResource") {
            resource = new SVGResource(m_FullPathToMainFolder, new_file_path);
        } else if (resdesc == "FontResource") {
            resource = new FontResource(m_FullPathToMainFolder, new_file_path);
        } else if (resdesc == "HTMLResource") {
            resource = new HTMLResource(m_FullPathToMainFolder, new_file_path, m_Resources);
        } else if (resdesc == "CSSResource") {
            resource = new CSSResource(m_FullPathToMainFolder, new_file_path);
        } else if (resdesc == "XMLResource") {
            resource = new XMLResource(m_FullPathToMainFolder, new_file_path);
        } else {
            // Fallback mechanism
	    new_file_path = m_FullPathToMainFolder + "/" + GetDefaultFolderForGroup("Misc") + "/" + filename;
            resource = new Resource(m_FullPathToMainFolder, new_file_path);
        }

        m_Resources[ resource->GetIdentifier() ] = resource;

        // Note:  m_FullPathToMainFolder **never** ends with a "/"
	QString book_path = bookpath;
	if (book_path.isEmpty()) {
            book_path = new_file_path.right(new_file_path.length() - m_FullPathToMainFolder.length() - 1);
	}
        m_Path2Resource[ book_path ] = resource;
        resource->SetEpubVersion(m_OPF->GetEpubVersion());
        resource->SetMediaType(mt);
        resource->SetShortPathName(filename);
    }

    QFile::copy(fullfilepath, new_file_path);

    if (QThread::currentThread() != QApplication::instance()->thread()) {
        resource->moveToThread(QApplication::instance()->thread());
    }

    connect(resource, SIGNAL(Deleted(const Resource *)),
            this,     SLOT(RemoveResource(const Resource *)), Qt::DirectConnection);
    connect(resource, SIGNAL(Renamed(const Resource *, QString)),
            this,     SLOT(ResourceRenamed(const Resource *, QString)), Qt::DirectConnection);
    connect(resource, SIGNAL(Moved(const Resource *, QString)),
            this,     SLOT(ResourceMoved(const Resource *, QString)), Qt::DirectConnection);

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

// Not guaranteed to be unique or to be found
// if not found returns an empty string
// uses a case insensitive match since can be used on case insensitive file systems
QString FolderKeeper::GetBookPathByPathEnd(const QString& path_end) const
{
    foreach(Resource *resource, m_Resources.values()) {
        QString bookpath = resource->GetRelativePath();
        if (bookpath.endsWith(path_end, Qt::CaseInsensitive)) {
            return bookpath ;
        }
    }
    return "";
}


// a Book path is the path from the m_MainFolder to that file O(1) as a hash
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

OPFResource*FolderKeeper::AddOPFToFolder(const QString &version, const QString &bookpath)
{
    QString OPFBookPath = "OEBPS/content.opf";
    if (!bookpath.isEmpty()) {
        OPFBookPath = bookpath;
    }
    QDir folder(m_FullPathToMainFolder);
    QString sdir = Utility::startingDir(OPFBookPath);
    if (!sdir.isEmpty()) folder.mkpath(sdir);
    m_OPF = new OPFResource(m_FullPathToMainFolder, m_FullPathToMainFolder + "/" + OPFBookPath, this);
    m_OPF->SetEpubVersion(version);
    m_OPF->SetMediaType("application/oebps-package+xml");
    m_OPF->SetShortPathName(OPFBookPath.split('/').last());
    m_Resources[ m_OPF->GetIdentifier() ] = m_OPF;
    m_Path2Resource[ m_OPF->GetRelativePath() ] = m_OPF;

    connect(m_OPF, SIGNAL(Deleted(const Resource *)), this, SLOT(RemoveResource(const Resource *)));
    // For ResourceAdded, the connection has to be DirectConnection,
    // otherwise the default of AutoConnection screws us when
    // AddContentFileToFolder is called from multiple threads.
    connect(this,  SIGNAL(ResourceAdded(const Resource *)),
	    m_OPF, SLOT(AddResource(const Resource *)), Qt::DirectConnection);
    connect(this,  SIGNAL(ResourceRemoved(const Resource *)),
	    m_OPF, SLOT(RemoveResource(const Resource *)));
    connect(m_OPF, SIGNAL(Renamed(const Resource *, QString)),
            this,     SLOT(ResourceRenamed(const Resource *, QString)), Qt::DirectConnection);
    connect(m_OPF, SIGNAL(Moved(const Resource *, QString)),
            this,     SLOT(ResourceMoved(const Resource *, QString)), Qt::DirectConnection);
    UpdateContainerXML(m_FullPathToMainFolder, OPFBookPath);
    return m_OPF;
}


void FolderKeeper::UpdateContainerXML(const QString& FullPathToMainFolder, const QString& opfbookpath)
{
    QDir folder(FullPathToMainFolder);
    folder.mkpath("META-INF");
    Utility::WriteUnicodeTextFile(CONTAINER_XML.arg(opfbookpath), FullPathToMainFolder + "/META-INF/container.xml");
}


NCXResource*FolderKeeper::AddNCXToFolder(const QString & version, const QString &bookpath)
{
    QString NCXBookPath = "OEBPS/toc.ncx";
    if (!bookpath.isEmpty()) {
        NCXBookPath = bookpath;
    }
    QDir folder(m_FullPathToMainFolder);
    QString sdir = Utility::startingDir(NCXBookPath);
    if (!sdir.isEmpty()) folder.mkpath(sdir);
    m_NCX = new NCXResource(m_FullPathToMainFolder, m_FullPathToMainFolder + "/" + NCXBookPath, this);
    m_NCX->SetMainID(m_OPF->GetMainIdentifierValue());
    m_NCX->SetEpubVersion(version);
    m_NCX->SetMediaType("application/x-dtbncx+xml");
    m_NCX->SetShortPathName(NCXBookPath.split('/').last());
    m_Resources[ m_NCX->GetIdentifier() ] = m_NCX;
    m_Path2Resource[ m_NCX->GetRelativePath() ] = m_NCX;
    connect(m_NCX, SIGNAL(Deleted(const Resource *)), this, SLOT(RemoveResource(const Resource *)));
    connect(m_NCX, SIGNAL(Renamed(const Resource *, QString)),
            this,     SLOT(ResourceRenamed(const Resource *, QString)), Qt::DirectConnection);
    connect(m_NCX, SIGNAL(Moved(const Resource *, QString)),
            this,     SLOT(ResourceMoved(const Resource *, QString)), Qt::DirectConnection);

    return m_NCX;
}


void FolderKeeper::RemoveNCXFromFolder()
{
    if (!m_NCX) {
        return;
    }
    disconnect(m_NCX, SIGNAL(Deleted(const Resource *)), this, SLOT(RemoveResource(const Resource *)));
    disconnect(m_NCX, SIGNAL(Renamed(const Resource *, QString)),
	       this,     SLOT(ResourceRenamed(const Resource *, QString)));
    disconnect(m_NCX, SIGNAL(Moved(const Resource *, QString)),
	       this,     SLOT(ResourceMoved(const Resource *, QString)));
    RemoveResource(m_NCX);    
    m_NCX = NULL;
    return;
}


QString FolderKeeper::GetFullPathToMainFolder() const
{
    return m_FullPathToMainFolder;
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
    if (resource != m_OPF) {
        m_OPF->ResourceRenamed(resource, old_full_path);
    }
    updateShortPathNames();
}

void FolderKeeper::ResourceMoved(const Resource *resource, const QString &old_full_path)
{
    // Renaming means the resource book path has changed and so we need to update it
    // Note:  m_FullPathToMainFolder **never** ends with a "/"                                                        
    QString book_path = old_full_path.right(old_full_path.length() - m_FullPathToMainFolder.length() - 1);
    Resource * res = m_Path2Resource[book_path];
    m_Path2Resource.remove(book_path);
    m_Path2Resource[resource->GetRelativePath()] = res;
    m_OPF->ResourceMoved(resource, old_full_path);
    updateShortPathNames();
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


// Note all paths do NOT end with "/"
void FolderKeeper::CreateStdGroupToFoldersMap()
{
    if (!m_StdGrpToFold.isEmpty()) return;
    m_StdGrpToFold[ "Text"   ] = QStringList() << "OEBPS/Text";
    m_StdGrpToFold[ "Styles" ] = QStringList() << "OEBPS/Styles";
    m_StdGrpToFold[ "Images" ] = QStringList() << "OEBPS/Images";
    m_StdGrpToFold[ "Fonts"  ] = QStringList() << "OEBPS/Fonts";
    m_StdGrpToFold[ "Audio"  ] = QStringList() << "OEBPS/Audio";
    m_StdGrpToFold[ "Video"  ] = QStringList() << "OEBPS/Video";
    m_StdGrpToFold[ "Misc"   ] = QStringList() << "OEBPS/Misc";
    m_StdGrpToFold[ "ncx"    ] = QStringList() << "OEBPS";
    m_StdGrpToFold[ "opf"    ] = QStringList() << "OEBPS";
    m_StdGrpToFold[ "other"  ] = QStringList() << "";
}

QString FolderKeeper::GetStdFolderForGroup(const QString &group)
{
  CreateStdGroupToFoldersMap();
  return m_StdGrpToFold.value(group, QStringList() << "").first();
}


// Note all paths do NOT end with "/"
void FolderKeeper::CreateGroupToFoldersMap()
{
    if (!m_GrpToFold.isEmpty()) return;
    m_GrpToFold[ "Text"   ] = QStringList() << "OEBPS/Text";
    m_GrpToFold[ "Styles" ] = QStringList() << "OEBPS/Styles";
    m_GrpToFold[ "Images" ] = QStringList() << "OEBPS/Images";
    m_GrpToFold[ "Fonts"  ] = QStringList() << "OEBPS/Fonts";
    m_GrpToFold[ "Audio"  ] = QStringList() << "OEBPS/Audio";
    m_GrpToFold[ "Video"  ] = QStringList() << "OEBPS/Video";
    m_GrpToFold[ "Misc"   ] = QStringList() << "OEBPS/Misc";
    m_GrpToFold[ "ncx"    ] = QStringList() << "OEBPS";
    m_GrpToFold[ "opf"    ] = QStringList() << "OEBPS";
    m_GrpToFold[ "other"  ] = QStringList() << "";
}


QStringList FolderKeeper::GetFoldersForGroup(const QString &group)
{
    CreateGroupToFoldersMap();
    return m_GrpToFold.value(group, QStringList() << "");
}


QString FolderKeeper::GetDefaultFolderForGroup(const QString &group)
{
    return GetFoldersForGroup(group).first();
}


#if 0
QString FolderKeeper::buildShortName(const QString &bookpath, int lvl)
{
    QStringList pieces = bookpath.split('/');
    if (lvl == 1) return pieces.last();
    int n =  pieces.length();
    if (lvl >= n) return "^" + bookpath;
    for (int i=lvl; i < n; i++) pieces.removeFirst();
    return pieces.join('/');
}


void FolderKeeper::updateShortPathNames()
{
    QStringList bookpaths = GetAllBookPaths();

    QHash<QString,QString>BookToSPN;
    QHash<QString, QStringList> NameToBooks;
    QSet<QString> DupSet;
    int lvl = 1;

    // assign filenames as initial short names and create set of duplicate
    // filenames to make unique
    foreach(QString bkpath, bookpaths) {
        QString aname = buildShortName(bkpath, lvl);
        BookToSPN[bkpath] = aname;
        if (NameToBooks.contains(aname)) {
            DupSet.insert(aname);
            NameToBooks[aname].append(bkpath);
        } else {
            NameToBooks[aname] = QStringList() << bkpath;
        }
    }

    // now work just through any to-do list of duplicates
    // until all duplicates are gone
    QStringList todolst = DupSet.toList();
    while(!todolst.isEmpty()) {
        DupSet.clear();
        lvl++;
        foreach(QString aname, todolst) {
            QStringList bklst = NameToBooks[aname];
            NameToBooks.remove(aname);
            foreach(QString bkpath, bklst) {
	        QString newname = buildShortName(bkpath, lvl);
	        BookToSPN[bkpath] = newname;
	        if (NameToBooks.contains(newname)) {
	            DupSet.insert(newname);
	            NameToBooks[newname].append(bkpath);
	        } else {
	            NameToBooks[newname] = QStringList() << bkpath;
	        }
            }
        }
        todolst = DupSet.toList();
    }
    // now set the short path name for each resource
    foreach(QString bookpath, bookpaths) {
        Resource * resource = GetResourceByBookPath(bookpath);
	QString shortname = BookToSPN[bookpath];
	if (resource->ShortPathName() != shortname) {
	    resource->SetShortPathName(shortname);
	}
    }
}
#else
void FolderKeeper::updateShortPathNames()
{
    QStringList bookpaths = GetAllBookPaths();
    QSet<QString> UsedSet;
    QSet<QString> DupSet;

    // assign filenames as initial short names and create set of duplicate
    // filenames to make unique
    foreach(QString bkpath, bookpaths) {
        QString aname = bkpath.split("/").last();
	if (UsedSet.contains(aname)) DupSet.insert(aname);
	UsedSet.insert(aname);
    }
    // if not a duplicate name, use the filename as the short path name
    // otherwise use the full bookpath
    foreach(QString bkpath, bookpaths) {
        QString shortname = bkpath.split("/").last();
	if (DupSet.contains(shortname)) {
	    shortname = bkpath;
	}
        Resource * resource = GetResourceByBookPath(bkpath);
	if (resource->ShortPathName() != shortname) {
	    resource->SetShortPathName(shortname);
	}
    }
}
#endif

// fyi - generates folder paths that do NOT end with a "/"
void FolderKeeper::SetGroupFolders(const QStringList &bookpaths, const QStringList &mtypes)
{
    QHash< QString, QStringList > group_folder;
    QHash< QString, QList<int> > group_count;
    QStringList mediatypes = mtypes;

    // walk bookpaths and mtypes to determine folders
    // actually being used according to the opf
    int i = 0;
    foreach(QString bookpath, bookpaths) {
        QString mtype = mtypes.at(i);
        QString group = MediaTypes::instance()->GetGroupFromMediaType(mtype, "other");
        QStringList folderlst = group_folder.value(group,QStringList());
        QList<int> countlst = group_count.value(group, QList<int>());
        QString sdir = Utility::startingDir(bookpath);
        if (!folderlst.contains(sdir)) {
            folderlst << sdir;
            countlst << 1;
        } else {
            int pos = folderlst.indexOf(sdir);
            countlst.replace(pos, countlst.at(pos) + 1);
        }
        group_folder[group] = folderlst;
        group_count[group] = countlst;
        i++;
    }

    // finally sort each group's list of folders by number 
    // of files of that type in each folder.
    // the default folder for that group will be the first
    QStringList dirlst;
    bool use_lower_case = false;
    QStringList keys = group_folder.keys();
    foreach(QString group, keys) {
        QStringList folderlst = group_folder[group];
        QList<int> countlst = group_count[group];
        QStringList sortedlst = Utility::sortByCounts(folderlst, countlst);
        group_folder[group] = sortedlst;
        if (groupA.contains(group)) {
            QString afolder = sortedlst.at(0);
            if (afolder.indexOf(group.toLower()) > -1) use_lower_case = true;
        }
        dirlst << sortedlst.at(0);
    }
    // now back fill any missing group folders
    QString commonbase = Utility::longestCommonPath(dirlst, "/");
    if (commonbase == "/") commonbase ="";
    foreach(QString group, groupA) {
        QStringList folderlst = group_folder.value(group, QStringList());
        QString gname = group;
        if (use_lower_case) gname = gname.toLower();
        if (folderlst.isEmpty()) {
            folderlst << commonbase + gname;
            group_folder[group] = folderlst;
        }
    }

    // update m_GrpToFold with result
    m_GrpToFold.clear();
    m_GrpToFold = group_folder;
}

void FolderKeeper::PerformInitialLoads()
{
    QList<Resource *> resources = GetResourceList();
    foreach(Resource * resource, resources) {
        TextResource * text_resource = qobject_cast<TextResource*>(resource);
	if (text_resource) {
	    text_resource->InitialLoad();
	}
    }
}
