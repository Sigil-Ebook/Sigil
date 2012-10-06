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
#include <QtCore/QTime>
#include <QtGui/QDesktopServices>

#include "MiscEditors/IndexEditorModel.h"

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
   m_FSWatcher( new QFileSystemWatcher() ),
   m_IsDataModified(false)
{
    m_SettingsPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + SETTINGS_FILE;

    LoadInitialData();

    // Save it to make sure we have a file in case it was loaded from examples
    SaveData();

    if (!m_FSWatcher->files().contains(m_SettingsPath) ) {
        m_FSWatcher->addPath(m_SettingsPath);
    }

    connect( m_FSWatcher, SIGNAL( fileChanged( const QString& ) ),
             this,        SLOT( SettingsFileChanged( const QString& )), Qt::DirectConnection );

    connect(this, SIGNAL(itemChanged(QStandardItem*)),
            this, SLOT(ItemChangedHandler(QStandardItem*)));
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

void IndexEditorModel::RowsRemovedHandler( const QModelIndex & parent, int start, int end )
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
    disconnect(this, SIGNAL(itemChanged(       QStandardItem*)),
               this, SLOT(  ItemChangedHandler(QStandardItem*)));

    IndexEditorModel::indexEntry* entry = GetEntry(item);

    QStringList names = entry->pattern.split("\n");
    QString first_name = names.first();
    names.removeFirst();

    // Update the first entry
    item->setText(first_name);

    foreach (QString name, names) {
        entry->pattern = name;
        QStandardItem *new_item = AddFullNameEntry(entry, item);
        item = new_item;
    }

    connect(this, SIGNAL(itemChanged(       QStandardItem*)),
            this, SLOT(  ItemChangedHandler(QStandardItem*)));
}

void IndexEditorModel::ClearData()
{
    clear();

    QStringList header;
    header.append(tr("Text to Include"));
    header.append(tr("Index Entries"));

    setHorizontalHeaderLabels(header);
}

void IndexEditorModel::SettingsFileChanged( const QString &path ) const
{
    // The file may have been deleted prior to writing a new version - give it a chance to write.
    QTime wake_time = QTime::currentTime().addMSecs(1000);
    while( !QFile::exists(path) && QTime::currentTime() < wake_time ) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    // The signal is also received after resource files are removed / renamed,
    // but it can be safely ignored because QFileSystemWatcher automatically stops watching them.
    if ( QFile::exists(path) )
    {
        // Some editors write the updated contents to a temporary file
        // and then atomically move it over the watched file.
        // In this case QFileSystemWatcher loses track of the file, so we have to add it again.
        if ( !m_FSWatcher->files().contains(path) ) {
            m_FSWatcher->addPath( path );
        }

        instance()->LoadInitialData();
    }
}

void IndexEditorModel::LoadInitialData(QString filename)
{
    ClearData();

    LoadData(filename);

    SetDataModified(false);
}

void IndexEditorModel::LoadData(QString filename, QStandardItem *item)
{
    SettingsStore *settings;
    if (filename.isEmpty()) {
        settings = new SettingsStore(m_SettingsPath);
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

QStandardItem* IndexEditorModel::AddFullNameEntry(IndexEditorModel::indexEntry *entry, QStandardItem *parent_item, int row)
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

QStandardItem* IndexEditorModel::AddEntryToModel(IndexEditorModel::indexEntry *entry, QStandardItem *parent_item, int row)
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

    SetDataModified(true);
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

    // Stop watching the file while we save it
    if (m_FSWatcher->files().contains(m_SettingsPath) ) {
        m_FSWatcher->removePath(m_SettingsPath);
    }

    // Open the default file for save, or specific file for export
    SettingsStore *settings;
    if (filename.isEmpty()) {
        settings = new SettingsStore(m_SettingsPath);
    }
    else {
        settings = new SettingsStore(filename);
    }

    settings->sync();
    if (!settings->isWritable()) {
        message = tr("Unable to create file %1").arg(filename);

        // Watch the file again
        m_FSWatcher->addPath(m_SettingsPath);

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
        foreach (QString pattern, entry->pattern.split("\n")) {
            settings->setArrayIndex(i++);
            settings->setValue(ENTRY_PATTERN, pattern);
            settings->setValue(ENTRY_INDEX_ENTRY, entry->index_entry);
        }
    }

    settings->endArray();

    // Make sure file is created/updated so it can be checked
    settings->sync();

    // Watch the file again
    m_FSWatcher->addPath(m_SettingsPath);

    SetDataModified(true);
    return message;
}
