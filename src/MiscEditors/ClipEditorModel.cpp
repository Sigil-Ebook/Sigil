/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 Grant Drake
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
#include <QtCore/QStandardPaths>
#include <QRegularExpression>

#include "MiscEditors/ClipEditorModel.h"
#include "sigil_constants.h"

static const QString SETTINGS_FILE          = "sigil_clips.ini";
static const QString SETTINGS_GROUP         = "clip_entries";
static const QString ENTRY_NAME             = "Name";
static const QString ENTRY_TEXT             = "Text";

const int COLUMNS = 2;

static const int IS_GROUP_ROLE = Qt::UserRole + 1;
static const int FULLNAME_ROLE = Qt::UserRole + 2;

static const QString CLIP_EXAMPLES_FILE = "clip_entries.ini";

ClipEditorModel *ClipEditorModel::m_instance = 0;

ClipEditorModel *ClipEditorModel::instance()
{
    if (m_instance == 0) {
        m_instance = new ClipEditorModel();
    }

    return m_instance;
}

ClipEditorModel::ClipEditorModel(QObject *parent)
    : QStandardItemModel(parent),
      m_FSWatcher(new QFileSystemWatcher()),
      m_IsDataModified(false)
{
    m_SettingsPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + SETTINGS_FILE;
    QStringList header;
    header.append(tr("Name"));
    header.append(tr("Text"));
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

ClipEditorModel::~ClipEditorModel()
{
    delete m_FSWatcher;
    m_FSWatcher = 0;

    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
}

void ClipEditorModel::SetDataModified(bool modified)
{
    m_IsDataModified = modified;
    ClipsUpdated();
}

bool ClipEditorModel::IsDataModified()
{
    return m_IsDataModified;
}

Qt::DropActions ClipEditorModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool ClipEditorModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    QModelIndex new_parent(parent);

    if (parent.column() > 0) {
        // User is trying to drop onto a child column - treat it as though they are dropping in the initial column.
        new_parent = index(parent.row(), 0, parent.parent());
    }

    if (column > 0) {
        // Same as above but for when user drops into the between row selector in a child column.
        column = 0;
    }

    // If dropped on an non-group entry convert to parent item group/row
    if (new_parent.isValid() && !itemFromIndex(new_parent)->data(IS_GROUP_ROLE).toBool()) {
        row = new_parent.row() + 1;
        new_parent = new_parent.parent();
    }

    if (!QStandardItemModel::dropMimeData(data, action, row, column, new_parent)) {
        return false;
    }

    // As our drag/drop completed successfully, recalculate the fullname for all the items in the model
    UpdateFullName(invisibleRootItem());

    // If we dropped onto a parent group, make sure it is expanded.
    if (new_parent.isValid()) {
        emit ItemDropped(new_parent);
    }

    SetDataModified(true);
    return true;
}

void ClipEditorModel::RowsRemovedHandler(const QModelIndex &parent, int start, int end)
{
    SetDataModified(true);
}

void ClipEditorModel::ItemChangedHandler(QStandardItem *item)
{
    Q_ASSERT(item);

    if (item->column() != 0) {
        SetDataModified(true);
        return;
    }

    // Restore name if nothing entered or contains a group indicator
    if (item->text().isEmpty() || item->text().contains("/")) {
        QString name = item->data(FULLNAME_ROLE).toString();
        name.replace(QRegularExpression("/$"), "");

        if (name.contains("/")) {
            name = name.split("/").last();
        }

        // Disconnect change signal while changing the items
        disconnect(this, SIGNAL(itemChanged(QStandardItem *)),
                   this, SLOT(ItemChangedHandler(QStandardItem *)));
        item->setText(name);
        connect(this, SIGNAL(itemChanged(QStandardItem *)),
                this, SLOT(ItemChangedHandler(QStandardItem *)));
        return;
    }

    Rename(item);
}

void ClipEditorModel::Rename(QStandardItem *item, const QString &name)
{
    // Update the name and all its children
    // Disconnect change signal while changing the items
    disconnect(this, SIGNAL(itemChanged(QStandardItem *)),
               this, SLOT(ItemChangedHandler(QStandardItem *)));

    // If item name not already changed, set it
    if (name != "") {
        item->setText(name);
    }

    UpdateFullName(item);
    connect(this, SIGNAL(itemChanged(QStandardItem *)),
            this, SLOT(ItemChangedHandler(QStandardItem *)));
    SetDataModified(true);
}

void ClipEditorModel::UpdateFullName(QStandardItem *item)
{
    QStandardItem *parent_item = item->parent();

    if (!parent_item) {
        parent_item = invisibleRootItem();
    }

    QString fullname = parent_item->data(FULLNAME_ROLE).toString() + item->text();
    QString tooltip;

    tooltip = fullname;
    if (item->data(IS_GROUP_ROLE).toBool()) {
        fullname.append("/");
    } else {
        QStandardItem *text_item = parent_item->child(item->row(), 1);
        if (text_item) {
            tooltip += "\n\n" % text_item->text();
            tooltip.replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;");
        }
    }

    item->setToolTip(tooltip);
    item->setData(fullname, FULLNAME_ROLE);

    if (item->hasChildren()) {
        for (int row = 0; row < item->rowCount(); row++) {
            UpdateFullName(item->child(row, 0));
        }
    }
}

void ClipEditorModel::SettingsFileChanged(const QString &path) const
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

bool ClipEditorModel::ItemIsGroup(QStandardItem *item)
{
    return item->data(IS_GROUP_ROLE).toBool();
}

QString ClipEditorModel::GetFullName(QStandardItem *item)
{
    return item->data(FULLNAME_ROLE).toString();
}

ClipEditorModel::clipEntry *ClipEditorModel::GetEntryFromNumber(int clip_number)
{
    return GetEntry(GetItemFromNumber(clip_number));
}

QStandardItem *ClipEditorModel::GetItemFromNumber(int clip_number)
{
    QStandardItem *item = NULL;

    clip_number--;

    if (clip_number < 0) {
        return item;
    }

    // Get the entry for the number, skipping over group entries so that
    // the entries can be at top or bottom, or anywhere, in the list.
    int row = 0;
    int rows = invisibleRootItem()->rowCount();
    for (int i = 0; i <= clip_number; i++) {
        while (row < rows) {
            item = invisibleRootItem()->child(row, 0);
            row++;
            if (!item || !item->data(IS_GROUP_ROLE).toBool()) {
                break;
            }
        }
        if (row >= rows) {
            break;
        }
        if (i == clip_number) {
            return item;
        }
    }

    return NULL;
}


ClipEditorModel::clipEntry *ClipEditorModel::GetEntryFromName(const QString &name, QStandardItem *item)
{
    return GetEntry(GetItemFromName(name, item));
}

QStandardItem *ClipEditorModel::GetItemFromName(QString name, QStandardItem *item)
{
    QStandardItem *found_item = NULL;

    if (!item) {
        item = invisibleRootItem();
    }

    if (item != invisibleRootItem() && item->data(FULLNAME_ROLE).toString() == name) {
        return item;
    }

    for (int row = 0; row < item->rowCount(); row++) {
        found_item = GetItemFromName(name, item->child(row, 0));

        // Return with first found entry
        if (found_item) {
            return found_item;
        }
    }

    return found_item;
}

void ClipEditorModel::LoadInitialData()
{
    removeRows(0, rowCount());
    LoadData();

    if (invisibleRootItem()->rowCount() == 0) {
        AddExampleEntries();
    }

    SetDataModified(false);
}

void ClipEditorModel::LoadData(const QString &filename, QStandardItem *item)
{
    SettingsStore *settings;

    if (filename.isEmpty()) {
        settings = new SettingsStore(m_SettingsPath);
    } else {
        settings = new SettingsStore(filename);
    }

    int size = settings->beginReadArray(SETTINGS_GROUP);

    // Add one entry at a time to the list
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        ClipEditorModel::clipEntry *entry = new ClipEditorModel::clipEntry();
        QString fullname = settings->value(ENTRY_NAME).toString();
        fullname.replace(QRegularExpression("\\s*/+\\s*"), "/");
        fullname.replace(QRegularExpression("^/"), "");
        entry->is_group = fullname.endsWith("/");
        // Name is set to fullname only while looping through parent groups when adding
        entry->name = fullname;
        entry->fullname = fullname;
        entry->text = settings->value(ENTRY_TEXT).toString();
        AddFullNameEntry(entry, item);
    }

    settings->endArray();
}

void ClipEditorModel::AddFullNameEntry(ClipEditorModel::clipEntry *entry, QStandardItem *parent_item, int row)
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

        foreach(QString group_name, group_names) {
            bool found = false;

            for (int r = 0; r < parent_item->rowCount(); r++) {
                if (parent_item->child(r, 0)->data(IS_GROUP_ROLE).toBool() && parent_item->child(r, 0)->text() == group_name) {
                    parent_item = parent_item->child(r, 0);
                    found = true;
                    break;
                }
            }

            if (!found) {
                ClipEditorModel::clipEntry *new_entry = new ClipEditorModel::clipEntry();
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

QStandardItem *ClipEditorModel::AddEntryToModel(ClipEditorModel::clipEntry *entry, bool is_group, QStandardItem *parent_item, int row)
{
    // parent_item must be a group item
    if (!parent_item) {
        parent_item = invisibleRootItem();
    }

    // Create an empty entry if none supplied
    if (!entry) {
        entry = new ClipEditorModel::clipEntry();
        entry->is_group = is_group;

        if (!is_group) {
            entry->name = "Text";
            entry->text = "";
        } else {
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
        QFont font;
        font.setWeight(QFont::Bold);
        group_item->setFont(font);
        rowItems << group_item;

        for (int col = 1; col < COLUMNS ; col++) {
            QStandardItem *item = new QStandardItem("");
            item->setEditable(false);
            rowItems << item;
        }
    } else {
        rowItems << new QStandardItem(entry->name);
        rowItems << new QStandardItem(entry->text);
    }

    rowItems[0]->setData(entry->is_group, IS_GROUP_ROLE);
    rowItems[0]->setData(entry->fullname, FULLNAME_ROLE);
    QString tooltip;
    if (entry->is_group) {
        tooltip = entry->fullname;
    } else {
        tooltip = entry->fullname % "\n\n" % entry->text;
    }
    rowItems[0]->setToolTip(tooltip);
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
    return new_item;
}

void ClipEditorModel::AddExampleEntries()
{
    QString examples_dir;
#ifdef Q_OS_MAC
    examples_dir = QCoreApplication::applicationDirPath() + "/../examples/";
#elif defined(Q_OS_WIN32)
    examples_dir = QCoreApplication::applicationDirPath() + "/examples/";
#else
    // all flavours of linux / unix
    // user supplied environment variable to 'share/sigil' directory will override everything
    if (!sigil_extra_root.isEmpty()) {
        examples_dir = sigil_extra_root + "/examples/";
    } else {
        examples_dir = sigil_share_root + "/examples/";
    }
#endif
    LoadData(examples_dir % CLIP_EXAMPLES_FILE);
}

QList<QStandardItem *> ClipEditorModel::GetNonGroupItems(QList<QStandardItem *> items)
{
    QList<QStandardItem *> all_items;
    foreach(QStandardItem * item, items) {
        all_items.append(GetNonGroupItems(item));
    }
    return all_items;
}

QList<QStandardItem *> ClipEditorModel::GetNonGroupItems(QStandardItem *item)
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

QList<QStandardItem *> ClipEditorModel::GetNonParentItems(QList<QStandardItem *> items)
{
    QList<QStandardItem *> all_items;
    foreach(QStandardItem * item, items) {
        all_items.append(GetNonParentItems(item));
    }
    return all_items;
}

QList<QStandardItem *> ClipEditorModel::GetNonParentItems(QStandardItem *item)
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

QList<ClipEditorModel::clipEntry *> ClipEditorModel::GetEntries(QList<QStandardItem *> items)
{
    QList<ClipEditorModel::clipEntry *> entries;
    foreach(QStandardItem * item, items) {
        entries.append(GetEntry(item));
    }
    return entries;
}

ClipEditorModel::clipEntry *ClipEditorModel::GetEntry(QStandardItem *item)
{
    QStandardItem *parent_item;

    if (item == NULL) {
        return NULL;
    }

    if (item->parent()) {
        parent_item = item->parent();
    } else {
        parent_item = invisibleRootItem();
    }

    ClipEditorModel::clipEntry *entry = new ClipEditorModel::clipEntry();
    entry->is_group =    parent_item->child(item->row(), 0)->data(IS_GROUP_ROLE).toBool();
    entry->fullname =    parent_item->child(item->row(), 0)->data(FULLNAME_ROLE).toString();
    entry->name =        parent_item->child(item->row(), 0)->text();
    if (!entry->is_group) {
        QStandardItem *text_item = parent_item->child(item->row(), 1);
        if (text_item) {
            entry->text = text_item->text();
        } else {
            entry->text = "";
        }
    }
    return entry;
}

ClipEditorModel::clipEntry *ClipEditorModel::GetEntry(const QModelIndex &index)
{
    QStandardItem *item = itemFromIndex(index);

    if (!item) {
        return NULL;
    }

    return GetEntry(item);
}


QStandardItem *ClipEditorModel::GetItemFromId(quintptr id, int row, QStandardItem *item) const
{
    QStandardItem *found_item = NULL;

    if (!item) {
        item = invisibleRootItem();
    }

    if (item->index().internalId() == id) {
        if (item->parent()) {
            item = item->parent();
        } else {
            item = invisibleRootItem();
        }

        return item->child(row, 0);
    }

    for (int r = 0; r < item->rowCount(); r++) {
        found_item = GetItemFromId((quintptr)id, row, item->child(r, 0));

        // Return with first found entry
        if (found_item) {
            return found_item;
        }
    }

    return found_item;
}

QString ClipEditorModel::SaveData(QList<ClipEditorModel::clipEntry *> entries, const QString &filename)
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
    if (m_FSWatcher->files().contains(m_SettingsPath)) {
        m_FSWatcher->removePath(m_SettingsPath);
    }

    // Open the default file for save, or specific file for export
    SettingsStore *settings;

    if (filename.isEmpty()) {
        settings = new SettingsStore(m_SettingsPath);
    } else {
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
    foreach(ClipEditorModel::clipEntry * entry, entries) {
        settings->setArrayIndex(i++);
        settings->setValue(ENTRY_NAME, entry->fullname);

        if (!entry->is_group) {
            settings->setValue(ENTRY_TEXT, entry->text);
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

QVariant ClipEditorModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.column() == 1 && role == Qt::SizeHintRole) {
        // Make all rows the same height using the name column to ensure text limited to a single line
        return data(this->index(0, 0), role).toSize();
    }

    return QStandardItemModel::data(index, role);
}
