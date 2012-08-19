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
#include <QtGui/QAction>
#include <QtGui/QMenu>

#include "Dialogs/ClipboardEditor.h"
#include "Misc/Utility.h"

static const QString SETTINGS_GROUP = "clipboard_editor";
static const QString FILE_EXTENSION = "ini";

ClipboardEditor::ClipboardEditor(QWidget *parent)
    :
    QDialog(parent),
    m_LastFolderOpen(QString()),
    m_ContextMenu(new QMenu(this))
{
    ui.setupUi(this);

    SetupClipboardEditorTree();

    CreateContextMenuActions();

    ConnectSignalsSlots();
}

ClipboardEditor::~ClipboardEditor()
{
    // Restore data from file since we aren't saving
    m_ClipboardEditorModel->LoadInitialData();
}

void ClipboardEditor::SetupClipboardEditorTree()
{
    m_ClipboardEditorModel = ClipboardEditorModel::instance();

    ui.ClipboardEditorTree->setModel(m_ClipboardEditorModel);
    ui.ClipboardEditorTree->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.ClipboardEditorTree->setSortingEnabled(false);
    ui.ClipboardEditorTree->setWordWrap(true);
    ui.ClipboardEditorTree->setAlternatingRowColors(true);

    ui.ClipboardEditorTree->header()->setToolTip(
        "<p>" + tr("Right click on an entry to see a context menu of actions.") + "</p>" +
        "<p>" + tr("You can also right click in Code View to select an entry.") + "</p>" +
        "<dl>" +
        "<dt><b>" + tr("Name") + "</b><dd>" + tr("Name of your entry or group.") + "</dd>" +
        "<dt><b>" + tr("Description") + "</b><dd>" + tr("Optional.") + "</dd>" +
        "<dt><b>" + tr("Text") + "</b><dd>" + tr("The text to insert. Use \\0 to include selected text.") + "</dd>" +
        "</dl>");

    ui.ClipboardEditorTree->header()->setStretchLastSection(true);
    for (int column = 0; column < ui.ClipboardEditorTree->header()->count(); column++) {
        ui.ClipboardEditorTree->resizeColumnToContents(column);
    }
}

bool ClipboardEditor::SaveData(QList<ClipboardEditorModel::clipEntry*> entries, QString filename)
{
    QString message = m_ClipboardEditorModel->SaveData(entries, filename);

    if (!message.isEmpty()) {
        Utility::DisplayStdErrorDialog(tr("Cannot save entries.") + "\n\n" + message);
    }
    return message.isEmpty();
}

void ClipboardEditor::PasteIntoDocument()
{
    emit PasteSelectedClipboardRequest(GetSelectedEntries());
}

void ClipboardEditor::showEvent(QShowEvent *event)
{
    ReadSettings();

    ui.ClipboardEditorTree->expandAll();
    ui.Filter->setCurrentIndex(0);
    ui.FilterText->clear();
    ui.FilterText->setFocus();
}

int ClipboardEditor::SelectedRowsCount()
{
    int count = 0;
    if (ui.ClipboardEditorTree->selectionModel()->hasSelection()) {
        count = ui.ClipboardEditorTree->selectionModel()->selectedRows(0).count();
    }

    return count;
}

QList<ClipboardEditorModel::clipEntry*> ClipboardEditor::GetSelectedEntries()
{
    QList<ClipboardEditorModel::clipEntry *> selected_entries;

    if (ui.ClipboardEditorTree->selectionModel()->hasSelection()) {

        QList<QStandardItem*> items = m_ClipboardEditorModel->GetNonGroupItems(GetSelectedItems());

        if (!ItemsAreUnique(items)) {
            return selected_entries;
        }

        selected_entries = m_ClipboardEditorModel->GetEntries(items);
    }

    return selected_entries;
}

QList<QStandardItem*> ClipboardEditor::GetSelectedItems()
{
    // Shift-click order is top to bottom regardless of starting position
    // Ctrl-click order is first clicked to last clicked (included shift-clicks stay ordered as is)

    QModelIndexList selected_indexes = ui.ClipboardEditorTree->selectionModel()->selectedRows(0);
    QList<QStandardItem*> selected_items;

    foreach (QModelIndex index, selected_indexes) {
        selected_items.append(m_ClipboardEditorModel->itemFromIndex(index));
    }

    return selected_items;
}

bool ClipboardEditor::ItemsAreUnique(QList<QStandardItem*> items)
{
    // Although saving a group and a sub item works, it could be confusing to users to
    // have and entry appear twice so its more predictable just to prevent it and warn the user
    if (items.toSet().count() != items.count()) {
        Utility::DisplayStdErrorDialog(tr("You cannot select an entry and a group containing the entry."));
        return false;
    }

    return true;
}

QStandardItem* ClipboardEditor::AddEntry(bool is_group, ClipboardEditorModel::clipEntry *clip_entry, bool insert_after)
{
    QStandardItem *parent_item = NULL;
    QStandardItem *new_item = NULL;
    int row = 0;

    // If adding a new/blank entry add it after the selected entry.
    if (insert_after) {
        if (ui.ClipboardEditorTree->selectionModel()->hasSelection()) {
            QModelIndexList selected_indexes = ui.ClipboardEditorTree->selectionModel()->selectedRows(0);
            parent_item = GetSelectedItems().last();

            if (!parent_item) {
                return parent_item;
            }

            if (!m_ClipboardEditorModel->ItemIsGroup(parent_item)) {
                    row = parent_item->row() + 1;
                    parent_item = parent_item->parent();
            }
        }
    }

    // Make sure the new entry can be seen
    if (parent_item) {
        ui.ClipboardEditorTree->expand(parent_item->index());
    }

    new_item = m_ClipboardEditorModel->AddEntryToModel(clip_entry, is_group, parent_item, row);
    QModelIndex new_index = new_item->index();

    // Select the added item and set it for editing
    ui.ClipboardEditorTree->selectionModel()->clear();
    ui.ClipboardEditorTree->setCurrentIndex(new_index);
    ui.ClipboardEditorTree->selectionModel()->select(new_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    ui.ClipboardEditorTree->edit(new_index);
    ui.ClipboardEditorTree->selectionModel()->select(new_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);

    return new_item;
}

QStandardItem* ClipboardEditor::AddGroup()
{
    return AddEntry(true);
}

void ClipboardEditor::Cut()
{
    if (Copy()) {
        Delete();
    }
}

bool ClipboardEditor::Copy()
{
    if (SelectedRowsCount() < 1) {
        return false;
    }

    while (m_SavedClipboardEntries.count()) {
        m_SavedClipboardEntries.removeAt(0);
    }

    QList<ClipboardEditorModel::clipEntry *> entries = GetSelectedEntries();
    if (!entries.count()) {
        return false;
    }

    foreach (QStandardItem *item, GetSelectedItems()) {
        ClipboardEditorModel::clipEntry *entry = m_ClipboardEditorModel->GetEntry(item);
        if (entry->is_group) {
            Utility::DisplayStdErrorDialog(tr("You cannot Copy or Cut groups - use drag-and-drop.")) ;
            return false;
        }
    }

    foreach (ClipboardEditorModel::clipEntry *entry, entries) {
        ClipboardEditorModel::clipEntry *save_entry = new ClipboardEditorModel::clipEntry();
        save_entry->name = entry->name;
        save_entry->is_group = entry->is_group;
        save_entry->description = entry->description;
        save_entry->text = entry->text;
        m_SavedClipboardEntries.append(save_entry);
    }
    return true;
}

void ClipboardEditor::Paste()
{
    foreach (ClipboardEditorModel::clipEntry *entry, m_SavedClipboardEntries) {
        AddEntry(entry->is_group, entry);
    }
}

void ClipboardEditor::Delete()
{
    if (SelectedRowsCount() < 1) {
        return;
    }

    // Delete one at a time as selection may not be contiguous
    int row = -1;
    QModelIndex parent_index;
    while (ui.ClipboardEditorTree->selectionModel()->hasSelection()) {
        QModelIndex index = ui.ClipboardEditorTree->selectionModel()->selectedRows(0).first();
        if (index.isValid()) {
            row = index.row();
            parent_index = index.parent();
            m_ClipboardEditorModel->removeRows(row, 1, parent_index);
        }
    }

    // Select the nearest row in the group if there is one
    int parent_row_count;
    if (parent_index.isValid()) {
        parent_row_count = m_ClipboardEditorModel->itemFromIndex(parent_index)->rowCount();
    }
    else {
        parent_row_count = m_ClipboardEditorModel->invisibleRootItem()->rowCount();
    }
    
    if (parent_row_count && row >= 0) {
        if (row >= parent_row_count) {
            row = parent_row_count - 1;
        }
        if (row >= 0) {
            QModelIndex select_index = m_ClipboardEditorModel->index(row, 0, parent_index);
            ui.ClipboardEditorTree->setCurrentIndex(select_index);
            ui.ClipboardEditorTree->selectionModel()->select(select_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }
    }
}

void ClipboardEditor::Import()
{
    if (SelectedRowsCount() > 1) {
        return;
    }

    // Get the filename to import from
    QString filter_string = "*." % FILE_EXTENSION;

    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Import Entries"),
                                                    m_LastFolderOpen,
                                                    filter_string
                                                   );

    // Load the file and save the last folder opened
    if (!filename.isEmpty()) {
        // Create a new group for the imported items after the selected item
        // Avoids merging with existing groups, etc.
        QStandardItem *item = AddGroup();

        if (item) {
            m_ClipboardEditorModel->Rename(item, "Imported");

            m_ClipboardEditorModel->LoadData(filename, item);
    
            m_LastFolderOpen = QFileInfo(filename).absolutePath();
            WriteSettings();
        }
    }
}

void ClipboardEditor::Export()
{
    if (SelectedRowsCount() < 1) {
        return;
    }

    QList<QStandardItem*> items = GetSelectedItems();

    if (!ItemsAreUnique(m_ClipboardEditorModel->GetNonParentItems(items))) {
        return;
    }

    QList<ClipboardEditorModel::clipEntry*> entries;

    foreach (QStandardItem *item, items) {

        // Get all subitems of an item not just the item itself
        QList<QStandardItem*> sub_items = m_ClipboardEditorModel->GetNonParentItems(item);

        // Get the parent path of the item 
        QString parent_path = "";
        if (item->parent()) {
            parent_path = m_ClipboardEditorModel->GetFullName(item->parent());
        }

        foreach (QStandardItem *item, sub_items) {
            ClipboardEditorModel::clipEntry *entry = m_ClipboardEditorModel->GetEntry(item);

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
                                                    tr("Export Selected Entries"),
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

void ClipboardEditor::CollapseAll()
{
    ui.ClipboardEditorTree->collapseAll();
}

void ClipboardEditor::ExpandAll()
{
    ui.ClipboardEditorTree->expandAll();
}

bool ClipboardEditor::FilterEntries(const QString &text, QStandardItem *item)
{
    const QString lowercaseText = text.toLower();
    bool hidden = false;

    QModelIndex parent_index;
    if (item && item->parent()) {
        parent_index = item->parent()->index();
    }

    if (item) {
        // Hide the entry if it doesn't contain the entered text, otherwise show it
        ClipboardEditorModel::clipEntry *entry = m_ClipboardEditorModel->GetEntry(item);
        if (ui.Filter->currentIndex() == 0) {
            hidden = !(text.isEmpty() || entry->name.toLower().contains(lowercaseText));
        }
        else {
            hidden = !(text.isEmpty() || entry->name.toLower().contains(lowercaseText) ||
                       entry->description.toLower().contains(lowercaseText) ||
                       entry->text.toLower().contains(lowercaseText));
        }
        ui.ClipboardEditorTree->setRowHidden(item->row(), parent_index, hidden);
    }
    else {
        item = m_ClipboardEditorModel->invisibleRootItem();
    }

    // Recursively set children
    // Show group if any children are visible, but do not hide in case other children are visible
    for (int row = 0; row < item->rowCount(); row++) {
        if (!FilterEntries(text, item->child(row, 0))) {
            hidden = false;
            ui.ClipboardEditorTree->setRowHidden(item->row(), parent_index, hidden);
        }
    }
    
    return hidden;
}

void ClipboardEditor::FilterEditTextChangedSlot(const QString &text)
{
    FilterEntries(text);

    ui.ClipboardEditorTree->expandAll();
    ui.ClipboardEditorTree->selectionModel()->clear();

    if (!text.isEmpty()) {
        SelectFirstVisibleNonGroup(m_ClipboardEditorModel->invisibleRootItem());
    }

    return;
}

bool ClipboardEditor::SelectFirstVisibleNonGroup(QStandardItem *item)
{
    QModelIndex parent_index;
    if (item->parent()) {
        parent_index = item->parent()->index();
    }

    // If the item is not a group and its visible select it and finish
    if (item != m_ClipboardEditorModel->invisibleRootItem() && !ui.ClipboardEditorTree->isRowHidden(item->row(), parent_index)) {
        if (!m_ClipboardEditorModel->ItemIsGroup(item)) {
            ui.ClipboardEditorTree->selectionModel()->select(m_ClipboardEditorModel->index(item->row(), 0, parent_index), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            ui.ClipboardEditorTree->setCurrentIndex(item->index());
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

void ClipboardEditor::ReadSettings()
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
    for (int column = 0; column < size && column < ui.ClipboardEditorTree->header()->count(); column++) {
        settings.setArrayIndex(column);
        int column_width = settings.value("width").toInt();
        if (column_width) {
            ui.ClipboardEditorTree->setColumnWidth(column, column_width);
        }
    }
    settings.endArray();

    // Last folder open
    m_LastFolderOpen = settings.value("last_folder_open").toString();

    settings.endGroup();
}

void ClipboardEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    // Column widths
    settings.beginWriteArray("column_data");
    for (int column = 0; column < ui.ClipboardEditorTree->header()->count(); column++) {
        settings.setArrayIndex(column);
        settings.setValue("width", ui.ClipboardEditorTree->columnWidth(column));
    }
    settings.endArray();

    // Last folder open
    settings.setValue( "last_folder_open", m_LastFolderOpen);

    settings.endGroup();
}

void ClipboardEditor::CreateContextMenuActions()
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

void ClipboardEditor::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);

    m_ContextMenu->exec(ui.ClipboardEditorTree->viewport()->mapToGlobal(point));
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

void ClipboardEditor::SetupContextMenu(const QPoint &point)
{
    int selected_row_count = SelectedRowsCount();

    m_ContextMenu->addAction(m_AddEntry);

    m_ContextMenu->addAction(m_AddGroup);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Cut);
    m_Cut->setEnabled(selected_row_count > 0);

    m_ContextMenu->addAction(m_Copy);
    m_Copy->setEnabled(selected_row_count > 0);

    m_ContextMenu->addAction(m_Paste);
    m_Paste->setEnabled(m_SavedClipboardEntries.count());

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Delete);
    m_Delete->setEnabled(selected_row_count > 0);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Import);
    m_Import->setEnabled(selected_row_count <= 1);

    m_ContextMenu->addAction(m_Export);
    m_Export->setEnabled(selected_row_count > 0);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_CollapseAll);

    m_ContextMenu->addAction(m_ExpandAll);
}

void ClipboardEditor::reject()
{
    m_ClipboardEditorModel->LoadInitialData();
    QDialog::reject();
}

void ClipboardEditor::accept()
{
    if (SaveData()) {
        WriteSettings();
        PasteIntoDocument();
        QDialog::accept();
    }
}

void ClipboardEditor::ConnectSignalsSlots()
{
    connect(ui.FilterText,          SIGNAL(textChanged(QString)), this, SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.PasteIntoDocument,   SIGNAL(clicked()),            this, SLOT(PasteIntoDocument()));

    connect(ui.ClipboardEditorTree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this,                   SLOT(  OpenContextMenu(                  const QPoint&)));

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
