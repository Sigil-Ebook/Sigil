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
#include "ResourceObjects/CSSResource.h"

CSSResource::CSSResource( const QString &fullfilepath, QObject *parent )
    : TextResource( fullfilepath, parent )
{

}

bool CSSResource::DeleteCSStyles( QList<CSSInfo::CSSSelector*> css_selectors)
{
    CSSInfo css_info(GetText());

    // Search for selectors with the same definition and line and remove from text
    const QString &new_resource_text = css_info.removeMatchingSelectors(css_selectors);

    if (!new_resource_text.isNull()) {
        // At least one of the selector(s) was removed.
        SetText(new_resource_text);
        return true;
    }
    return false;
}

Resource::ResourceType CSSResource::Type() const
{
    return Resource::CSSResourceType;
}
