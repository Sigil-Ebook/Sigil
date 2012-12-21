/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>

#include "BookManipulation/FolderKeeper.h"
#include "MainUI/TableOfContents.h"
#include "Misc/Utility.h"
#include "ResourceObjects/NCXResource.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

static const int COLUMN_INDENTATION = 10;
static const int REFRESH_DELAY = 1000;

TableOfContents::TableOfContents(QWidget *parent)
    :
    QDockWidget(tr("Table Of Contents"), parent),
    m_Book(NULL),
    m_MainWidget(*new QWidget(this)),
    m_Layout(*new QVBoxLayout(&m_MainWidget)),
    m_TreeView(*new QTreeView(&m_MainWidget)),
    m_RefreshTimer(*new QTimer(this)),
    m_NCXModel(*new NCXModel(this))
{
    m_Layout.setContentsMargins(0, 0, 0, 0);
#ifdef Q_WS_MAC
    m_Layout.setSpacing(4);
#endif
    m_Layout.addWidget(&m_TreeView);
    m_MainWidget.setLayout(&m_Layout);
    setWidget(&m_MainWidget);
    m_RefreshTimer.setInterval(REFRESH_DELAY);
    m_RefreshTimer.setSingleShot(true);
    SetupTreeView();
    connect(&m_TreeView, SIGNAL(clicked(const QModelIndex &)),
            this,        SLOT(ItemClickedHandler(const QModelIndex &)));
    connect(&m_RefreshTimer, SIGNAL(timeout()),
            this,            SLOT(Refresh()));
}

void TableOfContents::showEvent(QShowEvent *event)
{
    QDockWidget::showEvent(event);
    raise();
}

void TableOfContents::SetBook(QSharedPointer< Book > book)
{
    m_Book = book;
    m_NCXModel.SetBook(book);
    connect(&m_Book->GetNCX(), SIGNAL(Modified()),
            this,              SLOT(StartRefreshDelay()));
    Refresh();
}


void TableOfContents::Refresh()
{
    m_NCXModel.Refresh();
}


void TableOfContents::StartRefreshDelay()
{
    // Repeatedly calling start() will re-start the timer
    // and that's exactly what we want.
    // We want the timer to fire REFRESH_DELAY miliseconds
    // after the user has stopped typing up the NCX.
    m_RefreshTimer.start();
}


void TableOfContents::RenumberTOCContents()
{
    m_Book->GetNCX().GenerateNCXFromTOCContents(*m_Book, m_NCXModel);
}


void TableOfContents::ItemClickedHandler(const QModelIndex &index)
{
    QUrl url         = m_NCXModel.GetUrlForIndex(index);
    QString filename = QFileInfo(url.path()).fileName();

    try {
        Resource &resource = m_Book->GetFolderKeeper().GetResourceByFilename(filename);
        emit OpenResourceRequest(resource, -1, -1, QString(), MainWindow::ViewState_Unknown, url.fragment());
    } catch (const ResourceDoesNotExist &) {
        Utility::DisplayStdErrorDialog(
            tr("The file \"%1\" does not exist.")
            .arg(filename)
        );
    }
}


void TableOfContents::GenerateTocFromHeadings()
{
    emit GenerateTocRequest();
}


NCXModel::NCXEntry TableOfContents::GetRootEntry()
{
    return m_NCXModel.GetRootNCXEntry();
}


void TableOfContents::SetupTreeView()
{
    m_TreeView.setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_TreeView.setSortingEnabled(false);
    m_TreeView.sortByColumn(-1);
    m_TreeView.setUniformRowHeights(true);
    m_TreeView.setDragEnabled(false);
    m_TreeView.setAcceptDrops(false);
    m_TreeView.setDropIndicatorShown(false);
    m_TreeView.setDragDropMode(QAbstractItemView::NoDragDrop);
    m_TreeView.setAnimated(true);
    m_TreeView.setModel(&m_NCXModel);
    m_TreeView.setIndentation(COLUMN_INDENTATION);
    m_TreeView.setHeaderHidden(true);
}

void TableOfContents::CollapseAll()
{
    m_TreeView.collapseAll();
}

void TableOfContents::ExpandAll()
{
    m_TreeView.expandAll();
}

void TableOfContents::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    // Add menu options
    QAction *collapseAction = new QAction(tr("Collapse All"), menu);
    QAction *expandAction = new QAction(tr("Expand All"), menu);
    menu->addAction(collapseAction);
    connect(collapseAction, SIGNAL(triggered()), this, SLOT(CollapseAll()));
    menu->addAction(expandAction);
    connect(expandAction, SIGNAL(triggered()), this, SLOT(ExpandAll()));
    menu->exec(mapToGlobal(event->pos()));
}



