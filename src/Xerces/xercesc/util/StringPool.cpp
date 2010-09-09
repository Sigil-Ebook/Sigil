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
 * $Id: StringPool.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/StringPool.hpp>
#include <assert.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLStringPool: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLStringPool::XMLStringPool(const  unsigned int  modulus,
                             MemoryManager* const manager) :

    fMemoryManager(manager)
    , fIdMap(0)
    , fHashTable(0)
    , fMapCapacity(64)
    , fCurId(1)
{
    // Create the hash table, passing it the modulus
    fHashTable = new (fMemoryManager) RefHashTableOf<PoolElem>(modulus, false, fMemoryManager);

    // Do an initial allocation of the id map and zero it all out
    fIdMap = (PoolElem**) fMemoryManager->allocate
    (
        fMapCapacity * sizeof(PoolElem*)
    ); //new PoolElem*[fMapCapacity];
    memset(fIdMap, 0, sizeof(PoolElem*) * fMapCapacity);
}

XMLStringPool::~XMLStringPool()
{
    // delete all buckelements, since the hashtable doesn't adopt the elements anymore
    for (unsigned int index = 1; index < fCurId; index++)
    {
        //fIdMap[index]->~PoolElem();                                // we have no destructor
        fMemoryManager->deallocate((void*) fIdMap[index]->fString);  // deallocate memory
        fMemoryManager->deallocate(fIdMap[index]);                   // deallocate memory
    }
    delete fHashTable;
    fMemoryManager->deallocate(fIdMap); //delete [] fIdMap;
}

// ---------------------------------------------------------------------------
//  XMLStringPool: Pool management methods
// ---------------------------------------------------------------------------
void XMLStringPool::flushAll()
{
    // delete all buckelements, since the hashtable doesn't adopt the elements anymore
    for (unsigned int index = 1; index < fCurId; index++)
    {
        //fIdMap[index]->~PoolElem();                                // we have no destructor
        fMemoryManager->deallocate((void*) fIdMap[index]->fString);  // deallocate memory
        fMemoryManager->deallocate(fIdMap[index]);                   // deallocate memory
    }
    fCurId = 1;
    fHashTable->removeAll();
}

// ---------------------------------------------------------------------------
//  XMLStringPool: Private helper methods
// ---------------------------------------------------------------------------
unsigned int XMLStringPool::addNewEntry(const XMLCh* const newString)
{
    // See if we need to expand the id map
    if (fCurId == fMapCapacity)
    {
        // Calculate the new capacity, create a temp new map, and zero it
        const unsigned int newCap = (unsigned int)(fMapCapacity * 1.5);
        PoolElem** newMap = (PoolElem**) fMemoryManager->allocate
        (
            newCap * sizeof(PoolElem*)
        ); //new PoolElem*[newCap];
        memset(newMap, 0, sizeof(PoolElem*) * newCap);

        //
        //  Copy over the old elements from the old map. They are just pointers
        //  so we can do it all at once.
        //
        memcpy(newMap, fIdMap, sizeof(PoolElem*) * fMapCapacity);

        // Clean up the old map and store the new info
        fMemoryManager->deallocate(fIdMap); //delete [] fIdMap;
        fIdMap = newMap;
        fMapCapacity = newCap;
    }

    //
    //  Ok, now create a new element and add it to the hash table. Then store
    //  this new element in the id map at the current id index, then bump the
    //  id index.
    //
    PoolElem* newElem = (PoolElem*) fMemoryManager->allocate(sizeof(PoolElem));
    newElem->fId      = fCurId;
    newElem->fString  = XMLString::replicate(newString, fMemoryManager);
    fHashTable->put((void*)newElem->fString, newElem);
    fIdMap[fCurId] = newElem;

    // Bump the current id and return the id of the new elem we just added
    fCurId++;
    return newElem->fId;
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(XMLStringPool)

void XMLStringPool::serialize(XSerializeEngine& serEng)
{
    /***
     * Since we are pretty sure that fIdMap and fHashTable is 
     * not shared by any other object, therefore there is no owned/referenced
     * issue. Thus we can serialize the raw data only, rather than serializing 
     * both fIdMap and fHashTable.
     *
     * And we can rebuild the fIdMap and fHashTable out of the raw data during
     * deserialization.
     *
    ***/
    if (serEng.isStoring())
    {
        serEng<<fCurId;
        for (unsigned int index = 1; index < fCurId; index++)
        {
            const XMLCh* stringData = getValueForId(index);
            serEng.writeString(stringData);
        }
    }
    else
    {
        unsigned int mapSize;
        serEng>>mapSize;
        assert(1 == fCurId);  //make sure empty

        for (unsigned int index = 1; index < mapSize; index++)
        {
            XMLCh* stringData;
            serEng.readString(stringData);
            addNewEntry(stringData);

            //we got to deallocate this string 
            //since stringpool will duplicate this string in the PoolElem and own that copy
            fMemoryManager->deallocate(stringData);
        }
    }
}

XMLStringPool::XMLStringPool(MemoryManager* const manager) :
    fMemoryManager(manager)
    , fIdMap(0)
    , fHashTable(0)
    , fMapCapacity(64)
    , fCurId(1)
{
    // Create the hash table, passing it the modulus
    fHashTable = new (fMemoryManager) RefHashTableOf<PoolElem>(109, false, fMemoryManager);

    // Do an initial allocation of the id map and zero it all out
    fIdMap = (PoolElem**) fMemoryManager->allocate
    (
        fMapCapacity * sizeof(PoolElem*)
    ); //new PoolElem*[fMapCapacity];
    memset(fIdMap, 0, sizeof(PoolElem*) * fMapCapacity);
}

XERCES_CPP_NAMESPACE_END
