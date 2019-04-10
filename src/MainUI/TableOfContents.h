/************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks, Stratford, Ontario, Canada
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
#ifndef TABLEOFCONTENTS_H
#define TABLEOFCONTENTS_H

#include <QtCore/QSharedPointer>
#include <QtWidgets/QDockWidget>

#include "BookManipulation/Book.h"
#include "MainUI/MainWindow.h"
#include "MainUI/TOCModel.h"

class QTimer;
class QTreeView;
class QVBoxLayout;

class Book;

/**
 * Represents the pane in which the book's NCX TOC is rendered.
 * Clicking on an entry takes the user to that location.
 */
class TableOfContents : public QDockWidget
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param parent The QObject's parent.
     */
    TableOfContents(QWidget *parent = 0);

public slots:

    /**
     * Sets the book whose TOC the should be displayed.
     *
     * @param book The book to be displayed.
     */
    void SetBook(QSharedPointer<Book> book);

    /**
     * Refreshes the display of the book's TOC.
     */
    void Refresh();

    /**
     * Starts the refresh operation, but on a delay.
     * Repeatedly calling this function resets the delay timer.
     */
    void StartRefreshDelay();

    /**
     * Refresh the TOC file contents to renumber entries
     */
    void RenumberTOCContents();

    TOCModel::TOCEntry GetRootEntry();

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

    /**
     * Emitted when the TOC wants a resource to be opened.
     *
     * @param resource The resource that should be opened.
     * @param fragment The fragment ID to which the new tab should be scrolled.
     */
    void OpenResourceRequest(Resource *resource,
                             int line_to_scroll_to = -1,
                             int position_to_scroll_to = -1,
                             const QString &caret_location_to_scroll_to = QString(),
                             const QUrl &fragment = QUrl());

private:

    /**
     * Performs the initial setup for the tree view.
     */
    void SetupTreeView();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The book currently being displayed.
     */
    QSharedPointer<Book> m_Book;

    /**
     * A container widget for the TOC UI widgets.
     */
    QWidget *m_MainWidget;

    /**
     * The layout for the container widget.
     */
    QVBoxLayout *m_Layout;

    /**
     * The tree view used to represent the TOC.
     */
    QTreeView *m_TreeView;

    /**
     * The timer that provides the delay for the refresh operation.
     */
    QTimer m_RefreshTimer;

    /**
     * The data model used to feed the tree view.
     */
    TOCModel *m_TOCModel;

    QString m_EpubVersion;

};

#endif // TABLEOFCONTENTS_H
