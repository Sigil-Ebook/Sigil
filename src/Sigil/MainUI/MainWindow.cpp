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
#include "Misc/Utility.h"
#include "MainWindow.h"
#include "BookManipulation/FolderKeeper.h"
#include "Exporters/ExportEPUB.h"
#include "Exporters/ExportSGF.h"
#include "Dialogs/MetaEditor.h"
#include "Dialogs/About.h"
#include "Dialogs/TOCEditor.h"
#include "Dialogs/FindReplace.h"
#include "Importers/ImporterFactory.h"
#include "Exporters/ExporterFactory.h"
#include "BookManipulation/BookNormalization.h"
#include "BookManipulation/SigilMarkup.h"
#include "MainUI/BookBrowser.h"
#include "Tabs/ContentTab.h"
#include "Tabs/TabManager.h"


static const int STATUSBAR_MSG_DISPLAY_TIME = 2000;
static const int TEXT_ELIDE_WIDTH           = 300;
static const QString SETTINGS_GROUP         = "mainwindow";
static const float ZOOM_STEP                = 0.1f;
static const float ZOOM_MIN                 = 0.09f;
static const float ZOOM_MAX                 = 5.0f;
static const float ZOOM_NORMAL              = 1.0f;
static const int ZOOM_SLIDER_MIN            = 0;
static const int ZOOM_SLIDER_MAX            = 1000;
static const int ZOOM_SLIDER_MIDDLE         = qRound( ( ZOOM_SLIDER_MAX - ZOOM_SLIDER_MIN ) / 2.0f );
static const int ZOOM_SLIDER_WIDTH          = 140;
static const QString REPORTING_ISSUES_WIKI  = "http://code.google.com/p/sigil/wiki/ReportingIssues";
static const QString SIGIL_DEV_BLOG         = "http://sigildev.blogspot.com/";

// The <hr> tag is wrapped in <div>'s because of issue #78;
// basically it's a workaround for a webkit bug
const QString BREAK_TAG_INSERT              = "<div><hr class=\"sigilChapterBreak\" /></div>";

static const QString FRAME_NAME = "managerframe";
static const QString TAB_STYLE_SHEET =  "#managerframe {border-top: 0px solid white;"
                                        "border-left: 1px solid grey;"
                                        "border-right: 1px solid grey;"
                                        "border-bottom: 1px solid grey;} ";

QStringList MainWindow::s_RecentFiles = QStringList();

// Constructor.
// The first argument is the path to the file that the window
// should load (new file loaded if empty); the second is the
// window's parent; the third specifies the flags used to modify window behaviour
MainWindow::MainWindow( const QString &openfilepath, QWidget *parent, Qt::WFlags flags )
    : 
    QMainWindow( parent, flags ),
    m_CurrentFile( QString() ),
    m_Book( new Book() ),
    m_LastFolderOpen( QString() ),
    m_LastFolderSave( QString() ),
    m_TabManager( *new TabManager( this ) ),
    m_cbHeadings( NULL ),
    m_slZoomSlider( NULL ),
    m_lbZoomLabel( NULL ),
    c_SaveFilters( GetSaveFiltersMap() ),
    c_LoadFilters( GetLoadFiltersMap() )
{
    ui.setupUi( this );
	
    // Telling Qt to delete this window
    // from memory when it is closed
	setAttribute( Qt::WA_DeleteOnClose );

    ExtendUI();

    // Needs to come before signals connect
    // (avoiding side-effects)
    ReadSettings();

    ConnectSignalsToSlots();

    CreateRecentFilesActions();
    UpdateRecentFileActions();

    TabChanged( NULL, &m_TabManager.GetCurrentContentTab() );

    LoadInitialFile( openfilepath );
}


// Overrides the closeEvent handler so we can check
// for saved status before actually closing
void MainWindow::closeEvent( QCloseEvent *event )
{
    if ( MaybeSave() )
    {
        WriteSettings();

        event->accept();
    } 

    else
    {
        event->ignore();
    }
}


// Implements New action functionality
void MainWindow::New()
{
    // The nasty IFDEFs are here to enable the multi-document
    // interface on the Mac; Lin and Win just use multiple
    // instances of the Sigil application
#ifndef Q_WS_MAC
    if ( MaybeSave() )
#endif
    {
#ifdef Q_WS_MAC
        MainWindow *new_window = new MainWindow();
        new_window->show();
#else
        CreateNew();
#endif
    }
}


// Implements Open action functionality
void MainWindow::Open()
{
    // The nasty IFDEFs are here to enable the multi-document
    // interface on the Mac; Lin and Win just use multiple
    // instances of the Sigil application
#ifndef Q_WS_MAC
    if ( MaybeSave() )
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

        QString filename = QFileDialog::getOpenFileName(    this,
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
}


// Implements Open recent file action functionality
void MainWindow::OpenRecentFile()
{
    // The nasty IFDEFs are here to enable the multi-document
    // interface on the Mac; Lin and Win just use multiple
    // instances of the Sigil application
    
    QAction *action = qobject_cast< QAction *>( sender() );
    
    if ( action != NULL )
    {
#ifndef Q_WS_MAC
        if ( MaybeSave() )
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


// Implements Save action functionality
bool MainWindow::Save()
{
    if ( m_CurrentFile.isEmpty() )
    {
        return SaveAs();
    }

    else
    {
        return SaveFile( m_CurrentFile );
    }
}


// Implements Save As action functionality
bool MainWindow::SaveAs()
{
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
    if ( c_SaveFilters.contains( QFileInfo( m_CurrentFile ).suffix().toLower() ) )
    {
        save_path       = m_LastFolderSave + "/" + QFileInfo( m_CurrentFile ).fileName();
        default_filter  = c_SaveFilters.value( QFileInfo( m_CurrentFile ).suffix().toLower() );
    }

    // If not, we change the extension to SGF
    else
    {
        save_path       = m_LastFolderSave + "/" + QFileInfo( m_CurrentFile ).baseName() + ".sgf";
        default_filter  = c_SaveFilters.value( "sgf" );
    }

    QString filename = QFileDialog::getSaveFileName(    this, 
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


// Implements Find action functionality
void MainWindow::Find()
{
    if ( m_FindReplace.isNull() )
    {   
        // Qt will delete this dialog from memory when it closes
        m_FindReplace = new FindReplace( true, m_TabManager, this );

        m_FindReplace.data()->show();
    }
}


// Implements Replace action functionality
void MainWindow::Replace()
{
    if ( m_FindReplace.isNull() )
    {   
        // Qt will delete this dialog from memory when it closes
        m_FindReplace = new FindReplace( false, m_TabManager, this );

        m_FindReplace.data()->show();
    }
}

// Implements Zoom In action functionality
void MainWindow::ZoomIn()
{
    ZoomByStep( true );  
}


// Implements Zoom Out action functionality
void MainWindow::ZoomOut()
{
    ZoomByStep( false );  
}


// Implements Meta Editor action functionality
void MainWindow::MetaEditorDialog()
{
    // ALWAYS clean up source first before
    // using m_Book outside of MainWindow!
    //TidyUp();

    MetaEditor meta( m_Book, this );

    if ( meta.exec() == QDialog::Accepted )

        setWindowModified( true );        
}


// Implements TOC Preview action functionality
void MainWindow::TOCEditorDialog()
{
    // ALWAYS clean up source first before
    // using m_Book outside of MainWindow!
    //TidyUp();

    TOCEditor toc( m_Book, this );

    if ( toc.exec() == QDialog::Accepted )
    {
        //UpdateBookViewFromSource();
        //UpdateCodeViewFromSource();
    }
}


// Implements Report An Issue action functionality
void MainWindow::ReportAnIssue()
{
    QDesktopServices::openUrl( QUrl( REPORTING_ISSUES_WIKI ) );
}


// Implements Sigil Dev Blog action functionality
void MainWindow::SigilDevBlog()
{
    QDesktopServices::openUrl( QUrl( SIGIL_DEV_BLOG ) );
}


// Implements About action functionality
void MainWindow::AboutDialog()
{
    About about( this );

    about.exec();
}



// Gets called every time the document is modified;
// changes the UI to accordingly;
// (star in titlebar on win and lin, different button colors on mac)
void MainWindow::DocumentWasModified()
{
    setWindowModified( m_TabManager.GetCurrentContentTab().IsModified() );
}


void MainWindow::TabChanged( ContentTab* old_tab, ContentTab* new_tab )
{
    BreakTabConnections( old_tab );
    MakeTabConnections( new_tab );
}


void MainWindow::UpdateUI()
{
    ContentTab &tab = m_TabManager.GetCurrentContentTab();

    if ( &tab == NULL )

        return;

    ui.actionCut  ->setEnabled( tab.CutEnabled() );
    ui.actionCopy ->setEnabled( tab.CopyEnabled() );
    ui.actionPaste->setEnabled( tab.PasteEnabled() );

    ui.actionBold     ->setChecked( tab.BoldChecked() );
    ui.actionItalic   ->setChecked( tab.ItalicChecked() );
    ui.actionUnderline->setChecked( tab.UnderlineChecked() );

    ui.actionStrikethrough     ->setChecked( tab.StrikethroughChecked() );
    ui.actionInsertBulletedList->setChecked( tab.BulletListChecked() );
    ui.actionInsertNumberedList->setChecked( tab.NumberListChecked() );

    ui.actionBookView ->setChecked( tab.BookViewChecked()  );
    ui.actionSplitView->setChecked( tab.SplitViewChecked() );
    ui.actionCodeView ->setChecked( tab.CodeViewChecked()  );  

    SelectEntryInHeadingCombo( tab.GetCaretElementName() );    
}


// When the user switches tabs, we need to enable/disable
// the WYSIWYG actions depending on the new tab's "view state" 
void MainWindow::TabSwitchChanges()
{
    ContentTab &tab = m_TabManager.GetCurrentContentTab();

    if ( &tab == NULL )

        return;

    if ( tab.GetViewState() == ContentTab::ViewState_BookView )

        SetStateActionsBookView();

    else if ( tab.GetViewState() == ContentTab::ViewState_CodeView )

        SetStateActionsCodeView();

    else if ( tab.GetViewState() == ContentTab::ViewState_RawView )
    
        SetStateActionsRawView();

    else 

        SetStateActionsStaticView();

    // State of zoom controls depends on current tab/view
    float zoom_factor = tab.GetZoomFactor();
    UpdateZoomLabel( zoom_factor );
    UpdateZoomSlider( zoom_factor );
}


// Set initial state for actions in Book View
// (enable the actions the Code View disabled)
void MainWindow::SetStateActionsBookView()
{
    ui.actionUndo->setEnabled( true );
    ui.actionRedo->setEnabled( true );

    ui.actionCut  ->setEnabled( true );  
    ui.actionCopy ->setEnabled( true ); 
    ui.actionPaste->setEnabled( true ); 

    ui.actionFind   ->setEnabled( true );
    ui.actionReplace->setEnabled( true );

    ui.actionBookView ->setEnabled( true );
    ui.actionSplitView->setEnabled( true );
    ui.actionCodeView ->setEnabled( true );  

    ui.actionBold         ->setEnabled( true );
    ui.actionItalic       ->setEnabled( true );
    ui.actionUnderline    ->setEnabled( true );
    ui.actionStrikethrough->setEnabled( true );

    ui.actionAlignLeft ->setEnabled( true );
    ui.actionCenter    ->setEnabled( true );
    ui.actionAlignRight->setEnabled( true );
    ui.actionJustify   ->setEnabled( true );

    ui.actionInsertImage       ->setEnabled( true );
    ui.actionInsertChapterBreak->setEnabled( true );

    ui.actionInsertBulletedList->setEnabled( true );
    ui.actionInsertNumberedList->setEnabled( true );

    ui.actionDecreaseIndent->setEnabled( true );
    ui.actionIncreaseIndent->setEnabled( true );

    m_cbHeadings->setEnabled( true );
}


// Set initial state for actions in Code View
// (disable the actions used in Book View that
// are not appropriate here)
void MainWindow::SetStateActionsCodeView()
{
    SetStateActionsBookView();

    // TODO: We shouldn't really disable these.
    // The Code View should insert correct HTML
    // when these are triggered

    ui.actionBold         ->setEnabled( false );
    ui.actionItalic       ->setEnabled( false );
    ui.actionUnderline    ->setEnabled( false );
    ui.actionStrikethrough->setEnabled( false );

    ui.actionAlignLeft ->setEnabled( false );
    ui.actionCenter    ->setEnabled( false );
    ui.actionAlignRight->setEnabled( false );
    ui.actionJustify   ->setEnabled( false );

    ui.actionInsertImage       ->setEnabled( false );
    ui.actionInsertChapterBreak->setEnabled( false );
    
    ui.actionInsertBulletedList->setEnabled( false );
    ui.actionInsertNumberedList->setEnabled( false );

    ui.actionDecreaseIndent->setEnabled( false );
    ui.actionIncreaseIndent->setEnabled( false );

    m_cbHeadings->setEnabled( false );

    ui.actionBold     ->setChecked( false );
    ui.actionItalic   ->setChecked( false );
    ui.actionUnderline->setChecked( false );

    ui.actionStrikethrough     ->setChecked( false );
    ui.actionInsertBulletedList->setChecked( false );
    ui.actionInsertNumberedList->setChecked( false );
}

// Set initial state for actions in Raw View
// (same as Code View, but, the actions for switching
// views are off as well; Raw View is for CSS, XML ... editing )
void MainWindow::SetStateActionsRawView()
{
    SetStateActionsCodeView();

    ui.actionBookView ->setEnabled( false );
    ui.actionSplitView->setEnabled( false );
    ui.actionCodeView ->setEnabled( false );  
}

void MainWindow::SetStateActionsStaticView()
{
    SetStateActionsRawView();

    ui.actionUndo->setEnabled( false );
    ui.actionRedo->setEnabled( false );

    ui.actionCut  ->setEnabled( false );  
    ui.actionCopy ->setEnabled( false ); 
    ui.actionPaste->setEnabled( false ); 

    ui.actionFind   ->setEnabled( false );
    ui.actionReplace->setEnabled( false );
}


// Zooms the current view with the new zoom slider value
void MainWindow::SliderZoom( int slider_value )
{
    float new_zoom_factor     = SliderRangeToZoomFactor( slider_value );
    float current_zoom_factor = m_TabManager.GetCurrentContentTab().GetZoomFactor();

    // We try to prevent infinite loops...
    if ( !qFuzzyCompare( new_zoom_factor, current_zoom_factor ) )

        ZoomByFactor( new_zoom_factor );
}

// Updates the zoom controls by reading the current
// zoom factor from the view. Needed on View changeover.
void MainWindow::UpdateZoomControls()
{
    float zoom_factor = m_TabManager.GetCurrentContentTab().GetZoomFactor(); 

    UpdateZoomSlider( zoom_factor );
    UpdateZoomLabel( zoom_factor );
}


// Updates the zooming slider to reflect the new zoom factor
void MainWindow::UpdateZoomSlider( float new_zoom_factor )
{
    m_slZoomSlider->setValue( ZoomFactorToSliderRange( new_zoom_factor ) );
}


// Updates the zoom label to reflect the state of the zoom slider.
// This is needed so the user can see to what zoom value the slider
// is being dragged to.
void MainWindow::UpdateZoomLabel( int slider_value )
{
    float zoom_factor = SliderRangeToZoomFactor( slider_value );

    UpdateZoomLabel( zoom_factor );
}


// Updates the zoom label to reflect the new zoom factor
void MainWindow::UpdateZoomLabel( float new_zoom_factor )
{
    m_lbZoomLabel->setText( QString( "%1% " ).arg( qRound( new_zoom_factor * 100 ) ) );
}


// Reads all the stored application settings like
// window position, geometry etc.
void MainWindow::ReadSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    QByteArray geometry = settings.value( "geometry" ).toByteArray();

    if ( !geometry.isNull() )

        restoreGeometry( geometry );

    // The positions of all the toolbars and dock widgets
    QByteArray toolbars = settings.value( "toolbars" ).toByteArray();

    if ( !toolbars.isNull() )

        restoreState( toolbars );

    // The position of the splitter handle in split view
    QByteArray splitter_position = settings.value( "splitview_splitter" ).toByteArray();

    // FIXME: store splitter position... multiples?
    //if ( !splitter_position.isNull() )

    //    ui.splitter->restoreState( splitter_position );

    // The last folders used for saving and opening files
    m_LastFolderSave    = settings.value( "lastfoldersave"  ).toString();
    m_LastFolderOpen    = settings.value( "lastfolderopen"  ).toString();

    // The list of recent files
    s_RecentFiles       = settings.value( "recentfiles" ).toStringList();

    // View Editor zoom factors
    //float zoom_factor = (float) settings.value( "codeviewzoom" ).toDouble();
    //m_wCodeView->SetZoomFactor( zoom_factor >= ZOOM_MIN ? zoom_factor : ZOOM_NORMAL );

    //zoom_factor = (float) settings.value( "bookviewzoom" ).toDouble();
    //m_wBookView->SetZoomFactor( zoom_factor >= ZOOM_MIN ? zoom_factor : ZOOM_NORMAL );
}


// Writes all the stored application settings like
// window position, geometry etc.
void MainWindow::WriteSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // The size of the window and it's full screen status
    settings.setValue( "geometry", saveGeometry() );

    // The positions of all the toolbars and dock widgets
    settings.setValue( "toolbars", saveState() );

    // The position of the splitter handle in split view
    // FIXME: splitter positions
    //settings.setValue( "splitview_splitter", ui.splitter->saveState() );

    // The last folders used for saving and opening files
    settings.setValue( "lastfoldersave",  m_LastFolderSave  );
    settings.setValue( "lastfolderopen",  m_LastFolderOpen  );   

    // The list of recent files
    settings.setValue( "recentfiles", s_RecentFiles );

    // View Editor zoom factors
    //settings.setValue( "bookviewzoom", m_wBookView->GetZoomFactor() );
    //settings.setValue( "codeviewzoom", m_wCodeView->GetZoomFactor() );
}


// Gets called on possible saves; asks the user
// does he want to save; 
// if the user chooses SAVE, we save and continue
// if the user chooses DISCARD, we don't save and continue
// if the user chooses CANCEL, we don't save and stop what we were doing
bool MainWindow::MaybeSave()
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

// Creates a new, empty book and replaces
// the current one with it
void MainWindow::CreateNew()
{
    m_Book = QSharedPointer< Book >( new Book() );
    
    m_Book->source =	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n"
                    "    \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n\n"							
                    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                    "<head>\n"
                    "<title></title>\n"
                    "</head>\n"
                    "<body>\n"
                    
                    // The "nbsp" is here so that the user starts writing
                    // inside the <p> element; if it's not here, webkit
                    // inserts text _outside_ the <p> element
                    "<p>&nbsp;</p>\n"
                    "</body>\n"
                    "</html>";
    
    // Add Sigil-specific markup
    m_Book->source = SigilMarkup::AddSigilMarkup( m_Book->source );
    
    //m_wBookView->SetBook( m_Book );    
    //m_wCodeView->SetBook( m_Book );
    
    SetCurrentFile( "" );
}


// Loads from the file specified
void MainWindow::LoadFile( const QString &filename )
{
    if ( !Utility::IsFileReadable( filename ) )

        return;

    try
    {
        QApplication::setOverrideCursor( Qt::WaitCursor );

        // Create the new book, clean up the old one
        // (destructors take care of that)
        m_Book = ImporterFactory().GetImporter( filename ).GetBook();

        // Add Sigil-specific markup to non-SGF files
        //if ( QFileInfo( filename ).suffix().toLower() != "sgf" )

        //    m_Book->source = SigilMarkup::AddSigilMarkup( m_Book->source );

        m_BookBrowser->SetBook( m_Book );

        QApplication::restoreOverrideCursor();

        SetCurrentFile( filename );

        statusBar()->showMessage( tr( "File loaded" ), STATUSBAR_MSG_DISPLAY_TIME );
    }
    
    catch ( const ExceptionBase &exception )
    {
        Utility::DisplayStdErrorDialog( "Cannot load file " + filename + ": " + Utility::GetExceptionInfo( exception ) );

        QApplication::restoreOverrideCursor();
    }
}


// Saves to the file specified
bool MainWindow::SaveFile( const QString &filename )
{
    QString extension = QFileInfo( filename ).suffix().toLower();

    // TODO: Move to ExporterFactory and throw exception
    // when the user tries to save an unsupported type
    if ( !IsSupportedSaveType( extension ) )
    {
        QMessageBox::warning(	0,
                                tr( "Sigil" ),
                                tr( "Sigil currently cannot save files of type \"%1\".\n"
                                    "Please choose a different format." )
                                .arg( extension )
                            );
        return false;
    }

    QApplication::setOverrideCursor( Qt::WaitCursor );

    //TidyUp();

    // We delete the file if it exists
    Utility::DeleteFile( filename );

    // If the file is not an SGF, 
    // we normalize the book before exporting
//     if ( extension != "sgf" )
// 
//         ExporterFactory().GetExporter( filename, BookNormalization::Normalize( m_Book ) ).WriteBook();
// 
//     else
//         
//         ExporterFactory().GetExporter( filename, m_Book ).WriteBook();

    QApplication::restoreOverrideCursor();

    SetCurrentFile( filename );

    statusBar()->showMessage( tr( "File saved" ), STATUSBAR_MSG_DISPLAY_TIME );

    return true;
}


// Returns true if the provided extension is supported as a save type
bool MainWindow::IsSupportedSaveType( const QString &extension ) const
{
    QStringList supported;

    supported << "epub" << "sgf";

    return supported.contains( extension );
}


// Performs zoom operations in the views using the default
// zoom step. Setting zoom_in to true zooms the views *in*,
// and a setting of false zooms them *out*. The zoom value
// is first wrapped to the nearest zoom step (relative to the zoom direction).
void MainWindow::ZoomByStep( bool zoom_in )
{
    // We use a negative zoom stepping if we are zooming *out*
    float zoom_stepping       = zoom_in ? ZOOM_STEP : - ZOOM_STEP;

    // If we are zooming in, we round UP;
    // on zoom out, we round DOWN.
    float rounding_helper     = zoom_in ? 0.05f : - 0.05f;

    float current_zoom_factor = m_TabManager.GetCurrentContentTab().GetZoomFactor();
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


// Sets the provided zoom factor on the active view editor.
// Valid values are between ZOOM_MAX and ZOOM_MIN, others are ignored.
void MainWindow::ZoomByFactor( float new_zoom_factor )
{
    if ( new_zoom_factor > ZOOM_MAX || new_zoom_factor < ZOOM_MIN )

        return;

    m_TabManager.GetCurrentContentTab().SetZoomFactor( new_zoom_factor );
}


// Converts a zoom factor to a value in the slider range
int MainWindow::ZoomFactorToSliderRange( float zoom_factor ) const
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


// Converts a value in the zoom slider range to a zoom factor
float MainWindow::SliderRangeToZoomFactor( int slider_range_value ) const
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


// Returns a map with keys being extensions of file types
// we can load, and the values being filters for use in file dialogs
const QMap< QString, QString > MainWindow::GetLoadFiltersMap() const
{
    QMap< QString, QString > file_filters;

    file_filters[ "sgf"   ] = tr( "Sigil Format files (*.sgf)" );
    file_filters[ "epub"  ] = tr( "EPUB files (*.epub)" );
    file_filters[ "htm"   ] = tr( "HTML files (*.htm, *.html, *.xhtml)" );
    file_filters[ "html"  ] = tr( "HTML files (*.htm, *.html, *.xhtml)" );
    file_filters[ "xhtml" ] = tr( "HTML files (*.htm, *.html, *.xhtml)" );
    file_filters[ "txt"   ] = tr( "Text files (*.txt)" );
    file_filters[ "*"     ] = tr( "All files (*.*)" );

    return file_filters;
}


// Returns a map with keys being extensions of file types
// we can save, and the values being filters for use in file dialogs
const QMap< QString, QString > MainWindow::GetSaveFiltersMap() const
{
    QMap< QString, QString > file_filters;

    file_filters[ "sgf"  ] = tr( "Sigil Format file (*.sgf)" );
    file_filters[ "epub" ] = tr( "EPUB file (*.epub)" );

    return file_filters;
}


// Sets the current file in window title;
// updates the recent files list
void MainWindow::SetCurrentFile( const QString &filename )
{
    m_CurrentFile = filename;

    setWindowModified( false );

    QString shownName;

    if ( m_CurrentFile.isEmpty() )

        shownName = "untitled.sgf";

    else
     
        shownName = QFileInfo( m_CurrentFile ).fileName();

    // Update the titlebar
    setWindowTitle( tr( "%1[*] - %2" ).arg( shownName ).arg( tr( "Sigil" ) ) );

    if ( m_CurrentFile.isEmpty() )

        return;

    // Update recent files actions
    s_RecentFiles.removeAll( filename );
    s_RecentFiles.prepend( filename );

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


// Selects the appropriate entry in the heading combo box
// based on the provided name of the element
void MainWindow::SelectEntryInHeadingCombo( const QString &element_name )
{
    QString select = "";

    if ( !element_name.isEmpty() )
    {
        if ( ( element_name[ 0 ].toLower() == QChar( 'h' ) ) && ( element_name[ 1 ].isDigit() ) )

            select = "Heading " + QString( element_name[ 1 ] );

        else

            select = "Normal";
    }

    else
    {
        select = "<Select heading>";
    }

    m_cbHeadings->setCurrentIndex( m_cbHeadings->findText( select ) );
}


// Creates and adds the recent files actions
// to the File menu
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
        ui.menuFile->insertAction( actlist[ actlist.size() - 2 ], m_RecentFileActions[ i ] );

        connect( m_RecentFileActions[ i ], SIGNAL( triggered() ), this, SLOT( OpenRecentFile() ) );
    }
}


// Updates the recent files actions when the
// list of files to be listed has changed
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

    QAction *separator = ui.menuFile->actions()[ ui.menuFile->actions().size() - 2 ];

    // If we have any actions with files shown,
    // display the separator; otherwise, don't
    if ( num_recent_files > 0 )

        separator->setVisible( true );

    else

        separator->setVisible( false );	
}


// Qt Designer is not able to create all the widgets
// we want in the MainWindow, so we use this function
// to extend the UI created by the Designer
void MainWindow::ExtendUI()
{
    // Creating the tabs and the book browser 

    // We want a nice frame around the tab manager
    QFrame *frame = new QFrame( this );
    QLayout *layout = new QVBoxLayout( frame );
    frame->setLayout( layout );
    layout->addWidget( &m_TabManager );
    layout->setContentsMargins( 0, 0, 0, 0 );

    frame->setObjectName( FRAME_NAME );
    frame->setStyleSheet( TAB_STYLE_SHEET );

    setCentralWidget( frame );       

    m_BookBrowser = new BookBrowser( this );
    addDockWidget( Qt::LeftDockWidgetArea, m_BookBrowser );

    // Creating the Heading combo box

    m_cbHeadings = new QComboBox();

    QStringList headings;
    
    headings << "<Select heading>"
             << "Normal"
             << "Heading 1"
             << "Heading 2"
             << "Heading 3"
             << "Heading 4"
             << "Heading 5"
             << "Heading 6";

    m_cbHeadings->addItems( headings );
    m_cbHeadings->setToolTip(   "<p style='padding-top: 0.5em;'><b>Style with heading</b></p>"
                                "<p style='margin-left: 0.5em;'>Style the selected text with a heading.</p>"
                                "<p style='margin-left: 0.5em;'>It is recommended to use H1 for Titles, "
                                "H2 for Authors, H3 for Chapters and H4-H6 for Subsections.</p>" 
                            );

    ui.toolBarHeadings->addWidget( m_cbHeadings );

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
    
    // We use the "close" action only on Macs,
    // because they need it for the multi-document interface
#ifndef Q_WS_MAC
    ui.actionClose->setEnabled( false );
    ui.actionClose->setVisible( false );
#else
    QList<QToolBar *> all_toolbars = findChildren<QToolBar *>();
    
    foreach( QToolBar *toolbar, all_toolbars )
    {
        toolbar->setIconSize( QSize( 32, 32 ) );
    }
#endif

    // We override the default color for highlighted text
    // so we can actually *see* the text that the FindReplace
    // dialog finds in Book View... sadly, QWebView ignores a custom
    // palette set on it directly, so we have to do this globally.
    QPalette palette;
    palette.setColor( QPalette::Inactive, QPalette::Highlight, Qt::darkGreen );
    palette.setColor( QPalette::Inactive, QPalette::HighlightedText, Qt::white );
    qApp->setPalette( palette );
}


// If a file was provided to be loaded
// with this main window instance, that file is loaded;
// if not, or it can't be opened, an empty file is loaded
void MainWindow::LoadInitialFile( const QString &openfilepath )
{
    if ( !openfilepath.isEmpty() )
    {
        LoadFile( openfilepath );
    }

    else
    {
        CreateNew();
    }
}


// Connects all the required signals to their slots
void MainWindow::ConnectSignalsToSlots()
{
    connect( ui.actionExit,                 SIGNAL( triggered() ),      qApp,   SLOT( closeAllWindows()     ) );
    connect( ui.actionClose,                SIGNAL( triggered() ),      this,   SLOT( close()               ) );
    connect( ui.actionNew,                  SIGNAL( triggered() ),      this,   SLOT( New()                 ) );
    connect( ui.actionOpen,                 SIGNAL( triggered() ),      this,   SLOT( Open()                ) );
    connect( ui.actionSave,                 SIGNAL( triggered() ),      this,   SLOT( Save()                ) );
    connect( ui.actionSaveAs,               SIGNAL( triggered() ),      this,   SLOT( SaveAs()              ) );
    connect( ui.actionFind,                 SIGNAL( triggered() ),      this,   SLOT( Find()                ) );
    connect( ui.actionReplace,              SIGNAL( triggered() ),      this,   SLOT( Replace()             ) );
    connect( ui.actionZoomIn,               SIGNAL( triggered() ),      this,   SLOT( ZoomIn()              ) );
    connect( ui.actionZoomOut,              SIGNAL( triggered() ),      this,   SLOT( ZoomOut()             ) );  
    connect( ui.actionMetaEditor,           SIGNAL( triggered() ),      this,   SLOT( MetaEditorDialog()    ) );
    connect( ui.actionTOCEditor,            SIGNAL( triggered() ),      this,   SLOT( TOCEditorDialog()     ) );
    connect( ui.actionReportAnIssue,        SIGNAL( triggered() ),      this,   SLOT( ReportAnIssue()       ) );
    connect( ui.actionSigilDevBlog,         SIGNAL( triggered() ),      this,   SLOT( SigilDevBlog()        ) );
    connect( ui.actionAbout,                SIGNAL( triggered() ),      this,   SLOT( AboutDialog()         ) );
    
    connect( &m_TabManager,                 SIGNAL( TabChanged( ContentTab*, ContentTab* ) ), this, SLOT( TabChanged( ContentTab*, ContentTab* ) ) );
    connect( &m_TabManager,                 SIGNAL( TabChanged( ContentTab*, ContentTab* ) ), this, SLOT( UpdateUI() ) );
    connect( &m_TabManager,                 SIGNAL( TabChanged( ContentTab*, ContentTab* ) ), this, SLOT( TabSwitchChanges() ) );

    connect( m_slZoomSlider,                SIGNAL( valueChanged( int ) ),          this,   SLOT( SliderZoom( int ) ) );

    // We also update the label when the slider moves... this is to show
    // the zoom value the slider will land on while it is being moved.
    connect( m_slZoomSlider,                SIGNAL( sliderMoved( int ) ),           this,   SLOT( UpdateZoomLabel( int ) ) );

    connect( m_BookBrowser, SIGNAL( ResourceDoubleClicked( Resource& ) ),
             &m_TabManager, SLOT(   OpenResource(          Resource& ) ) );

    connect( m_BookBrowser, SIGNAL( OpenResourceRequest( Resource&, const QUrl& ) ),
            &m_TabManager,  SLOT(   OpenResource(        Resource&, const QUrl& ) ) );
    
    connect( &m_TabManager, SIGNAL( OpenUrlRequest( const QUrl& ) ),
             m_BookBrowser, SLOT(  OpenUrlResource( const QUrl& ) ) );
}

void MainWindow::MakeTabConnections( ContentTab *tab )
{
    if ( tab == NULL )

        return;

    connect( ui.actionUndo,                 SIGNAL( triggered() ),  tab,   SLOT( Undo()                ) );
    connect( ui.actionRedo,                 SIGNAL( triggered() ),  tab,   SLOT( Redo()                ) );
    connect( ui.actionCut,                  SIGNAL( triggered() ),  tab,   SLOT( Cut()                 ) );
    connect( ui.actionCopy,                 SIGNAL( triggered() ),  tab,   SLOT( Copy()                ) );
    connect( ui.actionPaste,                SIGNAL( triggered() ),  tab,   SLOT( Paste()               ) );
    connect( ui.actionBold,                 SIGNAL( triggered() ),  tab,   SLOT( Bold()                ) );
    connect( ui.actionItalic,               SIGNAL( triggered() ),  tab,   SLOT( Italic()              ) );
    connect( ui.actionUnderline,            SIGNAL( triggered() ),  tab,   SLOT( Underline()           ) );
    connect( ui.actionStrikethrough,        SIGNAL( triggered() ),  tab,   SLOT( Strikethrough()       ) );
    connect( ui.actionAlignLeft,            SIGNAL( triggered() ),  tab,   SLOT( AlignLeft()           ) );
    connect( ui.actionCenter,               SIGNAL( triggered() ),  tab,   SLOT( Center()              ) );
    connect( ui.actionAlignRight,           SIGNAL( triggered() ),  tab,   SLOT( AlignRight()          ) );
    connect( ui.actionJustify,              SIGNAL( triggered() ),  tab,   SLOT( Justify()             ) );
    connect( ui.actionInsertChapterBreak,   SIGNAL( triggered() ),  tab,   SLOT( InsertChapterBreak()  ) );
    connect( ui.actionInsertImage,          SIGNAL( triggered() ),  tab,   SLOT( InsertImage()         ) );
    connect( ui.actionInsertBulletedList,   SIGNAL( triggered() ),  tab,   SLOT( InsertBulletedList()  ) );
    connect( ui.actionInsertNumberedList,   SIGNAL( triggered() ),  tab,   SLOT( InsertNumberedList()  ) );
    connect( ui.actionDecreaseIndent,       SIGNAL( triggered() ),  tab,   SLOT( DecreaseIndent()      ) );
    connect( ui.actionIncreaseIndent,       SIGNAL( triggered() ),  tab,   SLOT( IncreaseIndent()      ) );
    connect( ui.actionRemoveFormatting,     SIGNAL( triggered() ),  tab,   SLOT( RemoveFormatting()    ) );

    connect( ui.actionPrintPreview,         SIGNAL( triggered() ),  tab,   SLOT( PrintPreview()        ) );
    connect( ui.actionPrint,                SIGNAL( triggered() ),  tab,   SLOT( Print()               ) );

    connect( ui.actionBookView,             SIGNAL( triggered() ),  tab,   SLOT( BookView()            ) );
    connect( ui.actionSplitView,            SIGNAL( triggered() ),  tab,   SLOT( SplitView()           ) );
    connect( ui.actionCodeView,             SIGNAL( triggered() ),  tab,   SLOT( CodeView()            ) );   

    connect( m_cbHeadings,                  SIGNAL( activated( const QString& ) ),  tab,   SLOT( HeadingStyle( const QString& ) ) );

    connect( tab,   SIGNAL( ViewChanged() ),                this,   SLOT( UpdateUI()                ) );
    connect( tab,   SIGNAL( SelectionChanged() ),           this,   SLOT( UpdateUI()                ) );
    connect( tab,   SIGNAL( EnteringBookView() ),           this,   SLOT( SetStateActionsBookView() ) );
    connect( tab,   SIGNAL( EnteringCodeView() ),           this,   SLOT( SetStateActionsCodeView() ) );
    connect( tab,   SIGNAL( EnteringBookView() ),           this,   SLOT( UpdateZoomControls()      ) );
    connect( tab,   SIGNAL( EnteringCodeView() ),           this,   SLOT( UpdateZoomControls()      ) );
    connect( tab,   SIGNAL( ContentChanged() ),             this,   SLOT( DocumentWasModified()     ) );
    connect( tab,   SIGNAL( ZoomFactorChanged( float ) ),   this,   SLOT( UpdateZoomLabel( float )  ) );
    connect( tab,   SIGNAL( ZoomFactorChanged( float ) ),   this,   SLOT( UpdateZoomSlider( float ) ) );
}

void MainWindow::BreakTabConnections( ContentTab *tab )
{
    if ( tab == NULL )

        return;

    disconnect( ui.actionUndo,                0, tab, 0 );
    disconnect( ui.actionRedo,                0, tab, 0 );
    disconnect( ui.actionCut,                 0, tab, 0 );
    disconnect( ui.actionCopy,                0, tab, 0 );
    disconnect( ui.actionPaste,               0, tab, 0 );
    disconnect( ui.actionBold,                0, tab, 0 );
    disconnect( ui.actionItalic,              0, tab, 0 );
    disconnect( ui.actionUnderline,           0, tab, 0 );
    disconnect( ui.actionStrikethrough,       0, tab, 0 );
    disconnect( ui.actionAlignLeft,           0, tab, 0 );
    disconnect( ui.actionCenter,              0, tab, 0 );
    disconnect( ui.actionAlignRight,          0, tab, 0 );
    disconnect( ui.actionJustify,             0, tab, 0 );
    disconnect( ui.actionInsertChapterBreak,  0, tab, 0 );
    disconnect( ui.actionInsertImage,         0, tab, 0 );
    disconnect( ui.actionInsertBulletedList,  0, tab, 0 );
    disconnect( ui.actionInsertNumberedList,  0, tab, 0 );
    disconnect( ui.actionDecreaseIndent,      0, tab, 0 );
    disconnect( ui.actionIncreaseIndent,      0, tab, 0 );
    disconnect( ui.actionRemoveFormatting,    0, tab, 0 );

    disconnect( ui.actionPrintPreview,        0, tab, 0 );
    disconnect( ui.actionPrint,               0, tab, 0 );

    disconnect( ui.actionBookView,            0, tab, 0 );
    disconnect( ui.actionSplitView,           0, tab, 0 );
    disconnect( ui.actionCodeView,            0, tab, 0 );   

    disconnect( m_cbHeadings,                 0, tab, 0 );

    disconnect( tab,                          0, this, 0 );
}







