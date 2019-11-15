/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford, Ontario, Canada
**  Copyright (C) 2013      John Schember <john@nachtimwald.com>
**  Copyright (C) 2013      Dave Heiland
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
#include <QtWidgets/QFileDialog>
#include <QtGui/QFont>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "sigil_exception.h"
#include "BookManipulation/FolderKeeper.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Dialogs/ReportsWidgets/LinksWidget.h"
#include "Misc/HTMLSpellCheck.h"
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"

static const QString SETTINGS_GROUP = "reports";
static const QString DEFAULT_REPORT_FILE = "LinksReport.csv";

LinksWidget::LinksWidget()
    :
    m_ItemModel(new QStandardItemModel),
    m_LastDirSaved(QString()),
    m_LastFileSaved(QString())
{
    ui.setupUi(this);
    connectSignalsSlots();
}

LinksWidget::~LinksWidget()
{
    delete m_ItemModel;
}

void LinksWidget::CreateReport(QSharedPointer<Book> book)
{
    m_Book = book;
    m_HTMLResources = m_Book->GetFolderKeeper()->GetResourceTypeList<HTMLResource>(false);

    SetupTable();
}

void LinksWidget::SetupTable(int sort_column, Qt::SortOrder sort_order)
{
    m_ItemModel->clear();
    QStringList header;
    header.append(tr("File"));
    header.append(tr("Line"));
    header.append(tr("ID"));
    header.append(tr("Text"));
    header.append(tr("Target File"));
    header.append(tr("Target ID"));
    header.append(tr("Target Exists?"));
    header.append(tr("Target Text"));
    header.append(tr("Target's Target File"));
    header.append(tr("Target's Target ID"));
    header.append(tr("Match?"));
    m_ItemModel->setHorizontalHeaderLabels(header);
    ui.fileTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.fileTree->setModel(m_ItemModel);
    ui.fileTree->header()->setSortIndicatorShown(true);
    ui.fileTree->header()->setToolTip(
        tr("Report shows all source and target links using the anchor tag \"a\".")
    );

    // Key is book path of html file
    QHash<QString, QList<XhtmlDoc::XMLElement> > links = m_Book->GetLinkElements();

    // Key is book path of html file
    QHash<QString, QStringList> all_ids = m_Book->GetIdsInHTMLFiles();

    // html_filenames is a list of html book paths
    QStringList html_filenames;
    foreach(Resource *resource, m_HTMLResources) {
        html_filenames.append(resource->GetRelativePath());
    }

    foreach(Resource *resource, m_HTMLResources) {
        QString filepath = resource->GetRelativePath();
        QString path = resource->GetFullPath();
        QString file_spname = resource->ShortPathName();

        foreach(XhtmlDoc::XMLElement element, links[filepath]) {
            QList<QStandardItem *> rowItems;

            // Source file
            QStandardItem *item = new QStandardItem();
            QString source_file = file_spname;
            item->setText(source_file);
            item->setToolTip(filepath);
	    item->setData(filepath);
            rowItems << item;

            // Source Line Number
            item = new QStandardItem();
            QString lineno = QString::number(element.lineno);
            item->setText(lineno);
            item->setToolTip(filepath);
            rowItems << item;

            // Source id
            item = new QStandardItem();
            QString source_id = element.attributes["id"];
            item->setText(source_id);
            rowItems << item;

            // Source text
            item = new QStandardItem();
            item->setText(element.text);
            rowItems << item;

            // Source target file & id
            QString href = element.attributes["href"];
            QUrl url(href);
            QString href_file;
            QString href_id;
            bool is_target_file = false;
            if (url.scheme().isEmpty() || url.scheme() == "file") {
                href_file = url.path();
                href_id = url.fragment();
                is_target_file = true;
            } else {
                // Just show url
                href_file = href;
            }
            item = new QStandardItem();
            item->setText(href_file);
            rowItems << item;

            item = new QStandardItem();
            item->setText(href_id);
            rowItems << item;

            // Target exists in book
            QString target_valid = tr("n/a");
	    QString bkpath;
            if (is_target_file) {
                if (!href.isEmpty()) {
                    target_valid = tr("no");
                    // first handle the case of local internal link (just fragment)
		    if (href_file.isEmpty()) {
		        bkpath = filepath;
		    } else {
                        // find bookpath of target
		        bkpath = Utility::buildBookPath(href_file, resource->GetFolder());
		    }	
                    if (html_filenames.contains(bkpath)) {
                        if (href_id.isEmpty() || all_ids[bkpath].contains(href_id)) {
                            target_valid = tr("yes");
                        }
                    }
                }
            }
            item = new QStandardItem();
            item->setText(target_valid);
            rowItems << item;

            if (is_target_file && !href_id.isEmpty()) {
                // Find the target element for this link
                // As long as an anchor tag was used!
                XhtmlDoc::XMLElement target;
                bool found = false;

                foreach(XhtmlDoc::XMLElement target_element, links[bkpath]) {
                    if (href_id == target_element.attributes["id"]) {
                        target = target_element;
                        found = true;
                        break;
                    }
                }
                if (found) {

                    // Target Text
                    item = new QStandardItem();
                    item->setText(target.text);
                    rowItems << item;

                    // Target's Target file and id
                    QString target_href = target.attributes["href"];
                    QUrl target_url(target_href);
                    QString target_href_file;
                    QString target_href_id;
                    if (target_url.scheme().isEmpty() || target_url.scheme() == "file") {
                        target_href_file = target_url.path();
                        target_href_id = target_url.fragment();
                    } else {
                        // Just show url
                        target_href_file = target_href;
                    }

                    item = new QStandardItem();
                    item->setText(target_href_file);
                    rowItems << item;

                    item = new QStandardItem();
                    item->setText(target_href_id);
                    rowItems << item;

		    QString target_bkpath;
                    // Match - destination link points to source
                    if (target_href_file.isEmpty()) {
		        target_bkpath = bkpath;
                    } else {
		        Resource * res =  m_Book->GetFolderKeeper()->GetResourceByBookPath(bkpath);
		        target_bkpath = Utility::buildBookPath(target_href_file, res->GetFolder());
		    }
                    QString match = tr("no");
                    if (!source_id.isEmpty() && filepath == target_bkpath && source_id == target_href_id) {
                        match = tr("yes");
                    }
                    item = new QStandardItem();
                    item->setText(match);
                    rowItems << item;
                }
            }
            // Add item to table
            m_ItemModel->appendRow(rowItems);
            for (int i = 0; i < rowItems.count(); i++) {
                rowItems[i]->setEditable(false);
            }
        }
    }

    for (int i = 0; i < ui.fileTree->header()->count(); i++) {
        ui.fileTree->resizeColumnToContents(i);
    }
}


void LinksWidget::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();
    QStandardItem *root_item = m_ItemModel->invisibleRootItem();
    QModelIndex parent_index;
    // Hide rows that don't contain the filter text
    int first_visible_row = -1;

    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText) ||
            root_item->child(row, 2)->text().toLower().contains(lowercaseText) ||
            root_item->child(row, 3)->text().toLower().contains(lowercaseText) ||
            root_item->child(row, 4)->text().toLower().contains(lowercaseText) ||
            root_item->child(row, 5)->text().toLower().contains(lowercaseText) ||
            root_item->child(row, 6)->text().toLower().contains(lowercaseText)) {
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

void LinksWidget::DoubleClick()
{
    QModelIndex index = ui.fileTree->selectionModel()->selectedRows(0).first();
    if (index.row() != m_ItemModel->rowCount() - 1) {
        // IMPORTANT:  file name is in column 0, and line number is in column 1
        // This should match order of header above
        QString bookpath = m_ItemModel->item(index.row(), 0)->data().toString();
        QString lineno = m_ItemModel->item(index.row(), 1)->text();
        emit OpenFileRequest(bookpath, lineno.toInt());
    }
}

void LinksWidget::Save()
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
    QFileDialog::Options options = QFileDialog::Options();
#ifdef Q_OS_MAC
    options = options | QFileDialog::DontUseNativeDialog;
#endif

    QString destination = QFileDialog::getSaveFileName(this,
                          tr("Save Report As Comma Separated File"),
                          save_path,
                          filter_string,
			  &default_filter,
                          options
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

void LinksWidget::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    m_LastDirSaved = settings.value("last_dir_saved").toString();
    m_LastFileSaved = settings.value("last_file_saved_links").toString();

    if (m_LastFileSaved.isEmpty()) {
        m_LastFileSaved = DEFAULT_REPORT_FILE;
    }

    settings.endGroup();
}

void LinksWidget::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    settings.setValue("last_dir_saved", m_LastDirSaved);
    settings.setValue("last_file_saved_links", m_LastFileSaved);
    settings.endGroup();
}


void LinksWidget::connectSignalsSlots()
{
    connect(ui.leFilter,  SIGNAL(textChanged(QString)),
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.fileTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(DoubleClick()));
    connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SIGNAL(CloseDialog()));
    connect(ui.buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(Save()));
}

