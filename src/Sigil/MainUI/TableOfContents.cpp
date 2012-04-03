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

#include <QtCore/QFileInfo>
#include <QtCore/QTimer>
#include <QtGui/QPushButton>
#include <QtGui/QTreeView>
#include <QtGui/QVBoxLayout>

#include "BookManipulation/BookNormalization.h"
#include "BookManipulation/FolderKeeper.h"
#include "MainUI/TableOfContents.h"
#include "Misc/Utility.h"
#include "ResourceObjects/NCXResource.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

static const int COLUMN_INDENTATION = 10;
static const int REFRESH_DELAY = 1000;

TableOfContents::TableOfContents( QWidget *parent )
    : 
    QDockWidget( tr( "Table of Contents" ), parent ),
    m_Book( NULL ),
    m_MainWidget( *new QWidget( this ) ),
    m_ButtonHolderWidget( *new QWidget( &m_MainWidget ) ),
    m_Layout( *new QVBoxLayout( &m_MainWidget ) ),
    m_TreeView( *new QTreeView( &m_MainWidget ) ),
    m_GenerateTocButton( *new QPushButton( tr( "Generate TOC from headings" ), &m_ButtonHolderWidget ) ),
    m_GenerateInlineTocButton(*new QPushButton(tr("Generate Inline HTML TOC"), &m_ButtonHolderWidget)),
    m_RefreshTimer( *new QTimer( this ) ),
    m_NCXModel( *new NCXModel( this ) )
{
    m_Layout.setContentsMargins( 0, 0, 0, 0 );
    
#ifdef Q_WS_MAC
    m_Layout.setSpacing( 4 );
#endif
    
    QVBoxLayout *layout = new QVBoxLayout( &m_ButtonHolderWidget );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->addWidget( &m_GenerateTocButton );
    layout->addWidget(&m_GenerateInlineTocButton);
    m_ButtonHolderWidget.setLayout( layout );
    
    m_Layout.addWidget( &m_TreeView );
    m_Layout.addWidget( &m_ButtonHolderWidget );
    
    m_MainWidget.setLayout( &m_Layout );

    setWidget( &m_MainWidget );

    m_RefreshTimer.setInterval( REFRESH_DELAY );
    m_RefreshTimer.setSingleShot( true );
        
    SetupTreeView();

    connect( &m_TreeView, SIGNAL( clicked(            const QModelIndex& ) ), 
             this,        SLOT(   ItemClickedHandler( const QModelIndex& ) ) );

    connect( &m_GenerateTocButton, SIGNAL( clicked() ), 
             this,                 SLOT( GenerateTocFromHeadings() ) );

    connect(&m_GenerateInlineTocButton, SIGNAL(clicked()), this, SLOT(GenerateInlineToc()));

    connect( &m_RefreshTimer, SIGNAL( timeout() ), 
             this,            SLOT( Refresh() ) );
}


void TableOfContents::SetBook( QSharedPointer< Book > book )
{
    m_Book = book;
    m_NCXModel.SetBook( book );

    connect( &m_Book->GetNCX(), SIGNAL( Modified()), 
             this,              SLOT( StartRefreshDelay() ) );

    Refresh();
}


void TableOfContents::Refresh()
{
    m_NCXModel.Refresh();
}


void TableOfContents::StartRefreshDelay()
{
    // Repeatedly calling start() will re-start the timer
    // and that's exactly what we want.
    // We want the timer to fire REFRESH_DELAY miliseconds
    // after the user has stopped typing up the NCX.
    m_RefreshTimer.start();
}


void TableOfContents::RefreshTOCContents()
{
    m_Book->GetNCX().GenerateNCXFromTOCContents( *m_Book, m_NCXModel );
}


void TableOfContents::ItemClickedHandler( const QModelIndex &index )
{
    QUrl url         = m_NCXModel.GetUrlForIndex( index );
    QString filename = QFileInfo( url.path() ).fileName();

    try
    {
        Resource &resource = m_Book->GetFolderKeeper().GetResourceByFilename( filename );

        emit OpenResourceRequest( resource, false, url.fragment() );
    }

    catch ( const ResourceDoesNotExist& )
    {
        Utility::DisplayStdErrorDialog( 
            tr( "The file \"%1\" does not exist." )
            .arg( filename )
            );
    }       
}


void TableOfContents::GenerateTocFromHeadings()
{
    emit GenerateTocRequest();
}

void TableOfContents::GenerateInlineToc()
{
    emit GenerateInlineTocRequest(m_NCXModel.GetRootNCXEntry());
}


void TableOfContents::SetupTreeView()
{
    m_TreeView.setEditTriggers( QAbstractItemView::NoEditTriggers );
    m_TreeView.setSortingEnabled( false );
    m_TreeView.sortByColumn( -1 );
    m_TreeView.setUniformRowHeights( true );
    m_TreeView.setDragEnabled( false );
    m_TreeView.setAcceptDrops( false );
    m_TreeView.setDropIndicatorShown( false );
    m_TreeView.setDragDropMode( QAbstractItemView::NoDragDrop );
    m_TreeView.setAnimated( true );

    m_TreeView.setModel( &m_NCXModel ); 

    m_TreeView.setIndentation( COLUMN_INDENTATION );
    m_TreeView.setHeaderHidden( true );
}



