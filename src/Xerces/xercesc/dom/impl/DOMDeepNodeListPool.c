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

/**
 * $Id: DOMDeepNodeListPool.c 883368 2009-11-23 15:28:19Z amassari $
 */


// ---------------------------------------------------------------------------
//  Include
// ---------------------------------------------------------------------------
#include <xercesc/util/XercesDefs.hpp>
#if defined(XERCES_TMPLSINC)
#include <xercesc/dom/impl/DOMDeepNodeListPool.hpp>
#endif

#include <assert.h>

XERCES_CPP_NAMESPACE_BEGIN



// ---------------------------------------------------------------------------
//  DOMDeepNodeListPool: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
DOMDeepNodeListPool<TVal, THasher>::DOMDeepNodeListPool( const XMLSize_t modulus
                                              , const bool adoptElems
                                              , const XMLSize_t initSize) :
    fAdoptedElems(adoptElems)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fIdPtrs(0)
    , fIdPtrsCount(initSize)
    , fIdCounter(0)
    , fMemoryManager(XMLPlatformUtils::fgMemoryManager)
{
    initialize(modulus);

    //
    //  Allocate the initial id pointers array. We don't have to zero them
    //  out since the fIdCounter value tells us which ones are valid. The
    //  zeroth element is never used (and represents an invalid pool id.)
    //
    if (!fIdPtrsCount)
        fIdPtrsCount = 256;

    fIdPtrs = (TVal**) fMemoryManager->allocate(fIdPtrsCount * sizeof(TVal*));//new TVal*[fIdPtrsCount];
    fIdPtrs[0] = 0;
}

template <class TVal, class THasher>
DOMDeepNodeListPool<TVal, THasher>::DOMDeepNodeListPool( const XMLSize_t modulus
                                              , const bool adoptElems
                                              , const THasher& hasher
                                              , const XMLSize_t initSize) :
    fAdoptedElems(adoptElems)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fIdPtrs(0)
    , fIdPtrsCount(initSize)
    , fIdCounter(0)
    , fMemoryManager(XMLPlatformUtils::fgMemoryManager)
    , fHasher(hasher)
{
    initialize(modulus);

    //
    //  Allocate the initial id pointers array. We don't have to zero them
    //  out since the fIdCounter value tells us which ones are valid. The
    //  zeroth element is never used (and represents an invalid pool id.)
    //
    if (!fIdPtrsCount)
        fIdPtrsCount = 256;

    fIdPtrs = (TVal**) fMemoryManager->allocate(fIdPtrsCount * sizeof(TVal*));//new TVal*[fIdPtrsCount];
    fIdPtrs[0] = 0;
}

template <class TVal, class THasher>
DOMDeepNodeListPool<TVal, THasher>::DOMDeepNodeListPool( const XMLSize_t modulus
                                              , const XMLSize_t initSize) :
    fAdoptedElems(true)
    , fBucketList(0)
    , fHashModulus(modulus)
    , fIdPtrs(0)
    , fIdPtrsCount(initSize)
    , fIdCounter(0)
    , fMemoryManager(XMLPlatformUtils::fgMemoryManager)
{
    initialize(modulus);

    //
    //  Allocate the initial id pointers array. We don't have to zero them
    //  out since the fIdCounter value tells us which ones are valid. The
    //  zeroth element is never used (and represents an invalid pool id.)
    //
    if (!fIdPtrsCount)
        fIdPtrsCount = 256;

    fIdPtrs = (TVal**) fMemoryManager->allocate(fIdPtrsCount * sizeof(TVal*));//new TVal*[fIdPtrsCount];
    fIdPtrs[0] = 0;
}

template <class TVal, class THasher>
void DOMDeepNodeListPool<TVal, THasher>::initialize(const XMLSize_t modulus)
{
	if (modulus == 0)
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::HshTbl_ZeroModulus, fMemoryManager);

    // Allocate the bucket list and zero them
    fBucketList = (DOMDeepNodeListPoolTableBucketElem<TVal>**)
        fMemoryManager->allocate
        (
            fHashModulus * sizeof(DOMDeepNodeListPoolTableBucketElem<TVal>*)
        );//new DOMDeepNodeListPoolTableBucketElem<TVal>*[fHashModulus];
    for (XMLSize_t index = 0; index < fHashModulus; index++)
        fBucketList[index] = 0;
}

template <class TVal, class THasher>
DOMDeepNodeListPool<TVal, THasher>::~DOMDeepNodeListPool()
{
    removeAll();

    // Then delete the bucket list & hasher & id pointers list
    fMemoryManager->deallocate(fIdPtrs);//delete [] fIdPtrs;
    fMemoryManager->deallocate(fBucketList);//delete [] fBucketList;
}


// ---------------------------------------------------------------------------
//  DOMDeepNodeListPool: Element management
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
bool DOMDeepNodeListPool<TVal, THasher>::isEmpty() const
{
    // Just check the bucket list for non-empty elements
    for (XMLSize_t buckInd = 0; buckInd < fHashModulus; buckInd++)
    {
        if (fBucketList[buckInd] != 0)
            return false;
    }
    return true;
}

template <class TVal, class THasher>
bool DOMDeepNodeListPool<TVal, THasher>::containsKey( const void* const key1
                                           , const XMLCh* const key2
                                           , const XMLCh* const key3) const
{
    XMLSize_t hashVal;
    const DOMDeepNodeListPoolTableBucketElem<TVal>* findIt = findBucketElem(key1, key2, key3, hashVal);
    return (findIt != 0);
}

template <class TVal, class THasher>
void DOMDeepNodeListPool<TVal, THasher>::removeAll()
{
    if (fIdCounter == 0) return;

    // Clean up the buckets first
    for (XMLSize_t buckInd = 0; buckInd < fHashModulus; buckInd++)
    {
        // Get the bucket list head for this entry
        DOMDeepNodeListPoolTableBucketElem<TVal>* curElem = fBucketList[buckInd];
        DOMDeepNodeListPoolTableBucketElem<TVal>* nextElem;
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
            fMemoryManager->deallocate(curElem->fKey2);//delete [] curElem->fKey2;
            fMemoryManager->deallocate(curElem->fKey3);//delete [] curElem->fKey3;

            delete curElem;
            curElem = nextElem;
        }

        // Clean out this entry
        fBucketList[buckInd] = 0;
    }

    // Reset the id counter
    fIdCounter = 0;
}

template <class TVal, class THasher>
void DOMDeepNodeListPool<TVal, THasher>::cleanup()
{
    removeAll();

    // Then delete the bucket list & hasher & id pointers list
    fMemoryManager->deallocate(fIdPtrs);//delete [] fIdPtrs;
    fMemoryManager->deallocate(fBucketList);//delete [] fBucketList;
}



// ---------------------------------------------------------------------------
//  DOMDeepNodeListPool: Getters
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
TVal*
DOMDeepNodeListPool<TVal, THasher>::getByKey(const void* const key1, const XMLCh* const key2, const XMLCh* const key3)
{
    XMLSize_t hashVal;
    DOMDeepNodeListPoolTableBucketElem<TVal>* findIt = findBucketElem(key1, key2, key3, hashVal);
    if (!findIt)
        return 0;
    return findIt->fData;
}

template <class TVal, class THasher>
const TVal*
DOMDeepNodeListPool<TVal, THasher>::getByKey(const void* const key1, const XMLCh* const key2, const XMLCh* const key3) const
{
    XMLSize_t hashVal;
    const DOMDeepNodeListPoolTableBucketElem<TVal>* findIt = findBucketElem(key1, key2, key3, hashVal);
    if (!findIt)
        return 0;
    return findIt->fData;
}

template <class TVal, class THasher>
TVal*
DOMDeepNodeListPool<TVal, THasher>::getById(const XMLSize_t elemId)
{
    // If its either zero or beyond our current id, its an error
    if (!elemId || (elemId > fIdCounter))
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::Pool_InvalidId, fMemoryManager);

    return fIdPtrs[elemId];
}

template <class TVal, class THasher>
const TVal*
DOMDeepNodeListPool<TVal, THasher>::getById(const XMLSize_t elemId) const
{
    // If its either zero or beyond our current id, its an error
    if (!elemId || (elemId > fIdCounter))
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::Pool_InvalidId, fMemoryManager);

    return fIdPtrs[elemId];
}

// ---------------------------------------------------------------------------
//  DOMDeepNodeListPool: Putters
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
XMLSize_t
DOMDeepNodeListPool<TVal, THasher>::put(void* key1, XMLCh* key2, XMLCh* key3, TVal* const valueToAdopt)
{
    // First see if the key exists already
    XMLSize_t hashVal;
    DOMDeepNodeListPoolTableBucketElem<TVal>* newBucket = findBucketElem(key1, key2, key3, hashVal);

    //
    //  If so,then update its value. If not, then we need to add it to
    //  the right bucket
    //
    if (newBucket)
    {
        if (fAdoptedElems)
            delete newBucket->fData;

        fMemoryManager->deallocate(newBucket->fKey2);//delete[] newBucket->fKey2;
        fMemoryManager->deallocate(newBucket->fKey3);//delete[] newBucket->fKey3;

        newBucket->fData = valueToAdopt;
        newBucket->fKey1 = key1;
        newBucket->fKey2 = XMLString::replicate(key2, fMemoryManager);
        newBucket->fKey3 = XMLString::replicate(key3, fMemoryManager);
    }
    else
    {
    // Revisit: the gcc compiler 2.95.x is generating an
    // internal compiler error message. So we use the default
    // memory manager for now.
#if defined (XML_GCC_VERSION) && (XML_GCC_VERSION < 29600)
        newBucket = new DOMDeepNodeListPoolTableBucketElem<TVal>
        (
            key1
            , key2
            , key3
            , valueToAdopt
            , fBucketList[hashVal]
            , fMemoryManager
        );
#else
        newBucket = new (fMemoryManager) DOMDeepNodeListPoolTableBucketElem<TVal>
        (
            key1
            , key2
            , key3
            , valueToAdopt
            , fBucketList[hashVal]
            , fMemoryManager
        );
#endif
        fBucketList[hashVal] = newBucket;
    }

    //
    //  Give this new one the next available id and add to the pointer list.
    //  Expand the list if that is now required.
    //
    if (fIdCounter + 1 == fIdPtrsCount)
    {
        // Create a new count 1.5 times larger and allocate a new array
        XMLSize_t newCount = (XMLSize_t)(fIdPtrsCount * 1.5);
        TVal** newArray = (TVal**) fMemoryManager->allocate
        (
            newCount * sizeof(TVal*)
        );//new TVal*[newCount];

        // Copy over the old contents to the new array
        memcpy(newArray, fIdPtrs, fIdPtrsCount * sizeof(TVal*));

        // Ok, toss the old array and store the new data
        fMemoryManager->deallocate(fIdPtrs); //delete [] fIdPtrs;
        fIdPtrs = newArray;
        fIdPtrsCount = newCount;
    }
    const XMLSize_t retId = ++fIdCounter;
    fIdPtrs[retId] = valueToAdopt;

    // Return the id that we gave to this element
    return retId;
}

// ---------------------------------------------------------------------------
//  DOMDeepNodeListPool: Private methods
// ---------------------------------------------------------------------------
template <class TVal, class THasher>
DOMDeepNodeListPoolTableBucketElem<TVal>* DOMDeepNodeListPool<TVal, THasher>::
findBucketElem(const void* const key1, const XMLCh* const key2, const XMLCh* const key3, XMLSize_t& hashVal)
{
    // Hash the key
    hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    // Search that bucket for the key
    DOMDeepNodeListPoolTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    while (curElem)
    {
        //key2 and key3 are XMLCh*, compareString takes null pointer vs zero len string the same
        //but we need them to be treated as different keys in this case
        if (fHasher.equals(key1, curElem->fKey1) && (XMLString::equals(key2, curElem->fKey2)) && (XMLString::equals(key3, curElem->fKey3))) {
            if (!key2 || !curElem->fKey2) {
                if (key2 || curElem->fKey2) {
                    curElem = curElem->fNext;
                    continue;
                }
            }
            if (!key3 || !curElem->fKey3) {
                if (key3 || curElem->fKey3) {
                    curElem = curElem->fNext;
                    continue;
                }
            }

            return curElem;
        }

        curElem = curElem->fNext;
    }
    return 0;
}

template <class TVal, class THasher>
const DOMDeepNodeListPoolTableBucketElem<TVal>* DOMDeepNodeListPool<TVal, THasher>::
findBucketElem(const void* const key1, const XMLCh* const key2, const XMLCh* const key3, XMLSize_t& hashVal) const
{
    // Hash the key
    hashVal = fHasher.getHashVal(key1, fHashModulus);
    assert(hashVal < fHashModulus);

    // Search that bucket for the key
    const DOMDeepNodeListPoolTableBucketElem<TVal>* curElem = fBucketList[hashVal];
    while (curElem)
    {
        //key2 and key3 are XMLCh*, compareString takes null pointer vs zero len string the same
        //but we need them to be treated as different keys in this case
        if (fHasher.equals(key1, curElem->fKey1) && (XMLString::equals(key2, curElem->fKey2)) && (XMLString::equals(key3, curElem->fKey3))) {
            if (!key2 || !curElem->fKey2) {
                if (key2 || curElem->fKey2) {
                    curElem = curElem->fNext;
                    continue;
                }
            }
            if (!key3 || !curElem->fKey3) {
                if (key3 || curElem->fKey3) {
                    curElem = curElem->fNext;
                    continue;
                }
            }
            return curElem;
        }

        curElem = curElem->fNext;
    }
    return 0;
}

XERCES_CPP_NAMESPACE_END
