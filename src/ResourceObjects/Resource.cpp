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
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtWidgets/QFileIconProvider>

#include "Misc/Utility.h"
#include "ResourceObjects/Resource.h"

const int WAIT_FOR_WRITE_DELAY = 100;

Resource::Resource(const QString &mainfolder, const QString &fullfilepath, QObject *parent)
    :
    QObject(parent),
    m_Identifier(Utility::CreateUUID()),
    m_MainFolder(mainfolder),
    m_FullFilePath(fullfilepath),
    m_LastSaved(0),
    m_LastWrittenTo(0),
    m_LastWrittenSize(0),
    m_CurrentBookRelPath(""),
    m_EpubVersion("2.0"),
    m_ReadWriteLock(QReadWriteLock::Recursive)
{
}

bool Resource::operator< (const Resource &other)
{
    return Filename() < other.Filename();
}


QString Resource::GetIdentifier() const
{
    return m_Identifier;
}


QString Resource::Filename() const
{
    return QFileInfo(m_FullFilePath).fileName();
}


QString Resource::GetFolder() const
{
    // Pathname of the directory within the EPUB.
    return QFileInfo(GetRelativePath()).absolutePath().remove(0, 1);
}


QString Resource::GetRelativePath() const
{
    // Pathname of the file within the EPUB.
    return m_FullFilePath.right(m_FullFilePath.length() - m_MainFolder.length());
}

QString Resource::GetRelativePathToOEBPS() const
{
    return QFileInfo(m_FullFilePath).dir().dirName() + "/" + Filename();
}


QString Resource::GetRelativePathToRoot() const
{
    QFileInfo info(m_FullFilePath);
    QDir parent_dir = info.dir();
    QString parent_name = parent_dir.dirName();
    parent_dir.cdUp();
    QString grandparent_name = parent_dir.dirName();
    return grandparent_name + "/" + parent_name + "/" + Filename();
}


QString Resource::GetFullPath() const
{
    return m_FullFilePath;
}


QUrl Resource::GetBaseUrl() const
{
    return QUrl::fromLocalFile(QFileInfo(m_FullFilePath).absolutePath() + "/");
}


void Resource::SetCurrentBookRelPath(const QString& current_path)
{
    m_CurrentBookRelPath = current_path;
}


QString Resource::GetCurrentBookRelPath()
{
  if (m_CurrentBookRelPath.isEmpty()) {
      return GetRelativePathToRoot();
  }
  return m_CurrentBookRelPath;
}

void Resource::SetEpubVersion(const QString& version)
{
    m_EpubVersion = version;
}


QString Resource::GetEpubVersion() const
{
  return m_EpubVersion;
}


QReadWriteLock &Resource::GetLock() const
{
    return m_ReadWriteLock;
}


QIcon Resource::Icon() const
{
    return QFileIconProvider().icon(QFileInfo(m_FullFilePath));
}


bool Resource::RenameTo(const QString &new_filename)
{
    QString new_path;
    bool successful = false;
    {
        QWriteLocker locker(&m_ReadWriteLock);
        new_path = QFileInfo(m_FullFilePath).absolutePath() + "/" + new_filename;
        successful = Utility::RenameFile(m_FullFilePath, new_path);
    }

    if (successful) {
        QString old_path = m_FullFilePath;
        m_FullFilePath = new_path;
        emit Renamed(this, old_path);
    }

    return successful;
}

bool Resource::Delete()
{
    bool successful = false;
    {
        QWriteLocker locker(&m_ReadWriteLock);
        successful = Utility::SDeleteFile(m_FullFilePath);
    }

    if (successful) {
        emit Deleted(this);
        deleteLater();
    }

    return successful;
}


Resource::ResourceType Resource::Type() const
{
    return Resource::GenericResourceType;
}

bool Resource::LoadFromDisk()
{
    return false;
}

void Resource::SaveToDisk(bool book_wide_save)
{
    const QDateTime lastModifiedDate = QFileInfo(m_FullFilePath).lastModified();

    if (lastModifiedDate.isValid()) {
        m_LastSaved = lastModifiedDate.toMSecsSinceEpoch();
    }
}

void Resource::FileChangedOnDisk()
{
    QFileInfo latestFileInfo(m_FullFilePath);
    const QDateTime lastModifiedDate = latestFileInfo.lastModified();
    m_LastWrittenTo = lastModifiedDate.isValid() ? lastModifiedDate.toMSecsSinceEpoch() : 0;
    m_LastWrittenSize = latestFileInfo.size();
    QTimer::singleShot(WAIT_FOR_WRITE_DELAY, this, SLOT(ResourceFileModified()));
}

void Resource::ResourceFileModified()
{
    QFileInfo newFileInfo(m_FullFilePath);
    const QDateTime lastModifiedDate = newFileInfo.lastModified();
    qint64 latestWrittenTo = lastModifiedDate.isValid() ? lastModifiedDate.toMSecsSinceEpoch() : 0;
    qint64 latestWrittenSize = newFileInfo.size();

    if (latestWrittenTo == m_LastSaved) {
        // The FileChangedOnDisk has triggered even though the data in the file has not changed.
        // This can happen if the FileWatcher is monitoring a file that Sigil has just performed
        // a disk operation with, such as Saving before a Merge. In this circumstance the data
        // loaded in memory by Sigil may be more up to date than that on disk (such as after the
        // merge but before user has chosen to Save) so we want to ignore the file change notification.
        return;
    }

    if ((latestWrittenTo != m_LastWrittenTo) || (latestWrittenSize != m_LastWrittenSize)) {
        // The file is still being written to.
        m_LastWrittenTo = latestWrittenTo;
        m_LastWrittenSize = latestWrittenSize;
        QTimer::singleShot(WAIT_FOR_WRITE_DELAY, this, SLOT(ResourceFileModified()));
    } else {
        if (LoadFromDisk()) {
            // will trigger marking the book as modified
            emit ResourceUpdatedFromDisk(this);
        }

        // will trigger updates in other resources that link to this resource
        emit ResourceUpdatedOnDisk();
    }
}
