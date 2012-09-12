/************************************************************************
**
**  Copyright (C) 2012  Daniel Pavel <daniel.pavel@gmail.com>
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

#pragma once
#ifndef OPENEXTERNALLY_H
#define OPENEXTERNALLY_H

#include "ResourceObjects/Resource.h"

class OpenExternally
{

public:

    static bool mayOpen( const Resource::ResourceType type );

    static bool openFile( const QString& filePath, const QString& application );

    static const QString editorForResourceType( const Resource::ResourceType type );

    static const QString selectEditorForResourceType( const Resource::ResourceType type );

    static const QString prettyApplicationName( const QString& applicationpath );
};

#endif // OPENEXTERNALLY_H
