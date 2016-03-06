/************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2013 Dave Heiland
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

#include <QtCore/QStringList>
#include <QtGui/QStandardItem>
#include <QKeyEvent>

#include "BookManipulation/Book.h"
#include "Dialogs/EditTOC.h"
#include "Dialogs/SelectHyperlink.h"
#include "Misc/SettingsStore.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NavProcessor.h"
#include "sigil_constants.h"

static const QString SETTINGS_GROUP   = "edit_toc";
static const int COLUMN_INDENTATION = 20;

EditTOC::EditTOC(QSharedPointer<Book> book, QList<Resource *> resources, QWidget *parent)
    :
    QDialog(parent),
    m_Book(book),
    m_Resources(resources),
    m_TableOfContents(new QStandardItemModel(this)),
    m_ContextMenu(new QMenu(this)),
    m_TOCModel(new TOCModel(this))
{
    // Remove the Nav resource from list of HTMLResources if it exists (EPUB3)
    HTMLResource* nav_resource = m_Book->GetConstOPF()->GetNavResource();
    if (nav_resource) {
        m_Resources.removeOne(nav_resource);
    }

    ui.setupUi(this);
    ui.TOCTree->setContextMenuPolicy(Qt::CustomContextMenu);
    ui.TOCTree->installEventFilter(this);
    ui.TOCTree->setModel(m_TableOfContents);
    ui.TOCTree->setIndentation(COLUMN_INDENTATION);
    CreateContextMenuActions();
    ConnectSignalsToSlots();

    CreateTOCModel();
    UpdateTreeViewDisplay();
    ReadSettings();
}

EditTOC::~EditTOC()
{
    WriteSettings();
}

void EditTOC::UpdateTreeViewDisplay()
{
    ui.TOCTree->expandAll();
}

void EditTOC::CreateTOCModel()
{
    m_TOCModel->SetBook(m_Book);

    TOCModel::TOCEntry toc_entry = m_TOCModel->GetRootTOCEntry();

    m_TableOfContents->clear();
    QStringList header;
    header.append(tr("TOC Entry"));
    header.append(tr("Target"));
    m_TableOfContents->setHorizontalHeaderLabels(header);

    BuildModel(toc_entry);
}

void EditTOC::Save()
{
    QString version = m_Book->GetConstOPF()->GetEpubVersion();
    if (version.startsWith('3')) {
        NavProcessor navproc(m_Book->GetConstOPF()->GetNavResource());
        navproc.GenerateNavTOCFromTOCEntries(ConvertTableToEntries());
    } else {
        m_Book->GetNCX()->GenerateNCXFromTOCEntries(m_Book.data(), ConvertTableToEntries());
    }
}

TOCModel::TOCEntry EditTOC::ConvertTableToEntries()
{
    return ConvertItemToEntry(m_TableOfContents->invisibleRootItem());
}

TOCModel::TOCEntry EditTOC::ConvertItemToEntry(QStandardItem *item)
{
    TOCModel::TOCEntry entry;

    if (item != m_TableOfContents->invisibleRootItem()) {
        entry.text = item->text();
        QStandardItem *parent_item = item->parent();
        if (!parent_item) {
            parent_item = m_TableOfContents->invisibleRootItem();
        }
        entry.target = parent_item->child(item->row(), 1)->text();
    } else {
        entry.is_root = true;
    }

    if (!item->hasChildren()) {
        return entry;
    }

    for (int row = 0; row < item->rowCount(); row++) {
        entry.children.append(ConvertItemToEntry(item->child(row, 0)));
    }
    return entry;
}

void EditTOC::ExpandChildren(QStandardItem *item)
{
    QModelIndexList indexes;

    if (item->hasChildren()) {
        for (int i = 0; i < item->rowCount(); i++) {
            ExpandChildren(item->child(i, 0));
        }
    }

    ui.TOCTree->expand(item->index());
}

void EditTOC::MoveLeft()
{
    QModelIndex index = CheckSelection(0);
    if (!index.isValid()) {
        return;
    }

    QStandardItem *item = m_TableOfContents->itemFromIndex(index);

    QStandardItem *parent_item = item->parent();
    // Can't indent above top level
    if (!parent_item) {
        return;
    }

    QStandardItem *grandparent_item = parent_item->parent();
    if (!grandparent_item) {
        grandparent_item = m_TableOfContents->invisibleRootItem();
    }

    // Make siblings following the entry into children
    int row = item->row() ;
    while (row + 1 < parent_item->rowCount()) {
        QList<QStandardItem *> row_items = parent_item->takeRow(row + 1);
        int row_count = item->rowCount();
        item->setChild(row_count, 0, row_items[0]);
        item->setChild(row_count, 1, row_items[1]);
    }

    // Make item child of grandparent
    int parent_row = parent_item->row();
    QList<QStandardItem *> row_items = parent_item->takeRow(row);
    grandparent_item->insertRow(parent_row + 1, row_items);

    // Reselect the item
    QModelIndex item_index = grandparent_item->child(parent_row + 1)->index();
    ui.TOCTree->selectionModel()->clear();
    ui.TOCTree->setCurrentIndex(item_index);
    ui.TOCTree->selectionModel()->select(item_index, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    ExpandChildren(grandparent_item->child(parent_row + 1));
}

void EditTOC::MoveRight()
{
    QModelIndex index = CheckSelection(0);
    if (!index.isValid()) {
        return;
    }

    QStandardItem *item = m_TableOfContents->itemFromIndex(index);
    int item_row = item->row();

    // Can't indent if row above is already parent
    if (item_row == 0) {
        return;
    }

    QStandardItem *parent_item = item->parent();
    if (!parent_item) {
        parent_item = m_TableOfContents->invisibleRootItem();
    }

    // Make the item above the parent of this item
    QList<QStandardItem *> row_items = parent_item->takeRow(item_row);
    QStandardItem *new_parent = parent_item->child(item_row - 1, 0);
    new_parent->insertRow(new_parent->rowCount(), row_items);
    QStandardItem *new_item = new_parent->child(new_parent->rowCount() - 1, 0);

    // Reselect the item
    ui.TOCTree->selectionModel()->clear();
    ui.TOCTree->setCurrentIndex(item->index());
    ui.TOCTree->selectionModel()->select(item->index(), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    ExpandChildren(new_item);
}

void EditTOC::MoveUp()
{
    QModelIndex index = CheckSelection(0);
    if (!index.isValid()) {
        return;
    }
    QStandardItem *item = m_TableOfContents->itemFromIndex(index);

    int item_row = item->row();

    // Can't move up if this row is already the top most one of its parent
    if (item_row == 0) {
        return;
    }

    QStandardItem *parent_item = item->parent();
    if (!parent_item) {
        parent_item = m_TableOfContents->invisibleRootItem();
    }

    QList<QStandardItem *> row_items = parent_item->takeRow(item_row);
    parent_item->insertRow(item_row - 1, row_items);

    // Reselect the item
    ui.TOCTree->selectionModel()->clear();
    ui.TOCTree->setCurrentIndex(item->index());
    ui.TOCTree->selectionModel()->select(item->index(), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    ExpandChildren(item);
}

void EditTOC::MoveDown()
{
    QModelIndex index = CheckSelection(0);
    if (!index.isValid()) {
        return;
    }
    QStandardItem *item = m_TableOfContents->itemFromIndex(index);

    QStandardItem *parent_item = item->parent();
    if (!parent_item) {
        parent_item = m_TableOfContents->invisibleRootItem();
    }

    int item_row = item->row();

    // Can't move down if this row is already the last one of its parent
    if (item_row == parent_item->rowCount() - 1) {
        return;
    }

    QList<QStandardItem *> row_items = parent_item->takeRow(item_row);
    parent_item->insertRow(item_row + 1, row_items);

    // Reselect the item
    ui.TOCTree->selectionModel()->clear();
    ui.TOCTree->setCurrentIndex(item->index());
    ui.TOCTree->selectionModel()->select(item->index(), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    ExpandChildren(item);
}

void EditTOC::AddEntryAbove()
{
    AddEntry(true);
}

void EditTOC::AddEntryBelow()
{
    AddEntry(false);
}

void EditTOC::AddEntry(bool above)
{
    QModelIndex index = CheckSelection(0);
    if (!index.isValid()) {
        return;
    }

    QStandardItem *item = m_TableOfContents->itemFromIndex(index);

    QStandardItem *parent_item = item->parent();
    if (!parent_item) {
        parent_item = m_TableOfContents->invisibleRootItem();
    }

    // Add a new empty row of items
    QStandardItem *entry_item = new QStandardItem();
    QStandardItem *target_item = new QStandardItem();
    QList<QStandardItem *> row_items;
    row_items << entry_item << target_item ;
    int location = 1;
    if (above) {
        location = 0;
    }
    parent_item->insertRow(item->row() + location,row_items);

    // Select the new row
    ui.TOCTree->selectionModel()->clear();
    ui.TOCTree->setCurrentIndex(entry_item->index());
    ui.TOCTree->selectionModel()->select(entry_item->index(), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
}

QModelIndex EditTOC::CheckSelection(int row)
{
    if (!ui.TOCTree->selectionModel()->hasSelection()) {
        return QModelIndex();
    }

    QModelIndexList selected_indexes = ui.TOCTree->selectionModel()->selectedRows(row);

    if (selected_indexes.count() != 1) {
        return QModelIndex();
    }

    return selected_indexes.first();
}

void EditTOC::DeleteEntry()
{
    QModelIndex index = CheckSelection(0);
    if (!index.isValid()) {
        return;
    }

    QStandardItem *item = m_TableOfContents->itemFromIndex(index);

    QStandardItem *parent_item = item->parent();
    if (!parent_item) {
        parent_item = m_TableOfContents->invisibleRootItem();
    }

    parent_item->takeRow(item->row());
}

void EditTOC::SelectTarget()
{
    QModelIndex index = CheckSelection(1);
    if (!index.isValid()) {
        return;
    }

    QStandardItem *item = m_TableOfContents->itemFromIndex(index);

    SelectHyperlink select_target(item->text(), NULL, m_Resources, m_Book, this);

    if (select_target.exec() == QDialog::Accepted) {
        item->setText(select_target.GetTarget());
    }
}

void EditTOC::ReadSettings()
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

    for (int column = 0; column < size && column < ui.TOCTree->header()->count(); column++) {
        settings.setArrayIndex(column);
        int column_width = settings.value("width").toInt();

        if (column_width) {
            ui.TOCTree->setColumnWidth(column, column_width);
        }
    }
    settings.endArray();

    settings.endGroup();
}

void EditTOC::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    // Column widths
    settings.beginWriteArray("column_data");

    for (int column = 0; column < ui.TOCTree->header()->count(); column++) {
        settings.setArrayIndex(column);
        settings.setValue("width", ui.TOCTree->columnWidth(column));
    }

    settings.endArray();

    settings.endGroup();
}

void EditTOC::Rename()
{
    if (!ui.TOCTree->selectionModel()->hasSelection()) {
        return;
    }

    if (ui.TOCTree->selectionModel()->selectedRows(0).count() != 1) {
        return;
    }

    ui.TOCTree->edit(ui.TOCTree->currentIndex());
}

void EditTOC::CollapseAll()
{
    ui.TOCTree->collapseAll();
}

void EditTOC::ExpandAll()
{
    ui.TOCTree->expandAll();
}

void EditTOC::CreateContextMenuActions()
{
    m_Rename = new QAction(tr("Rename"),     this);
    m_Delete = new QAction(tr("Delete"),     this);
    m_Rename->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_R));
    m_Delete->setShortcut(QKeySequence::Delete);
    // Has to be added to the dialog itself for the keyboard shortcut to work.
    addAction(m_Rename);
    addAction(m_Delete);

    m_MoveUp = new QAction(tr("Move Up"),       this);
    m_MoveDown = new QAction(tr("Move Down"),   this);
    m_MoveUp->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Up));
    m_MoveDown->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Down));
    addAction(m_MoveUp);
    addAction(m_MoveDown);

    m_ExpandAll= new QAction(tr("Expand All"),     this);
    m_CollapseAll = new QAction(tr("Collapse All"),  this);
}

void EditTOC::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);
    m_ContextMenu->exec(ui.TOCTree->viewport()->mapToGlobal(point));
    m_ContextMenu->clear();
}

void EditTOC::SetupContextMenu(const QPoint &point)
{
    m_ContextMenu->addAction(m_Rename);
    m_ContextMenu->addAction(m_Delete);
    m_ContextMenu->addSeparator();
    m_ContextMenu->addAction(m_CollapseAll);
    m_ContextMenu->addAction(m_ExpandAll);
}

bool EditTOC::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui.TOCTree) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            int key = keyEvent->key();

            if (key == Qt::Key_Left) {
                MoveLeft();
                return true;
            } else if (key == Qt::Key_Right) {
                MoveRight();
                return true;
            }
        }
    }

    // pass the event on to the parent class
    return QDialog::eventFilter(obj, event);
}

void EditTOC::BuildModel(const TOCModel::TOCEntry &root_entry)
{
    foreach(const TOCModel::TOCEntry& child_entry, root_entry.children) {
        AddEntryToParentItem(child_entry, m_TableOfContents->invisibleRootItem(), 1);
    }
}

void EditTOC::AddEntryToParentItem(const TOCModel::TOCEntry &entry, QStandardItem *parent, int level)
{
    Q_ASSERT(parent);
    QStandardItem *entry_item = new QStandardItem(entry.text);
    QStandardItem *target_item = new QStandardItem(entry.target);

    QList<QStandardItem *> row_items;
    row_items << entry_item << target_item ;
    parent->appendRow(row_items);

    foreach(const TOCModel::TOCEntry &child_entry, entry.children) {
        AddEntryToParentItem(child_entry, entry_item, level + 1);
    }
}

void EditTOC::ConnectSignalsToSlots()
{
    connect(this,               SIGNAL(accepted()),           this, SLOT(Save()));
    connect(ui.AddEntryAbove,   SIGNAL(clicked()),            this, SLOT(AddEntryAbove()));
    connect(ui.AddEntryBelow,   SIGNAL(clicked()),            this, SLOT(AddEntryBelow()));
    connect(ui.DeleteEntry,     SIGNAL(clicked()),            this, SLOT(DeleteEntry()));
    connect(ui.MoveLeft,        SIGNAL(clicked()),            this, SLOT(MoveLeft()));
    connect(ui.MoveRight,       SIGNAL(clicked()),            this, SLOT(MoveRight()));
    connect(ui.MoveUp,          SIGNAL(clicked()),            this, SLOT(MoveUp()));
    connect(ui.MoveDown,        SIGNAL(clicked()),            this, SLOT(MoveDown()));
    connect(m_MoveUp,           SIGNAL(triggered()),          this, SLOT(MoveUp()));
    connect(m_MoveDown,         SIGNAL(triggered()),          this, SLOT(MoveDown()));
    connect(ui.SelectTarget,    SIGNAL(clicked()),            this, SLOT(SelectTarget()));
    connect(ui.TOCTree,         SIGNAL(customContextMenuRequested(const QPoint &)),
            this,               SLOT(OpenContextMenu(const QPoint &)));
    connect(m_Rename,           SIGNAL(triggered()), this, SLOT(Rename()));
    connect(m_Delete,           SIGNAL(triggered()), this, SLOT(DeleteEntry()));
    connect(m_CollapseAll,      SIGNAL(triggered()), this, SLOT(CollapseAll()));
    connect(m_ExpandAll,        SIGNAL(triggered()), this, SLOT(ExpandAll()));
}
