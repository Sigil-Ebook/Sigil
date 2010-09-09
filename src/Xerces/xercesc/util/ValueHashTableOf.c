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
 * $Id: ValueHashTableOf.c 679340 2008-07-24 10:28:29Z borisk $
 */


// ---------------------------------------------------------------------------
//  Include
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/ValueHashTableOf.hpp>
#endif

#include <xercesc/util/NullPointerException.hpp>
#include <xercesc/util/Janitor.hpp>
#include <assert.h>
#include <new>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  ValueHashTableOf: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
ValueHashTableOf<TVal, THasher>::ValueHashTableOf( const XMLSize_t modulus
                                                   , const THasher& hasher
                                                   , MemoryManager* const manager)
    : fMemoryManager(manager)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fInitialModulus(modulus)
    , fCount(0)
    , fHasher(hasher)
{
    initialize(modulus);
}

template <class TVal, class THasher>
ValueHashTableOf<TVal, THasher>::ValueHashTableOf( const XMLSize_t modulus
                                                   , MemoryManager* const manager)
    : fMemoryManager(manager)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fInitialModulus(modulus)
    , fCount(0)
    , fHasher()
{
    initialize(modulus);
}

template <class TVal, class THasher>
void ValueHashTableOf<TVal, THasher>::initialize(const XMLSize_t modulus)
{
    if (modulus == 0)
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::HshTbl_ZeroModulus, fMemoryManager);

    // Allocate the bucket list and zero them
    fBucketList = (ValueHashTableBucketElem<TVal>**) fMemoryManager->allocate
    (
        fHashModulus * sizeof(ValueHashTableBucketElem<TVal>*)
    ); //new ValueHashTableBucketElem<TVal>*[fHashModulus];
    memset(fBucketList, 0, sizeof(fBucketList[0]) * fHashModulus);
}

template <class TVal, class THasher>
ValueHashTableOf<TVal, THasher>::~ValueHashTableOf()
{
    removeAll();

    // Then delete the bucket list & hasher
    fMemoryManager->deallocate(fBucketList); //delete [] fBucketList;
}


// ---------------------------------------------------------------------------
//  ValueHashTableOf: Element management
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
bool ValueHashTableOf<TVal, THasher>::isEmpty() const
{
    return fCount==0;
}

template <class TVal, class THasher>
bool ValueHashTableOf<TVal, THasher>::
containsKey(const void* const key) const
{
    XMLSize_t hashVal;
    const ValueHashTableBucketElem<TVal>* findIt = findBucketElem(key, hashVal);
    return (findIt != 0);
}

template <class TVal, class THasher>
void ValueHashTableOf<TVal, THasher>::
removeKey(const void* const key)
{
    XMLSize_t hashVal;
    removeBucketElem(key, hashVal);
}

template <class TVal, class THasher>
void ValueHashTableOf<TVal, THasher>::removeAll()
{
    if(isEmpty())
        return;

    // Clean up the buckets first
    for (XMLSize_t buckInd = 0; buckInd < fHashModulus; buckInd++)
    {
        // Get the bucket list head for this entry
        ValueHashTableBucketElem<TVal>* curElem = fBucketList[buckInd];
        ValueHashTableBucketElem<TVal>* nextElem;
        while (curElem)
        {
            // Save the next element before we hose this one
            nextElem = curElem->fNext;

            // delete the current element and move forward
            // destructor is empty...
            // curElem->~ValueHashTableBucketElem();
            fMemoryManager->deallocate(curElem);
            curElem = nextElem;
        }

        // Clean out this entry
        fBucketList[buckInd] = 0;
    }
    fCount = 0;
}


// ---------------------------------------------------------------------------
//  ValueHashTableOf: Getters
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
TVal& ValueHashTableOf<TVal, THasher>::get(const void* const key, MemoryManager* const manager)
{
    XMLSize_t hashVal;
    ValueHashTableBucketElem<TVal>* findIt = findBucketElem(key, hashVal);
    if (!findIt)
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::HshTbl_NoSuchKeyExists, manager);

    return findIt->fData;
}

template <class TVal, class THasher>
const TVal& ValueHashTableOf<TVal, THasher>::
get(const void* const key) const
{
    XMLSize_t hashVal;
    const ValueHashTableBucketElem<TVal>* findIt = findBucketElem(key, hashVal);
    if (!findIt)
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::HshTbl_NoSuchKeyExists, fMemoryManager);

    return findIt->fData;
}


// ---------------------------------------------------------------------------
//  ValueHashTableOf: Putters
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
void ValueHashTableOf<TVal, THasher>::put(void* key, const TVal& valueToAdopt)
{
    // Apply 0.75 load factor to find threshold.
    XMLSize_t threshold = fHashModulus * 3 / 4;

    // If we've grown too big, expand the table and rehash.
    if (fCount >= threshold)
        rehash();

    // First see if the key exists already
    XMLSize_t hashVal;
    ValueHashTableBucketElem<TVal>* newBucket = findBucketElem(key, hashVal);

    //
    //  If so,then update its value. If not, then we need to add it to
    //  the right bucket
    //
    if (newBucket)
    {
        newBucket->fData = valueToAdopt;
        newBucket->fKey = key;
    }
     else
    {
        newBucket =
            new (fMemoryManager->allocate(sizeof(ValueHashTableBucketElem<TVal>)))
            ValueHashTableBucketElem<TVal>(key, valueToAdopt, fBucketList[hashVal]);
        fBucketList[hashVal] = newBucket;
        fCount++;
    }
}



// ---------------------------------------------------------------------------
//  ValueHashTableOf: Private methods
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
void ValueHashTableOf<TVal, THasher>::rehash()
{
    const XMLSize_t newMod = (fHashModulus * 2) + 1;

    ValueHashTableBucketElem<TVal>** newBucketList =
        (ValueHashTableBucketElem<TVal>**) fMemoryManager->allocate
    (
        newMod * sizeof(ValueHashTableBucketElem<TVal>*)
    );//new RefHashTableBucketElem<TVal>*[newMod];

    // Make sure the new bucket list is destroyed if an
    // exception is thrown.
    ArrayJanitor<ValueHashTableBucketElem<TVal>*>  guard(newBucketList, fMemoryManager);

    memset(newBucketList, 0, newMod * sizeof(newBucketList[0]));


    // Rehash all existing entries.
    for (XMLSize_t index = 0; index < fHashModulus; index++)
    {
        // Get the bucket list head for this entry
        ValueHashTableBucketElem<TVal>* curElem = fBucketList[index];

        while (curElem)
        {
            // Save the next element before we detach this one
            ValueHashTableBucketElem<TVal>* const nextElem = curElem->fNext;

            const XMLSize_t hashVal = fHasher.getHashVal(curElem->fKey, newMod);
            assert(hashVal < newMod);

            ValueHashTableBucketElem<TVal>* const newHeadElem = newBucketList[hashVal];

            // Insert at the start of this bucket's list.
            curElem->fNext = newHeadElem;
            newBucketList[hashVal] = curElem;

            curElem = nextElem;
        }
    }

    ValueHashTableBucketElem<TVal>** const oldBucketList = fBucketList;

    // Everything is OK at this point, so update the
    // member variables.
    fBucketList = guard.release();
    fHashModulus = newMod;

    // Delete the old bucket list.
    fMemoryManager->deallocate(oldBucketList);//delete[] oldBucketList;

}

template <class TVal, class THasher>
inline ValueHashTableBucketElem<TVal>* ValueHashTableOf<TVal, THasher>::
findBucketElem(const void* const key, XMLSize_t& hashVal)
{
    // Hash the key
    hashVal = fHasher.getHashVal(key, fHashModulus);
    assert(hashVal < fHashModulus);

    // Search that bucket for the key
    ValueHashTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    while (curElem)
    {
        if (fHasher.equals(key, curElem->fKey))
            return curElem;

        curElem = curElem->fNext;
    }
    return 0;
}

template <class TVal, class THasher>
inline const ValueHashTableBucketElem<TVal>* ValueHashTableOf<TVal, THasher>::
findBucketElem(const void* const key, XMLSize_t& hashVal) const
{
    // Hash the key
    hashVal = fHasher.getHashVal(key, fHashModulus);
    assert(hashVal < fHashModulus);

    // Search that bucket for the key
    const ValueHashTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    while (curElem)
    {
        if (fHasher.equals(key, curElem->fKey))
            return curElem;

        curElem = curElem->fNext;
    }
    return 0;
}


template <class TVal, class THasher>
void ValueHashTableOf<TVal, THasher>::
removeBucketElem(const void* const key, XMLSize_t& hashVal)
{
    // Hash the key
    hashVal = fHasher.getHashVal(key, fHashModulus);
    assert(hashVal < fHashModulus);

    //
    //  Search the given bucket for this key. Keep up with the previous
    //  element so we can patch around it.
    //
    ValueHashTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    ValueHashTableBucketElem<TVal>* lastElem = 0;

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

            // Delete the current element
            // delete curElem;
            // destructor is empty...
            // curElem->~ValueHashTableBucketElem();
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




// ---------------------------------------------------------------------------
//  ValueHashTableOfEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
ValueHashTableOfEnumerator<TVal, THasher>::
ValueHashTableOfEnumerator(ValueHashTableOf<TVal, THasher>* const toEnum
                           , const bool adopt
                           , MemoryManager* const manager)
    : fAdopted(adopt), fCurElem(0), fCurHash((XMLSize_t)-1), fToEnum(toEnum), fMemoryManager(manager)
{
    if (!toEnum)
        ThrowXMLwithMemMgr(NullPointerException, XMLExcepts::CPtr_PointerIsZero, manager);

    //
    //  Find the next available bucket element in the hash table. If it
    //  comes back zero, that just means the table is empty.
    //
    //  Note that the -1 in the current hash tells it to start from the
    //  beginning.
    //
    findNext();
}

template <class TVal, class THasher>
ValueHashTableOfEnumerator<TVal, THasher>::~ValueHashTableOfEnumerator()
{
    if (fAdopted)
        delete fToEnum;
}


// ---------------------------------------------------------------------------
//  ValueHashTableOfEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
bool ValueHashTableOfEnumerator<TVal, THasher>::hasMoreElements() const
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
TVal& ValueHashTableOfEnumerator<TVal, THasher>::nextElement()
{
    // Make sure we have an element to return
    if (!hasMoreElements())
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::Enum_NoMoreElements, fMemoryManager);

    //
    //  Save the current element, then move up to the next one for the
    //  next time around.
    //
    ValueHashTableBucketElem<TVal>* saveElem = fCurElem;
    findNext();

    return saveElem->fData;
}

template <class TVal, class THasher>
void* ValueHashTableOfEnumerator<TVal, THasher>::nextElementKey()
{
    // Make sure we have an element to return
    if (!hasMoreElements())
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::Enum_NoMoreElements, fMemoryManager);

    //
    //  Save the current element, then move up to the next one for the
    //  next time around.
    //
    ValueHashTableBucketElem<TVal>* saveElem = fCurElem;
    findNext();

    return saveElem->fKey;
}


template <class TVal, class THasher>
void ValueHashTableOfEnumerator<TVal, THasher>::Reset()
{
    fCurHash = (XMLSize_t)-1;
    fCurElem = 0;
    findNext();
}



// ---------------------------------------------------------------------------
//  ValueHashTableOfEnumerator: Private helper methods
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
void ValueHashTableOfEnumerator<TVal, THasher>::findNext()
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
        if (++fCurHash == fToEnum->fHashModulus)
            return;

        // Else find the next non-empty bucket
        while (fToEnum->fBucketList[fCurHash]==0)
        {
            // Bump to the next hash value. If we max out return
            if (++fCurHash == fToEnum->fHashModulus)
                return;
        }
        fCurElem = fToEnum->fBucketList[fCurHash];
    }
}

XERCES_CPP_NAMESPACE_END
