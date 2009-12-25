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

#pragma once
#ifndef CSSRESOURCE_H
#define CSSRESOURCE_H

#include "TextResource.h"

class CSSResource : public TextResource 
{
    Q_OBJECT

public:
    
    CSSResource( const QString &fullfilepath, QHash< QString, Resource* > *hash_owner, QObject *parent = NULL );

    virtual ResourceType Type() const;
};

#endif // CSSRESOURCE_H
