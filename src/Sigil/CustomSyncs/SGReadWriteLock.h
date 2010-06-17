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
#ifndef SGREADWRITELOCK_H
#define SGREADWRITELOCK_H

#include <QReadWriteLock>
#include <QMutex>
#include <QSet>

class QThread;

/**
 * A wrapper around QReadWriteLock, providing
 * a "shallow", non-recursive lock. This lock can be
 * locked only once, and multiple calls to the lock
 * functions return \c false if the thread already 
 * has a lock. Same thing goes for unlocking: the lock
 * is unlocked only if the thread has a lock.
 *
 * Other than that, provides the same features: there can be
 * multiple concurrent readers, but only one writer who has priority.
 *
 * @warning DOES \b NOT SUPPORT RECURSIVE LOCKING! Using recursive locking 
 * with this primitive will make the thread lose the lock on the first unlock call!
 */
class SGReadWriteLock
{

public:

    /**
     * Constructor.
     */
    SGReadWriteLock();

    /**
     * Locks for reading if the calling thread doesn't have a lock.
     *
     * @return \c false if the thread already had a lock.
     */
    bool LockForReadIfNeeded();


    /**
     * Locks for writing if the calling thread doesn't have a lock.
     *
     * @return \c false if the thread already had a lock.
     */
    bool LockForWriteIfNeeded();

    /**
     * Unlocks if the calling thread has a lock.
     *
     * @return \c false if the thread didn't have a lock.
     */
    bool UnlockIfNeeded();

    /**
     * Used to determine if the calling thread has any type of lock or not.
     *
     * @return \c true if the calling thread has a lock.
     */
    bool CurrentThreadHasLock();

private:

    /**
     * A set of all the current reader threads.
     */
    QSet< QThread* > m_CurrentReaders;

    /**
     * The current writer or NULL if no thread has write access.
     */
    QThread* m_CurrentWriter;

    /**
     * Used to protect access to m_CurrentWriter and m_CurrentReaders
     */
    QMutex m_StateAccessMutex;

    /**
     * The actual lock.
     */
    QReadWriteLock m_ReadWriteLock;
};

#endif // SGREADWRITELOCK_H