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
#include "TabManager.h"
#include "ContentTab.h"
#include "FlowTab.h"

TabManager::TabManager( QWidget *parent )
    : 
    QTabWidget( parent )
{
    connect( this, SIGNAL( currentChanged( int ) ), this, SLOT( EmitTabChanged() ) );

    setDocumentMode( true );
    setMovable( true );
    setTabsClosable( true );

    addTab( new FlowTab( "C:\\Users\\Valloric\\Desktop\\sigil_benches\\search_test.html", this ), "test" );
    addTab( new FlowTab( "C:\\Users\\Valloric\\Desktop\\sigil_benches\\search_test.html", this ), "test2" );
}

    
ContentTab & TabManager::GetCurrentContentTab()
{
    return *qobject_cast< ContentTab* >( currentWidget() );
}


void TabManager::EmitTabChanged()
{
    ContentTab *current_tab = qobject_cast< ContentTab* >( currentWidget() );

    emit TabChanged( m_LastContentTab.data(), current_tab );

    m_LastContentTab = QWeakPointer< ContentTab >( current_tab );
}