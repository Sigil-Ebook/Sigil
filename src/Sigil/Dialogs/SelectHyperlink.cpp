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

#include "Dialogs/SelectHyperlink.h"
#include "ResourceObjects/HTMLResource.h"
#include "Misc/SettingsStore.h"

static QString SETTINGS_GROUP = "select_hyperlink";

SelectHyperlink::SelectHyperlink(QString default_href, HTMLResource *html_resource, QList<Resource*> resources, QSharedPointer< Book > book, QWidget *parent)
    :
    QDialog(parent),
    m_DefaultHref(default_href),
    m_HTMLResource(html_resource),
    m_Resources(resources),
    m_Book(book),
    m_SelectHyperlinkModel(new QStandardItemModel)
{
    ui.setupUi(this);
    connectSignalsSlots();

    ReadSettings();

    SetList();

    // Default entry on the target url not the filter
    ui.href->setFocus();
}

void SelectHyperlink::SetList()
{
    m_SelectHyperlinkModel->clear();

    QStringList header;

    header.append(tr("Existing Filenames and IDs in the EPUB:"));

    m_SelectHyperlinkModel->setHorizontalHeaderLabels(header);

    ui.list->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui.list->setModel(m_SelectHyperlinkModel);

    // Get the complete list of valid targets
    QHash<QString, QStringList> id_names_hash = m_Book->GetIdsInHTMLFiles();

    // Display filenames#ids in order starting with just filename
    foreach (Resource *resource, m_Resources) {
        QString filename = resource->Filename();
        QStringList ids = QStringList() << "" << id_names_hash[filename];

        foreach(QString id, ids) {
            QString target = filename;
            if (!id.isEmpty()) {
                target.append("#" % id);
            }

            QList<QStandardItem *> rowItems;

            QStandardItem *target_item = new QStandardItem();
            target_item->setText(target);
            rowItems << target_item;

            m_SelectHyperlinkModel->appendRow(rowItems);

            rowItems[0]->setEditable(false);
        }
    }
            
    // Set default href name
    ui.href->setText(m_DefaultHref);
}

QString SelectHyperlink::GetSelectedText()
{
    QStandardItem *item = NULL;

    if (ui.list->selectionModel()->hasSelection()) {
        QModelIndexList selected_indexes = ui.list->selectionModel()->selectedRows(0);
        if (!selected_indexes.isEmpty()) { item = m_SelectHyperlinkModel->itemFromIndex(selected_indexes.last());
        }
    }
    return item->text();
}

void SelectHyperlink::FilterEditTextChangedSlot(const QString &text)
{
    const QString lowercaseText = text.toLower();

    QStandardItem *root_item = m_SelectHyperlinkModel->invisibleRootItem();
    QModelIndex parent_index;

    // Hide rows that don't contain the filter text
    int first_visible_row = -1;
    for (int row = 0; row < root_item->rowCount(); row++) {
        if (text.isEmpty() || root_item->child(row, 0)->text().toLower().contains(lowercaseText)) {
            ui.list->setRowHidden(row, parent_index, false);
            if (first_visible_row == -1) {
                first_visible_row = row;
            }
        }
        else {
            ui.list->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.list->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    }
    else {
        // Clear current and selection, which clears preview image
        ui.list->setCurrentIndex(QModelIndex());
    }
}

QString SelectHyperlink::GetText()
{
    return ui.href->text();
}

void SelectHyperlink::DoubleClicked(const QModelIndex &index)
{
    QStandardItem *item = m_SelectHyperlinkModel->itemFromIndex(index);
    ui.href->setText(item->text());
    accept();
}
void SelectHyperlink::Clicked(const QModelIndex &index)
{
    QStandardItem *item = m_SelectHyperlinkModel->itemFromIndex(index);
    ui.href->setText(item->text());
}

void SelectHyperlink::ReadSettings()
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

void SelectHyperlink::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);

    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());

    settings.endGroup();
}

void SelectHyperlink::connectSignalsSlots()
{
    connect(this,         SIGNAL(accepted()),
            this,         SLOT(WriteSettings()));
    connect(ui.filter,    SIGNAL(textChanged(QString)),
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect (ui.list,     SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(DoubleClicked(const QModelIndex &)));
    connect (ui.list,     SIGNAL(clicked(const QModelIndex &)),
            this,         SLOT(Clicked(const QModelIndex &)));
}
