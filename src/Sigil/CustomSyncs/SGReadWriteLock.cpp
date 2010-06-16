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
#include "SGReadWriteLock.h"

SGReadWriteLock::SGReadWriteLock()
    : m_CurrentWriter( NULL )
{

}


bool SGReadWriteLock::LockForReadIfNeeded()
{
    if ( CurrentThreadHasLock() )

        return false;

    m_ReadWriteLock.lockForRead();

    QMutexLocker locker( &m_StateAccessMutex );
    m_CurrentReaders.insert( QThread::currentThread() );

    return true;
}


bool SGReadWriteLock::LockForWriteIfNeeded()
{
    if ( CurrentThreadHasLock() )

        return false;

    m_ReadWriteLock.lockForWrite();

    QMutexLocker locker( &m_StateAccessMutex );
    m_CurrentWriter = QThread::currentThread();

    return true;
}


bool SGReadWriteLock::UnlockIfNeeded()
{
    if ( !CurrentThreadHasLock() )

        return false;    

    {
        QMutexLocker locker( &m_StateAccessMutex );

        QThread* current_thread = QThread::currentThread();
        if ( current_thread == m_CurrentWriter )
        
            m_CurrentWriter = NULL;
       
        else
        
            m_CurrentReaders.remove( current_thread );        
    }

    m_ReadWriteLock.unlock();

    return true;
}


bool SGReadWriteLock::CurrentThreadHasLock()
{
    QMutexLocker locker( &m_StateAccessMutex );

    QThread* current_thread = QThread::currentThread();

    if ( current_thread == m_CurrentWriter )

        return true;

    return m_CurrentReaders.contains( current_thread );
}

