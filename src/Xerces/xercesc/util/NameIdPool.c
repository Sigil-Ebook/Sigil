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
 * $Id: NameIdPool.c 883368 2009-11-23 15:28:19Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/NameIdPool.hpp>
#endif

#include <xercesc/util/IllegalArgumentException.hpp>
#include <xercesc/util/NoSuchElementException.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <new>
#include <assert.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  NameIdPool: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem>
NameIdPool<TElem>::NameIdPool( const XMLSize_t      hashModulus
                             , const XMLSize_t      initSize
                             , MemoryManager* const manager) :
    fMemoryManager(manager)
    , fIdPtrs(0)
    , fIdPtrsCount(initSize)
    , fIdCounter(0)
    , fBucketList(hashModulus, manager)
{
    if (!hashModulus)
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::Pool_ZeroModulus, fMemoryManager);

    //
    //  Allocate the initial id pointers array. We don't have to zero them
    //  out since the fIdCounter value tells us which ones are valid. The
    //  zeroth element is never used (and represents an invalid pool id.)
    //
    if (!fIdPtrsCount)
        fIdPtrsCount = 256;
    fIdPtrs = (TElem**) fMemoryManager->allocate
    (
        fIdPtrsCount * sizeof(TElem*)
    );
    fIdPtrs[0] = 0;
}

template <class TElem> NameIdPool<TElem>::~NameIdPool()
{
    //
    //  Delete the id pointers list. The stuff it points to will be cleaned
    //  up when we clean the bucket lists.
    //
    fMemoryManager->deallocate(fIdPtrs); //delete [] fIdPtrs;
}


// ---------------------------------------------------------------------------
//  NameIdPool: Element management
// ---------------------------------------------------------------------------
template <class TElem>
inline bool NameIdPool<TElem>::
containsKey(const XMLCh* const key) const
{
    if (fIdCounter == 0) return false;
    return fBucketList.containsKey(key);
}


template <class TElem> void NameIdPool<TElem>::removeAll()
{
    if (fIdCounter == 0) return;

    fBucketList.removeAll();

    // Reset the id counter
    fIdCounter = 0;
}


// ---------------------------------------------------------------------------
//  NameIdPool: Getters
// ---------------------------------------------------------------------------
template <class TElem>
inline TElem* NameIdPool<TElem>::
getByKey(const XMLCh* const key)
{
    if (fIdCounter == 0) return 0;
    return fBucketList.get(key);
}

template <class TElem>
inline const TElem* NameIdPool<TElem>::
getByKey(const XMLCh* const key) const
{
    if (fIdCounter == 0) return 0;
    return fBucketList.get(key);
}

template <class TElem>
inline TElem* NameIdPool<TElem>::
getById(const XMLSize_t elemId)
{
    // If its either zero or beyond our current id, its an error
    if (!elemId || (elemId > fIdCounter))
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::Pool_InvalidId, fMemoryManager);

    return fIdPtrs[elemId];
}

template <class TElem>
inline const TElem* NameIdPool<TElem>::
getById(const XMLSize_t elemId) const
{
    // If its either zero or beyond our current id, its an error
    if (!elemId || (elemId > fIdCounter))
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::Pool_InvalidId, fMemoryManager);

    return fIdPtrs[elemId];
}

template <class TElem>
inline MemoryManager* NameIdPool<TElem>::getMemoryManager() const
{
    return fMemoryManager;
}

// ---------------------------------------------------------------------------
//  NameIdPool: Setters
// ---------------------------------------------------------------------------
template <class TElem>
XMLSize_t NameIdPool<TElem>::put(TElem* const elemToAdopt)
{
    // First see if the key exists already. If so, its an error
    if(containsKey(elemToAdopt->getKey()))
    {
        ThrowXMLwithMemMgr1
        (
            IllegalArgumentException
            , XMLExcepts::Pool_ElemAlreadyExists
            , elemToAdopt->getKey()
            , fMemoryManager
        );
    }

    fBucketList.put((void*)elemToAdopt->getKey(), elemToAdopt);

    //
    //  Give this new one the next available id and add to the pointer list.
    //  Expand the list if that is now required.
    //
    if (fIdCounter + 1 == fIdPtrsCount)
    {
        // Create a new count 1.5 times larger and allocate a new array
        XMLSize_t newCount = (XMLSize_t)(fIdPtrsCount * 1.5);
        TElem** newArray = (TElem**) fMemoryManager->allocate
        (
            newCount * sizeof(TElem*)
        ); //new TElem*[newCount];

        // Copy over the old contents to the new array
        memcpy(newArray, fIdPtrs, fIdPtrsCount * sizeof(TElem*));

        // Ok, toss the old array and store the new data
        fMemoryManager->deallocate(fIdPtrs); //delete [] fIdPtrs;
        fIdPtrs = newArray;
        fIdPtrsCount = newCount;
    }
    const XMLSize_t retId = ++fIdCounter;
    fIdPtrs[retId] = elemToAdopt;

    // Set the id on the passed element
    elemToAdopt->setId(retId);

    // Return the id that we gave to this element
    return retId;
}


// ---------------------------------------------------------------------------
//  NameIdPoolEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem> NameIdPoolEnumerator<TElem>::
NameIdPoolEnumerator(NameIdPool<TElem>* const toEnum
                     , MemoryManager* const manager) :

    XMLEnumerator<TElem>()
    , fCurIndex(0)
    , fToEnum(toEnum)
    , fMemoryManager(manager)
{
    Reset();
}

template <class TElem> NameIdPoolEnumerator<TElem>::
NameIdPoolEnumerator(const NameIdPoolEnumerator<TElem>& toCopy) :
    XMLEnumerator<TElem>(toCopy)
    , XMemory(toCopy)
    , fCurIndex(toCopy.fCurIndex)
    , fToEnum(toCopy.fToEnum)
    , fMemoryManager(toCopy.fMemoryManager)
{
}

template <class TElem> NameIdPoolEnumerator<TElem>::~NameIdPoolEnumerator()
{
    // We don't own the pool being enumerated, so no cleanup required
}


// ---------------------------------------------------------------------------
//  NameIdPoolEnumerator: Public operators
// ---------------------------------------------------------------------------
template <class TElem> NameIdPoolEnumerator<TElem>& NameIdPoolEnumerator<TElem>::
operator=(const NameIdPoolEnumerator<TElem>& toAssign)
{
    if (this == &toAssign)
        return *this;
    fMemoryManager = toAssign.fMemoryManager;
    fCurIndex      = toAssign.fCurIndex;
    fToEnum        = toAssign.fToEnum;
    return *this;
}

// ---------------------------------------------------------------------------
//  NameIdPoolEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class TElem> bool NameIdPoolEnumerator<TElem>::
hasMoreElements() const
{
    // If our index is zero or past the end, then we are done
    if (!fCurIndex || (fCurIndex > fToEnum->fIdCounter))
        return false;
    return true;
}

template <class TElem> TElem& NameIdPoolEnumerator<TElem>::nextElement()
{
    // If our index is zero or past the end, then we are done
    if (!fCurIndex || (fCurIndex > fToEnum->fIdCounter))
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::Enum_NoMoreElements, fMemoryManager);

    // Return the current element and bump the index
    return *fToEnum->fIdPtrs[fCurIndex++];
}


template <class TElem> void NameIdPoolEnumerator<TElem>::Reset()
{
    //
    //  Find the next available bucket element in the pool. We use the id
    //  array since its very easy to enumerator through by just maintaining
    //  an index. If the id counter is zero, then its empty and we leave the
    //  current index to zero.
    //
    fCurIndex = fToEnum->fIdCounter ? 1:0;
}

template <class TElem> XMLSize_t NameIdPoolEnumerator<TElem>::size() const
{
    return fToEnum->fIdCounter;
}

XERCES_CPP_NAMESPACE_END
