/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#include <QtWidgets/QFileDialog>
#include <QtGui/QFont>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QMessageBox>

#include "sigil_constants.h"
#include "sigil_exception.h"
#include "BookManipulation/FolderKeeper.h"
#include "Dialogs/ReportsWidgets/ClassesInHTMLFilesWidget.h"
#include "Misc/CSSInfo.h"
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"

static const QString SETTINGS_GROUP = "reports";
static const QString DEFAULT_REPORT_FILE = "StyleClassesInHTMLFilesReport.csv";

ClassesInHTMLFilesWidget::ClassesInHTMLFilesWidget()
    :
    m_ItemModel(new QStandardItemModel),
    m_LastDirSaved(QString()),
    m_LastFileSaved(QString())
{
    ui.setupUi(this);
    connectSignalsSlots();
}

ClassesInHTMLFilesWidget::~ClassesInHTMLFilesWidget()
{
    delete m_ItemModel;
}

void ClassesInHTMLFilesWidget::CreateReport(QSharedPointer<Book> book)
{
    m_Book = book;
    SetupTable();
    QList<BookReports::StyleData *> html_classes_usage = BookReports::GetHTMLClassUsage(m_Book);
    AddTableData(html_classes_usage);
    qDeleteAll(html_classes_usage);

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

void ClassesInHTMLFilesWidget::AddTableData(const QList<BookReports::StyleData *> html_classes_usage)
{
    foreach(BookReports::StyleData *class_usage, html_classes_usage) {
        // Skip custom Sigil classes that are only used as markers not styles
        if (class_usage->html_class_name == SIGIL_NOT_IN_TOC_CLASS ||
            class_usage->html_class_name == OLD_SIGIL_NOT_IN_TOC_CLASS ||
            class_usage->html_class_name == SIGIL_INDEX_CLASS) {
            continue;
        }

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
        } else {
            ui.fileTree->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.fileTree->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    } else {
        // Clear current and selection, which clears preview image
        ui.fileTree->setCurrentIndex(QModelIndex());
    }
}

void ClassesInHTMLFilesWidget::DoubleClick()
{
    QModelIndex index = ui.fileTree->selectionModel()->selectedRows(0).first();
    QString filename = m_ItemModel->itemFromIndex(index)->text();
    emit OpenFileRequest(filename, 1);
}

void ClassesInHTMLFilesWidget::Save()
{
    QString report_info;
    QString row_text;

    // Get headings
    for (int col = 0; col < ui.fileTree->header()->count(); col++) {
        QStandardItem *item = m_ItemModel->horizontalHeaderItem(col);
        QString text = "";
        if (item) {
            text = item->text();
        }
        if (col == 0) {
            row_text.append(text);
        } else {
            row_text.append("," % text);
        }
    }

    report_info.append(row_text % "\n");

    // Get data from table
    for (int row = 0; row < m_ItemModel->rowCount(); row++) {
        row_text = "";

        for (int col = 0; col < ui.fileTree->header()->count(); col++) {
            QStandardItem *item = m_ItemModel->item(row, col);
            QString text = "";
            if (item) {
                text = item->text();
            }
            if (col == 0) {
                row_text.append(text);
            } else {
                row_text.append("," % text);
            }
        }

        report_info.append(row_text % "\n");
    }

    // Save the file
    ReadSettings();
    QString filter_string = "*.csv;;*.txt;;*.*";
    QString default_filter = "";
    QString save_path = m_LastDirSaved + "/" + m_LastFileSaved;
    QString destination = QFileDialog::getSaveFileName(this,
                          tr("Save Report As Comma Separated File"),
                          save_path,
                          filter_string,
                          &default_filter
                                                      );

    if (destination.isEmpty()) {
        return;
    }

    try {
        Utility::WriteUnicodeTextFile(report_info, destination);
    } catch (CannotOpenFile) {
        QMessageBox::warning(this, tr("Sigil"), tr("Cannot save report file."));
    }

    m_LastDirSaved = QFileInfo(destination).absolutePath();
    m_LastFileSaved = QFileInfo(destination).fileName();
    WriteSettings();
}

void ClassesInHTMLFilesWidget::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    m_LastDirSaved = settings.value("last_dir_saved").toString();
    m_LastFileSaved = settings.value("last_file_saved_classes_in_html").toString();

    if (m_LastFileSaved.isEmpty()) {
        m_LastFileSaved = "ClassesInHTMLFilesReport.csv";
    }

    settings.endGroup();
}

void ClassesInHTMLFilesWidget::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    settings.setValue("last_dir_saved", m_LastDirSaved);
    settings.setValue("last_file_saved_classes_html", m_LastFileSaved);
    settings.endGroup();
}


void ClassesInHTMLFilesWidget::connectSignalsSlots()
{
    connect(ui.Filter,    SIGNAL(textChanged(QString)),
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.fileTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(DoubleClick()));
    connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SIGNAL(CloseDialog()));
    connect(ui.buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(Save()));
}
