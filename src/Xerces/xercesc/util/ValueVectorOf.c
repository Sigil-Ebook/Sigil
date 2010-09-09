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
 * $Id: ValueVectorOf.c 676911 2008-07-15 13:27:32Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/ValueVectorOf.hpp>
#endif
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  ValueVectorOf: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem>
ValueVectorOf<TElem>::ValueVectorOf(const XMLSize_t maxElems,
                                    MemoryManager* const manager,
                                    const bool toCallDestructor) :

    fCallDestructor(toCallDestructor)
    , fCurCount(0)
    , fMaxCount(maxElems)
    , fElemList(0)
    , fMemoryManager(manager)
{
    fElemList = (TElem*) fMemoryManager->allocate
    (
        fMaxCount * sizeof(TElem)
    ); //new TElem[fMaxCount];

    memset(fElemList, 0, fMaxCount * sizeof(TElem));
}

template <class TElem>
ValueVectorOf<TElem>::ValueVectorOf(const ValueVectorOf<TElem>& toCopy) :
    XMemory(toCopy)
    , fCallDestructor(toCopy.fCallDestructor)
    , fCurCount(toCopy.fCurCount)
    , fMaxCount(toCopy.fMaxCount)
    , fElemList(0)
    , fMemoryManager(toCopy.fMemoryManager)
{
    fElemList = (TElem*) fMemoryManager->allocate
    (
        fMaxCount * sizeof(TElem)
    ); //new TElem[fMaxCount];

    memset(fElemList, 0, fMaxCount * sizeof(TElem));
    for (XMLSize_t index = 0; index < fCurCount; index++)
        fElemList[index] = toCopy.fElemList[index];
}

template <class TElem> ValueVectorOf<TElem>::~ValueVectorOf()
{
    if (fCallDestructor) {
        for (XMLSize_t index=fMaxCount; index > 0; index--)
            fElemList[index-1].~TElem();
    }
    fMemoryManager->deallocate(fElemList); //delete [] fElemList;
}



// ---------------------------------------------------------------------------
//  ValueVectorOf: Operators
// ---------------------------------------------------------------------------
template <class TElem> ValueVectorOf<TElem>&
ValueVectorOf<TElem>::operator=(const ValueVectorOf<TElem>& toAssign)
{
    if (this == &toAssign)
        return *this;

    // Reallocate if required
    if (fMaxCount < toAssign.fCurCount)
    {
        fMemoryManager->deallocate(fElemList); //delete [] fElemList;
        fElemList = (TElem*) fMemoryManager->allocate
        (
            toAssign.fMaxCount * sizeof(TElem)
        ); //new TElem[toAssign.fMaxCount];
        fMaxCount = toAssign.fMaxCount;
    }

    fCurCount = toAssign.fCurCount;
    for (XMLSize_t index = 0; index < fCurCount; index++)
        fElemList[index] = toAssign.fElemList[index];

    return *this;
}


// ---------------------------------------------------------------------------
//  ValueVectorOf: Element management
// ---------------------------------------------------------------------------
template <class TElem> void ValueVectorOf<TElem>::addElement(const TElem& toAdd)
{
    ensureExtraCapacity(1);
    fElemList[fCurCount++] = toAdd;
}

template <class TElem> void ValueVectorOf<TElem>::
setElementAt(const TElem& toSet, const XMLSize_t setAt)
{
    if (setAt >= fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);
    fElemList[setAt] = toSet;
}

template <class TElem> void ValueVectorOf<TElem>::
insertElementAt(const TElem& toInsert, const XMLSize_t insertAt)
{
    if (insertAt == fCurCount)
    {
        addElement(toInsert);
        return;
    }

    if (insertAt > fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);

    // Make room for the newbie
    ensureExtraCapacity(1);
    for (XMLSize_t index = fCurCount; index > insertAt; index--)
        fElemList[index] = fElemList[index-1];

    // And stick it in and bump the count
    fElemList[insertAt] = toInsert;
    fCurCount++;
}

template <class TElem> void ValueVectorOf<TElem>::
removeElementAt(const XMLSize_t removeAt)
{
    if (removeAt >= fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);

    // Copy down every element above remove point
    for (XMLSize_t index = removeAt; index < fCurCount-1; index++)
        fElemList[index] = fElemList[index+1];

    // And bump down count
    fCurCount--;
}

template <class TElem> void ValueVectorOf<TElem>::removeAllElements()
{
    fCurCount = 0;
}

template <class TElem>
bool ValueVectorOf<TElem>::containsElement(const TElem& toCheck,
                                           const XMLSize_t startIndex) {

    for (XMLSize_t i = startIndex; i < fCurCount; i++) {
        if (fElemList[i] == toCheck) {
            return true;
        }
    }

    return false;
}


// ---------------------------------------------------------------------------
//  ValueVectorOf: Getter methods
// ---------------------------------------------------------------------------
template <class TElem> const TElem& ValueVectorOf<TElem>::
elementAt(const XMLSize_t getAt) const
{
    if (getAt >= fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);
    return fElemList[getAt];
}

template <class TElem> TElem& ValueVectorOf<TElem>::
elementAt(const XMLSize_t getAt)
{
    if (getAt >= fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);
    return fElemList[getAt];
}

template <class TElem> XMLSize_t ValueVectorOf<TElem>::curCapacity() const
{
    return fMaxCount;
}

template <class TElem> XMLSize_t ValueVectorOf<TElem>::size() const
{
    return fCurCount;
}

template <class TElem>
MemoryManager* ValueVectorOf<TElem>::getMemoryManager() const
{
    return fMemoryManager;
}

// ---------------------------------------------------------------------------
//  ValueVectorOf: Miscellaneous
// ---------------------------------------------------------------------------
template <class TElem> void ValueVectorOf<TElem>::
ensureExtraCapacity(const XMLSize_t length)
{
    XMLSize_t newMax = fCurCount + length;

    if (newMax > fMaxCount)
    {
        // Avoid too many reallocations by expanding by a percentage
        XMLSize_t minNewMax = (XMLSize_t)((double)fCurCount * 1.25);
        if (newMax < minNewMax)
            newMax = minNewMax;

        TElem* newList = (TElem*) fMemoryManager->allocate
        (
            newMax * sizeof(TElem)
        ); //new TElem[newMax];
        for (XMLSize_t index = 0; index < fCurCount; index++)
            newList[index] = fElemList[index];

        fMemoryManager->deallocate(fElemList); //delete [] fElemList;
        fElemList = newList;
        fMaxCount = newMax;
    }
}

template <class TElem> const TElem* ValueVectorOf<TElem>::rawData() const
{
    return fElemList;
}



// ---------------------------------------------------------------------------
//  ValueVectorEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem> ValueVectorEnumerator<TElem>::
ValueVectorEnumerator(       ValueVectorOf<TElem>* const toEnum
                     , const bool                        adopt) :
    fAdopted(adopt)
    , fCurIndex(0)
    , fToEnum(toEnum)
{
}

template <class TElem> ValueVectorEnumerator<TElem>::~ValueVectorEnumerator()
{
    if (fAdopted)
        delete fToEnum;
}


// ---------------------------------------------------------------------------
//  ValueVectorEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class TElem> bool
ValueVectorEnumerator<TElem>::hasMoreElements() const
{
    if (fCurIndex >= fToEnum->size())
        return false;
    return true;
}

template <class TElem> TElem& ValueVectorEnumerator<TElem>::nextElement()
{
    return fToEnum->elementAt(fCurIndex++);
}

template <class TElem> void ValueVectorEnumerator<TElem>::Reset()
{
    fCurIndex = 0;
}

XERCES_CPP_NAMESPACE_END
