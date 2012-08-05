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

#include <QtCore/QSignalMapper>
#include <QtGui/QFileDialog>
#include <QtGui/QContextMenuEvent>

#include "Dialogs/SearchEditor.h"
#include "Misc/Utility.h"

static const QString SETTINGS_GROUP = "search_editor";
static const QString FILE_EXTENSION = "ini";

SearchEditor::SearchEditor(QWidget *parent)
    : 
    QDialog(parent),
    m_LastFolderOpen(QString()),
    m_cbDelegate(new SearchEditorItemDelegate()),
    m_ContextMenu(new QMenu(this))
{
    ui.setupUi(this);	

    SetupSearchEditorTree();

    CreateContextMenuActions();

    ConnectSignalsSlots();
} 

SearchEditor::~SearchEditor()
{
    // Restore data from file since we aren't saving
    m_SearchEditorModel->LoadInitialData();

    if (m_cbDelegate) {
        delete m_cbDelegate;
        m_cbDelegate = 0;
    }
}

void SearchEditor::SetupSearchEditorTree()
{
    m_SearchEditorModel = SearchEditorModel::instance();

    ui.SearchEditorTree->setModel(m_SearchEditorModel);
    ui.SearchEditorTree->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.SearchEditorTree->setSortingEnabled(false);
    ui.SearchEditorTree->setWordWrap(true);
    ui.SearchEditorTree->setAlternatingRowColors(true); 


    ui.SearchEditorTree->header()->setToolTip(
        "<p>" + tr("Right click on an entry to see a context menu of actions.") + "</p>" +
        "<p>" + tr("You can also right click on the Find text box in the Find & Replace window to select an entry.") + "</p>" +
        "<dl>" +
        "<dt><b>" + tr("Name") + "</b><dd>" + tr("Name of your entry or group.") + "</dd>" +
        "<dt><b>" + tr("Description") + "</b><dd>" + tr("Optional.") + "</dd>" +
        "<dt><b>" + tr("Find") + "</b><dd>" + tr("The text to put into the Find box.") + "</dd>" +
        "<dt><b>" + tr("Replace") + "</b><dd>" + tr("The text to put into the Replace box.") + "</dd>" +
        "<dt><b>" + tr("Mode") + "</b><dd>" + tr("What to search.") + "</dd>" +
        "<dt><b>" + tr("Where") + "</b><dd>" + tr("Where to search.") + "</dd>" +
        "<dt><b>" + tr("Direction") + "</b><dd>" + tr("Direction to search.") + "</dd>" +
        "</dl>");

    ui.SearchEditorTree->header()->setStretchLastSection(false);
    for (int column = 0; column < ui.SearchEditorTree->header()->count(); column++) {
        ui.SearchEditorTree->resizeColumnToContents(column);
    }

    // The mode columns use a combobox for editing their values
    ui.SearchEditorTree->setItemDelegateForColumn(4, m_cbDelegate);
    ui.SearchEditorTree->setItemDelegateForColumn(5, m_cbDelegate);
    ui.SearchEditorTree->setItemDelegateForColumn(6, m_cbDelegate);
}

bool SearchEditor::SaveData(QList<SearchEditorModel::searchEntry*> entries, QString filename)
{
    QString message = m_SearchEditorModel->SaveData(entries, filename);

    if (!message.isEmpty()) {
        Utility::DisplayStdErrorDialog(tr("Cannot save entries.") + "\n\n" + message);
    }
    return message.isEmpty();
}

void SearchEditor::LoadFindReplace()
{
    emit LoadSelectedSearchRequest(GetSelectedEntry(false));
}

void SearchEditor::Find()
{
    emit FindSelectedSearchRequest(GetSelectedEntries());
}

void SearchEditor::Replace()
{
    emit ReplaceSelectedSearchRequest(GetSelectedEntries());
}

void SearchEditor::CountAll()
{
    emit CountAllSelectedSearchRequest(GetSelectedEntries());
}

void SearchEditor::ReplaceAll()
{
    emit ReplaceAllSelectedSearchRequest(GetSelectedEntries());
}

void SearchEditor::showEvent(QShowEvent *event)
{
    ReadSettings();

    ui.SearchEditorTree->expandAll();
    ui.Filter->setCurrentIndex(0);
    ui.FilterText->clear();
    ui.FilterText->setFocus();
}

int SearchEditor::SelectedRowsCount()
{
    int count = 0;
    if (ui.SearchEditorTree->selectionModel()->hasSelection()) {
        count = ui.SearchEditorTree->selectionModel()->selectedRows(0).count();
    }

    return count;
}

SearchEditorModel::searchEntry* SearchEditor::GetSelectedEntry(bool show_warning)
{
    SearchEditorModel::searchEntry *entry = NULL;

    if (ui.SearchEditorTree->selectionModel()->hasSelection()) {
        QStandardItem* item = NULL;
        QModelIndexList selected_indexes = ui.SearchEditorTree->selectionModel()->selectedRows(0);

        if (selected_indexes.count() == 1) {
            item = m_SearchEditorModel->itemFromIndex(selected_indexes.first());
        }
        else if (show_warning) {
            Utility::DisplayStdErrorDialog(tr("You cannot select more than one entry when using this action."));
            return entry;
        }

        if (item) {
            if (!m_SearchEditorModel->ItemIsGroup(item)) {
                entry = m_SearchEditorModel->GetEntry(item);
            }
            else if (show_warning) {
                Utility::DisplayStdErrorDialog(tr("You cannot select a group for this action."));
            }
        }
    }

    return entry;
}

QList<SearchEditorModel::searchEntry*> SearchEditor::GetSelectedEntries()
{
    QList<SearchEditorModel::searchEntry *> selected_entries;

    if (ui.SearchEditorTree->selectionModel()->hasSelection()) {

        QList<QStandardItem*> items = m_SearchEditorModel->GetNonGroupItems(GetSelectedItems());

        if (!ItemsAreUnique(items)) {
            return selected_entries;
        }

        selected_entries = m_SearchEditorModel->GetEntries(items);
    }

    return selected_entries;
}

QList<QStandardItem*> SearchEditor::GetSelectedItems()
{
    // Shift-click order is top to bottom regardless of starting position
    // Ctrl-click order is first clicked to last clicked (included shift-clicks stay ordered as is)

    QModelIndexList selected_indexes = ui.SearchEditorTree->selectionModel()->selectedRows(0);
    QList<QStandardItem*> selected_items;

    foreach (QModelIndex index, selected_indexes) {
        selected_items.append(m_SearchEditorModel->itemFromIndex(index));
    }

    return selected_items;
}

bool SearchEditor::ItemsAreUnique(QList<QStandardItem*> items)
{
    // Although saving a group and a sub item works, it could be confusing to users to
    // have and entry appear twice so its more predictable just to prevent it and warn the user
    if (items.toSet().count() != items.count()) {
        Utility::DisplayStdErrorDialog(tr("You cannot select an entry and a group containing the entry."));
        return false;
    }

    return true;
}

QStandardItem* SearchEditor::AddEntry(bool is_group, SearchEditorModel::searchEntry *search_entry, bool insert_after)
{
    QStandardItem *parent_item = NULL;
    QStandardItem *new_item = NULL;
    int row = 0;

    // If adding a new/blank entry add it after the selected entries.
    if (insert_after) {
        if (ui.SearchEditorTree->selectionModel()->hasSelection()) {
            parent_item = GetSelectedItems().last();
            if (!parent_item) {
                return parent_item;
            }
            if (!m_SearchEditorModel->ItemIsGroup(parent_item)) {
                    row = parent_item->row() + 1;
                    parent_item = parent_item->parent();
            }
        }
    }

    // Make sure the new entry can be seen
    if (parent_item) {
        ui.SearchEditorTree->expand(parent_item->index());
    }

    new_item = m_SearchEditorModel->AddEntryToModel(search_entry, is_group, parent_item, row);
    QModelIndex new_index = new_item->index();

    // Select the added item and set it for editing
    ui.SearchEditorTree->selectionModel()->clear();
    ui.SearchEditorTree->setCurrentIndex(new_index);
    ui.SearchEditorTree->selectionModel()->select(new_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    ui.SearchEditorTree->edit(new_index);
    ui.SearchEditorTree->setCurrentIndex(new_index);
    ui.SearchEditorTree->selectionModel()->select(new_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);

    return new_item;
}

QStandardItem* SearchEditor::AddGroup()
{
    return AddEntry(true);
}

void SearchEditor::Cut()
{
    if (Copy()) {
        Delete();
    }
}

bool SearchEditor::Copy()
{
    if (SelectedRowsCount() < 1) {
        return false;
    }

    while (m_SavedSearchEntries.count()) {
        m_SavedSearchEntries.removeAt(0);
    }

    QList<SearchEditorModel::searchEntry *> entries = GetSelectedEntries();
    if (!entries.count()) {
        return false;
    }

    foreach (QStandardItem *item, GetSelectedItems()) {
        SearchEditorModel::searchEntry *entry = m_SearchEditorModel->GetEntry(item);
        if (entry->is_group) {
            Utility::DisplayStdErrorDialog(tr("You cannot Copy or Cut groups - use drag-and-drop.")) ;
            return false;
        }
    }

    foreach (SearchEditorModel::searchEntry *entry, entries) {
        SearchEditorModel::searchEntry *save_entry = new SearchEditorModel::searchEntry();
        save_entry->name = entry->name;
        save_entry->description = entry->description;
        save_entry->find = entry->find;
        save_entry->replace = entry->replace;
        save_entry->search_mode = entry->search_mode;
        save_entry->look_where = entry->look_where;
        save_entry->search_direction = entry->search_direction;
        m_SavedSearchEntries.append(save_entry);
    }

    return true;
}

void SearchEditor::Paste()
{
    foreach (SearchEditorModel::searchEntry *entry, m_SavedSearchEntries) {
        AddEntry(entry->is_group, entry);
    }
}

void SearchEditor::Delete()
{
    if (SelectedRowsCount() < 1) {
        return;
    }

    // Delete one at a time as selection may not be contiguous
    int row = -1;
    QModelIndex parent_index;
    while (ui.SearchEditorTree->selectionModel()->hasSelection()) {
        QModelIndex index = ui.SearchEditorTree->selectionModel()->selectedRows(0).first();
        if (index.isValid()) {
            row = index.row();
            parent_index = index.parent();
            m_SearchEditorModel->removeRows(row, 1, parent_index);
        }
    }

    // Select the nearest row in the group, or the group if no rows left
    int parent_row_count;
    if (parent_index.isValid()) {
        parent_row_count = m_SearchEditorModel->itemFromIndex(parent_index)->rowCount();
    }
    else {
        parent_row_count = m_SearchEditorModel->invisibleRootItem()->rowCount();
    }

    if (parent_row_count && row >= parent_row_count) {
        row = parent_row_count - 1;
    }

    if (parent_row_count == 0) {
        row = parent_index.row();
        parent_index = parent_index.parent();
    }

    QModelIndex select_index = m_SearchEditorModel->index(row, 0, parent_index);
    ui.SearchEditorTree->setCurrentIndex(select_index);
    ui.SearchEditorTree->selectionModel()->select(select_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

void SearchEditor::Import()
{
    if (SelectedRowsCount() > 1) {
        return;
    }

    // Get the filename to import from
    QString filter_string = "*." % FILE_EXTENSION;

    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Import Search Entries"),
                                                    m_LastFolderOpen,
                                                    filter_string
                                                   );

    // Load the file and save the last folder opened
    if (!filename.isEmpty()) {
        // Create a new group for the imported items after the selected item
        // Avoids merging with existing groups, etc.
        QStandardItem *item = AddGroup();

        if (item) {
            m_SearchEditorModel->Rename(item, "Imported");

            m_SearchEditorModel->LoadData(filename, item);
    
            m_LastFolderOpen = QFileInfo(filename).absolutePath();
            WriteSettings();
        }
    }
}

void SearchEditor::Export()
{
    if (SelectedRowsCount() < 1) {
        return;
    }

    QList<QStandardItem*> items = GetSelectedItems();

    if (!ItemsAreUnique(m_SearchEditorModel->GetNonParentItems(items))) {
        return;
    }

    QList<SearchEditorModel::searchEntry*> entries;

    foreach (QStandardItem *item, items) {

        // Get all subitems of an item not just the item itself
        QList<QStandardItem*> sub_items = m_SearchEditorModel->GetNonParentItems(item);

        // Get the parent path of the item 
        QString parent_path = "";
        if (item->parent()) {
            parent_path = m_SearchEditorModel->GetFullName(item->parent());
        }

        foreach (QStandardItem *item, sub_items) {
            SearchEditorModel::searchEntry *entry = m_SearchEditorModel->GetEntry(item);

            // Remove the top level paths since we're exporting a subset
            entry->fullname.replace(QRegExp(parent_path), "");
            entry->name = entry->fullname;

            entries.append(entry);
        }
    }

    // Get the filename to use
    QString filter_string = "*." % FILE_EXTENSION;
    QString default_filter = "*";

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Export Selected Searches"),
                                                    m_LastFolderOpen,
                                                    filter_string,
                                                    &default_filter
                                                   );
    if (filename.isEmpty()) {
        return;
    }

    QString extension = QFileInfo(filename).suffix().toLower();
    if (extension != FILE_EXTENSION) {
        filename += "." % FILE_EXTENSION;
    }

    // Save the data, and last folder opened if successful
    if (SaveData(entries, filename)) {
        m_LastFolderOpen = QFileInfo(filename).absolutePath();
        WriteSettings();
    }
}

void SearchEditor::CollapseAll()
{
    ui.SearchEditorTree->collapseAll();
}

void SearchEditor::ExpandAll()
{
    ui.SearchEditorTree->expandAll();
}

bool SearchEditor::FilterEntries(const QString &text, QStandardItem *item)
{
    const QString lowercaseText = text.toLower();
    bool hidden = false;

    QModelIndex parent_index;
    if (item && item->parent()) {
        parent_index = item->parent()->index();
    }

    if (item) {
        // Hide the entry if it doesn't contain the entered text, otherwise show it
        SearchEditorModel::searchEntry *entry = m_SearchEditorModel->GetEntry(item);
        if (ui.Filter->currentIndex() == 0) {
            hidden = !(text.isEmpty() || entry->name.toLower().contains(lowercaseText));
        }
        else {
            hidden = !(text.isEmpty() || entry->name.toLower().contains(lowercaseText) ||
                       entry->description.toLower().contains(lowercaseText) ||
                       entry->find.toLower().contains(lowercaseText) ||
                       entry->replace.toLower().contains(lowercaseText));
        }
        ui.SearchEditorTree->setRowHidden(item->row(), parent_index, hidden);
    }
    else {
        item = m_SearchEditorModel->invisibleRootItem();
    }

    // Recursively set children
    // Show group if any children are visible, but do not hide in case other children are visible
    for (int row = 0; row < item->rowCount(); row++) {
        if (!FilterEntries(text, item->child(row, 0))) {
            hidden = false;
            ui.SearchEditorTree->setRowHidden(item->row(), parent_index, hidden);
        }
    }
    
    return hidden;
}

void SearchEditor::FilterEditTextChangedSlot(const QString &text)
{
    FilterEntries(text);

    ui.SearchEditorTree->expandAll();
    ui.SearchEditorTree->selectionModel()->clear();

    if (!text.isEmpty()) {
        SelectFirstVisibleNonGroup(m_SearchEditorModel->invisibleRootItem());
    }

    return;
}

bool SearchEditor::SelectFirstVisibleNonGroup(QStandardItem *item)
{
    QModelIndex parent_index;
    if (item->parent()) {
        parent_index = item->parent()->index();
    }

    // If the item is not a group and its visible select it and finish
    if (item != m_SearchEditorModel->invisibleRootItem() && !ui.SearchEditorTree->isRowHidden(item->row(), parent_index)) {
        if (!m_SearchEditorModel->ItemIsGroup(item)) {
            ui.SearchEditorTree->selectionModel()->select(m_SearchEditorModel->index(item->row(), 0, parent_index), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            ui.SearchEditorTree->setCurrentIndex(item->index());
            return true;
        }
    }

    // Recursively check children of any groups
    for (int row = 0; row < item->rowCount(); row++) {
        if (SelectFirstVisibleNonGroup(item->child(row, 0))) {
            return true;
        }
    }

    return false;
}

void SearchEditor::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    // Column widths
    int size = settings.beginReadArray("column_data");
    for (int column = 0; column < size && column < ui.SearchEditorTree->header()->count(); column++) {
        settings.setArrayIndex(column);
        int column_width = settings.value("width").toInt();
        if (column_width) {
            ui.SearchEditorTree->setColumnWidth(column, column_width);
        }
    }
    settings.endArray();

    // Last folder open
    m_LastFolderOpen = settings.value("last_folder_open").toString();

    settings.endGroup();
}

void SearchEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    // Column widths
    settings.beginWriteArray("column_data");
    for (int column = 0; column < ui.SearchEditorTree->header()->count(); column++) {
        settings.setArrayIndex(column);
        settings.setValue("width", ui.SearchEditorTree->columnWidth(column));
    }
    settings.endArray();

    // Last folder open
    settings.setValue( "last_folder_open", m_LastFolderOpen);

    settings.endGroup();
}

void SearchEditor::CreateContextMenuActions()
{
    m_AddEntry  =   new QAction(tr( "Add Entry" ),  this );
    m_AddGroup  =   new QAction(tr( "Add Group" ),  this );
    m_Cut       =   new QAction(tr( "Cut" ),        this );
    m_Copy      =   new QAction(tr( "Copy" ),       this );
    m_Paste     =   new QAction(tr( "Paste" ),      this );
    m_Delete    =   new QAction(tr( "Delete" ),     this );
    m_Import    =   new QAction(tr( "Import" ),     this );
    m_Export    =   new QAction(tr( "Export" ),     this );
    m_CollapseAll = new QAction(tr( "Collapse All" ),  this );
    m_ExpandAll =   new QAction(tr( "Expand All" ),  this );

    m_AddEntry->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_E));
    m_AddGroup->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_G));
    m_Cut->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_X));
    m_Copy->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_C));
    m_Paste->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_V));
    m_Delete->setShortcut(QKeySequence::Delete);

    // Has to be added to the dialog itself for the keyboard shortcut to work.
    addAction(m_AddEntry);
    addAction(m_AddGroup);
    addAction(m_Cut);
    addAction(m_Copy);
    addAction(m_Paste);
    addAction(m_Delete);
}

void SearchEditor::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);

    m_ContextMenu->exec(ui.SearchEditorTree->viewport()->mapToGlobal(point));
    m_ContextMenu->clear();

    // Make sure every action is enabled - in case shortcut is used after context menu disables some.
    m_AddEntry->setEnabled(true);
    m_AddGroup->setEnabled(true);
    m_Cut->setEnabled(true);
    m_Copy->setEnabled(true);
    m_Paste->setEnabled(true);
    m_Delete->setEnabled(true);
    m_Import->setEnabled(true);
    m_Export->setEnabled(true);
    m_CollapseAll->setEnabled(true);
    m_ExpandAll->setEnabled(true);
}

void SearchEditor::SetupContextMenu(const QPoint &point)
{
    int selected_rows_count = SelectedRowsCount();

    m_ContextMenu->addAction(m_AddEntry);

    m_ContextMenu->addAction(m_AddGroup);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Cut);
    m_Cut->setEnabled(selected_rows_count > 0);

    m_ContextMenu->addAction(m_Copy);
    m_Copy->setEnabled(selected_rows_count > 0);

    m_ContextMenu->addAction(m_Paste);
    m_Paste->setEnabled(m_SavedSearchEntries.count());

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Delete);
    m_Delete->setEnabled(selected_rows_count > 0);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Import);
    m_Import->setEnabled(selected_rows_count <= 1);

    m_ContextMenu->addAction(m_Export);
    m_Export->setEnabled(selected_rows_count > 0);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_CollapseAll);

    m_ContextMenu->addAction(m_ExpandAll);
}


void SearchEditor::reject()
{
    m_SearchEditorModel->LoadInitialData();
    QDialog::reject();
}

void SearchEditor::accept()
{
    if (SaveData()) {
        WriteSettings();
        LoadFindReplace();
        QDialog::accept();
    }
}

void SearchEditor::ConnectSignalsSlots()
{
    connect(ui.FilterText, SIGNAL(textChanged(QString)), this, SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.Find,       SIGNAL(clicked()),            this, SLOT(Find()));
    connect(ui.Replace,    SIGNAL(clicked()),            this, SLOT(Replace()));
    connect(ui.CountAll,   SIGNAL(clicked()),            this, SLOT(CountAll()));
    connect(ui.ReplaceAll, SIGNAL(clicked()),            this, SLOT(ReplaceAll()));

    connect(ui.SearchEditorTree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this,                SLOT(  OpenContextMenu(           const QPoint&)));

    connect(m_AddEntry,    SIGNAL(triggered()), this, SLOT(AddEntry()));
    connect(m_AddGroup,    SIGNAL(triggered()), this, SLOT(AddGroup()));
    connect(m_Cut,         SIGNAL(triggered()), this, SLOT(Cut()));
    connect(m_Copy,        SIGNAL(triggered()), this, SLOT(Copy()));
    connect(m_Paste,       SIGNAL(triggered()), this, SLOT(Paste()));
    connect(m_Delete,      SIGNAL(triggered()), this, SLOT(Delete()));
    connect(m_Import,      SIGNAL(triggered()), this, SLOT(Import()));
    connect(m_Export,      SIGNAL(triggered()), this, SLOT(Export()));
    connect(m_CollapseAll, SIGNAL(triggered()), this, SLOT(CollapseAll()));
    connect(m_ExpandAll,   SIGNAL(triggered()), this, SLOT(ExpandAll()));
}
