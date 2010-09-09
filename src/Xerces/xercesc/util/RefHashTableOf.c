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
 * $Id: RefHashTableOf.c 678409 2008-07-21 13:08:10Z borisk $
 */


// ---------------------------------------------------------------------------
//  Include
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/RefHashTableOf.hpp>
#endif

#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/NullPointerException.hpp>
#include <new>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  RefHashTableOf: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
RefHashTableOf<TVal, THasher>::RefHashTableOf(
  const XMLSize_t modulus,
  MemoryManager* const manager)

    : fMemoryManager(manager)
    , fAdoptedElems(true)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fInitialModulus(modulus)
    , fCount(0)
{
    initialize(modulus);
}

template <class TVal, class THasher>
RefHashTableOf<TVal, THasher>::RefHashTableOf(
  const XMLSize_t modulus,
  const THasher& hasher,
  MemoryManager* const manager)

    : fMemoryManager(manager)
    , fAdoptedElems(true)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fInitialModulus(modulus)
    , fCount(0)
    , fHasher (hasher)
{
    initialize(modulus);
}

template <class TVal, class THasher>
RefHashTableOf<TVal, THasher>::RefHashTableOf(
  const XMLSize_t modulus,
  const bool adoptElems,
  MemoryManager* const manager)

    : fMemoryManager(manager)
    , fAdoptedElems(adoptElems)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fInitialModulus(modulus)
    , fCount(0)

{
    initialize(modulus);
}

template <class TVal, class THasher>
RefHashTableOf<TVal, THasher>::RefHashTableOf(
  const XMLSize_t modulus,
  const bool adoptElems,
  const THasher& hasher,
  MemoryManager* const manager)

    : fMemoryManager(manager)
    , fAdoptedElems(adoptElems)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fInitialModulus(modulus)
    , fCount(0)
    , fHasher (hasher)
{
    initialize(modulus);
}

template <class TVal, class THasher>
void RefHashTableOf<TVal, THasher>::initialize(const XMLSize_t modulus)
{
    if (modulus == 0)
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::HshTbl_ZeroModulus, fMemoryManager);

    // Allocate the bucket list and zero them
    fBucketList = (RefHashTableBucketElem<TVal>**) fMemoryManager->allocate
    (
        fHashModulus * sizeof(RefHashTableBucketElem<TVal>*)
    );
    for (XMLSize_t index = 0; index < fHashModulus; index++)
        fBucketList[index] = 0;
}

template <class TVal, class THasher>
RefHashTableOf<TVal, THasher>::~RefHashTableOf()
{
    cleanup();
}


// ---------------------------------------------------------------------------
//  RefHashTableOf: Element management
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
inline bool RefHashTableOf<TVal, THasher>::isEmpty() const
{
    return fCount==0;
}

template <class TVal, class THasher>
inline bool RefHashTableOf<TVal, THasher>::containsKey(const void* const key) const
{
    XMLSize_t hashVal;
    const RefHashTableBucketElem<TVal>* findIt = findBucketElem(key, hashVal);
    return (findIt != 0);
}

template <class TVal, class THasher>
void RefHashTableOf<TVal, THasher>::
removeKey(const void* const key)
{
    // Hash the key
    XMLSize_t hashVal = fHasher.getHashVal(key, fHashModulus);

    //
    //  Search the given bucket for this key. Keep up with the previous
    //  element so we can patch around it.
    //
    RefHashTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    RefHashTableBucketElem<TVal>* lastElem = 0;

    while (curElem)
    {
        if (fHasher.equals(key, curElem->fKey))
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

            // If we adopted the data, then delete it too
            //    (Note:  the userdata hash table instance has data type of void *.
            //    This will generate compiler warnings here on some platforms, but they
            //    can be ignored since fAdoptedElements is false.
            if (fAdoptedElems)
                delete curElem->fData;

            // Then delete the current element and move forward
            // delete curElem;
            // destructor doesn't do anything...
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
void RefHashTableOf<TVal, THasher>::removeAll()
{
    if(isEmpty())
        return;

    // Clean up the buckets first
    for (XMLSize_t buckInd = 0; buckInd < fHashModulus; buckInd++)
    {
        // Get the bucket list head for this entry
        RefHashTableBucketElem<TVal>* curElem = fBucketList[buckInd];
        RefHashTableBucketElem<TVal>* nextElem;
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
             // delete curElem;
            // destructor doesn't do anything...
            // curElem->~RefHashTableBucketElem();
            fMemoryManager->deallocate(curElem);
            curElem = nextElem;
        }

        // Clean out this entry
        fBucketList[buckInd] = 0;
    }

    fCount = 0;
}

// This method returns the data associated with a key. The key entry is deleted. The caller
// now owns the returned data (case of hashtable adopting the data).
// This function is called by transferElement so that the undeleted data can be transferred
// to a new key which will own that data.
template <class TVal, class THasher> TVal* RefHashTableOf<TVal, THasher>::
orphanKey(const void* const key)
{
    // Hash the key
    TVal* retVal = 0;
    XMLSize_t hashVal = fHasher.getHashVal(key, fHashModulus);

    //
    //  Search the given bucket for this key. Keep up with the previous
    //  element so we can patch around it.
    //
    RefHashTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    RefHashTableBucketElem<TVal>* lastElem = 0;

    while (curElem)
    {
        if (fHasher.equals(key, curElem->fKey))
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

            retVal = curElem->fData;

            // Delete the current element
            // delete curElem;
            // destructor doesn't do anything...
            // curElem->~RefHashTableBucketElem();
            fMemoryManager->deallocate(curElem);
            break;
        }

        // Move both pointers upwards
        lastElem = curElem;
        curElem = curElem->fNext;
    }

    // We never found that key
    if (!retVal)
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::HshTbl_NoSuchKeyExists, fMemoryManager);

    return retVal;
}

//
// cleanup():
//   similar to destructor
//   called to cleanup the memory, in case destructor cannot be called
//
template <class TVal, class THasher>
void RefHashTableOf<TVal, THasher>::cleanup()
{
    removeAll();

    // Then delete the bucket list & hasher
    fMemoryManager->deallocate(fBucketList);
    fBucketList = 0;
}

//
// reinitialize():
//   similar to constructor
//   called to re-construct the fElemList from scratch again
//
template <class TVal, class THasher>
void RefHashTableOf<TVal, THasher>::reinitialize(const THasher& hasher)
{
    if (fBucketList)
        cleanup();

    fHasher = hasher;

    fHashModulus = fInitialModulus;
    initialize(fHashModulus);
}



// this function transfer the data from key1 to key2
// this is equivalent to calling
//  1.  get(key1) to retrieve the data,
//  2.  removeKey(key1),
//  3.  and then put(key2, data)
// except that the data is not deleted in "removeKey" even it is adopted so that it
// can be transferred to key2.
// whatever key2 has originally will be purged (if adopted)
template <class TVal, class THasher>
inline void RefHashTableOf<TVal, THasher>::transferElement(const void* const key1, void* key2)
{
    put(key2, orphanKey(key1));
}


// ---------------------------------------------------------------------------
//  RefHashTableOf: Getters
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
inline TVal* RefHashTableOf<TVal, THasher>::get(const void* const key)
{
    XMLSize_t hashVal;
    RefHashTableBucketElem<TVal>* findIt = findBucketElem(key, hashVal);
    return findIt ? findIt->fData : 0;
}

template <class TVal, class THasher>
inline const TVal* RefHashTableOf<TVal, THasher>::
get(const void* const key) const
{
    XMLSize_t hashVal;
    const RefHashTableBucketElem<TVal>* findIt = findBucketElem(key, hashVal);
    return findIt ? findIt->fData : 0;
}

template <class TVal, class THasher>
inline MemoryManager* RefHashTableOf<TVal, THasher>::getMemoryManager() const
{
    return fMemoryManager;
}

template <class TVal, class THasher>
inline XMLSize_t RefHashTableOf<TVal, THasher>::getHashModulus() const
{
    return fHashModulus;
}

template <class TVal, class THasher>
inline XMLSize_t RefHashTableOf<TVal, THasher>::getCount() const
{
    return fCount;
}

// ---------------------------------------------------------------------------
//  RefHashTableOf: Getters
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
inline void RefHashTableOf<TVal, THasher>::setAdoptElements(const bool aValue)
{
    fAdoptedElems = aValue;
}

// ---------------------------------------------------------------------------
//  RefHashTableOf: Putters
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
void RefHashTableOf<TVal, THasher>::put(void* key, TVal* const valueToAdopt)
{
    // Apply 0.75 load factor to find threshold.
    XMLSize_t threshold = fHashModulus * 3 / 4;

    // If we've grown too big, expand the table and rehash.
    if (fCount >= threshold)
        rehash();

    // First see if the key exists already
    XMLSize_t hashVal;
    RefHashTableBucketElem<TVal>* newBucket = findBucketElem(key, hashVal);

    //
    //  If so,then update its value. If not, then we need to add it to
    //  the right bucket
    //
    if (newBucket)
    {
        if (fAdoptedElems)
            delete newBucket->fData;
        newBucket->fData = valueToAdopt;
        newBucket->fKey = key;
    }
    else
    {
        newBucket =
             new (fMemoryManager->allocate(sizeof(RefHashTableBucketElem<TVal>)))
             RefHashTableBucketElem<TVal>(key, valueToAdopt, fBucketList[hashVal]);
        fBucketList[hashVal] = newBucket;
        fCount++;
    }
}



// ---------------------------------------------------------------------------
//  RefHashTableOf: Private methods
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
void RefHashTableOf<TVal, THasher>::rehash()
{
    const XMLSize_t newMod = (fHashModulus * 2) + 1;

    RefHashTableBucketElem<TVal>** newBucketList =
        (RefHashTableBucketElem<TVal>**) fMemoryManager->allocate
    (
        newMod * sizeof(RefHashTableBucketElem<TVal>*)
    );

    // Make sure the new bucket list is destroyed if an
    // exception is thrown.
    ArrayJanitor<RefHashTableBucketElem<TVal>*>  guard(newBucketList, fMemoryManager);

    memset(newBucketList, 0, newMod * sizeof(newBucketList[0]));


    // Rehash all existing entries.
    for (XMLSize_t index = 0; index < fHashModulus; index++)
    {
        // Get the bucket list head for this entry
        RefHashTableBucketElem<TVal>* curElem = fBucketList[index];

        while (curElem)
        {
            // Save the next element before we detach this one
            RefHashTableBucketElem<TVal>* const nextElem = curElem->fNext;

            const XMLSize_t hashVal = fHasher.getHashVal(curElem->fKey, newMod);

            RefHashTableBucketElem<TVal>* const newHeadElem = newBucketList[hashVal];

            // Insert at the start of this bucket's list.
            curElem->fNext = newHeadElem;
            newBucketList[hashVal] = curElem;

            curElem = nextElem;
        }
    }

    RefHashTableBucketElem<TVal>** const oldBucketList = fBucketList;

    // Everything is OK at this point, so update the
    // member variables.
    fBucketList = guard.release();
    fHashModulus = newMod;

    // Delete the old bucket list.
    fMemoryManager->deallocate(oldBucketList);//delete[] oldBucketList;

}

template <class TVal, class THasher>
inline RefHashTableBucketElem<TVal>* RefHashTableOf<TVal, THasher>::
findBucketElem(const void* const key, XMLSize_t& hashVal)
{
    // Hash the key
    hashVal = fHasher.getHashVal(key, fHashModulus);

    // Search that bucket for the key
    RefHashTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    while (curElem)
    {
        if (fHasher.equals(key, curElem->fKey))
            return curElem;

        curElem = curElem->fNext;
    }
    return 0;
}

template <class TVal, class THasher>
inline const RefHashTableBucketElem<TVal>* RefHashTableOf<TVal, THasher>::
findBucketElem(const void* const key, XMLSize_t& hashVal) const
{
    // Hash the key
    hashVal = fHasher.getHashVal(key, fHashModulus);

    // Search that bucket for the key
    const RefHashTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    while (curElem)
    {
        if (fHasher.equals(key, curElem->fKey))
            return curElem;

        curElem = curElem->fNext;
    }
    return 0;
}


// ---------------------------------------------------------------------------
//  RefHashTableOfEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TVal, class THasher> RefHashTableOfEnumerator<TVal, THasher>::
RefHashTableOfEnumerator(RefHashTableOf<TVal, THasher>* const toEnum
                         , const bool adopt
                         , MemoryManager* const manager)
    : fAdopted(adopt), fCurElem(0), fCurHash((XMLSize_t)-1), fToEnum(toEnum)
    , fMemoryManager(manager)
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
RefHashTableOfEnumerator<TVal, THasher>::~RefHashTableOfEnumerator()
{
    if (fAdopted)
        delete fToEnum;
}


template <class TVal, class THasher> RefHashTableOfEnumerator<TVal, THasher>::
RefHashTableOfEnumerator(const RefHashTableOfEnumerator<TVal, THasher>& toCopy) :
    XMLEnumerator<TVal>(toCopy)
    , XMemory(toCopy)
    , fAdopted(toCopy.fAdopted)
    , fCurElem(toCopy.fCurElem)
    , fCurHash(toCopy.fCurHash)
    , fToEnum(toCopy.fToEnum)
    , fMemoryManager(toCopy.fMemoryManager)
{
}
// ---------------------------------------------------------------------------
//  RefHashTableOfEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
bool RefHashTableOfEnumerator<TVal, THasher>::hasMoreElements() const
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
TVal& RefHashTableOfEnumerator<TVal, THasher>::nextElement()
{
    // Make sure we have an element to return
    if (!hasMoreElements())
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::Enum_NoMoreElements, fMemoryManager);

    //
    //  Save the current element, then move up to the next one for the
    //  next time around.
    //
    RefHashTableBucketElem<TVal>* saveElem = fCurElem;
    findNext();

    return *saveElem->fData;
}

template <class TVal, class THasher>
void* RefHashTableOfEnumerator<TVal, THasher>::nextElementKey()
{
    // Make sure we have an element to return
    if (!hasMoreElements())
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::Enum_NoMoreElements, fMemoryManager);

    //
    //  Save the current element, then move up to the next one for the
    //  next time around.
    //
    RefHashTableBucketElem<TVal>* saveElem = fCurElem;
    findNext();

    return saveElem->fKey;
}

template <class TVal, class THasher>
void RefHashTableOfEnumerator<TVal, THasher>::Reset()
{
    fCurHash = (XMLSize_t)-1;
    fCurElem = 0;
    findNext();
}



// ---------------------------------------------------------------------------
//  RefHashTableOfEnumerator: Private helper methods
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
void RefHashTableOfEnumerator<TVal, THasher>::findNext()
{
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
