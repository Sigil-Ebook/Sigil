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

#include <QtGui/QLayout>
#include <QtGui/QVBoxLayout>

#include "ResourceObjects/Resource.h"
#include "Tabs/FlowTab.h"
#include "ViewEditors/Searchable.h"


ContentTab::ContentTab(Resource &resource, QWidget *parent)
    :
    QWidget(parent),
    m_Resource(resource),
    m_Layout(*new QVBoxLayout(this))
{
    connect(&resource, SIGNAL(Deleted(const Resource &)),          this, SLOT(EmitDeleteMe()));
    connect(&resource, SIGNAL(Renamed(const Resource &, QString)), this, SLOT(EmitTabRenamed()));
    m_Layout.setContentsMargins(0, 0, 0, 0);
    setLayout(&m_Layout);
}


QString ContentTab::GetFilename()
{
    return m_Resource.Filename();
}


QIcon ContentTab::GetIcon()
{
    return m_Resource.Icon();
}


Resource &ContentTab::GetLoadedResource()
{
    return m_Resource;
}


Searchable *ContentTab::GetSearchableContent()
{
    return NULL;
}


void ContentTab::Close()
{
    SaveTabContent();
    EmitDeleteMe();
}


void ContentTab::SaveTabContent()
{
}


void ContentTab::LoadTabContent()
{
}

void ContentTab::EmitCentralTabRequest()
{
    emit CentralTabRequest(this);
}


void ContentTab::ContentChangedExternally()
{
    // This seems to be the easiest way to get the tab's display to
    // update and reflect the changed contents. But it still causes a
    // scroll to top in Code View (though the cursor remains unchanged).
    // The focus mechanism for Code View needs to be looked at.
    activateWindow();
}

void ContentTab::ChangeCasing(const Utility::Casing casing)
{
}

void ContentTab::EmitDeleteMe()
{
    emit DeleteMe(this);
}


void ContentTab::EmitTabRenamed()
{
    emit TabRenamed(this);
}


void ContentTab::focusInEvent(QFocusEvent *event)
{
    QWidget::focusInEvent(event);
    LoadTabContent();
}


void ContentTab::focusOutEvent(QFocusEvent *event)
{
    QWidget::focusOutEvent(event);
    SaveTabContent();
}
