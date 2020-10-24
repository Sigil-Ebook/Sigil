/************************************************************************
**
**  Copyright (C) 2020 Kevin B. Hendricks, Stratford, ON, Canada
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
#ifndef ASCIIFY_H
#define ASCIIFY_H

#include <cstddef>

/**
 * Singleton.
 *
 * AsciiFy
 */

class QString;

class AsciiFy
{

public:

    static AsciiFy *instance();
    
    QString convertToPlainAscii(const QString &ninput) const;
    
    bool containsOnlyAscii(const QString &ntext);


private:

    // constructor must be private since singleton
    AsciiFy();

    static AsciiFy *m_instance;
    static const char *unidecode_text;
    static const size_t unidecode_pos[];

};

#endif // ASCIIFY_H
