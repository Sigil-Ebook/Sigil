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
#include <QtConcurrent/QtConcurrent>
#include <QDebug>

#include "Misc/TempFolder.h"
#include "Misc/SettingsStore.h"

TempFolder::TempFolder()
    : m_tempDir(GetNewTempFolderTemplate())
{
    // verify m_tempDir was properly created
    if (!m_tempDir.isValid()) {
        qDebug() << "Invalid m_tempDir" << m_tempDir.path();
    }

    // will be cleaned manually in the destructor
    m_tempDir.setAutoRemove(false);
}

TempFolder::~TempFolder()
{
    // To be super safe here ...
    // only manually delete things if the temp directory is actually valid
    if (m_tempDir.isValid()) {
        QtConcurrent::run(DeleteFolderAndFiles, m_tempDir.path());
    }
}


QString TempFolder::GetPath()
{
    return m_tempDir.path();
}


QString TempFolder::GetPathToSigilScratchpad()
{
    SettingsStore ss;
    if (!QDir(ss.tempFolderHome()).exists()) {
        return QDir::tempPath();
    }
    return ss.tempFolderHome();
}


QString TempFolder::GetNewTempFolderTemplate()
{
    SettingsStore ss;
    if (!QDir(ss.tempFolderHome()).exists()) {
        return QDir::tempPath() + "/Sigil-XXXXXX";
    }
    return ss.tempFolderHome() + "/Sigil-XXXXXX"; 
}


bool TempFolder::DeleteFolderAndFiles(const QString &fullfolderpath)
{
    QDir folder(fullfolderpath);
    return folder.removeRecursively();
}



