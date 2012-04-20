/************************************************************************
**
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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

#include <QtGui/QStandardItem>

#include "Dialogs/LinkStylesheets.h"
#include "Misc/SettingsStore.h"
#include "sigil_constants.h"

static const QString SETTINGS_GROUP   = "link_stylesheets";

// Constructor;
LinkStylesheets::LinkStylesheets( QList<std::pair<QString, bool> > stylesheets_map, QWidget *parent )
    : 
    QDialog( parent ),
    m_StylesheetsMap( stylesheets_map )
{
    ui.setupUi( this );
    ConnectSignalsToSlots();

    ui.StylesheetsView->setModel( &m_StylesheetsModel );

    CreateStylesheetsModel();

    UpdateTreeViewDisplay();

    ReadSettings();
}


// Updates the display of the tree view (resize columns)
void LinkStylesheets::UpdateTreeViewDisplay()
{      
    ui.StylesheetsView->expandAll();
    ui.StylesheetsView->resizeColumnToContents( 0 );
    ui.StylesheetsView->setColumnWidth( 0, ui.StylesheetsView->columnWidth( 0 ) );    

    ui.StylesheetsView->setCurrentIndex( m_StylesheetsModel.index(0,0) );
}

// Creates the model that is displayed in the tree view
void LinkStylesheets::CreateStylesheetsModel()
{
    m_StylesheetsModel.clear();

    QStringList header;

    header.append( tr( "Include" ) );
    header.append( tr( "Stylesheet" ) );

    m_StylesheetsModel.setHorizontalHeaderLabels( header );

    // Inserts all entries
    for ( int i = 0; i < m_StylesheetsMap.count(); i++ )
    {
        InsertStylesheetIntoModel( m_StylesheetsMap.at( i ) );
    }
}


// Inserts the specified heading into the model
void LinkStylesheets::InsertStylesheetIntoModel( std::pair<QString, bool> stylesheet_pair )
{
    QStandardItem *item_filename       = new QStandardItem( stylesheet_pair.first );
    QStandardItem *item_included_check = new QStandardItem();

    item_included_check->setEditable( false );
    item_included_check->setCheckable( true );
    item_filename->setEditable( false );
    item_filename->setDragEnabled( false );
    item_filename->setDropEnabled( false );

    if ( stylesheet_pair.second  )
    {
        item_included_check->setCheckState( Qt::Checked );
    }
    else
    {
        item_included_check->setCheckState( Qt::Unchecked );
    }

    QList< QStandardItem* > items;        
    items << item_included_check << item_filename;

    m_StylesheetsModel.invisibleRootItem()->appendRow( items );
}


// Reads all the stored dialog settings like window position, size, etc.
void LinkStylesheets::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value( "geometry" ).toByteArray();

    if ( !geometry.isNull() )
    {
        restoreGeometry( geometry );
    }

    settings.endGroup();
}


// Writes all the stored dialog settings like window position, size, etc.
void LinkStylesheets::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    settings.setValue( "geometry", saveGeometry() );

    settings.endGroup();
}

void LinkStylesheets::MoveUp()
{
    QModelIndexList selected_indexes = ui.StylesheetsView->selectionModel()->selectedIndexes();

    if ( selected_indexes.isEmpty() )
    {
        return;
    }

    QModelIndex index = selected_indexes.first();
    int row = index.row();
    if ( row == 0 )
    {
        return;
    }

    QList< QStandardItem* > items =  m_StylesheetsModel.invisibleRootItem()->takeRow( row - 1 );       
    
    m_StylesheetsModel.invisibleRootItem()->insertRow( row, items );
}

void LinkStylesheets::MoveDown()
{
    QModelIndexList selected_indexes = ui.StylesheetsView->selectionModel()->selectedIndexes();

    if ( selected_indexes.isEmpty() )
    {
        return;
    }

    QModelIndex index = selected_indexes.first();
    int row = index.row();
    if ( row == m_StylesheetsModel.invisibleRootItem()->rowCount() - 1 )
    {
        return;
    }

    QList< QStandardItem* > items =  m_StylesheetsModel.invisibleRootItem()->takeRow( row + 1 );       
    
    m_StylesheetsModel.invisibleRootItem()->insertRow( row, items );
}


void LinkStylesheets::UpdateStylesheets()
{
    m_Stylesheets.clear();

    int rows = m_StylesheetsModel.invisibleRootItem()->rowCount();
    for ( int row = 0; row < rows; row++ )
    {
        QList< QStandardItem* > items =  m_StylesheetsModel.invisibleRootItem()->takeRow( 0 );       

        if ( items.at( 0 )->checkState() == Qt::Checked )
        {
            m_Stylesheets << items.at( 1 )->data( Qt::DisplayRole ).toString();
        }
    }
}

QStringList LinkStylesheets::GetStylesheets()
{
    return m_Stylesheets;
}

void LinkStylesheets::ConnectSignalsToSlots()
{
    connect( ui.MoveUp, SIGNAL( clicked() ),  this, SLOT( MoveUp() ));
    connect( ui.MoveDown, SIGNAL( clicked() ),  this, SLOT( MoveDown() ));
    connect( this, SIGNAL( accepted() ), this, SLOT( UpdateStylesheets() ) );
    connect( this, SIGNAL( accepted() ), this, SLOT( WriteSettings() ) );
}
