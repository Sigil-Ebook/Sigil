/************************************************************************
**
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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

#include <QtCore/QFile>
#include <QtGui/QFont>
#include <QtGui/QMessageBox>

#include "ResourceObjects/HTMLResource.h"
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "CSSFilesWidget.h"

static const int THUMBNAIL_SIZE = 100;
static const int THUMBNAIL_SIZE_INCREMENT = 50;

static QString SETTINGS_GROUP = "reports_css_files";

CSSFilesWidget::CSSFilesWidget(QList<Resource*> html_resources, QList<Resource*> css_resources, QSharedPointer<Book> book)
    :
    m_HTMLResources(html_resources),
    m_CSSResources(css_resources),
    m_Book(book),
    m_ItemModel(new QStandardItemModel),
    m_ContextMenu(new QMenu(this)),
    m_DeleteFiles(false)

{
    ui.setupUi(this);

    ui.fileTree->setContextMenuPolicy(Qt::CustomContextMenu);

    CreateContextMenuActions();

    connectSignalsSlots();

    SetupTable();
}

void CSSFilesWidget::SetupTable(int sort_column, Qt::SortOrder sort_order)
{
    m_ItemModel->clear();

    QStringList header;

    header.append(tr("Name" ));
    header.append(tr("Size (KB)" ));
    header.append(tr("Times Used" ));

    m_ItemModel->setHorizontalHeaderLabels(header);

    ui.fileTree->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui.fileTree->setModel(m_ItemModel);

    ui.fileTree->header()->setSortIndicatorShown(true);

    // Get all a count of all the linked stylesheets
    QHash<QString, int> linked_stylesheets_hash;
    foreach (Resource *html_resource, m_HTMLResources) {
        QString html_filename = html_resource->Filename();

        // Get the linked stylesheets for this file
        HTMLResource *html_type_resource = dynamic_cast<HTMLResource *>( html_resource );
        QStringList linked_stylesheets = m_Book->GetStylesheetsInHTMLFile(html_type_resource);
        foreach (QString stylesheet, linked_stylesheets) {
            if (linked_stylesheets.contains(stylesheet)) {
                linked_stylesheets_hash[stylesheet]++;
            }
            else {
                linked_stylesheets_hash[stylesheet] = 1;
            }
        }
    }

    double total_size = 0;
    int total_links = 0;

    foreach (Resource *resource, m_CSSResources) {
            QString filepath = "../" + resource->GetRelativePathToOEBPS();
            QString path = resource->GetFullPath();

            QList<QStandardItem *> rowItems;

            // Filename
            QStandardItem *name_item = new QStandardItem();
            name_item->setText(resource->Filename());
            name_item->setToolTip(filepath);
            rowItems << name_item;

            // File Size
            double ffsize = QFile(path).size() / 1024.0;
            total_size += ffsize;
            QString fsize = QLocale().toString(ffsize, 'f', 2);
            NumericItem *size_item = new NumericItem();
            size_item->setText(fsize);
            rowItems << size_item;

            // Times Used
            int count = 0;
            if (linked_stylesheets_hash.contains(filepath)) {
                count = linked_stylesheets_hash[filepath];
            }
            total_links += count;
            NumericItem *link_item = new NumericItem();
            link_item->setText(QString::number(count));
            rowItems << link_item;

            for (int i = 0; i < rowItems.count(); i++) {
                rowItems[i]->setEditable(false);
            }
            m_ItemModel->appendRow(rowItems);
    }

    // Sort before adding the totals row
    // Since sortIndicator calls this routine, must disconnect/reconnect while resorting
    disconnect (ui.fileTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    ui.fileTree->header()->setSortIndicator(sort_column, sort_order);
    connect (ui.fileTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));

    // Totals
    NumericItem *nitem;
    QList<QStandardItem *> rowItems;

    // Files
    nitem = new NumericItem();
    nitem->setText(QString::number(m_CSSResources.count()) % tr(" files"));
    rowItems << nitem;

    // File size
    nitem = new NumericItem();
    nitem->setText(QLocale().toString(total_size, 'f', 2) % tr("KB"));
    rowItems << nitem;

    // Links - leave blank
    nitem = new NumericItem();
    nitem->setText("");
    rowItems << nitem;

    // Add the row in bold
    QFont font = *new QFont();
    font.setWeight(QFont::Bold);
    for (int i = 0; i < rowItems.count(); i++) {
        rowItems[i]->setEditable(false);
        rowItems[i]->setFont(font);
    }
    m_ItemModel->appendRow(rowItems);

    for (int i = 0; i < ui.fileTree->header()->count(); i++) {
        ui.fileTree->resizeColumnToContents(i);
    }
}

void CSSFilesWidget::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();

    QStandardItem *root_item = m_ItemModel->invisibleRootItem();
    QModelIndex parent_index;

    // Hide rows that don't contain the filter text
    int first_visible_row = -1;
    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText)) {
            ui.fileTree->setRowHidden(row, parent_index, false);
            if (first_visible_row == -1) {
                first_visible_row = row;
            }
        }
        else {
            ui.fileTree->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.fileTree->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    }
    else {
        // Clear current and selection, which clears preview image
        ui.fileTree->setCurrentIndex(QModelIndex());
    }
}

void CSSFilesWidget::Sort(int logicalindex, Qt::SortOrder order)
{
    SetupTable(logicalindex, order);
}

ReportsWidget::Results CSSFilesWidget::saveSettings()
{
    ReportsWidget::Results results;

    results.filename = "";
    results.line = -1;
    results.files_to_delete.clear();
    results.styles_to_delete.clear();

    if (ui.fileTree->selectionModel()->hasSelection()) {
        if (m_DeleteFiles) {
            foreach (QModelIndex index, ui.fileTree->selectionModel()->selectedRows(0)) {
                results.files_to_delete.append(m_ItemModel->itemFromIndex(index)->text());
            }
        }
        else {
            QModelIndex index = ui.fileTree->selectionModel()->selectedRows(0).first();
            if (index.row() != m_ItemModel->rowCount() - 1) {
                results.filename = m_ItemModel->itemFromIndex(index)->text();
            }
        }
    }

    return results;
}

void CSSFilesWidget::Delete()
{
    QMessageBox::StandardButton button_pressed;
    button_pressed = QMessageBox::warning(  this,
                      tr( "Sigil" ), tr( "Are you sure you want to delete the selected files from the Book?") % "\n" % tr( "This action cannot be reversed." ),
                                                QMessageBox::Ok | QMessageBox::Cancel
                                         );

    if ( button_pressed == QMessageBox::Ok ) {
        m_DeleteFiles = true;
        emit Done();
    }
}

void CSSFilesWidget::CreateContextMenuActions()
{
    m_Delete    = new QAction(tr("Delete From Book") + "...",     this);

    m_Delete->setShortcut(QKeySequence::Delete);

    // Has to be added to the dialog itself for the keyboard shortcut to work.
    addAction(m_Delete);
}

void CSSFilesWidget::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);

    m_ContextMenu->exec(ui.fileTree->viewport()->mapToGlobal(point));
    m_ContextMenu->clear();
}

void CSSFilesWidget::SetupContextMenu(const QPoint &point)
{
    m_ContextMenu->addAction(m_Delete);
}

void CSSFilesWidget::connectSignalsSlots()
{
    connect(ui.leFilter,             SIGNAL(textChanged(QString)),
            this,                    SLOT(FilterEditTextChangedSlot(QString)));
    connect (ui.fileTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), 
            this,                    SLOT(Sort(int, Qt::SortOrder)));
    connect (ui.fileTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SIGNAL(Done()));

    connect(ui.fileTree,  SIGNAL(customContextMenuRequested(const QPoint&)),
            this,         SLOT(  OpenContextMenu(                  const QPoint&)));
    connect(m_Delete,     SIGNAL(triggered()), this, SLOT(Delete()));

}
