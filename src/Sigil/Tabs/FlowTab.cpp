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


FlowTab::FlowTab( Resource& resource, const QUrl &fragment, QWidget *parent )
    : 
    ContentTab( resource, parent ),
    m_FragmentToScroll( fragment ),
    m_HTMLResource( *( qobject_cast< HTMLResource* >( &resource ) ) ),
    m_Splitter( *new QSplitter( this ) ),
    m_wBookView( *new BookViewEditor( this ) ),
    m_wCodeView( *new CodeViewEditor( CodeViewEditor::Highlight_XHTML, this ) ),
    m_IsLastViewBook( true ),
    m_InSplitView( false )
{
    // Loading a flow tab can take a while. We set the wait
    // cursor and clear it at the end of the delayed initialization.
    QApplication::setOverrideCursor( Qt::WaitCursor );

    m_Layout.addWidget( &m_Splitter );

    m_Splitter.setOrientation( Qt::Vertical );
    m_Splitter.addWidget( &m_wBookView );
    m_Splitter.addWidget( &m_wCodeView );

    ConnectSignalsToSlots();

    // We need to set this in the constructor too,
    // so that the ContentTab focus handlers don't 
    // get called when the tab is created.
    setFocusProxy( &m_wBookView );

    // We perform delayed initialization after the widget is on
    // the screen. This way, the user perceives less load time.
    QTimer::singleShot( 0, this, SLOT( DelayedInitialization() ) );
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
    if ( m_IsLastViewBook )
        
        return m_wBookView.pageAction( QWebPage::ToggleBold )->isChecked();

    else

        return ContentTab::BoldChecked();
}


bool FlowTab::ItalicChecked()
{
    if ( m_IsLastViewBook )
    
        return m_wBookView.pageAction( QWebPage::ToggleItalic )->isChecked();

    else

        return ContentTab::ItalicChecked();
}


bool FlowTab::UnderlineChecked()
{
    if ( m_IsLastViewBook )
        
        return m_wBookView.pageAction( QWebPage::ToggleUnderline )->isChecked(); 

    else

        return ContentTab::UnderlineChecked();
}


bool FlowTab::StrikethroughChecked()
{
    if ( m_IsLastViewBook )
    
        return m_wBookView.QueryCommandState( "strikeThrough" );

    else

        return ContentTab::StrikethroughChecked();
}


bool FlowTab::BulletListChecked()
{
    if ( m_IsLastViewBook )
        
        return m_wBookView.QueryCommandState( "insertUnorderedList" );

    else

        return ContentTab::BulletListChecked();
}


bool FlowTab::NumberListChecked()
{
    if ( m_IsLastViewBook )
    
        return m_wBookView.QueryCommandState( "insertOrderedList" );

    else

        return ContentTab::NumberListChecked();
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
    if ( m_IsLastViewBook )
        
        return m_wBookView.GetCaretElementName();

    else

        return ContentTab::GetCaretElementName();
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


Searchable* FlowTab::GetSearchableContent()
{
    if ( m_IsLastViewBook )

        return &m_wBookView;

    else

        return &m_wCodeView;
}


ContentTab::ViewState FlowTab::GetViewState()
{
    if ( m_IsLastViewBook )

        return ContentTab::ViewState_BookView;

    else

        return ContentTab::ViewState_CodeView;
}


void FlowTab::ScrollToFragment( const QString &fragment )
{   
    if ( m_wBookView.isVisible() )

        m_wBookView.ScrollToFragment( fragment );
}


void FlowTab::ScrollToTop()
{
    if ( m_IsLastViewBook )
    
        m_wBookView.ScrollToTop();

    else

        m_wCodeView.ScrollToTop();
}


void FlowTab::Undo()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Undo );

        m_HTMLResource.RemoveWebkitCruft();
    }

    else if ( m_wCodeView.hasFocus() )
    {
        m_wCodeView.undo();
    }
}


void FlowTab::Redo()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Redo );

        m_HTMLResource.RemoveWebkitCruft();
    }

    else if ( m_wCodeView.hasFocus() )
    {
        m_wCodeView.redo();
    }
}


void FlowTab::Cut()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Cut );

        m_HTMLResource.RemoveWebkitCruft();
    }

    else if ( m_wCodeView.hasFocus() )
    {
        m_wCodeView.cut();
    }
}


void FlowTab::Copy()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Copy );

        m_HTMLResource.RemoveWebkitCruft();
    }

    else if ( m_wCodeView.hasFocus() )
    {
        m_wCodeView.copy();
    }
}


void FlowTab::Paste()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Paste );

        m_HTMLResource.RemoveWebkitCruft();
    }

    else if ( m_wCodeView.hasFocus() )
    {
        m_wCodeView.paste();
    }
}


void FlowTab::Bold()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::ToggleBold );

        m_HTMLResource.RemoveWebkitCruft();
    }    
}


void FlowTab::Italic()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::ToggleItalic );

        m_HTMLResource.RemoveWebkitCruft();
    }    
}


void FlowTab::Underline()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::ToggleUnderline );

        m_HTMLResource.RemoveWebkitCruft();
    }    
}


void FlowTab::Strikethrough()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "strikeThrough" );

        m_HTMLResource.RemoveWebkitCruft();
    }    
}


void FlowTab::AlignLeft()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyLeft" );

        m_HTMLResource.RemoveWebkitCruft();
    }    
}


void FlowTab::Center()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyCenter" );

        m_HTMLResource.RemoveWebkitCruft();
    }    
}


void FlowTab::AlignRight()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyRight" );

        m_HTMLResource.RemoveWebkitCruft();
    }    
}


void FlowTab::Justify()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyFull" );

        m_HTMLResource.RemoveWebkitCruft();
    }    
}


void FlowTab::InsertChapterBreak()
{
    emit OldTabRequest( m_wBookView.SplitChapter(), m_HTMLResource );
}


void FlowTab::InsertImage( const QString &image_path )
{
    // Make sure the Book View has focus before inserting images,
    // otherwise they are not inserted
    m_wBookView.GrabFocus();
    m_wBookView.ExecCommand( "insertImage", image_path );

    m_HTMLResource.RemoveWebkitCruft();
}


void FlowTab::InsertBulletedList()
{
    m_wBookView.ExecCommand( "insertUnorderedList" );

    m_HTMLResource.RemoveWebkitCruft();
}


void FlowTab::InsertNumberedList()
{
    m_wBookView.ExecCommand( "insertOrderedList" );

    m_HTMLResource.RemoveWebkitCruft();
}


void FlowTab::DecreaseIndent()
{
    m_wBookView.page()->triggerAction( QWebPage::Outdent );
}


void FlowTab::IncreaseIndent()
{
    m_wBookView.page()->triggerAction( QWebPage::Indent );
}


void FlowTab::RemoveFormatting()
{
    m_wBookView.page()->triggerAction( QWebPage::RemoveFormat );
}


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


void FlowTab::PrintPreview()
{
    if ( !m_wBookView.hasFocus() && !m_wCodeView.hasFocus() )

        return;

    QPrintPreviewDialog *print_preview = new QPrintPreviewDialog( this );

    if ( m_IsLastViewBook )
    {
        connect( print_preview, SIGNAL( paintRequested( QPrinter * ) ),
                 &m_wBookView,  SLOT(   print( QPrinter *) ) 
               );
    }

    else
    {
        connect( print_preview, SIGNAL( paintRequested( QPrinter * ) ),
                 &m_wCodeView,  SLOT(   print( QPrinter *) ) 
               );
    }        

    print_preview->exec();
    print_preview->deleteLater();
}


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

    print_dialog->deleteLater();
}


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


void FlowTab::SaveContentOnTabLeave()
{
    if ( m_IsLastViewBook )

        m_HTMLResource.UpdateDomDocumentFromWebPage();

    else

        m_HTMLResource.UpdateDomDocumentFromTextDocument();

    ContentTab::SaveContentOnTabLeave();
}


void FlowTab::LoadContentOnTabEnter()
{
    ContentTab::LoadContentOnTabEnter();

    if ( m_IsLastViewBook )

        m_HTMLResource.UpdateWebPageFromDomDocument();

    else

        m_HTMLResource.UpdateTextDocumentFromDomDocument();
}


void FlowTab::TabFocusChange( QWidget *old_widget, QWidget *new_widget )
{   
    // Whole tab gains focus
    if ( ( new_widget == &m_wBookView || new_widget == &m_wCodeView ) &&
           old_widget != &m_wBookView &&
           old_widget != &m_wCodeView 
       )
    {
        LoadContentOnTabEnter();
    }   

    // Whole tab loses focus
    else if ( ( old_widget == &m_wBookView || old_widget == &m_wCodeView ) &&
              new_widget != &m_wBookView && 
              new_widget != &m_wCodeView 
            )
    {
        SaveContentOnTabLeave();
    } 
}


void FlowTab::SplitViewFocusSwitch( QWidget *old_widget, QWidget *new_widget )
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


void FlowTab::DelayedInitialization()
{
    m_HTMLResource.UpdateWebPageFromDomDocument();
    m_HTMLResource.UpdateTextDocumentFromDomDocument();
    m_wBookView.CustomSetWebPage( m_HTMLResource.GetWebPage() );
    m_wCodeView.CustomSetDocument( m_HTMLResource.GetTextDocument() );

    BookView();

    m_wBookView.ScrollToFragmentAfterLoad( m_FragmentToScroll.toString() );

    // Fix for missing blinking caret... even though
    // the Book View already has focus... QtWebkit is really lovely, isn't it?
    if ( hasFocus() )
    
        m_wBookView.GrabFocus();

    DelayedConnectSignalsToSlots();

    // Cursor set in constructor
    QApplication::restoreOverrideCursor();
}


void FlowTab::EmitContentChanged()
{
    emit ContentChanged();
}


void FlowTab::EnterBookView()
{
    m_wBookView.StoreCaretLocationUpdate( m_wCodeView.GetCaretLocation() );
    m_HTMLResource.UpdateWebPageFromTextDocument();

    m_IsLastViewBook = true;
    setFocusProxy( &m_wBookView );

    emit EnteringBookView();
}


void FlowTab::EnterCodeView()
{
    m_wCodeView.StoreCaretLocationUpdate( m_wBookView.GetCaretLocation() );
    m_HTMLResource.UpdateTextDocumentFromWebPage(); 

    m_IsLastViewBook = false; 
    setFocusProxy( &m_wCodeView );

    emit EnteringCodeView();
}


void FlowTab::ReadSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );

    // TODO: fill this... with what?
}


void FlowTab::WriteSettings()
{
    QSettings settings;
    settings.beginGroup( SETTINGS_GROUP );    
}


ViewEditor& FlowTab::GetActiveViewEditor() const
{
    if ( m_IsLastViewBook )

        return m_wBookView;

    else

        return m_wCodeView;
}


void FlowTab::ConnectSignalsToSlots()
{
    connect( &m_wBookView, SIGNAL( ZoomFactorChanged( float ) ), this, SIGNAL( ZoomFactorChanged( float ) ) );
    connect( &m_wCodeView, SIGNAL( ZoomFactorChanged( float ) ), this, SIGNAL( ZoomFactorChanged( float ) ) );

    connect( &m_wBookView, SIGNAL( selectionChanged() ), this, SIGNAL( SelectionChanged() ) );
    connect( &m_wCodeView, SIGNAL( selectionChanged() ), this, SIGNAL( SelectionChanged() ) );

    connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), this, SLOT( TabFocusChange( QWidget*, QWidget* ) ) );
    connect( qApp, SIGNAL( focusChanged( QWidget*, QWidget* ) ), this, SLOT( SplitViewFocusSwitch( QWidget*, QWidget* ) ) );
}


void FlowTab::DelayedConnectSignalsToSlots()
{
    connect( &m_wBookView, SIGNAL( textChanged() ),         this, SLOT( EmitContentChanged() ) );
    connect( &m_wCodeView, SIGNAL( FilteredTextChanged() ), this, SLOT( EmitContentChanged() ) );
}
