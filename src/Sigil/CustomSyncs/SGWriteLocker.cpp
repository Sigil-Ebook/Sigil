/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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

#include <stdafx.h>
#include "SGWriteLocker.h"


SGWriteLocker::SGWriteLocker( SGReadWriteLock* lock )
    : m_Lock( lock ) 
{
    m_Lock->LockForWriteIfNeeded();
}


SGWriteLocker::~SGWriteLocker()
{
    m_Lock->UnlockIfNeeded();
}


SGReadWriteLock* SGWriteLocker::ReadWriteLock() const
{
    return m_Lock;
}


bool SGWriteLocker::RelockIfNeeded()
{
    return m_Lock->LockForWriteIfNeeded();
}


bool SGWriteLocker::UnlockIfNeeded()
{
    return m_Lock->UnlockIfNeeded();
}