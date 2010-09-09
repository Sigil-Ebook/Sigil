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
 * $Id: SynchronizedStringPool.cpp 903137 2010-01-26 09:26:28Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/SynchronizedStringPool.hpp>


XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLSynchronizedStringPool: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLSynchronizedStringPool::XMLSynchronizedStringPool(const XMLStringPool *constPool
                , const  unsigned int  modulus
                , MemoryManager* const manager) :

    XMLStringPool(modulus, manager)
    , fConstPool(constPool)
    , fMutex(manager)
{
}

XMLSynchronizedStringPool::~XMLSynchronizedStringPool()
{
}


// ---------------------------------------------------------------------------
//  XMLSynchronizedStringPool: Pool management methods
// ---------------------------------------------------------------------------
unsigned int XMLSynchronizedStringPool::addOrFind(const XMLCh* const newString)
{
    unsigned int id = fConstPool->getId(newString);
    if(id)
        return id;
    // might have to add it to our own table.
    // synchronize this bit
    unsigned int constCount = fConstPool->getStringCount();
    XMLMutexLock lockInit(&fMutex);
    id = XMLStringPool::addOrFind(newString);
    return id+constCount;
}

bool XMLSynchronizedStringPool::exists(const XMLCh* const newString) const
{
    if(fConstPool->exists(newString))
        return true;

    XMLMutexLock lockInit(&const_cast<XMLSynchronizedStringPool*>(this)->fMutex);
    return XMLStringPool::exists(newString);
}

bool XMLSynchronizedStringPool::exists(const unsigned int id) const
{
    if (!id)
        return false;

    // First see if this id belongs to the const pool.
    //
    unsigned int constCount = fConstPool->getStringCount();

    if (id <= constCount)
      return true;

    // The rest needs to be synchronized.
    //
    XMLMutexLock lockInit(&const_cast<XMLSynchronizedStringPool*>(this)->fMutex);
    return id < fCurId + constCount;
}

void XMLSynchronizedStringPool::flushAll()
{
    // don't touch const pool!
    XMLStringPool::flushAll();
}


unsigned int XMLSynchronizedStringPool::getId(const XMLCh* const toFind) const
{
    unsigned int retVal = fConstPool->getId(toFind);
    if(retVal)
        return retVal;

    // make sure we return a truly unique id
    unsigned int constCount = fConstPool->getStringCount();
    XMLMutexLock lockInit(&const_cast<XMLSynchronizedStringPool*>(this)->fMutex);
    return XMLStringPool::getId(toFind)+constCount;
}


const XMLCh* XMLSynchronizedStringPool::getValueForId(const unsigned int id) const
{
    if (id <= fConstPool->getStringCount())
        return fConstPool->getValueForId(id);

    unsigned int constCount = fConstPool->getStringCount();
    XMLMutexLock lockInit(&const_cast<XMLSynchronizedStringPool*>(this)->fMutex);
    return XMLStringPool::getValueForId(id-constCount);
}

unsigned int XMLSynchronizedStringPool::getStringCount() const
{
    unsigned int constCount = fConstPool->getStringCount();
    XMLMutexLock lockInit(&const_cast<XMLSynchronizedStringPool*>(this)->fMutex);
    return fCurId+constCount-1;
}

XERCES_CPP_NAMESPACE_END
