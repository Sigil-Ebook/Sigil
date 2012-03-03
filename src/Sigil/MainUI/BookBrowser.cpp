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
#include <QtCore/QSignalMapper>
#include <QtGui/QFileDialog>
#include <QtGui/QMenu>
#include <QtGui/QTreeView>

#include "BookManipulation/Book.h"
#include "BookManipulation/FolderKeeper.h"
#include "Dialogs/RenameTemplate.h"
#include "Importers/ImportHTML.h"
#include "MainUI/BookBrowser.h"
#include "MainUI/OPFModel.h"
#include "Misc/FilenameDelegate.h"
#include "Misc/KeyboardShortcutManager.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Qxt/qxtconfirmationmessage.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "sigil_constants.h"
#include "sigil_exception.h"

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
    m_LastContextMenuType( Resource::GenericResourceType )
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
    m_TreeView.selectionModel()->clearSelection();

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

    QList <Resource *> all_files = m_OPFModel.GetResourceListInFolder( m_LastContextMenuType );
    QString msg = ( all_files.count() == resources.count() || resources.count() == 1 ) ?  tr( "Are you sure you want to sort ALL files alphanumerically?  You can also select just some of the files to sort.\n" ):
                                           tr( "Are you sure you want to sort the selected files alphanumerically?\n" );

    QMessageBox::StandardButton button_pressed;
    button_pressed = QMessageBox::warning(	this,
                  tr( "Sigil" ), msg % tr( "This action cannot be reversed." ),
                                            QMessageBox::Ok | QMessageBox::Cancel
                                         );
    if ( button_pressed != QMessageBox::Ok )
    { 
        return;
    }

    if ( resources.count() == 1 )
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

void BookBrowser::RefreshTOC()
{
    emit RefreshTOCContentsRequest();
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

QList <Resource *> BookBrowser::ValidSelectedHTMLResources()
{
    return ValidSelectedResources( Resource::HTMLResourceType );
}

QList <Resource *> BookBrowser::AllHTMLResources()
{
    return m_OPFModel.GetResourceListInFolder( Resource::HTMLResourceType );
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
    Resource::ResourceType resource_type = Resource::HTMLResourceType;

    if ( ValidSelectedItemCount() < 1 )
    {
        return resources;
    }

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
                resources.append( resource );
                resource_type = resource->Type();
            }
        }
    }

    // Sort according to treeview order
    QList <Resource *> sorted_resources;
    QList <Resource *> all_resources = m_OPFModel.GetResourceListInFolder( resource_type );

    foreach ( Resource *all_resource, all_resources )
    {
        foreach ( Resource *resource, resources )
        {
            if ( all_resource->Filename() == resource->Filename() )
            {
                sorted_resources.append( all_resource );
                break;
            }
        }
    }

    return sorted_resources;
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

void BookBrowser::SaveSelection()
{
    m_SavedSelection = m_TreeView.selectionModel()->selectedRows( 0 );
}

void BookBrowser::RestoreSelection()
{
    m_TreeView.selectionModel()->clearSelection();

    // Set the saved selectins
    foreach ( QModelIndex index, m_SavedSelection )
    {
        m_TreeView.selectionModel()->select( index, QItemSelectionModel::Select );
    }
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

            if ( current_html_resource && added_html_resource )
            {
                m_Book->MoveResourceAfter( *added_html_resource, *current_html_resource );
            }

            emit ResourceActivated( added_resource );
        }

        else
        {
            // TODO: adding a CSS file should add the referenced fonts too
            m_Book->GetFolderKeeper().AddContentFileToFolder( filepath );
        }
    }

    m_Book->SetMetadata( old_metadata );

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
    else
    {
        RenameSelected();
    }
}


void BookBrowser::RenameAll()
{
    RenameList( m_OPFModel.GetResourceListInFolder( Resource::HTMLResourceType ) );
}


void BookBrowser::RenameSelected()
{
    RenameList( ValidSelectedResources() );
}


void BookBrowser::RenameList( QList <Resource *> resources )
{
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
    store->setRenameTemplate(templateName);

    // Get the base text and starting number
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

    QString first_filename = resources.at(0)->Filename();
    QString extension = "";
    if ( first_filename.contains( '.' ) )
    {
        extension = first_filename.right( first_filename.length() - first_filename.lastIndexOf( '.' ) );
    }

    // Rename each entry in turn
    int i = templateNumber.toInt();
    foreach ( Resource *resource, resources )
    {
        QString name = QString( "%1%2" ).arg( templateBase ).arg( i, templateNumber.length(), 10, QChar( '0' ) ).append( extension );
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

    Resource::ResourceType resource_type = resources.first()->Type();

    if ( resource_type == Resource::OPFResourceType || 
         resource_type == Resource::NCXResourceType )
    {
        Utility::DisplayStdErrorDialog( 
            tr( "Neither the NCX nor the OPF can be removed." )
            );
        return;
    }
    else if ( resource_type == Resource::HTMLResourceType &&
        resources.count() == m_Book->GetFolderKeeper().GetResourceTypeList< HTMLResource >().count() ) 
    {
            {
                Utility::DisplayStdErrorDialog( 
                    tr( "You cannot remove all html files.\n"
                        "There always has to be at least one." )
                    );
                return;
            }
    }

    QMessageBox::StandardButton button_pressed;
    QString msg = resources.count() == 1 ? tr( "Are you sure you want to delete the selected file?\n" ):
                                           tr( "Are you sure you want to delete all the selected files?\n" );
    button_pressed = QMessageBox::warning(	this,
                      tr( "Sigil" ), msg % tr( "This action cannot be reversed." ),
                                                QMessageBox::Ok | QMessageBox::Cancel
                                         );
    if ( button_pressed != QMessageBox::Ok )
    { 
        return;
    }

    Resource *next_resource = ResourceToSelectAfterRemove();
    if ( next_resource )
    {
        Resource::ResourceType type = next_resource->Type();
        if ( next_resource &&
            ( type == Resource::HTMLResourceType ||
              type == Resource::ImageResourceType ||
              type == Resource::CSSResourceType ) )

        {
            emit ResourceActivated( *next_resource );
        }
    }

    foreach ( Resource *resource, resources ) 
    {
        resource->Delete();
    }

    emit BookContentModified();

    // Avoid full refresh so selection stays for non-openable resources
    m_OPFModel.Refresh();
    if ( next_resource )
    {
        UpdateSelection( *next_resource );
    }
}


Resource* BookBrowser::ResourceToSelectAfterRemove()
{
    QList <Resource *> selected_resources = ValidSelectedResources();

    if ( selected_resources.isEmpty() )
    {
        return NULL;
    }

    QList <Resource *> all_resources = m_OPFModel.GetResourceListInFolder( selected_resources.first() );

    if ( all_resources.isEmpty() )
    {
        return NULL;
    }

    Resource *top_resource = NULL;
    Resource *bottom_resource = NULL;
    bool in_delete = false;

    foreach ( Resource *all_resource, all_resources )
    {
        bool found = false;
        foreach ( Resource *selected_resource, selected_resources )
        {
            if ( all_resource->Filename() == selected_resource->Filename() )
            {
                in_delete = true;
                found = true;
                break;
            }
        }
        if ( !in_delete && !found )
        {
            top_resource = all_resource;
        }
        if ( in_delete && !found && !bottom_resource )
        {
            bottom_resource = all_resource;
        }
    }


    if ( bottom_resource )
    {
        top_resource = bottom_resource;
    }
    else if ( !top_resource )
    {
        all_resources = AllHTMLResources();
        if ( !all_resources.isEmpty() )
        {
            top_resource = all_resources.first();
        }
    }

    return top_resource;
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

void BookBrowser::MergeAll()
{
    MergeList( m_OPFModel.GetResourceListInFolder( Resource::HTMLResourceType ) );
}

void BookBrowser::Merge()
{
    MergeList( ValidSelectedResources( Resource::HTMLResourceType ) );
}

void BookBrowser::MergeList( QList <Resource *> resources )
{
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
    else
    {
        QList <Resource *> all_files = m_OPFModel.GetResourceListInFolder( m_LastContextMenuType );
        QString msg = all_files.count() == resources.count() ? tr( "Are you sure you want to merge ALL files?  You can also select just some of the files to merge.\n" ):
                                           tr( "Are you sure you want to merge the selected files?\n" );

        QMessageBox::StandardButton button_pressed;
        button_pressed = QMessageBox::warning(	this,
                      tr( "Sigil" ), msg % tr( "This action cannot be reversed." ),
                                                QMessageBox::Ok | QMessageBox::Cancel
                                             );
        if ( button_pressed != QMessageBox::Ok )
        { 
            return;
        }
    }

    // Save location of first file in folder since resource will be modified
    QModelIndex original_model_index = m_OPFModel.GetModelItemIndex( *resource1, OPFModel::IndexChoice_Current );

    if ( !m_Book->AreResourcesWellFormed( resources ) )
    {
        // Both dialog and well-formed error messages will be shown
        // Newly added blank sections will generate an error if the book is not saved
        Utility::DisplayStdErrorDialog( tr( "Merge aborted.\n\nOne of the files may have an error or has not been saved.\n\nTry saving your book or correcting any errors before merging." ) );
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
    SettingsStore *settings = SettingsStore::instance();
    settings->beginGroup( SETTINGS_GROUP );

    m_LastFolderOpen = settings->value( "lastfolderopen" ).toString();

    settings->endGroup();
}


void BookBrowser::WriteSettings()
{
    SettingsStore *settings = SettingsStore::instance();
    settings->beginGroup( SETTINGS_GROUP );

    settings->setValue( "lastfolderopen", m_LastFolderOpen );

    settings->endGroup();
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
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();

    m_AddNewHTML              = new QAction( tr( "Add Blank Section" ),     this );
    m_AddNewCSS               = new QAction( tr( "Add Blank Stylesheet" ),  this );
    m_AddExisting             = new QAction( tr( "Add Existing Files..." ), this );
    m_Rename                  = new QAction( tr( "Rename" ),                this );
    m_RenameAll               = new QAction( tr( "Rename All" ),            this );
    m_Remove                  = new QAction( tr( "Remove" ),                this );
    m_CoverImage              = new QAction( tr( "Cover Image" ),           this );
    m_Merge                   = new QAction( tr( "Merge" ),                 this );
    m_MergeAll                = new QAction( tr( "Merge All" ),             this );
    m_MergeWithPrevious       = new QAction( tr( "Merge With Previous" ),   this );
    m_AdobesObfuscationMethod = new QAction( tr( "Use Adobe's Method" ),    this );
    m_IdpfsObfuscationMethod  = new QAction( tr( "Use IDPF's Method" ),     this );
    m_SortHTML                = new QAction( tr( "Sort All" ),              this );
    m_SortHTMLSelected        = new QAction( tr( "Sort" ),                  this );
    m_RefreshTOC              = new QAction( tr( "Renumber TOC Entries" ),  this );

    m_CoverImage             ->setCheckable( true );  
    m_AdobesObfuscationMethod->setCheckable( true ); 
    m_IdpfsObfuscationMethod ->setCheckable( true );

    m_Remove->setShortcut( QKeySequence::Delete );
    m_MergeWithPrevious->setShortcut( QKeySequence( Qt::CTRL + Qt::ALT + Qt::Key_M ) );
    m_Merge->setToolTip( "Merge with previous file, or merge multiple files into one." );
    sm->registerAction( m_Merge, "MainWindow.BookBrowser.Merge" );

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

        m_ContextMenu.addSeparator();

        m_ContextMenu.addAction( m_Rename );

        QList <Resource *> all_files = m_OPFModel.GetResourceListInFolder( m_LastContextMenuType );
        if ( ValidSelectedItemCount() == 1  && all_files.count() > 1 )
        {
            m_ContextMenu.addAction( m_RenameAll );
        }
    }

    SetupResourceSpecificContextMenu( resource );    

    return true;
}


void BookBrowser::SetupResourceSpecificContextMenu( Resource *resource  )
{
    if ( resource->Type() == Resource::HTMLResourceType )
    {
        QList <Resource *> all_files = m_OPFModel.GetResourceListInFolder( Resource::HTMLResourceType );
        if ( ValidSelectedItemCount() == 1  && all_files.count() > 1 )
        {
            m_ContextMenu.addAction( m_SortHTML );
        }
        else if ( ValidSelectedItemCount() > 1 )
        {
            m_ContextMenu.addAction( m_SortHTMLSelected );
        }

        AddMergeAction( resource ); 

        m_ContextMenu.addSeparator();

    }

    if ( resource->Type() == Resource::FontResourceType && ValidSelectedItemCount() == 1 )

        SetupFontObfuscationMenu( resource );

    if ( resource->Type() == Resource::NCXResourceType && ValidSelectedItemCount() == 1 )
    {
        m_ContextMenu.addAction( m_RefreshTOC );
    }

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

    foreach ( Resource *valid_resource, ValidSelectedResources() )
    {
        HTMLResource *valid_html_resource = qobject_cast< HTMLResource* >( valid_resource );
        Q_ASSERT( html_resource );

        GuideSemantics::GuideSemanticType valid_semantic_type = 
            m_Book->GetOPF().GetGuideSemanticTypeForResource( *valid_html_resource );

        if ( valid_semantic_type != semantic_type )
        {
            return;
        }
    }

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
        QList <Resource *> all_files = m_OPFModel.GetResourceListInFolder( Resource::HTMLResourceType );
        int reading_order = m_Book->GetOPF().GetReadingOrder( *html_resource );

        // We can't add the action for the first file
        if ( reading_order > 0 )
        {
            m_ContextMenu.addSeparator();
            m_ContextMenu.addAction( m_MergeWithPrevious );    
        }
        // And shouldn't add Merge All in some cases
        if ( all_files.count() > 2  || ( all_files.count() > 1 &&  reading_order == 0 ) )
        {
            if ( reading_order == 0 )
            {
                m_ContextMenu.addSeparator();
            }
            m_ContextMenu.addAction( m_MergeAll );
        }
    }
    else
    {
        m_ContextMenu.addSeparator();
        m_ContextMenu.addAction( m_Merge );
    }
}


void BookBrowser::ConnectSignalsToSlots()
{
    connect( &m_TreeView, SIGNAL( activated(             const QModelIndex& ) ),
             this,         SLOT(  EmitResourceActivated( const QModelIndex& ) ) );

    connect( &m_TreeView, SIGNAL( customContextMenuRequested( const QPoint& ) ),
             this,        SLOT(   OpenContextMenu(            const QPoint& ) ) );

    connect( &m_OPFModel, SIGNAL( UpdateSelection(                Resource& ) ),
             this,        SLOT(   UpdateSelection(                Resource& ) ) );

    connect( m_AddNewHTML,              SIGNAL( triggered() ), this, SLOT( AddNewHTML()              ) );
    connect( m_SortHTML,                SIGNAL( triggered() ), this, SLOT( SortHTML()                ) );
    connect( m_RefreshTOC,              SIGNAL( triggered() ), this, SLOT( RefreshTOC()              ) );
    connect( m_SortHTMLSelected,        SIGNAL( triggered() ), this, SLOT( SortHTML()                ) );
    connect( m_AddNewCSS,               SIGNAL( triggered() ), this, SLOT( AddNewCSS()               ) );
    connect( m_AddExisting,             SIGNAL( triggered() ), this, SLOT( AddExisting()             ) );
    connect( m_Rename,                  SIGNAL( triggered() ), this, SLOT( Rename()                  ) );
    connect( m_RenameAll,               SIGNAL( triggered() ), this, SLOT( RenameAll()               ) );
    connect( m_Remove,                  SIGNAL( triggered() ), this, SLOT( Remove()                  ) );
    connect( m_CoverImage,              SIGNAL( triggered() ), this, SLOT( SetCoverImage()           ) );
    connect( m_Merge,                   SIGNAL( triggered() ), this, SLOT( Merge()                   ) );
    connect( m_MergeWithPrevious,       SIGNAL( triggered() ), this, SLOT( Merge()                   ) );
    connect( m_MergeAll,                SIGNAL( triggered() ), this, SLOT( MergeAll()                ) );

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
