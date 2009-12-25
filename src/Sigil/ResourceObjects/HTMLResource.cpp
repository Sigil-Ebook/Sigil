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
#include "HTMLResource.h"
#include "../Misc/Utility.h"

HTMLResource::HTMLResource( const QString &fullfilepath, 
                            QHash< QString, Resource* > *hash_owner,
                            int reading_order,
                            QObject *parent )
    : 
    TextResource( fullfilepath, hash_owner, parent ),
    m_ReadingOrder( reading_order )
{

}


Resource::ResourceType HTMLResource::Type() const
{
    return Resource::HTMLResource;
}

int HTMLResource::GetReadingOrder()
{
    return m_ReadingOrder;
}

void HTMLResource::SetReadingOrder( int reading_order )
{
    m_ReadingOrder = reading_order;
}