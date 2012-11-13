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
#ifndef FONTRESOURCE_H
#define FONTRESOURCE_H

#include "ResourceObjects/Resource.h"

/**
 * Represents a font file on disk (TTF or OTF).
 */
class FontResource : public Resource
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
    FontResource(const QString &fullfilepath, QObject *parent = NULL);

    // inherited
    virtual ResourceType Type() const;

    QString GetObfuscationAlgorithm() const;

    void SetObfuscationAlgorithm(const QString &algorithm);

    virtual bool LoadFromDisk();

private:

    QString m_ObfuscationAlgorithm;
};

#endif // FONTRESOURCE_H
