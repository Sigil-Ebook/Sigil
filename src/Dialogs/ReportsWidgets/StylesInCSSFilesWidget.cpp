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
#include <QtCore/QFileInfo>
#include <QtCore/QHashIterator>
#include <QtWidgets/QFileDialog>
#include <QtGui/QFont>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "sigil_exception.h"
#include "BookManipulation/FolderKeeper.h"
#include "Dialogs/ReportsWidgets/StylesInCSSFilesWidget.h"
#include "Misc/CSSInfo.h"
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/CSSResource.h"

static const QString SETTINGS_GROUP = "reports";
static const QString DEFAULT_REPORT_FILE = "StyleClassesInCSSFilesReport.csv";

StylesInCSSFilesWidget::StylesInCSSFilesWidget()
    :
    m_ItemModel(new QStandardItemModel),
    m_ContextMenu(new QMenu(this))
{
    ui.setupUi(this);
    ui.fileTree->setContextMenuPolicy(Qt::CustomContextMenu);
    CreateContextMenuActions();
    connectSignalsSlots();
}

void StylesInCSSFilesWidget::CreateReport(QSharedPointer<Book> book)
{
    m_Book = book;
    SetupTable();
    // Get the list of classes in HTML and what selectors they match
    QList<BookReports::StyleData *> html_classes_usage = BookReports::GetAllHTMLClassUsage(m_Book);
    // Get the list of selectors in CSS files and if they were matched by HTML classes
    QList<BookReports::StyleData *> css_selector_usage = BookReports::GetCSSSelectorUsage(m_Book, html_classes_usage);
    qDeleteAll(html_classes_usage);
    AddTableData(css_selector_usage);
    qDeleteAll(css_selector_usage);

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

void StylesInCSSFilesWidget::AddTableData(const QList<BookReports::StyleData *> css_selectors_usage)
{
    foreach(BookReports::StyleData *selector_usage, css_selectors_usage) {
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


void StylesInCSSFilesWidget::DoubleClick()
{
    QModelIndex index = ui.fileTree->selectionModel()->selectedRows(0).first();
    QString filename = m_ItemModel->itemFromIndex(index)->text();
    int line = m_ItemModel->itemFromIndex(index.sibling(index.row(), 1))->data().toInt();
    emit OpenFileRequest(filename, line);
}

void StylesInCSSFilesWidget::Delete()
{
    QString style_names;
    QHash<QString, QStringList> stylesheet_styles;
    foreach(QModelIndex index, ui.fileTree->selectionModel()->selectedRows(0)) {
        QString filename = m_ItemModel->itemFromIndex(index)->text();
        QString name = m_ItemModel->itemFromIndex(index.sibling(index.row(), 1))->text();
        stylesheet_styles[filename].append(name);
    }
    int count = 0;
    QHashIterator<QString, QStringList> it_stylesheet_styles(stylesheet_styles);

    while (it_stylesheet_styles.hasNext()) {
        it_stylesheet_styles.next();
        style_names += "\n\n" + it_stylesheet_styles.key() + ": " "\n";
        foreach(QString name, it_stylesheet_styles.value()) {
            style_names += name + ", ";
            count++;
        }
        style_names = style_names.left(style_names.length() - 2);
    }

    QList<BookReports::StyleData *> styles_to_delete;
    foreach(QModelIndex index, ui.fileTree->selectionModel()->selectedRows(0)) {
        BookReports::StyleData *style = new BookReports::StyleData();
        style->css_filename = m_ItemModel->itemFromIndex(index)->text();
        style->css_selector_text = m_ItemModel->itemFromIndex(index.sibling(index.row(), 1))->text();
        style->css_selector_line = m_ItemModel->itemFromIndex(index.sibling(index.row(), 1))->data().toInt();
        styles_to_delete.append(style);
    }
    emit DeleteStylesRequest(styles_to_delete);
}

void StylesInCSSFilesWidget::Save()
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
    // We do not enable the delete option if no rows selected.
    m_Delete->setEnabled(ui.fileTree->selectionModel()->selectedRows().count() > 0);
}


void StylesInCSSFilesWidget::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    m_LastDirSaved = settings.value("last_dir_saved").toString();
    m_LastFileSaved = settings.value("last_file_saved_styles_css_files").toString();

    if (m_LastFileSaved.isEmpty()) {
        m_LastFileSaved = DEFAULT_REPORT_FILE;
    }

    settings.endGroup();
}

void StylesInCSSFilesWidget::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    settings.setValue("last_dir_saved", m_LastDirSaved);
    settings.setValue("last_file_saved_styles_css_files", m_LastFileSaved);
    settings.endGroup();
}

void StylesInCSSFilesWidget::connectSignalsSlots()
{
    connect(ui.Filter,    SIGNAL(textChanged(QString)),
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.fileTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,          SLOT(DoubleClick()));
    connect(ui.fileTree,  SIGNAL(customContextMenuRequested(const QPoint &)),
            this,         SLOT(OpenContextMenu(const QPoint &)));
    connect(m_Delete,     SIGNAL(triggered()), this, SLOT(Delete()));
    connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SIGNAL(CloseDialog()));
    connect(ui.buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(Save()));
}
