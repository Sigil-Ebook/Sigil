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
#include "sigil_constants.h"

static QString SETTINGS_GROUP = "select_hyperlink";

SelectHyperlink::SelectHyperlink(QString default_href, HTMLResource *html_resource, QList<Resource *> resources, QSharedPointer<Book> book, QWidget *parent)
    :
    QDialog(parent),
    m_CurrentHTMLResource(html_resource),
    m_DefaultTarget(default_href),
    m_SavedTarget(QString()),
    m_Resources(resources),
    m_Book(book),
    m_SelectHyperlinkModel(new QStandardItemModel)
{
    ui.setupUi(this);
    connectSignalsSlots();
    ReadSettings();
    SetList();
    // Set default href name
    ui.href->setText(m_DefaultTarget);

    if (!m_DefaultTarget.isEmpty()) {
        SelectText(m_DefaultTarget);
    } else {
        SelectText(m_SavedTarget);
    }

    // Default entry on the target url not the filter
    ui.href->setFocus();
}

void SelectHyperlink::SetList()
{
    m_SelectHyperlinkModel->clear();
    QStringList header;
    header.append(tr("Targets in the Book"));
    m_SelectHyperlinkModel->setHorizontalHeaderLabels(header);
    ui.list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui.list->setModel(m_SelectHyperlinkModel);
    // Get the complete list of valid targets
    m_IDNames = m_Book->GetIdsInHTMLFiles();
    // Display in-file targets first, then in order
    AddEntry(m_CurrentHTMLResource);
    foreach(Resource * resource, m_Resources) {
        if (resource != m_CurrentHTMLResource) {
            AddEntry(resource);
        }
    }
}

void SelectHyperlink::AddEntry(Resource *resource)
{
    if (!resource) {
        return;
    }

    QString filename = resource->Filename();
    QStringList ids = QStringList() << "" << m_IDNames[filename];
    foreach(QString id, ids) {
        // Do not allow linking to index entries because they can be regenerated
        // and because they can take up a lot of room.
        if (id.startsWith(SIGIL_INDEX_ID_PREFIX)) {
            continue;
        }

        QString target = filename;
        QString filepath;
        // Only relative paths if inserting hyperlink not editing TOC
        if (m_CurrentHTMLResource) {
            filepath = "../";
        }
        filepath += resource->GetRelativePathToOEBPS();

        if (!id.isEmpty()) {
            QString fragment = "#" % id;
            filepath.append(fragment);

            // Show the short version if this is the same file
            if (m_CurrentHTMLResource && filename == m_CurrentHTMLResource->Filename()) {
                target = fragment;
            } else {
                target.append(fragment);
            }
        }

        QList<QStandardItem *> rowItems;
        QStandardItem *target_item = new QStandardItem();
        target_item->setText(target);
        target_item->setData(filepath);
        rowItems << target_item;
        m_SelectHyperlinkModel->appendRow(rowItems);
        rowItems[0]->setEditable(false);
    }
}

QString SelectHyperlink::GetSelectedText()
{
    QString text;

    if (ui.list->selectionModel()->hasSelection()) {
        QModelIndexList selected_indexes = ui.list->selectionModel()->selectedRows(0);

        if (!selected_indexes.isEmpty()) {
            QStandardItem *item = m_SelectHyperlinkModel->itemFromIndex(selected_indexes.last());

            if (item) {
                text = item->text();
            }
        }
    }

    return text;
}

void SelectHyperlink::SelectText(QString &text)
{
    if (!text.isEmpty()) {
        QModelIndex parent_index;
        QStandardItem *root_item = m_SelectHyperlinkModel->invisibleRootItem();
        // Convert search text to filename#fragment
        QString target = text;

        if (target.startsWith("#") && m_CurrentHTMLResource) {
            target = m_CurrentHTMLResource->Filename() + text;
        }

        if (target.contains("/")) {
            target = target.right(target.length() - target.lastIndexOf("/") - 1);
        }

        for (int row = 0; row < root_item->rowCount(); row++) {
            QStandardItem *child = root_item->child(row, 0);
            // Convert selection text to filename#fragment
            QString selection = child->data().toString();

            if (selection.contains("/")) {
                selection = selection.right(selection.length() - selection.lastIndexOf("/") - 1);
            }

            if (target == selection) {
                ui.list->selectionModel()->select(m_SelectHyperlinkModel->index(row, 0, parent_index), QItemSelectionModel::Select | QItemSelectionModel::Rows);
                ui.list->setFocus();
                ui.list->setCurrentIndex(child->index());
            }
        }
    }
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
        } else {
            ui.list->setRowHidden(row, parent_index, true);
        }
    }

    if (!text.isEmpty() && first_visible_row != -1) {
        // Select the first non-hidden row
        ui.list->setCurrentIndex(root_item->child(first_visible_row, 0)->index());
    } else {
        // Clear current and selection, which clears preview image
        ui.list->setCurrentIndex(QModelIndex());
    }
}

QString SelectHyperlink::GetTarget()
{
    QString target;

    target = ui.href->text();

    return target;
}

void SelectHyperlink::DoubleClicked(const QModelIndex &index)
{
    Clicked(index);
    accept();
}

void SelectHyperlink::Clicked(const QModelIndex &index)
{
    QStandardItem *item = m_SelectHyperlinkModel->itemFromIndex(index);

    if (item->text().startsWith("#")) {
        ui.href->setText(item->text());
    } else {
        ui.href->setText(item->data().toString());
    }
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

    m_SavedTarget = settings.value("lastselectedentry").toString();
    settings.endGroup();
}

void SelectHyperlink::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    settings.setValue("geometry", saveGeometry());
    settings.setValue("lastselectedentry", GetSelectedText());
    settings.endGroup();
}

void SelectHyperlink::connectSignalsSlots()
{
    connect(this,         SIGNAL(accepted()),
            this,         SLOT(WriteSettings()));
    connect(ui.filter,    SIGNAL(textChanged(QString)),
            this,         SLOT(FilterEditTextChangedSlot(QString)));
    connect(ui.list,     SIGNAL(doubleClicked(const QModelIndex &)),
            this,         SLOT(DoubleClicked(const QModelIndex &)));
    connect(ui.list,     SIGNAL(clicked(const QModelIndex &)),
            this,         SLOT(Clicked(const QModelIndex &)));
}
