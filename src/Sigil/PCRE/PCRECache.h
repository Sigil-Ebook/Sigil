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

#include "SPCRE.h"
#include <QCache>
#include <QString>

class PCRECache
{
public:
    static PCRECache *instance();
    ~PCRECache();

    bool insert(const QString &key, SPCRE *object);
    SPCRE *getObject(const QString &key);

private:
    PCRECache();

    QCache<QString, SPCRE> m_cache;
    static PCRECache *m_instance;
};

#endif // PCRECACHE_H
