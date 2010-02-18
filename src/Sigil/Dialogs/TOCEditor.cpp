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

#include <stdafx.h>
#include "TOCEditor.h"
#include "../BookManipulation/Book.h"
#include "../Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"

static const QString SETTINGS_GROUP   = "toc_editor";
static const int FIRST_COLUMN_PADDING = 30;

// Constructor;
// the first parameter is the book whose TOC
// is being edited, the second is the dialog's parent
TOCEditor::TOCEditor( QSharedPointer< Book > book, QWidget *parent )
    : 
    QDialog( parent ),
    m_Book( book )
{
    ui.setupUi( this );
    ConnectSignalsToSlots();

    ui.tvTOCDisplay->setModel( &m_TableOfContents );

    LockHTMLResources();

    m_Headings = Headings::GetHeadingList( m_Book->mainfolder.GetSortedHTMLResources() );
    m_Headings = Headings::MakeHeadingHeirarchy( m_Headings );

    CreateTOCModel();

    // Set the initial display state
    if ( ui.cbTOCItemsOnly->checkState() == Qt::Checked ) 

        RemoveExcludedItems( m_TableOfContents.invisibleRootItem() );

    UpdateTreeViewDisplay();
    ReadSettings();
}


// Destructor
TOCEditor::~TOCEditor()
{
    WriteSettings();

    UnlockHTMLResources();
}


// We need to filter the calls to functions that would normally
// connect directly to the itemChanged( QStandardItem* ) signal
// because some would-be slots delete the item in question.
// So, the signal connects here and this function calls the 
// appropriate item-handling functions. 
void TOCEditor::ModelItemFilter( QStandardItem *item )
{
    Q_ASSERT( item );

    if ( item->isCheckable() == true )
    
        UpdateHeadingInclusion( item );
}


// Switches the display between showing all headings
// and showing only headings that are to be included in the TOC
void TOCEditor::ChangeDisplayType(  int new_check_state  )
{
    // If checked, show only TOC items
    if ( new_check_state == Qt::Checked )
    {
        RemoveExcludedItems( m_TableOfContents.invisibleRootItem() );
    }

    // If unchecked, show all items
    else
    {
        CreateTOCModel();

        UpdateTreeViewDisplay();       
    }
}


void TOCEditor::UpdateHeadingElements()
{
    // We recreate the model to make sure even those
    // headings marked as "don't include" are in the model.
    CreateTOCModel();

    UpdateOneHeadingElement( m_TableOfContents.invisibleRootItem() );
}

void TOCEditor::UpdateOneHeadingElement( QStandardItem *item )
{
    Headings::Heading *heading = item->data().value< Headings::HeadingPointer >().heading;

    if ( heading != NULL )
    {
        // Update heading text/value
        heading->element.setNodeValue( item->text() ); 

        // Update heading inclusion: if a heading element
        // has the NOT_IN_TOC_CLASS class, then it's not in the TOC
        QString class_attribute = heading->element.attribute( "class", "" )
                                  .remove( NOT_IN_TOC_CLASS )
                                  .simplified();

        if ( !heading->include_in_toc )
       
            class_attribute.append( " " + NOT_IN_TOC_CLASS );

        heading->element.setAttribute( "class", class_attribute );
        heading->resource_file->MarkSecondaryCachesAsOld();
    }

    if ( item->hasChildren() == true )
    {
        for ( int i = 0; i < item->rowCount(); ++i )
        {
            UpdateOneHeadingElement( item->child( i ) );                
        }
    }
}


// Updates the inclusion of the heading in the TOC
// whenever that heading's "include in TOC" checkbox
// is checked/unchecked. 
void TOCEditor::UpdateHeadingInclusion( QStandardItem *checkbox_item )
{
    Q_ASSERT( checkbox_item );

    // Working around Qt design issues
    // to get the actual item parent
    QStandardItem *item_parent = NULL;

    if ( checkbox_item->parent() == 0 )

        item_parent = m_TableOfContents.invisibleRootItem();

    else

        item_parent = checkbox_item->parent();

    Headings::Heading *heading = item_parent->child( checkbox_item->row(), 0 )->
        data().value< Headings::HeadingPointer >().heading;

    Q_ASSERT( heading );

    if ( checkbox_item->checkState() == Qt::Unchecked )

        heading->include_in_toc = false;

    else

        heading->include_in_toc = true;

    if ( ui.cbTOCItemsOnly->checkState() == Qt::Checked ) 

        RemoveExcludedItems( m_TableOfContents.invisibleRootItem() );    
}


// Updates the text in the heading if
// it has been changed in the editor
void TOCEditor::UpdateHeadingText( QStandardItem *text_item )
{
    Q_ASSERT( text_item );

    Headings::Heading *heading = text_item->data().value< Headings::HeadingPointer >().heading;

    Q_ASSERT( heading );

    heading->element.setNodeValue( text_item->text() );
}


// Updates the display of the tree view
// (resizes columns etc.)
void TOCEditor::UpdateTreeViewDisplay()
{      
    ui.tvTOCDisplay->expandAll();
    ui.tvTOCDisplay->resizeColumnToContents( 0 );
    ui.tvTOCDisplay->setColumnWidth( 0, ui.tvTOCDisplay->columnWidth( 0 ) + FIRST_COLUMN_PADDING );    
}


// Creates the model that is displayed
// in the tree view 
void TOCEditor::CreateTOCModel()
{
    m_TableOfContents.clear();

    QStringList header;

    header.append( tr( "TOC Entry" ) );
    header.append( tr( "Include" ) );

    m_TableOfContents.setHorizontalHeaderLabels( header );

    // Recursively inserts all headings
    for ( int i = 0; i < m_Headings.count(); ++i )
    {
        InsertHeadingIntoModel( m_Headings[ i ], m_TableOfContents.invisibleRootItem() );
    }
}


// Inserts the specified heading into the model
// as the child of the specified parent item;
// recursively calls itself on the headings children,
// thus building a TOC tree
void TOCEditor::InsertHeadingIntoModel( Headings::Heading &heading, QStandardItem *parent_item )
{
    Q_ASSERT( parent_item );

    QStandardItem *item_heading             = new QStandardItem( heading.element.text() );
    QStandardItem *heading_included_check   = new QStandardItem();

    heading_included_check->setEditable( false );
    heading_included_check->setCheckable( true );

    // TODO: red/pink background for include == false headings

    if ( heading.include_in_toc == true )

        heading_included_check->setCheckState( Qt::Checked );

    else

        heading_included_check->setCheckState( Qt::Unchecked );

    // Storing a pointer to the heading that
    // is represented by this QStandardItem
    Headings::HeadingPointer wrap;
    wrap.heading = &heading;

    item_heading->setData( QVariant::fromValue( wrap ) );

    QList< QStandardItem* > items;        
    items << item_heading << heading_included_check;

    parent_item->appendRow( items );

    if ( !heading.children.isEmpty() )
    {
        for ( int i = 0; i < heading.children.count(); ++i )
        {
            InsertHeadingIntoModel( heading.children[ i ], item_heading );
        }
    }
}


// Removes from the tree items that represent headings
// that are not to be included in the TOC; the children
// of those items rise to their parent's hierarchy level
void TOCEditor::RemoveExcludedItems( QStandardItem *item )
{
    Q_ASSERT( item );

    // Recursively call itself on the item's children
    if ( item->hasChildren() == true )
    {
        int row_index = 0;

        while ( row_index < item->rowCount() )
        {
            QStandardItem *oldchild = item->child( row_index );

            RemoveExcludedItems( item->child( row_index ) );

            // We only increment the row_index if the
            // RemoveExcludedItems operation didn't end up
            // removing the child at that index.. if it did,
            // the next child is now at that index.
            if ( oldchild == item->child( row_index ) )
            
                row_index++;
        }
    }

    // The root item is always present
    if ( item == m_TableOfContents.invisibleRootItem() )

        return;

    //    Unfortunately, while the invisible root of a QStandardItemModel
    // can have children, those children do not have a parent set.
    // Of course, their parent is the invisible root, but their
    // parent() function returns 0. So now you have *two* tree levels
    // for which parent() returns 0. This is clearly inconsistent
    // and makes the whole idea of using the same recursive
    // functions for tree traversal rather difficult to implement.
    // The only item for which parent() should return 0 should be 
    // the invisible root.
    //    Admittedly, Qt has some design issues. The next few lines
    // try to work around them by manually setting an item parent.

    QStandardItem *item_parent = NULL;

    if ( item->parent() == 0 )
    {
        item_parent = m_TableOfContents.invisibleRootItem();
    }  

    else
    {
        item_parent = item->parent();
    }

    // We query the "include in TOC" checkbox
    Qt::CheckState check_state = item_parent->child( item->row(), 1 )->checkState();

    // We remove the current item if it shouldn't
    // be included in the TOC
    if ( check_state == Qt::Unchecked )
    {
        if ( item->hasChildren() == true )
        {
            // Item pushes its children up to its parent
            while ( item->rowCount() > 0 )
            {
                item_parent->insertRow( item->row(), item->takeRow( 0 ) );
            }
        }

        // Item removes itself
        item_parent->removeRow( item->row() );
    }
}

// Reads all the stored dialog settings like
// window position, geometry etc.
void TOCEditor::ReadSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value( "geometry" ).toByteArray();

    if ( !geometry.isNull() )
    
        restoreGeometry( geometry );
}


// Writes all the stored dialog settings like
// window position, geometry etc.
void TOCEditor::WriteSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    settings.setValue( "geometry", saveGeometry() );
}


void TOCEditor::LockHTMLResources()
{
    foreach( HTMLResource* resource, m_Book->mainfolder.GetSortedHTMLResources() )
    {
        resource->GetLock().lockForWrite();
    }
}


void TOCEditor::UnlockHTMLResources()
{
    foreach( HTMLResource* resource, m_Book->mainfolder.GetSortedHTMLResources() )
    {
        resource->GetLock().unlock();
    }
}


void TOCEditor::ConnectSignalsToSlots()
{
    connect( &m_TableOfContents, SIGNAL( itemChanged( QStandardItem* ) ),
             this,               SLOT(   ModelItemFilter( QStandardItem* ) ) 
           );

    connect( ui.cbTOCItemsOnly,  SIGNAL( stateChanged( int ) ),
             this,               SLOT(   ChangeDisplayType( int ) ) 
           );

    connect( this,				 SIGNAL( accepted() ),
             this,               SLOT(   UpdateHeadingElements()	) 
             );
}


