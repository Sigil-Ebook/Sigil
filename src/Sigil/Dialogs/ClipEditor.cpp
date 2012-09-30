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
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QFileDialog>
#include <QtGui/QMenu>
#include <QtGui/QPushButton>

#include "Dialogs/ClipEditor.h"
#include "Misc/Utility.h"

static const QString SETTINGS_GROUP = "clip_editor";
static const QString FILE_EXTENSION = "ini";

ClipEditor::ClipEditor(QWidget *parent)
    :
    QDialog(parent),
    m_LastFolderOpen(QString()),
    m_ContextMenu(new QMenu(this))
{
    ui.setupUi(this);

    SetupClipEditorTree();

    CreateContextMenuActions();

    ConnectSignalsSlots();
}

ClipEditor::~ClipEditor()
{
    // Restore data from file since we aren't saving
    m_ClipEditorModel->LoadInitialData();
}

void ClipEditor::SetupClipEditorTree()
{
    m_ClipEditorModel = ClipEditorModel::instance();

    ui.ClipEditorTree->setModel(m_ClipEditorModel);
    ui.ClipEditorTree->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.ClipEditorTree->setSortingEnabled(false);
    ui.ClipEditorTree->setWordWrap(true);
    ui.ClipEditorTree->setAlternatingRowColors(true);

    ui.ClipEditorTree->header()->setToolTip(
        "<p>" + tr("Right click on an entry to see a context menu of actions.") + "</p>" +
        "<p>" + tr("You can also right click in Code View to select an entry.") + "</p>" +
        "<dl>" +
        "<dt><b>" + tr("Name") + "</b><dd>" + tr("Name of your entry or group.") + "</dd>" +
        "<dt><b>" + tr("Text") + "</b><dd>" + tr("The text to insert. The text is treated like a Regex replacement expression so \\1 can be used to insert the text selected in Code View when you paste the clip.") + "</dd>" +
        "</dl>");

    ui.buttonBox->setToolTip( QString() +
        "<dl>" +
        "<dt><b>" + tr("Apply") + "</b><dd>" + tr("Paste the selected entry into the active window.") + "</dd>" +
        "<dt><b>" + tr("Cancel") + "</b><dd>" + tr("Close without saving.") + "</dd>" +
        "<dt><b>" + tr("OK") + "</b><dd>" + tr("Paste the selected entry, save your changes, and close.") + "</dd>" +
        "</dl>");


    ui.ClipEditorTree->header()->setStretchLastSection(true);
}

bool ClipEditor::SaveData(QList<ClipEditorModel::clipEntry*> entries, QString filename)
{
    QString message = m_ClipEditorModel->SaveData(entries, filename);

    if (!message.isEmpty()) {
        Utility::DisplayStdErrorDialog(tr("Cannot save entries.") + "\n\n" + message);
    }
    return message.isEmpty();
}

void ClipEditor::PasteIntoDocument()
{
    emit PasteSelectedClipRequest(GetSelectedEntries());
}

void ClipEditor::showEvent(QShowEvent *event)
{
    ReadSettings();

    ui.ClipEditorTree->expandAll();
    ui.Filter->setCurrentIndex(0);
    ui.FilterText->clear();
    ui.FilterText->setFocus();

    for (int column = 0; column < ui.ClipEditorTree->header()->count() - 1; column++) {
        ui.ClipEditorTree->resizeColumnToContents(column);
    }
}

int ClipEditor::SelectedRowsCount()
{
    int count = 0;
    if (ui.ClipEditorTree->selectionModel()->hasSelection()) {
        count = ui.ClipEditorTree->selectionModel()->selectedRows(0).count();
    }

    return count;
}

QList<ClipEditorModel::clipEntry*> ClipEditor::GetSelectedEntries()
{
    QList<ClipEditorModel::clipEntry *> selected_entries;

    if (ui.ClipEditorTree->selectionModel()->hasSelection()) {

        QList<QStandardItem*> items = m_ClipEditorModel->GetNonGroupItems(GetSelectedItems());

        if (!ItemsAreUnique(items)) {
            return selected_entries;
        }

        selected_entries = m_ClipEditorModel->GetEntries(items);
    }

    return selected_entries;
}

QList<QStandardItem*> ClipEditor::GetSelectedItems()
{
    // Shift-click order is top to bottom regardless of starting position
    // Ctrl-click order is first clicked to last clicked (included shift-clicks stay ordered as is)

    QModelIndexList selected_indexes = ui.ClipEditorTree->selectionModel()->selectedRows(0);
    QList<QStandardItem*> selected_items;

    foreach (QModelIndex index, selected_indexes) {
        selected_items.append(m_ClipEditorModel->itemFromIndex(index));
    }

    return selected_items;
}

bool ClipEditor::ItemsAreUnique(QList<QStandardItem*> items)
{
    // Although saving a group and a sub item works, it could be confusing to users to
    // have and entry appear twice so its more predictable just to prevent it and warn the user
    if (items.toSet().count() != items.count()) {
        Utility::DisplayStdErrorDialog(tr("You cannot select an entry and a group containing the entry."));
        return false;
    }

    return true;
}

QStandardItem* ClipEditor::AddEntry(bool is_group, ClipEditorModel::clipEntry *clip_entry, bool insert_after)
{
    QStandardItem *parent_item = NULL;
    QStandardItem *new_item = NULL;
    int row = 0;

    // If adding a new/blank entry add it after the selected entry.
    if (insert_after) {
        if (ui.ClipEditorTree->selectionModel()->hasSelection()) {
            parent_item = GetSelectedItems().last();

            if (!parent_item) {
                return parent_item;
            }

            if (!m_ClipEditorModel->ItemIsGroup(parent_item)) {
                    row = parent_item->row() + 1;
                    parent_item = parent_item->parent();
            }
        }
    }

    // Make sure the new entry can be seen
    if (parent_item) {
        ui.ClipEditorTree->expand(parent_item->index());
    }

    new_item = m_ClipEditorModel->AddEntryToModel(clip_entry, is_group, parent_item, row);
    QModelIndex new_index = new_item->index();

    // Select the added item and set it for editing
    ui.ClipEditorTree->selectionModel()->clear();
    ui.ClipEditorTree->setCurrentIndex(new_index);
    ui.ClipEditorTree->selectionModel()->select(new_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    ui.ClipEditorTree->edit(new_index);
    ui.ClipEditorTree->selectionModel()->select(new_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);

    return new_item;
}

QStandardItem* ClipEditor::AddGroup()
{
    return AddEntry(true);
}

void ClipEditor::Edit()
{
    ui.ClipEditorTree->edit(ui.ClipEditorTree->currentIndex());
}

void ClipEditor::Cut()
{
    if (Copy()) {
        Delete();
    }
}

bool ClipEditor::Copy()
{
    if (SelectedRowsCount() < 1) {
        return false;
    }

    while (m_SavedClipEntries.count()) {
        m_SavedClipEntries.removeAt(0);
    }

    QList<ClipEditorModel::clipEntry *> entries = GetSelectedEntries();
    if (!entries.count()) {
        return false;
    }

    foreach (QStandardItem *item, GetSelectedItems()) {
        ClipEditorModel::clipEntry *entry = m_ClipEditorModel->GetEntry(item);
        if (entry->is_group) {
            Utility::DisplayStdErrorDialog(tr("You cannot Copy or Cut groups - use drag-and-drop.")) ;
            return false;
        }
    }

    foreach (ClipEditorModel::clipEntry *entry, entries) {
        ClipEditorModel::clipEntry *save_entry = new ClipEditorModel::clipEntry();
        save_entry->name = entry->name;
        save_entry->is_group = entry->is_group;
        save_entry->text = entry->text;
        m_SavedClipEntries.append(save_entry);
    }
    return true;
}

void ClipEditor::Paste()
{
    foreach (ClipEditorModel::clipEntry *entry, m_SavedClipEntries) {
        AddEntry(entry->is_group, entry);
    }
}

void ClipEditor::Delete()
{
    if (SelectedRowsCount() < 1) {
        return;
    }

    // Delete one at a time as selection may not be contiguous
    int row = -1;
    QModelIndex parent_index;
    while (ui.ClipEditorTree->selectionModel()->hasSelection()) {
        QModelIndex index = ui.ClipEditorTree->selectionModel()->selectedRows(0).first();
        if (index.isValid()) {
            row = index.row();
            parent_index = index.parent();
            m_ClipEditorModel->removeRows(row, 1, parent_index);
        }
    }

    // Select the nearest row in the group if there is one
    int parent_row_count;
    if (parent_index.isValid()) {
        parent_row_count = m_ClipEditorModel->itemFromIndex(parent_index)->rowCount();
    }
    else {
        parent_row_count = m_ClipEditorModel->invisibleRootItem()->rowCount();
    }
    
    if (parent_row_count && row >= 0) {
        if (row >= parent_row_count) {
            row = parent_row_count - 1;
        }
        if (row >= 0) {
            QModelIndex select_index = m_ClipEditorModel->index(row, 0, parent_index);
            ui.ClipEditorTree->setCurrentIndex(select_index);
            ui.ClipEditorTree->selectionModel()->select(select_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }
    }
}

void ClipEditor::Import()
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
            m_ClipEditorModel->Rename(item, "Imported");

            m_ClipEditorModel->LoadData(filename, item);
    
            m_LastFolderOpen = QFileInfo(filename).absolutePath();
            WriteSettings();
        }
    }
}

void ClipEditor::ExportAll()
{
    QList<QStandardItem*> items = GetSelectedItems();

    QStandardItem *item = m_ClipEditorModel->invisibleRootItem();
    QModelIndex parent_index;

    for (int row = 0; row < item->rowCount(); row++) {
        items.append(item->child(row,0));
    }

    ExportItems(items);
}

void ClipEditor::Export()
{
    if (SelectedRowsCount() < 1) {
        return;
    }

    QList<QStandardItem*> items = GetSelectedItems();

    if (!ItemsAreUnique(m_ClipEditorModel->GetNonParentItems(items))) {
        return;
    }

    ExportItems(items);
}

void ClipEditor::ExportItems(QList<QStandardItem*> items)
{
    QList<ClipEditorModel::clipEntry*> entries;

    foreach (QStandardItem *item, items) {

        // Get all subitems of an item not just the item itself
        QList<QStandardItem*> sub_items = m_ClipEditorModel->GetNonParentItems(item);

        // Get the parent path of the item 
        QString parent_path = "";
        if (item->parent()) {
            parent_path = m_ClipEditorModel->GetFullName(item->parent());
        }

        foreach (QStandardItem *item, sub_items) {
            ClipEditorModel::clipEntry *entry = m_ClipEditorModel->GetEntry(item);

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

void ClipEditor::CollapseAll()
{
    ui.ClipEditorTree->collapseAll();
}

void ClipEditor::ExpandAll()
{
    ui.ClipEditorTree->expandAll();
}

bool ClipEditor::FilterEntries(const QString &text, QStandardItem *item)
{
    const QString lowercaseText = text.toLower();
    bool hidden = false;

    QModelIndex parent_index;
    if (item && item->parent()) {
        parent_index = item->parent()->index();
    }

    if (item) {
        // Hide the entry if it doesn't contain the entered text, otherwise show it
        ClipEditorModel::clipEntry *entry = m_ClipEditorModel->GetEntry(item);
        if (ui.Filter->currentIndex() == 0) {
            hidden = !(text.isEmpty() || entry->name.toLower().contains(lowercaseText));
        }
        else {
            hidden = !(text.isEmpty() || entry->name.toLower().contains(lowercaseText) ||
                       entry->text.toLower().contains(lowercaseText));
        }
        ui.ClipEditorTree->setRowHidden(item->row(), parent_index, hidden);
    }
    else {
        item = m_ClipEditorModel->invisibleRootItem();
    }

    // Recursively set children
    // Show group if any children are visible, but do not hide in case other children are visible
    for (int row = 0; row < item->rowCount(); row++) {
        if (!FilterEntries(text, item->child(row, 0))) {
            hidden = false;
            ui.ClipEditorTree->setRowHidden(item->row(), parent_index, hidden);
        }
    }
    
    return hidden;
}

void ClipEditor::FilterEditTextChangedSlot(const QString &text)
{
    FilterEntries(text);

    ui.ClipEditorTree->expandAll();
    ui.ClipEditorTree->selectionModel()->clear();

    if (!text.isEmpty()) {
        SelectFirstVisibleNonGroup(m_ClipEditorModel->invisibleRootItem());
    }

    return;
}

bool ClipEditor::SelectFirstVisibleNonGroup(QStandardItem *item)
{
    QModelIndex parent_index;
    if (item->parent()) {
        parent_index = item->parent()->index();
    }

    // If the item is not a group and its visible select it and finish
    if (item != m_ClipEditorModel->invisibleRootItem() && !ui.ClipEditorTree->isRowHidden(item->row(), parent_index)) {
        if (!m_ClipEditorModel->ItemIsGroup(item)) {
            ui.ClipEditorTree->selectionModel()->select(m_ClipEditorModel->index(item->row(), 0, parent_index), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            ui.ClipEditorTree->setCurrentIndex(item->index());
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

void ClipEditor::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    // Last folder open
    m_LastFolderOpen = settings.value("last_folder_open").toString();

    settings.endGroup();
}

void ClipEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    // Last folder open
    settings.setValue( "last_folder_open", m_LastFolderOpen);

    settings.endGroup();
}

void ClipEditor::CreateContextMenuActions()
{
    m_AddEntry  =   new QAction(tr( "Add Entry" ),  this );
    m_AddGroup  =   new QAction(tr( "Add Group" ),  this );
    m_Edit      =   new QAction(tr( "Edit" ),       this );
    m_Cut       =   new QAction(tr( "Cut" ),        this );
    m_Copy      =   new QAction(tr( "Copy" ),       this );
    m_Paste     =   new QAction(tr( "Paste" ),      this );
    m_Delete    =   new QAction(tr( "Delete" ),     this );
    m_Import    =   new QAction(tr( "Import" ),     this );
    m_Export    =   new QAction(tr( "Export" ),     this );
    m_ExportAll =   new QAction(tr( "Export All" ), this );
    m_CollapseAll = new QAction(tr( "Collapse All" ),  this );
    m_ExpandAll =   new QAction(tr( "Expand All" ),  this );

    m_AddEntry->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_E));
    m_AddGroup->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_G));
    m_Edit->setShortcut(QKeySequence(Qt::Key_F2));
    m_Cut->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_X));
    m_Copy->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_C));
    m_Paste->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_V));
    m_Delete->setShortcut(QKeySequence::Delete);

    // Has to be added to the dialog itself for the keyboard shortcut to work.
    addAction(m_AddEntry);
    addAction(m_AddGroup);
    addAction(m_Edit);
    addAction(m_Cut);
    addAction(m_Copy);
    addAction(m_Paste);
    addAction(m_Delete);
}

void ClipEditor::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);

    m_ContextMenu->exec(ui.ClipEditorTree->viewport()->mapToGlobal(point));
    m_ContextMenu->clear();

    // Make sure every action is enabled - in case shortcut is used after context menu disables some.
    m_AddEntry->setEnabled(true);
    m_AddGroup->setEnabled(true);
    m_Edit->setEnabled(true);
    m_Cut->setEnabled(true);
    m_Copy->setEnabled(true);
    m_Paste->setEnabled(true);
    m_Delete->setEnabled(true);
    m_Import->setEnabled(true);
    m_Export->setEnabled(true);
    m_ExportAll->setEnabled(true);
    m_CollapseAll->setEnabled(true);
    m_ExpandAll->setEnabled(true);
}

void ClipEditor::SetupContextMenu(const QPoint &point)
{
    int selected_rows_count = SelectedRowsCount();

    m_ContextMenu->addAction(m_AddEntry);

    m_ContextMenu->addAction(m_AddGroup);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Edit);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Cut);
    m_Cut->setEnabled(selected_rows_count > 0);

    m_ContextMenu->addAction(m_Copy);
    m_Copy->setEnabled(selected_rows_count > 0);

    m_ContextMenu->addAction(m_Paste);
    m_Paste->setEnabled(m_SavedClipEntries.count());

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Delete);
    m_Delete->setEnabled(selected_rows_count > 0);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Import);
    m_Import->setEnabled(selected_rows_count <= 1);

    m_ContextMenu->addAction(m_Export);
    m_Export->setEnabled(selected_rows_count > 0);

    m_ContextMenu->addAction(m_ExportAll);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_CollapseAll);

    m_ContextMenu->addAction(m_ExpandAll);
}

void ClipEditor::reject()
{
    m_ClipEditorModel->LoadInitialData();
    QDialog::reject();
}

void ClipEditor::accept()
{
    if (SaveData()) {
        WriteSettings();
        PasteIntoDocument();
        QDialog::accept();
    }
}

void ClipEditor::Apply()
{
    PasteIntoDocument();
}

void ClipEditor::MoveUp()
{
    MoveVertical(false);
}

void ClipEditor::MoveDown()
{
    MoveVertical(true);
}

void ClipEditor::MoveVertical(bool move_down)
{
    if (!ui.ClipEditorTree->selectionModel()->hasSelection()) {
        return;
    }
    QModelIndexList selected_indexes = ui.ClipEditorTree->selectionModel()->selectedRows(0);
    if (selected_indexes.count() > 1) {
        return;
    }

    // Identify the selected item
    QModelIndex index = selected_indexes.first();
    int row = index.row();

    QStandardItem *item = m_ClipEditorModel->itemFromIndex(index);
    QStandardItem *parent_item = item->parent();
    if (!parent_item) {
        parent_item = m_ClipEditorModel->invisibleRootItem();
    }

    int destination_row;
    if (move_down) {
        if (row >= parent_item->rowCount() - 1) {
            return;
        }
        destination_row = row + 1;
    }
    else {
        if (row == 0) {
            return;
        }
        destination_row = row - 1;
    }

    // Swap the item rows
    QList<QStandardItem *> row_items = parent_item->takeRow(row);
    parent_item->insertRow(destination_row, row_items);

    // Get index
    QModelIndex destination_index = parent_item->child(destination_row, 0)->index();

    // Select the item row again
    ui.ClipEditorTree->selectionModel()->clear();
    ui.ClipEditorTree->setCurrentIndex(destination_index);
    ui.ClipEditorTree->selectionModel()->select(destination_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);

    ui.ClipEditorTree->expand(parent_item->index());
}

void ClipEditor::MoveLeft()
{
    MoveHorizontal(true);
}

void ClipEditor::MoveRight()
{
    MoveHorizontal(false);
}

void ClipEditor::MoveHorizontal(bool move_left)
{
    if (!ui.ClipEditorTree->selectionModel()->hasSelection()) {
        return;
    }
    QModelIndexList selected_indexes = ui.ClipEditorTree->selectionModel()->selectedRows(0);
    if (selected_indexes.count() > 1) {
        return;
    }

    // Identify the source information
    QModelIndex source_index = selected_indexes.first();
    int source_row = source_index.row();
    QStandardItem *source_item = m_ClipEditorModel->itemFromIndex(source_index);

    QStandardItem *source_parent_item = source_item->parent();
    if (!source_parent_item) {
        source_parent_item = m_ClipEditorModel->invisibleRootItem();
    }
    
    QStandardItem *destination_parent_item;
    int destination_row = 0;

    if (move_left) {
        // Skip if at root
        if (!source_parent_item) {
            return;
        }

        // Move below parent
        destination_parent_item = source_parent_item->parent();
        if (!destination_parent_item) {
            destination_parent_item = m_ClipEditorModel->invisibleRootItem();
        }
        destination_row = source_parent_item->index().row() + 1;
    }
    else {
        QModelIndex index_above = ui.ClipEditorTree->indexAbove(source_index);

        if (!index_above.isValid()) {
            return;
        }

        QStandardItem *item = m_ClipEditorModel->itemFromIndex(index_above);

        if (source_parent_item == item) {
            return;
        }
        ClipEditorModel::clipEntry *entry = m_ClipEditorModel->GetEntry(item);

        // Only move right if immediately under a group 
        if (entry ->is_group) {
            destination_parent_item = item;
        }
        else {
            // Or if the item above is in a different group
            if (item->parent() && item->parent() != source_parent_item) {
                destination_parent_item = item->parent();
            }
            else {
                return;
            }
        }

        destination_row = destination_parent_item->rowCount();
    }

    // Swap the item rows
    QList<QStandardItem *> row_items = source_parent_item->takeRow(source_row);
    destination_parent_item->insertRow(destination_row, row_items);

    QModelIndex destination_index = destination_parent_item->child(destination_row)->index();

    // Select the item row again
    ui.ClipEditorTree->selectionModel()->clear();
    ui.ClipEditorTree->setCurrentIndex(destination_index);
    ui.ClipEditorTree->selectionModel()->select(destination_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}


void ClipEditor::ConnectSignalsSlots()
{
    connect(ui.FilterText,          SIGNAL(textChanged(QString)), this, SLOT(FilterEditTextChangedSlot(QString)));

    connect(ui.ClipEditorTree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this,                   SLOT(  OpenContextMenu(                  const QPoint&)));

    connect(ui.MoveUp,     SIGNAL(clicked()),            this, SLOT(MoveUp()));
    connect(ui.MoveDown,   SIGNAL(clicked()),            this, SLOT(MoveDown()));
    connect(ui.MoveLeft,   SIGNAL(clicked()),            this, SLOT(MoveLeft()));
    connect(ui.MoveRight,  SIGNAL(clicked()),            this, SLOT(MoveRight()));

    connect(m_AddEntry,    SIGNAL(triggered()), this, SLOT(AddEntry()));
    connect(m_AddGroup,    SIGNAL(triggered()), this, SLOT(AddGroup()));
    connect(m_Edit,        SIGNAL(triggered()), this, SLOT(Edit()));
    connect(m_Cut,         SIGNAL(triggered()), this, SLOT(Cut()));
    connect(m_Copy,        SIGNAL(triggered()), this, SLOT(Copy()));
    connect(m_Paste,       SIGNAL(triggered()), this, SLOT(Paste()));
    connect(m_Delete,      SIGNAL(triggered()), this, SLOT(Delete()));
    connect(m_Import,      SIGNAL(triggered()), this, SLOT(Import()));
    connect(m_Export,      SIGNAL(triggered()), this, SLOT(Export()));
    connect(m_ExportAll,   SIGNAL(triggered()), this, SLOT(ExportAll()));
    connect(m_CollapseAll, SIGNAL(triggered()), this, SLOT(CollapseAll()));
    connect(m_ExpandAll,   SIGNAL(triggered()), this, SLOT(ExpandAll()));

    connect(ui.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(Apply()));
}
