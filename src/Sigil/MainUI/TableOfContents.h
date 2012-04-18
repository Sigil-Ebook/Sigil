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

#include <QtCore/QSharedPointer>
#include <QtGui/QDockWidget>

#include "BookManipulation/Book.h"
#include "MainUI/NCXModel.h"

class QModelIndex;
class QPushButton;
class QTimer;
class QTreeView;
class QVBoxLayout;
class QWidget;

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

    /**
     * Starts the refresh operation, but on a delay.
     * Repeatedly calling this function resets the delay timer.
     */
    void StartRefreshDelay();

    /**
     * Refresh the TOC file contents to renumber entries
     */
    void RefreshTOCContents();

private slots:

    /**
     * Handles the mouse clicks in the TOC and causes
     * those clicks to open/activate files in the main view area.
     * 
     * @param index The model index of the item clicked.
     */
    void ItemClickedHandler( const QModelIndex &index );

    /**
     * Implements the "Generate TOC From headings" button functionality.
     */
    void GenerateTocFromHeadings();

    void GenerateInlineToc();

    void CollapseAll();

    void ExpandAll();

protected:

    void contextMenuEvent(QContextMenuEvent *event);

signals:

    /**
     * Emitted when the TOC wants a resource to be opened.
     *
     * @param resource The resource that should be opened.
     * @param precede_current_tab Should the new tab precede the currently opened one.
     * @param fragment The fragment ID to which the new tab should be scrolled.
     */
    void OpenResourceRequest( Resource &resource, bool precede_current_tab, const QUrl &fragment );

    void GenerateTocRequest();

    void GenerateInlineTocRequest(NCXModel::NCXEntry);

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
     * A container widget for the TOC UI widgets.
     */
    QWidget &m_MainWidget;
    
    /**
     * A container widget for the m_GenerateTocButton.
     * Used to work around a visual glitch on Mac OS X.
     * If we didn't use this, then we would have an ugly
     * margin on the left and right side of the m_TreeView.
     */
    QWidget &m_ButtonHolderWidget;
    
    /**
     * The layout for the container widget.
     */
    QVBoxLayout &m_Layout;

    /**
     * The tree view used to represent the TOC.
     */
    QTreeView &m_TreeView;

    /**
     * The button that initiates the TOC-generation-from-headings process.
     */
    QPushButton &m_GenerateTocButton;

    /**
     * The button that initiates the inline HTML TOC file generation process.
     */
    QPushButton &m_GenerateInlineTocButton;

    /**
     * The timer that provides the delay for the refresh operation.
     */
    QTimer &m_RefreshTimer;
    
    /**
     * The data model used to feed the tree view.
     */
    NCXModel &m_NCXModel;
};

#endif // TABLEOFCONTENTS_H


