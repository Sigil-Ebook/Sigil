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
#ifndef TABLEOFCONTENTS_H
#define TABLEOFCONTENTS_H

#include <QDockWidget>
#include <QSharedPointer>
#include "BookManipulation/Book.h"

class NCXModel;
class QTreeView;
class QModelIndex;


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
    TableOfContents( QWidget *parent = 0 );

public slots:

    /**
     * Sets the book whose TOC the should be displayed.
     *
     * @param book The book to be displayed.
     */
    void SetBook( QSharedPointer< Book > book );

    /**
     * Refreshes the display of the book's TOC.
     */
    void Refresh();

private slots:

    /**
     * Handles the mouse clicks in the TOC and causes
     * those clicks to open/activate files in the main view area.
     * 
     * @param index The model index of the item clicked.
     */
    void ItemClickedHandler( const QModelIndex &index );

signals:

    /**
     * Emitted when the TOC wants a resource to be opened.
     *
     * @param resource The resource that should be opened.
     * @param precede_current_tab Should the new tab precede the currently opened one.
     * @param fragment The fragment ID to which the new tab should be scrolled.
     */
    void OpenResourceRequest( Resource &resource, bool precede_current_tab, const QUrl &fragment );

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
    QSharedPointer< Book > m_Book;

    /**
     * The tree view used to represent the TOC.
     */
    QTreeView &m_TreeView;
    
    /**
     * The data model used to feed the tree view.
     */
    NCXModel &m_NCXModel;
};

#endif // TABLEOFCONTENTS_H


