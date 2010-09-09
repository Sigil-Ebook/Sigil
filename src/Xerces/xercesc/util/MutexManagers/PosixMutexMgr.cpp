/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id: PosixMutexMgr.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// on some platforms, THREAD_MUTEX_RECURSIVE is defined only if _GNU_SOURCE is defined
#ifndef _GNU_SOURCE
 #define _GNU_SOURCE
#endif

#include <pthread.h>

#include <xercesc/util/MutexManagers/PosixMutexMgr.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/PanicHandler.hpp>

XERCES_CPP_NAMESPACE_BEGIN


//	Wrap up the mutex with XMemory
class PosixMutexWrap : public XMemory {
public:
	pthread_mutex_t	m;
};


PosixMutexMgr::PosixMutexMgr()
{
}


PosixMutexMgr::~PosixMutexMgr()
{
}


XMLMutexHandle
PosixMutexMgr::create(MemoryManager* const manager)
{
    PosixMutexWrap* mutex = new (manager) PosixMutexWrap;
    
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    
    if (pthread_mutex_init(&mutex->m, &attr))
        XMLPlatformUtils::panic(PanicHandler::Panic_MutexErr);
        
    pthread_mutexattr_destroy(&attr);

    return (void*)(mutex);
}


void
PosixMutexMgr::destroy(XMLMutexHandle mtx, MemoryManager* const manager)
{
	PosixMutexWrap* mutex = (PosixMutexWrap*)(mtx);
    if (mutex != NULL)
    {
        if (pthread_mutex_destroy(&mutex->m))
        {
            ThrowXMLwithMemMgr(XMLPlatformUtilsException,
                     XMLExcepts::Mutex_CouldNotDestroy, manager);
        }
        delete mutex;
    }
}


void
PosixMutexMgr::lock(XMLMutexHandle mtx)
{
	PosixMutexWrap* mutex = (PosixMutexWrap*)(mtx);
    if (mutex != NULL)
    {
        if (pthread_mutex_lock(&mutex->m))
            XMLPlatformUtils::panic(PanicHandler::Panic_MutexErr);
    }
}


void
PosixMutexMgr::unlock(XMLMutexHandle mtx)
{
	PosixMutexWrap* mutex = (PosixMutexWrap*)(mtx);
    if (mutex != NULL)
    {
        if (pthread_mutex_unlock(&mutex->m))
            XMLPlatformUtils::panic(PanicHandler::Panic_MutexErr);
    }
}


XERCES_CPP_NAMESPACE_END

