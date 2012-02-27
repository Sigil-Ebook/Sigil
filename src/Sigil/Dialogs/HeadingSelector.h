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
#ifndef HEADINGSELECTOR_H
#define HEADINGSELECTOR_H

#include <QtCore/QList>
#include <QtCore/QSharedPointer>
#include <QtGui/QDialog>
#include <QtGui/QStandardItemModel>

#include "ui_HeadingSelector.h"
#include "BookManipulation/Headings.h"

class Book;
class QStandardItem;

class HeadingSelector : public QDialog
{
    Q_OBJECT

public:

    // Constructor;
    // the first parameter is the book whose TOC
    // is being edited, the second is the dialog's parent
    HeadingSelector( QSharedPointer< Book > book, QWidget *parent = 0 );

    // Destructor
    ~HeadingSelector();

private slots:

    // We need to filter the calls to functions that would normally
    // connect directly to the itemChanged( QStandardItem* ) signal
    // because some would-be slots delete the item in question.
    // So, the signal connects here and this function calls the 
    // appropriate item-handling functions. 
    void ModelItemFilter( QStandardItem *item  );
    
    // Switches the display between showing all headings
    // and showing only headings that are to be included in the TOC
    void ChangeDisplayType( int new_check_state );

    // Updates the heading elements with new text
    // values and Sigil inclusion class
    void UpdateHeadingElements();

    // Selects headings to be included/excluded from TOC
    void SelectHeadingLevelInclusion( const QString& heading_level );

private:

    // We need this to be able to use a forward
    // declaration of Book in the QSharedPointer
    Q_DISABLE_COPY( HeadingSelector )

    void UpdateOneHeadingElement( QStandardItem *item );

    // Updates the inclusion of the heading in the TOC
    // whenever that heading's "include in TOC" checkbox
    // is checked/unchecked. 
    void UpdateHeadingInclusion( QStandardItem *checkbox_item );

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

    bool AddRowToVisiblePredecessorSucceeded( const QList< QStandardItem* > &child_row, 
                                              QStandardItem* row_parent );

    /**
     * Adds the child row to the "correct" item in the hierarchy below the
     * specified item. We are looking for the first previous item/heading
     * that has a lower level and that should be included in the TOC.
     *
     * @param item The item in which hierarchy the row should be placed.
     * @param child_row The row that we want to add.
     * @param child_index_limit If specified, then it's the index of the 
     *                          row's (disappearing) parent in row's grandparent
     *                          children list. We only look at children with
     *                          a lower index than this.
     */
    bool AddRowToCorrectItem( QStandardItem* item, 
                              const QList< QStandardItem* > &child_row,
                              int child_index_limit = -1 );

    QStandardItem* GetActualItemParent( const QStandardItem *item );

    Headings::Heading* GetItemHeading( const QStandardItem *item );    


    // Get the maximum heading level for all headings
    int GetMaxHeadingLevel( QList< Headings::Heading > flat_headings );

    // Add the selectable entries to the Select Heading combo box
    void PopulateSelectHeadingCombo( int max_heading_level );

    // Sets all headings to be included in or excluded from the TOC
    void SetAllHeadingInclusion( int upToLevel );

    // Sets one heading to be included in or excluded from the TOC
    void SetOneHeadingInclusion( Headings::Heading &heading, int upToLevel );

    // Reads all the stored dialog settings like
    // window position, geometry etc.
    void ReadSettings();

    // Writes all the stored dialog settings like
    // window position, geometry etc.
    void WriteSettings();

    void LockHTMLResources();

    void UnlockHTMLResources();

    void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // The book whose TOC is being edited
    QSharedPointer< Book > m_Book;

    // The model displayed and edited in the tree view
    QStandardItemModel m_TableOfContents;

    // The tree of all the headings in the book
    QList< Headings::Heading > m_Headings;

    // Holds all the widgets Qt Designer created for us
    Ui::HeadingSelector ui;
};


#endif // HEADINGSELECTOR_H

