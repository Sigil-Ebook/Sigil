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
#include "BookBrowser.h"
#include "OPFModel.h"
#include "../BookManipulation/Book.h"
#include <QTreeView>
#include "../Misc/Utility.h"

// We will add a few spaces to the front so the title isn't
// glued to the widget side when it's docked. Ugly, but works.
static const QString DOCK_WIDGET_TITLE = QObject::tr( "Book Browser" );
static const int COLUMN_INDENTATION = 10;


BookBrowser::BookBrowser( QWidget *parent )
    : 
    QDockWidget( "   " + DOCK_WIDGET_TITLE, parent ),
    m_TreeView( *new QTreeView( this ) ),
    m_OPFModel( *new OPFModel( this ) )
{   
    setWidget( &m_TreeView );

    setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable );
    setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

    m_TreeView.setEditTriggers( QAbstractItemView::EditKeyPressed );
    m_TreeView.setSortingEnabled( false );
    m_TreeView.sortByColumn( -1 );
    m_TreeView.setUniformRowHeights( true );
    m_TreeView.setDragEnabled( true );
    m_TreeView.setAcceptDrops( false );
    m_TreeView.setDropIndicatorShown( true );
    m_TreeView.setDragDropMode( QAbstractItemView::InternalMove );
    //m_TreeView.setSelectionMode( QAbstractItemView::SingleSelection );
    
    m_TreeView.setModel( &m_OPFModel ); 

    for ( int i = 1; i < m_OPFModel.columnCount(); ++i )
    {
        m_TreeView.hideColumn( i );
    }

    m_TreeView.setIndentation( COLUMN_INDENTATION );
    m_TreeView.setHeaderHidden( true );

    connect( &m_TreeView, SIGNAL( doubleClicked(             const QModelIndex& ) ), 
            this,         SLOT(   EmitResourceDoubleClicked( const QModelIndex& ) ) );
}


void BookBrowser::SetBook( Book &book )
{
    m_Book = &book;

    m_OPFModel.SetBook( book );

    // TODO: fake that the "first" HTML file has been double clicked
    // so that we have a default first tab opened
}

void BookBrowser::OpenUrlResource( const QUrl &url )
{
    try
    {
        Resource &resource = m_Book->mainfolder.GetResourceByFilename( QFileInfo( url.path() ).fileName() );

        emit OpenResourceRequest( resource, url.fragment() );
    }

    catch ( const ExceptionBase &exception )
    {
        Utility::DisplayStdErrorDialog( Utility::GetExceptionInfo( exception ) );
    }       
}


void BookBrowser::EmitResourceDoubleClicked( const QModelIndex &index )
{
    QString identifier( m_OPFModel.itemFromIndex( index )->data().toString() );  

    if ( !identifier.isEmpty() )

        emit ResourceDoubleClicked( m_Book->mainfolder.GetResourceByIdentifier( identifier ) );
}
