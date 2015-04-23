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

#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtGui/QContextMenuEvent>
#include <QtWidgets/QAction>
#include <QtWidgets/QMenu>

#include "MainUI/ClipsWindow.h"

static const int COLUMN_INDENTATION = 15;

ClipsWindow::ClipsWindow(QWidget *parent)
    :
    QDockWidget(tr("Clips"), parent),
    m_MainWidget(new QWidget(this)),
    m_Layout(new QVBoxLayout(m_MainWidget)),
    m_TreeView(new QTreeView(m_MainWidget))
{
    m_Layout->setContentsMargins(0, 0, 0, 0);
#ifdef Q_OS_MAC
    m_Layout->setSpacing(4);
#endif
    m_Layout->addWidget(m_TreeView);
    m_MainWidget->setLayout(m_Layout);
    setWidget(m_MainWidget);

    SetupTreeView();

    connect(m_TreeView, SIGNAL(clicked(const QModelIndex &)),
            this,        SLOT(ItemClickedHandler(const QModelIndex &)));
}

void ClipsWindow::showEvent(QShowEvent *event)
{
    QDockWidget::showEvent(event);
    raise();
}

void ClipsWindow::SetupTreeView()
{
    m_ClipsModel = ClipEditorModel::instance();
    m_TreeView->setModel(m_ClipsModel);
    m_TreeView->setSortingEnabled(false);
    m_TreeView->setWordWrap(false);
    m_TreeView->setAlternatingRowColors(false);

    m_TreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_TreeView->sortByColumn(-1);
    m_TreeView->setUniformRowHeights(true);
    m_TreeView->setDragEnabled(false);
    m_TreeView->setAcceptDrops(false);
    m_TreeView->setDropIndicatorShown(false);
    m_TreeView->setDragDropMode(QAbstractItemView::NoDragDrop);
    m_TreeView->setAnimated(true);
    m_TreeView->setIndentation(COLUMN_INDENTATION);
    m_TreeView->setHeaderHidden(true);

    m_TreeView->setColumnHidden(1, true);

    m_TreeView->expandAll();
}


void ClipsWindow::ItemClickedHandler(const QModelIndex &index)
{
    ClipEditorModel::clipEntry *clip = ClipEditorModel::instance()->GetEntry(index);

    QList<ClipEditorModel::clipEntry *> clips;
    clips.append(clip);

    emit PasteClips(clips);
}


void ClipsWindow::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = new QMenu(this);
    // Add menu options
    QAction *collapseAction = new QAction(tr("Collapse All"), menu);
    QAction *expandAction = new QAction(tr("Expand All"), menu);
    menu->addAction(collapseAction);
    connect(collapseAction, SIGNAL(triggered()), m_TreeView, SLOT(collapseAll()));
    menu->addAction(expandAction);
    connect(expandAction, SIGNAL(triggered()), m_TreeView, SLOT(expandAll()));
    menu->exec(mapToGlobal(event->pos()));
}
