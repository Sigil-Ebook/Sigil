/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
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

#include <QtWidgets/QCompleter>
#include <QtWidgets/QLineEdit>

#include "Dialogs/SelectFolder.h"
#include "BookManipulation/FolderKeeper.h"
#include "Misc/SettingsStore.h"

static QString SETTINGS_GROUP = "select_folder";
static QString PLACEHOLDER = "<epub root>";

SelectFolder::SelectFolder(QString group, QSharedPointer<Book> book, QWidget *parent)
    :
    QDialog(parent),
    m_SelectedText(QString()),
    m_group(group),
    m_Book(book)
{
    ui.setupUi(this);

    QCompleter *qc = ui.fold->completer();
    qc->setCaseSensitivity(Qt::CaseSensitive);

    connectSignalsSlots();
    ReadSettings();
    SetList();
}

void SelectFolder::SetList()
{
    QStringList folders = m_Book->GetFolderKeeper()->GetFoldersForGroup(m_group);
    foreach(QString folder, folders) {
        if (folder.isEmpty()) {
	    ui.fold->addItem(PLACEHOLDER);
        } else{ 
            ui.fold->addItem(folder);
        }
    }
    // Set default id name
    ui.fold->setEditText(m_SelectedText);
}


QString SelectFolder::GetFolder()
{
    if (m_SelectedText == PLACEHOLDER) m_SelectedText = "";
    return m_SelectedText;
}

void SelectFolder::SetSelectedText()
{
    m_SelectedText = ui.fold->currentText();
}

void SelectFolder::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    settings.endGroup();
}

void SelectFolder::WriteSettings()
{
    SetSelectedText();
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void SelectFolder::connectSignalsSlots()
{
    connect(this, SIGNAL(accepted()), this, SLOT(WriteSettings()));
}
