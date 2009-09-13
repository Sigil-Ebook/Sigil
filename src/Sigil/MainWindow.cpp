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

#include "stdafx.h"
#include "Utility.h"
#include "MainWindow.h"
#include "CleanSource.h"
#include "FolderKeeper.h"
#include "ExportEPUB.h"
#include "ExportSGF.h"
#include "XHTMLHighlighter.h"
#include "MetaEditor.h"
#include "About.h"
#include "TOCEditor.h"
#include "ImporterFactory.h"
#include "ExporterFactory.h"
#include "BookNormalization.h"
#include "SigilMarkup.h"
#include "CodeViewEditor.h"
#include "BookViewEditor.h"

static const int STATUSBAR_MSG_DISPLAY_TIME = 2000;
static const int TAB_SPACES_WIDTH           = 4;
static const int TEXT_ELIDE_WIDTH           = 300;
static const QString SETTINGS_GROUP         = "mainwindow";

// The <hr> tag is wrapped in <div>'s because of issue #78;
// basically it's a workaround for a webkit bug
const QString BREAK_TAG_INSERT              = "<div><hr class=\"sigilChapterBreak\" /></div>";

QStringList MainWindow::m_RecentFiles = QStringList();

// Constructor
// The first argument is the path to the file that the window
// should load (new file loaded if empty); the second is the
// windows parent; the third specifies the flags used to modify window behaviour
MainWindow::MainWindow( const QString &openfilepath, QWidget *parent, Qt::WFlags flags )
    : QMainWindow( parent, flags )
{
    ui.setupUi( this );	
	
    // Telling Qt to delete this window
    // from memory when it is closed
	setAttribute( Qt::WA_DeleteOnClose );

    ExtendUI();

    ConnectSignalsToSlots();

    ReadSettings();

    CreateRecentFilesActions();
    UpdateRecentFileActions();

    SetUpCodeView();

    ui.actionBookView->trigger();

    m_isLastViewBook = true;

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
        QString filters = tr(   "Sigil Format files (*.sgf);;"
                                "EPUB files (*.epub);;"
                                "HTML files (*.htm, *.html, *.xhtml);;"
                                "Text files (*.txt);;"
                                "All files (*.*)" 
                            );

        QString default_filter = tr( "All files (*.*)" );

        QString filename = QFileDialog::getOpenFileName(    this,
                                                            tr( "Open File" ),
                                                            m_LastFolderOpen,
                                                            filters,
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
    QString filters = tr( "Sigil Format file (*.sgf);;EPUB file (*.epub)" );

    QString default_filter = tr( "Sigil Format file (*.sgf)" );

    QString filename = QFileDialog::getSaveFileName(    this, 
                                                        tr( "Save File" ), 
                                                        m_LastFolderSave,
                                                        filters,
                                                        &default_filter
                                                   );

    if ( filename.isEmpty() )

        return false;

    // Store the folder the user saved to
    m_LastFolderSave = QFileInfo( filename ).absolutePath();

    return SaveFile( filename );
}


// Implements Undo action functionality
void MainWindow::Undo()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->page()->triggerAction( QWebPage::Undo );

        RemoveAppleClasses();
    }

    else if ( m_wCodeView->hasFocus() )
    {
        m_wCodeView->undo();
    }
}


// Implements Redo action functionality
void MainWindow::Redo()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->page()->triggerAction( QWebPage::Redo );

        RemoveAppleClasses();
    }

    else if ( m_wCodeView->hasFocus() )
    {
        m_wCodeView->redo();
    }
}


// Implements Cut action functionality
void MainWindow::Cut()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->page()->triggerAction( QWebPage::Cut );

        RemoveAppleClasses();
    }

    else if ( m_wCodeView->hasFocus() )
    {
        m_wCodeView->cut();
    }
}


// Implements Copy action functionality
void MainWindow::Copy()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->page()->triggerAction( QWebPage::Copy );

        RemoveAppleClasses();
    }

    else if ( m_wCodeView->hasFocus() )
    {
        m_wCodeView->copy();
    }
}


// Implements Paste action functionality
void MainWindow::Paste()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->page()->triggerAction( QWebPage::Paste );

        RemoveAppleClasses();
    }

    else if ( m_wCodeView->hasFocus() )
    {
        m_wCodeView->paste();
    }
}


// Implements Bold action functionality
void MainWindow::Bold()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->page()->triggerAction( QWebPage::ToggleBold );

        RemoveAppleClasses();
    }

    // TODO: insert required HTML for Code View
}


// Implements Italic action functionality
void MainWindow::Italic()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->page()->triggerAction( QWebPage::ToggleItalic );

        RemoveAppleClasses();
    }

    // TODO: insert required HTML for Code View
}


// Implements Underline action functionality
void MainWindow::Underline()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->page()->triggerAction( QWebPage::ToggleUnderline );

        RemoveAppleClasses();
    }

    // TODO: insert required HTML for Code View
}


// Implements Strikethrough action functionality
void MainWindow::Strikethrough()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->ExecCommand( "strikeThrough" );

        RemoveAppleClasses();
    }

    // TODO: insert required HTML for Code View
}


// Implements Align Left action functionality
void MainWindow::AlignLeft()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->ExecCommand( "justifyLeft" );

        RemoveAppleClasses();
    }
    
    // TODO: insert required HTML for Code View
}


// Implements Center action functionality
void MainWindow::Center()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->ExecCommand( "justifyCenter" );

        RemoveAppleClasses();
    }

    // TODO: insert required HTML for Code View
}


// Implements Align Right action functionality
void MainWindow::AlignRight()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->ExecCommand( "justifyRight" );

        RemoveAppleClasses();
    }

    // TODO: insert required HTML for Code View
}


// Implements Justify action functionality
void MainWindow::Justify()
{
    if ( m_wBookView->hasFocus() )
    {
        m_wBookView->ExecCommand( "justifyFull" );

        RemoveAppleClasses();
    }

    // TODO: insert required HTML for Code View
}


// Implements Book View action functionality
void MainWindow::BookView()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );
    
    // Update the book view if we just edited
    // in the code view
    if ( !m_isLastViewBook )
    {
        UpdateBookViewFromSource();

        m_wBookView->StoreCaretLocationUpdate( m_wCodeView->GetCaretLocation() );
    }
    
    m_wBookView->show();
    m_wCodeView->hide();	

    // Update the "toggle" button states
    ui.actionBookView->setChecked(  true    );
    ui.actionSplitView->setChecked( false   );
    ui.actionCodeView->setChecked(  false   );

    // Set initial state for actions in this view
    SetStateActionsBookView();

    m_isLastViewBook = true;
    
    QApplication::restoreOverrideCursor();
}


// Implements Split View action functionality
void MainWindow::SplitView()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );

    // Update the required view
    if ( !m_isLastViewBook )
    {
        UpdateBookViewFromSource();

        m_wBookView->StoreCaretLocationUpdate( m_wCodeView->GetCaretLocation() );
    }

    else
    {
        UpdateCodeViewFromSource();

        m_wCodeView->StoreCaretLocationUpdate( m_wBookView->GetCaretLocation() );
    }

    m_wBookView->show();
    m_wCodeView->show();

    // Update the "toggle" button states
    ui.actionBookView->setChecked(  false   );
    ui.actionSplitView->setChecked( true    );
    ui.actionCodeView->setChecked(  false   );  

    QApplication::restoreOverrideCursor();
}


// Implements Code View action functionality
void MainWindow::CodeView()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );

    // Update the code view if we just edited
    // in the book view
    if ( m_isLastViewBook )
    {
        UpdateCodeViewFromSource();

        m_wCodeView->StoreCaretLocationUpdate( m_wBookView->GetCaretLocation() );
    }

    m_wBookView->hide();
    m_wCodeView->show();     

    // Update the "toggle" button states
    ui.actionBookView->setChecked(  false   );
    ui.actionSplitView->setChecked( false   );
    ui.actionCodeView->setChecked(  true    );  

    // Set initial state for actions in this view
    SetStateActionsCodeView();  

    m_isLastViewBook = false;

    QApplication::restoreOverrideCursor();
}


// Implements Insert chapter break action functionality
void MainWindow::InsertChapterBreak()
{
    m_wBookView->ExecCommand( "insertHTML", BREAK_TAG_INSERT );

    RemoveAppleClasses();
}


// Implements Insert image action functionality
void MainWindow::InsertImage()
{
    QString filename = QFileDialog::getOpenFileName(    this, 
                                                        tr( "Insert Image" ), 
                                                        m_LastFolderImage, 
                                                        tr( "Images (*.png *.jpg *.jpeg *.gif *.svg)")
                                                   );

    if ( filename.isEmpty() )

        return;

    // Store the folder the user inserted the image from
    m_LastFolderImage = QFileInfo( filename ).absolutePath();

    QString relative_path = "../" + m_Book.mainfolder.AddContentFileToFolder( filename );

    m_wBookView->ExecCommand( "insertImage", relative_path );

    RemoveAppleClasses();
}


// Implements Insert bulleted list action functionality
void MainWindow::InsertBulletedList()
{
    m_wBookView->ExecCommand( "insertUnorderedList" );

    RemoveAppleClasses();
}


// Implements Insert numbered list action functionality
void MainWindow::InsertNumberedList()
{
    m_wBookView->ExecCommand( "insertOrderedList" );

    RemoveAppleClasses();
}


// Implements the heading combo box functionality
void MainWindow::HeadingStyle( const QString& heading_type )
{
    QChar last_char = heading_type[ heading_type.count() - 1 ];

    // For heading_type == "Heading #"
    if ( last_char.isDigit() )

        m_wBookView->ExecCommand( "formatBlock", "H" + QString( last_char ) );

    else if ( heading_type == "Normal" )

        m_wBookView->ExecCommand( "formatBlock", "P" );

    // else is "<Select heading>" which does nothing
}

// Implements Print Preview action functionality
void MainWindow::PrintPreview()
{
    if ( !m_wBookView->hasFocus() && !m_wCodeView->hasFocus() )

        return;

    QPrintPreviewDialog *print_preview = new QPrintPreviewDialog( this );

    if ( m_isLastViewBook )
    {
        connect(    print_preview,     SIGNAL( paintRequested( QPrinter * ) ),
                    m_wBookView,       SLOT(   print( QPrinter *) ) 
                );
    }

    else
    {
        connect(    print_preview,     SIGNAL( paintRequested( QPrinter * ) ),
                    m_wCodeView,       SLOT(   print( QPrinter *) ) 
               );
    }        
    
    print_preview->exec();
}


// Implements Print action functionality
void MainWindow::Print()
{
    if ( !m_wBookView->hasFocus() && !m_wCodeView->hasFocus() )

        return;

    QPrinter printer;

    QPrintDialog *print_dialog = new QPrintDialog( &printer, this );
    print_dialog->setWindowTitle( tr( "Print Document" ) );

    if ( print_dialog->exec() == QDialog::Accepted )
    {
        if ( m_isLastViewBook )

            m_wBookView->print( &printer );

        else

            m_wCodeView->print( &printer );
    }
}


// Implements Meta Editor action functionality
void MainWindow::MetaEditorDialog()
{
    // ALWAYS clean up source first before
    // using m_Book outside of MainWindow!
    TidyUp();

    MetaEditor meta( m_Book, this );

    meta.exec();
}


// Implements TOC Preview action functionality
void MainWindow::TOCEditorDialog()
{
    // ALWAYS clean up source first before
    // using m_Book outside of MainWindow!
    TidyUp();

    TOCEditor toc( m_Book, this );

    if ( toc.exec() == QDialog::Accepted )
    {
        UpdateBookViewFromSource();
        UpdateCodeViewFromSource();
    }
}


// Implements About action functionality
void MainWindow::AboutDialog()
{
    About about( this );

    about.exec();
}


// Used to catch the focus changeover from one widget
// (code or book view) to the other; needed for source synchronization.
void MainWindow::FocusFilter( QWidget *old_widget, QWidget *new_widget )
{
    // We make sure we are looking at focus changes
    // in Split View; otherwise, we don't care
    if ( !ui.actionSplitView->isChecked() )
    
        return;

    // If we switched focus from the book view to the code view...
    if ( ( old_widget == m_wBookView ) && ( new_widget == m_wCodeView ) )
    {        
        // ...and if we haven't updated yet...
        if ( m_OldSource != m_Book.source )
        {
            QApplication::setOverrideCursor( Qt::WaitCursor );

            // ...update the code view
            UpdateCodeViewFromSource();            

            QApplication::restoreOverrideCursor();
        }

        m_wCodeView->StoreCaretLocationUpdate( m_wBookView->GetCaretLocation() );

        m_isLastViewBook = false;

        // Set initial state for actions in this view
        SetStateActionsCodeView();      
    }

    // If we switched focus from the code view to the book view...
    else if ( ( old_widget == m_wCodeView ) && ( new_widget == m_wBookView ) )
    {
        // ...and if we haven't updated yet...
        if ( m_OldSource != m_Book.source )
        {
            QApplication::setOverrideCursor( Qt::WaitCursor );

            // ...update the book view
            UpdateBookViewFromSource();

            QApplication::restoreOverrideCursor();
        }

        m_wBookView->StoreCaretLocationUpdate( m_wCodeView->GetCaretLocation() );

        m_isLastViewBook = true;

        // Set initial state for actions in this view
        SetStateActionsBookView();
    }

    // else we don't care
}


// Gets called every time the document is modified;
// changes the UI to accordingly;
// (star in titlebar on win and lin, different button colors on mac)
void MainWindow::DocumentWasModified()
{
    // TODO: This won't work right until we unify the undo stacks
    if ( m_wBookView->isModified() || m_wCodeView->document()->isModified() )

        setWindowModified( true );

    else

        setWindowModified( false );
}


// Updates action states based on 
// the current selection in Book View
void MainWindow::UpdateUIBookView()
{
    // TODO: the undo/redo actions are returning false
    // when they are actually enabled... strange, to say the least...
    //ui.actionUndo  ->setEnabled( wBookView->pageAction( QWebPage::Undo  )->isEnabled() );
    //ui.actionRedo  ->setEnabled( wBookView->pageAction( QWebPage::Redo  )->isEnabled() );
    ui.actionCut   ->setEnabled( m_wBookView->pageAction( QWebPage::Cut   )->isEnabled() );
    ui.actionCopy  ->setEnabled( m_wBookView->pageAction( QWebPage::Copy  )->isEnabled() );
    ui.actionPaste ->setEnabled( m_wBookView->pageAction( QWebPage::Paste )->isEnabled() );

    ui.actionBold      ->setChecked( m_wBookView->pageAction( QWebPage::ToggleBold      )->isChecked() );
    ui.actionItalic    ->setChecked( m_wBookView->pageAction( QWebPage::ToggleItalic    )->isChecked() );
    ui.actionUnderline ->setChecked( m_wBookView->pageAction( QWebPage::ToggleUnderline )->isChecked() );
    
    ui.actionStrikethrough      ->setChecked( m_wBookView->QueryCommandState( "strikeThrough"       ) );
    ui.actionInsertBulletedList ->setChecked( m_wBookView->QueryCommandState( "insertUnorderedList" ) );
    ui.actionInsertNumberedList ->setChecked( m_wBookView->QueryCommandState( "insertOrderedList"   ) );

    SelectEntryInHeadingCombo( m_wBookView->GetCaretElementName() );
}


// Updates action states based on 
// the current selection in Code View
void MainWindow::UpdateUICodeView()
{
    // TODO: these are turned off to be consistent with the book view
    //ui.actionUndo  ->setEnabled( wCodeView->document()->isUndoAvailable() );
    //ui.actionRedo  ->setEnabled( wCodeView->document()->isRedoAvailable() );
    ui.actionCut   ->setEnabled( m_wCodeView->textCursor().hasSelection() );
    ui.actionCopy  ->setEnabled( m_wCodeView->textCursor().hasSelection() );
    ui.actionPaste ->setEnabled( m_wCodeView->canPaste() );
}


// Set initial state for actions in Book View
// (enable the actions the Code View disabled)
void MainWindow::SetStateActionsBookView()
{
    ui.actionBold          ->setEnabled( true );
    ui.actionItalic        ->setEnabled( true );
    ui.actionUnderline     ->setEnabled( true );
    ui.actionStrikethrough ->setEnabled( true );

    ui.actionAlignLeft  ->setEnabled( true );
    ui.actionCenter     ->setEnabled( true );
    ui.actionAlignRight ->setEnabled( true );
    ui.actionJustify    ->setEnabled( true );

    ui.actionInsertImage        ->setEnabled( true );
    ui.actionInsertChapterBreak ->setEnabled( true );

    ui.actionInsertBulletedList ->setEnabled( true );
    ui.actionInsertNumberedList ->setEnabled( true );

    m_cbHeadings->setEnabled( true );
}


// Set initial state for actions in Code View
// (disable the actions used in Book View that
// are not appropriate here)
void MainWindow::SetStateActionsCodeView()
{
    // Book View might have disabled some of these
    // depending on the user's selection
    ui.actionUndo  ->setEnabled( true );
    ui.actionRedo  ->setEnabled( true );

    ui.actionCut   ->setEnabled( true );
    ui.actionCopy  ->setEnabled( true );
    ui.actionPaste ->setEnabled( true );

    // TODO: We shouldn't really disable these.
    // The Code View should insert correct HTML
    // when these are triggered

    ui.actionBold          ->setEnabled( false );
    ui.actionItalic        ->setEnabled( false );
    ui.actionUnderline     ->setEnabled( false );
    ui.actionStrikethrough ->setEnabled( false );

    ui.actionAlignLeft  ->setEnabled( false );
    ui.actionCenter     ->setEnabled( false );
    ui.actionAlignRight ->setEnabled( false );
    ui.actionJustify    ->setEnabled( false );

    ui.actionInsertImage        ->setEnabled( false );
    ui.actionInsertChapterBreak ->setEnabled( false );
    
    ui.actionInsertBulletedList ->setEnabled( false );
    ui.actionInsertNumberedList ->setEnabled( false );

    m_cbHeadings->setEnabled( false );

    ui.actionBold      ->setChecked( false );
    ui.actionItalic    ->setChecked( false );
    ui.actionUnderline ->setChecked( false );

    ui.actionStrikethrough      ->setChecked( false );
    ui.actionInsertBulletedList ->setChecked( false );
    ui.actionInsertNumberedList ->setChecked( false );
}


// Updates the m_Book.source variable whenever
// the user edits in book view
void MainWindow::UpdateSourceFromBookView()
{
    m_Book.source = m_wBookView->page()->mainFrame()->toHtml();
}


// Updates the m_Book.source variable whenever
// the user edits in code view
void MainWindow::UpdateSourceFromCodeView()
{
    m_Book.source = m_wCodeView->toPlainText();	
}


// On changeover, updates the code in code view
void MainWindow::UpdateCodeViewFromSource()
{
    TidyUp();

    m_wCodeView->SetBook( m_Book );

    // Store current source so we can compare and check
    // if we updated yet or we haven't
    m_OldSource = m_Book.source;
}


// On changeover, updates the code in book view
void MainWindow::UpdateBookViewFromSource()
{
    TidyUp();

    m_wBookView->SetBook( m_Book );

    // Store current source so we can compare and check
    // if we updated yet or we haven't
    m_OldSource = m_Book.source;
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

    if ( !splitter_position.isNull() )

        ui.splitter->restoreState( splitter_position );

    // The last folders used for saving and opening files
    m_LastFolderSave    = settings.value( "lastfoldersave"  ).toString();
    m_LastFolderOpen    = settings.value( "lastfolderopen"  ).toString();
    m_LastFolderImage   = settings.value( "lastfolderimage" ).toString();

    // The list of recent files
    m_RecentFiles    = settings.value( "recentfiles" ).toStringList();
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
    settings.setValue( "splitview_splitter", ui.splitter->saveState() );

    // The last folders used for saving and opening files
    settings.setValue( "lastfoldersave",  m_LastFolderSave  );
    settings.setValue( "lastfolderopen",  m_LastFolderOpen  );
    settings.setValue( "lastfolderimage", m_LastFolderImage );

    // The list of recent files
    settings.setValue( "recentfiles", m_RecentFiles );
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
    m_Book = Book();
    
    m_Book.source =	"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
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
    m_Book.source = SigilMarkup::AddSigilMarkup( m_Book.source );
    
    m_wBookView->SetBook( m_Book );    
    m_wCodeView->SetBook( m_Book );
    
    SetCurrentFile( "" );
}


// Loads from the file specified
void MainWindow::LoadFile( const QString &filename )
{
    if ( !Utility::IsFileReadable( filename ) )

        return;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    // Create the new book, clean up the old one
    // (destructors take care of that)
    m_Book = ImporterFactory().GetImporter( filename ).GetBook();

    // Add Sigil-specific markup to non-SGF files
    if ( QFileInfo( filename ).suffix().toLower() != "sgf" )
    
        m_Book.source = SigilMarkup::AddSigilMarkup( m_Book.source );

    m_wBookView->SetBook( m_Book );
    m_wCodeView->SetBook( m_Book );
   
    QApplication::restoreOverrideCursor();

    SetCurrentFile( filename );

    statusBar()->showMessage( tr( "File loaded" ), STATUSBAR_MSG_DISPLAY_TIME );
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

    TidyUp();

    // If the file is not an SGF, 
    // we normalize the book before exporting
    if ( extension != "sgf" )

        ExporterFactory().GetExporter( filename, BookNormalization::Normalize( m_Book ) ).WriteBook();

    else
        
        ExporterFactory().GetExporter( filename, m_Book ).WriteBook();

    QApplication::restoreOverrideCursor();

    SetCurrentFile( filename );

    statusBar()->showMessage( tr( "File saved" ), STATUSBAR_MSG_DISPLAY_TIME );

    return true;
}

// Returns true if the provided extension is supported as a save type
bool MainWindow::IsSupportedSaveType( const QString &extension )
{
    QStringList supported;

    supported << "epub" << "sgf";

    return supported.contains( extension );
}


// Runs HTML Tidy on m_Book.source variable
void MainWindow::TidyUp()
{
    RemoveAppleClasses();

    m_Book.source = CleanSource::Clean( m_Book.source );
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
    m_RecentFiles.removeAll( filename );
    m_RecentFiles.prepend( filename );

    while ( m_RecentFiles.size() > MAX_RECENT_FILES )
    {
        m_RecentFiles.removeLast();
    }
    
    // Update the recent files actions on
    // ALL the main windows
    foreach ( QWidget *window, QApplication::topLevelWidgets() ) 
    {
        if ( MainWindow *mainWin = qobject_cast< MainWindow * >( window ) )
            
            mainWin->UpdateRecentFileActions();
    }
}

// Removes every occurrence of class="Apple-style-span"
// with which webkit litters our source code 
void MainWindow::RemoveAppleClasses()
{
    m_Book.source.replace( QRegExp( "(class=\"[^\"]*)Apple-style-span" ), "\\1" );
}

// Selects the appropriate entry in the heading combo box
// based on the provided name of the element
void MainWindow::SelectEntryInHeadingCombo( const QString &element_name )
{
    QString select = "";

    if ( !element_name.isEmpty() )
    {
        if ( ( element_name[ 0 ] == QChar( 'H' ) ) && ( element_name[ 1 ].isDigit() ) )

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


// Initializes the code view
void MainWindow::SetUpCodeView()
{
    // Let's try to use Consolas as our font
    QFont font( "Consolas", 10 );

    // But just in case, say we want a fixed width font
    // if Consolas is not on the system
    font.setStyleHint( QFont::TypeWriter );

    m_wCodeView->setFont( font );
    m_wCodeView->setTabStopWidth( TAB_SPACES_WIDTH * QFontMetrics( font ).width( ' ' ) );

    m_Highlighter = new XHTMLHighlighter( m_wCodeView->document() );
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
    int num_recent_files = qMin( m_RecentFiles.size(), MAX_RECENT_FILES );

    // Store the filenames to the actions and display those actions
    for ( int i = 0; i < num_recent_files; ++i ) 
    {
        QString text = tr( "&%1 %2" ).arg( i + 1 ).arg( QFileInfo( m_RecentFiles[ i ] ).fileName() );

        m_RecentFileActions[ i ]->setText( fontMetrics().elidedText( text, Qt::ElideRight, TEXT_ELIDE_WIDTH ) );
        m_RecentFileActions[ i ]->setData( m_RecentFiles[ i ] );
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
    m_cbHeadings->setToolTip(   "Style the selected text with a heading.\n\n"
                                "It is recommended to use H1 for Titles, H2 for Authors,\n"
                                "H3 for Chapters and H4-H6 for Subsections." 
                            );

    ui.toolBarHeadings->addWidget( m_cbHeadings );

    m_wBookView = new BookViewEditor( ui.splitter );
    ui.splitter->addWidget( m_wBookView );

    m_wCodeView = new CodeViewEditor( ui.splitter );
    ui.splitter->addWidget( m_wCodeView );
    
    // We use the "close" action only on Macs,
    // because they need it for the multi-document interface
#ifndef Q_WS_MAC
    ui.actionClose->setEnabled( false );
    ui.actionClose->setVisible( false );
#endif
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
    connect( ui.actionUndo,                 SIGNAL( triggered() ),      this,   SLOT( Undo()                ) );
    connect( ui.actionRedo,                 SIGNAL( triggered() ),      this,   SLOT( Redo()                ) );
    connect( ui.actionCut,                  SIGNAL( triggered() ),      this,   SLOT( Cut()                 ) );
    connect( ui.actionCopy,                 SIGNAL( triggered() ),      this,   SLOT( Copy()                ) );
    connect( ui.actionPaste,                SIGNAL( triggered() ),      this,   SLOT( Paste()               ) );
    connect( ui.actionBold,                 SIGNAL( triggered() ),      this,   SLOT( Bold()                ) );
    connect( ui.actionItalic,               SIGNAL( triggered() ),      this,   SLOT( Italic()              ) );
    connect( ui.actionUnderline,            SIGNAL( triggered() ),      this,   SLOT( Underline()           ) );
    connect( ui.actionStrikethrough,        SIGNAL( triggered() ),      this,   SLOT( Strikethrough()       ) );
    connect( ui.actionAlignLeft,            SIGNAL( triggered() ),      this,   SLOT( AlignLeft()           ) );
    connect( ui.actionCenter,               SIGNAL( triggered() ),      this,   SLOT( Center()              ) );
    connect( ui.actionAlignRight,           SIGNAL( triggered() ),      this,   SLOT( AlignRight()          ) );
    connect( ui.actionJustify,              SIGNAL( triggered() ),      this,   SLOT( Justify()             ) );
    connect( ui.actionBookView,             SIGNAL( triggered() ),      this,   SLOT( BookView()            ) );
    connect( ui.actionSplitView,            SIGNAL( triggered() ),      this,   SLOT( SplitView()           ) );
    connect( ui.actionCodeView,             SIGNAL( triggered() ),      this,   SLOT( CodeView()            ) );
    connect( ui.actionInsertChapterBreak,   SIGNAL( triggered() ),      this,   SLOT( InsertChapterBreak()  ) );
    connect( ui.actionInsertImage,          SIGNAL( triggered() ),      this,   SLOT( InsertImage()         ) );
    connect( ui.actionInsertBulletedList,   SIGNAL( triggered() ),      this,   SLOT( InsertBulletedList()  ) );
    connect( ui.actionInsertNumberedList,   SIGNAL( triggered() ),      this,   SLOT( InsertNumberedList()  ) );
    connect( ui.actionPrintPreview,         SIGNAL( triggered() ),      this,   SLOT( PrintPreview()        ) );
    connect( ui.actionPrint,                SIGNAL( triggered() ),      this,   SLOT( Print()               ) );
    
    connect( ui.actionMetaEditor,           SIGNAL( triggered() ),      this,   SLOT( MetaEditorDialog()    ) );
    connect( ui.actionTOCEditor,            SIGNAL( triggered() ),      this,   SLOT( TOCEditorDialog()     ) );
    connect( ui.actionAbout,                SIGNAL( triggered() ),      this,   SLOT( AboutDialog()         ) );
    
    connect( m_wBookView->page(),           SIGNAL( selectionChanged() ),   this,   SLOT( UpdateUIBookView() ) );
    connect( m_wCodeView,                   SIGNAL( selectionChanged() ),   this,   SLOT( UpdateUICodeView() ) );
    connect( m_wBookView->page(),           SIGNAL( contentsChanged() ),    this,   SLOT( DocumentWasModified() ) );
    connect( m_wCodeView,                   SIGNAL( textChanged() ),        this,   SLOT( DocumentWasModified() ) );
    connect( m_wBookView->page(),           SIGNAL( contentsChanged() ),    this,   SLOT( UpdateSourceFromBookView() ) );
    connect( m_wCodeView,                   SIGNAL( textChanged() ),        this,   SLOT( UpdateSourceFromCodeView() ) );

    connect( m_cbHeadings,  SIGNAL( activated( const QString& ) ),          this,   SLOT( HeadingStyle( const QString& ) ) );

    connect( qApp,          SIGNAL( focusChanged( QWidget*, QWidget* ) ),   this,   SLOT( FocusFilter( QWidget*, QWidget* ) ) );
}








