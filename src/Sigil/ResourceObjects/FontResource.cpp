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

#include "Misc/Utility.h"
#include "ResourceObjects/FontResource.h"

FontResource::FontResource( const QString &fullfilepath, QObject *parent )
    : Resource( fullfilepath, parent )
{

}


Resource::ResourceType FontResource::Type() const
{
    return Resource::FontResourceType;
}


QString FontResource::GetObfuscationAlgorithm() const
{
    return m_ObfuscationAlgorithm;
}


void FontResource::SetObfuscationAlgorithm( const QString &algorithm )
{
    m_ObfuscationAlgorithm = algorithm;
}
