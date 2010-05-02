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
#include "../ViewEditors/Searchable.h"
#include "../ResourceObjects/Resource.h"


ContentTab::ContentTab( Resource& resource, QWidget *parent )
    :
    QWidget( parent ),
    m_Resource( resource ),
    m_Layout( *new QVBoxLayout( this ) )
{
    connect( &resource, SIGNAL( Deleted() ),            this, SLOT( EmitDeleteMe()   ) );
    connect( &resource, SIGNAL( RenamedTo( QString ) ), this, SLOT( EmitTabRenamed() ) );

    m_Layout.setContentsMargins( 0, 0, 0, 0 );

    setLayout( &m_Layout );
}


QString ContentTab::GetFilename()
{
    return m_Resource.Filename();
}


QIcon ContentTab::GetIcon()
{
    return m_Resource.Icon();
}


Resource& ContentTab::GetLoadedResource()
{
    return m_Resource;
}


Searchable* ContentTab::GetSearchableContent()
{
    return NULL;
}


void ContentTab::Close()
{
    // TODO: save tab data here
    
    EmitDeleteMe();
}


void ContentTab::EmitDeleteMe()
{
    emit DeleteMe( this );
}

void ContentTab::EmitTabRenamed()
{
    emit TabRenamed( this );
}


void ContentTab::SaveContentOnTabLeave()
{
    m_Resource.GetLock().unlock();
}


void ContentTab::LoadContentOnTabEnter()
{
    m_Resource.GetLock().lockForWrite();
}


void ContentTab::focusInEvent( QFocusEvent *event )
{
    QWidget::focusInEvent( event );

    LoadContentOnTabEnter();
}


void ContentTab::focusOutEvent( QFocusEvent *event )
{
    QWidget::focusOutEvent( event );

    SaveContentOnTabLeave();
}

