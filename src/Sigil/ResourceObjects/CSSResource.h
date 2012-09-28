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

#pragma once
#ifndef CSSRESOURCE_H
#define CSSRESOURCE_H

#include "Misc/CSSInfo.h"
#include "ResourceObjects/TextResource.h"

/**
 * Represents a CSS stylesheet on disk. 
 */
class CSSResource : public TextResource 
{
    Q_OBJECT

public:
    
    /**
     * Constructor.
     *
     * @param fullfilepath The full path to the file that this
     *                     resource is representing.
     * @param parent The object's parent.
     */
    CSSResource( const QString &fullfilepath, QObject *parent = NULL );

    ~CSSResource();

    bool DeleteCSStyles( QList<CSSInfo::CSSSelector*> css_selectors);

    // inherited
    virtual ResourceType Type() const;

    void ValidateStylesheetWithW3C();

private:

    QList<QString> m_TemporaryValidationFiles;
};

#endif // CSSRESOURCE_H
