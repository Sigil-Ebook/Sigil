/************************************************************************
**
**  Copyright (C) 2015-2019 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
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


XMLTab::XMLTab(XMLResource *resource, int line_to_scroll_to, int position_to_scroll_to, QWidget *parent)
    :
    TextTab(resource, CodeViewEditor::Highlight_XHTML, line_to_scroll_to, position_to_scroll_to, parent),
    m_XMLResource(resource),
    m_WellFormedCheckComponent(new WellFormedCheckComponent(this, parent))
{
    ConnectSignalsToSlots();
}


XMLTab::~XMLTab()
{
    m_WellFormedCheckComponent->deleteLater();
}


void XMLTab::ScrollToLine(int line)
{
    TextTab::ScrollToLine(line);
}

void XMLTab::ScrollToPosition(int cursor_position)
{
    TextTab::ScrollToPosition(cursor_position);
}


void XMLTab::AutoFixWellFormedErrors()
{
    QString mtype = m_XMLResource->GetMediaType();
    m_wCodeView->ReplaceDocumentText(CleanSource::ProcessXML(m_wCodeView->toPlainText(),mtype));
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

QString XMLTab::GetShortPathName()
{
    return ContentTab::GetShortPathName();
}


bool XMLTab::IsDataWellFormed()
{
    XhtmlDoc::WellFormedError error = m_XMLResource->WellFormedErrorLocation();
    bool well_formed = error.line == -1;

    if (!well_formed) {
        m_WellFormedCheckComponent->DemandAttentionIfAllowed(error);
    }

    return well_formed;
}


void XMLTab::ConnectSignalsToSlots()
{
    connect(m_wCodeView, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)), this, SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)));
}

