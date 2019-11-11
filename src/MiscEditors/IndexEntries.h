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

#pragma once
#ifndef INDEXENTRIES_H
#define INDEXENTRIES_H

#include <QStandardItem>

/**
 *   Holds the Index entries to put into the Index
 */
class IndexEntries
{

public:
    static IndexEntries *instance();
    ~IndexEntries();

    void Clear();

    QStandardItem  *GetRootItem();

    void AddOneEntry(QString text, QString bookpath, QString index_id_value);

private:
    IndexEntries();

    QStandardItem *AddEntryToModel(QString entry, QStandardItem *parent_item, int row);

    QStandardItem *m_BookIndexRootItem;

    static IndexEntries *m_instance;
};

#endif // INDEXENTRIES_H
