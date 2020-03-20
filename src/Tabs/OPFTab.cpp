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

#include "ResourceObjects/OPFResource.h"
#include "Tabs/OPFTab.h"

OPFTab::OPFTab(OPFResource *resource, int line_to_scroll_to, int position_to_scroll_to, QWidget *parent)
    :
    XMLTab(resource, line_to_scroll_to, position_to_scroll_to, parent),
    m_OPFResource(resource),
    m_LastPosition(-1)
{
    ConnectSignalsToSlots();
}


void OPFTab::ReloadTabIfPending()
{
    if (!isVisible()) {
        return;
    }
    setFocus();
}

void OPFTab::ResourceModified()
{
    if (m_LastPosition > 0) {
        m_wCodeView->ScrollToPosition(m_LastPosition);
        m_LastPosition = -1;
    }
}

void OPFTab::ResourceTextChanging()
{
    // Store an exact position of cursor
    m_LastPosition = m_wCodeView->GetCursorPosition();
}

void OPFTab::AutoFixWellFormedErrors()
{
    m_OPFResource->AutoFixWellFormedErrors();
}

void OPFTab::ConnectSignalsToSlots()
{
    connect(m_OPFResource, SIGNAL(TextChanging()), this, SLOT(ResourceTextChanging()));
    connect(m_OPFResource, SIGNAL(Modified()), this, SLOT(ResourceModified()));
    connect(m_OPFResource, SIGNAL(LoadedFromDisk()), this, SLOT(ReloadTabIfPending()));
}
