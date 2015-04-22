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


#pragma once
#ifndef PCRECACHE_H
#define PCRECACHE_H

#include <QtCore/QCache>
#include <QtCore/QString>

#include "pcre/SPCRE.h"

/**
 * Singleton. A cache of SPCRE regular expression objects.
 *
 * The SPCRE's are cached to improve performance.
 */
class PCRECache
{
public:
    /**
     * The accessor function to access the cache.
     */
    static PCRECache *instance();
    ~PCRECache();

    /**
     * Insert an SPCRE into the cache. The key is the regular expression
     * pattern by the SPCRE as a string.
     *
     * @param key The key associated with the SPCRE.
     * @param object The SPCRE to store.
     *
     * @return True if the object was successfully inserted.
     */
    bool insert(const QString &key, SPCRE *object);
    /**
     * Retrieve the SPCRE object from the cache.
     *
     * If the object does not exist it is created and inserted into te cache
     * then returned.
     *
     * @param key The key associated with the SPCRE.
     */
    SPCRE *getObject(const QString &key);

private:
    /**
     * Private constructor.
     */
    PCRECache();

    // The cache that we store the SPCRE's.
    QCache<QString, SPCRE> m_cache;
    // The single instance of the cache.
    static PCRECache *m_instance;
};

#endif // PCRECACHE_H
