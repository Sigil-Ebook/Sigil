/************************************************************************
**
**  Copyright (C) 2015-2022 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#include <QFileInfo>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>

#include "MiscEditors/SearchEditorModel.h"
#include "Misc/Utility.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #define QT_ENUM_SKIPEMPTYPARTS Qt::SkipEmptyParts
    #define QT_ENUM_KEEPEMPTYPARTS Qt::KeepEmptyParts
#else
    #define QT_ENUM_SKIPEMPTYPARTS QString::SkipEmptyParts
    #define QT_ENUM_KEEPEMPTYPARTS QString::KeepEmptyParts
#endif

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
static const QString SETTINGS_FILE = SEARCHES_V2_SETTINGS_FILE;
#else
static const QString SETTINGS_FILE = SEARCHES_V6_SETTINGS_FILE;
#endif

static const QString SETTINGS_GROUP         = "search_entries";
static const QString ENTRY_NAME             = "Name";
static const QString ENTRY_FIND             = "Find";
static const QString ENTRY_REPLACE          = "Replace";
static const QString ENTRY_CONTROLS         = "Controls";

const int COLUMNS = 4;

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
      m_FSWatcher(new QFileSystemWatcher()),
      m_IsDataModified(false)
{
    m_SettingsPath = Utility::DefinePrefsDir() + "/" + SETTINGS_FILE;
    QStringList header;
    header.append(tr("Name"));
    header.append(tr("Find"));
    header.append(tr("Replace"));
    header.append(tr("Controls"));
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

bool SearchEditorModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
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

void SearchEditorModel::RowsRemovedHandler(const QModelIndex &parent, int start, int end)
{
    SetDataModified(true);
}


void SearchEditorModel::ItemChangedHandler(QStandardItem *item)
{
    Q_ASSERT(item);

    if (item->column() == 3) {
        QString controls = item->text();
        item->setToolTip(BuildControlsToolTip(controls));
    }

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

        item->setText(name);
        return;
    }

    Rename(item);
}

void SearchEditorModel::Rename(QStandardItem *item, const QString &name)
{
    // Update the name and all its children
    // Disconnect change signal while changing the items
    disconnect(this, SIGNAL(itemChanged(QStandardItem *)),
               this, SLOT(ItemChangedHandler(QStandardItem *)));

    // If item name not already changed, set it
    if (name != "") {
        disconnect(this, SIGNAL(itemChanged(QStandardItem *)),
                   this, SLOT(ItemChangedHandler(QStandardItem *)));
        item->setText(name);
        connect(this, SIGNAL(itemChanged(QStandardItem *)),
                this, SLOT(ItemChangedHandler(QStandardItem *)));
    }

    UpdateFullName(item);
    connect(this, SIGNAL(itemChanged(QStandardItem *)),
            this, SLOT(ItemChangedHandler(QStandardItem *)));
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
        UpdateFullName(item->child(row, 0));
    }
}

void SearchEditorModel::SettingsFileChanged(const QString &path) const
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

bool SearchEditorModel::ItemIsGroup(QStandardItem *item)
{
    return item->data(IS_GROUP_ROLE).toBool();
}

QString SearchEditorModel::GetFullName(QStandardItem *item)
{
    return item->data(FULLNAME_ROLE).toString();
}

SearchEditorModel::searchEntry *SearchEditorModel::GetEntryFromName(const QString &name, QStandardItem *item)
{
    return GetEntry(GetItemFromName(name, item));
}

QStandardItem *SearchEditorModel::GetItemFromName(const QString &name, QStandardItem *item)
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
    QString settings_path = filename;
    if (settings_path.isEmpty()) settings_path = m_SettingsPath;

    SettingsStore ss(settings_path);

    int size = ss.beginReadArray(SETTINGS_GROUP);

    // Add one entry at a time to the list
    for (int i = 0; i < size; ++i) {
        ss.setArrayIndex(i);
        SearchEditorModel::searchEntry *entry = new SearchEditorModel::searchEntry();
        QString fullname = ss.value(ENTRY_NAME).toString();
        fullname.replace(QRegularExpression("\\s*/+\\s*"), "/");
        fullname.replace(QRegularExpression("^/"), "");
        entry->is_group = fullname.endsWith("/");
        // Name is set to fullname only while looping through parent groups when adding
        entry->name = fullname;
        entry->fullname = fullname;
        entry->find = ss.value(ENTRY_FIND).toString();
        entry->replace = ss.value(ENTRY_REPLACE).toString();
        entry->controls = ss.value(ENTRY_CONTROLS, "").toString();
        AddFullNameEntry(entry, item);
        // done with the temporary entry so remove it
        delete entry;
    }
    ss.endArray();
}


void SearchEditorModel::LoadTextData(const QString &filename, QStandardItem *item, const QChar &sep)
{
    QFileInfo fi(filename);
    QString groupname = fi.fileName() + "/";
    int cnt = 1;
    if (fi.exists()) {
        QString data = Utility::ReadUnicodeTextFile(filename);
        QStringList datalines = data.split('\n');
        foreach(QString aline, datalines) {
            if (!aline.isEmpty()) {
                QStringList findreplace;
                if (sep == ',') {
                    findreplace  = Utility::parseCSVLine(aline);
                } else { 
                    findreplace = aline.split(sep);
                }
                // add to end to prevent errors
                findreplace << "" << "" << "" << "";
                QString localname = "rep" + QStringLiteral("%1").arg(cnt, 5, 10, QLatin1Char('0'));
                QString fullname;
                SearchEditorModel::searchEntry *entry = new SearchEditorModel::searchEntry();
                // if no name info appears
                if (findreplace.at(0).isEmpty()) {
                    fullname = groupname + localname;
                } else {
                    // this was created by an Export so fullname is present
                    fullname = findreplace.at(0);
                }
                fullname.replace(QRegularExpression("\\s*/+\\s*"), "/");
                fullname.replace(QRegularExpression("^/"), "");
                entry->is_group = fullname.endsWith("/");
                // Name is set to fullname only while looping through parent groups when adding
                entry->name = fullname;
                entry->fullname = fullname;
                entry->find = findreplace.at(1);
                entry->replace = findreplace.at(2);
                entry->controls = findreplace.at(3);
                
                AddFullNameEntry(entry, item);
                // done with the temporary entry so remove it
                delete entry;
                cnt++;
            }
        }
    }
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
        QStringList group_names = entry->name.split("/", QT_ENUM_SKIPEMPTYPARTS);
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
                SearchEditorModel::searchEntry *new_entry = new SearchEditorModel::searchEntry();
                new_entry->is_group = true;
                new_entry->name = group_name;
                parent_item = AddEntryToModel(new_entry, new_entry->is_group, parent_item, parent_item->rowCount());
                delete new_entry;
            }
        }
        row = parent_item->rowCount();
    }

    if (!entry->is_group) {
        entry->name = entry_name;
        AddEntryToModel(entry, entry->is_group, parent_item, row);
    }
}


void SearchEditorModel::FillControls(const QList<QStandardItem*> &items)
{
    if (items.isEmpty()) return;
    
    QStandardItem *source_item = items.at(0);
    
    QStandardItem *parent_item = invisibleRootItem();
    if (source_item->parent()) {
        parent_item = source_item->parent();
    }
    QString controls = parent_item->child(source_item->row(), 3)->text();

    for (int i = 1; i < items.size(); i++) {
        QStandardItem* aitem = items.at(i);
        parent_item = invisibleRootItem();
        if (aitem->parent()) {
            parent_item = aitem->parent();
        }
        parent_item->child(aitem->row(), 3)->setText(controls);
    }
    SetDataModified(true);
}


QString SearchEditorModel::BuildControlsToolTip(const QString & controls)
{
    QString tooltip_controls = "";
    if (controls != "") {
        if (controls.contains("NL")) {
            tooltip_controls.append("NL - " + tr("Mode: Normal") + "\n");
        }
        if (controls.contains("RX")) {
            tooltip_controls.append("RX - " + tr("Mode: Regular Expression") + "\n");
        }
        if (controls.contains("CS")) {
            tooltip_controls.append("CS - " + tr("Mode: Case Sensitive") + "\n");
        }
        if (controls.contains("UP")) {
            tooltip_controls.append("UP - " + tr("Direction: Up") + "\n");
        }
        if (controls.contains("DN")) {
            tooltip_controls.append("DN - " + tr("Direction: Down") + "\n");
        }
        if (controls.contains("CF")) {
            tooltip_controls.append("CF - " + tr("Target: Current File") + "\n");
        }
        if (controls.contains("AH")) {
            tooltip_controls.append("AH - " + tr("Target: All HTML Files") + "\n");
        }
        if (controls.contains("SH")) {
            tooltip_controls.append("SH - " + tr("Target: Selected HTML Files") + "\n");
        }
        if (controls.contains("TH")) {
            tooltip_controls.append("TH - " + tr("Target: Tabbed HTML Files") + "\n");
        }
        if (controls.contains("AC")) {
            tooltip_controls.append("AC - " + tr("Target: All CSS Files") + "\n");
        }
        if (controls.contains("SC")) {
            tooltip_controls.append("SC - " + tr("Target: Selected CSS Files") + "\n");
        }
        if (controls.contains("TC")) {
            tooltip_controls.append("TC - " + tr("Target: Tabbed CSS Files") + "\n");
        }
        if (controls.contains("OP")) {
            tooltip_controls.append("OP - " + tr("Target: OPF File") + "\n");
        }
        if (controls.contains("NX")) {
            tooltip_controls.append("NX - " + tr("Target: NCX File") + "\n");
        }
        if (controls.contains("DA")) {
            tooltip_controls.append("DA - " + tr("Option: DotAll") + "\n");
        }
        if (controls.contains("MM")) {
            tooltip_controls.append("MM - " + tr("Option: Minimal Match") + "\n");
        }
        if (controls.contains("AT")) {
            tooltip_controls.append("AT - " + tr("Option: Auto Tokenise") + "\n");
        }
        if (controls.contains("WR")) {
            tooltip_controls.append("WR - " + tr("Option: Wrap") + "\n");
        }
    }
    return tooltip_controls;
}

QStandardItem *SearchEditorModel::AddEntryToModel(SearchEditorModel::searchEntry *entry, bool is_group, QStandardItem *parent_item, int row)
{
    bool clean_up_needed = false;
    // parent_item must be a group item
    if (!parent_item) {
        parent_item = invisibleRootItem();
    }

    // Create an empty entry if none supplied
    if (!entry) {
        entry = new SearchEditorModel::searchEntry();
        clean_up_needed = true;
        entry->is_group = is_group;

        if (!is_group) {
            entry->name = "Search";
            entry->find = "";
            entry->replace = "";
            entry->controls = "";
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
        rowItems << new QStandardItem(entry->find);
        rowItems << new QStandardItem(entry->replace);
        rowItems << new QStandardItem(entry->controls);
    }

    rowItems[0]->setData(entry->is_group, IS_GROUP_ROLE);
    rowItems[0]->setData(entry->fullname, FULLNAME_ROLE);
    rowItems[0]->setToolTip(entry->fullname);
    rowItems[3]->setToolTip(BuildControlsToolTip(entry->controls));
    
    // Add the new item to the model at the specified row
    QStandardItem *new_item;

    if (row < 0 || row >= parent_item->rowCount()) {
        parent_item->appendRow(rowItems);
        new_item = parent_item->child(parent_item->rowCount() - 1, 0);
    } else {
        parent_item->insertRow(row, rowItems);
        new_item = parent_item->child(row, 0);
    }

    if (clean_up_needed) delete entry;
    SetDataModified(true);
    return new_item;
}

void SearchEditorModel::AddExampleEntries()
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
    LoadData(examples_dir % SEARCH_EXAMPLES_FILE);
}

QList<QStandardItem *> SearchEditorModel::GetNonGroupItems(QList<QStandardItem *> items)
{
    QList<QStandardItem *> all_items;
    foreach(QStandardItem * item, items) {
        all_items.append(GetNonGroupItems(item));
    }
    return all_items;
}

QList<QStandardItem *> SearchEditorModel::GetNonGroupItems(QStandardItem *item)
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
    foreach(QStandardItem * item, items) {
        all_items.append(GetNonParentItems(item));
    }
    return all_items;
}

QList<QStandardItem *> SearchEditorModel::GetNonParentItems(QStandardItem *item)
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

QList<SearchEditorModel::searchEntry *> SearchEditorModel::GetEntries(QList<QStandardItem *> items)
{
    QList<SearchEditorModel::searchEntry *> entries;
    foreach(QStandardItem * item, items) {
        entries.append(GetEntry(item));
    }
    return entries;
}

SearchEditorModel::searchEntry *SearchEditorModel::GetEntry(QStandardItem *item)
{
    QStandardItem *parent_item;

    if (item->parent()) {
        parent_item = item->parent();
    } else {
        parent_item = invisibleRootItem();
    }

    SearchEditorModel::searchEntry *entry = new SearchEditorModel::searchEntry();
    entry->is_group =    parent_item->child(item->row(), 0)->data(IS_GROUP_ROLE).toBool();
    entry->fullname =    parent_item->child(item->row(), 0)->data(FULLNAME_ROLE).toString();
    entry->name =        parent_item->child(item->row(), 0)->text();
    entry->find =        parent_item->child(item->row(), 1)->text();
    entry->replace =     parent_item->child(item->row(), 2)->text();
    entry->controls =    parent_item->child(item->row(), 3)->text();
    return entry;
}

QStandardItem *SearchEditorModel::GetItemFromId(quintptr id, int row, QStandardItem *item) const
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


QString SearchEditorModel::SaveTextData(QList<SearchEditorModel::searchEntry *> entries,
                                        const QString &filename,
                                        const QChar &sep)
{
    QString message = "";
    bool clean_up_needed = false;
    QString save_path = filename;

    // Save everything if no entries selected
    if (entries.isEmpty()) {
        QList<QStandardItem *> items = GetNonParentItems(invisibleRootItem());

        if (!items.isEmpty()) {
            // GetEntries calls GetEntry which creates each entry with new
            entries = GetEntries(items);
            clean_up_needed = true;
        }
    }

    // Create Data to write to file
    QStringList res;
    foreach(SearchEditorModel::searchEntry * entry, entries) {
        QStringList data;
        data << entry->fullname;
        if (!entry->is_group) {
           data << entry->find;
           data << entry->replace;
           data << entry->controls;
        }
        if (sep == ',') {
            res << Utility::createCSVLine(data);
        } else {
            res << data.join(sep);
        }
    }
    QString text = res.join('\n');
    try {
        Utility::WriteUnicodeTextFile(text, save_path);
    } catch (CannotOpenFile& e) {
        message = QString(e.what());
    }

    // delete each entry if we created them above
    if (clean_up_needed) {
        foreach(SearchEditorModel::searchEntry* entry, entries) {
            delete entry;
        }
    }
    return message;
}


QString SearchEditorModel::SaveData(QList<SearchEditorModel::searchEntry *> entries, const QString &filename)
{
    QString message = "";
    bool clean_up_needed = false;
    QString settings_path = filename;
    if (settings_path.isEmpty()) settings_path = m_SettingsPath;

    // Save everything if no entries selected
    if (entries.isEmpty()) {
        QList<QStandardItem *> items = GetNonParentItems(invisibleRootItem());

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
    {
        SettingsStore ss(settings_path);

        // ss.sync();

        if (!ss.isWritable()) {
            message = tr("Unable to create file %1").arg(filename);
            // Watch the file again
            m_FSWatcher->addPath(settings_path);
            // delete each entry if we created them above   
            if (clean_up_needed) {
                foreach(SearchEditorModel::searchEntry* entry, entries) {
                    delete entry;
                }
            }
            return message;
        }

        // Remove the old values to account for deletions
        ss.remove(SETTINGS_GROUP);
        ss.beginWriteArray(SETTINGS_GROUP);
        int i = 0;
        foreach(SearchEditorModel::searchEntry * entry, entries) {
            ss.setArrayIndex(i++);
            ss.setValue(ENTRY_NAME, entry->fullname);

            if (!entry->is_group) {
                ss.setValue(ENTRY_FIND, entry->find);
                ss.setValue(ENTRY_REPLACE, entry->replace);
                ss.setValue(ENTRY_CONTROLS, entry->controls);
            }
        }
    
        // delete each entry if we created them above
        if (clean_up_needed) {
            foreach(SearchEditorModel::searchEntry* entry, entries) {
                delete entry;
            }
        }

        ss.endArray();

        // Make sure file is created/updated so it can be checked
        ss.sync();
    }

    // Watch the file again
    m_FSWatcher->addPath(settings_path);
    SetDataModified(false);
    return message;
}

QVariant SearchEditorModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.column() > 0 && role == Qt::SizeHintRole) {
        // Make all rows the same height using the name column to ensure text limited to a single line
        return data(this->index(0, 0), role).toSize();
    }

    return QStandardItemModel::data(index, role);
}
