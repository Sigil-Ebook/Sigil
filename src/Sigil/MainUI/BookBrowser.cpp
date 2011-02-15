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

#include <stdafx.h>
#include "BookBrowser.h"
#include "OPFModel.h"
#include "BookManipulation/Book.h"
#include "BookManipulation/GuideSemantics.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "Importers/ImportHTML.h"
#include "Qxt/qxtconfirmationmessage.h"
#include <QTreeView>

static const QString SETTINGS_GROUP = "bookbrowser";
static const QString OPF_NCX_EDIT_WARNING_KEY = SETTINGS_GROUP + "-opfncx-warning";
static const int COLUMN_INDENTATION = 10;


BookBrowser::BookBrowser( QWidget *parent )
    : 
    QDockWidget( tr( "Book Browser" ), parent ),
    m_Book( NULL ),
    m_TreeView( *new QTreeView( this ) ),
    m_OPFModel( *new OPFModel( this ) ),
    m_ContextMenu( *new QMenu( this ) ),
    m_SemanticsContextMenu( *new QMenu( this ) ),
    m_FontObfuscationContextMenu( *new QMenu( this ) ),
    m_GuideSemanticMapper( *new QSignalMapper( this ) ),
    m_LastContextMenuType( Resource::GenericResource )
{ 
    m_SemanticsContextMenu      .setTitle( tr( "Add Semantics"    ) );
    m_FontObfuscationContextMenu.setTitle( tr( "Font Obfuscation" ) );

    setWidget( &m_TreeView );
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
    ExpandTextFolder();

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
        QMessageBox::critical( 0,
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
    {
        Resource &resource = m_Book->GetFolderKeeper().GetResourceByIdentifier( identifier );

        if ( ShouldContinueOpeningResource( resource ) )

            emit ResourceDoubleClicked( resource );
    }
}


void BookBrowser::OpenContextMenu( const QPoint &point )
{
    if ( !SuccessfullySetupContextMenu( point ) )

        return;

    m_ContextMenu.exec( m_TreeView.viewport()->mapToGlobal( point ) );
    m_ContextMenu.clear();
    m_SemanticsContextMenu.clear();
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

    // TODO: this should be automatic through signals/slots
    m_Book->SetModified( true );
}


void BookBrowser::AddExisting()
{
    QStringList filepaths = QFileDialog::getOpenFileNames(  this, 
                                                            tr( "Add existing file(s)" ),
                                                            m_LastFolderOpen
                                                         );

    if ( filepaths.isEmpty() )

        return;

    m_LastFolderOpen = QFileInfo( filepaths.first() ).absolutePath();

    // We need to store the current metadata since the 
    // GetBook call will clear it.
    QHash< QString, QList< QVariant > > old_metadata = m_Book->GetMetadata();

    QStringList current_filenames = m_Book->GetConstFolderKeeper().GetAllFilenames();

    foreach( QString filepath, filepaths )
    {
        QString filename = QFileInfo( filepath ).fileName();

        if ( current_filenames.contains( filename ) )
        {
            QMessageBox::warning( this,
                                  tr( "Sigil" ),
                                  tr( "A file with the name \"%1\" already exists in the epub." )
                                  .arg( filename )
                                );
            continue;
        }

        if ( TEXT_EXTENSIONS.contains( QFileInfo( filepath ).suffix().toLower() ) )
        {
            ImportHTML html_import( filepath );
            html_import.SetBook( m_Book, true );

            // Since we set the Book manually,
            // this call merely mutates our Book.
            html_import.GetBook();
        }

        else
        {
            // TODO: adding a CSS file should add the referenced fonts too
            m_Book->GetFolderKeeper().AddContentFileToFolder( filepath );
        }
    }    

    m_Book->SetMetadata( old_metadata );

    // TODO: this should be automatic through signals/slots
    m_Book->SetModified( true );
    
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
         m_Book->GetConstFolderKeeper().GetResourceTypeList< HTMLResource >().count() == 1 )
    {
        QMessageBox::critical( 0,
                               tr( "Sigil" ),
                               tr( "The last HTML file cannot be removed.\n" 
                                   "There always has to be at least one." )
                             );

        return;
    }

    if ( resource_type == Resource::OPFResource || 
         resource_type == Resource::NCXResource )
    {
        QMessageBox::critical( 0,
                               tr( "Sigil" ),
                               tr( "Neither the NCX nor the OPF can be removed." )
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

    m_Book->NormalizeReadingOrders();

    // TODO: this should be automatic through signals/slots
    m_Book->SetModified( true );

    Refresh();
}


void BookBrowser::SetCoverImage()
{
    ImageResource *changing_image = qobject_cast< ImageResource* >( GetCurrentResource() );
    Q_ASSERT( changing_image );

    // Turn on.
    if ( !changing_image->IsCoverImage() )
    {
        foreach( ImageResource *image_resource, m_Book->GetFolderKeeper().GetResourceTypeList< ImageResource >() )
        {
            image_resource->SetIsCoverImage( false );
        }

        changing_image->SetIsCoverImage( true );            
    }

    // Turn off.
    else
    {
        changing_image->SetIsCoverImage( false );
    }

    // TODO: this should be automatic through signals/slots
    m_Book->SetModified( true );
}


void BookBrowser::AddGuideSemanticType( int type )
{
    GuideSemantics::GuideSemanticType semantic_type_to_add = (GuideSemantics::GuideSemanticType) type;

    HTMLResource *changing_html = qobject_cast< HTMLResource* >( GetCurrentResource() );
    Q_ASSERT( changing_html );

    // Turn on.
    if ( changing_html->GetGuideSemanticType() != semantic_type_to_add )
    {
        // Industry best practice is to have only one 
        // <guide> reference type instance per book.
        // The only exception is the Text type, of which  
        // we customarily have more than one instance.
        if ( semantic_type_to_add != GuideSemantics::Text )
        {
            foreach( HTMLResource *html_resource, m_Book->GetFolderKeeper().GetResourceTypeList< HTMLResource >() )
            {
                if ( html_resource->GetGuideSemanticType() == semantic_type_to_add )
                {
                    html_resource->SetGuideSemanticType( GuideSemantics::NoType );
    
                    // There is no "break" statement here because we might
                    // load an epub that has several instance of one guide type.
                    // We preserve them on load, but if the user is intent on
                    // changing them, then we enforce "one type instance per book".
                }
            }
        }

        changing_html->SetGuideSemanticType( semantic_type_to_add );
    }

    // Turn off.
    else
    {
        changing_html->SetGuideSemanticType( GuideSemantics::NoType );
    }

    // TODO: this should be automatic through signals/slots
    m_Book->SetModified( true );
}


void BookBrowser::MergeWithPrevious()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );

    HTMLResource *html_resource = qobject_cast< HTMLResource* >( GetCurrentResource() );
    Q_ASSERT( html_resource );

    m_Book->MergeWithPrevious( *html_resource );
    Refresh();

    QApplication::restoreOverrideCursor();
}


void BookBrowser::AdobesObfuscationMethod()
{
    FontResource *font_resource = qobject_cast< FontResource* >( GetCurrentResource() );
    Q_ASSERT( font_resource );

    QString current_algorithm = font_resource->GetObfuscationAlgorithm();

    if ( current_algorithm == IDPF_FONT_ALGO_ID || current_algorithm.isEmpty() )

        font_resource->SetObfuscationAlgorithm( ADOBE_FONT_ALGO_ID );

    else

        font_resource->SetObfuscationAlgorithm( "" );
}


void BookBrowser::IdpfsObfuscationMethod()
{
    FontResource *font_resource = qobject_cast< FontResource* >( GetCurrentResource() );
    Q_ASSERT( font_resource );

    QString current_algorithm = font_resource->GetObfuscationAlgorithm();

    if ( current_algorithm == ADOBE_FONT_ALGO_ID || current_algorithm.isEmpty() )

        font_resource->SetObfuscationAlgorithm( IDPF_FONT_ALGO_ID );

    else

        font_resource->SetObfuscationAlgorithm( "" );
}


bool BookBrowser::ShouldContinueOpeningResource( const Resource &resource )
{
    if ( resource.Type() != Resource::OPFResource &&
         resource.Type() != Resource::NCXResource )
    {
        return true;
    }

    QxtConfirmationMessage message( 
        QMessageBox::Information,
        tr( "Sigil" ),
        tr( "Editing the OPF and NCX files is for experts only!\n\nContinue?" ),
        tr( "Don't show again." ),
        QMessageBox::Ok | QMessageBox::Cancel, 
        this
        );

    message.setOverrideSettingsKey( OPF_NCX_EDIT_WARNING_KEY );
    message.setDefaultButton( QMessageBox::Cancel );

    return message.exec() == QMessageBox::Yes;
}


void BookBrowser::ExpandTextFolder()
{
    m_TreeView.expand( m_OPFModel.GetTextFolderModelIndex() );
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
    m_AddNew                  = new QAction( tr( "Add New Item" ),          this );
    m_AddExisting             = new QAction( tr( "Add Existing Items..." ), this );
    m_Rename                  = new QAction( tr( "Rename" ),                this );
    m_Remove                  = new QAction( tr( "Remove" ),                this );
    m_CoverImage              = new QAction( tr( "Cover Image" ),           this );
    m_MergeWithPrevious       = new QAction( tr( "Merge With Previous" ),   this );
    m_AdobesObfuscationMethod = new QAction( tr( "Use Adobe's Method" ),    this );
    m_IdpfsObfuscationMethod  = new QAction( tr( "Use IDPF's Method" ),     this );

    m_CoverImage             ->setCheckable( true );  
    m_AdobesObfuscationMethod->setCheckable( true ); 
    m_IdpfsObfuscationMethod ->setCheckable( true );

    m_Remove->setShortcut( QKeySequence::Delete );

    // Has to be added to the book browser itself as well
    // for the keyboard shortcut to work.
    addAction( m_Remove );    

    CreateGuideSemanticActions();
}


void BookBrowser::CreateGuideSemanticActions()
{
    QAction *action = NULL;

    action = new QAction( tr( "Cover" ), this );
    action->setData( GuideSemantics::Cover );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Cover );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Title Page" ), this );
    action->setData( GuideSemantics::TitlePage );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::TitlePage );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Table Of Contents" ), this );
    action->setData( GuideSemantics::TableOfContents );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::TableOfContents );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Index" ), this );
    action->setData( GuideSemantics::Index );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Index );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Glossary" ), this );
    action->setData( GuideSemantics::Glossary );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Glossary );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Acknowledgements" ), this );
    action->setData( GuideSemantics::Acknowledgements );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Acknowledgements );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Bibliography" ), this );
    action->setData( GuideSemantics::Bibliography );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Bibliography );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Colophon" ), this );
    action->setData( GuideSemantics::Colophon );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Colophon );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "CopyrightPage" ), this );
    action->setData( GuideSemantics::CopyrightPage );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::CopyrightPage );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Dedication" ), this );
    action->setData( GuideSemantics::Dedication );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Dedication );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Epigraph" ), this );
    action->setData( GuideSemantics::Epigraph );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Epigraph );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Foreword" ), this );
    action->setData( GuideSemantics::Foreword );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Foreword );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "List Of Illustrations" ), this );
    action->setData( GuideSemantics::ListOfIllustrations );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::ListOfIllustrations );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "List Of Tables" ), this );
    action->setData( GuideSemantics::ListOfTables );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::ListOfTables );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Notes" ), this );
    action->setData( GuideSemantics::Notes );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Notes );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Preface" ), this );
    action->setData( GuideSemantics::Preface );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Preface );
    m_GuideSemanticActions.append( action );

    action = new QAction( tr( "Text" ), this );
    action->setData( GuideSemantics::Text );
    m_GuideSemanticMapper.setMapping( action, GuideSemantics::Text );
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

    if ( m_LastContextMenuType != Resource::OPFResource &&
         m_LastContextMenuType != Resource::NCXResource )
    {
        m_ContextMenu.addAction( m_Remove );
        m_ContextMenu.addAction( m_Rename );
        m_ContextMenu.addSeparator();
    }

    SetupResourceSpecificContextMenu( resource );    

    return true;
}


void BookBrowser::SetupResourceSpecificContextMenu( Resource *resource  )
{
    if ( resource->Type() == Resource::HTMLResource )
    
        AddMergeWithPreviousAction( resource ); 

    if ( resource->Type() == Resource::FontResource )

        SetupFontObfuscationMenu( resource );

    SetupSemanticContextmenu( resource );
}


void BookBrowser::SetupSemanticContextmenu( Resource *resource )
{
    if ( resource->Type() != Resource::HTMLResource && 
         resource->Type() != Resource::ImageResource )
    {
        return;
    }
    
    if ( resource->Type() == Resource::HTMLResource )

        SetupHTMLSemanticContextMenu( resource );

    else // Resource::ImageResource

        SetupImageSemanticContextMenu( resource );

    m_ContextMenu.addMenu( &m_SemanticsContextMenu );
}


void BookBrowser::SetupHTMLSemanticContextMenu( Resource *resource )
{
    foreach( QAction* action, m_GuideSemanticActions )
    {
        m_SemanticsContextMenu.addAction( action );
    }

    SetHTMLSemanticActionCheckState( resource );    
}


void BookBrowser::SetupImageSemanticContextMenu( Resource *resource )
{
    m_SemanticsContextMenu.addAction( m_CoverImage );

    ImageResource *image_resource = qobject_cast< ImageResource* >( GetCurrentResource() );
    Q_ASSERT( image_resource );

    m_CoverImage->setChecked( false );

    if ( image_resource->IsCoverImage() )

        m_CoverImage->setChecked( true );
}


void BookBrowser::SetHTMLSemanticActionCheckState( Resource *resource )
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


void BookBrowser::SetupFontObfuscationMenu( Resource *resource )
{
    m_FontObfuscationContextMenu.addAction( m_AdobesObfuscationMethod );    
    m_FontObfuscationContextMenu.addAction( m_IdpfsObfuscationMethod );    

    SetFontObfuscationActionCheckState( resource );

    m_ContextMenu.addMenu( &m_FontObfuscationContextMenu );
}


void BookBrowser::SetFontObfuscationActionCheckState( Resource *resource )
{
    if ( resource->Type() != Resource::FontResource )

        return;

    FontResource *font_resource = qobject_cast< FontResource* >( resource );
    Q_ASSERT( font_resource );

    QString algorithm = font_resource->GetObfuscationAlgorithm();

    m_AdobesObfuscationMethod->setChecked( algorithm == ADOBE_FONT_ALGO_ID );
    m_IdpfsObfuscationMethod ->setChecked( algorithm == IDPF_FONT_ALGO_ID  );
}


void BookBrowser::AddMergeWithPreviousAction( Resource *resource )
{
    HTMLResource *html_resource = qobject_cast< HTMLResource* >( resource );
    Q_ASSERT( html_resource );

    // We can't add the action for the first file;
    // what would be the "previous" file? Looping back
    // doesn't make any sense.
    if ( html_resource->GetReadingOrder() > 0 )

        m_ContextMenu.addAction( m_MergeWithPrevious );    
}


void BookBrowser::ConnectSignalsToSlots()
{
    connect( &m_TreeView, SIGNAL( doubleClicked(             const QModelIndex& ) ), 
             this,         SLOT(  EmitResourceDoubleClicked( const QModelIndex& ) ) );

    connect( &m_TreeView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this,        SLOT(   OpenContextMenu(            const QPoint& ) ) );

    connect( m_AddNew,                  SIGNAL( triggered() ), this, SLOT( AddNew()                  ) );
    connect( m_AddExisting,             SIGNAL( triggered() ), this, SLOT( AddExisting()             ) );
    connect( m_Rename,                  SIGNAL( triggered() ), this, SLOT( Rename()                  ) );
    connect( m_Remove,                  SIGNAL( triggered() ), this, SLOT( Remove()                  ) );
    connect( m_CoverImage,              SIGNAL( triggered() ), this, SLOT( SetCoverImage()           ) );
    connect( m_MergeWithPrevious,       SIGNAL( triggered() ), this, SLOT( MergeWithPrevious()       ) );

    connect( m_AdobesObfuscationMethod, SIGNAL( triggered() ), this, SLOT( AdobesObfuscationMethod() ) );
    connect( m_IdpfsObfuscationMethod,  SIGNAL( triggered() ), this, SLOT( IdpfsObfuscationMethod()  ) );

    foreach( QAction* action, m_GuideSemanticActions )
    {
        connect( action, SIGNAL( triggered() ), &m_GuideSemanticMapper, SLOT( map() ) );
    }

    connect( &m_GuideSemanticMapper, SIGNAL( mapped( int ) ), this, SLOT( AddGuideSemanticType( int ) ) );
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










