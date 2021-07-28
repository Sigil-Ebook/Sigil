/************************************************************************
**
**  Copyright (C) 2021 Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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

#include <QtGui/QStandardItem>

#include "Dialogs/LinkJavascripts.h"
#include "Misc/SettingsStore.h"
#include "sigil_constants.h"

static const QString SETTINGS_GROUP   = "link_javascripts";

// Constructor;
LinkJavascripts::LinkJavascripts(QList<std::pair<QString, bool>> javascripts_map, QWidget *parent)
    :
    QDialog(parent),
    m_JavascriptsMap(javascripts_map)
{
    ui.setupUi(this);
    ConnectSignalsToSlots();
    ui.JavascriptsView->setModel(&m_JavascriptsModel);
    CreateJavascriptsModel();
    UpdateTreeViewDisplay();
    ReadSettings();
}


// Updates the display of the tree view (resize columns)
void LinkJavascripts::UpdateTreeViewDisplay()
{
    ui.JavascriptsView->expandAll();
    ui.JavascriptsView->resizeColumnToContents(0);
    ui.JavascriptsView->setColumnWidth(0, ui.JavascriptsView->columnWidth(0));
    ui.JavascriptsView->setCurrentIndex(m_JavascriptsModel.index(0, 0));
}

// Creates the model that is displayed in the tree view
void LinkJavascripts::CreateJavascriptsModel()
{
    m_JavascriptsModel.clear();
    QStringList header;
    header.append(tr("Include"));
    header.append(tr("Javascript"));
    m_JavascriptsModel.setHorizontalHeaderLabels(header);

    // Inserts all entries
    for (int i = 0; i < m_JavascriptsMap.count(); i++) {
        InsertJavascriptIntoModel(m_JavascriptsMap.at(i));
    }
}


// Inserts the specified heading into the model
void LinkJavascripts::InsertJavascriptIntoModel(std::pair<QString, bool> javascript_pair)
{
    QStandardItem *item_bookpath       = new QStandardItem(javascript_pair.first);
    QStandardItem *item_included_check = new QStandardItem();
    item_included_check->setEditable(false);
    item_included_check->setCheckable(true);
    item_bookpath->setEditable(false);
    item_bookpath->setDragEnabled(false);
    item_bookpath->setDropEnabled(false);

    if (javascript_pair.second) {
        item_included_check->setCheckState(Qt::Checked);
    } else {
        item_included_check->setCheckState(Qt::Unchecked);
    }

    QList<QStandardItem *> items;
    items << item_included_check << item_bookpath;
    m_JavascriptsModel.invisibleRootItem()->appendRow(items);
}


// Reads all the stored dialog settings like window position, size, etc.
void LinkJavascripts::ReadSettings()
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


// Writes all the stored dialog settings like window position, size, etc.
void LinkJavascripts::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}

void LinkJavascripts::MoveUp()
{
    QModelIndexList selected_indexes = ui.JavascriptsView->selectionModel()->selectedIndexes();

    if (selected_indexes.isEmpty()) {
        return;
    }

    QModelIndex index = selected_indexes.first();
    int row = index.row();

    if (row == 0) {
        return;
    }

    QList<QStandardItem *> items =  m_JavascriptsModel.invisibleRootItem()->takeRow(row - 1);
    m_JavascriptsModel.invisibleRootItem()->insertRow(row, items);
}

void LinkJavascripts::MoveDown()
{
    QModelIndexList selected_indexes = ui.JavascriptsView->selectionModel()->selectedIndexes();

    if (selected_indexes.isEmpty()) {
        return;
    }

    QModelIndex index = selected_indexes.first();
    int row = index.row();

    if (row == m_JavascriptsModel.invisibleRootItem()->rowCount() - 1) {
        return;
    }

    QList<QStandardItem *> items =  m_JavascriptsModel.invisibleRootItem()->takeRow(row + 1);
    m_JavascriptsModel.invisibleRootItem()->insertRow(row, items);
}


void LinkJavascripts::UpdateJavascripts()
{
    m_Javascripts.clear();
    int rows = m_JavascriptsModel.invisibleRootItem()->rowCount();

    for (int row = 0; row < rows; row++) {
        QList<QStandardItem *> items =  m_JavascriptsModel.invisibleRootItem()->takeRow(0);

        if (items.at(0)->checkState() == Qt::Checked) {
            m_Javascripts << items.at(1)->data(Qt::DisplayRole).toString();
        }
    }
}

QStringList LinkJavascripts::GetJavascripts()
{
    return m_Javascripts;
}

void LinkJavascripts::ConnectSignalsToSlots()
{
    connect(ui.MoveUp, SIGNAL(clicked()),  this, SLOT(MoveUp()));
    connect(ui.MoveDown, SIGNAL(clicked()),  this, SLOT(MoveDown()));
    connect(this, SIGNAL(accepted()), this, SLOT(UpdateJavascripts()));
    connect(this, SIGNAL(accepted()), this, SLOT(WriteSettings()));
}
