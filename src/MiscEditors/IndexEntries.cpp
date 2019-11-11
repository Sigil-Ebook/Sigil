/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2012      John Schember <john@nachtimwald.com>
**  Copyright (C) 2012      Dave Heiland
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

#include "MiscEditors/IndexEntries.h"
#include "MiscEditors/IndexEditorModel.h"

IndexEntries *IndexEntries::m_instance = 0;

IndexEntries *IndexEntries::instance()
{
    if (m_instance == 0) {
        m_instance = new IndexEntries();
    }

    return m_instance;
}

IndexEntries::IndexEntries()
    :
    m_BookIndexRootItem(new QStandardItem())
{
}

IndexEntries::~IndexEntries()
{
    if (m_instance) {
        delete m_instance;
        m_instance = 0;
    }
}

QStandardItem *IndexEntries::GetRootItem()
{
    return m_BookIndexRootItem;
}

void IndexEntries::AddOneEntry(QString text, QString bookpath, QString index_id_value)
{
    QStandardItem *parent_item = m_BookIndexRootItem;
    QStringList names = text.split("/", QString::SkipEmptyParts);
    names.append(bookpath % "#" % index_id_value);
    // Add names in heirachary
    foreach(QString name, names) {
        bool found = false;
        int insert_at_row = -1;

        for (int r = 0; r < parent_item->rowCount(); r++) {
            if (name == parent_item->child(r, 0)->text()) {
                parent_item = parent_item->child(r, 0);
                found = true;
                break;
            }
            // If not already in tree, add the entry in sorted order
            // Only sort categories - leave bookpath#id (last child) in order found
            else if (parent_item->child(0, 0)->rowCount() && (name.toLower().localeAwareCompare(parent_item->child(r, 0)->text().toLower()) < 0)) {
                insert_at_row = r ;
                break;
            }
        }

        if (!found) {
            parent_item = AddEntryToModel(name, parent_item, insert_at_row);
        }
    }
}


QStandardItem *IndexEntries::AddEntryToModel(QString entry, QStandardItem *parent_item, int row)
{
    if (!parent_item) {
        parent_item = m_BookIndexRootItem;
    }

    QList<QStandardItem *> rowItems;
    rowItems << new QStandardItem(entry);
    // Append or insert the item into the model as needed
    QStandardItem *new_item;

    if (row < 0 || row >= parent_item->rowCount()) {
        parent_item->appendRow(rowItems);
        new_item = parent_item->child(parent_item->rowCount() - 1, 0);
    } else {
        parent_item->insertRow(row, rowItems);
        new_item = parent_item->child(row, 0);
    }

    return new_item;
}

void IndexEntries::Clear()
{
    while (m_BookIndexRootItem->rowCount()) {
        m_BookIndexRootItem->removeRow(0);
    }
}
