/************************************************************************
**
**  Copyright (C) 2026 Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef CODEPOINTNAMES_H
#define CODEPOINTNAMES_H

#include <QHash>

class QString;


/**
 * Singleton.
 *
 * CodepointNames
 */
 

class CodepointNames
{

public:

    static CodepointNames *instance();

    QString GetName(int cp);

private:

    CodepointNames();

    void SetNameCache();

    QHash<int, QString> m_NameCache;
    
    static CodepointNames *m_instance;

};

#endif // CODEPOINTNAMES_H
