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
#include "FlowTab.h"
#include "Misc/Utility.h"
#include "ViewEditors/CodeViewEditor.h"
#include "ViewEditors/BookViewEditor.h"
#include "ResourceObjects/HTMLResource.h"
#include "WellFormedCheckComponent.h"
#include "BookManipulation/CleanSource.h"
#include "Misc/SettingsStore.h"
#include <QUrl>

static const QString SETTINGS_GROUP = "flowtab";


FlowTab::FlowTab( HTMLResource& resource, 
                  const QUrl &fragment, 
                  ContentTab::ViewState view_state,
                  int line_to_scroll_to,
                  QWidget *parent )
    : 
    ContentTab( resource, parent ),
    m_FragmentToScroll( fragment ),
    m_LineToScrollTo( line_to_scroll_to ),
    m_HTMLResource( resource ),
    m_Splitter( *new QSplitter( this ) ),
    m_wBookView( *new BookViewEditor( this ) ),
    m_wCodeView( *new CodeViewEditor( CodeViewEditor::Highlight_XHTML, true, this ) ),
    m_IsLastViewBook( true ),
    m_InSplitView( false ),
    m_StartingViewState( view_state ),
    m_WellFormedCheckComponent( *new WellFormedCheckComponent( *this ) )
{
    // Loading a flow tab can take a while. We set the wait
    // cursor and clear it at the end of the delayed initialization.
    QApplication::setOverrideCursor( Qt::WaitCursor );

    m_Layout.addWidget( &m_Splitter );

    LoadSettings();

    ConnectSignalsToSlots();

    // We need to set this in the constructor too,
    // so that the ContentTab focus handlers don't 
    // get called when the tab is created.
    if (view_state == ContentTab::ViewState_BookView )
    {
        setFocusProxy( &m_wBookView );
    }
    else
    {
        setFocusProxy( &m_wCodeView );
    }

    // We perform delayed initialization after the widget is on
    // the screen. This way, the user perceives less load time.
    QTimer::singleShot( 0, this, SLOT( DelayedInitialization() ) );
}


FlowTab::~FlowTab()
{
    m_WellFormedCheckComponent.deleteLater();
}


bool FlowTab::IsModified()
{
    return m_wBookView.isModified() || m_wCodeView.document()->isModified();
}


bool FlowTab::PrintEnabled()
{
    return true;
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


int FlowTab::GetCursorLine() const
{
    return GetActiveViewEditor().GetCursorLine();
}


int FlowTab::GetCursorColumn() const
{
    return GetActiveViewEditor().GetCursorColumn();
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


void FlowTab::UpdateDisplay()
{
    m_wBookView.UpdateDisplay();
    m_wCodeView.UpdateDisplay();
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


void FlowTab::SetViewState( ViewState new_view_state )
{
    if ( new_view_state == ContentTab::ViewState_BookView )
    
        BookView();

    else if ( new_view_state == ContentTab::ViewState_CodeView )

        CodeView();

    // otherwise ignore it
}


bool FlowTab::IsLoadingFinished()
{
    return m_wBookView.IsLoadingFinished() && m_wCodeView.IsLoadingFinished();
}

void FlowTab::ExecuteCaretUpdate()
{
    if (m_IsLastViewBook) {
        m_wBookView.ExecuteCaretUpdate();
    }
    else
    {
        m_wCodeView.ExecuteCaretUpdate();
    }
}

void FlowTab::ScrollToFragment( const QString &fragment )
{
    if (m_IsLastViewBook) {
        m_wBookView.ScrollToFragment(fragment);
    }
    else {
        m_wCodeView.ScrollToFragment(fragment);
    }
}


void FlowTab::ScrollToLine( int line )
{
    if ( m_wCodeView.isVisible() )

        m_wCodeView.ScrollToLine( line );
}


void FlowTab::ScrollToTop()
{
   // Scroll *both* views to the top, as this may be called before DelayedInitialisation() fires.
   m_wBookView.ScrollToTop();
   m_wCodeView.ScrollToTop();
}


void FlowTab::AutoFixWellFormedErrors()
{
    if ( m_IsLastViewBook )

        return;

    m_wCodeView.ReplaceDocumentText( CleanSource::PrettyPrint( CleanSource::Clean( m_wCodeView.toPlainText() ) ) );
}


bool FlowTab::GetCheckWellFormedErrors()
{
    return m_WellFormedCheckComponent.GetCheckWellFormedErrors();
}


void FlowTab::SetWellFormedDialogsEnabledState( bool enabled )
{
    m_WellFormedCheckComponent.SetWellFormedDialogsEnabledState( enabled );
}


void FlowTab::SetCheckWellFormedErrorsState( bool enabled )
{
    m_WellFormedCheckComponent.SetCheckWellFormedErrorsState( enabled );
}


void FlowTab::TakeControlOfUI()
{
    EmitCentralTabRequest();
    setFocus();
}


QString FlowTab::GetFilename()
{
    return ContentTab::GetFilename();
}


bool FlowTab::IsDataWellFormed()
{
    if ( m_IsLastViewBook || !GetCheckWellFormedErrors() )
    {
        m_safeToLoad = true;
        return true;
    }

    XhtmlDoc::WellFormedError error = XhtmlDoc::WellFormedErrorForSource( m_wCodeView.toPlainText() );
    bool well_formed = error.line == -1;

    if ( !well_formed )
    {
        m_safeToLoad = false;
        m_WellFormedCheckComponent.DemandAttentionIfAllowed( error );
    }
    else
    {
        m_safeToLoad = true;
    }

    return well_formed;
}


void FlowTab::Undo()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::Undo );
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
    }    
}


void FlowTab::Italic()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::ToggleItalic );
    }    
}


void FlowTab::Underline()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.page()->triggerAction( QWebPage::ToggleUnderline );
    }    
}


void FlowTab::Strikethrough()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "strikeThrough" );
    }    
}


void FlowTab::AlignLeft()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyLeft" );
    }    
}


void FlowTab::Center()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyCenter" );
    }    
}


void FlowTab::AlignRight()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyRight" );
    }    
}


void FlowTab::Justify()
{
    if ( m_wBookView.hasFocus() )
    {
        m_wBookView.ExecCommand( "justifyFull" );
    }    
}


void FlowTab::SplitChapter()
{
    if ( !IsDataWellFormed() )

        return;

    if ( m_IsLastViewBook )

        emit OldTabRequest( m_wBookView.SplitChapter(), m_HTMLResource );

    else

        emit OldTabRequest( m_wCodeView.SplitChapter(), m_HTMLResource );
}


void FlowTab::InsertSGFChapterMarker()
{
    if ( !IsDataWellFormed() )

        return;

    if ( m_wBookView.hasFocus() )
    
        m_wBookView.ExecCommand( "insertHTML", BREAK_TAG_INSERT );    

    else if ( m_wCodeView.hasFocus() )
    
        m_wCodeView.InsertSGFChapterMarker();    
}


void FlowTab::SplitOnSGFChapterMarkers()
{
    if ( !IsDataWellFormed() )

        return;

    SaveTabContent();
    emit NewChaptersRequest( m_HTMLResource.SplitOnSGFChapterMarkers(), m_HTMLResource );
    LoadTabContent();
}


void FlowTab::InsertImage( const QString &image_path )
{
    if (m_IsLastViewBook) {
        m_wBookView.ExecCommand( "insertImage", image_path );
    }
    else {
        m_wCodeView.insertPlainText(QString("<img src=\"%1\"/>").arg(image_path));
    }
}


void FlowTab::InsertBulletedList()
{
    m_wBookView.ExecCommand( "insertUnorderedList" );
}


void FlowTab::InsertNumberedList()
{
    m_wBookView.ExecCommand( "insertOrderedList" );
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
    // This slot is invoked from MainWindow 
    // (via the combobox signal), while
    // the FlowTab does not have a modify lock.
    // So we need to get one first.
    LoadTabContent();

    QChar last_char = heading_type[ heading_type.count() - 1 ];

    // For heading_type == "Heading #"
    if ( last_char.isDigit() )

        m_wBookView.FormatBlock( "h" + QString( last_char ) );

    else if ( heading_type == "Normal" )

        m_wBookView.FormatBlock( "p" );

    // else is "<Select heading>" which does nothing

    SaveTabContent();

    // Focus will have switched to the combobox, so let BookView grab it back
    m_wBookView.GrabFocus();
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

    QPrintDialog print_dialog(&printer, this);
    print_dialog.setWindowTitle(tr("Print %1").arg(GetFilename()));

    if (print_dialog.exec() == QDialog::Accepted) {
        if (m_IsLastViewBook) {
            m_wBookView.print( &printer );
        }
        else {
            m_wCodeView.print( &printer );
        }
    }
}


void FlowTab::BookView()
{
    // The user probably got here by pressing one of
    // the View buttons, and we may need to reset the
    // checked state of those buttons.
    emit ViewButtonsStateChanged();

    if ( !IsDataWellFormed() )

        return;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    // Update the book view cursor and toolbar button states.
    if ( !m_IsLastViewBook )

        EnterBookView();

    m_InSplitView = false;

    m_wCodeView.hide();
    m_wBookView.show();

    setFocusProxy( &m_wBookView );
    m_wBookView.GrabFocus();

    m_IsLastViewBook = true;

    emit ViewChanged();

    QApplication::restoreOverrideCursor();
}


void FlowTab::SplitView()
{
    // The user probably got here by pressing one of
    // the View buttons, and we may need to reset the
    // checked state of those buttons.
    emit ViewButtonsStateChanged();

    if ( !IsDataWellFormed() )

        return;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    // Update whichever view is newly displayed.
    // This needs to be done explicitly as that view will not be given focus.
    // All other updates happen solely through the focus event handlers.
    LoadTabContent();
    if (m_IsLastViewBook) {
        EnterBookView();
    }
    else {
        EnterBookView();
    }

    m_InSplitView = true;

    m_wBookView.show();
    m_wCodeView.show();
    m_wCodeView.SetDelayedCursorScreenCenteringRequired();

    emit ViewChanged();

    QApplication::restoreOverrideCursor();
}


void FlowTab::CodeView()
{    
    QApplication::setOverrideCursor( Qt::WaitCursor );

    // Update the code view cursor and the toolbar button states.
    if ( m_IsLastViewBook )
    
        EnterCodeView();

    m_InSplitView = false;

    m_wBookView.hide();
    m_wCodeView.show();       
    m_wCodeView.SetDelayedCursorScreenCenteringRequired();

    setFocusProxy( &m_wCodeView );

    // Make sure the cursor is properly displayed
    if( !m_wCodeView.hasFocus() )
    {
        m_wCodeView.setFocus();
    }

    m_IsLastViewBook = false; 

    emit ViewChanged();

    QApplication::restoreOverrideCursor();
}


void FlowTab::SaveTabContent()
{
    if ( m_IsLastViewBook )
    {
        m_HTMLResource.UpdateDomDocumentFromWebPage();

        // Save the cursor location for when the tab is re-opened
        m_wBookView.StoreCaretLocationUpdate( m_wBookView.GetCaretLocation() );
    }
    else
    {
        m_HTMLResource.UpdateDomDocumentFromTextDocument();

        // Save the cursor location for when the tab is re-opened
        m_wCodeView.StoreCaretLocationUpdate( m_wCodeView.GetCaretLocation() );
    }

    m_safeToLoad = true;
}


void FlowTab::LoadTabContent()
{
    if( m_safeToLoad )
    {
        if ( m_IsLastViewBook )
        {
            m_HTMLResource.UpdateWebPageFromDomDocument();
        }
        else
        {
            m_HTMLResource.UpdateTextDocumentFromDomDocument();
        }
    }
}


void FlowTab::LoadSettings()
{
    SettingsStore *store = SettingsStore::instance();
    m_Splitter.setOrientation(store->splitViewOrientation());

    // If widgets already exist, splitter will rearrange them
    if ( store->splitViewOrder() ) 
    {
        m_Splitter.addWidget( &m_wBookView );
        m_Splitter.addWidget( &m_wCodeView );
    }
    else
    {
        m_Splitter.addWidget( &m_wCodeView );
        m_Splitter.addWidget( &m_wBookView );
    }
}


void FlowTab::LeaveEditor( QWidget *editor )
{
    if( editor == &m_wBookView )
    {
        m_IsLastViewBook = true;
        SaveTabContent();
    }
    else if( editor == &m_wCodeView )
    {
        if( IsDataWellFormed() )
        {
            m_IsLastViewBook = false;
            SaveTabContent();
        }
        else {
            m_wCodeView.setFocus();
        }
    }
}

void FlowTab::EnterEditor( QWidget *editor )
{
    // Force a save before we do a load.
    // What can happen is, if the user has cv open, does a find and replace
    // then switches to bv the cv contents with the replacment are not saved.
    // The flow is cv -> cv focus lost -> save cv contents -> f&r gains focus ->
    // cv text changed -> bv focus -> bv loads saved data. The changes from
    // f&r are never saved so we force them to be saved before loading the
    // data into the new view to ensure the newest data is used.
    if ( m_IsLastViewBook )
    {
        m_HTMLResource.UpdateDomDocumentFromWebPage();
    }
    else
    {
        if( !GetCheckWellFormedErrors() )
        {
            m_HTMLResource.UpdateDomDocumentFromTextDocument();
        }
    }

    if( editor == &m_wBookView && !m_IsLastViewBook )
    {
        m_IsLastViewBook = true;
        LoadTabContent();

        if( m_InSplitView )
        {
            QApplication::setOverrideCursor( Qt::WaitCursor );
            EnterBookView();
            QApplication::restoreOverrideCursor();
        }
    }
    else if( editor == &m_wCodeView && m_IsLastViewBook )
    {
        m_IsLastViewBook = false;
        LoadTabContent();

        if( m_InSplitView )
        {
            QApplication::setOverrideCursor( Qt::WaitCursor );
            EnterCodeView();
            QApplication::restoreOverrideCursor();
        }
    }

    EmitUpdateCursorPosition();
}


void FlowTab::DelayedInitialization()
{
    m_HTMLResource.UpdateWebPageFromDomDocument();
    m_HTMLResource.UpdateTextDocumentFromDomDocument();
    m_wBookView.CustomSetWebPage( m_HTMLResource.GetWebPage() );
    m_wCodeView.CustomSetDocument( m_HTMLResource.GetTextDocument() );

    switch( m_StartingViewState )
    {
        case ContentTab::ViewState_BookView: 
        {
            m_IsLastViewBook = true;

            BookView();

            m_wBookView.ScrollToFragmentAfterLoad( m_FragmentToScroll.toString() );
            break;
        }

        case ContentTab::ViewState_CodeView:
        {
            // Stop Code View attempting to read the content from the web page, since it already
            // has a valid copy of the content and the web page might not have finished loading yet.
            m_IsLastViewBook = false;

            CodeView();

            if( m_LineToScrollTo > 0 )
            {
                m_wCodeView.ScrollToLine( m_LineToScrollTo );
            }
            else
            {
                m_wCodeView.ScrollToFragment( m_FragmentToScroll.toString() );
            }

            break;
        }

        case ContentTab::ViewState_NoFocusBookView:
        {
            m_StartingViewState = ContentTab::ViewState_BookView;
            m_wCodeView.hide();
            m_wBookView.show();
            m_IsLastViewBook = true;
            break;
        }

        case ContentTab::ViewState_NoFocusCodeView:
        {
            m_StartingViewState = ContentTab::ViewState_CodeView;
            m_wBookView.hide();
            m_wCodeView.show();
            m_IsLastViewBook = false;
            break;
        }

        // Don't care about these so ignore them.
        case ContentTab::ViewState_AnyView:
        case ContentTab::ViewState_RawView:
        case ContentTab::ViewState_StaticView:
        default:
            break;
    }

    m_wBookView.Zoom();
    m_wCodeView.Zoom();

    m_safeToLoad = true;

    DelayedConnectSignalsToSlots();

    // Cursor set in constructor
    QApplication::restoreOverrideCursor();
}


void FlowTab::EmitContentChanged()
{
    m_safeToLoad = false;
    emit ContentChanged();
}


void FlowTab::EmitUpdateCursorPosition()
{
    emit UpdateCursorPosition(GetCursorLine(), GetCursorColumn());
}


void FlowTab::EnterBookView()
{
    if ( !IsDataWellFormed() )

        return;

    m_wBookView.StoreCaretLocationUpdate( m_wCodeView.GetCaretLocation() );
    // All changes to the document are routed through the Dom Document via the Load and
    // Save routines called by the focus handlers. The Dom Document is thus always the
    // canonical version, rather than allowing edits to be routed directly between the Text
    // Document and the Web page.

    emit EnteringBookView();
}


void FlowTab::EnterCodeView()
{
    m_wCodeView.StoreCaretLocationUpdate( m_wBookView.GetCaretLocation() );
    // See above note.

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
    connect(&m_wCodeView, SIGNAL(cursorPositionChanged()), this, SLOT(EmitUpdateCursorPosition()));

    connect( &m_wBookView, SIGNAL( ZoomFactorChanged( float ) ), this, SIGNAL( ZoomFactorChanged( float ) ) );
    connect( &m_wCodeView, SIGNAL( ZoomFactorChanged( float ) ), this, SIGNAL( ZoomFactorChanged( float ) ) );

    connect( &m_wBookView, SIGNAL( selectionChanged() ),         this, SIGNAL( SelectionChanged() ) );
    connect( &m_wCodeView, SIGNAL( selectionChanged() ),         this, SIGNAL( SelectionChanged() ) );

    connect( &m_wCodeView, SIGNAL( FocusGained( QWidget* ) ),    this, SLOT( EnterEditor( QWidget* ) ) );
    connect( &m_wCodeView, SIGNAL( FocusLost( QWidget* )   ),    this, SLOT( LeaveEditor( QWidget* ) ) );

    connect( &m_wBookView, SIGNAL( FocusGained( QWidget* ) ),    this, SLOT( EnterEditor( QWidget* ) ) );
    connect( &m_wBookView, SIGNAL( FocusLost( QWidget* )   ),    this, SLOT( LeaveEditor( QWidget* ) ) );

    SettingsStore *store = SettingsStore::instance();
    connect( store, SIGNAL( settingsChanged() ), this, SLOT( LoadSettings() ) );
}


void FlowTab::DelayedConnectSignalsToSlots()
{
    connect( &m_wBookView, SIGNAL( textChanged() ),         this, SLOT( EmitContentChanged() ) );
    connect( &m_wCodeView, SIGNAL( FilteredTextChanged() ), this, SLOT( EmitContentChanged() ) );
}


