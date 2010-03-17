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
#include "../Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "../Importers/ImportHTML.h"
#include <QTreeView>

static const QString SETTINGS_GROUP = "bookbrowser";

// We will add a few spaces to the front so the title isn't
// glued to the widget side when it's docked. Ugly, but works.
static const QString DOCK_WIDGET_TITLE = QObject::tr( "Book Browser" );
static const int COLUMN_INDENTATION = 10;


BookBrowser::BookBrowser( QWidget *parent )
    : 
    QDockWidget( "   " + DOCK_WIDGET_TITLE, parent ),
    m_TreeView( *new QTreeView( this ) ),
    m_OPFModel( *new OPFModel( this ) ),
    m_ContextMenu( *new QMenu( this ) ),
    m_LastContextMenuType( Resource::GenericResource )
{   
    setWidget( &m_TreeView );

    setFeatures( QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable );
    setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );

    ReadSettings();

    SetupTreeView();
    CreateContextMenuActions();
    ConnectSignalsToSlots();
}


BookBrowser::~BookBrowser()
{
    WriteSettings();
}


void BookBrowser::SetBook( QSharedPointer< Book > book )
{
    m_Book = book;

    m_OPFModel.SetBook( book );

    try
    {
        // Here we fake that the "first" HTML file has been double clicked
        // so that we have a default first tab opened.
        // An exception is thrown if there are no HTML files in the epub.
        EmitResourceDoubleClicked( m_OPFModel.GetFirstHTMLModelIndex() );
    }
    
    // No exception variable since we don't use it
    catch ( NoHTMLFiles& )
    {
       // Do nothing. No HTML files, no first file opened.
    }    
}


void BookBrowser::Refresh()
{
    int scroll_value = m_TreeView.verticalScrollBar()->value();
    m_OPFModel.Refresh();
    m_TreeView.verticalScrollBar()->setValue( scroll_value );
}


void BookBrowser::OpenUrlResource( const QUrl &url )
{
    const QString &filename = QFileInfo( url.path() ).fileName();

    try
    {
        Resource &resource = m_Book->GetFolderKeeper().GetResourceByFilename( filename );

        emit OpenResourceRequest( resource, false, url.fragment() );
    }

    catch ( const ResourceDoesNotExist& )
    {
        QMessageBox::warning( 0,
                              tr( "Sigil" ),
                              tr( "The file \"%1\" does not exist." )
                              .arg( filename )
                            );
    }       
}


void BookBrowser::EmitResourceDoubleClicked( const QModelIndex &index )
{
    QString identifier( m_OPFModel.itemFromIndex( index )->data().toString() );  

    if ( !identifier.isEmpty() )

        emit ResourceDoubleClicked( m_Book->GetFolderKeeper().GetResourceByIdentifier( identifier ) );
}


void BookBrowser::OpenContextMenu( const QPoint &point )
{
    if ( !SuccessfullySetupContextMenu( point ) )

        return;

    m_ContextMenu.exec( m_TreeView.viewport()->mapToGlobal( point ) );
    m_ContextMenu.clear();
}


void BookBrowser::AddNew()
{
    if ( m_LastContextMenuType == Resource::HTMLResource )
    {
        m_Book->CreateEmptyHTMLFile();
    }

    else if ( m_LastContextMenuType == Resource::CSSResource )
    {
        m_Book->CreateEmptyCSSFile();
    }

    Refresh();
}


void BookBrowser::AddExisting()
{
    QStringList filenames = QFileDialog::getOpenFileNames(  this, 
                                                            tr( "Add existing file(s)" ),
                                                            m_LastFolderOpen
                                                         );

    if ( filenames.isEmpty() )

        return;

    m_LastFolderOpen = QFileInfo( filenames.first() ).absolutePath();

    foreach( QString filename, filenames )
    {
        if ( TEXT_EXTENSIONS.contains( QFileInfo( filename ).suffix().toLower() ) )
        {
            ImportHTML html_import( filename );
            html_import.SetBook( m_Book );

            // Since we set the Book manually,
            // this call merely mutates our Book.
            html_import.GetBook();
        }

        else
        {
            // TODO: adding a CSS file should add the referenced fonts too
            m_Book->GetFolderKeeper().AddContentFileToFolder( filename );
        }
    }    
    
    Refresh();
}


void BookBrowser::Rename()
{
    // The actual renaming code is in OPFModel::ItemChangedHandler
    m_TreeView.edit( m_TreeView.currentIndex() );
}


void BookBrowser::Remove()
{
    Resource *resource = GetCurrentResource();
    
    if ( !resource )

        return;

    Resource::ResourceType resource_type = resource->Type();

    if ( resource_type == Resource::HTMLResource &&
         m_Book->GetConstFolderKeeper().GetSortedHTMLResources().count() == 1 )
    {
        QMessageBox::warning( 0,
                              tr( "Sigil" ),
                              tr( "The last HTML file cannot be removed.\n" 
                                  "There always has to be at least one." )
                            );

        return;
    }

    QMessageBox::StandardButton button_pressed;
    button_pressed = QMessageBox::warning(	this,
                                            tr( "Sigil" ),
                                            tr( "Are you sure you want to delete the file \"%1\"?\n"
                                                "This action cannot be reversed." )
                                            .arg( resource->Filename() ),
                                            QMessageBox::Ok | QMessageBox::Cancel
                                         );

    if ( button_pressed != QMessageBox::Ok )

        return;

    resource->Delete();

    Refresh();
}


void BookBrowser::ReadSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    m_LastFolderOpen = settings.value( "lastfolderopen" ).toString();
}


void BookBrowser::WriteSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    settings.setValue( "lastfolderopen", m_LastFolderOpen );
}


void BookBrowser::SetupTreeView()
{
    m_TreeView.setEditTriggers( QAbstractItemView::EditKeyPressed );
    m_TreeView.setSortingEnabled( false );
    m_TreeView.sortByColumn( -1 );
    m_TreeView.setUniformRowHeights( true );
    m_TreeView.setDragEnabled( true );
    m_TreeView.setAcceptDrops( false );
    m_TreeView.setDropIndicatorShown( true );
    m_TreeView.setDragDropMode( QAbstractItemView::InternalMove );
    m_TreeView.setContextMenuPolicy( Qt::CustomContextMenu );

    m_TreeView.setModel( &m_OPFModel ); 

    for ( int i = 1; i < m_OPFModel.columnCount(); ++i )
    {
        m_TreeView.hideColumn( i );
    }

    m_TreeView.setIndentation( COLUMN_INDENTATION );
    m_TreeView.setHeaderHidden( true );
}


void BookBrowser::CreateContextMenuActions()
{
    m_AddNew      = new QAction( "Add new item...",       this );
    m_AddExisting = new QAction( "Add existing items...", this );
    m_Rename      = new QAction( "Rename",                this );
    m_Remove      = new QAction( "Remove",                this );

    m_Remove->setShortcut( QKeySequence::Delete );

    addAction( m_Remove );
}


bool BookBrowser::SuccessfullySetupContextMenu( const QPoint &point )
{
    m_ContextMenu.addAction( m_AddExisting );

    QModelIndex index = m_TreeView.indexAt( point );

    if ( !index.isValid() )

        return false;

    QStandardItem *item = m_OPFModel.itemFromIndex( index );
    Q_ASSERT( item );

    m_LastContextMenuType = m_OPFModel.GetResourceType( item );

    if ( m_LastContextMenuType == Resource::HTMLResource ||
         m_LastContextMenuType == Resource::CSSResource )
    {
        m_ContextMenu.addAction( m_AddNew );
    }

    Resource *resource = GetCurrentResource();

    // We just don't add the remove and rename
    // actions, but we do pop up the context menu.
    if ( !resource )

        return true; 

    m_ContextMenu.addSeparator();
    m_ContextMenu.addAction( m_Remove );
    m_ContextMenu.addAction( m_Rename );

    return true;
}


void BookBrowser::ConnectSignalsToSlots()
{
    connect( &m_TreeView, SIGNAL( doubleClicked(             const QModelIndex& ) ), 
             this,         SLOT(  EmitResourceDoubleClicked( const QModelIndex& ) ) );

    connect( &m_TreeView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this,        SLOT(   OpenContextMenu(            const QPoint& ) ) );

    connect( m_AddNew,      SIGNAL( triggered() ), this, SLOT( AddNew()      ) );
    connect( m_AddExisting, SIGNAL( triggered() ), this, SLOT( AddExisting() ) );
    connect( m_Rename,      SIGNAL( triggered() ), this, SLOT( Rename()      ) );
    connect( m_Remove,      SIGNAL( triggered() ), this, SLOT( Remove()      ) );
}


Resource* BookBrowser::GetCurrentResource()
{
    QModelIndex index = m_TreeView.currentIndex();

    if ( !index.isValid() )

        return NULL;

    QStandardItem *item = m_OPFModel.itemFromIndex( index );
    Q_ASSERT( item );

    const QString &identifier = item->data().toString(); 
    return &m_Book->GetFolderKeeper().GetResourceByIdentifier( identifier );
}   


