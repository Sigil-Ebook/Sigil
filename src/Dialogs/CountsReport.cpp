/************************************************************************
**
**  Copyright (C) 2022 Kevin B. Hendricks, Stratford, Ontario
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

#include <QFont>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include "Misc/NumericItem.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"

#include "Dialogs/CountsReport.h"

#include "sigil_exception.h"

static const QString SETTINGS_GROUP = "counts_report";
static const QString DEFAULT_REPORT_FILE = "CountsReport.csv";


CountsReport::CountsReport(QWidget* parent)
    :
    QDialog(parent),
    m_ItemModel(new QStandardItemModel),
    m_LastDirSaved(QString()),
    m_LastFileSaved(QString())
{
    ui.setupUi(this);
    connectSignalsSlots();
    ReadSettings();
}

CountsReport::~CountsReport()
{
    WriteSettings();
    foreach(SearchEditorModel::searchEntry* entry, m_entries) {
        if (entry) delete entry;
    }
    m_entries.clear();
    delete m_ItemModel;
}

void CountsReport::closeEvent(QCloseEvent *e)
{
    WriteSettings();
    QDialog::closeEvent(e);
}

void CountsReport::reject()
{
    WriteSettings();
    QDialog::reject();
}

void CountsReport::CreateReport(QList<SearchEditorModel::searchEntry*> entries)
{
    // note entries are each created with new so we are taking ownership
    // of all of them for cleanup purposes
    m_entries = entries;
    SetupTable();
}

void CountsReport::SetupTable(int sort_column, Qt::SortOrder sort_order)
{
    m_ItemModel->clear();
    QStringList header;
    header.append(tr("Search Full Name"));
    header.append(tr("Find"));
    header.append(tr("Target"));
    header.append(tr("Count"));
    m_ItemModel->setHorizontalHeaderLabels(header);
    ui.countsTree->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.countsTree->setModel(m_ItemModel);
    ui.countsTree->header()->setSortIndicatorShown(true);
    int total_count = 0;
    int num_entries = 0;
    foreach(SearchEditorModel::searchEntry* entry, m_entries) {
        if (!entry) continue;
        QString fullname = entry->fullname;
        QString find = entry->find;
        QString controls = entry->controls;
        QString target;
        if (controls.contains("AH")) target = tr("All HTML Files");
        if (controls.contains("SH")) target = tr("Selected HTML Files");
        if (controls.contains("TH")) target = tr("Tabbed HTML Files");
        if (controls.contains("AC")) target = tr("All CSS Files");
        if (controls.contains("SC")) target = tr("Selected CSS Files");
        if (controls.contains("TC")) target = tr("Tabbed CSS Files");
        if (controls.contains("OP")) target = tr("OPF File");
        if (controls.contains("NX")) target = tr("NCX File");
        QList<QStandardItem *> rowItems;
        QStandardItem *item;
        // Full Name
        item = new QStandardItem();
        item->setText(fullname);
        rowItems << item;
        num_entries++;
        // Find
        item = new QStandardItem();
        item->setText(find);
        rowItems << item;
        // Target
        item = new QStandardItem();
        item ->setText(target);
        rowItems << item;
        // Count
        int count = -1;
        emit CountRequest(entry, count);
        if (count > -1) total_count += count;
        NumericItem *count_item = new NumericItem();
        count_item->setText(QString::number(count));
        count_item->setTextAlignment(Qt::AlignRight);
        rowItems << count_item;
        // Add item to table
        m_ItemModel->appendRow(rowItems);
        for (int i = 0; i < rowItems.count(); i++) {
            rowItems[i]->setEditable(false);
        }
    }
    // Sort before adding the totals row
    // Since sortIndicator calls this routine, must disconnect/reconnect while resorting
    disconnect(ui.countsTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    ui.countsTree->header()->setSortIndicator(sort_column, sort_order);
    connect(ui.countsTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    // Totals
    NumericItem *nitem;
    QList<QStandardItem *> rowItems;
    // Full Name
    nitem = new NumericItem();
    nitem->setText(QString::number(num_entries));
    nitem->setTextAlignment(Qt::AlignRight);
    rowItems << nitem;
    // Finds
    nitem = new NumericItem();
    rowItems << nitem;
    // Target
    nitem = new NumericItem();
    rowItems << nitem;
    // Count 
    nitem = new NumericItem();
    nitem->setText(QString::number(total_count));
    nitem->setTextAlignment(Qt::AlignRight);
    rowItems << nitem;
    QFont font;
    font.setWeight(QFont::Bold);
    for (int i = 0; i < rowItems.count(); i++) {
        rowItems[i]->setEditable(false);
        rowItems[i]->setFont(font);
    }

    m_ItemModel->appendRow(rowItems);

    for (int i = 0; i < ui.countsTree->header()->count(); i++) {
        ui.countsTree->resizeColumnToContents(i);
    }
}


void CountsReport::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();
    QStandardItem *root_item = m_ItemModel->invisibleRootItem();
    QModelIndex parent_index;
    // Hide rows that don't contain the filter text
    int first_visible_row = -1;

    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText)) {
            ui.countsTree->setRowHidden(row, parent_index, false);

            if (first_visible_row == -1) {
                first_visible_row = row;
            }
        } else {
            ui.countsTree->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.countsTree->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    } else {
        // Clear current and selection, which clears preview image
        ui.countsTree->setCurrentIndex(QModelIndex());
    }
}

void CountsReport::Sort(int logicalindex, Qt::SortOrder order)
{
    SetupTable(logicalindex, order);
}

void CountsReport::Save()
{
    QStringList report_info;
    QStringList heading_row;

    // Get headings
    for (int col = 0; col < ui.countsTree->header()->count(); col++) {
        QStandardItem *item = m_ItemModel->horizontalHeaderItem(col);
        QString text = "";
        if (item) {
            text = item->text();
        }
        heading_row << text;
    }
    report_info << Utility::createCSVLine(heading_row);

    // Get data from table
    for (int row = 0; row < m_ItemModel->rowCount(); row++) {
        QStringList data_row;
        for (int col = 0; col < ui.countsTree->header()->count(); col++) {
            QStandardItem *item = m_ItemModel->item(row, col);
            QString text = "";
            if (item) {
                text = item->text();
            }
            data_row << text;
        }
        report_info << Utility::createCSVLine(data_row);
    }

    QString data = report_info.join('\n') + '\n';
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
                                                       options);

    if (destination.isEmpty()) {
        return;
    }

    try {
        Utility::WriteUnicodeTextFile(data, destination);
    } catch (CannotOpenFile&) {
        QMessageBox::warning(this, tr("Sigil"), tr("Cannot save report file."));
    }

    m_LastDirSaved = QFileInfo(destination).absolutePath();
    m_LastFileSaved = QFileInfo(destination).fileName();
    WriteSettings();
}

void CountsReport::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    m_LastDirSaved = settings.value("last_dir_saved").toString();
    m_LastFileSaved = settings.value("last_file_saved_all_files").toString();

    if (m_LastFileSaved.isEmpty()) {
        m_LastFileSaved = DEFAULT_REPORT_FILE;
    }

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }
    settings.endGroup();
}

void CountsReport::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // Last file open
    settings.setValue("last_dir_saved", m_LastDirSaved);
    settings.setValue("last_file_saved_all_files", m_LastFileSaved);
    settings.setValue("geometry", saveGeometry());
    settings.endGroup();
}


void CountsReport::connectSignalsSlots()
{
    connect(ui.leFilter,  SIGNAL(textChanged(QString)),
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.countsTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
    connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));
    connect(ui.buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(Save()));
}

