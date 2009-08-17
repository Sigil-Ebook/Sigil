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
#ifndef TOCEDITOR_H
#define TOCEDITOR_H

#include <QtGui/QDialog>
#include <QStandardItemModel>

#include "ui_TOCEditor.h"
#include "Headings.h"

class Book;

class TOCEditor : public QDialog
{
    Q_OBJECT

public:

    // Constructor;
    // the first parameter is the book whose TOC
    // is being edited, the second is the dialog's parent
    TOCEditor( Book &book, QWidget *parent = 0 );

private slots:

    // Updates the inclusion of the heading in the TOC
    // whenever that heading's "include in TOC" checkbox
    // is checked/unchecked. 
    void UpdateHeadingInclusion( QStandardItem *checkbox_item );

    // Switches the display between showing all headings
    // and showing only headings that are to be included in the TOC
    void ChangeDisplayType( int new_check_state );

    // Updates the Book's XHTML source code
    // with the new information on headings
    void UpdateBookSource();

private:    
    
    // Updates the display of the tree view
    // (resizes columns etc.)
    void UpdateTreeViewDisplay();              

    // Creates the model that is displayed
    // in the tree view 
    void CreateTOCModel();

    // Inserts the specified heading into the model
    // as the child of the specified parent item;
    // recursively calls itself on the headings children,
    // thus building a TOC tree
    void InsertHeadingIntoModel( Headings::Heading &heading, QStandardItem *parent_item );

    // Removes from the tree items that represent headings
    // that are not to be included in the TOC; the children
    // of those items rise to their parent's hierarchy level
    void RemoveExcludedItems( QStandardItem *item );


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // The book whose TOC is being edited
    Book &m_Book;

    // The model displayed and edited in the tree view
    QStandardItemModel m_TableOfContents;

    // The tree of all the headings in the book
    QList< Headings::Heading > m_Headings;

    // Holds all the widgets Qt Designer created for us
    Ui::TOCEditor ui;
};


#endif // TOCEDITOR_H

