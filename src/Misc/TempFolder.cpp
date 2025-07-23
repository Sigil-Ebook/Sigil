/************************************************************************
**
**  Copyright (C) 2015-2023 Kevin B. Hendricks, Stratford Ontario Canada
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

#include <QDir>
#include <QtConcurrent>
#include <QFuture>
#include <QDebug>

#include "Misc/TempFolder.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"

TempFolder::TempFolder()
    : m_tempDir(GetNewTempFolderTemplate())
{
    // verify m_tempDir was properly created
    if (!m_tempDir.isValid()) {
        qDebug() << "Error: Invalid m_tempDir" << m_tempDir.path();
    }

    // will be cleaned manually in the destructor
    m_tempDir.setAutoRemove(false);
}


TempFolder::TempFolder(const QString base_path)
    : m_tempDir(GetNewTempFolderTemplateFromBasePath(base_path))
{
    // verify m_tempDir was properly created
    if (!m_tempDir.isValid()) {
        qDebug() << "Error: Invalid m_tempDir" << m_tempDir.path();
    }

    // will be cleaned manually in the destructor
    m_tempDir.setAutoRemove(false);
}


TempFolder::~TempFolder()
{
    // To be super safe here ...
    // only manually delete things if the temp directory is actually valid
    if (m_tempDir.isValid()) {
        QFuture<bool> afuture = QtConcurrent::run(DeleteFolderAndFiles, m_tempDir.path());
    }
}


QString TempFolder::GetPath()
{
    return m_tempDir.path();
}


QString TempFolder::GetPathToSigilScratchpad()
{
    SettingsStore ss;
    QString temp_path = ss.tempFolderHome();
    if (temp_path == "<SIGIL_DEFAULT_TEMP_HOME>") {
        return Utility::DefinePrefsDir() + "/workspace";
    }
    if (!QDir(temp_path).exists()) {
        return Utility::DefinePrefsDir() + "/workspace";;
    }
    return temp_path;
}


QString TempFolder::GetNewTempFolderTemplate()
{
    SettingsStore ss;
    QString temp_path = ss.tempFolderHome();
    if (temp_path == "<SIGIL_DEFAULT_TEMP_HOME>") {
       return Utility::DefinePrefsDir() + "/workspace"  + "/Sigil-XXXXXX";
    }
    if (!QDir(temp_path).exists()) {
        return Utility::DefinePrefsDir() + "/workspace" + "/Sigil-XXXXXX";;
    }
    return temp_path + "/Sigil-XXXXXX"; 
}


QString TempFolder::GetNewTempFolderTemplateFromBasePath(const QString base_path)
{
    return base_path + "/Sigil-XXXXXX";
}


bool TempFolder::DeleteFolderAndFiles(const QString &fullfolderpath)
{
    QDir folder(fullfolderpath);
    return folder.removeRecursively();
}



