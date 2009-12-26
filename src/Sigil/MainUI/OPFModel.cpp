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
#include "OPFModel.h"
#include "../BookManipulation/Book.h"
#include "../ResourceObjects/Resource.h"
#include "../ResourceObjects/HTMLResource.h"
#include <limits>

static const int NO_READING_ORDER   = std::numeric_limits< int >::max();
static const int READING_ORDER_ROLE = Qt::UserRole + 2;

OPFModel::OPFModel( QWidget *parent )
    : 
    QStandardItemModel( parent ),
    m_TextFolderItem(   *new QStandardItem( TEXT_FOLDER_NAME  ) ),
    m_StylesFolderItem( *new QStandardItem( STYLE_FOLDER_NAME ) ),
    m_ImagesFolderItem( *new QStandardItem( IMAGE_FOLDER_NAME ) ),
    m_FontsFolderItem(  *new QStandardItem( FONT_FOLDER_NAME  ) ),
    m_MiscFolderItem(   *new QStandardItem( MISC_FOLDER_NAME  ) )
{
    QList< QStandardItem* > items;

    items.append( &m_TextFolderItem   );
    items.append( &m_StylesFolderItem );
    items.append( &m_ImagesFolderItem );
    items.append( &m_FontsFolderItem  );
    items.append( &m_MiscFolderItem   );

    QIcon folder_icon = QFileIconProvider().icon( QFileIconProvider::Folder );

    foreach( QStandardItem *item, items )
    {
        item->setIcon( folder_icon );
        item->setEditable( false );
        item->setDragEnabled( false );
        item->setDropEnabled( false );
        appendRow( item );
    }    

    // We enable reordering of files in the text folder
    m_TextFolderItem.setDropEnabled( true );
    invisibleRootItem()->setDropEnabled( false );
}

void OPFModel::SetBook( Book &book )
{
    m_Book = &book;

    ResetModel();

    SortFilesByFilenames();
    SortHTMLFilesByReadingOrder();
}


void OPFModel::sort( int column, Qt::SortOrder order )
{
    return;
}


Qt::DropActions OPFModel::supportedDropActions() const
{
    return Qt::MoveAction;
}


void OPFModel::ResetModel()
{
    Q_ASSERT( m_Book );

    ClearModel();

    QList< Resource* > resources = m_Book->mainfolder.GetResourceList();
    QHash< int, QStandardItem* > text_items;

    int num_ordered_html_files = 0;

    foreach( Resource *resource, resources )
    {
        QStandardItem *item = new QStandardItem( resource->Icon(), resource->Filename() );
        item->setDropEnabled( false );
        item->setData( resource->GetIdentifier() );
        
        if ( resource->Type() == Resource::HTMLResource )
        {
            int reading_order = qobject_cast< HTMLResource* >( resource )->GetReadingOrder();

            if ( reading_order == -1 )
            
                reading_order = NO_READING_ORDER;

            item->setData( reading_order, READING_ORDER_ROLE );
            m_TextFolderItem.appendRow( item );
        }

        else if ( resource->Type() == Resource::CSSResource || resource->Type() == Resource::XPGTResource )
        {
            m_StylesFolderItem.appendRow( item );
        }

        else if ( resource->Type() == Resource::ImageResource )
        {
            m_ImagesFolderItem.appendRow( item );
        }

        else if ( resource->Type() == Resource::FontResource )
        {
            m_FontsFolderItem.appendRow( item );
        }

        else
        {
            m_MiscFolderItem.appendRow( item );        
        }
    }
}

void OPFModel::ClearModel()
{
    while ( m_TextFolderItem.rowCount() != 0 )
    {
        m_TextFolderItem.removeRow( 0 );
    }

    while ( m_StylesFolderItem.rowCount() != 0 )
    {
        m_StylesFolderItem.removeRow( 0 );
    }

    while ( m_ImagesFolderItem.rowCount() != 0 )
    {
        m_ImagesFolderItem.removeRow( 0 );
    }

    while ( m_FontsFolderItem.rowCount() != 0 )
    {
        m_FontsFolderItem.removeRow( 0 );
    }

    while ( m_MiscFolderItem.rowCount() != 0 )
    {
        m_MiscFolderItem.removeRow( 0 );
    }
}

void OPFModel::SortFilesByFilenames()
{
    for ( int i = 0; i < invisibleRootItem()->rowCount(); ++i )
    {
        invisibleRootItem()->child( i )->sortChildren( 0 );
    }
}

void OPFModel::SortHTMLFilesByReadingOrder()
{
    int old_sort_role = sortRole();
    setSortRole( READING_ORDER_ROLE );

    m_TextFolderItem.sortChildren( 0 );

    setSortRole( old_sort_role );
}