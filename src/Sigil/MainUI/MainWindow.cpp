/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
**  Copyright (C) 2012  John Schember <john@nachtimwald.com>
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
#include <QtGui/QDesktopServices>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>

#include "BookManipulation/BookNormalization.h"
#include "BookManipulation/FolderKeeper.h"
#include "Dialogs/About.h"
#include "Dialogs/HeadingSelector.h"
#include "Dialogs/ImageList.h"
#include "Dialogs/MetaEditor.h"
#include "Dialogs/Preferences.h"
#include "Exporters/ExportEPUB.h"
#include "Exporters/ExporterFactory.h"
#include "Importers/ImporterFactory.h"
#include "Importers/ImportHTML.h"
#include "MainUI/BookBrowser.h"
#include "MainUI/MainWindow.h"
#include "MainUI/FindReplace.h"
#include "MainUI/TableOfContents.h"
#include "MainUI/ValidationResultsView.h"
#include "Misc/KeyboardShortcutManager.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Misc/TOCHTMLWriter.h"
#include "Misc/Utility.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "Tabs/FlowTab.h"
#include "Tabs/OPFTab.h"
#include "Tabs/TabManager.h"

static const int TEXT_ELIDE_WIDTH           = 300;
static const QString SETTINGS_GROUP         = "mainwindow";
const float ZOOM_STEP                = 0.1f;
const float ZOOM_MIN                 = 0.09f;
const float ZOOM_MAX                 = 5.0f;
const float ZOOM_NORMAL              = 1.0f;
static const int ZOOM_SLIDER_MIN            = 0;
static const int ZOOM_SLIDER_MAX            = 1000;
static const int ZOOM_SLIDER_MIDDLE         = 500;
static const int ZOOM_SLIDER_WIDTH          = 140;
static const QString REPORTING_ISSUES_WIKI  = "http://code.google.com/p/sigil/wiki/ReportingIssues";
static const QString DONATE_WIKI            = "http://code.google.com/p/sigil/wiki/Donate";
static const QString SIGIL_DEV_BLOG         = "http://sigildev.blogspot.com/";
static const QString USER_MANUAL_URL        = "http://web.sigil.googlecode.com/git/contents.html";
static const QString FAQ_WIKI_URL           = "http://code.google.com/p/sigil/wiki/FAQ";

static const QString BOOK_BROWSER_NAME            = "bookbrowser";
static const QString FIND_REPLACE_NAME            = "findreplace";
static const QString VALIDATION_RESULTS_VIEW_NAME = "validationresultsname";
static const QString TABLE_OF_CONTENTS_NAME       = "tableofcontents";
static const QString FRAME_NAME                   = "managerframe";
static const QString TAB_STYLE_SHEET              = "#managerframe {border-top: 0px solid white;"
                                                    "border-left: 1px solid grey;"
                                                    "border-right: 1px solid grey;"
                                                    "border-bottom: 1px solid grey;} ";
static const QString HTML_TOC_FILE = "toc.html";

static const QStringList SUPPORTED_SAVE_TYPE = QStringList() << "epub"; 

QStringList MainWindow::s_RecentFiles = QStringList();

bool MainWindow::m_ShouldUseTidy = true;

MainWindow::MainWindow( const QString &openfilepath, QWidget *parent, Qt::WFlags flags )
    : 
    QMainWindow( parent, flags ),
    m_CurrentFilePath( QString() ),
    m_Book( new Book() ),
    m_LastFolderOpen( QString() ),
    m_LastFolderSave( QString() ),
    m_TabManager( *new TabManager( this ) ),
    m_BookBrowser( NULL ),
    m_FindReplace( new FindReplace( *this ) ),
    m_TableOfContents( NULL ),
    m_ValidationResultsView( NULL ),
    m_slZoomSlider( NULL ),
    m_lbZoomLabel( NULL ),
    c_SaveFilters( GetSaveFiltersMap() ),
    c_LoadFilters( GetLoadFiltersMap() ),
    m_CheckWellFormedErrors( true ),
    m_ViewState( MainWindow::ViewState_BookView )
{
    ui.setupUi( this );
    
    // Telling Qt to delete this window
    // from memory when it is closed
    setAttribute( Qt::WA_DeleteOnClose );

    ExtendUI();
    PlatformSpecificTweaks();

    // Needs to come before signals connect
    // (avoiding side-effects)
    ReadSettings();

    ConnectSignalsToSlots();

    CreateRecentFilesActions();
    UpdateRecentFileActions();

    ChangeSignalsWhenTabChanges(NULL, &m_TabManager.GetCurrentContentTab());

    LoadInitialFile(openfilepath);
    // Ensure the UI is setup properly for the default view state.
    SetViewState(m_ViewState);
}


void MainWindow::SaveBrowserSelection()
{
    return m_BookBrowser->SaveSelection();
}


void MainWindow::RestoreBrowserSelection()
{
    return m_BookBrowser->RestoreSelection();
}


QList <Resource *> MainWindow::GetValidSelectedHTMLResources()
{
    return m_BookBrowser->ValidSelectedHTMLResources();
}


QList <Resource *> MainWindow::GetAllHTMLResources()
{
    return m_BookBrowser->AllHTMLResources();
}


QSharedPointer< Book > MainWindow::GetCurrentBook()
{
    return m_Book;
}


ContentTab& MainWindow::GetCurrentContentTab()
{
    return m_TabManager.GetCurrentContentTab();
}


void MainWindow::OpenResource( Resource& resource,
                               bool precede_current_tab,
                               const QUrl &fragment,
                               MainWindow::ViewState view_state,
                               int line_to_scroll_to)
{
    MainWindow::ViewState vs = m_ViewState;
    if (view_state != MainWindow::ViewState_Unknown) {
        vs = view_state;
    }

    m_TabManager.OpenResource( resource, precede_current_tab, fragment, vs, line_to_scroll_to );

    if (vs != m_ViewState) {
        SetViewState(vs);
    }
}


QMutex& MainWindow::GetStatusBarMutex()
{
    return m_StatusBarMutex;
}


void MainWindow::ShowMessageOnCurrentStatusBar( const QString &message, 
                                                int millisecond_duration )
{
    MainWindow& main_window = GetCurrentMainWindow();
    QMutexLocker locker( &main_window.GetStatusBarMutex() );
    QStatusBar* status_bar = main_window.statusBar();

    // In Sigil, every MainWindow has to have a status bar
    Q_ASSERT( status_bar );

    status_bar->showMessage( message, millisecond_duration );
}


bool MainWindow::ShouldUseTidyClean()
{
    return m_ShouldUseTidy;
 
}


void MainWindow::closeEvent( QCloseEvent *event )
{
    m_TabManager.WellFormedDialogsEnabled( false );

    if ( MaybeSaveDialogSaysProceed() )
    {
        WriteSettings();

        event->accept();
    } 

    else
    {
        event->ignore();
        m_TabManager.WellFormedDialogsEnabled( true );
    }
}


void MainWindow::New()
{
    m_TabManager.WellFormedDialogsEnabled( false );

    // The nasty IFDEFs are here to enable the multi-document
    // interface on the Mac; Lin and Win just use multiple
    // instances of the Sigil application
#ifndef Q_WS_MAC
    if ( MaybeSaveDialogSaysProceed() )
#endif
    {
#ifdef Q_WS_MAC
        MainWindow *new_window = new MainWindow();
        new_window->show();
#else
        CreateNewBook();
#endif
    }

    m_TabManager.WellFormedDialogsEnabled( true );
}


void MainWindow::Open()
{
    m_TabManager.WellFormedDialogsEnabled( false );

    // The nasty IFDEFs are here to enable the multi-document
    // interface on the Mac; Lin and Win just use multiple
    // instances of the Sigil application
#ifndef Q_WS_MAC
    if ( MaybeSaveDialogSaysProceed() )
#endif
    {
        QStringList filters( c_LoadFilters.values() );
        filters.removeDuplicates();

        QString filter_string = "";

        foreach( QString filter, filters )
        {
            filter_string += filter + ";;";
        }

        // "All Files (*.*)" is the default
        QString default_filter = c_LoadFilters.value( "*" );

        QString filename = QFileDialog::getOpenFileName( this,
                                                         tr( "Open File" ),
                                                         m_LastFolderOpen,
                                                         filter_string,
                                                         &default_filter
                                                       );

        if ( !filename.isEmpty() )
        {
            // Store the folder the user opened from
            m_LastFolderOpen = QFileInfo( filename ).absolutePath();
            
#ifdef Q_WS_MAC
            MainWindow *new_window = new MainWindow( filename );
            new_window->show();
#else
            LoadFile( filename );
#endif
        }
    }

    m_TabManager.WellFormedDialogsEnabled( true );
}


void MainWindow::OpenRecentFile()
{
    // The nasty IFDEFs are here to enable the multi-document
    // interface on the Mac; Lin and Win just use multiple
    // instances of the Sigil application
    
    QAction *action = qobject_cast< QAction *>( sender() );
    
    if ( action != NULL )
    {
#ifndef Q_WS_MAC
        if ( MaybeSaveDialogSaysProceed() )
#endif
        {   

#ifdef Q_WS_MAC
            MainWindow *new_window = new MainWindow( action->data().toString() );
            new_window->show();
#else
            LoadFile( action->data().toString() );
#endif
        }
    }
}


bool MainWindow::Save()
{
    if ( !m_TabManager.TabDataIsWellFormed() )

        return false;

    if ( m_CurrentFilePath.isEmpty() )
    {
        return SaveAs();
    }

    else
    {
        return SaveFile( m_CurrentFilePath );
    }
}


bool MainWindow::SaveAs()
{
    if ( !m_TabManager.TabDataIsWellFormed() )

        return false;

    QStringList filters( c_SaveFilters.values() );
    filters.removeDuplicates();

    QString filter_string = "";

    foreach( QString filter, filters )
    {
        filter_string += filter + ";;";
    }

    QString save_path       = "";
    QString default_filter  = "";

    // If we can save this file type, then we use the current filename
    if ( c_SaveFilters.contains( QFileInfo( m_CurrentFilePath ).suffix().toLower() ) )
    {
        save_path       = m_LastFolderSave + "/" + QFileInfo( m_CurrentFilePath ).fileName();
        default_filter  = c_SaveFilters.value( QFileInfo( m_CurrentFilePath ).suffix().toLower() );
    }

    // If not, we change the extension to EPUB
    else
    {
        save_path       = m_LastFolderSave + "/" + QFileInfo( m_CurrentFilePath ).completeBaseName() + ".epub";
        default_filter  = c_SaveFilters.value( "epub" );
    }

    QString filename = QFileDialog::getSaveFileName( this, 
                                                     tr( "Save File" ), 
                                                     save_path,
                                                     filter_string,
                                                     &default_filter
                                                   );

    if ( filename.isEmpty() )

        return false;

    // Store the folder the user saved to
    m_LastFolderSave = QFileInfo( filename ).absolutePath();

    return SaveFile( filename );
}


void MainWindow::Find()
{
    m_TabManager.SaveTabData();

    m_FindReplace->SetUpFindText();
    m_FindReplace->show();
}


void MainWindow::GoToLine()
{
    ContentTab &tab = m_TabManager.GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }

    int line = QInputDialog::getInt( this, tr("Go To Line"), tr("Line #"), -1, 1 );
    if ( line >= 1 )
    {
        m_TabManager.OpenResource( tab.GetLoadedResource(), false, QUrl(), MainWindow::ViewState_CodeView, line );
        SetViewState(MainWindow::ViewState_CodeView);
    }
}


void MainWindow::ZoomIn()
{
    ZoomByStep( true );
}


void MainWindow::ZoomOut()
{
    ZoomByStep( false );  
}


void MainWindow::ZoomReset()
{
    ZoomByFactor( ZOOM_NORMAL );
}


void MainWindow::InsertImage()
{
    QStringList image_filepaths;

    QStringList all_filepaths = m_Book->GetFolderKeeper().GetAllFilenames();
    foreach (QString filepath, all_filepaths) {
        if (IMAGE_EXTENSIONS.contains(QFileInfo(filepath).suffix().toLower())) {
            image_filepaths.append(QFileInfo(filepath).fileName());
        }
    }

    if (image_filepaths.isEmpty()) {
        QMessageBox::warning( this,
                              tr( "Sigil"),
                              tr( "<p>There are no images available in your book to insert.</p><p>Use the menu option <b>File->New->Add Existing</b> to add images to your book before trying to insert an image.</p>")
                            );
        return;
    }

    ImageList image_list(this);
    image_list.setBasepath(m_Book->GetFolderKeeper().GetFullPathToImageFolder());
    image_list.setImages(image_filepaths);

    if (image_list.exec() == QDialog::Accepted) {
        QString selected_image = image_list.selectedImage();

        if (!selected_image.isEmpty()) {
            FlowTab &flow_tab = *qobject_cast<FlowTab*>(&m_TabManager.GetCurrentContentTab());
            Q_ASSERT(&flow_tab);

            const Resource &resource = m_Book->GetFolderKeeper().GetResourceByFilename(selected_image);
            const QString &relative_path = "../" + resource.GetRelativePathToOEBPS();

            flow_tab.InsertImage(relative_path);
        }
    }
}


void MainWindow::SetViewState(MainWindow::ViewState view_state)
{
    if (view_state == m_ViewState) {
        return;
    }
    if (view_state == MainWindow::ViewState_Unknown) {
        view_state = ViewState_BookView;
    }

    m_ViewState = view_state;
    UpdateViewState();
}


void MainWindow::SetTabViewState()
{
    SetViewState(m_ViewState);
}

void MainWindow::MergeResources(QList <Resource *> resources)
{
    if (resources.isEmpty()) {
        return;
    }

    // Convert merge previous to merge selected so all files can be checked for validity
    if (resources.count() == 1) {
        Resource *resource = m_Book->PreviousResource(resources.first());
        if (!resource) {
            QMessageBox::warning(this, tr("Sigil"), tr("One resource selected and there is no previous resource to merge into."));
            return;
        }
        resources.prepend(resource);
    }
    else {
        QMessageBox::StandardButton button_pressed;
        button_pressed = QMessageBox::warning(this, tr("Sigil"), tr("Are you sure you want to merge the selected files?\nThis action cannot be reversed."), QMessageBox::Ok | QMessageBox::Cancel);
        if (button_pressed != QMessageBox::Ok) {
            return;
        }
    }

    foreach (Resource *resource, resources) {
        if (!m_TabManager.TabDataIsWellFormed(*resource)) {
            Utility::DisplayStdErrorDialog(tr("Merge aborted.\n\nOne of the files may have an error or has not been saved.\n\nTry saving your book or correcting any errors before merging."));
            return;
        }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Save the tab data then dave it to disk.
    // Merging work based off of the data on disk.
    SaveTabData();
    m_Book->SaveAllResourcesToDisk();

    foreach (Resource *resource, resources) {
        if (!m_TabManager.TabDataIsWellFormed(*resource)) {
            QMessageBox::critical(this, tr("Sigil"), tr("Cannot merge: %1 data is not well formed.").arg(resource->Filename()));
            return;
        }
    }

    foreach (Resource *resource, resources) {
        if (!m_TabManager.CloseTabForResource(*resource)) {
            QMessageBox::critical(this, tr("Sigil"), tr("Cannot merge\n\nCannot close tab: %1").arg(resource->Filename()));
            return;
        }
    }

    Resource *resource1 = resources.takeFirst();
    HTMLResource &html_resource1 = *qobject_cast<HTMLResource *>(resource1);

    bool merge_okay = true;
    foreach (Resource *resource, resources) {
        if (resource != NULL && merge_okay) {
            HTMLResource &html_resource2 = *qobject_cast<HTMLResource *>(resource);
            merge_okay = m_Book->Merge(html_resource1, html_resource2);
        }
    }

    m_BookBrowser->Refresh();

    OpenResource(*resource1);
    UpdateBrowserSelectionToTab();

    QApplication::restoreOverrideCursor();
}

void MainWindow::GenerateToc()
{
    SaveTabData();

    QList<Resource *> resources = m_BookBrowser->AllHTMLResources();
    if (resources.isEmpty()) {
        return;
    }
    foreach (Resource *resource, resources) {
        if (!m_TabManager.TabDataIsWellFormed(*resource)) {
            QMessageBox::critical(this, tr("Sigil"), tr("Cannot generate TOC: %1 data is not well formed.").arg(resource->Filename()));
            return;
        }
    }

    {
        HeadingSelector toc(m_Book, this);
        if (toc.exec() != QDialog::Accepted) {
            return;
        }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Ensure that all headings have an id attribute
    BookNormalization::Normalize(m_Book);

    m_Book->GetNCX().GenerateNCXFromBookContents(*m_Book);
    m_TabManager.ReloadTabData();

    QApplication::restoreOverrideCursor();
}

void MainWindow::GenerateInlineToc(NCXModel::NCXEntry ncx_root_entry)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    HTMLResource *tocResource = NULL;
    QList<HTMLResource *> htmlResources;

    // Turn the list of Resources that are really HTMLResources to a real list
    // of HTMLResources.
    QList<Resource *> resources = m_BookBrowser->AllHTMLResources();
    foreach (Resource *resource, resources) {
        HTMLResource *htmlResource = qobject_cast<HTMLResource *>(resource);
        if (htmlResource) {
            htmlResources.append(htmlResource);

            // Check if this is an existing toc file.
            if (resource->Filename() == HTML_TOC_FILE) {
                tocResource = htmlResource;
                m_TabManager.CloseTabForResource(*tocResource);
            }
        }
    }

    // Create the an HTMLResource for the TOC if it doesn't exit.
    if (tocResource == NULL) {
        tocResource = &m_Book->CreateEmptyHTMLFile();
        tocResource->RenameTo(HTML_TOC_FILE);
        htmlResources.insert(0, tocResource);
        m_Book->GetOPF().UpdateSpineOrder(htmlResources);
    }

    TOCHTMLWriter toc(ncx_root_entry);
    tocResource->SetText(toc.WriteXML());
    m_Book->GetOPF().AddGuideSemanticType(*tocResource, GuideSemantics::TableOfContents);

    m_BookBrowser->Refresh();
    OpenResource(*tocResource);

    QApplication::restoreOverrideCursor();
}

void MainWindow::BookView()
{
    SetViewState( MainWindow::ViewState_BookView );
}


void MainWindow::SplitView()
{
    SetViewState( MainWindow::ViewState_SplitView );
}


void MainWindow::CodeView()
{
    SetViewState( MainWindow::ViewState_CodeView );
}


MainWindow::ViewState MainWindow::GetViewState()
{
    return m_ViewState;
}

void MainWindow::AnyCodeView()
{
    SetViewState( MainWindow::ViewState_CodeView );
}

bool MainWindow::CloseAllTabs()
{
    return m_TabManager.TryCloseAllTabs();
}

void MainWindow::SaveTabData()
{
    m_TabManager.SaveTabData();
}

void MainWindow::MetaEditorDialog()
{
    if ( !m_TabManager.TabDataIsWellFormed() )

        return;

    MetaEditor meta( m_Book->GetOPF(), this );
    meta.exec();
    // We really should be checking if the metadata was changed
    // not if the user clicked OK in the dialog.
    if (meta.result() == QDialog::Accepted) {
        m_Book->SetModified( true );
    }
}


void MainWindow::UserManual()
{
    QDesktopServices::openUrl( QUrl( USER_MANUAL_URL ) );
}


void MainWindow::FrequentlyAskedQuestions()
{
    QDesktopServices::openUrl( QUrl( FAQ_WIKI_URL ) );
}


void MainWindow::Donate()
{
    QDesktopServices::openUrl( QUrl( DONATE_WIKI ) );
}


void MainWindow::ReportAnIssue()
{
    QDesktopServices::openUrl( QUrl( REPORTING_ISSUES_WIKI ) );
}


void MainWindow::SigilDevBlog()
{
    QDesktopServices::openUrl( QUrl( SIGIL_DEV_BLOG ) );
}


void MainWindow::AboutDialog()
{
    About about( this );

    about.exec();
}


void MainWindow::PreferencesDialog()
{
    Preferences preferences( this );
    preferences.exec();

    emit SettingsChanged();
}


void MainWindow::ValidateEpub()
{
    m_ValidationResultsView->ValidateCurrentBook();
}


void MainWindow::ChangeSignalsWhenTabChanges( ContentTab* old_tab, ContentTab* new_tab )
{
    BreakTabConnections( old_tab );
    MakeTabConnections( new_tab );
}


void MainWindow::UpdateViewState()
{
    ContentTab &tab = GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }
    Resource::ResourceType type = tab.GetLoadedResource().Type();

    if (type == Resource::HTMLResourceType) {
        if (m_ViewState == MainWindow::ViewState_CodeView) {
            SetStateActionsCodeView();
        }
        else if (m_ViewState == MainWindow::ViewState_SplitView) {
            SetStateActionsSplitView();
        }
        else {
            if (m_ViewState != MainWindow::ViewState_BookView) {
                m_ViewState = MainWindow::ViewState_BookView;
            }
            SetStateActionsBookView();
        }

        FlowTab *ftab = dynamic_cast<FlowTab *>(&tab);
        if (ftab) {
            ftab->SetViewState(m_ViewState);
        }
    }
    else if (type == Resource::XMLResourceType ||
             type == Resource::XPGTResourceType ||
             type == Resource::OPFResourceType ||
             type == Resource::NCXResourceType ||
             type == Resource::TextResourceType ||
             type == Resource::CSSResourceType)
    {
        SetStateActionsRawView();
    }
    else {
        SetStateActionsStaticView();
    }
}


void MainWindow::UpdateUIOnTabChanges()
{
    ContentTab &tab = m_TabManager.GetCurrentContentTab();

    if ( &tab == NULL )

        return;

    ui.actionPrintPreview->setEnabled( tab.PrintEnabled() );
    ui.actionPrint->setEnabled( tab.PrintEnabled() );

    ui.actionCut  ->setEnabled( tab.CutEnabled() );
    ui.actionCopy ->setEnabled( tab.CopyEnabled() );
    ui.actionPaste->setEnabled( tab.PasteEnabled() );

    // State of zoom controls depends on current tab/view
    float zoom_factor = tab.GetZoomFactor();
    UpdateZoomLabel( zoom_factor );
    UpdateZoomSlider( zoom_factor );
}


void MainWindow::UpdateUiWhenTabsSwitch()
{    
    ContentTab &tab = GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }

    UpdateViewState();

    UpdateCursorPositionLabel(tab.GetCursorLine(), tab.GetCursorColumn());

    // State of zoom controls depends on current tab/view
    float zoom_factor = tab.GetZoomFactor();
    UpdateZoomLabel( zoom_factor );
    UpdateZoomSlider( zoom_factor );
}


void MainWindow::SetStateActionsBookView()
{
    ui.actionUndo->setEnabled(true);
    ui.actionRedo->setEnabled(true);

    ui.actionCut->setEnabled(true);
    ui.actionCopy->setEnabled(true);
    ui.actionPaste->setEnabled(true);

    ui.actionBookView->setEnabled(true);
    ui.actionSplitView->setEnabled(true);
    ui.actionCodeView->setEnabled(true);

    ui.actionBookView->setChecked(true);
    ui.actionSplitView->setChecked(false);
    ui.actionCodeView->setChecked(false);

    ui.actionInsertImage->setEnabled(true);
    ui.actionSplitChapter->setEnabled(true);

    m_FindReplace->SetCapabilities(FindReplace::CAPABILITY_FIND);
}

void MainWindow::SetStateActionsSplitView()
{
    SetStateActionsBookView();

    ui.actionBookView ->setChecked(false);
    ui.actionSplitView->setChecked(true);
    ui.actionCodeView ->setChecked(false);

    m_FindReplace->SetCapabilities(FindReplace::CAPABILITY_ALL);
}

void MainWindow::SetStateActionsCodeView()
{
    SetStateActionsBookView();

    ui.actionBookView ->setChecked(false);
    ui.actionSplitView->setChecked(false);
    ui.actionCodeView ->setChecked(true);

    m_FindReplace->SetCapabilities(FindReplace::CAPABILITY_ALL);
}


void MainWindow::SetStateActionsRawView()
{
    SetStateActionsCodeView();

    ui.actionBookView->setEnabled(false);
    ui.actionSplitView->setEnabled(false);
    ui.actionCodeView->setEnabled(false);

    ui.actionBookView->setChecked(false);
    ui.actionSplitView->setChecked(false);
    ui.actionCodeView->setChecked(false);

    ui.actionInsertImage->setEnabled(false);
    ui.actionSplitChapter->setEnabled(false);

    m_FindReplace->SetCapabilities(FindReplace::CAPABILITY_FIND |
        FindReplace::CAPABILITY_LOOK_CURRENT |
        FindReplace::CAPABILITY_REPLACE |
        FindReplace::CAPABILITY_MODE_NORMAL |
        FindReplace::CAPABILITY_MODE_CASE_SENSITIVE |
        FindReplace::CAPABILITY_MODE_REGEX |
        FindReplace::CAPABILITY_MODE_REGEX_DOT_ALL);
}


void MainWindow::SetStateActionsStaticView()
{
    SetStateActionsRawView();

    ui.actionUndo->setEnabled(false);
    ui.actionRedo->setEnabled(false);

    ui.actionCut->setEnabled(false);
    ui.actionCopy->setEnabled(false);
    ui.actionPaste->setEnabled(false);

    m_FindReplace->SetCapabilities(FindReplace::CAPABILITY_NONE);
}


void MainWindow::UpdateCursorPositionLabel(int line, int column)
{
    if (line >= 0 && column >=0) {
        const QString l = QString::number(line);
        const QString c = QString::number(column);

        m_lbCursorPosition->setText(tr("Line: %1, Col: %2").arg(l).arg(c));
    }
    else {
        m_lbCursorPosition->clear();
    }
}


void MainWindow::SliderZoom( int slider_value )
{
    ContentTab &tab = m_TabManager.GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }

    float new_zoom_factor     = SliderRangeToZoomFactor( slider_value );
    float current_zoom_factor = tab.GetZoomFactor();

    // We try to prevent infinite loops...
    if ( !qFuzzyCompare( new_zoom_factor, current_zoom_factor ) )

        ZoomByFactor( new_zoom_factor );
}


void MainWindow::UpdateZoomControls()
{
    ContentTab &tab = m_TabManager.GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }

    float zoom_factor = tab.GetZoomFactor();

    UpdateZoomSlider( zoom_factor );
    UpdateZoomLabel( zoom_factor );
}


void MainWindow::UpdateZoomSlider( float new_zoom_factor )
{
    m_slZoomSlider->setValue( ZoomFactorToSliderRange( new_zoom_factor ) );
}


void MainWindow::UpdateZoomLabel( int slider_value )
{
    float zoom_factor = SliderRangeToZoomFactor( slider_value );

    UpdateZoomLabel( zoom_factor );
}


void MainWindow::SetTidyCleanOption( bool new_state )
{
    m_ShouldUseTidy = new_state;
}


void MainWindow::SetCheckWellFormedErrors( bool new_state )
{
    m_CheckWellFormedErrors = new_state;
    m_TabManager.SetCheckWellFormedErrors( new_state );
}


void MainWindow::UpdateZoomLabel( float new_zoom_factor )
{
    m_lbZoomLabel->setText( QString( "%1% " ).arg( qRound( new_zoom_factor * 100 ) ) );
}


void MainWindow::CreateChapterBreakOldTab( QString content, HTMLResource& originating_resource )
{
    HTMLResource& html_resource = m_Book->CreateChapterBreakOriginalResource( content, originating_resource );

    m_BookBrowser->Refresh();

    OpenResource( html_resource, true, QUrl() );

    // We want the current tab to be scrolled to the top.
    FlowTab *flow_tab = qobject_cast< FlowTab* >( &GetCurrentContentTab() );
    if ( flow_tab )
    {
        flow_tab->ScrollToTop();
    }

    statusBar()->showMessage( tr( "Chapter split. You may need to update the Table of Contents." ), STATUSBAR_MSG_DISPLAY_TIME );
}


void MainWindow::CreateNewChapters( QStringList new_chapters, HTMLResource &originalResource )
{   
    m_Book->CreateNewChapters( new_chapters, originalResource );
    m_BookBrowser->Refresh();

    statusBar()->showMessage( tr( "Chapters split. You may need to update the Table of Contents." ), STATUSBAR_MSG_DISPLAY_TIME );
}


// Change the selected/highlighted resource to match the current tab
void MainWindow::UpdateBrowserSelectionToTab()
{
    ContentTab &tab = m_TabManager.GetCurrentContentTab();
    if ( &tab != NULL )
    {
        m_BookBrowser->UpdateSelection( tab.GetLoadedResource() );
    }
}


void MainWindow::ReadSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and its full screen status
    QByteArray geometry = settings.value( "geometry" ).toByteArray();

    if ( !geometry.isNull() )

        restoreGeometry( geometry );

    // The positions of all the toolbars and dock widgets
    QByteArray toolbars = settings.value( "toolbars" ).toByteArray();

    if ( !toolbars.isNull() )

        restoreState( toolbars );

    // For the tidyclean option, we want to default to true
    // if no value has been set.
    QVariant tidyclean = settings.value( "tidyclean" );
    m_ShouldUseTidy = tidyclean.isNull() ? true : tidyclean.toBool();
    ui.actionTidyClean->setChecked( m_ShouldUseTidy );

    // For the checkwellformed option, we want to default to true
    // if no value has been set.
    QVariant checkwellformederrors = settings.value( "checkwellformederrors" );
    m_CheckWellFormedErrors = checkwellformederrors.isNull() ? true : checkwellformederrors.toBool();
    ui.actionCheckWellFormedErrors->setChecked( m_CheckWellFormedErrors );
    SetCheckWellFormedErrors( m_CheckWellFormedErrors );

    // The position of the splitter handle in split view
    //QByteArray splitter_position = settings.value( "splitview_splitter" ).toByteArray();

    // FIXME: store splitter position... multiples?
    //if ( !splitter_position.isNull() )

    //    ui.splitter->restoreState( splitter_position );

    // The last folders used for saving and opening files
    m_LastFolderSave  = settings.value( "lastfoldersave"  ).toString();
    m_LastFolderOpen  = settings.value( "lastfolderopen"  ).toString();
    m_LastFolderAdd   = settings.value( "lastfolderadd" ).toString();

    // The list of recent files
    s_RecentFiles    = settings.value( "recentfiles" ).toStringList();

    settings.endGroup();
}


void MainWindow::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    settings.setValue( "geometry", saveGeometry() );

    // The positions of all the toolbars and dock widgets
    settings.setValue( "toolbars", saveState() );

    // Whether the user wants Tidy to be used.
    settings.setValue( "tidyclean", m_ShouldUseTidy );

    // Whether the user wants to be informed about well-formed errors
    settings.setValue( "checkwellformederrors", m_CheckWellFormedErrors );

    // The position of the splitter handle in split view
    // FIXME: splitter positions
    //settings.setValue( "splitview_splitter", ui.splitter->saveState() );

    // The last folders used for saving and opening files
    settings.setValue( "lastfoldersave",  m_LastFolderSave  );
    settings.setValue( "lastfolderopen",  m_LastFolderOpen  );
    settings.setValue( "lastfolderadd", m_LastFolderAdd );

    // The list of recent files
    settings.setValue( "recentfiles", s_RecentFiles );

    KeyboardShortcutManager::instance()->writeSettings();

    settings.endGroup();
}


bool MainWindow::MaybeSaveDialogSaysProceed()
{
    if ( isWindowModified() )
    {
        QMessageBox::StandardButton button_pressed;

        button_pressed = QMessageBox::warning(	this,
                                                tr( "Sigil" ),
                                                tr( "The document has been modified.\n"
                                                     "Do you want to save your changes?"),
                                                QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
                                             );

        if ( button_pressed == QMessageBox::Save )

            return Save();

        else if ( button_pressed == QMessageBox::Cancel )

            return false;
    }

    return true;
}


void MainWindow::SetNewBook( QSharedPointer< Book > new_book )
{
    m_Book = new_book;
    m_BookBrowser->SetBook( m_Book );
    m_TableOfContents->SetBook( m_Book );
    m_ValidationResultsView->SetBook( m_Book );

    connect( m_Book.data(), SIGNAL( ModifiedStateChanged( bool ) ), this, SLOT( setWindowModified( bool ) ) );
    connect( m_BookBrowser,     SIGNAL( GuideSemanticTypeAdded( const HTMLResource&, GuideSemantics::GuideSemanticType ) ),
             &m_Book->GetOPF(), SLOT(   AddGuideSemanticType(   const HTMLResource&, GuideSemantics::GuideSemanticType ) ) );
    connect( m_BookBrowser,     SIGNAL( CoverImageSet(           const ImageResource& ) ),
             &m_Book->GetOPF(), SLOT(   SetResourceAsCoverImage( const ImageResource& ) ) );
}


void MainWindow::CreateNewBook()
{
    QSharedPointer< Book > new_book = QSharedPointer< Book >( new Book() );
    new_book->CreateEmptyHTMLFile();
    
    SetNewBook( new_book );
    new_book->SetModified( false );
    UpdateUiWithCurrentFile( "" );
}


void MainWindow::LoadFile( const QString &fullfilepath )
{
    if ( !Utility::IsFileReadable( fullfilepath ) )

        return;

    try
    {
        QApplication::setOverrideCursor( Qt::WaitCursor );
        m_Book->SetModified( false );

        // Create the new book, clean up the old one
        // (destructors take care of that)
        SetNewBook( ImporterFactory().GetImporter( fullfilepath ).GetBook() );

        // The m_IsModified state variable is set in GetBook() to indicate whether the OPF
        // file was invalid and had to be recreated.
        // Since this happens before the connections have been established, it needs to be
        // tested and retoggled if true in order to indicate the actual state.
        if( m_Book->IsModified() )
        {
            m_Book->SetModified( false );
            m_Book->SetModified( true );
        }

        QApplication::restoreOverrideCursor();

        UpdateUiWithCurrentFile( fullfilepath );
        statusBar()->showMessage( tr( "File loaded" ), STATUSBAR_MSG_DISPLAY_TIME );
    }

    catch ( const FileEncryptedWithDrm& )
    {
        QApplication::restoreOverrideCursor();

        Utility::DisplayStdErrorDialog( 
            tr( "The creator of this file has encrypted it with DRM. "
                "Sigil cannot open such files." ) );        
    }
    catch ( const ExceptionBase &exception )
    {
        QApplication::restoreOverrideCursor();

        Utility::DisplayExceptionErrorDialog( tr("Cannot load file %1: %2").arg(fullfilepath).arg(Utility::GetExceptionInfo( exception ) ));
    }
}


bool MainWindow::SaveFile( const QString &fullfilepath )
{
    try
    {
        m_TabManager.SaveTabData();

        QString extension = QFileInfo( fullfilepath ).suffix().toLower();

        // TODO: Move to ExporterFactory and throw exception
        // when the user tries to save an unsupported type
        if ( !SUPPORTED_SAVE_TYPE.contains( extension ) )
        {
            Utility::DisplayStdErrorDialog( 
                tr( "Sigil currently cannot save files of type \"%1\".\n"
                    "Please choose a different format." )
                .arg( extension )
                );

            return false;
        }

        QApplication::setOverrideCursor( Qt::WaitCursor );

        BookNormalization::Normalize( m_Book );
        ExporterFactory().GetExporter( fullfilepath, m_Book ).WriteBook();
        
        QApplication::restoreOverrideCursor();

        // Return the focus back to the current tab
        ContentTab &tab = GetCurrentContentTab();

        if ( &tab != NULL )

            tab.setFocus();

        m_Book->SetModified( false );
        UpdateUiWithCurrentFile( fullfilepath );
        statusBar()->showMessage( tr( "File saved" ), STATUSBAR_MSG_DISPLAY_TIME );        
    }
    catch ( const ExceptionBase &exception )
    {
        QApplication::restoreOverrideCursor();

        Utility::DisplayExceptionErrorDialog(tr("Cannot save file %1: %2").arg(fullfilepath).arg(Utility::GetExceptionInfo( exception ) ));

            return false;
    }

    return true;
}


void MainWindow::ZoomByStep( bool zoom_in )
{
    ContentTab &tab = m_TabManager.GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }

    // We use a negative zoom stepping if we are zooming *out*
    float zoom_stepping       = zoom_in ? ZOOM_STEP : - ZOOM_STEP;

    // If we are zooming in, we round UP;
    // on zoom out, we round DOWN.
    float rounding_helper     = zoom_in ? 0.05f : - 0.05f;

    float current_zoom_factor = tab.GetZoomFactor();
    float rounded_zoom_factor = Utility::RoundToOneDecimal( current_zoom_factor + rounding_helper );

    // If the rounded value is nearly the same as the original value,
    // then the original was rounded to begin with and so we
    // add the zoom increment
    if ( qAbs( current_zoom_factor - rounded_zoom_factor ) < 0.01f )

        ZoomByFactor( Utility::RoundToOneDecimal( current_zoom_factor + zoom_stepping ) );

    // ...otherwise we first zoom to the rounded value
    else

        ZoomByFactor( rounded_zoom_factor );
}


void MainWindow::ZoomByFactor( float new_zoom_factor )
{
    ContentTab &tab = m_TabManager.GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }

    if ( new_zoom_factor > ZOOM_MAX || new_zoom_factor < ZOOM_MIN )

        return;

    tab.SetZoomFactor( new_zoom_factor );
}


int MainWindow::ZoomFactorToSliderRange( float zoom_factor )
{
    // We want a precise value for the 100% zoom,
    // so we pick up all float values near it.
    if ( qFuzzyCompare( zoom_factor, ZOOM_NORMAL ) ) 
    
        return ZOOM_SLIDER_MIDDLE;

    // We actually use two ranges: one for the below 100% zoom,
    // and one for the above 100%. This is so that the 100% mark
    // rests in the middle of the slider.
    if ( zoom_factor < ZOOM_NORMAL )
    {
         double range            = ZOOM_NORMAL - ZOOM_MIN;
         double normalized_value = zoom_factor - ZOOM_MIN;
         double range_proportion = normalized_value / range;
 
         return ZOOM_SLIDER_MIN + qRound( range_proportion * ( ZOOM_SLIDER_MIDDLE - ZOOM_SLIDER_MIN ) );
    }

    else
    {
        double range            = ZOOM_MAX - ZOOM_NORMAL;
        double normalized_value = zoom_factor - ZOOM_NORMAL;
        double range_proportion = normalized_value / range;

        return ZOOM_SLIDER_MIDDLE + qRound( range_proportion * ZOOM_SLIDER_MIDDLE );
    }
}


float MainWindow::SliderRangeToZoomFactor( int slider_range_value )
{
    // We want a precise value for the 100% zoom
    if ( slider_range_value == ZOOM_SLIDER_MIDDLE )

        return ZOOM_NORMAL;

    // We actually use two ranges: one for the below 100% zoom,
    // and one for the above 100%. This is so that the 100% mark
    // rests in the middle of the slider. 
    if ( slider_range_value < ZOOM_SLIDER_MIDDLE )
    {
        double range            = ZOOM_SLIDER_MIDDLE - ZOOM_SLIDER_MIN;
        double normalized_value = slider_range_value - ZOOM_SLIDER_MIN;
        double range_proportion = normalized_value / range;

        return ZOOM_MIN + range_proportion * ( ZOOM_NORMAL - ZOOM_MIN );
    }

    else
    {
        double range            = ZOOM_SLIDER_MAX - ZOOM_SLIDER_MIDDLE;
        double normalized_value = slider_range_value - ZOOM_SLIDER_MIDDLE;
        double range_proportion = normalized_value / range;

        return ZOOM_NORMAL + range_proportion * ( ZOOM_MAX - ZOOM_NORMAL );
    }
}


const QMap< QString, QString > MainWindow::GetLoadFiltersMap()
{
    QMap< QString, QString > file_filters;

    file_filters[ "epub"  ] = tr( "EPUB files (*.epub)" );
    file_filters[ "htm"   ] = tr( "HTML files (*.htm *.html *.xhtml)" );
    file_filters[ "html"  ] = tr( "HTML files (*.htm *.html *.xhtml)" );
    file_filters[ "xhtml" ] = tr( "HTML files (*.htm *.html *.xhtml)" );
    file_filters[ "txt"   ] = tr( "Text files (*.txt)" );
    file_filters[ "*"     ] = tr( "All files (*.*)" );

    return file_filters;
}


const QMap< QString, QString > MainWindow::GetSaveFiltersMap()
{
    QMap< QString, QString > file_filters;

    file_filters[ "epub" ] = tr( "EPUB file (*.epub)" );

    return file_filters;
}


MainWindow& MainWindow::GetCurrentMainWindow()
{
    QObject *object = qobject_cast< QObject* >( QApplication::activeWindow() );
    MainWindow *main_window = NULL;

    // In Sigil, every window has to be either a MainWindow,
    // or the child of one.
    while (true)
    {
        main_window = qobject_cast< MainWindow* >( object );

        if ( main_window )
        {
            break;
        }

        else
        {
            object = object->parent();
            Q_ASSERT( object );
        }
    }

    return *main_window;
}


void MainWindow::UpdateUiWithCurrentFile( const QString &fullfilepath )
{
    m_CurrentFilePath = fullfilepath;

    QString shownName = m_CurrentFilePath.isEmpty() ? "untitled.epub" : QFileInfo( m_CurrentFilePath ).fileName();

    // Update the titlebar
    setWindowTitle( tr( "%1[*] - %2" ).arg( shownName ).arg( tr( "Sigil" ) ) );

    if ( m_CurrentFilePath.isEmpty() )

        return;

    // Update recent files actions
    s_RecentFiles.removeAll( m_CurrentFilePath );
    s_RecentFiles.prepend( m_CurrentFilePath );

    while ( s_RecentFiles.size() > MAX_RECENT_FILES )
    {
        s_RecentFiles.removeLast();
    }
    
    // Update the recent files actions on
    // ALL the main windows
    foreach ( QWidget *window, QApplication::topLevelWidgets() ) 
    {
        if ( MainWindow *mainWin = qobject_cast< MainWindow * >( window ) )
            
            mainWin->UpdateRecentFileActions();
    }
}


void MainWindow::CreateRecentFilesActions()
{
    for ( int i = 0; i < MAX_RECENT_FILES; ++i ) 
    {
        m_RecentFileActions[ i ] = new QAction( this );

        // The actions are not visible until we put a filename in them
        m_RecentFileActions[ i ]->setVisible( false );

        QList<QAction *> actlist = ui.menuFile->actions();

        // Add the new action just below the Quit action
        // and the separator behind it
        ui.menuFile->insertAction( actlist[ actlist.size() - 3 ], m_RecentFileActions[ i ] );

        connect( m_RecentFileActions[ i ], SIGNAL( triggered() ), this, SLOT( OpenRecentFile() ) );
    }
}


void MainWindow::UpdateRecentFileActions()
{
    int num_recent_files = qMin( s_RecentFiles.size(), MAX_RECENT_FILES );

    // Store the filenames to the actions and display those actions
    for ( int i = 0; i < num_recent_files; ++i ) 
    {
        QString text = tr( "&%1 %2" ).arg( i + 1 ).arg( QFileInfo( s_RecentFiles[ i ] ).fileName() );

        m_RecentFileActions[ i ]->setText( fontMetrics().elidedText( text, Qt::ElideRight, TEXT_ELIDE_WIDTH ) );
        m_RecentFileActions[ i ]->setData( s_RecentFiles[ i ] );
        m_RecentFileActions[ i ]->setVisible( true );
    }

    // If we have fewer files than actions, hide the other actions
    for ( int j = num_recent_files; j < MAX_RECENT_FILES; ++j )
    {
        m_RecentFileActions[ j ]->setVisible( false );
    }

    QAction *separator = ui.menuFile->actions()[ ui.menuFile->actions().size() - 3 ];

    // If we have any actions with files shown,
    // display the separator; otherwise, don't
    if ( num_recent_files > 0 )

        separator->setVisible( true );

    else

        separator->setVisible( false );
}


void MainWindow::PlatformSpecificTweaks()
{
    // We use the "close" action only on Macs,
    // because they need it for the multi-document interface
#ifndef Q_WS_MAC
    ui.actionClose->setEnabled( false );
    ui.actionClose->setVisible( false );
#else
    // Macs also use bigger icons
    QList<QToolBar *> all_toolbars = findChildren<QToolBar *>();

    foreach( QToolBar *toolbar, all_toolbars )
    {
        toolbar->setIconSize( QSize( 32, 32 ) );
    }

    // The F11 shortcust is reserved for the OS on Macs,
    // so we change it to Cmd/Ctrl+F11
    ui.actionCodeView->setShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_F11 ) );
#endif
}


void MainWindow::ExtendUI()
{
    // Creating the tabs and the book browser 

    m_FindReplace->hide();
    // We want a nice frame around the tab manager
    QFrame *frame = new QFrame( this );
    QLayout *layout = new QVBoxLayout( frame );
    frame->setLayout( layout );
    layout->addWidget( &m_TabManager );
    layout->addWidget( m_FindReplace );
    layout->setContentsMargins( 0, 0, 0, 0 );
    layout->setSpacing( 1 );

    frame->setObjectName( FRAME_NAME );
    frame->setStyleSheet( TAB_STYLE_SHEET );

    setCentralWidget( frame );

    m_BookBrowser = new BookBrowser( this );
    m_BookBrowser->setObjectName( BOOK_BROWSER_NAME );
    addDockWidget( Qt::LeftDockWidgetArea, m_BookBrowser );

    m_TableOfContents = new TableOfContents( this );
    m_TableOfContents->setObjectName( TABLE_OF_CONTENTS_NAME );
    addDockWidget( Qt::RightDockWidgetArea, m_TableOfContents );

    m_ValidationResultsView = new ValidationResultsView( this );
    m_ValidationResultsView->setObjectName( VALIDATION_RESULTS_VIEW_NAME );
    addDockWidget( Qt::BottomDockWidgetArea, m_ValidationResultsView );

    // By default, we want the validation results view to be hidden
    // *for first-time users*. That is, when a new user installs and opens Sigil,
    // the val. results view is hidden, but if he leaves it open before exiting,
    // then it will be open when he opens Sigil the next time.
    // Basically, restoreGeometry() in ReadSettings() overrules this command.
    m_ValidationResultsView->hide();

    ui.menuView->addSeparator();
    ui.menuView->addAction( m_BookBrowser->toggleViewAction() );
    m_BookBrowser->toggleViewAction()->setShortcut( QKeySequence( Qt::ALT + Qt::Key_F1 ) );

    ui.menuView->addAction( m_ValidationResultsView->toggleViewAction() );
    m_ValidationResultsView->toggleViewAction()->setShortcut( QKeySequence( Qt::ALT + Qt::Key_F2 ) );

    ui.menuView->addAction( m_TableOfContents->toggleViewAction() );
    m_TableOfContents->toggleViewAction()->setShortcut( QKeySequence( Qt::ALT + Qt::Key_F3 ) );

    // Create the view menu to hide and show toolbars.
    ui.menuToolbars->addAction(ui.toolBarDonate->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarFileActions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarInsertions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextManip->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTools->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarViews->toggleViewAction());

    m_lbCursorPosition = new QLabel( QString (""), statusBar() );
    statusBar()->addPermanentWidget( m_lbCursorPosition );
    UpdateCursorPositionLabel(0, 0);

    // Creating the zoom controls in the status bar
    m_slZoomSlider = new QSlider( Qt::Horizontal, statusBar() );
    m_slZoomSlider->setTracking( false ); 
    m_slZoomSlider->setTickInterval( ZOOM_SLIDER_MIDDLE );
    m_slZoomSlider->setTickPosition( QSlider::TicksBelow );
    m_slZoomSlider->setFixedWidth( ZOOM_SLIDER_WIDTH );
    m_slZoomSlider->setMinimum( ZOOM_SLIDER_MIN );
    m_slZoomSlider->setMaximum( ZOOM_SLIDER_MAX );
    m_slZoomSlider->setValue( ZOOM_SLIDER_MIDDLE );

    QToolButton *zoom_out = new QToolButton( statusBar() );
    zoom_out->setDefaultAction( ui.actionZoomOut );

    QToolButton *zoom_in = new QToolButton( statusBar() );
    zoom_in->setDefaultAction( ui.actionZoomIn );

    m_lbZoomLabel = new QLabel( QString( "100% " ), statusBar() );
    
    statusBar()->addPermanentWidget( m_lbZoomLabel  );
    statusBar()->addPermanentWidget( zoom_out       );
    statusBar()->addPermanentWidget( m_slZoomSlider );
    statusBar()->addPermanentWidget( zoom_in        );

    // We override the default color for highlighted text
    // so we can actually *see* the text that the FindReplace
    // dialog finds in Book View... sadly, QWebView ignores a custom
    // palette set on it directly, so we have to do this globally.
    QPalette palette;
    palette.setColor( QPalette::Inactive, QPalette::Highlight, Qt::darkGreen );
    palette.setColor( QPalette::Inactive, QPalette::HighlightedText, Qt::white );
    qApp->setPalette( palette );

    // Setup userdefined keyboard shortcuts for actions.
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();
    // Note: shortcut action Ids should not be translated.
    // File
    sm->registerAction(ui.actionNew, "MainWindow.New");
    sm->registerAction(ui.actionNewHTMLFile, "MainWindow.NewHTMLFile");
    sm->registerAction(ui.actionNewCSSFile, "MainWindow.NewCSSFile");
    sm->registerAction(ui.actionOpen, "MainWindow.Open");
#ifndef Q_WS_MAC
    sm->registerAction(ui.actionClose, "MainWindow.Close");
#endif
    sm->registerAction(ui.actionSave, "MainWindow.Save");
    sm->registerAction(ui.actionSaveAs, "MainWindow.SaveAs");
    sm->registerAction(ui.actionPrintPreview, "MainWindow.PrintPreview");
    sm->registerAction(ui.actionPrint, "MainWindow.Print");
    sm->registerAction(ui.actionValidateEpub, "MainWindow.ValidateEpub");
    sm->registerAction(ui.actionExit, "MainWindow.Exit");
    // Edit
    sm->registerAction(ui.actionUndo, "MainWindow.Undo");
    sm->registerAction(ui.actionRedo, "MainWindow.Redo");
    sm->registerAction(ui.actionCut, "MainWindow.Cut");
    sm->registerAction(ui.actionCopy, "MainWindow.Copy");
    sm->registerAction(ui.actionPaste, "MainWindow.Paste");
    sm->registerAction(ui.actionFind, "MainWindow.Find");
    sm->registerAction(ui.actionFindNext, "MainWindow.FindNext");
    sm->registerAction(ui.actionFindPrevious, "MainWindow.FindPrevious");
    sm->registerAction(ui.actionReplaceNext, "MainWindow.ReplaceNext");
    sm->registerAction(ui.actionReplacePrevious, "MainWindow.ReplacePrevious");
    sm->registerAction(ui.actionReplaceAll, "MainWindow.ReplaceAll");
    sm->registerAction(ui.actionCount, "MainWindow.Count");
    sm->registerAction(ui.actionGoToLine, "MainWindow.GoToLine");
    sm->registerAction(ui.actionMetaEditor, "MainWindow.MetaEditor");
    sm->registerAction(ui.actionInsertImage, "MainWindow.InsertImage");
    sm->registerAction(ui.actionAddExistingFile, "MainWindow.AddExistingFile");
    sm->registerAction(ui.actionSplitChapter, "MainWindow.SplitChapter");
    sm->registerAction(ui.actionInsertSGFChapterMarker, "MainWindow.InsertSGFChapterMarker");
    sm->registerAction(ui.actionSplitOnSGFChapterMarkers, "MainWindow.SplitOnSGFChapterMarkers");
#ifndef Q_WS_MAC
    sm->registerAction(ui.actionPreferences, "MainWindow.Preferences");
#endif
    // View
    sm->registerAction(ui.actionBookView, "MainWindow.BookView");
    sm->registerAction(ui.actionSplitView, "MainWindow.SplitView");
    sm->registerAction(ui.actionCodeView, "MainWindow.CodeView");
    sm->registerAction(ui.actionZoomIn, "MainWindow.ZoomIn");
    sm->registerAction(ui.actionZoomOut, "MainWindow.ZoomOut");
    sm->registerAction(ui.actionZoomReset, "MainWindow.ZoomReset");
    sm->registerAction(m_BookBrowser->toggleViewAction(), "MainWindow.BookBrowser");
    sm->registerAction(m_ValidationResultsView->toggleViewAction(), "MainWindow.ValidationResults");
    sm->registerAction(m_TableOfContents->toggleViewAction(), "MainWindow.TableOfContents");
    // Tools
    sm->registerAction(ui.actionTidyClean, "MainWindow.TidyClean");
    sm->registerAction(ui.actionCheckWellFormedErrors, "MainWindow.CheckWellFormedErrors");
    // Window
    sm->registerAction(ui.actionNextTab, "MainWindow.NextTab");
    sm->registerAction(ui.actionPreviousTab, "MainWindow.PreviousTab");
    sm->registerAction(ui.actionCloseTab, "MainWindow.CloseTab");
    sm->registerAction(ui.actionCloseOtherTabs, "MainWindow.CloseOtherTabs");
    sm->registerAction(ui.actionOpenPreviousResource, "MainWindow.OpenPreviousResource");
    sm->registerAction(ui.actionOpenNextResource, "MainWindow.OpenNextResource");
    // Help
    sm->registerAction(ui.actionUserManual, "MainWindow.UserManual");
    sm->registerAction(ui.actionFAQ, "MainWindow.FAQ");
    sm->registerAction(ui.actionDonate, "MainWindow.Donate");
    sm->registerAction(ui.actionReportAnIssue, "MainWindow.ReportAnIssue");
    sm->registerAction(ui.actionSigilDevBlog, "MainWindow.SigilDevBlog");
    sm->registerAction(ui.actionAbout, "MainWindow.About");

    ExtendIconSizes();
}


void MainWindow::ExtendIconSizes()
{
    QIcon icon;
    icon = ui.actionNew->icon();
    icon.addFile(QString::fromUtf8(":/main/document-new_16px.png") );
    ui.actionNew->setIcon(icon);

    icon = ui.actionSave->icon();
    icon.addFile(QString::fromUtf8(":/main/document-save_16px.png"));
    ui.actionSave->setIcon(icon);

    icon = ui.actionSaveAs->icon();
    icon.addFile(QString::fromUtf8(":/main/document-save-as_16px.png"));
    ui.actionSaveAs->setIcon(icon);

    icon = ui.actionValidateEpub->icon();
    icon.addFile(QString::fromUtf8(":/main/document-validate_16px.png"));
    ui.actionValidateEpub->setIcon(icon);

    icon = ui.actionCut->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-cut_16px.png"));
    ui.actionCut->setIcon(icon);

    icon = ui.actionPaste->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-paste_16px.png"));
    ui.actionPaste->setIcon(icon);

    icon = ui.actionUndo->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-undo_16px.png"));
    ui.actionUndo->setIcon(icon);

    icon = ui.actionRedo->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-redo_16px.png"));
    ui.actionRedo->setIcon(icon);

    icon = ui.actionCopy->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-copy_16px.png"));
    ui.actionCopy->setIcon(icon);

    icon = ui.actionTidyClean->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-clear_16px.png"));
    ui.actionTidyClean->setIcon(icon);

    icon = ui.actionCheckWellFormedErrors->icon();
    icon.addFile(QString::fromUtf8(":/main/document-well-formed_check_16px.png"));
    ui.actionCheckWellFormedErrors->setIcon(icon);

    icon = ui.actionOpen->icon();
    icon.addFile(QString::fromUtf8(":/main/document-open_16px.png"));
    ui.actionOpen->setIcon(icon);

    icon = ui.actionExit->icon();
    icon.addFile(QString::fromUtf8(":/main/process-stop_16px.png"));
    ui.actionExit->setIcon(icon);

    icon = ui.actionAbout->icon();
    icon.addFile(QString::fromUtf8(":/main/help-browser_16px.png"));
    ui.actionAbout->setIcon(icon);

    icon = ui.actionBookView->icon();
    icon.addFile(QString::fromUtf8(":/main/view-book_16px.png"));
    ui.actionBookView->setIcon(icon);

    icon = ui.actionSplitView->icon();
    icon.addFile(QString::fromUtf8(":/main/view-split_16px.png"));
    ui.actionSplitView->setIcon(icon);

    icon = ui.actionCodeView->icon();
    icon.addFile(QString::fromUtf8(":/main/view-code_16px.png"));
    ui.actionCodeView->setIcon(icon);

    icon = ui.actionSplitChapter->icon();
    icon.addFile(QString::fromUtf8(":/main/insert-chapter-break_16px.png"));
    ui.actionSplitChapter->setIcon(icon);

    icon = ui.actionInsertImage->icon();
    icon.addFile(QString::fromUtf8(":/main/insert-image_16px.png"));
    ui.actionInsertImage->setIcon(icon);

    icon = ui.actionPrint->icon();
    icon.addFile(QString::fromUtf8(":/main/document-print_16px.png"));
    ui.actionPrint->setIcon(icon);

    icon = ui.actionPrintPreview->icon();
    icon.addFile(QString::fromUtf8(":/main/document-print-preview_16px.png"));
    ui.actionPrintPreview->setIcon(icon);

    icon = ui.actionZoomIn->icon();
    icon.addFile(QString::fromUtf8(":/main/list-add_16px.png"));
    ui.actionZoomIn->setIcon(icon);

    icon = ui.actionZoomOut->icon();
    icon.addFile(QString::fromUtf8(":/main/list-remove_16px.png"));
    ui.actionZoomOut->setIcon(icon);

    icon = ui.actionFind->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-find_16px.png"));
    ui.actionFind->setIcon(icon);

    icon = ui.actionTidyClean->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-clear_16px.png"));
    ui.actionTidyClean->setIcon(icon);

    icon = ui.actionDonate->icon();
    icon.addFile(QString::fromUtf8(":/main/emblem-favorite_16px.png"));
    ui.actionDonate->setIcon(icon);
}


void MainWindow::LoadInitialFile( const QString &openfilepath )
{
    if (!openfilepath.isEmpty()) {
        LoadFile( openfilepath);
    }
    else {
        CreateNewBook();
    }
}


void MainWindow::ConnectSignalsToSlots()
{
    connect( ui.actionExit,          SIGNAL( triggered() ), qApp, SLOT( closeAllWindows()          ) );
    connect( ui.actionClose,         SIGNAL( triggered() ), this, SLOT( close()                    ) );
    connect( ui.actionNew,           SIGNAL( triggered() ), this, SLOT( New()                      ) );
    connect( ui.actionOpen,          SIGNAL( triggered() ), this, SLOT( Open()                     ) );
    connect( ui.actionNewHTMLFile,   SIGNAL( triggered() ), m_BookBrowser, SLOT( AddNewHTML()      ) );
    connect( ui.actionNewCSSFile,    SIGNAL( triggered() ), m_BookBrowser, SLOT( AddNewCSS()       ) );
    connect( ui.actionAddExistingFile,SIGNAL(triggered() ), m_BookBrowser, SLOT( AddExisting()     ) );
    connect( ui.actionSave,          SIGNAL( triggered() ), this, SLOT( Save()                     ) );
    connect( ui.actionSaveAs,        SIGNAL( triggered() ), this, SLOT( SaveAs()                   ) );
    connect( ui.actionFind,          SIGNAL( triggered() ), this, SLOT( Find()                     ) );
    connect( ui.actionFindNext,      SIGNAL( triggered() ), m_FindReplace, SLOT( FindNext()        ) );
    connect( ui.actionFindPrevious,  SIGNAL( triggered() ), m_FindReplace, SLOT( FindPrevious()    ) );
    connect( ui.actionReplaceNext,   SIGNAL( triggered() ), m_FindReplace, SLOT( ReplaceNext()     ) );
    connect( ui.actionReplacePrevious,SIGNAL(triggered() ), m_FindReplace, SLOT( ReplacePrevious() ) );
    connect( ui.actionReplaceAll,    SIGNAL( triggered() ), m_FindReplace, SLOT( ReplaceAll()      ) );
    connect( ui.actionCount,         SIGNAL( triggered() ), m_FindReplace, SLOT( Count()           ) );
    connect( ui.actionGoToLine,      SIGNAL( triggered() ), this, SLOT( GoToLine()                 ) );
    connect( ui.actionZoomIn,        SIGNAL( triggered() ), this, SLOT( ZoomIn()                   ) );
    connect( ui.actionZoomOut,       SIGNAL( triggered() ), this, SLOT( ZoomOut()                  ) );
    connect( ui.actionZoomReset,     SIGNAL( triggered() ), this, SLOT( ZoomReset()                ) );
    connect( ui.actionOpenPreviousResource, SIGNAL( triggered() ), m_BookBrowser, SLOT( OpenPreviousResource() ) );
    connect( ui.actionOpenNextResource,     SIGNAL( triggered() ), m_BookBrowser, SLOT( OpenNextResource()     ) );
    connect( ui.actionInsertImage,   SIGNAL( triggered() ), this, SLOT( InsertImage()              ) );
    connect( ui.actionMetaEditor,    SIGNAL( triggered() ), this, SLOT( MetaEditorDialog()         ) );
    connect( ui.actionUserManual,    SIGNAL( triggered() ), this, SLOT( UserManual()               ) );
    connect( ui.actionFAQ,           SIGNAL( triggered() ), this, SLOT( FrequentlyAskedQuestions() ) );
    connect( ui.actionDonate,        SIGNAL( triggered() ), this, SLOT( Donate()                   ) );
    connect( ui.actionReportAnIssue, SIGNAL( triggered() ), this, SLOT( ReportAnIssue()            ) );
    connect( ui.actionSigilDevBlog,  SIGNAL( triggered() ), this, SLOT( SigilDevBlog()             ) );
    connect( ui.actionAbout,         SIGNAL( triggered() ), this, SLOT( AboutDialog()              ) );
    connect( ui.actionPreferences,   SIGNAL( triggered() ), this, SLOT( PreferencesDialog()        ) );
    connect( ui.actionValidateEpub,  SIGNAL( triggered() ), this, SLOT( ValidateEpub()             ) );

    connect( ui.actionNextTab,       SIGNAL( triggered() ), &m_TabManager, SLOT( NextTab()     ) );
    connect( ui.actionPreviousTab,   SIGNAL( triggered() ), &m_TabManager, SLOT( PreviousTab() ) );
    connect( ui.actionCloseTab,      SIGNAL( triggered() ), &m_TabManager, SLOT( CloseTab()    ) );
    connect( ui.actionCloseOtherTabs,SIGNAL( triggered() ), &m_TabManager, SLOT( CloseOtherTabs() ) );

    connect( m_slZoomSlider,         SIGNAL( valueChanged( int ) ), this, SLOT( SliderZoom( int ) ) );

    // We also update the label when the slider moves... this is to show
    // the zoom value the slider will land on while it is being moved.
    connect( m_slZoomSlider,         SIGNAL( sliderMoved( int ) ),  this, SLOT( UpdateZoomLabel( int ) ) );

    connect( ui.actionTidyClean,     SIGNAL( triggered( bool ) ),   this, SLOT( SetTidyCleanOption( bool ) ) );
    connect( ui.actionCheckWellFormedErrors, SIGNAL( triggered( bool ) ), this, SLOT( SetCheckWellFormedErrors( bool ) ) );
    
    connect( ui.actionBookView,      SIGNAL( triggered() ),  this,   SLOT( BookView()  ) );
    connect( ui.actionSplitView,     SIGNAL( triggered() ),  this,   SLOT( SplitView() ) );
    connect( ui.actionCodeView,      SIGNAL( triggered() ),  this,   SLOT( CodeView()  ) ); 

    connect( &m_TabManager,          SIGNAL( TabChanged( ContentTab*, ContentTab* ) ), 
             this,                   SLOT( ChangeSignalsWhenTabChanges( ContentTab*, ContentTab* ) ) );

    connect( &m_TabManager,          SIGNAL( TabChanged( ContentTab*, ContentTab* ) ), 
             this,                   SLOT( UpdateUIOnTabChanges() ) );

    connect( &m_TabManager,          SIGNAL( TabChanged( ContentTab*, ContentTab* ) ),
             this,                   SLOT( UpdateUiWhenTabsSwitch() ) );

    connect( &m_TabManager,          SIGNAL( TabChanged( ContentTab*, ContentTab* ) ),
            this,                    SLOT(   UpdateBrowserSelectionToTab() ) );

    connect( &m_TabManager,          SIGNAL( TabChanged( ContentTab*, ContentTab* ) ), 
             this,                   SLOT(   SetTabViewState() ) );

    connect( m_BookBrowser,          SIGNAL( UpdateBrowserSelection() ),
            this,                    SLOT(   UpdateBrowserSelectionToTab() ) );

    connect( m_BookBrowser, SIGNAL( RefreshTOCContentsRequest() ),
             m_TableOfContents,     SLOT(   RefreshTOCContents() ) );

    connect( m_TableOfContents, SIGNAL( GenerateTocRequest() ),
             this,     SLOT(   GenerateToc() ) );
    connect(m_TableOfContents, SIGNAL(GenerateInlineTocRequest(NCXModel::NCXEntry)), this, SLOT(GenerateInlineToc(NCXModel::NCXEntry)));

    connect( m_BookBrowser, SIGNAL( RemoveTabRequest() ),
             &m_TabManager, SLOT(   RemoveTab() ) );

    connect( m_BookBrowser, SIGNAL( ResourceActivated( Resource& ) ),
             this, SLOT(   OpenResource(          Resource& ) ) );

    connect( m_BookBrowser, SIGNAL( OpenResourceRequest( Resource&, bool, const QUrl& ) ),
             this, SLOT(   OpenResource(        Resource&, bool, const QUrl& ) ) );

    connect(m_BookBrowser, SIGNAL(MergeResourcesRequest(QList<Resource *>)), this, SLOT(MergeResources(QList<Resource *>)));

    connect( m_TableOfContents, SIGNAL( OpenResourceRequest( Resource&, bool, const QUrl& ) ),
             this,     SLOT(   OpenResource(        Resource&, bool, const QUrl& ) ) );

    connect( m_ValidationResultsView, 
                SIGNAL( OpenResourceRequest( Resource&, bool, const QUrl&, MainWindow::ViewState, int ) ),
             this,
                SLOT(   OpenResource(        Resource&, bool, const QUrl&, MainWindow::ViewState, int ) ) );
    
    connect( &m_TabManager, SIGNAL( OpenUrlRequest(  const QUrl& ) ),
             m_BookBrowser, SLOT(   OpenUrlResource( const QUrl& ) ) );

    connect( &m_TabManager, SIGNAL( OldTabRequest(            QString, HTMLResource& ) ),
             this,          SLOT(   CreateChapterBreakOldTab( QString, HTMLResource& ) ) );

    connect( &m_TabManager, SIGNAL( NewChaptersRequest( QStringList, HTMLResource& ) ),
             this,          SLOT(   CreateNewChapters(  QStringList, HTMLResource& ) ) );
}


void MainWindow::MakeTabConnections( ContentTab *tab )
{
    if ( tab == NULL )

        return;

    if (tab->GetLoadedResource().Type() != Resource::ImageResourceType)
    {
        connect( ui.actionUndo,                     SIGNAL( triggered() ),  tab,   SLOT( Undo()                     ) );
        connect( ui.actionRedo,                     SIGNAL( triggered() ),  tab,   SLOT( Redo()                     ) );
        connect( ui.actionCut,                      SIGNAL( triggered() ),  tab,   SLOT( Cut()                      ) );
        connect( ui.actionCopy,                     SIGNAL( triggered() ),  tab,   SLOT( Copy()                     ) );
        connect( ui.actionPaste,                    SIGNAL( triggered() ),  tab,   SLOT( Paste()                    ) );
    }

    if (tab->GetLoadedResource().Type() == Resource::HTMLResourceType )
    {
        connect( ui.actionSplitChapter,             SIGNAL( triggered() ),  tab,   SLOT( SplitChapter()             ) );
        connect( ui.actionInsertSGFChapterMarker,   SIGNAL( triggered() ),  tab,   SLOT( InsertSGFChapterMarker()   ) );
        connect( ui.actionSplitOnSGFChapterMarkers, SIGNAL( triggered() ),  tab,   SLOT( SplitOnSGFChapterMarkers() ) );

        connect( ui.actionPrintPreview,             SIGNAL( triggered() ),  tab,   SLOT( PrintPreview()             ) );
        connect( ui.actionPrint,                    SIGNAL( triggered() ),  tab,   SLOT( Print()                    ) );
        connect( this,                              SIGNAL( SettingsChanged()), tab, SLOT( LoadSettings()           ) );
    
        connect( tab,   SIGNAL( ViewButtonsStateChanged() ),    this,          SLOT( UpdateUIOnTabChanges()    ) );
        connect( tab,   SIGNAL( ViewChanged() ),                this,          SLOT( UpdateUIOnTabChanges()    ) );
        connect( tab,   SIGNAL( SelectionChanged() ),           this,          SLOT( UpdateUIOnTabChanges()    ) );
        connect( tab,   SIGNAL( EnteringBookView() ),           this,          SLOT( SetStateActionsBookView() ) );
        connect( tab,   SIGNAL( EnteringBookPreview() ),        this,          SLOT( SetStateActionsSplitView() ) );
        connect( tab,   SIGNAL( EnteringCodeView() ),           this,          SLOT( SetStateActionsCodeView() ) );
        connect( tab,   SIGNAL( EnteringBookView() ),           this,          SLOT( UpdateZoomControls()      ) );
        connect( tab,   SIGNAL( EnteringBookPreview() ),        this,          SLOT( UpdateZoomControls() ) );
        connect( tab,   SIGNAL( EnteringCodeView() ),           this,          SLOT( UpdateZoomControls()      ) );
    }

    connect( tab,   SIGNAL( ContentChanged() ),             m_Book.data(), SLOT( SetModified()             ) );
    connect(tab, SIGNAL(UpdateCursorPosition(int,int)), this, SLOT(UpdateCursorPositionLabel(int,int)));
    connect( tab,   SIGNAL( ZoomFactorChanged( float ) ),   this,          SLOT( UpdateZoomLabel( float )  ) );
    connect( tab,   SIGNAL( ZoomFactorChanged( float ) ),   this,          SLOT( UpdateZoomSlider( float ) ) );
}


void MainWindow::BreakTabConnections( ContentTab *tab )
{
    if ( tab == NULL )

        return;

    disconnect( ui.actionUndo,                      0, tab, 0 );
    disconnect( ui.actionRedo,                      0, tab, 0 );
    disconnect( ui.actionCut,                       0, tab, 0 );
    disconnect( ui.actionCopy,                      0, tab, 0 );
    disconnect( ui.actionPaste,                     0, tab, 0 );
    disconnect( ui.actionSplitChapter,              0, tab, 0 );
    disconnect( ui.actionInsertSGFChapterMarker,    0, tab, 0 );
    disconnect( ui.actionSplitOnSGFChapterMarkers,  0, tab, 0 );

    disconnect( ui.actionPrintPreview,              0, tab, 0 );
    disconnect( ui.actionPrint,                     0, tab, 0 );

    disconnect( tab,                                0, this, 0 );
    disconnect( tab,                                0, m_Book.data(), 0 );
}

