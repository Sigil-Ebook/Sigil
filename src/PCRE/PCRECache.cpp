/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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

#include "PCRE/PCRECache.h"

PCRECache *PCRECache::m_instance = 0;

PCRECache *PCRECache::instance()
{
    if (m_instance == 0) {
        m_instance = new PCRECache();
    }

    return m_instance;
}

PCRECache::PCRECache()
{
}

bool PCRECache::insert(const QString &key, SPCRE *object)
{
    return m_cache.insert(key, object);
}

SPCRE *PCRECache::getObject(const QString &key)
{
    // Create a new SPCRE if it doesn't alreayd exit.
    // The key is the pattern for initializing the SPCRE.
    if (!m_cache.contains(key)) {
        SPCRE *spcre = new SPCRE(key);
        m_cache.insert(key, spcre, 1);
        return spcre;
    }

    return m_cache.object(key);
}
