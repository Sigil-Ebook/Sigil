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
#include <QtGui/QProgressDialog>
#include <QtGui/QMessageBox>

#include "Dialogs/IndexEditor.h"
#include "MiscEditors/IndexEditorTreeView.h"
#include "Misc/Utility.h"
#include "ResourceObjects/Resource.h"

static const QString SETTINGS_GROUP = "index_editor";
static const QString FILE_EXTENSION = "ini";

IndexEditor::IndexEditor(QWidget *parent)
    : 
    QDialog(parent),
    m_Book(NULL),
    m_LastFolderOpen(QString()),
    m_ContextMenu(new QMenu(this))
{
    ui.setupUi(this);	

    SetupIndexEditorTree();

    CreateContextMenuActions();

    ConnectSignalsSlots();
} 

IndexEditor::~IndexEditor()
{
    // Restore data from file since we aren't saving
    m_IndexEditorModel->LoadInitialData();
}

void IndexEditor::SetBook(QSharedPointer <Book> book)
{
    m_Book = book;
}

void IndexEditor::SetupIndexEditorTree()
{
    m_IndexEditorModel = IndexEditorModel::instance();

    ui.IndexEditorTree->setModel(m_IndexEditorModel);
    ui.IndexEditorTree->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.IndexEditorTree->setSortingEnabled(true);
    ui.IndexEditorTree->sortByColumn(0, Qt::AscendingOrder);
    ui.IndexEditorTree->setWordWrap(true);
    ui.IndexEditorTree->setAlternatingRowColors(true); 

    ui.IndexEditorTree->header()->setToolTip(
        "<p>" + tr("Right click on an entry to see a context menu of actions.") + "</p>" +
        "<p>" + tr("You can also right click in Code View to add selected text to the Index.") + "</p>" +
        "<dl>" +
        "<dt><b>" + tr("Text to Include") + "</b><dd>" + tr("The text to match in your document, e.g. 'Gutenberg'. You can list similar patterns by separating them with ';', e.g. 'Gutenberg;gutenberg'.  The text to match can be a regex pattern, e.g. '[gG]utenberg'.") + "</dd>" +
        "<dt><b>" + tr("Index Entries") + "</b><dd>" + tr("The entry to create in the Index. Leave blank to use text as is, or enter text to display.  Create multiple entries by separating entries by ';'.  Create multi-level entries by using '/' after a level name, e.g. 'Books/Fantasy/Alice in Wonderland;Characters/Alice") + "</dd>" +
        "</dl>");

    ui.IndexEditorTree->header()->setStretchLastSection(true);
    for (int column = 0; column < ui.IndexEditorTree->header()->count(); column++) {
        ui.IndexEditorTree->resizeColumnToContents(column);
    }
}

bool IndexEditor::SaveData(QList<IndexEditorModel::indexEntry*> entries, QString filename)
{
    QString message = m_IndexEditorModel->SaveData(entries, filename);

    if (!message.isEmpty()) {
        Utility::DisplayStdErrorDialog(tr("Cannot save entries.") + "\n\n" + message);
    }
    return message.isEmpty();
}

void IndexEditor::CreateIndex()
{
    emit CreateIndexRequest();
}

void IndexEditor::showEvent(QShowEvent *event)
{
    ReadSettings();

    ui.FilterText->clear();
    ui.FilterText->setFocus();
}

int IndexEditor::SelectedRowsCount()
{
    int count = 0;
    if (ui.IndexEditorTree->selectionModel()->hasSelection()) {
        count = ui.IndexEditorTree->selectionModel()->selectedRows(0).count();
    }

    return count;
}

QList<IndexEditorModel::indexEntry*> IndexEditor::GetSelectedEntries()
{
    QList<IndexEditorModel::indexEntry *> selected_entries;

    if (ui.IndexEditorTree->selectionModel()->hasSelection()) {
        selected_entries = m_IndexEditorModel->GetEntries(GetSelectedItems());
    }

    return selected_entries;
}

QList<QStandardItem*> IndexEditor::GetSelectedItems()
{
    // Shift-click order is top to bottom regardless of starting position
    // Ctrl-click order is first clicked to last clicked (included shift-clicks stay ordered as is)

    QModelIndexList selected_indexes = ui.IndexEditorTree->selectionModel()->selectedRows(0);
    QList<QStandardItem*> selected_items;

    foreach (QModelIndex index, selected_indexes) {
        selected_items.append(m_IndexEditorModel->itemFromIndex(index));
    }

    return selected_items;
}

QStandardItem* IndexEditor::AddEntry(bool is_group, IndexEditorModel::indexEntry *index_entry, bool insert_after)
{
    QStandardItem *parent_item = NULL;
    QStandardItem *new_item = NULL;
    int row = 0;

    // If adding a new/blank entry add it after the selected entry.
    if (insert_after) {
        if (ui.IndexEditorTree->selectionModel()->hasSelection()) {
            parent_item = GetSelectedItems().last();
            if (!parent_item) {
                return parent_item;
            }

            row = parent_item->row() + 1;
            parent_item = parent_item->parent();
        }
    }

    new_item = m_IndexEditorModel->AddEntryToModel(index_entry, false, parent_item, row);
    QModelIndex new_index = new_item->index();

    // Select the added item and set it for editing
    ui.IndexEditorTree->selectionModel()->clear();
    ui.IndexEditorTree->selectionModel()->select(new_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    ui.IndexEditorTree->setCurrentIndex(new_index);
    ui.IndexEditorTree->selectionModel()->select(new_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    ui.IndexEditorTree->edit(new_index);
    ui.IndexEditorTree->setCurrentIndex(new_index);
    ui.IndexEditorTree->selectionModel()->select(new_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);

    return new_item;
}

void IndexEditor::Cut()
{
    if (Copy()) {
        Delete();
    }
}

bool IndexEditor::Copy()
{
    if (SelectedRowsCount() < 1) {
        return false;
    }

    while (m_SavedIndexEntries.count()) {
        m_SavedIndexEntries.removeAt(0);
    }

    foreach (IndexEditorModel::indexEntry *entry, GetSelectedEntries()) {
        IndexEditorModel::indexEntry *save_entry = new IndexEditorModel::indexEntry();
        save_entry->pattern = entry->pattern;
        save_entry->index_entry = entry->index_entry;
        m_SavedIndexEntries.append(save_entry);
    }

    return true;
}

void IndexEditor::Paste()
{
    foreach (IndexEditorModel::indexEntry *entry, m_SavedIndexEntries) {
        AddEntry(false, entry);
    }
}

void IndexEditor::Delete()
{
    if (SelectedRowsCount() < 1) {
        return;
    }

    // Display progress dialog
    QProgressDialog progress(QObject::tr("Deleting entries..."), tr("Cancel"), 0, SelectedRowsCount(), this);
    progress.setMinimumDuration(1500);
    int progress_value = 0;

    // Deleting all entries can be done much quicker than deleting one by one.
    if (SelectedRowsCount() == m_IndexEditorModel->invisibleRootItem()->rowCount()) {
        m_IndexEditorModel->ClearData();
    }

    // Delete one at a time as selection may not be contiguous
    // Could be faster if dialog is hidden during delete.
    int row = -1;
    while (ui.IndexEditorTree->selectionModel()->hasSelection()) {
        // Set progress value and ensure dialog has time to display when doing extensive updates
        if (progress.wasCanceled()) {
            break;
        }
        progress.setValue(progress_value++);
        QApplication::processEvents();

        QModelIndex index = ui.IndexEditorTree->selectionModel()->selectedRows(0).first();
        if (index.isValid()) {
            row = index.row();
            m_IndexEditorModel->removeRows(row, 1, index.parent());
        }
    }

    int row_count = m_IndexEditorModel->invisibleRootItem()->rowCount();
    if (row_count && row >= 0) {
        if (row >= row_count) {
            row = row_count - 1;
        }
        QModelIndex select_index = m_IndexEditorModel->index(row, 0);
        ui.IndexEditorTree->setCurrentIndex(select_index);
        ui.IndexEditorTree->selectionModel()->select(select_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    }
}

void IndexEditor::AutoFill()
{
    if (SelectedRowsCount() > 1 || !m_Book) {
        return;
    }

    QSet<QString> unique_words = m_Book->GetUniqueWordsInAllHTML();

    // Display progress dialog
    QProgressDialog progress(QObject::tr("Automatically Filling Index..."), tr("Cancel"), 0, unique_words.count(), this);
    progress.setMinimumDuration(1500);
    int progress_value = 0;

    // Temporarily hide the Index dialog to significantly speed up adding entries by reducing gui updates
    hide();

    int count = 0;
    foreach(QString word, unique_words) {
        // Set progress value and ensure dialog has time to display when doing extensive updates
        progress.setValue(progress_value++);
        if (progress.wasCanceled()) {
            break;
        }
        QApplication::processEvents();

        IndexEditorModel::indexEntry *entry = new IndexEditorModel::indexEntry();
        entry->pattern = word;
        entry->index_entry = "";
        m_IndexEditorModel->AddEntryToModel(entry);
        count++;
    }

    ui.IndexEditorTree->sortByColumn(0, Qt::AscendingOrder);

    // Cancel the progress dialog since the information dialog will keep it open
    progress.cancel();

    show();

    QMessageBox::information(this, tr("Sigil"), tr("Added %1 entries.").arg(QString::number(count)));
}

void IndexEditor::Open()
{
    // Get the filename to open
    QString filter_string = "*." % FILE_EXTENSION;

    QString filename = QFileDialog::getOpenFileName(this,
                                                    tr("Load Entries From File"),
                                                    m_LastFolderOpen,
                                                    filter_string
                                                   );

    // Load the file and save the last folder opened
    if (!filename.isEmpty()) {
        m_IndexEditorModel->LoadInitialData(filename);

        m_LastFolderOpen = QFileInfo(filename).absolutePath();
        WriteSettings();
    }
}

void IndexEditor::SelectAll()
{
    ui.IndexEditorTree->selectAll();
}

void IndexEditor::SaveAs()
{
    // Get the filename to use
    QString filter_string = "*." % FILE_EXTENSION;
    QString default_filter = "*";

    QString filename = QFileDialog::getSaveFileName(this,
                                                    tr("Save Entries to File"),
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
    QList<IndexEditorModel::indexEntry *> entries;
    ui.IndexEditorTree->selectAll();
    entries = GetSelectedEntries();

    if (SaveData(entries, filename)) {
        m_LastFolderOpen = QFileInfo(filename).absolutePath();
        WriteSettings();
    }
}

void IndexEditor::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();
    QModelIndex root_index = m_IndexEditorModel->indexFromItem(m_IndexEditorModel->invisibleRootItem());

    for (int row = 0; row < m_IndexEditorModel->invisibleRootItem()->rowCount(); row++) {
        QStandardItem *item = m_IndexEditorModel->item(row, 0);
        IndexEditorModel::indexEntry *entry = m_IndexEditorModel->GetEntry(item);
        bool hidden = !(text.isEmpty() || entry->pattern.toLower().contains(lowercaseText) ||
                      entry->index_entry.toLower().contains(lowercaseText));
        ui.IndexEditorTree->setRowHidden(item->row(), root_index, hidden);
    }
}

void IndexEditor::ReadSettings()
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
    for (int column = 0; column < size && column < ui.IndexEditorTree->header()->count(); column++) {
        settings.setArrayIndex(column);
        int column_width = settings.value("width").toInt();
        if (column_width) {
            ui.IndexEditorTree->setColumnWidth(column, column_width);
        }
    }
    settings.endArray();

    // Last folder open
    m_LastFolderOpen = settings.value("last_folder_open").toString();

    settings.endGroup();
}

void IndexEditor::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    // Column widths
    settings.beginWriteArray("column_data");
    for (int column = 0; column < ui.IndexEditorTree->header()->count(); column++) {
        settings.setArrayIndex(column);
        settings.setValue("width", ui.IndexEditorTree->columnWidth(column));
    }
    settings.endArray();

    // Last folder open
    settings.setValue("last_folder_open", m_LastFolderOpen);

    settings.endGroup();
}

void IndexEditor::CreateContextMenuActions()
{
    m_AddEntry  = new QAction(tr("Add Entry"),  this);
    m_Cut       = new QAction(tr("Cut"),        this);
    m_Copy      = new QAction(tr("Copy"),       this);
    m_Paste     = new QAction(tr("Paste"),      this);
    m_Delete    = new QAction(tr("Delete"),     this);
    m_AutoFill  = new QAction(tr("Auto Fill"),  this);
    m_Open      = new QAction(tr("Open"),       this);
    m_SaveAs    = new QAction(tr("Save As"),    this);
    m_SelectAll = new QAction(tr("Select All"), this);

    m_AddEntry->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_E));
    m_Cut->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_X));
    m_Copy->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_C));
    m_Paste->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_V));
    m_Delete->setShortcut(QKeySequence::Delete);

    // Has to be added to the dialog itself for the keyboard shortcut to work.
    addAction(m_AddEntry);
    addAction(m_Cut);
    addAction(m_Copy);
    addAction(m_Paste);
    addAction(m_Delete);

}

void IndexEditor::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);

    m_ContextMenu->exec(ui.IndexEditorTree->viewport()->mapToGlobal(point));
    m_ContextMenu->clear();

    // Make sure every action is enabled - in case shortcut is used after context menu disables some.
    m_AddEntry->setEnabled(true);
    m_Cut->setEnabled(true);
    m_Copy->setEnabled(true);
    m_Paste->setEnabled(true);
    m_Delete->setEnabled(true);
    m_AutoFill->setEnabled(true);
    m_Open->setEnabled(true);
    m_SaveAs->setEnabled(true);
    m_SelectAll->setEnabled(true);
}

void IndexEditor::SetupContextMenu(const QPoint &point)
{
    int selected_rows_count = SelectedRowsCount();
    int total_row_count = m_IndexEditorModel->invisibleRootItem()->rowCount();

    m_ContextMenu->addAction(m_AddEntry);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Cut);
    m_Cut->setEnabled(selected_rows_count > 0); 

    m_ContextMenu->addAction(m_Copy);
    m_Copy->setEnabled(selected_rows_count > 0);

    m_ContextMenu->addAction(m_Paste);
    m_Paste->setEnabled(m_SavedIndexEntries.count());

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Delete);
    m_Delete->setEnabled(selected_rows_count > 0);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_Open);

    m_ContextMenu->addAction(m_SaveAs);
    m_SaveAs->setEnabled(total_row_count > 0);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_AutoFill);
    m_AutoFill->setEnabled(selected_rows_count <= 1);

    m_ContextMenu->addSeparator();

    m_ContextMenu->addAction(m_SelectAll);
    bool show_select_all = total_row_count && total_row_count != selected_rows_count;
    if (show_select_all) {
        m_ContextMenu->addSeparator();
    }
    m_SelectAll->setEnabled(show_select_all);
}

void IndexEditor::reject()
{
    m_IndexEditorModel->LoadInitialData();
    QDialog::reject();
}

void IndexEditor::accept()
{ 
    if (SaveData()) {
        WriteSettings();
        QDialog::accept();
    }
}

void IndexEditor::ConnectSignalsSlots()
{
    connect(ui.FilterText,  SIGNAL(textChanged(QString)), this, SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.CreateIndex, SIGNAL(clicked()),            this, SLOT(CreateIndex()));

    connect(ui.IndexEditorTree, SIGNAL(customContextMenuRequested(const QPoint&)),
            this,        SLOT(  OpenContextMenu(                  const QPoint&)));

    connect(m_AddEntry,  SIGNAL(triggered()), this, SLOT(AddEntry()));
    connect(m_Cut,       SIGNAL(triggered()), this, SLOT(Cut()));
    connect(m_Copy,      SIGNAL(triggered()), this, SLOT(Copy()));
    connect(m_Paste,     SIGNAL(triggered()), this, SLOT(Paste()));
    connect(m_Delete,    SIGNAL(triggered()), this, SLOT(Delete()));
    connect(m_AutoFill,  SIGNAL(triggered()), this, SLOT(AutoFill()));
    connect(m_Open,      SIGNAL(triggered()), this, SLOT(Open()));
    connect(m_SaveAs,    SIGNAL(triggered()), this, SLOT(SaveAs()));
    connect(m_SelectAll, SIGNAL(triggered()), this, SLOT(SelectAll()));
}
