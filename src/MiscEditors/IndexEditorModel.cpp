/************************************************************************
**
**  Copyright (C) 2019 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#include <QtCore/QTime>

#include "MiscEditors/IndexEditorModel.h"
#include "Misc/Utility.h"

static const QString SETTINGS_FILE          = "sigil_index.ini";
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
    : QStandardItemModel(parent),
      m_FSWatcher(new QFileSystemWatcher()),
      m_IsDataModified(false)
{
    m_SettingsPath = Utility::DefinePrefsDir() + "/" + SETTINGS_FILE;
    QStringList header;
    header.append(tr("Text to Include"));
    header.append(tr("Index Entries"));
    setHorizontalHeaderLabels(header);
    LoadInitialData();
    // Save it to make sure we have a file in case it was loaded from examples
    SaveData();

    if (!m_FSWatcher->files().contains(m_SettingsPath)) {
        m_FSWatcher->addPath(m_SettingsPath);
    }

    connect(m_FSWatcher, SIGNAL(fileChanged(const QString &)),
            this,        SLOT(SettingsFileChanged(const QString &)), Qt::DirectConnection);
    connect(this, SIGNAL(itemChanged(QStandardItem *)),
            this, SLOT(ItemChangedHandler(QStandardItem *)));
    connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
            this, SLOT(RowsRemovedHandler(const QModelIndex &, int, int)));
}

IndexEditorModel::~IndexEditorModel()
{
    delete m_FSWatcher;
    m_FSWatcher = 0;

    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
}

void IndexEditorModel::SetDataModified(bool modified)
{
    m_IsDataModified = modified;
}

bool IndexEditorModel::IsDataModified()
{
    return m_IsDataModified;
}

void IndexEditorModel::RowsRemovedHandler(const QModelIndex &parent, int start, int end)
{
    SetDataModified(true);
}

void IndexEditorModel::ItemChangedHandler(QStandardItem *item)
{
    Q_ASSERT(item);
    SetDataModified(true);

    if (item->column() != 0) {
        return;
    }

    // Split the entry into multiple entries if there is a return in it
    if (item->text().contains("\n")) {
        SplitEntry(item);
    }
}

void IndexEditorModel::SplitEntry(QStandardItem *item)
{
    // Disconnect change signal while changing the items
    disconnect(this, SIGNAL(itemChanged(QStandardItem *)),
               this, SLOT(ItemChangedHandler(QStandardItem *)));
    IndexEditorModel::indexEntry *entry = GetEntry(item);
    QStringList names = entry->pattern.split("\n");
    QString first_name = names.first();
    names.removeFirst();
    // Update the first entry
    item->setText(first_name);
    foreach(QString name, names) {
        entry->pattern = name;
        QStandardItem *new_item = AddFullNameEntry(entry, item);
        item = new_item;
    }
    connect(this, SIGNAL(itemChanged(QStandardItem *)),
            this, SLOT(ItemChangedHandler(QStandardItem *)));
}

void IndexEditorModel::ClearData()
{
    removeRows(0, rowCount());
}

void IndexEditorModel::SettingsFileChanged(const QString &path) const
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

        instance()->LoadInitialData();
        emit SettingsFileUpdated();
    }
}

void IndexEditorModel::LoadInitialData(const QString &filename)
{
    ClearData();
    LoadData(filename);
    SetDataModified(false);
}

void IndexEditorModel::LoadData(const QString &filename, QStandardItem *item)
{
    // Read text files with one line per entry
    if (filename.endsWith(".txt")) {
        QString index_data = Utility::ReadUnicodeTextFile(filename);
        foreach(QString line, index_data.split("\n")) {
            IndexEditorModel::indexEntry *entry = new IndexEditorModel::indexEntry();
            // Split on tab if present
            if (line.contains("\t")) {
                int tab_position = line.indexOf("\t");
                entry->pattern = line.left(tab_position);
                entry->index_entry = line.right(line.length() - tab_position - 1);
            } else {
                entry->pattern = line;
            }
            AddFullNameEntry(entry, item);
	    delete entry;
        }
        return;
    }

    // Read standard ini files
    QString settings_path = filename;
    if (settings_path.isEmpty()) settings_path = m_SettingsPath;

    SettingsStore* ss = new SettingsStore(settings_path);

    int size = ss->beginReadArray(SETTINGS_GROUP);

    // Add one entry at a time to the list
    for (int i = 0; i < size; ++i) {
        ss->setArrayIndex(i);
        IndexEditorModel::indexEntry *entry = new IndexEditorModel::indexEntry();
        entry->pattern = ss->value(ENTRY_PATTERN).toString();
        entry->index_entry = ss->value(ENTRY_INDEX_ENTRY).toString();

        if (!entry->pattern.isEmpty() || !entry->index_entry.isEmpty()) {
            AddFullNameEntry(entry, item);
        }
	delete entry;
    }

    ss->endArray();
    delete ss;
}

QStandardItem *IndexEditorModel::AddFullNameEntry(IndexEditorModel::indexEntry *entry, QStandardItem *parent_item, int row)
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

    return AddEntryToModel(entry, parent_item, row);
}

QStandardItem *IndexEditorModel::AddEntryToModel(IndexEditorModel::indexEntry *entry, QStandardItem *parent_item, int row)
{
    bool clean_up = false;

    // parent_item must be a group item
    if (!parent_item) {
        parent_item = invisibleRootItem();
    }

    // Create an empty entry if none supplied
    if (!entry) {
        entry = new IndexEditorModel::indexEntry();
        entry->pattern = "";
        entry->index_entry = "";
	clean_up = true;
    }

    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(entry->pattern);
    rowItems << new QStandardItem(entry->index_entry);
    // Add the new item to the model at the specified row
    QStandardItem *new_item;

    if (row < 0 || row >= parent_item->rowCount()) {
        parent_item->appendRow(rowItems);
        new_item = parent_item->child(parent_item->rowCount() - 1, 0);
    } else {
        parent_item->insertRow(row, rowItems);
        new_item = parent_item->child(row, 0);
    }

    SetDataModified(true);
    if (clean_up) delete entry;

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

QList<IndexEditorModel::indexEntry *> IndexEditorModel::GetEntries(QList<QStandardItem *> items)
{
    if (!items.count()) {
        items = GetItems();
    }

    QList<IndexEditorModel::indexEntry *> entries;
    foreach(QStandardItem * item, items) {
        entries.append(GetEntry(item));
    }
    return entries;
}

IndexEditorModel::indexEntry *IndexEditorModel::GetEntry(QStandardItem *item)
{
    QStandardItem *parent_item;

    if (item->parent()) {
        parent_item = item->parent();
    } else {
        parent_item = invisibleRootItem();
    }

    IndexEditorModel::indexEntry *entry = new IndexEditorModel::indexEntry();
    entry->pattern =        parent_item->child(item->row(), 0)->text();
    entry->index_entry = parent_item->child(item->row(), 1)->text();
    return entry;
}

QString IndexEditorModel::SaveData(QList<IndexEditorModel::indexEntry *> entries, const QString &filename)
{
    QString message = "";
    bool clean_up_needed = false;
    QString settings_path = filename;
    if (settings_path.isEmpty()) settings_path = m_SettingsPath;

    // Save everything if no entries selected
    if (entries.isEmpty()) {
        QList<QStandardItem *> items = GetItems();

        if (!items.isEmpty()) {
            // GetEntries calls GetEntry which creates each entry with new
            entries = GetEntries(items);
	    clean_up_needed = true;
        }
    }

    // Stop watching the file while we save it
    if (m_FSWatcher->files().contains(settings_path)) {
        m_FSWatcher->removePath(settings_path);
    }

    // Open the default file for save, or specific file for export
    SettingsStore* ss = new SettingsStore(settings_path);

    ss->sync();

    if (!ss->isWritable()) {
        message = tr("Unable to create file %1").arg(filename);
        // Watch the file again
        m_FSWatcher->addPath(settings_path);

        // delete each entry if we created them above 
        if (clean_up_needed) {
	    foreach(IndexEditorModel::indexEntry* entry, entries) {
	        delete entry;
	    }
        }
	delete ss;
        return message;
    }

    // Remove the old values to account for deletions
    ss->remove(SETTINGS_GROUP);
    ss->beginWriteArray(SETTINGS_GROUP);
    int i = 0;
    foreach(IndexEditorModel::indexEntry * entry, entries) {
        if (entry->pattern.isEmpty() && entry->index_entry.isEmpty()) {
            continue;
        }

        foreach(QString pattern, entry->pattern.split("\n")) {
            ss->setArrayIndex(i++);
            ss->setValue(ENTRY_PATTERN, pattern);
            ss->setValue(ENTRY_INDEX_ENTRY, entry->index_entry);
        }
    }

    // delete each entry if we created them above 
    if (clean_up_needed) {
	foreach(IndexEditorModel::indexEntry* entry, entries) {
	    delete entry;
	}
    }
    ss->endArray();
    // Make sure file is created/updated so it can be checked
    ss->sync();
    delete ss;

    // Watch the file again
    m_FSWatcher->addPath(settings_path);
    SetDataModified(false);
    return message;
}
