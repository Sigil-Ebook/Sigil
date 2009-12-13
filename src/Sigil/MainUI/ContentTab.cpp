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


ContentTab::ContentTab( const QString &filepath, QWidget *parent )
    :
    QWidget( parent ),
    m_Filepath( filepath ),
    m_Layout( *new QVBoxLayout( this ) )
{
    m_Layout.setContentsMargins( 0, 0, 0, 0 );

    setLayout( &m_Layout );
}

const QString & ContentTab::GetFilepath()
{
    return m_Filepath;
}

void ContentTab::UpdateFilepath( const QString &filepath )
{
    m_Filepath = filepath;
}

Searchable* ContentTab::GetSearchableContent()
{
    return NULL;
}