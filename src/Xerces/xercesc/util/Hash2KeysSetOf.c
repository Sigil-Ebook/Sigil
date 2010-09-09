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
 * $Id: Hash2KeysSetOf.c 883368 2009-11-23 15:28:19Z amassari $
 */


// ---------------------------------------------------------------------------
//  Include
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/Hash2KeysSetOf.hpp>
#endif

#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/NullPointerException.hpp>
#include <assert.h>
#include <new>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Hash2KeysSetOf: Constructors and Destructor
// ---------------------------------------------------------------------------

template <class THasher>
Hash2KeysSetOf<THasher>::Hash2KeysSetOf(
  const XMLSize_t modulus,
  MemoryManager* const manager)

    : fMemoryManager(manager)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fCount(0)
    , fAvailable(0)
{
    initialize(modulus);
}

template <class THasher>
Hash2KeysSetOf<THasher>::Hash2KeysSetOf(
  const XMLSize_t modulus,
  const THasher& hasher,
  MemoryManager* const manager)

    : fMemoryManager(manager)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fCount(0)
    , fAvailable(0)
    , fHasher (hasher)
{
    initialize(modulus);
}

template <class THasher>
void Hash2KeysSetOf<THasher>::initialize(const XMLSize_t modulus)
{
    if (modulus == 0)
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::HshTbl_ZeroModulus, fMemoryManager);

    // Allocate the bucket list and zero them
    fBucketList = (Hash2KeysSetBucketElem**) fMemoryManager->allocate
    (
        fHashModulus * sizeof(Hash2KeysSetBucketElem*)
    ); //new Hash2KeysSetBucketElem*[fHashModulus];
    memset(fBucketList, 0, sizeof(fBucketList[0]) * fHashModulus);
}

template <class THasher>
Hash2KeysSetOf<THasher>::~Hash2KeysSetOf()
{
    Hash2KeysSetBucketElem* nextElem;
    if(!isEmpty())
    {
        // Clean up the buckets first
        for (XMLSize_t buckInd = 0; buckInd < fHashModulus; buckInd++)
        {
            // Get the bucket list head for this entry
            Hash2KeysSetBucketElem* curElem = fBucketList[buckInd];
            while (curElem)
            {
                // Save the next element before we hose this one
                nextElem = curElem->fNext;
                fMemoryManager->deallocate(curElem);
                curElem = nextElem;
            }

            // Clean out this entry
            fBucketList[buckInd] = 0;
        }
    }
    // Then delete the list of available blocks
    Hash2KeysSetBucketElem* curElem = fAvailable;
    while (curElem)
    {
        // Save the next element before we hose this one
        nextElem = curElem->fNext;
        fMemoryManager->deallocate(curElem);
        curElem = nextElem;
    }
    fAvailable = 0;

    // Then delete the bucket list & hasher
    fMemoryManager->deallocate(fBucketList); //delete [] fBucketList;
    fBucketList = 0;
}


// ---------------------------------------------------------------------------
//  Hash2KeysSetOf: Element management
// ---------------------------------------------------------------------------
template <class THasher>
bool Hash2KeysSetOf<THasher>::isEmpty() const
{
    return (fCount==0);
}

template <class THasher>
bool Hash2KeysSetOf<THasher>::containsKey(const void* const key1, const int key2) const
{
    XMLSize_t hashVal;
    const Hash2KeysSetBucketElem* findIt = findBucketElem(key1, key2, hashVal);
    return (findIt != 0);
}

template <class THasher>
void Hash2KeysSetOf<THasher>::removeKey(const void* const key1, const int key2)
{
    // Hash the key
    XMLSize_t hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    //
    //  Search the given bucket for this key. Keep up with the previous
    //  element so we can patch around it.
    //
    Hash2KeysSetBucketElem* curElem = fBucketList[hashVal];
    Hash2KeysSetBucketElem* lastElem = 0;

    while (curElem)
    {
        if((key2==curElem->fKey2) && (fHasher.equals(key1, curElem->fKey1)))
        {
            if (!lastElem)
            {
                // It was the first in the bucket
                fBucketList[hashVal] = curElem->fNext;
            }
            else
            {
                // Patch around the current element
                lastElem->fNext = curElem->fNext;
            }

            // Move the current element to the list of available blocks
            curElem->fNext=fAvailable;
            fAvailable=curElem;

            fCount--;
            return;
        }

        // Move both pointers upwards
        lastElem = curElem;
        curElem = curElem->fNext;
    }

    // We never found that key
    ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::HshTbl_NoSuchKeyExists, fMemoryManager);
}

template <class THasher>
void Hash2KeysSetOf<THasher>::
removeKey(const void* const key1)
{
    // Hash the key
    XMLSize_t hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    //
    //  Search the given bucket for this key. Keep up with the previous
    //  element so we can patch around it.
    //
    Hash2KeysSetBucketElem* curElem = fBucketList[hashVal];
    Hash2KeysSetBucketElem* lastElem = 0;

    while (curElem)
    {
        if(fHasher.equals(key1, curElem->fKey1))
        {
            if (!lastElem)
            {
                // It was the first in the bucket
                fBucketList[hashVal] = curElem->fNext;
            }
            else
            {
                // Patch around the current element
                lastElem->fNext = curElem->fNext;
            }

            Hash2KeysSetBucketElem* toBeDeleted=curElem;
            curElem = curElem->fNext;

            // Move the current element to the list of available blocks
            toBeDeleted->fNext=fAvailable;
            fAvailable=toBeDeleted;

            fCount--;
        }
        else
        {
            // Move both pointers upwards
            lastElem = curElem;
            curElem = curElem->fNext;
        }
    }
}

template <class THasher>
void Hash2KeysSetOf<THasher>::removeAll()
{
    if(isEmpty())
        return;

    for (XMLSize_t buckInd = 0; buckInd < fHashModulus; buckInd++)
    {
        if(fBucketList[buckInd]!=0)
        {
            // Advance to the end of the chain, and connect it to the list of
            // available blocks
            Hash2KeysSetBucketElem* curElem = fBucketList[buckInd];
            while (curElem->fNext)
                curElem = curElem->fNext;
            curElem->fNext=fAvailable;
            fAvailable=fBucketList[buckInd];
            fBucketList[buckInd] = 0;
        }
    }
    fCount=0;
}

// ---------------------------------------------------------------------------
//  Hash2KeysSetOf: Getters
// ---------------------------------------------------------------------------
template <class THasher>
MemoryManager* Hash2KeysSetOf<THasher>::getMemoryManager() const
{
    return fMemoryManager;
}

template <class THasher>
XMLSize_t Hash2KeysSetOf<THasher>::getHashModulus() const
{
    return fHashModulus;
}

// ---------------------------------------------------------------------------
//  Hash2KeysSetOf: Putters
// ---------------------------------------------------------------------------
template <class THasher>
void Hash2KeysSetOf<THasher>::put(const void* key1, int key2)
{
    // Apply 4 load factor to find threshold.
    XMLSize_t threshold = fHashModulus * 4;

    // If we've grown too big, expand the table and rehash.
    if (fCount >= threshold)
        rehash();

    // First see if the key exists already
    XMLSize_t hashVal;
    Hash2KeysSetBucketElem* newBucket = findBucketElem(key1, key2, hashVal);

    //
    //  If so,then update its value. If not, then we need to add it to
    //  the right bucket
    //
    if (newBucket)
    {
        newBucket->fKey1 = key1;
        newBucket->fKey2 = key2;
    }
     else
    {
        if(fAvailable==0)
            newBucket = (Hash2KeysSetBucketElem*)fMemoryManager->allocate(sizeof(Hash2KeysSetBucketElem));
        else
        {
            newBucket = fAvailable;
            fAvailable = fAvailable->fNext;
        }
        newBucket->fKey1 = key1;
        newBucket->fKey2 = key2;
        newBucket->fNext = fBucketList[hashVal];
        fBucketList[hashVal] = newBucket;
        fCount++;
    }
}

template <class THasher>
bool Hash2KeysSetOf<THasher>::putIfNotPresent(const void* key1, int key2)
{
    // First see if the key exists already
    XMLSize_t hashVal;
    Hash2KeysSetBucketElem* newBucket = findBucketElem(key1, key2, hashVal);

    //
    //  If so,then update its value. If not, then we need to add it to
    //  the right bucket
    //
    if (newBucket)
        return false;

    // Apply 4 load factor to find threshold.
    XMLSize_t threshold = fHashModulus * 4;

    // If we've grown too big, expand the table and rehash.
    if (fCount >= threshold)
        rehash();

    if(fAvailable==0)
        newBucket = (Hash2KeysSetBucketElem*)fMemoryManager->allocate(sizeof(Hash2KeysSetBucketElem));
    else
    {
        newBucket = fAvailable;
        fAvailable = fAvailable->fNext;
    }
    newBucket->fKey1 = key1;
    newBucket->fKey2 = key2;
    newBucket->fNext = fBucketList[hashVal];
    fBucketList[hashVal] = newBucket;
    fCount++;
    return true;
}


// ---------------------------------------------------------------------------
//  Hash2KeysSetOf: Private methods
// ---------------------------------------------------------------------------
template <class THasher>
inline Hash2KeysSetBucketElem* Hash2KeysSetOf<THasher>::
findBucketElem(const void* const key1, const int key2, XMLSize_t& hashVal)
{
    // Hash the key
    hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    // Search that bucket for the key
    Hash2KeysSetBucketElem* curElem = fBucketList[hashVal];
    while (curElem)
    {
        if((key2==curElem->fKey2) && (fHasher.equals(key1, curElem->fKey1)))
            return curElem;

        curElem = curElem->fNext;
    }
    return 0;
}

template <class THasher>
inline const Hash2KeysSetBucketElem* Hash2KeysSetOf<THasher>::
findBucketElem(const void* const key1, const int key2, XMLSize_t& hashVal) const
{
    // Hash the key
    hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    // Search that bucket for the key
    const Hash2KeysSetBucketElem* curElem = fBucketList[hashVal];
    while (curElem)
    {
        if((key2==curElem->fKey2) && (fHasher.equals(key1, curElem->fKey1)))
            return curElem;

        curElem = curElem->fNext;
    }
    return 0;
}


template <class THasher>
void Hash2KeysSetOf<THasher>::
rehash()
{
    const XMLSize_t newMod = (fHashModulus * 8)+1;

    Hash2KeysSetBucketElem** newBucketList =
        (Hash2KeysSetBucketElem**) fMemoryManager->allocate
    (
        newMod * sizeof(Hash2KeysSetBucketElem*)
    );//new Hash2KeysSetBucketElem*[fHashModulus];

    // Make sure the new bucket list is destroyed if an
    // exception is thrown.
    ArrayJanitor<Hash2KeysSetBucketElem*>  guard(newBucketList, fMemoryManager);

    memset(newBucketList, 0, newMod * sizeof(newBucketList[0]));

    // Rehash all existing entries.
    for (XMLSize_t index = 0; index < fHashModulus; index++)
    {
        // Get the bucket list head for this entry
        Hash2KeysSetBucketElem* curElem = fBucketList[index];
        while (curElem)
        {
            // Save the next element before we detach this one
            Hash2KeysSetBucketElem* nextElem = curElem->fNext;

            const XMLSize_t hashVal = fHasher.getHashVal(curElem->fKey1, newMod);
            assert(hashVal < newMod);

            Hash2KeysSetBucketElem* newHeadElem = newBucketList[hashVal];

            // Insert at the start of this bucket's list.
            curElem->fNext = newHeadElem;
            newBucketList[hashVal] = curElem;

            curElem = nextElem;
        }
    }

    Hash2KeysSetBucketElem** const oldBucketList = fBucketList;

    // Everything is OK at this point, so update the
    // member variables.
    fBucketList = guard.release();
    fHashModulus = newMod;

    // Delete the old bucket list.
    fMemoryManager->deallocate(oldBucketList);//delete[] oldBucketList;

}



// ---------------------------------------------------------------------------
//  Hash2KeysSetOfEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class THasher>
Hash2KeysSetOfEnumerator<THasher>::
Hash2KeysSetOfEnumerator(Hash2KeysSetOf<THasher>* const toEnum
                              , const bool adopt
                              , MemoryManager* const manager)
    : fAdopted(adopt), fCurElem(0), fCurHash((XMLSize_t)-1), fToEnum(toEnum)
    , fMemoryManager(manager)
    , fLockPrimaryKey(0)
{
    if (!toEnum)
        ThrowXMLwithMemMgr(NullPointerException, XMLExcepts::CPtr_PointerIsZero, fMemoryManager);

    //
    //  Find the next available bucket element in the hash table. If it
    //  comes back zero, that just means the table is empty.
    //
    //  Note that the -1 in the current hash tells it to start
    //  from the beginning.
    //
    findNext();
}

template <class THasher>
Hash2KeysSetOfEnumerator<THasher>::~Hash2KeysSetOfEnumerator()
{
    if (fAdopted)
        delete fToEnum;
}


// ---------------------------------------------------------------------------
//  Hash2KeysSetOfEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class THasher>
bool Hash2KeysSetOfEnumerator<THasher>::hasMoreElements() const
{
    //
    //  If our current has is at the max and there are no more elements
    //  in the current bucket, then no more elements.
    //
    if (!fCurElem && (fCurHash == fToEnum->fHashModulus))
        return false;
    return true;
}

template <class THasher>
void Hash2KeysSetOfEnumerator<THasher>::nextElementKey(const void*& retKey1, int& retKey2)
{
    // Make sure we have an element to return
    if (!hasMoreElements())
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::Enum_NoMoreElements, fMemoryManager);

    //
    //  Save the current element, then move up to the next one for the
    //  next time around.
    //
    Hash2KeysSetBucketElem* saveElem = fCurElem;
    findNext();

    retKey1 = saveElem->fKey1;
    retKey2 = saveElem->fKey2;

    return;
}

template <class THasher>
void Hash2KeysSetOfEnumerator<THasher>::Reset()
{
    if(fLockPrimaryKey)
        fCurHash=fToEnum->fHasher.getHashVal(fLockPrimaryKey, fToEnum->fHashModulus);
    else
        fCurHash = (XMLSize_t)-1;

    fCurElem = 0;
    findNext();
}


template <class THasher>
void Hash2KeysSetOfEnumerator<THasher>::setPrimaryKey(const void* key)
{
    fLockPrimaryKey=key;
    Reset();
}

// ---------------------------------------------------------------------------
//  Hash2KeysSetOfEnumerator: Private helper methods
// ---------------------------------------------------------------------------
template <class THasher>
void Hash2KeysSetOfEnumerator<THasher>::findNext()
{
    //  Code to execute if we have to return only values with the primary key
    if(fLockPrimaryKey)
    {
        if(!fCurElem)
            fCurElem = fToEnum->fBucketList[fCurHash];
        else
            fCurElem = fCurElem->fNext;
        while (fCurElem && (!fToEnum->fHasher.equals(fLockPrimaryKey, fCurElem->fKey1)))
            fCurElem = fCurElem->fNext;
        // if we didn't found it, make so hasMoreElements() returns false
        if(!fCurElem)
            fCurHash = fToEnum->fHashModulus;
        return;
    }
    //
    //  If there is a current element, move to its next element. If this
    //  hits the end of the bucket, the next block will handle the rest.
    //
    if (fCurElem)
        fCurElem = fCurElem->fNext;

    //
    //  If the current element is null, then we have to move up to the
    //  next hash value. If that is the hash modulus, then we cannot
    //  go further.
    //
    if (!fCurElem)
    {
        fCurHash++;
        if (fCurHash == fToEnum->fHashModulus)
            return;

        // Else find the next non-empty bucket
        while (fToEnum->fBucketList[fCurHash]==0)
        {
            // Bump to the next hash value. If we max out return
            fCurHash++;
            if (fCurHash == fToEnum->fHashModulus)
                return;
        }
        fCurElem = fToEnum->fBucketList[fCurHash];
    }
}

XERCES_CPP_NAMESPACE_END
