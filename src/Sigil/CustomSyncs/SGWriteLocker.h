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

#pragma once
#ifndef SGWRITELOCKER_H
#define SGWRITELOCKER_H

#include "SGReadWriteLock.h"

/**
 * A convenience class that simplifies locking and 
 * unlocking read-write locks for write access.
 * Passing a lock in the constructor locks it and 
 * ensures that the lock will be released when this
 * object is destroyed.
 */
class SGWriteLocker
{

public:

    /**
     * Constructor.
     * Locks the lock for writing.
     * 
     * @param lock The ReadWriteLock to lock.
     */
    SGWriteLocker( SGReadWriteLock* lock );

    /**
     * Destructor.
     * Unlocks the lock.
     */
    ~SGWriteLocker();

    /**
     * Returns the managed lock.
     *
     * @return The managed lock.
     */
    SGReadWriteLock* ReadWriteLock() const; 

    /**
     * Relocks the lock for writing if the calling thread doesn't have a lock.
     *
     * @return \c true if the lock was locked.
     */
    bool RelockIfNeeded();

    /**
     * Unlocks the lock if the calling thread has a lock.
     *
     * @return \c true if the lock was unlocked.
     */
    bool UnlockIfNeeded();

private:

    /**
     * The lock to manage.
     */
    SGReadWriteLock* m_Lock;
};

#endif // SGWRITELOCKER_H