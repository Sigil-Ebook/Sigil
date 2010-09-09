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
// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/BaseRefVectorOf.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  BaseRefVectorOf: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem>
BaseRefVectorOf<TElem>::BaseRefVectorOf( const XMLSize_t maxElems
                                       , const bool adoptElems
                                       , MemoryManager* const manager) :

    fAdoptedElems(adoptElems)
    , fCurCount(0)
    , fMaxCount(maxElems)
    , fElemList(0)
    , fMemoryManager(manager)
{
    // Allocate and initialize the array
    fElemList = (TElem**) fMemoryManager->allocate(maxElems * sizeof(TElem*));//new TElem*[maxElems];
    for (XMLSize_t index = 0; index < maxElems; index++)
        fElemList[index] = 0;
}


//implemented so code will link
template <class TElem> BaseRefVectorOf<TElem>::~BaseRefVectorOf()
{
}


// ---------------------------------------------------------------------------
//  BaseRefVectorOf: Element management
// ---------------------------------------------------------------------------
template <class TElem> void BaseRefVectorOf<TElem>::addElement(TElem* const toAdd)
{
    ensureExtraCapacity(1);
    fElemList[fCurCount] = toAdd;
    fCurCount++;
}


template <class TElem> void
BaseRefVectorOf<TElem>::setElementAt(TElem* const toSet, const XMLSize_t setAt)
{
    if (setAt >= fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);

    if (fAdoptedElems)
        delete fElemList[setAt];
    fElemList[setAt] = toSet;
}

template <class TElem> void BaseRefVectorOf<TElem>::
insertElementAt(TElem* const toInsert, const XMLSize_t insertAt)
{
    if (insertAt == fCurCount)
    {
        addElement(toInsert);
        return;
    }

    if (insertAt > fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);

    ensureExtraCapacity(1);

    // Make room for the newbie
    for (XMLSize_t index = fCurCount; index > insertAt; index--)
        fElemList[index] = fElemList[index-1];

    // And stick it in and bump the count
    fElemList[insertAt] = toInsert;
    fCurCount++;
}

template <class TElem> TElem* BaseRefVectorOf<TElem>::
orphanElementAt(const XMLSize_t orphanAt)
{
    if (orphanAt >= fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);

    // Get the element we are going to orphan
    TElem* retVal = fElemList[orphanAt];

    // Optimize if its the last element
    if (orphanAt == fCurCount-1)
    {
        fElemList[orphanAt] = 0;
        fCurCount--;
        return retVal;
    }

    // Copy down every element above orphan point
    for (XMLSize_t index = orphanAt; index < fCurCount-1; index++)
        fElemList[index] = fElemList[index+1];

    // Keep unused elements zero for sanity's sake
    fElemList[fCurCount-1] = 0;

    // And bump down count
    fCurCount--;

    return retVal;
}

template <class TElem> void BaseRefVectorOf<TElem>::removeAllElements()
{
    for (XMLSize_t index = 0; index < fCurCount; index++)
    {
        if (fAdoptedElems)
          delete fElemList[index];

        // Keep unused elements zero for sanity's sake
        fElemList[index] = 0;
    }
    fCurCount = 0;
}

template <class TElem> void BaseRefVectorOf<TElem>::
removeElementAt(const XMLSize_t removeAt)
{
    if (removeAt >= fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);

    if (fAdoptedElems)
        delete fElemList[removeAt];

    // Optimize if its the last element
    if (removeAt == fCurCount-1)
    {
        fElemList[removeAt] = 0;
        fCurCount--;
        return;
    }

    // Copy down every element above remove point
    for (XMLSize_t index = removeAt; index < fCurCount-1; index++)
        fElemList[index] = fElemList[index+1];

    // Keep unused elements zero for sanity's sake
    fElemList[fCurCount-1] = 0;

    // And bump down count
    fCurCount--;
}

template <class TElem> void BaseRefVectorOf<TElem>::removeLastElement()
{
    if (!fCurCount)
        return;
    fCurCount--;

    if (fAdoptedElems)
        delete fElemList[fCurCount];
}

template <class TElem>
bool BaseRefVectorOf<TElem>::containsElement(const TElem* const toCheck) {

    for (XMLSize_t i = 0; i < fCurCount; i++) {
        if (fElemList[i] == toCheck) {
            return true;
        }
    }

    return false;
}

//
// cleanup():
//   similar to destructor
//   called to cleanup the memory, in case destructor cannot be called
//
template <class TElem> void BaseRefVectorOf<TElem>::cleanup()
{
    if (fAdoptedElems)
    {
        for (XMLSize_t index = 0; index < fCurCount; index++)
            delete fElemList[index];
    }
    fMemoryManager->deallocate(fElemList);//delete [] fElemList;
}

//
// reinitialize():
//   similar to constructor
//   called to re-construct the fElemList from scratch again
//
template <class TElem> void BaseRefVectorOf<TElem>::reinitialize()
{
    // reinitialize the array
    if (fElemList)
        cleanup();

    fElemList = (TElem**) fMemoryManager->allocate(fMaxCount * sizeof(TElem*));//new TElem*[fMaxCount];
    for (XMLSize_t index = 0; index < fMaxCount; index++)
        fElemList[index] = 0;

}

template <class TElem>
MemoryManager* BaseRefVectorOf<TElem>::getMemoryManager() const
{
    return fMemoryManager;
}


// ---------------------------------------------------------------------------
//  BaseRefVectorOf: Getter methods
// ---------------------------------------------------------------------------
template <class TElem> XMLSize_t BaseRefVectorOf<TElem>::curCapacity() const
{
    return fMaxCount;
}

template <class TElem> const TElem* BaseRefVectorOf<TElem>::
elementAt(const XMLSize_t getAt) const
{
    if (getAt >= fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);
    return fElemList[getAt];
}

template <class TElem> TElem*
BaseRefVectorOf<TElem>::elementAt(const XMLSize_t getAt)
{
    if (getAt >= fCurCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);
    return fElemList[getAt];
}

template <class TElem> XMLSize_t BaseRefVectorOf<TElem>::size() const
{
    return fCurCount;
}


// ---------------------------------------------------------------------------
//  BaseRefVectorOf: Miscellaneous
// ---------------------------------------------------------------------------
template <class TElem> void BaseRefVectorOf<TElem>::
ensureExtraCapacity(const XMLSize_t length)
{
    XMLSize_t newMax = fCurCount + length;

    if (newMax <= fMaxCount)
        return;

	// Choose how much bigger based on the current size.
	// This will grow half as much again.
    if (newMax < fMaxCount + fMaxCount/2)
        newMax = fMaxCount + fMaxCount/2;

    // Allocate the new array and copy over the existing stuff
    TElem** newList = (TElem**) fMemoryManager->allocate
    (
        newMax * sizeof(TElem*)
    );//new TElem*[newMax];
    XMLSize_t index = 0;
    for (; index < fCurCount; index++)
        newList[index] = fElemList[index];

    // Zero out the rest of them
    for (; index < newMax; index++)
        newList[index] = 0;

    // Clean up the old array and update our members
    fMemoryManager->deallocate(fElemList);//delete [] fElemList;
    fElemList = newList;
    fMaxCount = newMax;
}



// ---------------------------------------------------------------------------
//  AbstractBaseRefVectorEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem> BaseRefVectorEnumerator<TElem>::
BaseRefVectorEnumerator(        BaseRefVectorOf<TElem>* const   toEnum
                    , const bool                        adopt) :
    fAdopted(adopt)
    , fCurIndex(0)
    , fToEnum(toEnum)
{
}

template <class TElem> BaseRefVectorEnumerator<TElem>::~BaseRefVectorEnumerator()
{
    if (fAdopted)
        delete fToEnum;
}

template <class TElem> BaseRefVectorEnumerator<TElem>::
BaseRefVectorEnumerator(const BaseRefVectorEnumerator<TElem>& toCopy) :
    XMLEnumerator<TElem>(toCopy)
    , XMemory(toCopy)
    , fAdopted(toCopy.fAdopted)
    , fCurIndex(toCopy.fCurIndex)
    , fToEnum(toCopy.fToEnum)    
{
}
// ---------------------------------------------------------------------------
//  RefBaseRefVectorEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class TElem> bool BaseRefVectorEnumerator<TElem>::hasMoreElements() const
{
    if (fCurIndex >= fToEnum->size())
        return false;
    return true;
}

template <class TElem> TElem& BaseRefVectorEnumerator<TElem>::nextElement()
{
    return *(fToEnum->elementAt(fCurIndex++));
}

template <class TElem> void BaseRefVectorEnumerator<TElem>::Reset()
{
    fCurIndex = 0;
}

XERCES_CPP_NAMESPACE_END
