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

#include <QtCore/QFile>
#include <QtCore/QHashIterator>
#include <QtGui/QFont>

#include "Misc/NumericItem.h"
#include "Misc/CSSInfo.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"
#include "Misc/SettingsStore.h"
#include "ClassesInHTMLFilesWidget.h"

static QString SETTINGS_GROUP = "reports_classes_in_html_files";

ClassesInHTMLFilesWidget::ClassesInHTMLFilesWidget(QList<Resource *>html_resources, QList<Resource *>css_resources, QSharedPointer< Book > book)
    :
    m_HTMLResources(html_resources),
    m_CSSResources(css_resources),
    m_Book(book),
    m_ItemModel(new QStandardItemModel)
{
    ui.setupUi(this);
    connectSignalsSlots();

    SetupTable();

    QList<BookReports::StyleData *> html_classes_usage = BookReports::GetHTMLClassUsage(m_HTMLResources, m_CSSResources, m_Book);

    AddTableData(html_classes_usage);

    for (int i = 0; i < ui.fileTree->header()->count(); i++) {
        ui.fileTree->resizeColumnToContents(i);
    }

    // Sort commonly used order
    ui.fileTree->sortByColumn(2, Qt::AscendingOrder);
    ui.fileTree->sortByColumn(1, Qt::AscendingOrder);
    ui.fileTree->sortByColumn(0, Qt::AscendingOrder);
    ui.fileTree->sortByColumn(3, Qt::AscendingOrder);
}

void ClassesInHTMLFilesWidget::SetupTable()
{
    m_ItemModel->clear();

    QStringList header;

    header.append(tr("HTML File"));
    header.append(tr("Element"));
    header.append(tr("Class"));
    header.append(tr("Matched Selector"));
    header.append(tr("Found In"));

    m_ItemModel->setHorizontalHeaderLabels(header);

    ui.fileTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.fileTree->setModel(m_ItemModel);
    ui.fileTree->header()->setSortIndicatorShown(true);

    ui.fileTree->header()->setToolTip(
        tr("<p>This is a list of the style classes used in all HTML files and whether or not the style matched a selector in a linked stylesheet.<p>") %
        tr("<p>NOTE:</p>") %
        tr("<p>Due to the complexities of CSS you must check your code manually to be certain if a style is used or not.</p>")
        );
}

void ClassesInHTMLFilesWidget::AddTableData(QList<BookReports::StyleData *> html_classes_usage)
{
    foreach (BookReports::StyleData *class_usage, html_classes_usage) {
        // Write the table entries
        QList<QStandardItem *> rowItems;

        // File name
        QStandardItem *filename_item = new QStandardItem();
        filename_item->setText(class_usage->html_filename);
        rowItems << filename_item;

        // Element name
        QStandardItem *element_name_item = new QStandardItem();
        element_name_item->setText(class_usage->html_element_name);
        rowItems << element_name_item;

        // Class name
        QStandardItem *class_name_item = new QStandardItem();
        class_name_item->setText(class_usage->html_class_name);
        rowItems << class_name_item;

        // Selector
        QStandardItem *selector_text_item = new QStandardItem();
        selector_text_item->setText(class_usage->css_selector_text);
        rowItems << selector_text_item;

        // Found in
        QStandardItem *found_in_item = new QStandardItem();
        QString css_short_filename = class_usage->css_filename;
        css_short_filename = css_short_filename.right(css_short_filename.length() - css_short_filename.lastIndexOf('/') - 1);
        found_in_item->setText(css_short_filename);
        found_in_item->setToolTip(class_usage->css_filename);
        rowItems << found_in_item;

        for (int i = 0; i < rowItems.count(); i++) {
            rowItems[i]->setEditable(false);
        }

        m_ItemModel->appendRow(rowItems);
    }
}

void ClassesInHTMLFilesWidget::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();

    QStandardItem *root_item = m_ItemModel->invisibleRootItem();
    QModelIndex parent_index;

    // Hide rows that don't contain the filter text
    int first_visible_row = -1;
    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText) ||
                              root_item->child(row, 1)->text().toLower().contains(lowercaseText) ||
                              root_item->child(row, 2)->text().toLower().contains(lowercaseText) ||
                              root_item->child(row, 3)->text().toLower().contains(lowercaseText)) {
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

ReportsWidget::Results ClassesInHTMLFilesWidget::saveSettings()
{
    ReportsWidget::Results results;

    results.filename = "";
    results.line = -1;
    results.files_to_delete.clear();
    results.styles_to_delete.clear();

    if (ui.fileTree->selectionModel()->hasSelection()) {
        QModelIndex index = ui.fileTree->selectionModel()->selectedRows(0).first();
        if (index.row() != m_ItemModel->rowCount() - 1) {
            results.filename = m_ItemModel->itemFromIndex(index)->text();
        }
    }

    return results;
}

void ClassesInHTMLFilesWidget::connectSignalsSlots()
{
    connect(ui.Filter,    SIGNAL(textChanged(QString)), 
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect (ui.fileTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SIGNAL(Done()));
}
