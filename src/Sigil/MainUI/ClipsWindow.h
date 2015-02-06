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

#pragma once
#ifndef CLIPSWINDOW_H
#define CLIPSWINDOW_H

#include <QtWidgets/QDockWidget>

#include "MiscEditors/ClipEditorModel.h"

class QTreeView;
class QVBoxLayout;

/**
 * Represents the pane in which the book's NCX TOC is rendered.
 * Clicking on an entry takes the user to that location.
 */
class ClipsWindow : public QDockWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The QObject's parent.
     */
    ClipsWindow(QWidget *parent = 0);

private slots:

    /**
     * Handles the mouse clicks in the TOC and causes
     * those clicks to open/activate files in the main view area.
     *
     * @param index The model index of the item clicked.
     */
    void ItemClickedHandler(const QModelIndex &index);

protected:

    void contextMenuEvent(QContextMenuEvent *event);
    virtual void showEvent(QShowEvent *event);

signals:
    void PasteClips(QList<ClipEditorModel::clipEntry *> clips);

private:

    /**
     * Performs the initial setup for the tree view.
     */
    void SetupTreeView();

    /**
     * A container widget for the TOC UI widgets.
     */
    QWidget &m_MainWidget;

    /**
     * The layout for the container widget.
     */
    QVBoxLayout &m_Layout;

    /**
     * The treeview used to represent the TOC.
     */
    QTreeView &m_TreeView;

    ClipEditorModel *m_ClipsModel;
};

#endif // CLIPSWINDOW_H


