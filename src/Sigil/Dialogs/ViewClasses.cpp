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

#include "Dialogs/ViewClasses.h"
#include "Misc/NumericItem.h"
#include "ResourceObjects/HTMLResource.h"
#include "Misc/SettingsStore.h"

static const int COL_NAME = 0;

static QString SETTINGS_GROUP = "view_classes_files";

ViewClasses::ViewClasses(QSharedPointer< Book > book, QWidget *parent)
    :
    QDialog(parent),
    m_Book(book),
    m_ViewClassesModel(new QStandardItemModel),
    m_SelectedFile(QString())
{
    ui.setupUi(this);
    connectSignalsSlots();

    ReadSettings();

    SetFiles();
}

void ViewClasses::SetFiles(int sort_column, Qt::SortOrder sort_order)
{
    m_ViewClassesModel->clear();

    QStringList header;

    header.append(tr("Class"));
    header.append(tr("Times Used"));

    m_ViewClassesModel->setHorizontalHeaderLabels(header);

    ui.fileTree->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui.fileTree->setModel(m_ViewClassesModel);

    ui.fileTree->header()->setSortIndicatorShown(true);

    int total_classes = 0;

    QHash<QString, QStringList> class_names_hash = m_Book->GetAllClassesUsedInHTML();
    QHashIterator<QString, QStringList> iterator(class_names_hash);

    while (iterator.hasNext()) {
            iterator.next();
            QString class_name = iterator.key();
            QStringList class_names = iterator.value();

            QList<QStandardItem *> rowItems;

            // Class name
            QStandardItem *name_item = new QStandardItem();
            name_item->setText(class_name);
            rowItems << name_item;

            // Times Used
            NumericItem *count_item = new NumericItem();
            count_item->setText(QString::number(class_names.count()));
            total_classes += class_names.count();
            if (!class_names.isEmpty()) {
                class_names.removeDuplicates();
                count_item->setToolTip(class_names.join("\n"));
            }
            rowItems << count_item;

            for (int i = 0; i < rowItems.count(); i++) {
                rowItems[i]->setEditable(false);
            }
            m_ViewClassesModel->appendRow(rowItems);
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
    nitem->setText(QString::number(m_ViewClassesModel->rowCount()) % tr(" classes"));
    rowItems << nitem;

    // Times Used
    nitem = new NumericItem();
    nitem->setText(QString::number(total_classes));
    rowItems << nitem;

    QFont font = *new QFont();
    font.setWeight(QFont::Bold);
    for (int i = 0; i < rowItems.count(); i++) {
        rowItems[i]->setEditable(false);
        rowItems[i]->setFont(font);
    }

    m_ViewClassesModel->appendRow(rowItems);

    for (int i = 0; i < ui.fileTree->header()->count(); i++) {
        ui.fileTree->resizeColumnToContents(i);
    }
}


void ViewClasses::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();

    QStandardItem *root_item = m_ViewClassesModel->invisibleRootItem();
    QModelIndex parent_index;

    // Hide rows that don't contain the filter text
    int first_visible_row = -1;
    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, COL_NAME)->text().toLower().contains(lowercaseText)) {
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

void ViewClasses::Sort(int logicalindex, Qt::SortOrder order)
{
    SetFiles(logicalindex, order);
}

QString ViewClasses::SelectedFile()
{
    return m_SelectedFile;
}

void ViewClasses::SetSelectedFile()
{
    if (m_SelectedFile.isEmpty() && ui.fileTree->selectionModel()->hasSelection()) {
        QModelIndex index = ui.fileTree->selectionModel()->selectedRows(1).first();
        if (index.row() != m_ViewClassesModel->rowCount() - 1) {
            QString filenames = m_ViewClassesModel->itemFromIndex(index)->toolTip();
            // Return the first file that includes the class, not the class name
            m_SelectedFile = filenames.left(filenames.indexOf("\n"));
        }
    }
}

void ViewClasses::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value("geometry").toByteArray();

    if (!geometry.isNull()) {
        restoreGeometry(geometry);
    }

    settings.endGroup();
}

void ViewClasses::WriteSettings()
{
    SetSelectedFile();

    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    settings.endGroup();
}

void ViewClasses::connectSignalsSlots()
{
    connect(this,         SIGNAL(accepted()),
            this,         SLOT(WriteSettings()));
    connect(ui.Filter,    SIGNAL(textChanged(QString)), 
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect (ui.fileTree, SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(accept()));
    connect (ui.fileTree->header(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)), this, SLOT(Sort(int, Qt::SortOrder)));
}
