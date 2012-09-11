/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
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
#include <QtGui/QDesktopServices>
#include <QtGui/QFileDialog>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>
#include <QtGui/QToolBar>
#include <QtWebKit/QWebSettings>


#include "BookManipulation/BookNormalization.h"
#include "BookManipulation/Index.h"
#include "BookManipulation/FolderKeeper.h"
#include "Dialogs/About.h"
#include "Dialogs/ClipEditor.h"
#include "Dialogs/ClipboardHistorySelector.h"
#include "Dialogs/HeadingSelector.h"
#include "Dialogs/LinkStylesheets.h"
#include "Dialogs/MetaEditor.h"
#include "Dialogs/Preferences.h"
#include "Dialogs/SearchEditor.h"
#include "Dialogs/SelectCharacter.h"
#include "Dialogs/SelectImages.h"
#include "Dialogs/SelectHyperlink.h"
#include "Dialogs/SelectId.h"
#include "Dialogs/Reports.h"
#include "Exporters/ExportEPUB.h"
#include "Exporters/ExporterFactory.h"
#include "Importers/ImporterFactory.h"
#include "Importers/ImportHTML.h"
#include "MainUI/BookBrowser.h"
#include "MainUI/MainWindow.h"
#include "MainUI/FindReplace.h"
#include "MainUI/TableOfContents.h"
#include "MainUI/ValidationResultsView.h"
#include "Misc/CSSInfo.h"
#include "Misc/KeyboardShortcutManager.h"
#include "Misc/SettingsStore.h"
#include "Misc/SpellCheck.h"
#include "Misc/TOCHTMLWriter.h"
#include "Misc/Utility.h"
#include "MiscEditors/IndexHTMLWriter.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "SourceUpdates/LinkUpdates.h"
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
static const QString USER_GUIDE_URL         = "http://web.sigil.googlecode.com/git/files/OEBPS/Text/introduction.html";
static const QString FAQ_URL                = "http://web.sigil.googlecode.com/git/files/OEBPS/Text/faq.html";
static const QString TUTORIALS_URL          = "http://web.sigil.googlecode.com/git/files/OEBPS/Text/tutorials.html";

static const QString BOOK_BROWSER_NAME            = "bookbrowser";
static const QString FIND_REPLACE_NAME            = "findreplace";
static const QString VALIDATION_RESULTS_VIEW_NAME = "validationresultsname";
static const QString TABLE_OF_CONTENTS_NAME       = "tableofcontents";
static const QString FRAME_NAME                   = "managerframe";
static const QString TAB_STYLE_SHEET              = "#managerframe {border-top: 0px solid white;"
                                                    "border-left: 1px solid grey;"
                                                    "border-right: 1px solid grey;"
                                                    "border-bottom: 1px solid grey;} ";
static const QString HTML_TOC_FILE = "TOC.html";
static const QString HTML_INDEX_FILE = "Index.html";

static const QStringList SUPPORTED_SAVE_TYPE = QStringList() << "epub";

QStringList MainWindow::s_RecentFiles = QStringList();

MainWindow::MainWindow( const QString &openfilepath, QWidget *parent, Qt::WFlags flags )
    :
    QMainWindow( parent, flags ),
    m_cleanMapper(new QSignalMapper(this)),
    m_CurrentFilePath( QString() ),
    m_Book( new Book() ),
    m_LastFolderOpen( QString() ),
    m_SaveACopyFilename( QString() ),
    m_LastInsertedImage( QString() ),
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
    m_ViewState( MainWindow::ViewState_BookView ),
    m_headingMapper( new QSignalMapper( this ) ),
    m_casingChangeMapper( new QSignalMapper( this ) ),
    m_SearchEditor(new SearchEditor(this)),
    m_ClipEditor(new ClipEditor(this)),
    m_IndexEditor(new IndexEditor(this)),
    m_SelectCharacter(new SelectCharacter(this)),
    m_preserveHeadingAttributes( true ),
    m_LinkOrStyleBookmark(new LocationBookmark()),
    m_ClipboardHistorySelector(new ClipboardHistorySelector(this))
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


void MainWindow::SelectResources(QList<Resource *> resources)
{
    return m_BookBrowser->SelectResources(resources);
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

void MainWindow::OpenFilename( QString filename )
{
    QList<Resource *> resources = m_BookBrowser->AllImageResources() + m_BookBrowser->AllHTMLResources() + m_BookBrowser->AllCSSResources();
    foreach (Resource *resource, resources) {
        if (resource->Filename() == filename) {
            OpenResource(*resource);
            break;
        }
    }
}

void MainWindow::ResetLinkOrStyleBookmark()
{
    ResetLocationBookmark(m_LinkOrStyleBookmark);
    ui.actionGoBackFromLinkOrStyle->setEnabled(false);
}

void MainWindow::ResetLocationBookmark(MainWindow::LocationBookmark *locationBookmark)
{
    locationBookmark->filename = QString();
    locationBookmark->view_state = MainWindow::ViewState_Unknown;
    locationBookmark->bv_caret_location_update = QString();
    locationBookmark->cv_cursor_position = -1;
}

void MainWindow::GoBackFromLinkOrStyle()
{
    GoToBookmark(m_LinkOrStyleBookmark);
}

void MainWindow::GoToBookmark(MainWindow::LocationBookmark *locationBookmark)
{
    if (locationBookmark->filename.isEmpty()) {
        return;
    }

    try
    {
        Resource &resource = m_Book->GetFolderKeeper().GetResourceByFilename(locationBookmark->filename);

        SetViewState(locationBookmark->view_state);
        OpenResource(resource, false, QUrl(), locationBookmark->view_state, -1, locationBookmark->cv_cursor_position, locationBookmark->bv_caret_location_update);
    }
    catch (const ResourceDoesNotExist&)
    {
        // Nothing. Old file must have been deleted.
        ResetLocationBookmark(locationBookmark);
    }
}

void MainWindow::BookmarkLinkOrStyleLocation()
{
    ResetLinkOrStyleBookmark();

    ContentTab &tab = GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }
    Resource *current_resource = &tab.GetLoadedResource();

    m_LinkOrStyleBookmark->view_state = m_ViewState;
    m_LinkOrStyleBookmark->filename = current_resource->Filename();
    m_LinkOrStyleBookmark->cv_cursor_position = tab.GetCursorPosition();
    m_LinkOrStyleBookmark->bv_caret_location_update = tab.GetCaretLocationUpdate();

    ui.actionGoBackFromLinkOrStyle->setEnabled(!m_LinkOrStyleBookmark->filename.isEmpty());
}

void MainWindow::OpenUrl(const QUrl& url)
{
    if (url.isEmpty()) {
        return;
    }

    BookmarkLinkOrStyleLocation();

    if (url.scheme().isEmpty() || url.scheme() == "file") {
        Resource *resource = m_BookBrowser->GetUrlResource(url);
        if (!resource) {
            ResetLinkOrStyleBookmark();
            return;
        }
        if (url.fragment().isEmpty()) {;
            // If empty fragment force view to top of page
            OpenResource(*resource, false, url.fragment(), MainWindow::ViewState_Unknown, 1);
        }
        else {
            OpenResource(*resource, false, url.fragment());
        }
    } 
    else {
        QMessageBox::StandardButton button_pressed;
        button_pressed = QMessageBox::warning(this, tr("Sigil"), tr("Are you sure you want to open this external link?\n\n%1").arg(url.toString()), QMessageBox::Ok | QMessageBox::Cancel);
        if (button_pressed == QMessageBox::Ok) {
            QDesktopServices::openUrl(url);
        }
    }
}

void MainWindow::OpenResource( Resource& resource,
                               bool precede_current_tab,
                               const QUrl &fragment,
                               MainWindow::ViewState view_state,
                               int line_to_scroll_to,
                               int position_to_scroll_to,
                               QString caret_location_to_scroll_to,
                               bool grab_focus)
{
    MainWindow::ViewState vs = m_ViewState;
    if (view_state != MainWindow::ViewState_Unknown) {
        vs = view_state;
    }

    m_TabManager.OpenResource( resource, precede_current_tab, fragment, vs, line_to_scroll_to, position_to_scroll_to, caret_location_to_scroll_to, grab_focus );

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
        QString extension = QFileInfo( m_CurrentFilePath ).suffix().toLower();
        if ( !SUPPORTED_SAVE_TYPE.contains( extension ) )
        {
            return SaveAs();
        }

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
        save_path       = m_LastFolderOpen + "/" + QFileInfo( m_CurrentFilePath ).fileName();
        default_filter  = c_SaveFilters.value( QFileInfo( m_CurrentFilePath ).suffix().toLower() );
    }

    // If not, we change the extension to EPUB
    else
    {
        save_path       = m_LastFolderOpen + "/" + QFileInfo( m_CurrentFilePath ).completeBaseName() + ".epub";
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
    m_LastFolderOpen = QFileInfo( filename ).absolutePath();

    return SaveFile( filename );
}


bool MainWindow::SaveACopy()
{
    if (!m_TabManager.TabDataIsWellFormed()) {
        return false;
    }

    QStringList filters(c_SaveFilters.values());
    filters.removeDuplicates();

    QString filter_string = "*.epub";
    QString default_filter  = "*.epub";

    QString filename = QFileDialog::getSaveFileName( this,
                                                     tr( "Save a Copy" ),
                                                     m_SaveACopyFilename,
                                                     filter_string,
                                                     &default_filter
                                                   );

    if (filename.isEmpty()) {
        return false;
    }

    QString extension = QFileInfo(filename).suffix();
    if (extension.isEmpty()) {
        filename += ".epub";
    }

    // Store the filename the user saved to
    m_SaveACopyFilename = filename;

    return SaveFile(filename, false);
}


void MainWindow::Find()
{
    m_TabManager.SaveTabData();

    m_FindReplace->SetUpFindText();
    m_FindReplace->show();
}


void MainWindow::GoToLine()
{
    ContentTab &tab = GetCurrentContentTab();
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

void MainWindow::GoToLinkedStyleDefinition( const QString &element_name, const QString &style_class_name )
{
    // Invoked via a signal when the user has requested to navigate to a 
    // style definition and none was found in the inline styles, so look
    // at the linked resources for this tab instead.
    ContentTab &tab = GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }

    Resource *current_resource = &tab.GetLoadedResource();
    if (current_resource->Type() == Resource::HTMLResourceType) {
        BookmarkLinkOrStyleLocation();

        // Look in the linked stylesheets for a match
        QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();
        QStringList stylesheets = GetStylesheetsAlreadyLinked( current_resource );

        bool found_match = false;
        CSSResource *first_css_resource = 0;
        foreach( QString pathname, stylesheets )
        {
            // Check whether the stylesheet contains this style
            CSSResource *css_resource = NULL;
            foreach ( Resource *resource, css_resources )
            {
                if ( pathname == QString( "../" + resource->GetRelativePathToOEBPS()) ) {
                    // We have our resource matching this stylesheet.
                    css_resource = dynamic_cast<CSSResource *>( resource );
                    if (!first_css_resource) {
                        first_css_resource = css_resource;
                    }
                    break;
                }
            }
            CSSInfo css_info(css_resource->GetText(), true);
            CSSInfo::CSSSelector* selector = css_info.getCSSSelectorForElementClass(element_name, style_class_name);
            if (selector) {
                m_TabManager.OpenResource( *css_resource, false, QUrl(), MainWindow::ViewState_RawView,
                                            selector->line, -1, QString(), true );
                found_match = true;
                break;
            }
        }

        if (!found_match) {
            QString display_name;
            if (style_class_name.isEmpty()) {
                display_name = element_name;
            }
            else {
                display_name = QString(".%1 / %2.%1").arg(style_class_name).arg(element_name);
            }
            ShowMessageOnCurrentStatusBar(QString( tr("No CSS styles named") +  " " + display_name), 5000);
            // Open the first linked stylesheet if any
            if (first_css_resource) {
                OpenResource(*first_css_resource, false, QUrl(), MainWindow::ViewState_Unknown, 1);
            }
        }
    }
}


void MainWindow::SetRegexOptionDotAll( bool new_state )
{
    ui.actionRegexDotAll->setChecked( new_state );
    m_FindReplace->SetRegexOptionDotAll( new_state );
}


void MainWindow::SetRegexOptionMinimalMatch( bool new_state )
{
    ui.actionRegexMinimalMatch->setChecked( new_state );
    m_FindReplace->SetRegexOptionMinimalMatch( new_state );
}


void MainWindow::SetRegexOptionAutoTokenise( bool new_state )
{
    ui.actionRegexAutoTokenise->setChecked( new_state );
    m_FindReplace->SetRegexOptionAutoTokenise( new_state );
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


void MainWindow::IndexEditorDialog(IndexEditorModel::indexEntry* index_entry)
{
    if (!m_TabManager.TabDataIsWellFormed()) {
        return;
    }
    m_TabManager.SaveTabData();

    // non-modal dialog
    m_IndexEditor->show();
    m_IndexEditor->raise();
    m_IndexEditor->activateWindow();

    if (index_entry) {
        m_IndexEditor->AddEntry(false, index_entry, false);
    }
}

void MainWindow::CreateIndex()
{
    if (!m_TabManager.TabDataIsWellFormed()) {
        return;
    }
    SaveTabData();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    HTMLResource *index_resource = NULL;
    QList<HTMLResource *> html_resources;

    // Turn the list of Resources that are really HTMLResources to a real list
    // of HTMLResources.
    QList<Resource *> resources = m_BookBrowser->AllHTMLResources();
    foreach (Resource *resource, resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (html_resource) {
            html_resources.append(html_resource);

            // Check if this is an existing index file.
            if (m_Book->GetOPF().GetGuideSemanticTypeForResource(*html_resource) == GuideSemantics::Index) {
                index_resource = html_resource;
            }
            else if (resource->Filename() == HTML_INDEX_FILE && html_resource == NULL) {
                index_resource = html_resource;
            }
        }
    }

    // Close the tab so the focus saving doesn't overwrite the text were
    // replacing in the resource.
    if (index_resource != NULL) {
        m_TabManager.CloseTabForResource(*index_resource);
    }

    // Create an HTMLResource for the INDEX if it doesn't exist.
    if (index_resource == NULL) {
        index_resource = &m_Book->CreateEmptyHTMLFile();
        index_resource->RenameTo(HTML_INDEX_FILE);
        html_resources.append(index_resource);
        m_Book->GetOPF().UpdateSpineOrder(html_resources);
    }

    // Skip indexing the index page itself
    html_resources.removeOne(index_resource);

    // Scan the book, add ids for any tag containing at least one index entry and store the
    // document index entry at the same time (including custom and from the index editor).
    if (!Index::BuildIndex(html_resources)) {
        return;
    }

    // Write out the HTML index file.
    IndexHTMLWriter index;
    index_resource->SetText(index.WriteXML());

    // Setting a semantic on a resource that already has it set will remove the semantic.
    if (m_Book->GetOPF().GetGuideSemanticTypeForResource(*index_resource) != GuideSemantics::Index) {
        m_Book->GetOPF().AddGuideSemanticType(*index_resource, GuideSemantics::Index);
    }

    m_Book->SetModified();
    m_BookBrowser->Refresh();
    OpenResource(*index_resource);

    QApplication::restoreOverrideCursor();
}


void MainWindow::ReportsDialog()
{
    QList<Resource *> html_resources = m_BookBrowser->AllHTMLResources();
    QList<Resource *> image_resources = m_BookBrowser->AllImageResources();
    QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();

    Reports reports(html_resources, image_resources, css_resources, m_Book, this);

    if (reports.exec() == QDialog::Accepted) {
        OpenFilename(reports.SelectedFile());
    }
}

void MainWindow::InsertImage()
{
    m_TabManager.SaveTabData();

    QStringList selected_images;
    QList<Resource *> image_resources = m_BookBrowser->AllImageResources();

    if (image_resources.isEmpty()) {
        QMessageBox::warning( this,
                              tr( "Sigil"),
                              tr( "<p>There are no images available in your book to insert.</p><p>Use the menu option <b>File->New->Add Existing</b> to add images to your book before trying to insert an image.</p>")
                            );
        return;
    }

    QString basepath = m_Book->GetFolderKeeper().GetFullPathToImageFolder();
    if (!basepath.endsWith("/")) {
        basepath.append("/");
    }
    SelectImages select_images(basepath, image_resources, m_LastInsertedImage, this);

    if (select_images.exec() == QDialog::Accepted) {
        selected_images = select_images.SelectedImages();
    }

    InsertImages(selected_images);
}

void MainWindow::InsertImages(QStringList selected_images)
{
    FlowTab *flow_tab = qobject_cast<FlowTab*>(&GetCurrentContentTab());

    if (!(flow_tab && (m_ViewState == MainWindow::ViewState_CodeView || m_ViewState == MainWindow::ViewState_BookView))) {
        Utility::DisplayStdErrorDialog(tr("You cannot insert an image into the current tab.")); 
        return;
    }

    foreach (QString selected_image, selected_images) {
        try
        {
            const Resource &resource = m_Book->GetFolderKeeper().GetResourceByFilename(selected_image);
            const QString &relative_path = "../" + resource.GetRelativePathToOEBPS();
            flow_tab->InsertImage(relative_path);
        }
        catch (const ResourceDoesNotExist&)
        {
            Utility::DisplayStdErrorDialog(tr("The file \"%1\" does not exist.") .arg(selected_image));
        }
    }

    if (!selected_images.isEmpty()) {
        m_LastInsertedImage = selected_images.last();
    }
}

void MainWindow::InsertSpecialCharacter()
{
   if ( !m_TabManager.TabDataIsWellFormed() ) {
        return;
    }

    m_TabManager.SaveTabData();

    // non-modal dialog
    m_SelectCharacter->show();
    m_SelectCharacter->raise();
    m_SelectCharacter->activateWindow();
}

void MainWindow::InsertId()
{
    m_TabManager.SaveTabData();
   
    // Get current id attribute value if any
    ContentTab &tab = GetCurrentContentTab();
    FlowTab *flow_tab = qobject_cast<FlowTab*>(&tab);

    if (!(flow_tab->InsertIdEnabled())) {
        Utility::DisplayStdErrorDialog(tr("Unable to insert an id."));
        return;
    }

    QString id = flow_tab->GetAttributeId();
    HTMLResource *html_resource = qobject_cast< HTMLResource* >( &tab.GetLoadedResource() );

    SelectId select_id(id, html_resource, m_Book, this);

    if (select_id.exec() == QDialog::Accepted) {
        flow_tab->InsertId(select_id.SelectedText());
    }
}

void MainWindow::InsertHyperlink()
{
    m_TabManager.SaveTabData();
   
    // Get current id attribute value if any
    ContentTab &tab = GetCurrentContentTab();
    FlowTab *flow_tab = qobject_cast<FlowTab*>(&tab);

    if (!flow_tab->InsertHyperlinkEnabled()) {
        Utility::DisplayStdErrorDialog(tr("Unable to insert a hyperlink."));
        return;
    }

    QString href = flow_tab->GetAttributeHref();
    HTMLResource *html_resource = qobject_cast< HTMLResource* >( &tab.GetLoadedResource() );
    QList<Resource *> resources = m_BookBrowser->AllHTMLResources();

    SelectHyperlink select_hyperlink(href, html_resource, resources, m_Book, this);

    if (select_hyperlink.exec() == QDialog::Accepted) {
        flow_tab->InsertHyperlink(select_hyperlink.GetText());
    }
}

void MainWindow::SetViewState(MainWindow::ViewState view_state)
{
    if (view_state == MainWindow::ViewState_Unknown) {
        view_state = ViewState_BookView;
    }

    MainWindow::ViewState old_view_state = m_ViewState;
    bool set_tab_state = m_ViewState != view_state;
    m_ViewState = view_state;
    if (!UpdateViewState(set_tab_state)) {
        m_ViewState = old_view_state;
        ui.actionBookView->setChecked(false);
        ui.actionSplitView->setChecked(false);
        // Only CV in a Flow Tab would fail to allow the view to be changed due to
        // the well formed check failing. Due to this we know that we're still in CV.
        ui.actionCodeView->setChecked(true);
    }
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

    // Check if data is well formed before saving
    foreach (Resource *resource, resources) { if (!m_TabManager.TabDataIsWellFormed(*resource)) {
            Utility::DisplayStdErrorDialog(tr("Merge aborted.\n\nOne of the files may have an error or has not been saved.\n\nTry saving your book or correcting any errors before merging."));
            return;
        }
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Save the tab data then save it to disk, and recheck if data is still well formed
    // Merging work based off of the data on disk.
    SaveTabData();
    m_Book->SaveAllResourcesToDisk();

    foreach (Resource *resource, resources) {
        if (!m_TabManager.TabDataIsWellFormed(*resource)) {
            QMessageBox::critical(this, tr("Sigil"), tr("Cannot merge: %1 data is not well formed.").arg(resource->Filename()));
            return;
        }
    }

    // Close all tabs being updated to prevent BV overwriting the new data
    foreach (Resource *resource, resources) {
        if (!m_TabManager.CloseTabForResource(*resource)) {
            QMessageBox::critical(this, tr("Sigil"), tr("Cannot merge\n\nCannot close tab: %1").arg(resource->Filename()));
            return;
        }
    }

    Resource *resource1 = resources.takeFirst();
    HTMLResource &html_resource1 = *qobject_cast<HTMLResource *>(resource1);

    // Display progress dialog
    QProgressDialog progress(QObject::tr( "Merging Files.." ), QString(), 0, resources.count(), this);
    progress.setMinimumDuration(PROGRESS_BAR_MINIMUM_DURATION);
    int progress_value = 0;

    foreach (Resource *resource, resources) {
        // Set progress value and ensure dialog has time to display when doing extensive updates
        progress.setValue(progress_value++);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        if (resource) {
            HTMLResource &html_resource2 = *qobject_cast<HTMLResource *>(resource);
            if (!m_Book->Merge(html_resource1, html_resource2)) {
                QMessageBox::critical(this, tr("Sigil"), tr("Cannot merge file %1").arg(resource->Filename()));
                break;
            }
        }
    }

    m_BookBrowser->Refresh();

    OpenResource(*resource1);
    UpdateBrowserSelectionToTab();

    QApplication::restoreOverrideCursor();
}

void MainWindow::LinkStylesheetsToResources(QList <Resource *> resources)
{
    if (resources.isEmpty()) {
        return;
    }

    // Check if data is well formed before saving
    foreach (Resource *resource, resources) {
        if (!m_TabManager.TabDataIsWellFormed(*resource)) {
            Utility::DisplayStdErrorDialog(tr("Link aborted.") % "\n\n" % tr("One of the files may have an error or has not been saved.") % "\n\n" % tr("Try saving your book or correcting any errors before linking stylesheets."));
            return;
        }
    }

    // Save the tab data and recheck if data is still well formed
    SaveTabData();
    foreach (Resource *resource, resources) {
        if (!m_TabManager.TabDataIsWellFormed(*resource)) {
            QMessageBox::critical(this, tr("Sigil"), tr("Cannot link stylesheets: %1 data is not well formed.").arg(resource->Filename()));
            return;
        }
    }

    // Choose which stylesheets to link
    LinkStylesheets link( GetStylesheetsMap( resources ), this );

    if ( link.exec() != QDialog::Accepted )
    {
        return;
    }

    Resource *current_resource = NULL;
    ContentTab &tab = m_TabManager.GetCurrentContentTab();
    if ( &tab != NULL )
    {
        current_resource = &tab.GetLoadedResource();
    }

    // Close all tabs being updated to prevent BV overwriting the new data
    foreach (Resource *resource, resources) {
        if (!m_TabManager.CloseTabForResource(*resource)) {
            QMessageBox::critical(this, tr("Sigil"), tr("Cannot link stylesheets\n\nCannot close tab: %1").arg(resource->Filename()));
            return;
        }
    }

    QStringList stylesheets = link.GetStylesheets();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Convert HTML resources into HTMLResource types
    QList<HTMLResource *>html_resources;
    foreach( Resource *resource, resources )
    {
            html_resources.append( qobject_cast<HTMLResource*>(resource));
    }

    LinkUpdates::UpdateLinksInAllFiles( html_resources, stylesheets );
    m_Book->SetModified();

    if (current_resource && resources.contains(current_resource)) {
        OpenResource(*current_resource);
    }
    SelectResources(resources);

    QApplication::restoreOverrideCursor();
}

QList<std::pair< QString, bool> > MainWindow::GetStylesheetsMap( QList<Resource *> resources )
{
    QList< std::pair< QString, bool> > stylesheet_map;
    QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();

    // Use the first resource to get a list of known linked stylesheets in order.
    QStringList checked_linked_paths = GetStylesheetsAlreadyLinked( resources.at( 0 ) );

    // Then only consider them included if every selected resource includes
    // the same stylesheets in the same order.
    foreach ( Resource *valid_resource, resources )
    {
        QStringList linked_paths = GetStylesheetsAlreadyLinked( valid_resource );

        foreach ( QString path, checked_linked_paths )
        {
            if ( !linked_paths.contains( path ) )
            {
                checked_linked_paths.removeOne( path );
            }
        }
    }

    // Save the paths included in all resources in order
    foreach ( QString path, checked_linked_paths )
    {
        stylesheet_map.append( std::make_pair( path, true ) );
    }
    // Save all the remaining paths and mark them not included
    foreach ( Resource *resource, css_resources )
    {
        QString pathname = "../" + resource->GetRelativePathToOEBPS();
        if ( !checked_linked_paths.contains( pathname ) )
        {
            stylesheet_map.append( std::make_pair( pathname, false ) );
        }
    }

    return stylesheet_map;
}


QStringList MainWindow::GetStylesheetsAlreadyLinked( Resource *resource )
{
    HTMLResource *html_resource = qobject_cast< HTMLResource* >( resource );
    QStringList linked_stylesheets;

    QStringList existing_stylesheets;
    foreach (Resource *css_resource, m_BookBrowser->AllCSSResources() )
    {
        //existing_stylesheets.append( css_resource->Filename() );
        existing_stylesheets.append( "../" + css_resource->GetRelativePathToOEBPS() );
    }

    foreach( QString pathname, html_resource->GetLinkedStylesheets() )
    {
        // Only list the stylesheet if it exists in the book
        if ( existing_stylesheets.contains( pathname ) )
        {
            linked_stylesheets.append( pathname );
        }
    }

    return linked_stylesheets;
}

void MainWindow::RemoveResources()
{
    // Provide the open tab list to ensure one tab stays open
    m_BookBrowser->RemoveSelection(m_TabManager.GetTabResources());
}

void MainWindow::GenerateToc()
{
    SaveTabData();
    m_Book->SaveAllResourcesToDisk();

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
    

void MainWindow::CreateHTMLTOC()
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
            if (m_Book->GetOPF().GetGuideSemanticTypeForResource(*htmlResource) == GuideSemantics::TableOfContents) {
                tocResource = htmlResource;
            } else if (resource->Filename() == HTML_TOC_FILE && tocResource == NULL) {
                tocResource = htmlResource;
            }
        }
    }

    // Close the tab so the focus saving doesn't overwrite the text were
    // replacing in the resource.
    if (tocResource != NULL) {
        m_TabManager.CloseTabForResource(*tocResource);
    }

    // Create the an HTMLResource for the TOC if it doesn't exit.
    if (tocResource == NULL) {
        tocResource = &m_Book->CreateEmptyHTMLFile();
        tocResource->RenameTo(HTML_TOC_FILE);
        htmlResources.insert(0, tocResource);
        m_Book->GetOPF().UpdateSpineOrder(htmlResources);
    }

    TOCHTMLWriter toc(m_TableOfContents->GetRootEntry());
    tocResource->SetText(toc.WriteXML());

    // Setting a semantic on a resource that already has it set will remove the semantic.
    if (m_Book->GetOPF().GetGuideSemanticTypeForResource(*tocResource) != GuideSemantics::TableOfContents) {
        m_Book->GetOPF().AddGuideSemanticType(*tocResource, GuideSemantics::TableOfContents);
    }

    m_Book->SetModified();
    m_BookBrowser->Refresh();
    OpenResource(*tocResource);

    QApplication::restoreOverrideCursor();
}

void MainWindow::setCleanLevel(int level, bool store, bool show_warning)
{
    SettingsStore settings;
    SettingsStore::CleanLevel clean_level = SettingsStore::CleanLevel_Off;

    ui.actionCleanLevelOff->setChecked(false);
    ui.actionCleanLevelPrettyPrint->setChecked(false);
    ui.actionCleanLevelTidy->setChecked(false);

    switch (level) {
        case SettingsStore::CleanLevel_PrettyPrint:
            ui.actionCleanLevelPrettyPrint->setChecked(true);
            clean_level = SettingsStore::CleanLevel_PrettyPrint;
            break;
        case SettingsStore::CleanLevel_Tidy:
            ui.actionCleanLevelTidy->setChecked(true);
            clean_level = SettingsStore::CleanLevel_Tidy;
            break;
        default:
            ui.actionCleanLevelOff->setChecked(true);
            clean_level = SettingsStore::CleanLevel_Off;
            break;
    }

    if (store) {
        settings.setCleanLevel(clean_level);
    }

    if (show_warning) {
        QMessageBox::critical(this, tr("Sigil"), tr("You must restart Sigil for any change to the Clean Source setting to be used."));
    }
}

void MainWindow::ChangeCasing(int casing_mode)
{
    ContentTab &tab = GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }

    Utility::Casing casing;
    switch (casing_mode) {
        case Utility::Casing_Lowercase: {
            casing = Utility::Casing_Lowercase;
            break;
        }
        case Utility::Casing_Uppercase: {
            casing = Utility::Casing_Uppercase;
            break;
        }
        case Utility::Casing_Propercase: {
            casing = Utility::Casing_Propercase;
            break;
        }
        case Utility::Casing_Capitalize: {
            casing = Utility::Casing_Capitalize;
            break;
        }
        default:
            return;
    }
    tab.ChangeCasing( casing );
}

void MainWindow::ToggleViewState()
{
    ContentTab &tab = GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }

    Resource::ResourceType type = tab.GetLoadedResource().Type();

    if (type == Resource::HTMLResourceType) {
        if (m_ViewState == MainWindow::ViewState_CodeView) {
            SetViewState(MainWindow::ViewState_BookView);
        }
        else {
            SetViewState(MainWindow::ViewState_CodeView);
        }
    }
}

void MainWindow::BookView()
{
    SetViewState( MainWindow::ViewState_BookView );
}


void MainWindow::SplitView()
{
    SetViewState( MainWindow::ViewState_PreviewView );
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

void MainWindow::SearchEditorDialog(SearchEditorModel::searchEntry* search_entry)
{
    if ( !m_TabManager.TabDataIsWellFormed() ) {
        return;
    }

    m_TabManager.SaveTabData();

    // non-modal dialog
    m_SearchEditor->show();
    m_SearchEditor->raise();
    m_SearchEditor->activateWindow();

    if (search_entry) {
        m_SearchEditor->AddEntry(search_entry->is_group, search_entry, false);
    }
}

void MainWindow::ClipEditorDialog(ClipEditorModel::clipEntry* clip_entry)
{
    if ( !m_TabManager.TabDataIsWellFormed() ) {
        return;
    }

    m_TabManager.SaveTabData();

    // non-modal dialog
    m_ClipEditor->show();
    m_ClipEditor->raise();
    m_ClipEditor->activateWindow();

    if (clip_entry) {
        m_ClipEditor->AddEntry(clip_entry->is_group, clip_entry, false);
    }
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


void MainWindow::UserGuide()
{
    QDesktopServices::openUrl( QUrl( USER_GUIDE_URL ) );
}


void MainWindow::FrequentlyAskedQuestions()
{
    QDesktopServices::openUrl( QUrl( FAQ_URL ) );
}


void MainWindow::Tutorials()
{
    QDesktopServices::openUrl( QUrl( TUTORIALS_URL ) );
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

    if ( preferences.isReloadTabsRequired() ) 
    {
        m_TabManager.ReopenTabs();
    }
    else if ( preferences.isRefreshSpellingHighlightingRequired() )
    {
        RefreshSpellingHighlighting();
    }
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


bool MainWindow::UpdateViewState(bool set_tab_state)
{
    ContentTab &tab = GetCurrentContentTab();
    if (&tab == NULL) {
        return false;
    }
    Resource::ResourceType type = tab.GetLoadedResource().Type();

    if (type == Resource::HTMLResourceType) {
        if (set_tab_state) {
            FlowTab *ftab = dynamic_cast<FlowTab *>(&tab);
            if (ftab) {
                bool view_state_changed = ftab->SetViewState(m_ViewState);
                // We cannot reliably use Qt focus events to determine whether or
                // not to reload the contents of a tab. 
                ftab->ReloadTabIfPending();
                if (!view_state_changed) {
                    return false;
                }
            }
        }

        if (m_ViewState == MainWindow::ViewState_CodeView) {
            SetStateActionsCodeView();
        }
        else if (m_ViewState == MainWindow::ViewState_PreviewView) {
            SetStateActionsSplitView();
        }
        else {
            if (m_ViewState != MainWindow::ViewState_BookView) {
                m_ViewState = MainWindow::ViewState_BookView;
            }
            SetStateActionsBookView();
        }
    }
    else if (type == Resource::CSSResourceType)
    {
        SetStateActionsCSSView();
    }
    else if (type == Resource::XMLResourceType ||
             type == Resource::OPFResourceType ||
             type == Resource::NCXResourceType ||
             type == Resource::SVGResourceType ||
             type == Resource::TextResourceType)
    {
        SetStateActionsRawView();
    }
    else {
        SetStateActionsStaticView();
    }

    return true;
}


void MainWindow::UpdateUIOnTabChanges()
{
    ContentTab &tab = m_TabManager.GetCurrentContentTab();

    if ( &tab == NULL )

        return;

    ui.actionBookView               ->setEnabled( tab.ViewStatesEnabled() );
    ui.actionSplitView              ->setEnabled( tab.ViewStatesEnabled() );
    ui.actionCodeView               ->setEnabled( tab.ViewStatesEnabled() );

    ui.actionPrintPreview->setEnabled( tab.PrintEnabled() );
    ui.actionPrint->setEnabled( tab.PrintEnabled() );

    ui.actionCut                    ->setEnabled( tab.CutEnabled() );
    ui.actionCopy                   ->setEnabled( tab.CopyEnabled() );
    ui.actionPaste                  ->setEnabled( tab.PasteEnabled() );
    ui.actionInsertClosingTag       ->setEnabled( tab.InsertClosingTagEnabled() );
    ui.actionAddToIndex             ->setEnabled( tab.AddToIndexEnabled() );
    ui.actionMarkForIndex           ->setEnabled( tab.MarkForIndexEnabled() );
    ui.actionInsertId               ->setEnabled( tab.InsertIdEnabled() );
    ui.actionInsertHyperlink        ->setEnabled( tab.InsertHyperlinkEnabled() );
    ui.actionInsertSpecialCharacter ->setEnabled( tab.InsertSpecialCharacterEnabled() );
    ui.actionAutoSpellCheck         ->setEnabled( tab.ToggleAutoSpellcheckEnabled() );

    ui.actionBold           ->setChecked( tab.BoldChecked() );
    ui.actionItalic         ->setChecked( tab.ItalicChecked() );
    ui.actionUnderline      ->setChecked( tab.UnderlineChecked() );
    ui.actionStrikethrough  ->setChecked( tab.StrikethroughChecked() );
    ui.actionSubscript      ->setChecked( tab.SubscriptChecked() );
    ui.actionSuperscript    ->setChecked( tab.SuperscriptChecked() );

    ui.actionAlignLeft      ->setChecked( tab.AlignLeftChecked() );
    ui.actionAlignRight     ->setChecked( tab.AlignRightChecked() );
    ui.actionAlignCenter    ->setChecked( tab.AlignCenterChecked() );
    ui.actionAlignJustify   ->setChecked( tab.AlignJustifyChecked() );

    ui.actionInsertBulletedList ->setChecked( tab.BulletListChecked() );
    ui.actionInsertNumberedList ->setChecked( tab.NumberListChecked() );
    ui.actionRemoveFormatting   ->setEnabled( tab.RemoveFormattingEnabled() );
    ui.actionGoToLinkOrStyle    ->setEnabled( tab.GoToLinkOrStyleEnabled() );

    // State of zoom controls depends on current tab/view
    float zoom_factor = tab.GetZoomFactor();
    UpdateZoomLabel( zoom_factor );
    UpdateZoomSlider( zoom_factor );

    UpdateCursorPositionLabel(tab.GetCursorLine(), tab.GetCursorColumn());
    SelectEntryOnHeadingToolbar( tab.GetCaretElementName() );    
}


void MainWindow::UpdateUiWhenTabsSwitch()
{
    ContentTab &tab = GetCurrentContentTab();
    if (&tab == NULL) {
        return;
    }

    UpdateViewState();
}


void MainWindow::UpdateUIOnTabCountChange()
{
    ui.actionNextTab       ->setEnabled(m_TabManager.GetTabCount() > 1);
    ui.actionPreviousTab   ->setEnabled(m_TabManager.GetTabCount() > 1);
    ui.actionCloseTab      ->setEnabled(m_TabManager.GetTabCount() > 1);
    ui.actionCloseOtherTabs->setEnabled(m_TabManager.GetTabCount() > 1);
}


void MainWindow::SetStateActionsBookView()
{
    ui.actionBookView->setChecked(true);
    ui.actionSplitView->setChecked(false);
    ui.actionCodeView->setChecked(false);

    ui.actionUndo->setEnabled(true);
    ui.actionRedo->setEnabled(true);
	
    ui.actionCut  ->setEnabled(true);  
    ui.actionCopy ->setEnabled(true); 
    ui.actionPaste->setEnabled(true); 
    ui.actionPasteClipboardHistory->setEnabled(true);

    ui.actionBold         ->setEnabled(true);
    ui.actionItalic       ->setEnabled(true);
    ui.actionUnderline    ->setEnabled(true);
    ui.actionStrikethrough->setEnabled(true);
    ui.actionSubscript    ->setEnabled(true);
    ui.actionSuperscript  ->setEnabled(true);

    ui.actionAlignLeft   ->setEnabled(true);
    ui.actionAlignCenter ->setEnabled(true);
    ui.actionAlignRight  ->setEnabled(true);
    ui.actionAlignJustify->setEnabled(true);
	
	ui.actionDecreaseIndent->setEnabled(true);
    ui.actionIncreaseIndent->setEnabled(true);
    	
    ui.actionTextDirectionLTR    ->setEnabled(true);
    ui.actionTextDirectionRTL    ->setEnabled(true);
    ui.actionTextDirectionDefault->setEnabled(true);

    ui.actionInsertBulletedList->setEnabled(true);
    ui.actionInsertNumberedList->setEnabled(true);
    ui.actionRemoveFormatting->setEnabled(true);

    ui.menuHeadings->setEnabled(true);
    ui.actionHeading1->setEnabled(true);
    ui.actionHeading2->setEnabled(true);
    ui.actionHeading3->setEnabled(true);
    ui.actionHeading4->setEnabled(true);
    ui.actionHeading5->setEnabled(true);
    ui.actionHeading6->setEnabled(true);
    ui.actionHeadingNormal->setEnabled(true);

    ui.actionCasingLowercase  ->setEnabled(true);
    ui.actionCasingUppercase  ->setEnabled(true);
    ui.actionCasingPropercase ->setEnabled(true);
    ui.actionCasingCapitalize ->setEnabled(true);

    ui.actionInsertImage->setEnabled(true);
    ui.actionSplitChapter->setEnabled(true);
    ui.actionInsertClosingTag->setEnabled(false);
    ui.actionInsertSGFChapterMarker->setEnabled(true);

    ui.actionFind->setEnabled(true);
    ui.actionFindNext->setEnabled(true);
    ui.actionFindPrevious->setEnabled(true);
    ui.actionReplaceNext->setEnabled(false);
    ui.actionReplacePrevious->setEnabled(false);
    ui.actionReplaceAll->setEnabled(false);
    ui.actionCount->setEnabled(false);
    ui.actionGoToLine->setEnabled(false);
    ui.actionGoToLinkOrStyle->setEnabled(false);

    UpdateUIOnTabChanges();

    m_FindReplace->ShowHide();
}

void MainWindow::SetStateActionsSplitView()
{
    ui.actionBookView->setChecked(false);
    ui.actionSplitView->setChecked(true);
    ui.actionCodeView->setChecked(false);

    ui.actionUndo->setEnabled(false);
    ui.actionRedo->setEnabled(false);
	
    ui.actionCut  ->setEnabled(false);  
    ui.actionCopy ->setEnabled(false); 
    ui.actionPaste->setEnabled(false); 
    ui.actionPasteClipboardHistory->setEnabled(false);

    ui.actionBold         ->setEnabled(false);
    ui.actionItalic       ->setEnabled(false);
    ui.actionUnderline    ->setEnabled(false);
    ui.actionStrikethrough->setEnabled(false);
    ui.actionSubscript    ->setEnabled(false);
    ui.actionSuperscript  ->setEnabled(false);

    ui.actionAlignLeft   ->setEnabled(false);
    ui.actionAlignCenter ->setEnabled(false);
    ui.actionAlignRight  ->setEnabled(false);
    ui.actionAlignJustify->setEnabled(false);
	
	ui.actionDecreaseIndent->setEnabled(false);
    ui.actionIncreaseIndent->setEnabled(false);
    	
    ui.actionTextDirectionLTR    ->setEnabled(false);
    ui.actionTextDirectionRTL    ->setEnabled(false);
    ui.actionTextDirectionDefault->setEnabled(false);

    ui.actionInsertBulletedList->setEnabled(false);
    ui.actionInsertNumberedList->setEnabled(false);
    ui.actionRemoveFormatting->setEnabled(false);

    ui.menuHeadings->setEnabled(false);
    ui.actionHeading1->setEnabled(false);
    ui.actionHeading2->setEnabled(false);
    ui.actionHeading3->setEnabled(false);
    ui.actionHeading4->setEnabled(false);
    ui.actionHeading5->setEnabled(false);
    ui.actionHeading6->setEnabled(false);
    ui.actionHeadingNormal->setEnabled(false);

    ui.actionCasingLowercase  ->setEnabled(false);
    ui.actionCasingUppercase  ->setEnabled(false);
    ui.actionCasingPropercase ->setEnabled(false);
    ui.actionCasingCapitalize ->setEnabled(false);

    ui.actionInsertImage->setEnabled(false);
    ui.actionSplitChapter->setEnabled(false);
    ui.actionInsertClosingTag->setEnabled(false);
    ui.actionInsertSGFChapterMarker->setEnabled(false);

    ui.actionFind->setEnabled(true);
    ui.actionFindNext->setEnabled(true);
    ui.actionFindPrevious->setEnabled(true);
    ui.actionReplaceNext->setEnabled(false);
    ui.actionReplacePrevious->setEnabled(false);
    ui.actionReplaceAll->setEnabled(false);
    ui.actionCount->setEnabled(false);
    ui.actionGoToLine->setEnabled(false);
    ui.actionGoToLinkOrStyle->setEnabled(false);

    UpdateUIOnTabChanges();

    m_FindReplace->ShowHide();
}

void MainWindow::SetStateActionsCodeView()
{
    ui.actionBookView->setChecked(false);
    ui.actionSplitView->setChecked(false);
    ui.actionCodeView->setChecked(true);

    ui.actionUndo->setEnabled(true);
    ui.actionRedo->setEnabled(true);
	
    ui.actionCut  ->setEnabled(true);  
    ui.actionCopy ->setEnabled(true); 
    ui.actionPaste->setEnabled(true); 
    ui.actionPasteClipboardHistory->setEnabled(true);

    ui.actionBold         ->setEnabled(true);
    ui.actionItalic       ->setEnabled(true);
    ui.actionUnderline    ->setEnabled(true);
    ui.actionStrikethrough->setEnabled(true);
    ui.actionSubscript    ->setEnabled(true);
    ui.actionSuperscript  ->setEnabled(true);

    ui.actionAlignLeft   ->setEnabled(true);
    ui.actionAlignCenter ->setEnabled(true);
    ui.actionAlignRight  ->setEnabled(true);
    ui.actionAlignJustify->setEnabled(true);
	
	ui.actionDecreaseIndent->setEnabled(false);
    ui.actionIncreaseIndent->setEnabled(false);
    	
    ui.actionTextDirectionLTR    ->setEnabled(true);
    ui.actionTextDirectionRTL    ->setEnabled(true);
    ui.actionTextDirectionDefault->setEnabled(true);

    ui.actionInsertBulletedList->setEnabled(false);
    ui.actionInsertNumberedList->setEnabled(false);
    ui.actionRemoveFormatting->setEnabled(true);

    ui.menuHeadings->setEnabled(true);
    ui.actionHeading1->setEnabled(true);
    ui.actionHeading2->setEnabled(true);
    ui.actionHeading3->setEnabled(true);
    ui.actionHeading4->setEnabled(true);
    ui.actionHeading5->setEnabled(true);
    ui.actionHeading6->setEnabled(true);
    ui.actionHeadingNormal->setEnabled(true);

    ui.actionCasingLowercase  ->setEnabled(true);
    ui.actionCasingUppercase  ->setEnabled(true);
    ui.actionCasingPropercase ->setEnabled(true);
    ui.actionCasingCapitalize ->setEnabled(true);

    ui.actionInsertImage->setEnabled(true);
    ui.actionSplitChapter->setEnabled(true);
    ui.actionInsertSGFChapterMarker->setEnabled(true);
    ui.actionInsertClosingTag->setEnabled(true);

    ui.actionFind->setEnabled(true);
    ui.actionFindNext->setEnabled(true);
    ui.actionFindPrevious->setEnabled(true);
    ui.actionReplaceNext->setEnabled(true);
    ui.actionReplacePrevious->setEnabled(true);
    ui.actionReplaceAll->setEnabled(true);
    ui.actionCount->setEnabled(true);
    ui.actionGoToLine->setEnabled(true);
    ui.actionGoToLinkOrStyle->setEnabled(true);

    UpdateUIOnTabChanges();

    m_FindReplace->ShowHide();
}

void MainWindow::SetStateActionsCSSView()
{
    SetStateActionsRawView();

    ui.actionBold         ->setEnabled(true);
    ui.actionItalic       ->setEnabled(true);
    ui.actionUnderline    ->setEnabled(true);
    ui.actionStrikethrough->setEnabled(true);

    ui.actionAlignLeft   ->setEnabled(true);
    ui.actionAlignCenter ->setEnabled(true);
    ui.actionAlignRight  ->setEnabled(true);
    ui.actionAlignJustify->setEnabled(true);
    	
    ui.actionTextDirectionLTR    ->setEnabled(true);
    ui.actionTextDirectionRTL    ->setEnabled(true);
    ui.actionTextDirectionDefault->setEnabled(true);
}

void MainWindow::SetStateActionsRawView()
{
    ui.actionBookView->setChecked(false);
    ui.actionSplitView->setChecked(false);
    ui.actionCodeView->setChecked(false);

    ui.actionUndo->setEnabled(true);
    ui.actionRedo->setEnabled(true);
	
    ui.actionCut  ->setEnabled(false);  
    ui.actionCopy ->setEnabled(false); 
    ui.actionPaste->setEnabled(false); 
    ui.actionPasteClipboardHistory->setEnabled(true);

    ui.actionBold         ->setEnabled(false);
    ui.actionItalic       ->setEnabled(false);
    ui.actionUnderline    ->setEnabled(false);
    ui.actionStrikethrough->setEnabled(false);
    ui.actionSubscript    ->setEnabled(false);
    ui.actionSuperscript  ->setEnabled(false);

    ui.actionAlignLeft   ->setEnabled(false);
    ui.actionAlignCenter ->setEnabled(false);
    ui.actionAlignRight  ->setEnabled(false);
    ui.actionAlignJustify->setEnabled(false);
	
	ui.actionDecreaseIndent->setEnabled(false);
    ui.actionIncreaseIndent->setEnabled(false);
	
    ui.actionTextDirectionLTR    ->setEnabled(false);
    ui.actionTextDirectionRTL    ->setEnabled(false);
    ui.actionTextDirectionDefault->setEnabled(false);

    ui.actionInsertBulletedList->setEnabled(false);
    ui.actionInsertNumberedList->setEnabled(false);
    ui.actionRemoveFormatting->setEnabled(false);

    ui.menuHeadings->setEnabled(false);
    ui.actionHeading1->setEnabled(false);
    ui.actionHeading2->setEnabled(false);
    ui.actionHeading3->setEnabled(false);
    ui.actionHeading4->setEnabled(false);
    ui.actionHeading5->setEnabled(false);
    ui.actionHeading6->setEnabled(false);
    ui.actionHeadingNormal->setEnabled(false);

    ui.actionCasingLowercase  ->setEnabled(true);
    ui.actionCasingUppercase  ->setEnabled(true);
    ui.actionCasingPropercase ->setEnabled(true);
    ui.actionCasingCapitalize ->setEnabled(true);

    ui.actionInsertImage->setEnabled(false);
    ui.actionSplitChapter->setEnabled(false);
    ui.actionInsertSGFChapterMarker->setEnabled(false);

    ui.actionFind->setEnabled(true);
    ui.actionFindNext->setEnabled(true);
    ui.actionFindPrevious->setEnabled(true);
    ui.actionReplaceNext->setEnabled(true);
    ui.actionReplacePrevious->setEnabled(true);
    ui.actionReplaceAll->setEnabled(true);
    ui.actionCount->setEnabled(true);
    ui.actionGoToLine->setEnabled(true);
    ui.actionGoToLinkOrStyle->setEnabled(false);

    UpdateUIOnTabChanges();

    m_FindReplace->ShowHide();
}

void MainWindow::SetStateActionsStaticView()
{
    ui.actionBookView->setChecked(false);
    ui.actionSplitView->setChecked(false);
    ui.actionCodeView->setChecked(false);

    ui.actionUndo->setEnabled(false);
    ui.actionRedo->setEnabled(false);
	
    ui.actionCut  ->setEnabled(false);  
    ui.actionCopy ->setEnabled(false); 
    ui.actionPaste->setEnabled(false); 
    ui.actionPasteClipboardHistory->setEnabled(false);

    ui.actionBold         ->setEnabled(false);
    ui.actionItalic       ->setEnabled(false);
    ui.actionUnderline    ->setEnabled(false);
    ui.actionStrikethrough->setEnabled(false);
    ui.actionSubscript    ->setEnabled(false);
    ui.actionSuperscript  ->setEnabled(false);

    ui.actionAlignLeft   ->setEnabled(false);
    ui.actionAlignCenter ->setEnabled(false);
    ui.actionAlignRight  ->setEnabled(false);
    ui.actionAlignJustify->setEnabled(false);
	
	ui.actionDecreaseIndent->setEnabled(false);
    ui.actionIncreaseIndent->setEnabled(false);
	
    ui.actionTextDirectionLTR    ->setEnabled(false);
    ui.actionTextDirectionRTL    ->setEnabled(false);
    ui.actionTextDirectionDefault->setEnabled(false);

    ui.actionInsertBulletedList->setEnabled(false);
    ui.actionInsertNumberedList->setEnabled(false);
    ui.actionRemoveFormatting->setEnabled(false);

    ui.menuHeadings->setEnabled(false);
    ui.actionHeading1->setEnabled(false);
    ui.actionHeading2->setEnabled(false);
    ui.actionHeading3->setEnabled(false);
    ui.actionHeading4->setEnabled(false);
    ui.actionHeading5->setEnabled(false);
    ui.actionHeading6->setEnabled(false);
    ui.actionHeadingNormal->setEnabled(false);

    ui.actionCasingLowercase  ->setEnabled(false);
    ui.actionCasingUppercase  ->setEnabled(false);
    ui.actionCasingPropercase ->setEnabled(false);
    ui.actionCasingCapitalize ->setEnabled(false);

    ui.actionInsertImage->setEnabled(false);
    ui.actionSplitChapter->setEnabled(false);
    ui.actionInsertSGFChapterMarker->setEnabled(false);

    ui.actionFind->setEnabled(false);
    ui.actionFindNext->setEnabled(false);
    ui.actionFindPrevious->setEnabled(false);
    ui.actionReplaceNext->setEnabled(false);
    ui.actionReplacePrevious->setEnabled(false);
    ui.actionReplaceAll->setEnabled(false);
    ui.actionCount->setEnabled(false);
    ui.actionGoToLine->setEnabled(false);
    ui.actionGoToLinkOrStyle->setEnabled(false);

    UpdateUIOnTabChanges();

    // Only hide window, don't save closed state since its temporary
    m_FindReplace->hide();
}


void MainWindow::UpdateCursorPositionLabel(int line, int column)
{
    if (line > 0 && column > 0) {
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


void MainWindow::SetCheckWellFormedErrors( bool new_state )
{
    m_CheckWellFormedErrors = new_state;
    m_TabManager.SetCheckWellFormedErrors( new_state );
}


void MainWindow::SetAutoSpellCheck( bool new_state )
{
    SettingsStore settings;
    settings.setSpellCheck( new_state );
    emit SettingsChanged();
}

void MainWindow::ClearIgnoredWords()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    SpellCheck *sc = SpellCheck::instance();
    sc->reloadDictionary();

    // Need to reload any tabs to force spelling to be run again in CodeView
    RefreshSpellingHighlighting();

    QApplication::restoreOverrideCursor();
}

void MainWindow::RefreshSpellingHighlighting()
{
    QList<ContentTab*> content_tabs = m_TabManager.GetContentTabs();
    foreach(ContentTab *content_tab, content_tabs) {
        FlowTab *flow_tab = qobject_cast<FlowTab *>(content_tab);
        if (flow_tab) {
            flow_tab->RefreshSpellingHighlighting();
        }
    }
}

void MainWindow::UpdateZoomLabel( float new_zoom_factor )
{
    m_lbZoomLabel->setText( QString( "%1% " ).arg( qRound( new_zoom_factor * 100 ) ) );
}

void MainWindow::CreateChapterBreakOldTab( QString content, HTMLResource& originating_resource )
{
    if (content.isNull() || content.isEmpty()) {
        statusBar()->showMessage( tr( "Chapter cannot be split at this position." ), STATUSBAR_MSG_DISPLAY_TIME );
        return;
    }
    HTMLResource& html_resource = m_Book->CreateChapterBreakOriginalResource( content, originating_resource );

    m_BookBrowser->Refresh();

    // Open the old shortened content in a new tab preceding the current one.
    // without grabbing focus
    OpenResource( html_resource, true, QUrl(), m_ViewState, -1, -1, "", false );

    FlowTab *flow_tab = qobject_cast< FlowTab* >( &GetCurrentContentTab() );
    // We want the current tab to be scrolled to the top.
    if ( flow_tab )
    {
        flow_tab->ScrollToTop();
    }

    statusBar()->showMessage( tr( "Chapter split. You may need to update the Table of Contents." ), STATUSBAR_MSG_DISPLAY_TIME );
}

void MainWindow::SplitOnSGFChapterMarkers()
{
    QList<Resource *> html_resources = m_BookBrowser->AllHTMLResources();
    
    // Check if data is well formed before saving
    foreach (Resource *resource, html_resources) { 
        if (!m_TabManager.TabDataIsWellFormed(*resource)) {
            Utility::DisplayStdErrorDialog(tr("Split aborted.\n\nOne of the files may have an error or has not been saved.\n\nTry saving your book or correcting any errors before splitting."));
            return;
        }
    }

    // If have the current tab is open in BV, make sure it has its content saved so it won't later overwrite a split.
    FlowTab *flow_tab = qobject_cast<FlowTab*>(&GetCurrentContentTab());
    if ( flow_tab && ( flow_tab->GetViewState() == MainWindow::ViewState_BookView) ) {
        flow_tab->SaveTabContent();
    }
    
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QList<Resource *> *changed_resources = new QList<Resource *>();
    foreach (Resource *resource, html_resources) { 
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        QStringList new_chapters = html_resource->SplitOnSGFChapterMarkers();
        if ( !new_chapters.isEmpty() ) {
            m_Book->CreateNewChapters( new_chapters, *html_resource );
            changed_resources->append(resource);
        }
    }

    if ( changed_resources->count() > 0 ) {
        m_TabManager.ReloadTabDataForResources( *changed_resources );
        m_BookBrowser->Refresh();

        statusBar()->showMessage( tr( "Chapters split. You may need to update the Table of Contents." ), STATUSBAR_MSG_DISPLAY_TIME );
        if ( flow_tab && ( flow_tab->GetViewState() == MainWindow::ViewState_BookView) ) {
            // Our focus will have been moved to the book browser. Set it there and back to do
            // an equivalent of "GrabFocus()" to workaround Qt setFocus() not always working.
            m_BookBrowser->setFocus();
            flow_tab->setFocus();
        }
    }
    else {
        statusBar()->showMessage( tr( "No chapter split markers found." ), STATUSBAR_MSG_DISPLAY_TIME );
    }

    QApplication::restoreOverrideCursor();
}

void MainWindow::ShowPasteClipboardHistoryDialog()
{
    // We only want to show the dialog if focus is in a content tab.
    ContentTab *contentTab = &GetCurrentContentTab();
    if ( !contentTab || !contentTab->hasFocus() ) {
        return;
    }
    m_ClipboardHistorySelector->exec();
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

    ui.actionAutoSpellCheck->setChecked(settings.spellCheck());
    emit SettingsChanged();

    SettingsStore::CleanLevel clean_level = settings.cleanLevel();

    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and its full screen status
    QByteArray geometry = settings.value( "geometry" ).toByteArray();

    if ( !geometry.isNull() )

        restoreGeometry( geometry );

    // The positions of all the toolbars and dock widgets
    QByteArray toolbars = settings.value( "toolbars" ).toByteArray();

    if ( !toolbars.isNull() )

        restoreState( toolbars );

    setCleanLevel(clean_level, false, false);

    // For the checkwellformed option, we want to default to true
    // if no value has been set.
    QVariant checkwellformederrors = settings.value( "checkwellformederrors" );
    m_CheckWellFormedErrors = checkwellformederrors.isNull() ? true : checkwellformederrors.toBool();
    ui.actionCheckWellFormedErrors->setChecked( m_CheckWellFormedErrors );
    SetCheckWellFormedErrors( m_CheckWellFormedErrors );

    // The last folder used for saving and opening files
    m_LastFolderOpen  = settings.value( "lastfolderopen"  ).toString();

    // The last filename used for save a copy
    m_SaveACopyFilename = settings.value( "saveacopyfilename" ).toString();

    // The list of recent files
    s_RecentFiles    = settings.value( "recentfiles" ).toStringList();

    m_preserveHeadingAttributes = settings.value( "preserveheadingattributes", true ).toBool();
    SetPreserveHeadingAttributes( m_preserveHeadingAttributes );

    QVariant regexOptionDotAll = settings.value( "regexoptiondotall", false );
    SetRegexOptionDotAll( regexOptionDotAll.toBool() );

    QVariant regexOptionMinimalMatch = settings.value( "regexoptionminimalmatch", false );
    SetRegexOptionMinimalMatch( regexOptionMinimalMatch.toBool() );
    
    QVariant regexOptionAutoTokenise = settings.value( "regexoptionautotokenise", false );
    SetRegexOptionAutoTokenise( regexOptionAutoTokenise.toBool() );
    
    const QStringList clipboardHistory = settings.value( "clipboardringhistory" ).toStringList();
    m_ClipboardHistorySelector->LoadClipboardHistory(clipboardHistory);

    settings.endGroup();

    // Our default fonts for book view/web preview
    SettingsStore::BookViewAppearance bookViewAppearance = settings.bookViewAppearance();
    QWebSettings *web_settings = QWebSettings::globalSettings();
    web_settings->setFontSize(QWebSettings::DefaultFontSize, bookViewAppearance.font_size);
    web_settings->setFontFamily(QWebSettings::StandardFont, bookViewAppearance.font_family_standard);
    web_settings->setFontFamily(QWebSettings::SerifFont, bookViewAppearance.font_family_serif);
    web_settings->setFontFamily(QWebSettings::SansSerifFont, bookViewAppearance.font_family_sans_serif);
}


void MainWindow::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    settings.setValue( "geometry", saveGeometry() );

    // The positions of all the toolbars and dock widgets
    settings.setValue( "toolbars", saveState() );

    // Whether the user wants to be informed about well-formed errors
    settings.setValue( "checkwellformederrors", m_CheckWellFormedErrors );

    // The last folders used for saving and opening files
    settings.setValue( "lastfolderopen",  m_LastFolderOpen  );

    // The last filename used for save a copy
    settings.setValue( "saveacopyfilename",  m_SaveACopyFilename );

    // The list of recent files
    settings.setValue( "recentfiles", s_RecentFiles );

    settings.setValue( "preserveheadingattributes", m_preserveHeadingAttributes );

    settings.setValue( "regexoptiondotall", ui.actionRegexDotAll->isChecked() );
    settings.setValue( "regexoptionminimalmatch", ui.actionRegexMinimalMatch->isChecked() );
    settings.setValue( "regexoptionautotokenise", ui.actionRegexAutoTokenise->isChecked() );

    settings.setValue( "clipboardringhistory", m_ClipboardHistorySelector->GetClipboardHistory() );

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

    m_IndexEditor->SetBook( m_Book );
    ResetLinkOrStyleBookmark();

    connect( m_Book.data(), SIGNAL( ModifiedStateChanged( bool ) ), this, SLOT( setWindowModified( bool ) ) );
    connect( m_BookBrowser,     SIGNAL( GuideSemanticTypeAdded( const HTMLResource&, GuideSemantics::GuideSemanticType ) ),
             &m_Book->GetOPF(), SLOT(   AddGuideSemanticType(   const HTMLResource&, GuideSemantics::GuideSemanticType ) ) );
    connect( m_BookBrowser,     SIGNAL( CoverImageSet(           const ImageResource& ) ),
             &m_Book->GetOPF(), SLOT(   SetResourceAsCoverImage( const ImageResource& ) ) );
    connect( m_BookBrowser,     SIGNAL( ResourcesDeleted() ), this, SLOT( ResourcesAddedOrDeleted() ) );
    connect( m_BookBrowser,     SIGNAL( ResourcesAdded() ), this, SLOT( ResourcesAddedOrDeleted() ) );
}

void MainWindow::ResourcesAddedOrDeleted()
{

    ContentTab &tab = GetCurrentContentTab();

    QWebSettings::clearMemoryCaches();

    // Make sure currently visible tab is updated immediately
    if (&tab != NULL) {
        FlowTab *flow_tab = dynamic_cast<FlowTab *>(&tab);
        if (flow_tab) {
            flow_tab->LoadTabContent();
        }
    }
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

    // Store the folder the user opened from
    m_LastFolderOpen = QFileInfo(fullfilepath).absolutePath();

    // Clear the last inserted image
    m_LastInsertedImage = "";

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


bool MainWindow::SaveFile( const QString &fullfilepath, bool update_current_filename )
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

        ExporterFactory().GetExporter( fullfilepath, m_Book ).WriteBook();

        QApplication::restoreOverrideCursor();

        // Return the focus back to the current tab
        ContentTab &tab = GetCurrentContentTab();

        if ( &tab != NULL )

            tab.setFocus();

        if ( update_current_filename ) {
            m_Book->SetModified( false );
            UpdateUiWithCurrentFile( fullfilepath );
        }
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


void MainWindow::SelectEntryOnHeadingToolbar( const QString &element_name )
{
    ui.actionHeading1->setChecked(false);
    ui.actionHeading2->setChecked(false);
    ui.actionHeading3->setChecked(false);
    ui.actionHeading4->setChecked(false);
    ui.actionHeading5->setChecked(false);
    ui.actionHeading6->setChecked(false);
    ui.actionHeadingNormal->setChecked(false);

    if ( !element_name.isEmpty() )
    {
        if ( ( element_name[ 0 ].toLower() == QChar( 'h' ) ) && ( element_name[ 1 ].isDigit() ) )
        {
            QString heading_name = QString( element_name[ 1 ] );
            if (heading_name == "1") {
                ui.actionHeading1->setChecked(true);
            }
            else if (heading_name == "2") {
                ui.actionHeading2->setChecked(true);
            }
            else if (heading_name == "3") {
                ui.actionHeading3->setChecked(true);
            }
            else if (heading_name == "4") {
                ui.actionHeading4->setChecked(true);
            }
            else if (heading_name == "5") {
                ui.actionHeading5->setChecked(true);
            }
            else if (heading_name == "6") {
                ui.actionHeading6->setChecked(true);
            }
        }
        else
            ui.actionHeadingNormal->setChecked(true);
    }
}

void MainWindow::ApplyHeadingStyleToTab( const QString &heading_type )
{
    FlowTab *flow_tab = qobject_cast<FlowTab*>(&GetCurrentContentTab());
    if (flow_tab) {
        flow_tab->HeadingStyle(heading_type, m_preserveHeadingAttributes);
    }
}

void MainWindow::SetPreserveHeadingAttributes( bool new_state )
{
    m_preserveHeadingAttributes = new_state;
    ui.actionHeadingPreserveAttributes->setChecked( m_preserveHeadingAttributes );
}


void MainWindow::CreateRecentFilesActions()
{
    for ( int i = 0; i < MAX_RECENT_FILES; ++i )
    {
        m_RecentFileActions[ i ] = new QAction( this );

        // The actions are not visible until we put a filename in them
        m_RecentFileActions[ i ]->setVisible( false );

        QList<QAction *> actlist = ui.menuFile->actions();

        // Add the new action just above the Quit action
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
    m_FindReplace->ShowHide();

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
    ui.menuToolbars->addAction(ui.toolBarFileActions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextManip->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarViews->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarInsertions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarLinks->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarHeadings->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarIndents->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarLists->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextFormats->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextAlign->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarChangeCase->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextDirection->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarDonate->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTools->toggleViewAction());

    ui.toolBarTextDirection->setVisible(false);

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
    sm->registerAction(ui.actionNewSVGFile, "MainWindow.NewSVGFile");
    sm->registerAction(ui.actionAddExistingFile, "MainWindow.AddExistingFile");
    sm->registerAction(ui.actionOpen, "MainWindow.Open");
#ifndef Q_WS_MAC
    sm->registerAction(ui.actionClose, "MainWindow.Close");
#endif
    sm->registerAction(ui.actionSave, "MainWindow.Save");
    sm->registerAction(ui.actionSaveAs, "MainWindow.SaveAs");
    sm->registerAction(ui.actionSaveACopy, "MainWindow.SaveACopy");
    sm->registerAction(ui.actionPrintPreview, "MainWindow.PrintPreview");
    sm->registerAction(ui.actionPrint, "MainWindow.Print");
    sm->registerAction(ui.actionExit, "MainWindow.Exit");
    // Edit
    sm->registerAction(ui.actionUndo, "MainWindow.Undo");
    sm->registerAction(ui.actionRedo, "MainWindow.Redo");
    sm->registerAction(ui.actionCut, "MainWindow.Cut");
    sm->registerAction(ui.actionCopy, "MainWindow.Copy");
    sm->registerAction(ui.actionPaste, "MainWindow.Paste");
    sm->registerAction(ui.actionPasteClipboardHistory, "MainWindow.PasteClipboardHistory");
    sm->registerAction(ui.actionInsertImage, "MainWindow.InsertImage");
    sm->registerAction(ui.actionInsertSpecialCharacter, "MainWindow.InsertSpecialCharacter");
    sm->registerAction(ui.actionInsertId, "MainWindow.InsertId");
    sm->registerAction(ui.actionInsertHyperlink, "MainWindow.InsertHyperlink");
    sm->registerAction(ui.actionSplitChapter, "MainWindow.SplitChapter");
    sm->registerAction(ui.actionInsertSGFChapterMarker, "MainWindow.InsertSGFChapterMarker");
    sm->registerAction(ui.actionSplitOnSGFChapterMarkers, "MainWindow.SplitOnSGFChapterMarkers");
    sm->registerAction(ui.actionInsertClosingTag, "MainWindow.InsertClosingTag");
#ifndef Q_WS_MAC
    sm->registerAction(ui.actionPreferences, "MainWindow.Preferences");
#endif
    //Search
    sm->registerAction(ui.actionFind, "MainWindow.Find");
    sm->registerAction(ui.actionFindNext, "MainWindow.FindNext");
    sm->registerAction(ui.actionFindPrevious, "MainWindow.FindPrevious");
    sm->registerAction(ui.actionReplaceNext, "MainWindow.ReplaceNext");
    sm->registerAction(ui.actionReplacePrevious, "MainWindow.ReplacePrevious");
    sm->registerAction(ui.actionReplaceAll, "MainWindow.ReplaceAll");
    sm->registerAction(ui.actionCount, "MainWindow.Count");
    sm->registerAction(ui.actionGoToLine, "MainWindow.GoToLine");
    sm->registerAction(ui.actionGoToLinkOrStyle, "MainWindow.GoToLinkOrStyle");
    sm->registerAction(ui.actionGoBackFromLinkOrStyle, "MainWindow.GoBackFromLinkOrStyle");

    // Format
    sm->registerAction(ui.actionBold, "MainWindow.Bold");
    sm->registerAction(ui.actionItalic, "MainWindow.Italic");
    sm->registerAction(ui.actionUnderline, "MainWindow.Underline");
    sm->registerAction(ui.actionStrikethrough, "MainWindow.Strikethrough");
    sm->registerAction(ui.actionSubscript, "MainWindow.Subscript");
    sm->registerAction(ui.actionSuperscript, "MainWindow.Superscript");
    sm->registerAction(ui.actionAlignLeft, "MainWindow.AlignLeft");
    sm->registerAction(ui.actionAlignCenter, "MainWindow.AlignCenter");
    sm->registerAction(ui.actionAlignRight, "MainWindow.AlignRight");
    sm->registerAction(ui.actionAlignJustify, "MainWindow.AlignJustify");
    sm->registerAction(ui.actionInsertNumberedList, "MainWindow.InsertNumberedList");
    sm->registerAction(ui.actionInsertBulletedList, "MainWindow.InsertBulletedList");
    sm->registerAction(ui.actionIncreaseIndent, "MainWindow.IncreaseIndent");
    sm->registerAction(ui.actionDecreaseIndent, "MainWindow.DecreaseIndent");
    sm->registerAction(ui.actionTextDirectionLTR, "MainWindow.TextDirectionLTR");
    sm->registerAction(ui.actionTextDirectionRTL, "MainWindow.TextDirectionRTL");
    sm->registerAction(ui.actionTextDirectionDefault, "MainWindow.TextDirectionDefault");
    sm->registerAction(ui.actionRemoveFormatting, "MainWindow.RemoveFormatting");
    sm->registerAction(ui.actionHeading1, "MainWindow.Heading1");
    sm->registerAction(ui.actionHeading2, "MainWindow.Heading2");
    sm->registerAction(ui.actionHeading3, "MainWindow.Heading3");
    sm->registerAction(ui.actionHeading4, "MainWindow.Heading4");
    sm->registerAction(ui.actionHeading5, "MainWindow.Heading5");
    sm->registerAction(ui.actionHeading6, "MainWindow.Heading6");
    sm->registerAction(ui.actionHeadingNormal, "MainWindow.HeadingNormal");
    sm->registerAction(ui.actionHeadingPreserveAttributes, "MainWindow.HeadingPreserveAttributes");
    sm->registerAction(ui.actionCasingLowercase, "MainWindow.CasingLowercase");
    sm->registerAction(ui.actionCasingUppercase, "MainWindow.CasingUppercase");
    sm->registerAction(ui.actionCasingPropercase, "MainWindow.CasingPropercase");
    sm->registerAction(ui.actionCasingCapitalize, "MainWindow.CasingCapitalize");

    // Tools
    sm->registerAction(ui.actionMetaEditor, "MainWindow.MetaEditor");
    sm->registerAction(ui.actionGenerateTOC, "MainWindow.GenerateTOC");
    sm->registerAction(ui.actionCreateHTMLTOC, "MainWindow.CreateHTMLTOC");
    sm->registerAction(ui.actionValidateEpub, "MainWindow.ValidateEpub");
    sm->registerAction(ui.actionAutoSpellCheck, "MainWindow.AutoSpellCheck");
    sm->registerAction(ui.actionSpellCheck, "MainWindow.SpellCheck");
    sm->registerAction(ui.actionClearIgnoredWords, "MainWindow.ClearIgnoredWords");
    sm->registerAction(ui.actionReports, "MainWindow.Reports");
    sm->registerAction(ui.actionSearchEditor, "MainWindow.SearchEditor");
    sm->registerAction(ui.actionClipEditor, "MainWindow.ClipEditor");
    sm->registerAction(ui.actionIndexEditor, "MainWindow.IndexEditor");
    sm->registerAction(ui.actionAddToIndex, "MainWindow.AddToIndex");
    sm->registerAction(ui.actionMarkForIndex, "MainWindow.MarkForIndex");
    sm->registerAction(ui.actionCreateIndex, "MainWindow.CreateIndex");
    sm->registerAction(ui.actionCheckWellFormedErrors, "MainWindow.CheckWellFormedErrors");

    // View
    sm->registerAction(ui.actionBookView, "MainWindow.BookView");
    sm->registerAction(ui.actionSplitView, "MainWindow.SplitView");
    sm->registerAction(ui.actionCodeView, "MainWindow.CodeView");
    sm->registerAction(ui.actionToggleViewState, "MainWindow.ToggleViewState");
    sm->registerAction(ui.actionZoomIn, "MainWindow.ZoomIn");
    sm->registerAction(ui.actionZoomOut, "MainWindow.ZoomOut");
    sm->registerAction(ui.actionZoomReset, "MainWindow.ZoomReset");
    sm->registerAction(m_BookBrowser->toggleViewAction(), "MainWindow.BookBrowser");
    sm->registerAction(m_ValidationResultsView->toggleViewAction(), "MainWindow.ValidationResults");
    sm->registerAction(m_TableOfContents->toggleViewAction(), "MainWindow.TableOfContents");

    // Window
    sm->registerAction(ui.actionNextTab, "MainWindow.NextTab");
    sm->registerAction(ui.actionPreviousTab, "MainWindow.PreviousTab");
    sm->registerAction(ui.actionCloseTab, "MainWindow.CloseTab");
    sm->registerAction(ui.actionCloseOtherTabs, "MainWindow.CloseOtherTabs");
    sm->registerAction(ui.actionOpenPreviousResource, "MainWindow.OpenPreviousResource");
    sm->registerAction(ui.actionOpenNextResource, "MainWindow.OpenNextResource");

    // Help
    sm->registerAction(ui.actionUserGuide, "MainWindow.UserGuide");
    sm->registerAction(ui.actionFAQ, "MainWindow.FAQ");
    sm->registerAction(ui.actionTutorials, "MainWindow.FAQ");
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

    icon = ui.actionSpellCheck->icon();
    icon.addFile(QString::fromUtf8(":/main/document-spellcheck_16px.png"));
    ui.actionSpellCheck->setIcon(icon);

    icon = ui.actionAutoSpellCheck->icon();
    icon.addFile(QString::fromUtf8(":/main/document-autospellcheck_16px.png"));
    ui.actionAutoSpellCheck->setIcon(icon);

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

    icon = ui.actionCheckWellFormedErrors->icon();
    icon.addFile(QString::fromUtf8(":/main/document-well-formed_check_16px.png"));
    ui.actionCheckWellFormedErrors->setIcon(icon);

    icon = ui.actionAlignLeft->icon();
    icon.addFile(QString::fromUtf8(":/main/format-justify-left_16px.png"));
    ui.actionAlignLeft->setIcon(icon);

    icon = ui.actionAlignRight->icon();
    icon.addFile(QString::fromUtf8(":/main/format-justify-right_16px.png"));
    ui.actionAlignRight->setIcon(icon);

    icon = ui.actionAlignCenter->icon();
    icon.addFile(QString::fromUtf8(":/main/format-justify-center_16px.png"));
    ui.actionAlignCenter->setIcon(icon);

    icon = ui.actionAlignJustify->icon();
    icon.addFile(QString::fromUtf8(":/main/format-justify-fill_16px.png"));
    ui.actionAlignJustify->setIcon(icon);

    icon = ui.actionBold->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-bold_16px.png"));
    ui.actionBold->setIcon(icon);

    icon = ui.actionItalic->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-italic_16px.png"));
    ui.actionItalic->setIcon(icon);

    icon = ui.actionUnderline->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-underline_16px.png"));
    ui.actionUnderline->setIcon(icon);
    
    icon = ui.actionStrikethrough->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-strikethrough_16px.png"));
    ui.actionStrikethrough->setIcon(icon);

    icon = ui.actionSubscript->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-subscript_16px.png"));
    ui.actionSubscript->setIcon(icon);

    icon = ui.actionSuperscript->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-superscript_16px.png"));
    ui.actionSuperscript->setIcon(icon);

    icon = ui.actionInsertNumberedList->icon();
    icon.addFile(QString::fromUtf8(":/main/insert-numbered-list_16px.png"));
    ui.actionInsertNumberedList->setIcon(icon);

    icon = ui.actionInsertBulletedList->icon();
    icon.addFile(QString::fromUtf8(":/main/insert-bullet-list_16px.png"));
    ui.actionInsertBulletedList->setIcon(icon);

    icon = ui.actionIncreaseIndent->icon();
    icon.addFile(QString::fromUtf8(":/main/format-indent-more_16px.png"));
    ui.actionIncreaseIndent->setIcon(icon);

    icon = ui.actionDecreaseIndent->icon();
    icon.addFile(QString::fromUtf8(":/main/format-indent-less_16px.png"));
    ui.actionDecreaseIndent->setIcon(icon);
    
    icon = ui.actionCasingLowercase->icon();
    icon.addFile(QString::fromUtf8(":/main/format-case-lowercase_16px.png"));
    ui.actionCasingLowercase->setIcon(icon);
    
    icon = ui.actionCasingUppercase->icon();
    icon.addFile(QString::fromUtf8(":/main/format-case-uppercase_16px.png"));
    ui.actionCasingUppercase->setIcon(icon);
    
    icon = ui.actionCasingPropercase->icon();
    icon.addFile(QString::fromUtf8(":/main/format-case-propercase_16px.png"));
    ui.actionCasingPropercase->setIcon(icon);
    
    icon = ui.actionCasingCapitalize->icon();
    icon.addFile(QString::fromUtf8(":/main/format-case-capitalize_16px.png"));
    ui.actionCasingCapitalize->setIcon(icon);
    
    icon = ui.actionTextDirectionLTR->icon();
    icon.addFile(QString::fromUtf8(":/main/format-direction-ltr_16px.png"));
    ui.actionTextDirectionLTR->setIcon(icon);

    icon = ui.actionTextDirectionRTL->icon();
    icon.addFile(QString::fromUtf8(":/main/format-direction-rtl_16px.png"));
    ui.actionTextDirectionRTL->setIcon(icon);

    icon = ui.actionTextDirectionDefault->icon();
    icon.addFile(QString::fromUtf8(":/main/format-direction-default_16px.png"));
    ui.actionTextDirectionDefault->setIcon(icon);

    icon = ui.actionHeading1->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-1_16px.png"));
    ui.actionHeading1->setIcon(icon);

    icon = ui.actionHeading2->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-2_16px.png"));
    ui.actionHeading2->setIcon(icon);

    icon = ui.actionHeading3->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-3_16px.png"));
    ui.actionHeading3->setIcon(icon);

    icon = ui.actionHeading4->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-4_16px.png"));
    ui.actionHeading4->setIcon(icon);

    icon = ui.actionHeading5->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-5_16px.png"));
    ui.actionHeading5->setIcon(icon);

    icon = ui.actionHeading6->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-6_16px.png"));
    ui.actionHeading6->setIcon(icon);

    icon = ui.actionHeadingNormal->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-normal_16px.png"));
    ui.actionHeadingNormal->setIcon(icon);

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
    // Setup signal mapping for heading actions.
    connect( ui.actionHeading1, SIGNAL( triggered() ), m_headingMapper, SLOT( map() ) );
    m_headingMapper->setMapping( ui.actionHeading1, "1" );
    connect( ui.actionHeading2, SIGNAL( triggered() ), m_headingMapper, SLOT( map() ) );
    m_headingMapper->setMapping( ui.actionHeading2, "2" );
    connect( ui.actionHeading3, SIGNAL( triggered() ), m_headingMapper, SLOT( map() ) );
    m_headingMapper->setMapping( ui.actionHeading3, "3" );
    connect( ui.actionHeading4, SIGNAL( triggered() ), m_headingMapper, SLOT( map() ) );
    m_headingMapper->setMapping( ui.actionHeading4, "4" );
    connect( ui.actionHeading5, SIGNAL( triggered() ), m_headingMapper, SLOT( map() ) );
    m_headingMapper->setMapping( ui.actionHeading5, "5" );
    connect( ui.actionHeading6, SIGNAL( triggered() ), m_headingMapper, SLOT( map() ) );
    m_headingMapper->setMapping( ui.actionHeading6, "6" );
    connect( ui.actionHeadingNormal, SIGNAL( triggered() ), m_headingMapper, SLOT( map() ) );
    m_headingMapper->setMapping( ui.actionHeadingNormal, "Normal" );

    // File
    connect( ui.actionNew,           SIGNAL( triggered() ), this, SLOT( New()                      ) );
    connect( ui.actionOpen,          SIGNAL( triggered() ), this, SLOT( Open()                     ) );
    connect( ui.actionNewHTMLFile,   SIGNAL( triggered() ), m_BookBrowser, SLOT( AddNewHTML()      ) );
    connect( ui.actionNewCSSFile,    SIGNAL( triggered() ), m_BookBrowser, SLOT( AddNewCSS()       ) );
    connect( ui.actionNewSVGFile,    SIGNAL( triggered() ), m_BookBrowser, SLOT( AddNewSVG()       ) );
    connect( ui.actionAddExistingFile,SIGNAL(triggered() ), m_BookBrowser, SLOT( AddExisting()     ) );
    connect( ui.actionSave,          SIGNAL( triggered() ), this, SLOT( Save()                     ) );
    connect( ui.actionSaveAs,        SIGNAL( triggered() ), this, SLOT( SaveAs()                   ) );
    connect( ui.actionSaveACopy,     SIGNAL( triggered() ), this, SLOT( SaveACopy()                ) );
    connect( ui.actionClose,         SIGNAL( triggered() ), this, SLOT( close()                    ) );
    connect( ui.actionExit,          SIGNAL( triggered() ), qApp, SLOT( closeAllWindows()          ) );

    // Edit
    connect( ui.actionInsertImage,     SIGNAL( triggered() ), this, SLOT( InsertImage()              ) );
    connect( ui.actionInsertSpecialCharacter, SIGNAL( triggered() ), this, SLOT( InsertSpecialCharacter()              ) );
    connect( ui.actionInsertId,        SIGNAL( triggered() ),  this,   SLOT( InsertId()            ) );
    connect( ui.actionInsertHyperlink, SIGNAL( triggered() ),  this,   SLOT( InsertHyperlink()     ) );

    connect( ui.actionPreferences,     SIGNAL( triggered() ), this, SLOT( PreferencesDialog()        ) );

    // Search
    connect( ui.actionFind,          SIGNAL( triggered() ), this, SLOT( Find()                     ) );
    connect( ui.actionFindNext,      SIGNAL( triggered() ), m_FindReplace, SLOT( FindNext()        ) );
    connect( ui.actionFindPrevious,  SIGNAL( triggered() ), m_FindReplace, SLOT( FindPrevious()    ) );
    connect( ui.actionReplaceNext,   SIGNAL( triggered() ), m_FindReplace, SLOT( ReplaceNext()     ) );
    connect( ui.actionReplacePrevious,SIGNAL(triggered() ), m_FindReplace, SLOT( ReplacePrevious() ) );
    connect( ui.actionReplaceAll,    SIGNAL( triggered() ), m_FindReplace, SLOT( ReplaceAll()      ) );
    connect( ui.actionCount,         SIGNAL( triggered() ), m_FindReplace, SLOT( Count()           ) );
    connect( ui.actionGoToLine,      SIGNAL( triggered() ), this, SLOT( GoToLine()                 ) );
    connect( ui.actionRegexDotAll,   SIGNAL( triggered(bool) ), this, SLOT( SetRegexOptionDotAll(bool)        ) );
    connect( ui.actionRegexMinimalMatch, SIGNAL( triggered(bool) ), this, SLOT( SetRegexOptionMinimalMatch(bool) ) );
    connect( ui.actionRegexAutoTokenise, SIGNAL( triggered(bool) ), this, SLOT( SetRegexOptionAutoTokenise(bool) ) );

    // About
    connect( ui.actionUserGuide,     SIGNAL( triggered() ), this, SLOT( UserGuide()                ) );
    connect( ui.actionFAQ,           SIGNAL( triggered() ), this, SLOT( FrequentlyAskedQuestions() ) );
    connect( ui.actionTutorials,     SIGNAL( triggered() ), this, SLOT( Tutorials()                ) );
    connect( ui.actionDonate,        SIGNAL( triggered() ), this, SLOT( Donate()                   ) );
    connect( ui.actionReportAnIssue, SIGNAL( triggered() ), this, SLOT( ReportAnIssue()            ) );
    connect( ui.actionSigilDevBlog,  SIGNAL( triggered() ), this, SLOT( SigilDevBlog()             ) );
    connect( ui.actionAbout,         SIGNAL( triggered() ), this, SLOT( AboutDialog()              ) );

    // Tools
    connect( ui.actionMetaEditor,    SIGNAL( triggered() ), this, SLOT( MetaEditorDialog()         ) );
    connect( ui.actionValidateEpub,  SIGNAL( triggered() ), this, SLOT( ValidateEpub()             ) );
    connect( ui.actionAutoSpellCheck, SIGNAL( triggered( bool ) ), this, SLOT( SetAutoSpellCheck( bool ) ) );
    connect( ui.actionSpellCheck,    SIGNAL( triggered() ), m_FindReplace, SLOT( FindMisspelledWord() ) );
    connect( ui.actionClearIgnoredWords, SIGNAL( triggered() ), this, SLOT( ClearIgnoredWords()    ) );
    connect( ui.actionGenerateTOC,   SIGNAL( triggered() ), this, SLOT( GenerateToc()              ) );
    connect( ui.actionCreateHTMLTOC, SIGNAL( triggered() ), this, SLOT( CreateHTMLTOC()            ) );
    connect( ui.actionReports,       SIGNAL( triggered() ), this, SLOT( ReportsDialog()            ) );
    connect( ui.actionClipEditor,    SIGNAL( triggered() ), this, SLOT( ClipEditorDialog()         ) );
    connect( ui.actionSearchEditor,  SIGNAL( triggered() ), this, SLOT( SearchEditorDialog()       ) );
    connect( ui.actionIndexEditor,   SIGNAL( triggered() ), this, SLOT( IndexEditorDialog()        ) );
    connect( ui.actionCreateIndex,   SIGNAL( triggered() ), this, SLOT( CreateIndex()              ) );
    connect( ui.actionCheckWellFormedErrors, SIGNAL( triggered( bool ) ), this, SLOT( SetCheckWellFormedErrors( bool ) ) );

    // Tidy clean
    connect(ui.actionCleanLevelOff, SIGNAL(triggered()), m_cleanMapper, SLOT(map()));
    m_cleanMapper->setMapping(ui.actionCleanLevelOff, SettingsStore::CleanLevel_Off);
    connect(ui.actionCleanLevelPrettyPrint, SIGNAL(triggered()), m_cleanMapper, SLOT(map()));
    m_cleanMapper->setMapping(ui.actionCleanLevelPrettyPrint, SettingsStore::CleanLevel_PrettyPrint);
    connect(ui.actionCleanLevelTidy, SIGNAL(triggered()), m_cleanMapper, SLOT(map()));
    m_cleanMapper->setMapping(ui.actionCleanLevelTidy, SettingsStore::CleanLevel_Tidy);
    connect(m_cleanMapper, SIGNAL(mapped(int)), this, SLOT(setCleanLevel(int)));

    // Change case
    connect(ui.actionCasingLowercase,  SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    connect(ui.actionCasingUppercase,  SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    connect(ui.actionCasingPropercase, SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    connect(ui.actionCasingCapitalize, SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    m_casingChangeMapper->setMapping(ui.actionCasingLowercase,  Utility::Casing_Lowercase);
    m_casingChangeMapper->setMapping(ui.actionCasingUppercase,  Utility::Casing_Uppercase);
    m_casingChangeMapper->setMapping(ui.actionCasingPropercase, Utility::Casing_Propercase);
    m_casingChangeMapper->setMapping(ui.actionCasingCapitalize, Utility::Casing_Capitalize);
    connect(m_casingChangeMapper, SIGNAL(mapped(int)), this, SLOT(ChangeCasing(int)));

    // View
    connect( ui.actionZoomIn,        SIGNAL( triggered() ), this, SLOT( ZoomIn()                   ) );
    connect( ui.actionZoomOut,       SIGNAL( triggered() ), this, SLOT( ZoomOut()                  ) );
    connect( ui.actionZoomReset,     SIGNAL( triggered() ), this, SLOT( ZoomReset()                ) );
    connect( ui.actionBookView,      SIGNAL( triggered() ),  this,   SLOT( BookView()  ) );
    connect( ui.actionSplitView,     SIGNAL( triggered() ),  this,   SLOT( SplitView() ) );
    connect( ui.actionCodeView,      SIGNAL( triggered() ),  this,   SLOT( CodeView()  ) );
    connect( ui.actionToggleViewState, SIGNAL( triggered() ),  this,   SLOT( ToggleViewState()  ) );

    connect( ui.actionHeadingPreserveAttributes, SIGNAL( triggered( bool ) ), this, SLOT( SetPreserveHeadingAttributes( bool ) ) );
    connect( m_headingMapper,      SIGNAL( mapped( const QString& ) ),  this,   SLOT( ApplyHeadingStyleToTab( const QString& ) ) );
    
    // Window
    connect( ui.actionNextTab,       SIGNAL( triggered() ), &m_TabManager, SLOT( NextTab()     ) );
    connect( ui.actionPreviousTab,   SIGNAL( triggered() ), &m_TabManager, SLOT( PreviousTab() ) );
    connect( ui.actionCloseTab,      SIGNAL( triggered() ), &m_TabManager, SLOT( CloseTab()    ) );
    connect( ui.actionCloseOtherTabs,SIGNAL( triggered() ), &m_TabManager, SLOT( CloseOtherTabs() ) );
    connect( ui.actionOpenPreviousResource, SIGNAL( triggered() ), m_BookBrowser, SLOT( OpenPreviousResource() ) );
    connect( ui.actionOpenNextResource,     SIGNAL( triggered() ), m_BookBrowser, SLOT( OpenNextResource()     ) );
    connect( ui.actionGoBackFromLinkOrStyle,  SIGNAL( triggered() ), this,   SLOT( GoBackFromLinkOrStyle()  ) );
    
    connect( ui.actionSplitOnSGFChapterMarkers, SIGNAL( triggered() ),  this,   SLOT( SplitOnSGFChapterMarkers() ) );

    connect( ui.actionPasteClipboardHistory,    SIGNAL( triggered() ),  this,   SLOT( ShowPasteClipboardHistoryDialog() ) );

    // Slider
    connect( m_slZoomSlider,         SIGNAL( valueChanged( int ) ), this, SLOT( SliderZoom( int ) ) );
    // We also update the label when the slider moves... this is to show
    // the zoom value the slider will land on while it is being moved.
    connect( m_slZoomSlider,         SIGNAL( sliderMoved( int ) ),  this, SLOT( UpdateZoomLabel( int ) ) );


    connect( &m_TabManager,          SIGNAL( TabCountChanged() ), 
             this,                   SLOT( UpdateUIOnTabCountChange() ) );

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

    connect( m_BookBrowser, SIGNAL( RemoveTabRequest() ),
             &m_TabManager, SLOT(   RemoveTab() ) );

    connect( m_BookBrowser, SIGNAL( ResourceActivated( Resource& ) ),
             this, SLOT(   OpenResource(          Resource& ) ) );

    connect(m_BookBrowser, SIGNAL(MergeResourcesRequest(QList<Resource *>)), this, SLOT(MergeResources(QList<Resource *>)));

    connect(m_BookBrowser, SIGNAL(LinkStylesheetsToResourcesRequest(QList<Resource *>)), this, SLOT(LinkStylesheetsToResources(QList<Resource *>)));

    connect(m_BookBrowser, SIGNAL(InsertImagesRequest(QStringList)), this, SLOT(InsertImages(QStringList)));

    connect(m_BookBrowser, SIGNAL(RemoveResourcesRequest()), this, SLOT(RemoveResources()));

    connect( m_TableOfContents, SIGNAL( OpenResourceRequest( Resource&, bool, const QUrl& ) ),
             this,     SLOT(   OpenResource(        Resource&, bool, const QUrl& ) ) );

    connect( m_ValidationResultsView,
                SIGNAL( OpenResourceRequest( Resource&, bool, const QUrl&, MainWindow::ViewState, int ) ),
             this,
                SLOT(   OpenResource(        Resource&, bool, const QUrl&, MainWindow::ViewState, int ) ) );

    connect( &m_TabManager, SIGNAL( OpenUrlRequest( const QUrl& ) ),
             this, SLOT(   OpenUrl( const QUrl& ) ) );

    connect( &m_TabManager, SIGNAL( OldTabRequest(            QString, HTMLResource& ) ),
             this,          SLOT(   CreateChapterBreakOldTab( QString, HTMLResource& ) ) );

    connect( &m_TabManager, SIGNAL( ToggleViewStateRequest() ),
             this,          SLOT(   ToggleViewState() ) );

    connect(m_FindReplace, SIGNAL( OpenSearchEditorRequest(SearchEditorModel::searchEntry *) ),
            this,          SLOT( SearchEditorDialog(SearchEditorModel::searchEntry *)     ) );

    connect(m_FindReplace, SIGNAL( ShowMessageRequest(QString) ),
            m_SearchEditor, SLOT( ShowMessage(QString)  ) );

    connect(m_SearchEditor, SIGNAL(LoadSelectedSearchRequest(      SearchEditorModel::searchEntry *)),
            m_FindReplace,   SLOT( LoadSearch(                     SearchEditorModel::searchEntry *)));
    connect(m_SearchEditor, SIGNAL(FindSelectedSearchRequest(      QList<SearchEditorModel::searchEntry *>)),
            m_FindReplace,   SLOT( FindSearch(                     QList<SearchEditorModel::searchEntry *>)));
    connect(m_SearchEditor, SIGNAL(ReplaceSelectedSearchRequest(   QList<SearchEditorModel::searchEntry *>)),
            m_FindReplace,   SLOT( ReplaceSearch(                  QList<SearchEditorModel::searchEntry *>)));
    connect(m_SearchEditor, SIGNAL(CountAllSelectedSearchRequest(  QList<SearchEditorModel::searchEntry *>)),
            m_FindReplace,   SLOT( CountAllSearch(                 QList<SearchEditorModel::searchEntry *>)));
    connect(m_SearchEditor, SIGNAL(ReplaceAllSelectedSearchRequest(QList<SearchEditorModel::searchEntry *>)),
            m_FindReplace,   SLOT( ReplaceAllSearch(               QList<SearchEditorModel::searchEntry *>)));

    connect( m_IndexEditor, SIGNAL( CreateIndexRequest() ),
             this,            SLOT( CreateIndex() ) );
}

void MainWindow::MakeTabConnections( ContentTab *tab )
{
    if ( tab == NULL )

        return;

    // Triggered connections should be disconnected in BreakTabConnections
    if (tab->GetLoadedResource().Type() != Resource::ImageResourceType)
    {
        connect( ui.actionUndo,                     SIGNAL( triggered() ),  tab,   SLOT( Undo()                     ) );
        connect( ui.actionRedo,                     SIGNAL( triggered() ),  tab,   SLOT( Redo()                     ) );
        connect( ui.actionCut,                      SIGNAL( triggered() ),  tab,   SLOT( Cut()                      ) );
        connect( ui.actionCopy,                     SIGNAL( triggered() ),  tab,   SLOT( Copy()                     ) );
        connect( ui.actionPaste,                    SIGNAL( triggered() ),  tab,   SLOT( Paste()                    ) );
        connect( m_ClipboardHistorySelector,        SIGNAL( PasteFromClipboardRequest() ), tab, SLOT( Paste()       ) );

        connect( tab,   SIGNAL( OpenClipEditorRequest(ClipEditorModel::clipEntry *) ),
                 this,  SLOT (  ClipEditorDialog( ClipEditorModel::clipEntry * ) ) );
    }

    if (tab->GetLoadedResource().Type() == Resource::HTMLResourceType ||
        tab->GetLoadedResource().Type() == Resource::CSSResourceType)
    {
        connect( ui.actionBold,                     SIGNAL( triggered() ),  tab,   SLOT( Bold()                     ) );
        connect( ui.actionItalic,                   SIGNAL( triggered() ),  tab,   SLOT( Italic()                   ) );
        connect( ui.actionUnderline,                SIGNAL( triggered() ),  tab,   SLOT( Underline()                ) );    
        connect( ui.actionStrikethrough,            SIGNAL( triggered() ),  tab,   SLOT( Strikethrough()            ) );

        connect( ui.actionAlignLeft,                SIGNAL( triggered() ),  tab,   SLOT( AlignLeft()                ) );
        connect( ui.actionAlignCenter,              SIGNAL( triggered() ),  tab,   SLOT( AlignCenter()              ) );
        connect( ui.actionAlignRight,               SIGNAL( triggered() ),  tab,   SLOT( AlignRight()               ) );
        connect( ui.actionAlignJustify,             SIGNAL( triggered() ),  tab,   SLOT( AlignJustify()             ) );
        
        connect( ui.actionTextDirectionLTR,         SIGNAL( triggered() ),  tab,   SLOT( TextDirectionLeftToRight() ) );
        connect( ui.actionTextDirectionRTL,         SIGNAL( triggered() ),  tab,   SLOT( TextDirectionRightToLeft() ) );
        connect( ui.actionTextDirectionDefault,     SIGNAL( triggered() ),  tab,   SLOT( TextDirectionDefault()     ) );

        connect( tab,   SIGNAL( SelectionChanged() ),           this,          SLOT( UpdateUIOnTabChanges()         ) );
    }

    if (tab->GetLoadedResource().Type() == Resource::HTMLResourceType )
    {
        connect( ui.actionSubscript,                SIGNAL( triggered() ),  tab,   SLOT( Subscript()                ) );
        connect( ui.actionSuperscript,              SIGNAL( triggered() ),  tab,   SLOT( Superscript()              ) );
        connect( ui.actionInsertBulletedList,       SIGNAL( triggered() ),  tab,   SLOT( InsertBulletedList()       ) );
        connect( ui.actionInsertNumberedList,       SIGNAL( triggered() ),  tab,   SLOT( InsertNumberedList()       ) );
        connect( ui.actionDecreaseIndent,           SIGNAL( triggered() ),  tab,   SLOT( DecreaseIndent()           ) );
        connect( ui.actionIncreaseIndent,           SIGNAL( triggered() ),  tab,   SLOT( IncreaseIndent()           ) );
        connect( ui.actionRemoveFormatting,         SIGNAL( triggered() ),  tab,   SLOT( RemoveFormatting()         ) );

        connect( ui.actionSplitChapter,             SIGNAL( triggered() ),  tab,   SLOT( SplitChapter()             ) );
        connect( ui.actionInsertSGFChapterMarker,   SIGNAL( triggered() ),  tab,   SLOT( InsertSGFChapterMarker()   ) );
        connect( ui.actionInsertClosingTag,         SIGNAL( triggered() ),  tab,   SLOT( InsertClosingTag()         ) );
        connect( ui.actionGoToLinkOrStyle,          SIGNAL( triggered() ),  tab,   SLOT( GoToLinkOrStyle()          ) );

        connect( ui.actionPrintPreview,             SIGNAL( triggered() ),  tab,   SLOT( PrintPreview()             ) );
        connect( ui.actionPrint,                    SIGNAL( triggered() ),  tab,   SLOT( Print()                    ) );
        connect( ui.actionAddToIndex,               SIGNAL( triggered() ),  tab,   SLOT( AddToIndex()               ) );
        connect( ui.actionMarkForIndex,             SIGNAL( triggered() ),  tab,   SLOT( MarkForIndex()             ) );

        connect( this,                              SIGNAL( SettingsChanged()), tab, SLOT( LoadSettings()           ) );

        connect( m_SelectCharacter, SIGNAL( SelectedCharacter(const QString&) ),
                 tab,            SLOT( InsertText(const QString&) ) );
    
        connect( tab,   SIGNAL( EnteringBookView() ),           this,          SLOT( SetStateActionsBookView() ) );
        connect( tab,   SIGNAL( EnteringBookPreview() ),        this,          SLOT( SetStateActionsSplitView() ) );
        connect( tab,   SIGNAL( EnteringCodeView() ),           this,          SLOT( SetStateActionsCodeView() ) );
        connect( tab,   SIGNAL( EnteringBookView() ),           this,          SLOT( UpdateZoomControls()      ) );
        connect( tab,   SIGNAL( EnteringBookPreview() ),        this,          SLOT( UpdateZoomControls() ) );
        connect( tab,   SIGNAL( EnteringCodeView() ),           this,          SLOT( UpdateZoomControls()      ) );

        connect( tab,   SIGNAL( OpenIndexEditorRequest(IndexEditorModel::indexEntry *) ),
                 this,  SLOT (  IndexEditorDialog( IndexEditorModel::indexEntry * ) ) );

        connect( tab,   SIGNAL( GoToLinkedStyleDefinitionRequest( const QString&, const QString& ) ),
                 this,  SLOT (  GoToLinkedStyleDefinition( const QString&, const QString& ) ) );

        connect( tab,   SIGNAL( BookmarkLinkOrStyleLocationRequest() ),
                 this,  SLOT (  BookmarkLinkOrStyleLocation() ) );

        connect( tab,   SIGNAL( ClipboardSaveRequest() ),     m_ClipboardHistorySelector,  SLOT( SaveClipboardState() ) );
        connect( tab,   SIGNAL( ClipboardRestoreRequest() ),  m_ClipboardHistorySelector,  SLOT( RestoreClipboardState() ) );

        connect( tab,   SIGNAL( SpellingHighlightRefreshRequest() ), this,  SLOT(  RefreshSpellingHighlighting() ) );
    }

    if (tab->GetLoadedResource().Type() == Resource::HTMLResourceType ||
        tab->GetLoadedResource().Type() == Resource::CSSResourceType ) {
        connect( m_ClipEditor, SIGNAL( PasteSelectedClipRequest(QList<ClipEditorModel::clipEntry *>) ),
                 tab,                SLOT(   PasteClipEntries(QList<ClipEditorModel::clipEntry *>) ) );
    }

    connect( tab,   SIGNAL( ContentChanged() ),             m_Book.data(), SLOT( SetModified()             ) );
    connect( tab,   SIGNAL( UpdateCursorPosition(int,int)), this,          SLOT( UpdateCursorPositionLabel(int,int)));
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
    disconnect( ui.actionBold,                      0, tab, 0 );
    disconnect( ui.actionItalic,                    0, tab, 0 );
    disconnect( ui.actionUnderline,                 0, tab, 0 );
    disconnect( ui.actionStrikethrough,             0, tab, 0 );
    disconnect( ui.actionSubscript,                 0, tab, 0 );
    disconnect( ui.actionSuperscript,               0, tab, 0 );
    disconnect( ui.actionAlignLeft,                 0, tab, 0 );
    disconnect( ui.actionAlignCenter,               0, tab, 0 );
    disconnect( ui.actionAlignRight,                0, tab, 0 );
    disconnect( ui.actionAlignJustify,              0, tab, 0 );
    disconnect( ui.actionInsertBulletedList,        0, tab, 0 );
    disconnect( ui.actionInsertNumberedList,        0, tab, 0 );
    disconnect( ui.actionDecreaseIndent,            0, tab, 0 );
    disconnect( ui.actionIncreaseIndent,            0, tab, 0 );
    disconnect( ui.actionTextDirectionLTR,          0, tab, 0 );
    disconnect( ui.actionTextDirectionRTL,          0, tab, 0 );
    disconnect( ui.actionTextDirectionDefault,      0, tab, 0 );
    disconnect( ui.actionRemoveFormatting,          0, tab, 0 );

    disconnect( ui.actionSplitChapter,              0, tab, 0 );
    disconnect( ui.actionInsertSGFChapterMarker,    0, tab, 0 );
    disconnect( ui.actionInsertClosingTag,          0, tab, 0 );
    disconnect( ui.actionGoToLinkOrStyle,           0, tab, 0 );

    disconnect( ui.actionPrintPreview,              0, tab, 0 );
    disconnect( ui.actionPrint,                     0, tab, 0 );
    disconnect( ui.actionAddToIndex,                0, tab, 0 );
    disconnect( ui.actionMarkForIndex,              0, tab, 0 );

    disconnect( m_ClipEditor,                       0, tab, 0 );
    disconnect( m_ClipboardHistorySelector,         0, tab, 0 );

    disconnect( tab,                                0, this, 0 );
    disconnect( tab,                                0, m_Book.data(), 0 );

    disconnect( m_SelectCharacter,                  0, tab, 0 );
}

