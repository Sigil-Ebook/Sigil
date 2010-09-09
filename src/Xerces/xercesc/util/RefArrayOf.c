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
 * $Id: RefArrayOf.c 932887 2010-04-11 13:04:59Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/RefArrayOf.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  RefArrayOf: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem>
RefArrayOf<TElem>::RefArrayOf(const XMLSize_t size,
                              MemoryManager* const manager) :

    fSize(size)
    , fArray(0)
    , fMemoryManager(manager)
{
    fArray = (TElem**) fMemoryManager->allocate(fSize * sizeof(TElem*));//new TElem*[fSize];
    for (XMLSize_t index = 0; index < fSize; index++)
        fArray[index] = 0;
}

template <class TElem>
RefArrayOf<TElem>::RefArrayOf(TElem* values[],
                              const XMLSize_t size,
                              MemoryManager* const manager) :

    fSize(size)
    , fArray(0)
    , fMemoryManager(manager)
{
    fArray = (TElem**) fMemoryManager->allocate(fSize * sizeof(TElem*));//new TElem*[fSize];
    for (XMLSize_t index = 0; index < fSize; index++)
        fArray[index] = values[index];
}

template <class TElem> RefArrayOf<TElem>::
RefArrayOf(const RefArrayOf<TElem>& source) :

    fSize(source.fSize)
    , fArray(0)
    , fMemoryManager(source.fMemoryManager)
{
    fArray = (TElem**) fMemoryManager->allocate(fSize * sizeof(TElem*));//new TElem*[fSize];
    for (XMLSize_t index = 0; index < fSize; index++)
        fArray[index] = source.fArray[index];
}

template <class TElem> RefArrayOf<TElem>::~RefArrayOf()
{
    fMemoryManager->deallocate(fArray);//delete [] fArray;
}


// ---------------------------------------------------------------------------
//  RefArrayOf: Public operators
// ---------------------------------------------------------------------------
template <class TElem> TElem*& RefArrayOf<TElem>::
operator[](const XMLSize_t index)
{
    if (index >= fSize)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Array_BadIndex, fMemoryManager);
    return fArray[index];
}

template <class TElem> const TElem* RefArrayOf<TElem>::
operator[](const XMLSize_t index) const
{
    if (index >= fSize)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Array_BadIndex, fMemoryManager);
    return fArray[index];
}

template <class TElem> RefArrayOf<TElem>& RefArrayOf<TElem>::
operator=(const RefArrayOf<TElem>& toAssign)
{
    if (this == &toAssign)
        return *this;

    // Reallocate if not the same size
    if (toAssign.fSize != fSize)
    {
        fMemoryManager->deallocate(fArray);//delete [] fArray;
        fSize = toAssign.fSize;
        fArray = (TElem**) fMemoryManager->allocate(fSize * sizeof(TElem*));//new TElem*[fSize];
    }

    // Copy over the source elements
    for (XMLSize_t index = 0; index < fSize; index++)
        fArray[index] = toAssign.fArray[index];

    return *this;
}

template <class TElem> bool RefArrayOf<TElem>::
operator==(const RefArrayOf<TElem>& toCompare) const
{
    if (this == &toCompare)
        return true;

    if (fSize != toCompare.fSize)
        return false;

    for (XMLSize_t index = 0; index < fSize; index++)
    {
        if (fArray[index] != toCompare.fArray[index])
            return false;
    }
    return true;
}

template <class TElem> bool RefArrayOf<TElem>::
operator!=(const RefArrayOf<TElem>& toCompare) const
{
    return !operator==(toCompare);
}


// ---------------------------------------------------------------------------
//  RefArrayOf: Copy operations
// ---------------------------------------------------------------------------
template <class TElem> XMLSize_t RefArrayOf<TElem>::
copyFrom(const RefArrayOf<TElem>& srcArray)
{
    //
    //  Copy over as many of the source elements as will fit into
    //  this array.
    //
    const XMLSize_t count = fSize < srcArray.fSize ?
                                    fSize : srcArray.fSize;

    for (XMLSize_t index = 0; index < fSize; index++)
        fArray[index] = srcArray.fArray[index];

    return count;
}


// ---------------------------------------------------------------------------
//  RefArrayOf: Getter methods
// ---------------------------------------------------------------------------
template <class TElem> XMLSize_t RefArrayOf<TElem>::length() const
{
    return fSize;
}

template <class TElem> TElem** RefArrayOf<TElem>::rawData() const
{
    return fArray;
}


// ---------------------------------------------------------------------------
//  RefArrayOf: Element management methods
// ---------------------------------------------------------------------------
template <class TElem> void RefArrayOf<TElem>::deleteAt(const XMLSize_t index)
{
    if (index >= fSize)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Array_BadIndex, fMemoryManager);

    delete fArray[index];
    fArray[index] = 0;
}

template <class TElem> void RefArrayOf<TElem>::deleteAllElements()
{
    for (XMLSize_t index = 0; index < fSize; index++)
    {
        delete fArray[index];
        fArray[index] = 0;
    }
}

template <class TElem> void RefArrayOf<TElem>::resize(const XMLSize_t newSize)
{
    if (newSize == fSize)
        return;

    if (newSize < fSize)
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::Array_BadNewSize, fMemoryManager);

    // Allocate the new array
    TElem** newArray = (TElem**) fMemoryManager->allocate
    (
        newSize * sizeof(TElem*)
    );//new TElem*[newSize];

    // Copy the existing values
    XMLSize_t index = 0;
    for (; index < fSize; index++)
        newArray[index] = fArray[index];

    for (; index < newSize; index++)
        newArray[index] = 0;

    // Delete the old array and update our members
    fMemoryManager->deallocate(fArray);//delete [] fArray;
    fArray = newArray;
    fSize = newSize;
}




// ---------------------------------------------------------------------------
//  RefArrayEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem> RefArrayEnumerator<TElem>::
RefArrayEnumerator(         RefArrayOf<TElem>* const    toEnum
                    , const bool                        adopt) :
    fAdopted(adopt)
    , fCurIndex(0)
    , fToEnum(toEnum)
{
}

template <class TElem> RefArrayEnumerator<TElem>::~RefArrayEnumerator()
{
    if (fAdopted)
        delete fToEnum;
}


// ---------------------------------------------------------------------------
//  RefArrayEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class TElem> bool RefArrayEnumerator<TElem>::hasMoreElements() const
{
    if (fCurIndex >= fToEnum->length())
        return false;
    return true;
}

template <class TElem> TElem& RefArrayEnumerator<TElem>::nextElement()
{
    return *(*fToEnum)[fCurIndex++];
}

template <class TElem> void RefArrayEnumerator<TElem>::Reset()
{
    fCurIndex = 0;
}

XERCES_CPP_NAMESPACE_END
