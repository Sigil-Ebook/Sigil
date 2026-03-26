/************************************************************************
**
**  Copyright (C) 2016-2026 Kevin B. Hendricks, Stratford, Ontario, Canada
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

#include <QStringList>
#include <QListWidgetItem>
#include <QKeyEvent>
#include <QDebug>

#include "Dialogs/EditRO.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"

static const QString SETTINGS_GROUP   = "edit_ro";

EditRO::EditRO(QStringList bookpaths, QString selected, QWidget *parent)
    :
    QDialog(parent),
    m_BookPaths(bookpaths),
    m_updated(false)
{
    ui.setupUi(this);
    int row = 0;
    int selected_row = -1;
    foreach (QString bp, m_BookPaths) {
        ui.ROList->addItem(bp);
	if (bp == selected) selected_row = row;
	row++;
    }
    ui.ROList->installEventFilter(this);
    ConnectSignalsToSlots();

    ReadSettings();
    if (!selected.isEmpty()) {
        SetSelectedRow(selected_row);
    }
}


EditRO::~EditRO()
{
    WriteSettings();
}


QStringList EditRO::GetNewReadingOrder()
{
    if (m_updated) {
        return m_BookPaths;
    }
    return QStringList();
}


void EditRO::SetSelectedRow(int r)
{
    if ((r > -1) && (r < ui.ROList->count())) {
        ui.ROList->setCurrentRow(r);
    }
}


void EditRO::Save()
{
    m_BookPaths.clear();
    for(int i=0; i < ui.ROList->count(); i++) {
        QListWidgetItem* item = ui.ROList->item(i);
        m_BookPaths << item->text();
        // qDebug() << item->text();
    }
    m_updated = true;
}


void EditRO::ToTop()
{
    int current_row = ui.ROList->currentRow();
    if (current_row > 0) {
        QListWidgetItem* item = ui.ROList->takeItem(current_row);
        ui.ROList->insertItem(0, item);
        ui.ROList->setCurrentRow(0);
    }
}

void EditRO::ToBottom()
{
    int current_row = ui.ROList->currentRow();
    int last = ui.ROList->count() - 1;
    if ((current_row < ui.ROList->count() - 1) && (current_row != -1)) {
        QListWidgetItem* item = ui.ROList->takeItem(current_row);
        ui.ROList->insertItem(last, item);
        ui.ROList->setCurrentRow(last);
    }
}

void EditRO::MoveUp()
{
    int current_row = ui.ROList->currentRow();
    if (current_row > 0) {
        QListWidgetItem* item = ui.ROList->takeItem(current_row);
        ui.ROList->insertItem(current_row - 1, item);
        ui.ROList->setCurrentRow(current_row - 1);
    }
}

void EditRO::MoveDown()
{
    int current_row = ui.ROList->currentRow();
    if ((current_row < ui.ROList->count() - 1) && (current_row != -1)) {
        QListWidgetItem* item = ui.ROList->takeItem(current_row);
        ui.ROList->insertItem(current_row + 1, item);
        ui.ROList->setCurrentRow(current_row + 1);
    }
}


void EditRO::ReadSettings()
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

void EditRO::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}


bool EditRO::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui.ROList) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();

            if (key == Qt::Key_D) {
                MoveDown();
                return true;
            } else if (key == Qt::Key_U) {
                MoveUp();
                return true;
            } else if (key == Qt::Key_F) {
                ToTop();
                return true;
            } else if (key == Qt::Key_L) {
                ToBottom();
                return true;
            }
        }
    }
    // pass the event on to the parent class
    return QDialog::eventFilter(obj, event);
}


void EditRO::ConnectSignalsToSlots()
{
    connect(this,               SIGNAL(accepted()),           this, SLOT(Save()));
    connect(ui.ToTop,           SIGNAL(clicked()),            this, SLOT(ToTop()));
    connect(ui.ToEnd,           SIGNAL(clicked()),            this, SLOT(ToBottom()));
    connect(ui.MoveUp,          SIGNAL(clicked()),            this, SLOT(MoveUp()));
    connect(ui.MoveDown,        SIGNAL(clicked()),            this, SLOT(MoveDown()));
}
