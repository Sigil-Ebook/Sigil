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

#include "MiscEditors/SearchEditorModel.h"

static const QString SETTINGS_FILE          = "sigil_searches.ini";
static const QString SETTINGS_GROUP         = "search_entries";
static const QString ENTRY_NAME             = "Name";
static const QString ENTRY_FIND             = "Find";
static const QString ENTRY_REPLACE          = "Replace";

const int COLUMNS = 3;

static const int IS_GROUP_ROLE = Qt::UserRole + 1;
static const int FULLNAME_ROLE = Qt::UserRole + 2;

static const QString SEARCH_EXAMPLES_FILE = "search_entries.ini";

SearchEditorModel *SearchEditorModel::m_instance = 0;

SearchEditorModel *SearchEditorModel::instance()
{
    if (m_instance == 0) {
        m_instance = new SearchEditorModel();
    }

    return m_instance;
}

SearchEditorModel::SearchEditorModel(QObject *parent)
 : QStandardItemModel(parent),
   m_FSWatcher( new QFileSystemWatcher() ),
   m_IsDataModified(false)
{
    m_SettingsPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/" + SETTINGS_FILE;

    QStringList header;
    header.append(tr("Name"));
    header.append(tr("Find"));
    header.append(tr("Replace"));
    setHorizontalHeaderLabels(header);

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

    connect( this, SIGNAL( rowsRemoved(        const QModelIndex&, int, int ) ),
             this, SLOT(   RowsRemovedHandler( const QModelIndex&, int, int ) ) );
}

SearchEditorModel::~SearchEditorModel()
{
    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
}

void SearchEditorModel::SetDataModified(bool modified)
{
    m_IsDataModified = modified;
}

bool SearchEditorModel::IsDataModified()
{
    return m_IsDataModified;
}

Qt::DropActions SearchEditorModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData* SearchEditorModel::mimeData(const QModelIndexList &indexes) const 
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    // mimeData index is not the index of the actual item
    // index is the index of the first entry in the group
    // index.row() is the row of our actual index in the group
    // So you need index.parent()->child(index.row(),0) to get the actual index!
    foreach (QModelIndex index, indexes) {
        if (index.isValid() && index.column() == 0) {
            stream << index.internalId() << index.row();
        }
    }

    mimeData->setData("x-index", encodedData);
    return mimeData;
}

bool SearchEditorModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    if (action == Qt::IgnoreAction) {
        return true;
    }

    if (!data->hasFormat("x-index")) {
        return false;
    }

    // If dropped on an non-group entry convert to parent item group/row
    QModelIndex new_parent = parent;
    if (parent.isValid() && !itemFromIndex(parent)->data(IS_GROUP_ROLE).toBool()) {
        row = parent.row() + 1;
        new_parent = parent.parent();
    }

    QByteArray encodedData = data->data("x-index");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    QList<QStandardItem *> items;

    while (!stream.atEnd()) {
        qint64 id;
        int row;

        stream >> id >> row;
        QStandardItem* item = GetItemFromId(id, row);
        items.append(item);
    }

    // Abort if any items appear more than once in the selection
    QList<QStandardItem*> all_items;
    foreach (QStandardItem* item, items) {
        all_items.append(GetNonParentItems(item));
    }
    if (all_items.count() != all_items.toSet().count()) {
        return false;
    }

    foreach (QStandardItem* item, items) {
        QStandardItem *parent_item = NULL;
        if (new_parent.isValid()) {
            parent_item = itemFromIndex(new_parent);
        }

        // Get the parent path of the item so it can be moved
        QString parent_path = "";
        if (item->parent()) {
            parent_path = item->parent()->data(FULLNAME_ROLE).toString();
        }

        // Move all subitems of an item not just the item itself
        QList<QStandardItem*> sub_items = GetNonParentItems(item);

        if (item->data(IS_GROUP_ROLE).toBool()) {
            SearchEditorModel::searchEntry *top_group_entry = GetEntry(item);
            parent_item = AddEntryToModel(top_group_entry, top_group_entry->is_group, parent_item, row);
            parent_path = item->data(FULLNAME_ROLE).toString();
        }

        foreach (QStandardItem* item, sub_items) {
            SearchEditorModel::searchEntry *entry = GetEntry(item);

            // Remove the top level paths
            entry->fullname.replace(QRegExp(parent_path), "");
            entry->name = entry->fullname;

            AddFullNameEntry(entry, parent_item, row);
            if (row >= 0) {
                row++;
            }
        }
    }

    SetDataModified(true);
    return true;
}

void SearchEditorModel::RowsRemovedHandler( const QModelIndex & parent, int start, int end )
{
    SetDataModified(true);
}

void SearchEditorModel::ItemChangedHandler(QStandardItem *item)
{
    Q_ASSERT(item);

    if (item->column() != 0) {
        SetDataModified(true);
        return;
    }

    // Restore name if nothing entered or contains a group indicator
    if (item->text().isEmpty() || item->text().contains("/")) {
        QString name = item->data(FULLNAME_ROLE).toString();
        name.replace(QRegExp("/$"), "");
        if (name.contains("/")) {
            name = name.split("/").last();
        }
        item->setText(name);
        return;
    }

    Rename(item);
}

void SearchEditorModel::Rename(QStandardItem *item, const QString &name)
{
    // Update the name and all its children
    // Disconnect change signal while changing the items
    disconnect(this, SIGNAL(itemChanged(       QStandardItem*)),
               this, SLOT(  ItemChangedHandler(QStandardItem*)));

    // If item name not already changed, set it
    if (name != "") {
        disconnect(this, SIGNAL(itemChanged(       QStandardItem*)),
                   this, SLOT(  ItemChangedHandler(QStandardItem*)));

        item->setText(name);

        connect(this, SIGNAL(itemChanged(       QStandardItem*)),
                this, SLOT(  ItemChangedHandler(QStandardItem*)));
    }

    UpdateFullName(item);

    connect(this, SIGNAL(itemChanged(       QStandardItem*)),
            this, SLOT(  ItemChangedHandler(QStandardItem*)));

    SetDataModified(true);
}

void SearchEditorModel::UpdateFullName(QStandardItem *item)
{
    QStandardItem *parent_item = item->parent();
    if (!parent_item) {
        parent_item = invisibleRootItem();
    }

    QString fullname = parent_item->data(FULLNAME_ROLE).toString() + item->text();
    if (item->data(IS_GROUP_ROLE).toBool()) {
        fullname.append("/");
    }

    item->setToolTip(fullname);
    item->setData(fullname, FULLNAME_ROLE);

    for (int row = 0; row < item->rowCount(); row++) {
        UpdateFullName(item->child(row,0));
    }
}

void SearchEditorModel::SettingsFileChanged( const QString &path ) const
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
        emit SettingsFileUpdated();
    }
}

bool SearchEditorModel::ItemIsGroup(QStandardItem* item)
{
    return item->data(IS_GROUP_ROLE).toBool();
}

QString SearchEditorModel::GetFullName(QStandardItem* item)
{
    return item->data(FULLNAME_ROLE).toString();
}

SearchEditorModel::searchEntry* SearchEditorModel::GetEntryFromName(const QString &name, QStandardItem *item)
{
    return GetEntry(GetItemFromName(name, item));
}

QStandardItem* SearchEditorModel::GetItemFromName(const QString &name, QStandardItem *item)
{
    QStandardItem* found_item = NULL;

    if (!item) {
        item = invisibleRootItem();
    }

    if (item != invisibleRootItem() && item->data(FULLNAME_ROLE).toString() == name) {
        return item;
    }

    for (int row = 0; row < item->rowCount(); row++) {
        found_item = GetItemFromName(name, item->child(row,0));

        // Return with first found entry
        if (found_item) {
            return found_item;
        }
    }

    return found_item;
}

void SearchEditorModel::LoadInitialData()
{
    removeRows(0, rowCount());

    LoadData();

    if (invisibleRootItem()->rowCount() == 0) {
        AddExampleEntries();
    }

    SetDataModified(false);
}

void SearchEditorModel::LoadData(const QString &filename, QStandardItem *item)
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

        SearchEditorModel::searchEntry *entry = new SearchEditorModel::searchEntry();

        QString fullname = settings->value(ENTRY_NAME).toString();
        fullname.replace(QRegExp("\\s*/+\\s*"), "/");
        fullname.replace(QRegExp("^/"), "");

        entry->is_group = fullname.endsWith("/");

        // Name is set to fullname only while looping through parent groups when adding
        entry->name = fullname;
        entry->fullname = fullname;
        entry->find = settings->value(ENTRY_FIND).toString();
        entry->replace = settings->value(ENTRY_REPLACE).toString();

        AddFullNameEntry(entry, item);
    }

    settings->endArray();
}

void SearchEditorModel::AddFullNameEntry(SearchEditorModel::searchEntry *entry, QStandardItem *parent_item, int row)
{
    if (!parent_item) {
        parent_item = invisibleRootItem();
    }

    if (parent_item != invisibleRootItem() && !parent_item->data(IS_GROUP_ROLE).toBool()) {
        row = parent_item->row() + 1;
        parent_item = parent_item->parent();
    }

    if (row < 0) {
        row = parent_item->rowCount();
    }

    QString entry_name = entry->name;

    if (entry->name.contains("/")) {
        QStringList group_names = entry->name.split("/", QString::SkipEmptyParts);
        entry_name = group_names.last();
        if (!entry->is_group) {
            group_names.removeLast();
        }

        foreach (QString group_name, group_names) {
            bool found = false;
            for (int r = 0; r < parent_item->rowCount(); r++) {
                if (parent_item->child(r, 0)->data(IS_GROUP_ROLE).toBool() && parent_item->child(r, 0)->text() == group_name) {
                    parent_item = parent_item->child(r, 0);
                    found = true;
                    break;
                }
            }
            if (!found) {
                SearchEditorModel::searchEntry *new_entry = new SearchEditorModel::searchEntry();
                new_entry->is_group = true;
                new_entry->name = group_name;
                parent_item = AddEntryToModel(new_entry, new_entry->is_group, parent_item, parent_item->rowCount());
            }
        }
        row = parent_item->rowCount();
    }

    if (!entry->is_group) {
        entry->name = entry_name;
        AddEntryToModel(entry, entry->is_group, parent_item, row);
    }
}

QStandardItem *SearchEditorModel::AddEntryToModel(SearchEditorModel::searchEntry *entry, bool is_group, QStandardItem *parent_item, int row)
{
    // parent_item must be a group item

    if (!parent_item) {
        parent_item = invisibleRootItem();
    }

    // Create an empty entry if none supplied
    if (!entry) {
        entry = new SearchEditorModel::searchEntry();
        entry->is_group = is_group;
        if (!is_group) {
            entry->name = "Search";
            entry->find = "";
            entry->replace = "";
        }
        else {
            entry->name = "Group";
        }
    }

    entry->fullname = entry->name;
    if (parent_item != invisibleRootItem()) {
        // Set the fullname based on the parent entry
        entry->fullname = parent_item->data(FULLNAME_ROLE).toString() + entry->name;
    }

    if (entry->is_group) {
        entry->fullname += "/";
    }

    QList<QStandardItem *> rowItems;
    QStandardItem *group_item = parent_item;

    if (entry->is_group) {
        group_item = new QStandardItem(entry->name);
        QFont font = *new QFont();
        font.setWeight(QFont::Bold);
        group_item->setFont(font);

        rowItems << group_item;
        for (int col = 1; col < COLUMNS ; col++) {
            QStandardItem *item = new QStandardItem("");
            item->setEditable(false);
            rowItems << item;
        }
    }
    else {
        rowItems << new QStandardItem(entry->name);
        rowItems << new QStandardItem(entry->find);
        rowItems << new QStandardItem(entry->replace);
    }

    rowItems[0]->setData(entry->is_group, IS_GROUP_ROLE);
    rowItems[0]->setData(entry->fullname, FULLNAME_ROLE);
    rowItems[0]->setToolTip(entry->fullname);

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

void SearchEditorModel::AddExampleEntries()
{
    QString examples_dir;

#ifdef Q_WS_MAC
    examples_dir = QCoreApplication::applicationDirPath() + "/../examples/";
#endif
#ifdef Q_WS_WIN 
    examples_dir = QCoreApplication::applicationDirPath() + "/examples/";
#endif
#ifdef Q_WS_X11
    examples_dir = QCoreApplication::applicationDirPath() + "/../share/" + QCoreApplication::applicationName().toLower() + "/examples/";
#endif

    LoadData(examples_dir % SEARCH_EXAMPLES_FILE);
}

QList<QStandardItem *> SearchEditorModel::GetNonGroupItems(QList<QStandardItem *> items)
{
    QList<QStandardItem *> all_items;

    foreach (QStandardItem *item, items) {
        all_items.append(GetNonGroupItems(item));
    }

    return all_items;
}

QList<QStandardItem *> SearchEditorModel::GetNonGroupItems(QStandardItem* item)
{
    QList<QStandardItem *> items;

    if (!item->data(IS_GROUP_ROLE).toBool()) {
        items.append(item);
    }
    for (int row = 0; row < item->rowCount(); row++) {
        items.append(GetNonGroupItems(item->child(row, 0)));
    }

    return items;
}

QList<QStandardItem *> SearchEditorModel::GetNonParentItems(QList<QStandardItem *> items)
{
    QList<QStandardItem *> all_items;

    foreach (QStandardItem *item, items) {
        all_items.append(GetNonParentItems(item));
    }

    return all_items;
}

QList<QStandardItem *> SearchEditorModel::GetNonParentItems(QStandardItem* item)
{
    QList<QStandardItem *> items;

    if (item->rowCount() == 0) {
        if (item != invisibleRootItem()) {
            items.append(item);
        }
    }

    for (int row = 0; row < item->rowCount(); row++) {
        items.append(GetNonParentItems(item->child(row, 0)));
    }

    return items;
}

QList<SearchEditorModel::searchEntry *> SearchEditorModel::GetEntries(QList<QStandardItem*> items)
{
    QList<SearchEditorModel::searchEntry *> entries;

    foreach (QStandardItem *item, items) {
        entries.append(GetEntry(item));
    }

    return entries;
}

SearchEditorModel::searchEntry* SearchEditorModel::GetEntry(QStandardItem *item)
{
    QStandardItem *parent_item;

    if (item->parent()) {
        parent_item = item->parent();
    }
    else {
        parent_item = invisibleRootItem();
    }

    SearchEditorModel::searchEntry *entry = new SearchEditorModel::searchEntry();

    entry->is_group =    parent_item->child(item->row(), 0)->data(IS_GROUP_ROLE).toBool();
    entry->fullname =    parent_item->child(item->row(), 0)->data(FULLNAME_ROLE).toString();
    entry->name =        parent_item->child(item->row(), 0)->text();
    entry->find =        parent_item->child(item->row(), 1)->text();
    entry->replace =     parent_item->child(item->row(), 2)->text();

    return entry;
}

QStandardItem* SearchEditorModel::GetItemFromId(qint64 id, int row, QStandardItem *item) const
{
    QStandardItem* found_item = NULL;

    if (!item) {
        item = invisibleRootItem();
    }

    if (item->index().internalId() == id) {
        if (item->parent()) {
            item = item->parent();
        }
        else {
            item = invisibleRootItem();
        }
        return item->child(row, 0);
    }

    for (int r = 0; r < item->rowCount(); r++) {
        found_item = GetItemFromId(id, row, item->child(r,0));

        // Return with first found entry
        if (found_item) {
            return found_item;
        }
    }

    return found_item;
}

QString SearchEditorModel::SaveData(QList<SearchEditorModel::searchEntry*> entries, const QString &filename)
{
    QString message = "";

    // Save everything if no entries selected
    if (entries.isEmpty()) {
        QList<QStandardItem *> items = GetNonParentItems(invisibleRootItem());
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
    foreach (SearchEditorModel::searchEntry *entry, entries) {
        settings->setArrayIndex(i++);
        settings->setValue(ENTRY_NAME, entry->fullname);

        if (!entry->is_group) {
            settings->setValue(ENTRY_FIND, entry->find);
            settings->setValue(ENTRY_REPLACE, entry->replace);
        }
    }

    settings->endArray();

    // Make sure file is created/updated so it can be checked
    settings->sync();

    // Watch the file again
    m_FSWatcher->addPath(m_SettingsPath);

    SetDataModified(false);
    return message;
}

QVariant SearchEditorModel::data( const QModelIndex& index, int role ) const
{
    if (index.isValid() && index.column() > 0 && role == Qt::SizeHintRole ) {
        // Make all rows the same height using the name column to ensure text limited to a single line
        return data(this->index(0,0), role).toSize();
    }
    return QStandardItemModel::data(index, role);
}
