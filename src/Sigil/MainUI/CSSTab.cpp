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
#include "CSSTab.h"
#include "../ViewEditors/CodeViewEditor.h"
#include "../ResourceObjects/CSSResource.h"


CSSTab::CSSTab( Resource& resource, QWidget *parent )
    :
    ContentTab( resource, parent ),
    m_CSSResource( *( qobject_cast< CSSResource* >( &resource ) ) ),
    //m_TextDocument( *new QTextDocument( m_CSSResource.ReadFile(), this ) ),
    m_wCodeView( *new CodeViewEditor( CodeViewEditor::Highlight_CSS, this ) )
{
    m_Layout.addWidget( &m_wCodeView );

    ConnectSignalsToSlots();

    m_Source = m_CSSResource.ReadFile();

    m_wCodeView.SetContent( m_Source, QUrl() );
}   


bool CSSTab::IsModified()
{
    return m_wCodeView.document()->isModified();
}


bool CSSTab::CutEnabled()
{
    return m_wCodeView.textCursor().hasSelection();
}


bool CSSTab::CopyEnabled()
{
    return m_wCodeView.textCursor().hasSelection();
}


bool CSSTab::PasteEnabled()
{
    return m_wCodeView.canPaste();
}


float CSSTab::GetZoomFactor()
{
    return m_wCodeView.GetZoomFactor();
}

void CSSTab::SetZoomFactor( float new_zoom_factor )
{
    m_wCodeView.SetZoomFactor( new_zoom_factor );
}

void CSSTab::ConnectSignalsToSlots()
{
    connect( &m_wCodeView, SIGNAL( textChanged() ),              this, SIGNAL( ContentChanged() )           );
    connect( &m_wCodeView, SIGNAL( ZoomFactorChanged( float ) ), this, SIGNAL( ZoomFactorChanged( float ) ) );
    connect( &m_wCodeView, SIGNAL( selectionChanged() ),         this, SIGNAL( SelectionChanged() )         );
}