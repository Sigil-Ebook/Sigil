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
 * $Id: RefHash2KeysTableOf.c 679340 2008-07-24 10:28:29Z borisk $
 */


// ---------------------------------------------------------------------------
//  Include
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/RefHash2KeysTableOf.hpp>
#endif

#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/NullPointerException.hpp>
#include <assert.h>
#include <new>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  RefHash2KeysTableOf: Constructors and Destructor
// ---------------------------------------------------------------------------

template <class TVal, class THasher>
RefHash2KeysTableOf<TVal, THasher>::RefHash2KeysTableOf(
  const XMLSize_t modulus,
  MemoryManager* const manager)

    : fMemoryManager(manager)
    , fAdoptedElems(true)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fCount(0)
{
    initialize(modulus);
}

template <class TVal, class THasher>
RefHash2KeysTableOf<TVal, THasher>::RefHash2KeysTableOf(
  const XMLSize_t modulus,
  const THasher& hasher,
  MemoryManager* const manager)

    : fMemoryManager(manager)
    , fAdoptedElems(true)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fCount(0)
    , fHasher (hasher)
{
    initialize(modulus);
}

template <class TVal, class THasher>
RefHash2KeysTableOf<TVal, THasher>::RefHash2KeysTableOf(
  const XMLSize_t modulus,
  const bool adoptElems,
  MemoryManager* const manager)

    : fMemoryManager(manager)
    , fAdoptedElems(adoptElems)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fCount(0)

{
    initialize(modulus);
}

template <class TVal, class THasher>
RefHash2KeysTableOf<TVal, THasher>::RefHash2KeysTableOf(
  const XMLSize_t modulus,
  const bool adoptElems,
  const THasher& hasher,
  MemoryManager* const manager)

    : fMemoryManager(manager)
    , fAdoptedElems(adoptElems)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fCount(0)
    , fHasher (hasher)
{
    initialize(modulus);
}

template <class TVal, class THasher>
void RefHash2KeysTableOf<TVal, THasher>::initialize(const XMLSize_t modulus)
{
    if (modulus == 0)
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::HshTbl_ZeroModulus, fMemoryManager);

    // Allocate the bucket list and zero them
    fBucketList = (RefHash2KeysTableBucketElem<TVal>**) fMemoryManager->allocate
    (
        fHashModulus * sizeof(RefHash2KeysTableBucketElem<TVal>*)
    ); //new RefHash2KeysTableBucketElem<TVal>*[fHashModulus];
    memset(fBucketList, 0, sizeof(fBucketList[0]) * fHashModulus);
}

template <class TVal, class THasher>
RefHash2KeysTableOf<TVal, THasher>::~RefHash2KeysTableOf()
{
    removeAll();

    // Then delete the bucket list & hasher
    fMemoryManager->deallocate(fBucketList); //delete [] fBucketList;
    fBucketList = 0;
}


// ---------------------------------------------------------------------------
//  RefHash2KeysTableOf: Element management
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
bool RefHash2KeysTableOf<TVal, THasher>::isEmpty() const
{
    return (fCount==0);
}

template <class TVal, class THasher>
bool RefHash2KeysTableOf<TVal, THasher>::
containsKey(const void* const key1, const int key2) const
{
    XMLSize_t hashVal;
    const RefHash2KeysTableBucketElem<TVal>* findIt = findBucketElem(key1, key2, hashVal);
    return (findIt != 0);
}

template <class TVal, class THasher>
void RefHash2KeysTableOf<TVal, THasher>::
removeKey(const void* const key1, const int key2)
{
    // Hash the key
    XMLSize_t hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    //
    //  Search the given bucket for this key. Keep up with the previous
    //  element so we can patch around it.
    //
    RefHash2KeysTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    RefHash2KeysTableBucketElem<TVal>* lastElem = 0;

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

            // If we adopted the elements, then delete the data
            if (fAdoptedElems)
                delete curElem->fData;

            // Delete the current element
            // delete curElem;
            // destructor is empty...
            // curElem->~RefHash2KeysTableBucketElem();
            fMemoryManager->deallocate(curElem);
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

template <class TVal, class THasher>
void RefHash2KeysTableOf<TVal, THasher>::
removeKey(const void* const key1)
{
    // Hash the key
    XMLSize_t hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    //
    //  Search the given bucket for this key. Keep up with the previous
    //  element so we can patch around it.
    //
    RefHash2KeysTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    RefHash2KeysTableBucketElem<TVal>* lastElem = 0;

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

            // If we adopted the elements, then delete the data
            if (fAdoptedElems)
                delete curElem->fData;

            RefHash2KeysTableBucketElem<TVal>* toBeDeleted=curElem;
            curElem = curElem->fNext;

            // Delete the current element
            // delete curElem;
            // destructor is empty...
            // curElem->~RefHash2KeysTableBucketElem();
            fMemoryManager->deallocate(toBeDeleted);
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

template <class TVal, class THasher>
void RefHash2KeysTableOf<TVal, THasher>::removeAll()
{
    if(isEmpty())
        return;

    // Clean up the buckets first
    for (XMLSize_t buckInd = 0; buckInd < fHashModulus; buckInd++)
    {
        // Get the bucket list head for this entry
        RefHash2KeysTableBucketElem<TVal>* curElem = fBucketList[buckInd];
        RefHash2KeysTableBucketElem<TVal>* nextElem;
        while (curElem)
        {
            // Save the next element before we hose this one
            nextElem = curElem->fNext;

            // If we adopted the data, then delete it too
            //    (Note:  the userdata hash table instance has data type of void *.
            //    This will generate compiler warnings here on some platforms, but they
            //    can be ignored since fAdoptedElements is false.
            if (fAdoptedElems)
                delete curElem->fData;

            // Then delete the current element and move forward
            // destructor is empty...
            // curElem->~RefHash2KeysTableBucketElem();
            fMemoryManager->deallocate(curElem);
            curElem = nextElem;
        }

        // Clean out this entry
        fBucketList[buckInd] = 0;
    }
    fCount=0;
}

// this function transfer the data from key1 to key2
template <class TVal, class THasher>
void RefHash2KeysTableOf<TVal, THasher>::transferElement(const void* const key1, void* key2)
{
    // Hash the key
    XMLSize_t hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    //
    //  Search the given bucket for this key. Keep up with the previous
    //  element so we can patch around it.
    //
    RefHash2KeysTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    RefHash2KeysTableBucketElem<TVal>* lastElem = 0;

    while (curElem)
    {
        // if this element has the same primary key, remove it and add it using the new primary key
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

            // this code comes from put(), but it doesn't update fCount
            XMLSize_t hashVal2;
            RefHash2KeysTableBucketElem<TVal>* newBucket = findBucketElem(key2, curElem->fKey2, hashVal2);
            if (newBucket)
            {
                if (fAdoptedElems)
                    delete newBucket->fData;
                newBucket->fData = curElem->fData;
                newBucket->fKey1 = key2;
                newBucket->fKey2 = curElem->fKey2;
            }
             else
            {
                newBucket =
                    new (fMemoryManager->allocate(sizeof(RefHash2KeysTableBucketElem<TVal>)))
                    RefHash2KeysTableBucketElem<TVal>(key2, curElem->fKey2, curElem->fData, fBucketList[hashVal2]);
                fBucketList[hashVal2] = newBucket;
            }

            RefHash2KeysTableBucketElem<TVal>* elemToDelete = curElem;

            // Update just curElem; lastElem must stay the same
            curElem = curElem->fNext;

            // Delete the current element
            // delete elemToDelete;
            // destructor is empty...
            // curElem->~RefHash2KeysTableBucketElem();
            fMemoryManager->deallocate(elemToDelete);
        }
        else
        {
            // Move both pointers upwards
            lastElem = curElem;
            curElem = curElem->fNext;
        }
    }
}



// ---------------------------------------------------------------------------
//  RefHash2KeysTableOf: Getters
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
TVal* RefHash2KeysTableOf<TVal, THasher>::get(const void* const key1, const int key2)
{
    XMLSize_t hashVal;
    RefHash2KeysTableBucketElem<TVal>* findIt = findBucketElem(key1, key2, hashVal);
    if (!findIt)
        return 0;
    return findIt->fData;
}

template <class TVal, class THasher>
const TVal* RefHash2KeysTableOf<TVal, THasher>::
get(const void* const key1, const int key2) const
{
    XMLSize_t hashVal;
    const RefHash2KeysTableBucketElem<TVal>* findIt = findBucketElem(key1, key2, hashVal);
    if (!findIt)
        return 0;
    return findIt->fData;
}

template <class TVal, class THasher>
MemoryManager* RefHash2KeysTableOf<TVal, THasher>::getMemoryManager() const
{
    return fMemoryManager;
}

template <class TVal, class THasher>
XMLSize_t RefHash2KeysTableOf<TVal, THasher>::getHashModulus() const
{
    return fHashModulus;
}

// ---------------------------------------------------------------------------
//  RefHash2KeysTableOf: Putters
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
void RefHash2KeysTableOf<TVal, THasher>::put(void* key1, int key2, TVal* const valueToAdopt)
{
    // Apply 4 load factor to find threshold.
    XMLSize_t threshold = fHashModulus * 4;

    // If we've grown too big, expand the table and rehash.
    if (fCount >= threshold)
        rehash();

    // First see if the key exists already
    XMLSize_t hashVal;
    RefHash2KeysTableBucketElem<TVal>* newBucket = findBucketElem(key1, key2, hashVal);

    //
    //  If so,then update its value. If not, then we need to add it to
    //  the right bucket
    //
    if (newBucket)
    {
        if (fAdoptedElems)
            delete newBucket->fData;
        newBucket->fData = valueToAdopt;
        newBucket->fKey1 = key1;
        newBucket->fKey2 = key2;
    }
     else
    {
        newBucket =
            new (fMemoryManager->allocate(sizeof(RefHash2KeysTableBucketElem<TVal>)))
            RefHash2KeysTableBucketElem<TVal>(key1, key2, valueToAdopt, fBucketList[hashVal]);
        fBucketList[hashVal] = newBucket;
        fCount++;
    }
}



// ---------------------------------------------------------------------------
//  RefHash2KeysTableOf: Private methods
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
inline RefHash2KeysTableBucketElem<TVal>* RefHash2KeysTableOf<TVal, THasher>::
findBucketElem(const void* const key1, const int key2, XMLSize_t& hashVal)
{
    // Hash the key
    hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    // Search that bucket for the key
    RefHash2KeysTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    while (curElem)
    {
        if((key2==curElem->fKey2) && (fHasher.equals(key1, curElem->fKey1)))
            return curElem;

        curElem = curElem->fNext;
    }
    return 0;
}

template <class TVal, class THasher>
inline const RefHash2KeysTableBucketElem<TVal>* RefHash2KeysTableOf<TVal, THasher>::
findBucketElem(const void* const key1, const int key2, XMLSize_t& hashVal) const
{
    // Hash the key
    hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    // Search that bucket for the key
    const RefHash2KeysTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    while (curElem)
    {
        if((key2==curElem->fKey2) && (fHasher.equals(key1, curElem->fKey1)))
            return curElem;

        curElem = curElem->fNext;
    }
    return 0;
}


template <class TVal, class THasher>
void RefHash2KeysTableOf<TVal, THasher>::
rehash()
{
    const XMLSize_t newMod = (fHashModulus * 8)+1;

    RefHash2KeysTableBucketElem<TVal>** newBucketList =
        (RefHash2KeysTableBucketElem<TVal>**) fMemoryManager->allocate
    (
        newMod * sizeof(RefHash2KeysTableBucketElem<TVal>*)
    );//new RefHash2KeysTableBucketElem<TVal>*[fHashModulus];

    // Make sure the new bucket list is destroyed if an
    // exception is thrown.
    ArrayJanitor<RefHash2KeysTableBucketElem<TVal>*>  guard(newBucketList, fMemoryManager);

    memset(newBucketList, 0, newMod * sizeof(newBucketList[0]));

    // Rehash all existing entries.
    for (XMLSize_t index = 0; index < fHashModulus; index++)
    {
        // Get the bucket list head for this entry
        RefHash2KeysTableBucketElem<TVal>* curElem = fBucketList[index];
        while (curElem)
        {
            // Save the next element before we detach this one
            RefHash2KeysTableBucketElem<TVal>* nextElem = curElem->fNext;

            const XMLSize_t hashVal = fHasher.getHashVal(curElem->fKey1, newMod);
            assert(hashVal < newMod);

            RefHash2KeysTableBucketElem<TVal>* newHeadElem = newBucketList[hashVal];

            // Insert at the start of this bucket's list.
            curElem->fNext = newHeadElem;
            newBucketList[hashVal] = curElem;

            curElem = nextElem;
        }
    }

    RefHash2KeysTableBucketElem<TVal>** const oldBucketList = fBucketList;

    // Everything is OK at this point, so update the
    // member variables.
    fBucketList = guard.release();
    fHashModulus = newMod;

    // Delete the old bucket list.
    fMemoryManager->deallocate(oldBucketList);//delete[] oldBucketList;

}



// ---------------------------------------------------------------------------
//  RefHash2KeysTableOfEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
RefHash2KeysTableOfEnumerator<TVal, THasher>::
RefHash2KeysTableOfEnumerator(RefHash2KeysTableOf<TVal, THasher>* const toEnum
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

template <class TVal, class THasher>
RefHash2KeysTableOfEnumerator<TVal, THasher>::~RefHash2KeysTableOfEnumerator()
{
    if (fAdopted)
        delete fToEnum;
}


// ---------------------------------------------------------------------------
//  RefHash2KeysTableOfEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
bool RefHash2KeysTableOfEnumerator<TVal, THasher>::hasMoreElements() const
{
    //
    //  If our current has is at the max and there are no more elements
    //  in the current bucket, then no more elements.
    //
    if (!fCurElem && (fCurHash == fToEnum->fHashModulus))
        return false;
    return true;
}

template <class TVal, class THasher>
TVal& RefHash2KeysTableOfEnumerator<TVal, THasher>::nextElement()
{
    // Make sure we have an element to return
    if (!hasMoreElements())
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::Enum_NoMoreElements, fMemoryManager);

    //
    //  Save the current element, then move up to the next one for the
    //  next time around.
    //
    RefHash2KeysTableBucketElem<TVal>* saveElem = fCurElem;
    findNext();

    return *saveElem->fData;
}

template <class TVal, class THasher>
void RefHash2KeysTableOfEnumerator<TVal, THasher>::nextElementKey(void*& retKey1, int& retKey2)
{
    // Make sure we have an element to return
    if (!hasMoreElements())
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::Enum_NoMoreElements, fMemoryManager);

    //
    //  Save the current element, then move up to the next one for the
    //  next time around.
    //
    RefHash2KeysTableBucketElem<TVal>* saveElem = fCurElem;
    findNext();

    retKey1 = saveElem->fKey1;
    retKey2 = saveElem->fKey2;

    return;
}

template <class TVal, class THasher>
void RefHash2KeysTableOfEnumerator<TVal, THasher>::Reset()
{
    if(fLockPrimaryKey)
        fCurHash=fToEnum->fHasher.getHashVal(fLockPrimaryKey, fToEnum->fHashModulus);
    else
        fCurHash = (XMLSize_t)-1;

    fCurElem = 0;
    findNext();
}


template <class TVal, class THasher>
void RefHash2KeysTableOfEnumerator<TVal, THasher>::setPrimaryKey(const void* key)
{
    fLockPrimaryKey=key;
    Reset();
}

// ---------------------------------------------------------------------------
//  RefHash2KeysTableOfEnumerator: Private helper methods
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
void RefHash2KeysTableOfEnumerator<TVal, THasher>::findNext()
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
