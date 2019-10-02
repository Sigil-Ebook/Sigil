/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford, Ontario
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012-2013 Dave Heiland
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
#include "Dialogs/ReportsWidgets/AllFilesWidget.h"
#include "Misc/HTMLSpellCheck.h"
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NavProcessor.h"

static const QString SETTINGS_GROUP = "reports";
static const QString DEFAULT_REPORT_FILE = "AllFilesReport.csv";


AllFilesWidget::AllFilesWidget()
    :
    m_ItemModel(new QStandardItemModel),
    m_LastDirSaved(QString()),
    m_LastFileSaved(QString())
{
    ui.setupUi(this);
    connectSignalsSlots();
}

AllFilesWidget::~AllFilesWidget()
{
  delete m_ItemModel;
}

void AllFilesWidget::CreateReport(QSharedPointer<Book> book)
{
    m_Book = book;
    SetupTable();
}

void AllFilesWidget::SetupTable(int sort_column, Qt::SortOrder sort_order)
{
    // Need to rebuild m_AllResources since deletes can happen behind the scenes
    m_AllResources = m_Book->GetAllResources();
    QString version = m_Book->GetOPF()->GetEpubVersion();
    m_ItemModel->clear();
    QStringList header;
    header.append(tr("Directory"));
    header.append(tr("Name"));
    header.append(tr("File Size (KB)"));
    header.append(tr("Type"));
    header.append(tr("Semantics"));
    if (version.startsWith("3")) {
        header.append(tr("Properties"));
    }
    m_ItemModel->setHorizontalHeaderLabels(header);
    ui.fileTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.fileTree->setModel(m_ItemModel);
    ui.fileTree->header()->setSortIndicatorShown(true);
    double total_size = 0;
    QString main_folder = m_Book->GetFolderKeeper()->GetFullPathToMainFolder();
    foreach(Resource *resource, m_AllResources) {
        QString fullpath = resource->GetFullPath();
        QString filepath = resource->GetRelativePath();
        QString directory = resource->GetFolder();
        QString file_spname = resource->ShortPathName();
        QList<QStandardItem *> rowItems;
        QStandardItem *item;
        // Directory
        item = new QStandardItem();
        item->setText(directory);
        rowItems << item;
        // Filename
        item = new QStandardItem();
        item->setText(file_spname);
	item->setData(filepath);
        item->setToolTip(filepath);
        rowItems << item;
        // File Size
        double ffsize = QFile(fullpath).size() / 1024.0;
        total_size += ffsize;
        QString fsize = QString::number(ffsize, 'f', 2);
        NumericItem *size_item = new NumericItem();
        size_item->setText(fsize);
        rowItems << size_item;
        // Type
        item = new QStandardItem();
        item ->setText(GetType(resource));
        rowItems << item;
        // Semantics
        item = new QStandardItem();
        if (version.startsWith('3')) {
            NavProcessor navproc(m_Book->GetConstOPF()->GetNavResource());
            item->setText(navproc.GetLandmarkNameForResource(resource));
        } else {
            item->setText(m_Book->GetOPF()->GetGuideSemanticNameForResource(resource));
        }
        rowItems << item;
        // Manifest Properties
        if (version.startsWith('3')) {
            item = new QStandardItem();
            item->setText(m_Book->GetOPF()->GetManifestPropertiesForResource(resource));
            rowItems << item;
        }
        // Add item to table
        m_ItemModel->appendRow(rowItems);
        for (int i = 0; i < rowItems.count(); i++) {
            rowItems[i]->setEditable(false);
        }
    }
    // Sort before adding the totals row
    // Since sortIndicator calls this routine, must disconnect/reconnect while resorting
    disconnect(ui.fileTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    ui.fileTree->header()->setSortIndicator(sort_column, sort_order);
    connect(ui.fileTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    // Totals
    NumericItem *nitem;
    QList<QStandardItem *> rowItems;
    // Directory
    nitem = new NumericItem();
    rowItems << nitem;
    // Files
    nitem = new NumericItem();
    nitem->setText(QString(tr("%n file(s)", "", m_AllResources.count())));
    rowItems << nitem;
    // File size
    nitem = new NumericItem();
    nitem->setText(QLocale().toString(total_size, 'f', 2));
    rowItems << nitem;

    QFont font;
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


void AllFilesWidget::FilterEditTextChangedSlot(const QString &text)
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

void AllFilesWidget::Sort(int logicalindex, Qt::SortOrder order)
{
    SetupTable(logicalindex, order);
}

void AllFilesWidget::DoubleClick()
{
    QModelIndex index = ui.fileTree->selectionModel()->selectedRows(1).first();

    if (index.row() != m_ItemModel->rowCount() - 1) {
        QString filepath = m_ItemModel->itemFromIndex(index)->data().toString();
        emit OpenFileRequest(filepath, 1);
    }
}

void AllFilesWidget::Save()
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

QString AllFilesWidget::GetType(Resource *resource)
{
    QString type;

    switch (resource->Type()) {
        case Resource::HTMLResourceType: {
            type = "HTML";
            break;
        }

        case Resource::CSSResourceType: {
            type = "CSS";
            break;
        }

        case Resource::ImageResourceType:
        case Resource::SVGResourceType: {
            type = tr("Image");
            break;
        }

        case Resource::AudioResourceType: {
            type = tr("Audio");
            break;
        }

        case Resource::VideoResourceType: {
            type = tr("Video");
            break;
        }

        case Resource::FontResourceType: {
            type = tr("Font");
            break;
        }


        case Resource::OPFResourceType: {
            type = "OPF";
            break;
        }

        case Resource::NCXResourceType: {
            type = "NCX";
            break;
        }

        case Resource::XMLResourceType: {
            type = "XML";
            break;
        }

        case Resource::MiscTextResourceType:
        case Resource::TextResourceType: {
            type = "Text";
            break;
        }

        default:
            type = tr("Unknown");
            break;
    }

    return type;
}

void AllFilesWidget::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    m_LastDirSaved = settings.value("last_dir_saved").toString();
    m_LastFileSaved = settings.value("last_file_saved_all_files").toString();

    if (m_LastFileSaved.isEmpty()) {
        m_LastFileSaved = DEFAULT_REPORT_FILE;
    }

    settings.endGroup();
}

void AllFilesWidget::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    settings.setValue("last_dir_saved", m_LastDirSaved);
    settings.setValue("last_file_saved_all_files", m_LastFileSaved);
    settings.endGroup();
}


void AllFilesWidget::connectSignalsSlots()
{
    connect(ui.leFilter,  SIGNAL(textChanged(QString)),
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.fileTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(DoubleClick()));
    connect(ui.fileTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SIGNAL(CloseDialog()));
    connect(ui.buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(Save()));
}

