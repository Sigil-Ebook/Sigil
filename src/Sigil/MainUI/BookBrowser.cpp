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
#include "../BookManipulation/GuideSemantics.h"
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
    m_SemanticsContextMenu( *new QMenu( this ) ),
    m_SemanticMapper( *new QSignalMapper( this ) ),
    m_LastContextMenuType( Resource::GenericResource )
{ 
    m_SemanticsContextMenu.setTitle( tr( "Add Semantics" ) );

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

    // We need to store the current metadata since the 
    // GetBook call will clear it.
    QHash< QString, QList< QVariant > > old_metadata = m_Book->GetMetadata();

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

    m_Book->SetMetadata( old_metadata );
    
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


void BookBrowser::AddGuideSemanticType( int type )
{
    GuideSemantics::GuideSemanticType semantic_type = (GuideSemantics::GuideSemanticType) type;

    HTMLResource *html_resource = qobject_cast< HTMLResource* >( GetCurrentResource() );
    Q_ASSERT( html_resource );

    // There can be only one!
    foreach( HTMLResource *iter_resource, m_Book->GetFolderKeeper().GetSortedHTMLResources() )
    {
        if ( iter_resource->GetGuideSemanticType() == semantic_type )
        {
            iter_resource->SetGuideSemanticType( GuideSemantics::NoType );
            break;
        }
    }

    html_resource->SetGuideSemanticType( semantic_type );
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
    m_AddNew      = new QAction( tr( "Add New Item..." ),       this );
    m_AddExisting = new QAction( tr( "Add Existing Items..." ), this );
    m_Rename      = new QAction( tr( "Rename" ),                this );
    m_Remove      = new QAction( tr( "Remove" ),                this );

    m_Remove->setShortcut( QKeySequence::Delete );

    addAction( m_Remove );

    CreateGuideSemanticActions();
}


void BookBrowser::CreateGuideSemanticActions()
{
    QAction *action = NULL;

    action = new QAction( tr( "Cover" ), this );
    action->setData( GuideSemantics::Cover );
    m_SemanticMapper.setMapping( action, GuideSemantics::Cover );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Title Page" ), this );
    action->setData( GuideSemantics::TitlePage );
    m_SemanticMapper.setMapping( action, GuideSemantics::TitlePage );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Table Of Contents" ), this );
    action->setData( GuideSemantics::TableOfContents );
    m_SemanticMapper.setMapping( action, GuideSemantics::TableOfContents );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Index" ), this );
    action->setData( GuideSemantics::Index );
    m_SemanticMapper.setMapping( action, GuideSemantics::Index );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Glossary" ), this );
    action->setData( GuideSemantics::Glossary );
    m_SemanticMapper.setMapping( action, GuideSemantics::Glossary );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Acknowledgments" ), this );
    action->setData( GuideSemantics::Acknowledgments );
    m_SemanticMapper.setMapping( action, GuideSemantics::Acknowledgments );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Bibliography" ), this );
    action->setData( GuideSemantics::Bibliography );
    m_SemanticMapper.setMapping( action, GuideSemantics::Bibliography );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Colophon" ), this );
    action->setData( GuideSemantics::Colophon );
    m_SemanticMapper.setMapping( action, GuideSemantics::Colophon );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "CopyrightPage" ), this );
    action->setData( GuideSemantics::CopyrightPage );
    m_SemanticMapper.setMapping( action, GuideSemantics::CopyrightPage );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Dedication" ), this );
    action->setData( GuideSemantics::Dedication );
    m_SemanticMapper.setMapping( action, GuideSemantics::Dedication );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Epigraph" ), this );
    action->setData( GuideSemantics::Epigraph );
    m_SemanticMapper.setMapping( action, GuideSemantics::Epigraph );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Foreword" ), this );
    action->setData( GuideSemantics::Foreword );
    m_SemanticMapper.setMapping( action, GuideSemantics::Foreword );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "List Of Illustrations" ), this );
    action->setData( GuideSemantics::ListOfIllustrations );
    m_SemanticMapper.setMapping( action, GuideSemantics::ListOfIllustrations );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "List Of Tables" ), this );
    action->setData( GuideSemantics::ListOfTables );
    m_SemanticMapper.setMapping( action, GuideSemantics::ListOfTables );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Notes" ), this );
    action->setData( GuideSemantics::Notes );
    m_SemanticMapper.setMapping( action, GuideSemantics::Notes );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Preface" ), this );
    action->setData( GuideSemantics::Preface );
    m_SemanticMapper.setMapping( action, GuideSemantics::Preface );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Text" ), this );
    action->setData( GuideSemantics::Text );
    m_SemanticMapper.setMapping( action, GuideSemantics::Text );
    m_GuideSemanticActions.append( action );

    foreach( QAction* action, m_GuideSemanticActions )
    {
        action->setCheckable( true );
    }
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

    SetupSemanticContextmenu( resource );

    return true;
}


void BookBrowser::SetupSemanticContextmenu( Resource *resource )
{
    if ( resource->Type() != Resource::HTMLResource )

        return;

    foreach( QAction* action, m_GuideSemanticActions )
    {
        m_SemanticsContextMenu.addAction( action );
    }

    m_ContextMenu.addMenu( &m_SemanticsContextMenu );
    
    SetSemanticActionCheckState( resource );
}


void BookBrowser::SetSemanticActionCheckState( Resource *resource )
{
    if ( resource->Type() != Resource::HTMLResource )

        return;

    HTMLResource *html_resource = qobject_cast< HTMLResource* >( resource );
    Q_ASSERT( html_resource );

    foreach( QAction* action, m_GuideSemanticActions )
    {
        action->setChecked( false );
    }

    GuideSemantics::GuideSemanticType semantic_type = html_resource->GetGuideSemanticType();

    if ( semantic_type == GuideSemantics::NoType )

        return;

    foreach( QAction* action, m_GuideSemanticActions )
    {
        if ( action->data().toInt() == semantic_type )
        {
            action->setChecked( true );
            break;
        }
    }  
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

    foreach( QAction* action, m_GuideSemanticActions )
    {
        connect( action, SIGNAL( triggered() ), &m_SemanticMapper, SLOT( map() ) );
    }

    connect( &m_SemanticMapper, SIGNAL( mapped( int ) ), this, SLOT( AddGuideSemanticType( int ) ) );
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






