/************************************************************************
**
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

#include <QtCore/QCoreApplication>
#include <QByteArray>
#include <QDataStream>

#include "MiscEditors/IndexEditorModel.h"

static const QString SETTINGS_GROUP         = "index_entries";
static const QString ENTRY_PATTERN          = "Text to Include";
static const QString ENTRY_INDEX_ENTRY      = "Index Entries";

const int COLUMNS = 2;

static const int IS_GROUP_ROLE = Qt::UserRole + 1;
static const int FULLNAME_ROLE = Qt::UserRole + 2;


IndexEditorModel *IndexEditorModel::m_instance = 0;

IndexEditorModel *IndexEditorModel::instance()
{
    if (m_instance == 0) {
        m_instance = new IndexEditorModel();
    }

    return m_instance;
}

IndexEditorModel::IndexEditorModel(QObject *parent)
 : QStandardItemModel(parent)
{
    LoadInitialData();
}

IndexEditorModel::~IndexEditorModel()
{
    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
}

void IndexEditorModel::ClearData()
{
    clear();

    QStringList header;
    header.append(tr("Text to Include"));
    header.append(tr("Index Entries"));

    setHorizontalHeaderLabels(header);
}

void IndexEditorModel::LoadInitialData(QString filename)
{
    ClearData();

    LoadData(filename);
}

void IndexEditorModel::LoadData(QString filename, QStandardItem *item)
{
    SettingsStore *settings;
    if (filename.isEmpty()) {
        settings = new SettingsStore();
    }
    else {
        settings = new SettingsStore(filename);
    }

    int size = settings->beginReadArray(SETTINGS_GROUP);

    // Add one entry at a time to the list
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);

        IndexEditorModel::indexEntry *entry = new IndexEditorModel::indexEntry();

        entry->pattern = settings->value(ENTRY_PATTERN).toString();
        entry->index_entry = settings->value(ENTRY_INDEX_ENTRY).toString();

        if (!entry->pattern.isEmpty() || !entry->index_entry.isEmpty()) {
            AddFullNameEntry(entry, item);
        }
    }

    settings->endArray();
}

void IndexEditorModel::AddFullNameEntry(IndexEditorModel::indexEntry *entry, QStandardItem *parent_item, int row)
{
    if (!parent_item) {
        parent_item = invisibleRootItem();
    }

    if (parent_item != invisibleRootItem()) {
        row = parent_item->row() + 1;
        parent_item = parent_item->parent();
    }

    if (row < 0) {
        row = parent_item->rowCount();
    }
        AddEntryToModel(entry, false, parent_item, row);
}

QStandardItem *IndexEditorModel::AddEntryToModel(IndexEditorModel::indexEntry *entry, bool is_group, QStandardItem *parent_item, int row)
{
    // parent_item must be a group item

    if (!parent_item) {
        parent_item = invisibleRootItem();
    }

    // Create an empty entry if none supplied
    if (!entry) {
        entry = new IndexEditorModel::indexEntry();
            entry->pattern = "";
            entry->index_entry = "";
    }

    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(entry->pattern);
    rowItems << new QStandardItem(entry->index_entry);

    // Add the new item to the model at the specified row
    QStandardItem *new_item;
    if (row < 0 || row >= parent_item->rowCount()) {
        parent_item->appendRow(rowItems);
        new_item = parent_item->child(parent_item->rowCount() - 1, 0);
    }
    else {
        parent_item->insertRow(row, rowItems);
        new_item = parent_item->child(row, 0);
    }

    return new_item;
}

QList<QStandardItem *> IndexEditorModel::GetItems()
{
    QList<QStandardItem *> items;

    for (int row = 0; row < invisibleRootItem()->rowCount(); row++) {
        items.append(invisibleRootItem()->child(row, 0));
    }

    return items;
}

QList<IndexEditorModel::indexEntry *> IndexEditorModel::GetEntries(QList<QStandardItem*> items)
{
    if (!items.count()) {
        items = GetItems();
    }

    QList<IndexEditorModel::indexEntry *> entries;

    foreach (QStandardItem *item, items) {
        entries.append(GetEntry(item));
    }

    return entries;
}

IndexEditorModel::indexEntry* IndexEditorModel::GetEntry(QStandardItem *item)
{
    QStandardItem *parent_item;

    if (item->parent()) {
        parent_item = item->parent();
    }
    else {
        parent_item = invisibleRootItem();
    }

    IndexEditorModel::indexEntry *entry = new IndexEditorModel::indexEntry();

    entry->pattern =        parent_item->child(item->row(), 0)->text();
    entry->index_entry = parent_item->child(item->row(), 1)->text();

    return entry;
}

QString IndexEditorModel::SaveData(QList<IndexEditorModel::indexEntry*> entries, QString filename)
{
    QString message = "";

    // Save everything if no entries selected
    if (entries.isEmpty()) {
        QList<QStandardItem *> items = GetItems();
        if (!items.isEmpty()) {
            entries = GetEntries(items);
        }
    }

    // Open the default file for save, or specific file for export
    SettingsStore *settings;
    if (filename.isEmpty()) {
        settings = new SettingsStore();
    }
    else {
        settings = new SettingsStore(filename);
    }

    settings->sync();
    if (!settings->isWritable()) {
        message = tr("Unable to create file %1").arg(filename);
        return message;
    }

    // Remove the old values to account for deletions
    settings->remove(SETTINGS_GROUP);

    settings->beginWriteArray(SETTINGS_GROUP);

    int i = 0;
    foreach (IndexEditorModel::indexEntry *entry, entries) {
        if (entry->pattern.isEmpty() && entry->index_entry.isEmpty()) {
            continue;
        }
        settings->setArrayIndex(i++);
        settings->setValue(ENTRY_PATTERN, entry->pattern);
        settings->setValue(ENTRY_INDEX_ENTRY, entry->index_entry);
    }

    settings->endArray();

    // Make sure file is created/updated so it can be checked
    settings->sync();

    return message;
}
