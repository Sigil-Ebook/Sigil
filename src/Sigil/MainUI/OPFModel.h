/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#ifndef OPFMODEL_H
#define OPFMODEL_H

#include <QStandardItemModel>

class Book;

class OPFModel : public QStandardItemModel
{
    Q_OBJECT

public:

    OPFModel( QWidget *parent = 0 );

    void SetBook( Book &book );

    virtual void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );

    virtual Qt::DropActions supportedDropActions() const;

private:

    void ResetModel();

    void SortFilesByFilenames();

    void SortHTMLFilesByReadingOrder();

    void ClearModel();

    Book *m_Book;

    QStandardItem &m_TextFolderItem;
    QStandardItem &m_StylesFolderItem;
    QStandardItem &m_ImagesFolderItem;
    QStandardItem &m_FontsFolderItem;
    QStandardItem &m_MiscFolderItem;    
};


#endif // OPFMODEL_H