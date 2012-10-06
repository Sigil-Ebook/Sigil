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
#include <QtGui/QMessageBox>

#include "Misc/NumericItem.h"
#include "Misc/CSSInfo.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"
#include "Misc/SettingsStore.h"
#include "StylesInCSSFilesWidget.h"

static QString SETTINGS_GROUP = "reports_styles_in_css_files";

StylesInCSSFilesWidget::StylesInCSSFilesWidget(QList<Resource *>html_resources, QList<Resource *>css_resources, QSharedPointer< Book > book)
    :
    m_HTMLResources(html_resources),
    m_CSSResources(css_resources),
    m_Book(book),
    m_ItemModel(new QStandardItemModel),
    m_ContextMenu(new QMenu(this)),
    m_DeleteStyles(false)
{
    ui.setupUi(this);

    ui.fileTree->setContextMenuPolicy(Qt::CustomContextMenu);

    CreateContextMenuActions();

    connectSignalsSlots();

    SetupTable();

    // Get the list of classes in HTML and what selectors they match
    QList<BookReports::StyleData *> html_classes_usage = BookReports::GetHTMLClassUsage(m_HTMLResources, m_CSSResources, m_Book);

    // Get the list of selectors in CSS files and if they were matched by HTML classes
    QList<BookReports::StyleData *> css_selector_usage = BookReports::GetCSSSelectorUsage(m_CSSResources, html_classes_usage);

    AddTableData(css_selector_usage);

    for (int i = 0; i < ui.fileTree->header()->count(); i++) {
        ui.fileTree->resizeColumnToContents(i);
    }

    ui.fileTree->sortByColumn(1, Qt::AscendingOrder);
    ui.fileTree->sortByColumn(0, Qt::AscendingOrder);
    ui.fileTree->sortByColumn(2, Qt::AscendingOrder);
}

void StylesInCSSFilesWidget::SetupTable()
{
    m_ItemModel->clear();

    QStringList header;

    header.append(tr("CSS File"));
    header.append(tr("Class Selector"));
    header.append(tr("Used In HTML File"));

    m_ItemModel->setHorizontalHeaderLabels(header);

    ui.fileTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.fileTree->setModel(m_ItemModel);
    ui.fileTree->header()->setSortIndicatorShown(true);

    ui.fileTree->header()->setToolTip(
        tr("<p>This is a list of the class based selectors in all CSS files and whether or not the selector was matched from a style in an HTML file.<p>") %
        tr("<p>NOTE:</p>") %
        tr("<p>Due to the complexities of CSS you must check your code manually to be certain if a style is used or not.</p>")
        );
}

void StylesInCSSFilesWidget::AddTableData(QList<BookReports::StyleData *> css_selectors_usage)
{
    foreach (BookReports::StyleData *selector_usage, css_selectors_usage) {
        // Write the table entries
        QList<QStandardItem *> rowItems;

        // File name
        QStandardItem *filename_item = new QStandardItem();
        QString css_short_filename = selector_usage->css_filename;
        css_short_filename = css_short_filename.right(css_short_filename.length() - css_short_filename.lastIndexOf('/') - 1);
        filename_item->setText(css_short_filename);
        filename_item->setToolTip(selector_usage->css_filename);
        rowItems << filename_item;

        // Selector
        QStandardItem *selector_text_item = new QStandardItem();
        selector_text_item->setText(selector_usage->css_selector_text);
        selector_text_item->setData(selector_usage->css_selector_line);
        rowItems << selector_text_item;

        // Found in
        QStandardItem *found_in_item = new QStandardItem();
        found_in_item->setText(selector_usage->html_filename);
        rowItems << found_in_item;

        for (int i = 0; i < rowItems.count(); i++) {
            rowItems[i]->setEditable(false);
        }

        m_ItemModel->appendRow(rowItems);
    }
}


void StylesInCSSFilesWidget::FilterEditTextChangedSlot(const QString &text)
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

ReportsWidget::Results StylesInCSSFilesWidget::saveSettings()
{
    ReportsWidget::Results results;

    results.filename = "";
    results.line = -1;
    results.files_to_delete.clear();
    results.styles_to_delete.clear();

    if (ui.fileTree->selectionModel()->hasSelection()) {
        if (m_DeleteStyles) {
            foreach (QModelIndex index, ui.fileTree->selectionModel()->selectedRows(0)) {

                BookReports::StyleData *style = new BookReports::StyleData();
                style->css_filename = m_ItemModel->itemFromIndex(index)->text();
                style->css_selector_text = m_ItemModel->itemFromIndex(index.sibling(index.row(), 1))->text();
                style->css_selector_line = m_ItemModel->itemFromIndex(index.sibling(index.row(), 1))->data().toInt();

                results.styles_to_delete.append(style);
            }
        }
        else {
            QModelIndex index = ui.fileTree->selectionModel()->selectedRows(0).first();
            if (index.row() != m_ItemModel->rowCount() - 1) {
                results.filename = m_ItemModel->itemFromIndex(index)->text();
                results.line = m_ItemModel->itemFromIndex(index.sibling(index.row(), 1))->data().toInt();
            }
        }
    }

    return results;
}

void StylesInCSSFilesWidget::Delete()
{
    QString style_names; 
    QHash< QString, QStringList> stylesheet_styles;

    foreach (QModelIndex index, ui.fileTree->selectionModel()->selectedRows(0)) {
        QString filename = m_ItemModel->itemFromIndex(index)->text();
        QString name = m_ItemModel->itemFromIndex(index.sibling(index.row(), 1))->text();
        stylesheet_styles[filename].append(name);
    }

    int count = 0;
    QHashIterator< QString, QStringList> it_stylesheet_styles(stylesheet_styles);
    while (it_stylesheet_styles.hasNext()) {
        it_stylesheet_styles.next();
        style_names += "\n\n" + it_stylesheet_styles.key() + ": " "\n";
        foreach (QString name, it_stylesheet_styles.value()) {
            style_names += name + ", ";
            count++;
        }
        style_names = style_names.left(style_names.length() - 2);
    }

    QMessageBox::StandardButton button_pressed;
    QString msg = count == 1 ? tr( "Are you sure you want to delete the style listed below?\n" ):
                                           tr( "Are you sure you want to delete all the styles listed below?\n" );
    button_pressed = QMessageBox::warning(  this,
                      tr( "Sigil" ), msg % tr( "This action cannot be reversed." ) % style_names,
                                                QMessageBox::Ok | QMessageBox::Cancel
                                         );
    if ( button_pressed != QMessageBox::Ok ) {
        return;
    }

    m_DeleteStyles = true;

    emit Done();
}

void StylesInCSSFilesWidget::CreateContextMenuActions()
{
    m_Delete    = new QAction(tr("Delete From Stylesheet") + "...",     this);

    m_Delete->setShortcut(QKeySequence::Delete);

    // Has to be added to the dialog itself for the keyboard shortcut to work.
    addAction(m_Delete);
}

void StylesInCSSFilesWidget::OpenContextMenu(const QPoint &point)
{
    SetupContextMenu(point);

    m_ContextMenu->exec(ui.fileTree->viewport()->mapToGlobal(point));
    m_ContextMenu->clear();
}

void StylesInCSSFilesWidget::SetupContextMenu(const QPoint &point)
{
    m_ContextMenu->addAction(m_Delete);
}

void StylesInCSSFilesWidget::connectSignalsSlots()
{
    connect(ui.Filter,    SIGNAL(textChanged(QString)), 
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect (ui.fileTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,          SIGNAL(Done()));

    connect(ui.fileTree,  SIGNAL(customContextMenuRequested(const QPoint&)),
            this,         SLOT(  OpenContextMenu(                  const QPoint&)));
    connect(m_Delete,     SIGNAL(triggered()), this, SLOT(Delete()));
}
