/************************************************************************
**
**  Copyright (C) 2020 Kevin B. Hendricks, Stratford, ON, Canada
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

#include <QtCore/QDate>
#include <QtCore/QModelIndex>

#include "Dialogs/SelectCheckpoint.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"

static const QString SETTINGS_GROUP = "select_checkpoint";

SelectCheckpoint::SelectCheckpoint(const QStringList &checkpointlst, QWidget *parent)
    :
    QDialog(parent)
{
    ui.setupUi(this);
    foreach(QString atag, checkpointlst) {
	QStringList fields = atag.split("|");
	if (fields.length() == 3) {
	    QString key = fields.at(0);
	    QString dinfo = fields.at(1) + "\n" + fields.at(2);
	    m_CheckpointInfo[key] = dinfo;
            ui.lwProperties->addItem(key);
	}
    }

    connect(ui.lwProperties, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this,	          SLOT(UpdateDescription(QListWidgetItem *)));
    connect(this, SIGNAL(accepted()), this, SLOT(WriteSettings()));
    connect(ui.lwProperties, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(accept()));

    ReadSettings();
}

void SelectCheckpoint::UpdateDescription(QListWidgetItem *current)
{
    QString text;
    QString key = current->text();
    if (!key.isEmpty()) {
        text = m_CheckpointInfo.value(key, "");
    }
    if (!text.isEmpty()) {
        ui.lbDescription->setText(text);
    }
}

QStringList SelectCheckpoint::GetSelectedEntries()
{
    return m_SelectedEntries;
}

void SelectCheckpoint::SaveSelection()
{
    m_SelectedEntries.clear();
    foreach(QListWidgetItem * item, ui.lwProperties->selectedItems()) {
        QString key = item->text();
	if (!key.isEmpty()) {
            m_SelectedEntries.append(key);
	}
    }
}


void SelectCheckpoint::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    QByteArray splitter_position = settings.value("splitter").toByteArray();

    if (!splitter_position.isNull()) {
        ui.splitter->restoreState(splitter_position);
    }

    settings.endGroup();
}


void SelectCheckpoint::WriteSettings()
{
    SaveSelection();
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    // The position of the splitter handle
    settings.setValue("splitter", ui.splitter->saveState());
    settings.endGroup();
}
