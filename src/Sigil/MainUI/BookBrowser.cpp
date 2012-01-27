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
#include "Dialogs/RenameTemplate.h"
#include "Misc/FilenameDelegate.h"
#include "Misc/Utility.h"
#include "Misc/SettingsStore.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NCXResource.h"
#include "Importers/ImportHTML.h"
#include "BookManipulation/FolderKeeper.h"
#include "Qxt/qxtconfirmationmessage.h"
#include <QTreeView>

static const QString SETTINGS_GROUP = "bookbrowser";
static const QString OPF_NCX_EDIT_WARNING_KEY = SETTINGS_GROUP + "-opfncx-warning";
static const int COLUMN_INDENTATION = 10;


BookBrowser::BookBrowser( QWidget *parent )
    : 
    QDockWidget( tr( "Book Browser" ), parent ),
    m_Book( NULL ),
    m_MainWidget( *new QWidget( this ) ),
    m_ButtonHolderWidget( *new QWidget( &m_MainWidget ) ),
    m_Layout( *new QVBoxLayout( &m_MainWidget ) ),
    m_TreeView( *new QTreeView( this ) ),
    m_PreviousButton( *new QToolButton( &m_ButtonHolderWidget ) ),
    m_NextButton( *new QToolButton( &m_ButtonHolderWidget ) ),
    m_OPFModel( *new OPFModel( this ) ),
    m_ContextMenu( *new QMenu( this ) ),
    m_SemanticsContextMenu( *new QMenu( this ) ),
    m_FontObfuscationContextMenu( *new QMenu( this ) ),
    m_GuideSemanticMapper( *new QSignalMapper( this ) ),
    m_LastContextMenuType( Resource::GenericResourceType )
{ 
    m_SemanticsContextMenu      .setTitle( tr( "Add Semantics"    ) );
    m_FontObfuscationContextMenu.setTitle( tr( "Font Obfuscation" ) );

    setWidget( &m_TreeView );
    m_Layout.setContentsMargins( 0, 0, 0, 0 );

#ifdef Q_WS_MAC
    m_Layout.setSpacing( 4 );
#endif

    m_PreviousButton.setArrowType( Qt::UpArrow );
    m_PreviousButton.setToolTip( "Open previous file of same type" );
    m_NextButton.setArrowType( Qt::DownArrow );
    m_NextButton.setToolTip( "Open next file of same type" );

    QHBoxLayout *layout = new QHBoxLayout( &m_ButtonHolderWidget );
    layout->addWidget( &m_PreviousButton );
    layout->addWidget( &m_NextButton );
    layout->addStretch( 0 );

    m_ButtonHolderWidget.setLayout( layout );

    m_Layout.addWidget( &m_TreeView );
    m_Layout.addWidget( &m_ButtonHolderWidget );

    m_MainWidget.setLayout( &m_Layout );

    setWidget( &m_MainWidget );

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

    connect( this, SIGNAL( BookContentModified() ), m_Book.data(), SLOT( SetModified() ) );

    ExpandTextFolder();

    try
    {
        // Here we fake that the "first" HTML file has been double clicked
        // so that we have a default first tab opened.
        // An exception is thrown if there are no HTML files in the epub.
        EmitResourceActivated( m_OPFModel.GetFirstHTMLModelIndex() );
    }

    // No exception variable since we don't use it
    catch ( NoHTMLFiles& )
    {
       // Do nothing. No HTML files, no first file opened.
    }
}


void BookBrowser::Refresh()
{
    m_OPFModel.Refresh();

    emit UpdateBrowserSelection();
}


// Update selection to match resource
void BookBrowser::UpdateSelection( Resource &resource )
{
    // Clear selections
    m_TreeView.selectionModel()->setCurrentIndex( m_TreeView.model()->index( 0, 0 ), QItemSelectionModel::Clear );

    QModelIndex index = m_OPFModel.GetModelItemIndex( resource, OPFModel::IndexChoice_Current );
    m_TreeView.selectionModel()->setCurrentIndex( index, QItemSelectionModel::SelectCurrent );
}

void BookBrowser::OpenNextResource()
{
    Resource *resource = GetCurrentResource();
    if ( resource != NULL )
    {
        QModelIndex index = m_OPFModel.GetModelItemIndex( *resource, OPFModel::IndexChoice_Next );
        if ( index.isValid() )
        {
            emit ResourceActivated( *GetResourceByIndex( index ) );
        }
    }
}

void BookBrowser::OpenPreviousResource()
{
    Resource *resource = GetCurrentResource();
    if ( resource != NULL )
    {
        QModelIndex index = m_OPFModel.GetModelItemIndex( *resource, OPFModel::IndexChoice_Previous );
        if ( index.isValid() )
        {
            emit ResourceActivated( *GetResourceByIndex( index ) );
        }
   }
}

void BookBrowser::SortHTML()
{
    QList <Resource *> resources = ValidSelectedResources();

    if ( ValidSelectedItemCount() == 1 )
    {
        // Sort all items
        m_OPFModel.SortHTML(); 
    }
    else
    {
        QList <QModelIndex> indexList = m_TreeView.selectionModel()->selectedRows( 0 );

        m_OPFModel.SortHTML( indexList );
    }
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
        Utility::DisplayStdErrorDialog( 
            tr( "The file \"%1\" does not exist." )
            .arg( filename ) 
            );
    }       
}


void BookBrowser::EmitResourceActivated( const QModelIndex &index )
{
    QString identifier( m_OPFModel.itemFromIndex( index )->data().toString() );  

    if ( !identifier.isEmpty() )
    {
        Resource &resource = m_Book->GetFolderKeeper().GetResourceByIdentifier( identifier );

        if ( ShouldContinueOpeningResource( resource ) )

            emit ResourceActivated( resource );
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

QList <Resource *> BookBrowser::ValidSelectedResources( Resource::ResourceType resource_type )
{
    QList <Resource *> resources = ValidSelectedResources();
    foreach ( Resource *resource, resources )
    {
        if ( resource->Type() != resource_type )
        {
            resources.clear();
        }
    }

    return resources;
}

QList <Resource *> BookBrowser::ValidSelectedResources()
{
    QList <Resource *> resources;

    if ( ValidSelectedItemCount() > 0 )
    {
        // selectedRows appears to sort by index already, despite selectedIndexes not being sorted
        // If sorting is required, try qsort(list) but override < to sort by index number
        QList <QModelIndex> list = m_TreeView.selectionModel()->selectedRows( 0 );

        foreach ( QModelIndex index, list )
        {
            QStandardItem *item = m_OPFModel.itemFromIndex( index );
            if ( item != NULL )
            {
                const QString &identifier = item->data().toString();
    
                Resource *resource = &m_Book->GetFolderKeeper().GetResourceByIdentifier( identifier );
                if ( resource != NULL )
                {
                    resources.append(resource);
                }
            }
        }
    }

    return resources;
}


int BookBrowser::ValidSelectedItemCount()
{
    QList <QModelIndex> list = m_TreeView.selectionModel()->selectedRows( 0 );
    int count = list.length();

    if ( count <= 1 )
    {
        return count;
    }

    Resource::ResourceType resource_type = Resource::HTMLResourceType;

    foreach ( QModelIndex index, list )
    {
        QStandardItem *item = m_OPFModel.itemFromIndex( index );
        const QString &identifier = item->data().toString();

        // If folder item included, multiple selection is invalid
        if ( identifier == NULL )
        {
            return -1;
        }

        Resource *resource = &m_Book->GetFolderKeeper().GetResourceByIdentifier( identifier );

        // If for some reason there is something with an identifier but no resource, fail
        if ( resource == NULL )
        {
            return -1;
        }

        // Check that multiple selection only contains items of the same type
        if ( index == list[0] )
        {
            resource_type = resource->Type();
        }
        else if ( resource_type != resource->Type() )
        {
            return -1;
        }
    }
    return count;
}


void BookBrowser::AddNew()
{
    if ( m_LastContextMenuType == Resource::HTMLResourceType )
    {
        AddNewHTML();
    }
    else if ( m_LastContextMenuType == Resource::CSSResourceType )
    {
        AddNewCSS();
    }
}


// Create a new HTML file and insert it after the currently selected file
void BookBrowser::AddNewHTML()
{
    Resource *current_resource = GetCurrentResource();
    HTMLResource &current_html_resource = *qobject_cast< HTMLResource* >( current_resource );
    HTMLResource &new_html_resource = m_Book->CreateEmptyHTMLFile( current_html_resource );

    if ( current_resource != NULL )
    {
        m_Book->MoveResourceAfter( new_html_resource, current_html_resource );
    }

    // Open the new file in a tab
    emit ResourceActivated( new_html_resource );
    emit BookContentModified();
    Refresh();
}


void BookBrowser::AddNewCSS()
{
    CSSResource &new_resource = m_Book->CreateEmptyCSSFile();
    // Open the new file in a tab
    emit ResourceActivated( new_resource );
    emit BookContentModified();
    Refresh();
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

    QStringList current_filenames = m_Book->GetFolderKeeper().GetAllFilenames();

    HTMLResource *current_html_resource = qobject_cast< HTMLResource* >( GetCurrentResource() );

    foreach( QString filepath, filepaths )
    {
        QString filename = QFileInfo( filepath ).fileName();

        if ( current_filenames.contains( filename ) )
        {
            QMessageBox::warning( this,
                                  tr( "Sigil" ),
                                  tr( "A file with the name \"%1\" already exists in the book." )
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

            Resource &added_resource = m_Book->GetFolderKeeper().GetResourceByFilename( filename );
            HTMLResource *added_html_resource = qobject_cast< HTMLResource * >( &added_resource );

            m_Book->MoveResourceAfter( *added_html_resource, *current_html_resource );

            current_html_resource = added_html_resource;
        }

        else
        {
            // TODO: adding a CSS file should add the referenced fonts too
            m_Book->GetFolderKeeper().AddContentFileToFolder( filepath );
        }
    }

    m_Book->SetMetadata( old_metadata );

    emit ResourceActivated( *current_html_resource );
    emit BookContentModified();
    Refresh();
}


void BookBrowser::Rename()
{
    QList <Resource *> resources = ValidSelectedResources();

    if ( resources.isEmpty() )
    {
        return;
    }

    if ( resources.count() == 1 )
    { 
        // The actual renaming code is in OPFModel::ItemChangedHandler
        m_TreeView.edit( m_TreeView.currentIndex() );
    }
}


void BookBrowser::RenameSelected()
{
    QList <Resource *> resources = ValidSelectedResources();

    if ( resources.isEmpty() )
    {
        return;
    }

    // Load initial value from stored preferences
    SettingsStore *store = SettingsStore::instance();
    QString templateName = store->renameTemplate();

    RenameTemplate rename_template( templateName, this );

    // Get the template from the user
    if ( rename_template.exec() != QDialog::Accepted )
    {
        return;
    }
    templateName = rename_template.GetTemplateName();

    // Save the template for later
    store->setRenameTemplate( templateName );

    // Get the base text and staring number
    int pos = templateName.length() - 1;
    QString templateNumber = "";
    while ( pos > 0 && templateName[pos].isDigit() )
    {
        templateNumber.prepend(QString( "%1" ).arg( templateName[pos] ));
        pos--;
    }
    QString templateBase = templateName.left( pos + 1 );

    if ( templateNumber == "" )
    {
        templateNumber = "1";
    }

    // Rename each entry in turn
    int i = templateNumber.toInt();
    foreach ( Resource *resource, resources )
    {
        QString name = QString( "%1%2" ).arg( templateBase ).arg( i, templateNumber.length(), 10, QChar( '0' ) );
        if ( !m_OPFModel.RenameResource( *resource, name ) )
        {
            break;
        }
        i++;
    }
}


void BookBrowser::Remove()
{
    QList <Resource *> resources = ValidSelectedResources();

    if ( resources.isEmpty() )
    {
        return;
    }
    Resource::ResourceType resource_type = resources[0]->Type();

    if ( resource_type == Resource::OPFResourceType || 
         resource_type == Resource::NCXResourceType )
    {
        Utility::DisplayStdErrorDialog( 
            tr( "Neither the NCX nor the OPF can be removed." )
            );
        return;
    }
    else if ( resource_type == Resource::HTMLResourceType && resources.count() > 1 )
    {
        QMessageBox::StandardButton button_pressed;
        QString msg = resources.count() == 1 ? tr ( "Are you sure you want to delete the selected file?\n" ):
                                       tr ( "Are you sure you want to delete all the selected files?\n" );
        button_pressed = QMessageBox::warning(	this,
                                                tr( "Sigil" ), msg % tr( "This action cannot be reversed." ),
                                                QMessageBox::Ok | QMessageBox::Cancel
                                             );
    
        if ( button_pressed != QMessageBox::Ok )
        { 
            return;
        }
    }

    foreach ( Resource *resource, resources ) 
    {
        resource_type = resource->Type();

        if ( resource_type == Resource::HTMLResourceType && 
            m_Book->GetFolderKeeper().GetResourceTypeList< HTMLResource >().count() == 1 ) 
        {
            Utility::DisplayStdErrorDialog( 
                tr( "The last section cannot be removed.\n"
                    "There always has to be at least one." )
                );
        }
        else
        {
            resource->Delete();
        }
    }

    emit BookContentModified();

    Refresh();
}


void BookBrowser::SetCoverImage()
{
    ImageResource *image_resource = qobject_cast< ImageResource* >( GetCurrentResource() );
    Q_ASSERT( image_resource );

    emit CoverImageSet( *image_resource );
    emit BookContentModified();
}


void BookBrowser::AddGuideSemanticType( int type )
{
    QList <Resource *> resources = ValidSelectedResources();

    if ( resources.isEmpty() )
    {
        return;
    }

    foreach ( Resource *resource, resources )
    {
        HTMLResource *html_resource = qobject_cast< HTMLResource* >( resource );
        emit GuideSemanticTypeAdded( *html_resource, (GuideSemantics::GuideSemanticType) type );
    }
    emit BookContentModified();
}


void BookBrowser::Merge()
{
    QList <Resource *> resources = ValidSelectedResources( Resource::HTMLResourceType );
    Resource *resource1 = resources.first();

    // Skip merge if any non-html files selected - by keyboard shortcut or selection across folders
    if ( resources.isEmpty() )
    {
        return;
    }

    // Convert merge previous to merge selected so all files can be checked for validity
    if ( resources.count() == 1 )
    {
        resource1 = m_Book->PreviousResource( resource1 );
        resources.prepend( resource1 );
    }

    // Save location of first file in folder since resource will be modified
    QModelIndex original_model_index = m_OPFModel.GetModelItemIndex( *resource1, OPFModel::IndexChoice_Current );

    if ( !m_Book->AreResourcesWellFormed( resources ) )
    {
        // Both dialog and well-formed error messages will be shown
        Utility::DisplayStdErrorDialog( tr( "Merge aborted - one of the files was not well-formed." ) );
        return;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );

    resources.removeFirst();

    // Make resource active to make it easier to remove/update later
    emit ResourceActivated( *resource1 );

    HTMLResource &html_resource1 = *qobject_cast< HTMLResource *>( resource1 );
    bool merge_okay = true;
    foreach ( Resource *resource, resources )
    {
        if ( resource != NULL && merge_okay )
        {
            HTMLResource &html_resource2 = *qobject_cast< HTMLResource* >( resource );
            merge_okay = m_Book->Merge( html_resource1, html_resource2 );
        }
    }

    if ( merge_okay )
    {
        // Remove the current tab (original resource) to force tab to reload data when opening the resource
        emit RemoveTabRequest();

        // Use the original index since the old resource seems to become invalid
        EmitResourceActivated( original_model_index );
    }

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
    if ( resource.Type() != Resource::OPFResourceType &&
         resource.Type() != Resource::NCXResourceType )
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

    return message.exec() == QMessageBox::Ok;
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
    m_TreeView.setItemDelegate(new FilenameDelegate);
    m_TreeView.setSelectionMode( QAbstractItemView::ExtendedSelection );

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
    m_AddNewHTML              = new QAction( tr( "Add Blank Section" ),     this );
    m_AddNewCSS               = new QAction( tr( "Add Blank Stylesheet" ),  this );
    m_AddExisting             = new QAction( tr( "Add Existing Files..." ), this );
    m_Rename                  = new QAction( tr( "Rename" ),                this );
    m_RenameSelected          = new QAction( tr( "Rename" ),                this );
    m_Remove                  = new QAction( tr( "Remove" ),                this );
    m_CoverImage              = new QAction( tr( "Cover Image" ),           this );
    m_Merge                   = new QAction( tr( "Merge" ),                 this );
    m_MergeWithPrevious       = new QAction( tr( "Merge With Previous" ),   this );
    m_AdobesObfuscationMethod = new QAction( tr( "Use Adobe's Method" ),    this );
    m_IdpfsObfuscationMethod  = new QAction( tr( "Use IDPF's Method" ),     this );
    m_SortHTML                = new QAction( tr( "Sort All" ),              this );
    m_SortHTMLSelected        = new QAction( tr( "Sort" ),                  this );

    m_CoverImage             ->setCheckable( true );  
    m_AdobesObfuscationMethod->setCheckable( true ); 
    m_IdpfsObfuscationMethod ->setCheckable( true );

    m_Remove->setShortcut( QKeySequence::Delete );

    // Key handles merge with previous and merge selected
    m_Merge->setShortcut( QKeySequence( Qt::CTRL + Qt::ALT + Qt::Key_M ) );
    m_Merge->setToolTip( "Merge files in the book browser" );

    // Has to be added to the book browser itself as well
    // for the keyboard shortcut to work.
    addAction( m_Remove );    
    addAction( m_Merge );    

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

    QModelIndex index = m_TreeView.indexAt( point );

    if ( !index.isValid() )

        return false;

    int item_count = ValidSelectedItemCount();

    if ( item_count < 1 )
    {
        return false;
    }

    m_ContextMenu.addAction( m_AddExisting );

    QStandardItem *item = m_OPFModel.itemFromIndex( index );
    Q_ASSERT( item );

    m_LastContextMenuType = m_OPFModel.GetResourceType( item );

    if ( m_LastContextMenuType == Resource::HTMLResourceType )
    {
        m_ContextMenu.addAction( m_AddNewHTML );
    }
    else if ( m_LastContextMenuType == Resource::CSSResourceType )
    {
        m_ContextMenu.addAction( m_AddNewCSS );
    }

    Resource *resource = GetCurrentResource();

    // We just don't add the remove and rename
    // actions, but we do pop up the context menu.
    if ( !resource )

        return true; 

    m_ContextMenu.addSeparator();

    if ( m_LastContextMenuType != Resource::OPFResourceType &&
         m_LastContextMenuType != Resource::NCXResourceType )
    {
        m_ContextMenu.addAction( m_Remove );

        if ( item_count == 1 )
        {
            m_ContextMenu.addAction( m_Rename );
        }
        else 
        {
            m_ContextMenu.addAction( m_RenameSelected );
        }

        m_ContextMenu.addSeparator();
    }

    SetupResourceSpecificContextMenu( resource );    

    return true;
}


void BookBrowser::SetupResourceSpecificContextMenu( Resource *resource  )
{
    if ( resource->Type() == Resource::HTMLResourceType )
    {
        AddMergeAction( resource ); 

        if ( ValidSelectedItemCount() == 1 )
        {
            m_ContextMenu.addAction( m_SortHTML );
        }
        else
        {
            m_ContextMenu.addAction( m_SortHTMLSelected );
        }
        m_ContextMenu.addSeparator();

    }

    if ( resource->Type() == Resource::FontResourceType && ValidSelectedItemCount() == 1 )

        SetupFontObfuscationMenu( resource );

    SetupSemanticContextmenu( resource );
}


void BookBrowser::SetupSemanticContextmenu( Resource *resource )
{
    if ( resource->Type() != Resource::HTMLResourceType && 
         resource->Type() != Resource::ImageResourceType )
    {
        return;
    }
    
    if ( resource->Type() == Resource::HTMLResourceType )
    {
        SetupHTMLSemanticContextMenu( resource );
        m_ContextMenu.addMenu( &m_SemanticsContextMenu );
    }
    else // Resource::ImageResource
    {
        SetupImageSemanticContextMenu( resource );

        if ( ValidSelectedItemCount() == 1 )
        {
            m_ContextMenu.addMenu( &m_SemanticsContextMenu );
        }
    }
}


void BookBrowser::SetupHTMLSemanticContextMenu( Resource *resource )
{
    int item_count = ValidSelectedItemCount();
    
    foreach( QAction* action, m_GuideSemanticActions )
    {
        // Only 2 of the Semantic types can apply to multiple files in the Book
        if ( item_count == 1  || action->data() == GuideSemantics::Text  || action->data() == GuideSemantics::NoType )
        {
            m_SemanticsContextMenu.addAction( action );
        }
    }

    // Checkstate shown in menu won't be valid if items are of mixed states, but harmless
    SetHTMLSemanticActionCheckState( resource );    
}


void BookBrowser::SetupImageSemanticContextMenu( Resource *resource )
{
    m_SemanticsContextMenu.addAction( m_CoverImage );

    ImageResource *image_resource = qobject_cast< ImageResource* >( GetCurrentResource() );
    Q_ASSERT( image_resource );

    m_CoverImage->setChecked( false );

    if ( m_Book->GetOPF().IsCoverImage( *image_resource ) )

        m_CoverImage->setChecked( true );
}


void BookBrowser::SetHTMLSemanticActionCheckState( Resource *resource )
{
    if ( resource->Type() != Resource::HTMLResourceType )

        return;

    HTMLResource *html_resource = qobject_cast< HTMLResource* >( resource );
    Q_ASSERT( html_resource );

    foreach( QAction* action, m_GuideSemanticActions )
    {
        action->setChecked( false );
    }

    GuideSemantics::GuideSemanticType semantic_type = 
        m_Book->GetOPF().GetGuideSemanticTypeForResource( *html_resource );

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
    if ( resource->Type() != Resource::FontResourceType )

        return;

    FontResource *font_resource = qobject_cast< FontResource* >( resource );
    Q_ASSERT( font_resource );

    QString algorithm = font_resource->GetObfuscationAlgorithm();

    m_AdobesObfuscationMethod->setChecked( algorithm == ADOBE_FONT_ALGO_ID );
    m_IdpfsObfuscationMethod ->setChecked( algorithm == IDPF_FONT_ALGO_ID  );
}


void BookBrowser::AddMergeAction( Resource *resource )
{
    HTMLResource *html_resource = qobject_cast< HTMLResource* >( resource );
    Q_ASSERT( html_resource );

    if ( ValidSelectedItemCount() == 1 )
    {
        // We can't add the action for the first file
        if ( m_Book->GetOPF().GetReadingOrder( *html_resource ) > 0 )
        {
            m_ContextMenu.addAction( m_MergeWithPrevious );    
        }
    }
    else
    {
        m_ContextMenu.addAction( m_Merge );
    }
}


void BookBrowser::ConnectSignalsToSlots()
{
    connect( &m_TreeView, SIGNAL( activated(             const QModelIndex& ) ),
             this,         SLOT(  EmitResourceActivated( const QModelIndex& ) ) );

    connect( &m_TreeView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this,        SLOT(   OpenContextMenu(            const QPoint& ) ) );

    connect( m_AddNewHTML,              SIGNAL( triggered() ), this, SLOT( AddNewHTML()              ) );
    connect( m_SortHTML,                SIGNAL( triggered() ), this, SLOT( SortHTML()                ) );
    connect( m_SortHTMLSelected,        SIGNAL( triggered() ), this, SLOT( SortHTML()                ) );
    connect( m_AddNewCSS,               SIGNAL( triggered() ), this, SLOT( AddNewCSS()               ) );
    connect( m_AddExisting,             SIGNAL( triggered() ), this, SLOT( AddExisting()             ) );
    connect( m_Rename,                  SIGNAL( triggered() ), this, SLOT( Rename()                  ) );
    connect( m_RenameSelected,          SIGNAL( triggered() ), this, SLOT( RenameSelected()          ) );
    connect( m_Remove,                  SIGNAL( triggered() ), this, SLOT( Remove()                  ) );
    connect( m_CoverImage,              SIGNAL( triggered() ), this, SLOT( SetCoverImage()           ) );
    connect( m_Merge,                   SIGNAL( triggered() ), this, SLOT( Merge()                   ) );
    connect( m_MergeWithPrevious,       SIGNAL( triggered() ), this, SLOT( Merge()                   ) );

    connect( m_AdobesObfuscationMethod, SIGNAL( triggered() ), this, SLOT( AdobesObfuscationMethod() ) );
    connect( m_IdpfsObfuscationMethod,  SIGNAL( triggered() ), this, SLOT( IdpfsObfuscationMethod()  ) );

    connect( &m_PreviousButton,          SIGNAL( clicked() ), this, SLOT( OpenPreviousResource()     ) );
    connect( &m_NextButton,              SIGNAL( clicked() ), this, SLOT( OpenNextResource()         ) );

    foreach( QAction* action, m_GuideSemanticActions )
    {
        connect( action, SIGNAL( triggered() ), &m_GuideSemanticMapper, SLOT( map() ) );
    }

    connect( &m_GuideSemanticMapper, SIGNAL( mapped( int ) ), this, SLOT( AddGuideSemanticType( int ) ) );
}


Resource* BookBrowser::GetCurrentResource()
{
    return GetResourceByIndex( m_TreeView.currentIndex() );
}   


Resource* BookBrowser::GetResourceByIndex( QModelIndex index )
{
    if ( !index.isValid() )

        return NULL;

    QStandardItem *item = m_OPFModel.itemFromIndex( index );
    Q_ASSERT( item );

    const QString &identifier = item->data().toString(); 
    return &m_Book->GetFolderKeeper().GetResourceByIdentifier( identifier );
}   
