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
#include "FlowTab.h"
#include "../Misc/Utility.h"
#include "../ViewEditors/CodeViewEditor.h"
#include "../ViewEditors/BookViewEditor.h"
#include "../ResourceObjects/HTMLResource.h"
#include <QUrl>

static const QString SETTINGS_GROUP = "flowtab";

QString FlowTab::s_LastFolderImage = QString();


FlowTab::FlowTab( Resource& resource, const QUrl &fragment, QWidget *parent )
    : 
    ContentTab( resource, parent ),
    m_HTMLResource( *( qobject_cast< HTMLResource* >( &resource ) ) ),
    m_Splitter( *new QSplitter( this ) ),
    m_wBookView( *new BookViewEditor( m_HTMLResource.GetWebPage(), this ) ),
    m_wCodeView( *new CodeViewEditor( CodeViewEditor::Highlight_XHTML, this ) ),
    m_IsLastViewBook( true ),
    m_InSplitView( false )
{
    m_Layout.addWidget( &m_Splitter );

    m_Splitter.setOrientation( Qt::Vertical );
    m_Splitter.addWidget( &m_wBookView );
    m_Splitter.addWidget( &m_wCodeView );

    ConnectSignalsToSlots();

    m_HTMLResource.SyncFromDisk();

    BookView();

    m_wBookView.ScrollToFragmentAfterLoad( fragment.toString() );
}


bool FlowTab::IsModified()
{
    return m_wBookView.isModified() || m_wCodeView.document()->isModified();
}


bool FlowTab::CutEnabled()
{
    if ( m_IsLastViewBook )

        return m_wBookView.pageAction( QWebPage::Cut )->isEnabled();

    else

        return m_wCodeView.textCursor().hasSelection();
}


bool FlowTab::CopyEnabled()
{
    if ( m_IsLastViewBook )

        return m_wBookView.pageAction( QWebPage::Copy )->isEnabled();

    else

        return m_wCodeView.textCursor().hasSelection();
}


bool FlowTab::PasteEnabled()
{
    if ( m_IsLastViewBook )

        return m_wBookView.pageAction( QWebPage::Paste )->isEnabled();

    else

        return m_wCodeView.canPaste();
}


bool FlowTab::BoldChecked()
{
    return m_wBookView.pageAction( QWebPage::ToggleBold )->isChecked();
}


bool FlowTab::ItalicChecked()
{
    return m_wBookView.pageAction( QWebPage::ToggleItalic )->isChecked();
}


bool FlowTab::UnderlineChecked()
{
    return m_wBookView.pageAction( QWebPage::ToggleUnderline )->isChecked();       
}


bool FlowTab::StrikethroughChecked()
{
    return m_wBookView.QueryCommandState( "strikeThrough" );
}


bool FlowTab::BulletListChecked()
{
    return m_wBookView.QueryCommandState( "insertUnorderedList" );
}


bool FlowTab::NumberListChecked()
{
    return m_wBookView.QueryCommandState( "insertOrderedList" );
}


bool FlowTab::BookViewChecked()
{
    return !m_wBookView.isHidden() && m_wCodeView.isHidden();
}


bool FlowTab::SplitViewChecked()
{
    return !m_wBookView.isHidden() && !m_wCodeView.isHidden();
}


bool FlowTab::CodeViewChecked()
{
    return m_wBookView.isHidden() && !m_wCodeView.isHidden();
}


QString FlowTab::GetCaretElementName()
{
    return m_wBookView.GetCaretElementName();
}


float FlowTab::GetZoomFactor() const
{
    return GetActiveViewEditor().GetZoomFactor();
}


void FlowTab::SetZoomFactor( float new_zoom_factor )
{
    // We need to set a wait cursor for the Book View
    // since zoom operations take some time in it.
    if ( m_IsLastViewBook )
    {
        QApplication::setOverrideCursor( Qt::WaitCursor );
        m_wBookView.SetZoomFactor( new_zoom_factor );
        QApplication::restoreOverrideCursor();
    }

    else
    {
        m_wCodeView.SetZoomFactor( new_zoom_factor );
    } 
}


void FlowTab::ScrollToFragment( const QString &fragment )
{   
    if ( m_wBookView.isVisible() )

        m_wBookView.ScrollToFragment( fragment );
}


ContentTab::ViewState FlowTab::GetViewState()
{
    if ( m_IsLastViewBook )

        return ContentTab::ViewState_BookView;

    else

        return ContentTab::ViewState_CodeView;
}



// Implements Undo action functionality
void FlowTab::Undo()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Undo );

        m_HTMLResource.RemoveWebkitClasses();
    }

    else if ( m_wCodeView.hasFocus() )
    {
        m_wCodeView.undo();
    }
}


// Implements Redo action functionality
void FlowTab::Redo()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Redo );

        m_HTMLResource.RemoveWebkitClasses();
    }

    else if ( m_wCodeView.hasFocus() )
    {
        m_wCodeView.redo();
    }
}


// Implements Cut action functionality
void FlowTab::Cut()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Cut );

        m_HTMLResource.RemoveWebkitClasses();
    }

    else if ( m_wCodeView.hasFocus() )
    {
        m_wCodeView.cut();
    }
}


// Implements Copy action functionality
void FlowTab::Copy()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Copy );

        m_HTMLResource.RemoveWebkitClasses();
    }

    else if ( m_wCodeView.hasFocus() )
    {
        m_wCodeView.copy();
    }
}


// Implements Paste action functionality
void FlowTab::Paste()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Paste );

        m_HTMLResource.RemoveWebkitClasses();
    }

    else if ( m_wCodeView.hasFocus() )
    {
        m_wCodeView.paste();
    }
}


// Implements Bold action functionality
void FlowTab::Bold()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::ToggleBold );

        m_HTMLResource.RemoveWebkitClasses();
    }    
}


// Implements Italic action functionality
void FlowTab::Italic()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::ToggleItalic );

        m_HTMLResource.RemoveWebkitClasses();
    }    
}


// Implements Underline action functionality
void FlowTab::Underline()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::ToggleUnderline );

        m_HTMLResource.RemoveWebkitClasses();
    }    
}


// Implements Strikethrough action functionality
void FlowTab::Strikethrough()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "strikeThrough" );

        m_HTMLResource.RemoveWebkitClasses();
    }    
}


// Implements Align Left action functionality
void FlowTab::AlignLeft()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyLeft" );

        m_HTMLResource.RemoveWebkitClasses();
    }    
}


// Implements Center action functionality
void FlowTab::Center()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyCenter" );

        m_HTMLResource.RemoveWebkitClasses();
    }    
}


// Implements Align Right action functionality
void FlowTab::AlignRight()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyRight" );

        m_HTMLResource.RemoveWebkitClasses();
    }    
}


// Implements Justify action functionality
void FlowTab::Justify()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyFull" );

        m_HTMLResource.RemoveWebkitClasses();
    }    
}


// Implements Insert chapter break action functionality
void FlowTab::InsertChapterBreak()
{
    m_wBookView.ExecCommand( "insertHTML", BREAK_TAG_INSERT );

    m_HTMLResource.RemoveWebkitClasses();
}


// Implements Insert image action functionality
void FlowTab::InsertImage()
{
    QStringList filenames = QFileDialog::getOpenFileNames(  this, 
                                                            tr( "Insert Image(s)" ), 
                                                            s_LastFolderImage, 
                                                            tr( "Images (*.png *.jpg *.jpeg *.gif *.svg)")
                                                            );

    if ( filenames.isEmpty() )

        return;

    // Store the folder the user inserted the image from
    s_LastFolderImage = QFileInfo( filenames.first() ).absolutePath();

    // Make sure the Book View has focus before inserting images,
    // otherwise they are not inserted
    m_wBookView.GrabFocus();

    // FIXME: image loading

//     foreach( QString filename, filenames )
//     {
//         QString relative_path = "../" + m_Book.mainfolder.AddContentFileToFolder( filename );
// 
//         m_wBookView.ExecCommand( "insertImage", relative_path );
//     }    

    m_HTMLResource.RemoveWebkitClasses();
}


// Implements Insert bulleted list action functionality
void FlowTab::InsertBulletedList()
{
    m_wBookView.ExecCommand( "insertUnorderedList" );

    m_HTMLResource.RemoveWebkitClasses();
}


// Implements Insert numbered list action functionality
void FlowTab::InsertNumberedList()
{
    m_wBookView.ExecCommand( "insertOrderedList" );

    m_HTMLResource.RemoveWebkitClasses();
}


// Implements Decrease indent action functionality
void FlowTab::DecreaseIndent()
{
    m_wBookView.page()->triggerAction( QWebPage::Outdent );
}


// Implements Increase indent action functionality
void FlowTab::IncreaseIndent()
{
    m_wBookView.page()->triggerAction( QWebPage::Indent );
}


// Implements Remove Formatting action functionality
void FlowTab::RemoveFormatting()
{
    m_wBookView.page()->triggerAction( QWebPage::RemoveFormat );
}


// Implements the heading combo box functionality
void FlowTab::HeadingStyle( const QString& heading_type )
{
    QChar last_char = heading_type[ heading_type.count() - 1 ];

    // For heading_type == "Heading #"
    if ( last_char.isDigit() )

        m_wBookView.FormatBlock( "h" + QString( last_char ) );

    else if ( heading_type == "Normal" )

        m_wBookView.FormatBlock( "p" );

    // else is "<Select heading>" which does nothing
}


// Implements Print Preview action functionality
void FlowTab::PrintPreview()
{
    if ( !m_wBookView.hasFocus() && !m_wCodeView.hasFocus() )

        return;

    QPrintPreviewDialog *print_preview = new QPrintPreviewDialog( this );

    if ( m_IsLastViewBook )
    {
        connect(    print_preview,     SIGNAL( paintRequested( QPrinter * ) ),
                    &m_wBookView,       SLOT(   print( QPrinter *) ) 
               );
    }

    else
    {
        connect(    print_preview,     SIGNAL( paintRequested( QPrinter * ) ),
                    &m_wCodeView,       SLOT(   print( QPrinter *) ) 
               );
    }        

    print_preview->exec();

    // FIXME: maybe call deleteLater() here for dialog?
    // also for Print dialog
}


// Implements Print action functionality
void FlowTab::Print()
{
    if ( !m_wBookView.hasFocus() && !m_wCodeView.hasFocus() )

        return;

    QPrinter printer;

    QPrintDialog *print_dialog = new QPrintDialog( &printer, this );
    print_dialog->setWindowTitle( tr( "Print Document" ) );

    if ( m_IsLastViewBook )

        m_wBookView.print( &printer );

    else

        m_wCodeView.print( &printer );
}


// Implements Book View action functionality
void FlowTab::BookView()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );

    // Update the book view if we just edited
    // in the code view
    if ( !m_IsLastViewBook )

        EnterBookView();

    m_InSplitView = false;

    m_wBookView.show();
    m_wCodeView.hide();

    emit ViewChanged();

    QApplication::restoreOverrideCursor();
}


// Implements Split View action functionality
void FlowTab::SplitView()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );

    // Update the required view
    if ( !m_IsLastViewBook )

        EnterBookView();

    else
           
        EnterCodeView();

    m_InSplitView = true;

    m_wBookView.show();
    m_wCodeView.show();

    emit ViewChanged();

    QApplication::restoreOverrideCursor();
}


// Implements Code View action functionality
void FlowTab::CodeView()
{
    QApplication::setOverrideCursor( Qt::WaitCursor );

    // Update the code view if we just edited
    // in the book view
    if ( m_IsLastViewBook )
    
        EnterCodeView();    

    m_InSplitView = false;

    m_wBookView.hide();
    m_wCodeView.show();       

    emit ViewChanged();

    QApplication::restoreOverrideCursor();
}


// Used to catch the focus changeover from one widget
// (code or book view) to the other; needed for source synchronization.
void FlowTab::FocusFilter( QWidget *old_widget, QWidget *new_widget )
{
    // We make sure we are looking at focus changes
    // in Split View; otherwise, we don't care.
    if ( !m_InSplitView )

        return;

    // If we switched focus from the book view to the code view...
    if ( ( old_widget == &m_wBookView ) && ( new_widget == &m_wCodeView ) )
    { 
        QApplication::setOverrideCursor( Qt::WaitCursor );
        EnterCodeView();
        QApplication::restoreOverrideCursor();
    }

    // If we switched focus from the code view to the book view...
    else if ( ( old_widget == &m_wCodeView ) && ( new_widget == &m_wBookView ) )
    {
        QApplication::setOverrideCursor( Qt::WaitCursor );
        EnterBookView();
        QApplication::restoreOverrideCursor();
    }

    // else we don't care
}


void FlowTab::EmitContentChanged()
{
    emit ContentChanged();
}

void FlowTab::EnterBookView()
{
    m_wBookView.StoreCaretLocationUpdate( m_wCodeView.GetCaretLocation() );

    // If we haven't updated yet, then update
    if ( m_OldSource != m_wCodeView.toPlainText() )
    {
        UpdateStoredPageFromCodeView();
    }

    m_IsLastViewBook = true;  

    emit EnteringBookView();
}

void FlowTab::EnterCodeView()
{
    m_wCodeView.StoreCaretLocationUpdate( m_wBookView.GetCaretLocation() );

    // If we haven't updated yet, then update
    if ( m_OldSource != m_HTMLResource.GetHtml() )
    {
        UpdateCodeViewFromStoredPage();            
    }

    m_IsLastViewBook = false;   

    emit EnteringCodeView();
}


// On changeover, updates the code in code view
void FlowTab::UpdateCodeViewFromStoredPage()
{
    m_HTMLResource.RemoveWebkitClasses();

    QString source = m_HTMLResource.GetHtml();
    m_wCodeView.setPlainText( source );

    // Store current source so we can compare and check
    // if we updated yet or we haven't
    m_OldSource = source;
}


// On changeover, updates the code in book view
void FlowTab::UpdateStoredPageFromCodeView()
{
    QString source = m_wCodeView.toPlainText();
    m_HTMLResource.SetHtml( source );

    // Store current source so we can compare and check
    // if we updated yet or we haven't
    m_OldSource = source;
}

void FlowTab::ReadSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    s_LastFolderImage = settings.value( "lastfolderimage" ).toString();
}


// Returns the currently active View Editor
ViewEditor& FlowTab::GetActiveViewEditor() const
{
    if ( m_IsLastViewBook )

        return m_wBookView;

    else

        return m_wCodeView;
}


void FlowTab::WriteSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    settings.setValue( "lastfolderimage", s_LastFolderImage );
}


void FlowTab::ConnectSignalsToSlots()
{
    connect( &m_wBookView, SIGNAL( textChanged() ), this, SLOT( EmitContentChanged() ) );
    connect( &m_wCodeView, SIGNAL( textChanged() ), this, SLOT( EmitContentChanged() ) );

    connect( &m_wBookView, SIGNAL( ZoomFactorChanged( float ) ), this, SIGNAL( ZoomFactorChanged( float ) ) );
    connect( &m_wCodeView, SIGNAL( ZoomFactorChanged( float ) ), this, SIGNAL( ZoomFactorChanged( float ) ) );

    connect( m_wBookView.page(),   SIGNAL( selectionChanged() ), this, SIGNAL( SelectionChanged() ) );
    connect( &m_wCodeView,         SIGNAL( selectionChanged() ), this, SIGNAL( SelectionChanged() ) );

    connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), this, SLOT( FocusFilter( QWidget*, QWidget* ) ) );
}

