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
#ifndef BOOKBROWSER_H
#define BOOKBROWSER_H

#include <QDockWidget>
#include <QSharedPointer>
#include "../BookManipulation/Book.h"

class QTreeView;
class OPFModel;
class HTMLResource;
class Resource;
class QModelIndex;
class QUrl;
class QPoint;
class QMenu;
class QAction;


class BookBrowser : public QDockWidget
{
    Q_OBJECT

public:

    BookBrowser( QWidget *parent = 0 );

public slots:

    void SetBook( QSharedPointer< Book > book );

    void Refresh();

    void OpenUrlResource( const QUrl &url );

signals:

    void ResourceDoubleClicked( Resource &resource );

    void OpenResourceRequest( Resource &resource, bool preceed_current_tab, const QUrl &fragment );

private slots:

    void EmitResourceDoubleClicked( const QModelIndex &index );

    void OpenContextMenu( const QPoint &point );

private:

    void SetupTreeView();

    void CreateActions();

    void SetupContextMenu();

    void ConnectSignalsToSlots();

    QSharedPointer< Book > m_Book;
    
    QTreeView &m_TreeView;
    
    OPFModel &m_OPFModel;

    QMenu &m_ContextMenu;
    QAction *m_AddExisting;
    QAction *m_AddNew;
    QAction *m_Rename;
    QAction *m_Remove;
};

#endif // BOOKBROWSER_H


