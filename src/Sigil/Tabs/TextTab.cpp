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
#include "TextTab.h"
#include "ResourceObjects/TextResource.h"


TextTab::TextTab( TextResource& resource,
                  CodeViewEditor::HighlighterType type,  
                  int line_to_scroll_to,
                  QWidget *parent )
    :
    ContentTab( resource, parent ),
    m_wCodeView( *new CodeViewEditor( type, this ) ),
    m_TextResource( resource ),
    m_LineToScrollTo( line_to_scroll_to )
{
    m_Layout.addWidget( &m_wCodeView );
    setFocusProxy( &m_wCodeView );

    ConnectSignalsToSlots();

    m_TextResource.InitialLoad();

    // We perform delayed initialization after the widget is on
    // the screen. This way, the user perceives less load time.
    QTimer::singleShot( 0, this, SLOT( DelayedInitialization() ) );    
}   


void TextTab::ScrollToLine( int line )
{
    m_wCodeView.ScrollToLine( line );
}


bool TextTab::IsModified()
{
    return m_wCodeView.document()->isModified();
}


bool TextTab::CutEnabled()
{
    return m_wCodeView.textCursor().hasSelection();
}


bool TextTab::CopyEnabled()
{
    return m_wCodeView.textCursor().hasSelection();
}


bool TextTab::PasteEnabled()
{
    return m_wCodeView.canPaste();
}


float TextTab::GetZoomFactor() const
{
    return m_wCodeView.GetZoomFactor();
}


void TextTab::SetZoomFactor( float new_zoom_factor )
{
    m_wCodeView.SetZoomFactor( new_zoom_factor );
}


Searchable* TextTab::GetSearchableContent()
{
    return &m_wCodeView;
}


ContentTab::ViewState TextTab::GetViewState()
{
    return ContentTab::ViewState_RawView;
}


void TextTab::Undo()
{
    if( m_wCodeView.hasFocus() )
    {
        m_wCodeView.undo();
    }
}


void TextTab::Redo()
{
    if( m_wCodeView.hasFocus() )
    {
        m_wCodeView.redo();
    }
}


void TextTab::Cut()
{
    if( m_wCodeView.hasFocus() )
    {
        m_wCodeView.cut();
    }
}


void TextTab::Copy()
{
    if( m_wCodeView.hasFocus() )
    {
        m_wCodeView.copy();
    }
}


void TextTab::Paste()
{
    if( m_wCodeView.hasFocus() )
    {
        m_wCodeView.paste();
    }
}


void TextTab::SaveTabContent()
{
    // We can't perform the document modified check
    // here because that causes problems with epub export
    // when the user has not changed the text file.
    // (some text files have placeholder text on disk)
    if ( !m_wCodeView.document()->isModified() )
    {
        ContentTab::SaveTabContent();
        return;
    }

    m_TextResource.SaveToDisk();
    ContentTab::SaveTabContent();
}


void TextTab::SaveTabContent( QWidget *editor )
{
    Q_UNUSED( editor );

    SaveTabContent();
}


void TextTab::LoadTabContent()
{

}


void TextTab::LoadTabContent( QWidget *editor )
{
    Q_UNUSED( editor );

    LoadTabContent();
}


void TextTab::DelayedInitialization()
{
    m_wCodeView.CustomSetDocument( m_TextResource.GetTextDocumentForWriting() );

    m_wCodeView.ScrollToLine( m_LineToScrollTo );
}


void TextTab::ConnectSignalsToSlots()
{
    // We set the Code View as the focus proxy for the tab,
    // so the ContentTab focusIn/Out handlers are not called.
    connect( &m_wCodeView, SIGNAL( FocusGained( QWidget* ) ),    this, SLOT( LoadTabContent( QWidget* ) ) );
    connect( &m_wCodeView, SIGNAL( FocusLost( QWidget* ) ),      this, SLOT( SaveTabContent( QWidget* ) ) );

    connect( &m_wCodeView, SIGNAL( FilteredTextChanged() ),      this, SIGNAL( ContentChanged() )           );
    connect( &m_wCodeView, SIGNAL( ZoomFactorChanged( float ) ), this, SIGNAL( ZoomFactorChanged( float ) ) );
    connect( &m_wCodeView, SIGNAL( selectionChanged() ),         this, SIGNAL( SelectionChanged() )         );
}


