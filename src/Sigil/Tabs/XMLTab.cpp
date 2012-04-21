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

#include "BookManipulation/CleanSource.h"
#include "ResourceObjects/XMLResource.h"
#include "Tabs/WellFormedCheckComponent.h"
#include "Tabs/XMLTab.h"


XMLTab::XMLTab( XMLResource& resource, int line_to_scroll_to, QWidget *parent )
    :
    TextTab( resource, CodeViewEditor::Highlight_XHTML, line_to_scroll_to, parent ),
    m_XMLResource( resource ),
    m_WellFormedCheckComponent( *new WellFormedCheckComponent( *this ) )
{
    ConnectSignalsToSlots();
}


XMLTab::~XMLTab()
{
    m_WellFormedCheckComponent.deleteLater();
}


void XMLTab::ScrollToLine( int line )
{
    TextTab::ScrollToLine( line );
}

    
void XMLTab::AutoFixWellFormedErrors()
{
    m_wCodeView.ReplaceDocumentText( CleanSource::PrettyPrint( CleanSource::ProcessXML( m_wCodeView.toPlainText() ) ) );
}


void XMLTab::TakeControlOfUI()
{
    EmitCentralTabRequest();
    setFocus();
}


QString XMLTab::GetFilename()
{
    return ContentTab::GetFilename();
}


bool XMLTab::GetCheckWellFormedErrors()
{
    return m_WellFormedCheckComponent.GetCheckWellFormedErrors();
}


void XMLTab::SetWellFormedDialogsEnabledState( bool enabled )
{
    m_WellFormedCheckComponent.SetWellFormedDialogsEnabledState( enabled );
}


void XMLTab::SetCheckWellFormedErrorsState( bool enabled )
{
    m_WellFormedCheckComponent.SetCheckWellFormedErrorsState( enabled );
}


bool XMLTab::IsDataWellFormed()
{
    if ( !GetCheckWellFormedErrors() )
    {
        return true;
    }

    XhtmlDoc::WellFormedError error = m_XMLResource.WellFormedErrorLocation();
    bool well_formed = error.line == -1;

    if ( !well_formed )

        m_WellFormedCheckComponent.DemandAttentionIfAllowed( error );

    return well_formed;
}


void XMLTab::ConnectSignalsToSlots()
{
    // We set the Code View as the focus proxy for the tab,
    // so the ContentTab focusIn/Out handlers are not called.
    connect( &m_wCodeView, SIGNAL( FocusLost( QWidget* ) ), this, SLOT( IsDataWellFormed() ) );
}

