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

#include <QtCore/QtCore>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtConcurrent/QtConcurrent>

#include "Misc/TempFolder.h"
#include "Misc/Utility.h"

const QString PATH_SUFFIX = "/Sigil";

TempFolder::TempFolder()
    : m_PathToFolder(GetNewTempFolderPath())
{
    QDir folder(m_PathToFolder);
    folder.mkpath(folder.absolutePath());
}


TempFolder::~TempFolder()
{
    QtConcurrent::run(DeleteFolderAndFiles, m_PathToFolder);
}


QString TempFolder::GetPath()
{
    return m_PathToFolder;
}


QString TempFolder::GetPathToSigilScratchpad()
{
    return QDir::tempPath() + PATH_SUFFIX + "/scratchpad";
}


QString TempFolder::GetNewTempFolderPath()
{
    QString token = Utility::CreateUUID();
    return GetPathToSigilScratchpad() + "/" + token;
}


bool TempFolder::DeleteFolderAndFiles(const QString &fullfolderpath)
{
    // Make sure the path exists, otherwise very
    // bad things could happen
    if (!QFileInfo(fullfolderpath).exists()) {
        return false;
    }

    QDir folder(fullfolderpath);
    // Erase all the files in this folder
    foreach(QFileInfo file, folder.entryInfoList()) {
        if ((file.fileName() != ".") && (file.fileName() != "..")) {
            // If it's a file, delete it
            if (file.isFile()) {
                folder.remove(file.fileName());
            }
            // Else it's a directory, delete it recursively
            else {
                DeleteFolderAndFiles(file.absoluteFilePath());
            }
        }
    }
    // Delete the folder after it's empty
    folder.rmdir(folder.absolutePath());
    return true;
}



