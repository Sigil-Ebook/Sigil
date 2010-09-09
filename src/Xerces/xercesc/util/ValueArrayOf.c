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
 * $Id: ValueArrayOf.c 932887 2010-04-11 13:04:59Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/ValueArrayOf.hpp>
#endif


XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  ValueArrayOf: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem>
ValueArrayOf<TElem>::ValueArrayOf(const XMLSize_t size,
                                  MemoryManager* const manager) :

    fSize(size)
    , fArray(0)
    , fMemoryManager(manager)
{
    fArray = (TElem*) fMemoryManager->allocate(fSize * sizeof(TElem)); //new TElem[fSize];
}

template <class TElem>
ValueArrayOf<TElem>::ValueArrayOf( const TElem* values
                                 , const XMLSize_t size
                                 , MemoryManager* const manager) :

    fSize(size)
    , fArray(0)
    , fMemoryManager(manager)
{
    fArray = (TElem*) fMemoryManager->allocate(fSize * sizeof(TElem)); //new TElem[fSize];
    for (XMLSize_t index = 0; index < fSize; index++)
        fArray[index] = values[index];
}

template <class TElem>
ValueArrayOf<TElem>::ValueArrayOf(const ValueArrayOf<TElem>& source) :
    XMemory(source)
    , fSize(source.fSize)
    , fArray(0)
    , fMemoryManager(source.fMemoryManager)
{
    fArray = (TElem*) fMemoryManager->allocate(fSize * sizeof(TElem)); //new TElem[fSize];
    for (XMLSize_t index = 0; index < fSize; index++)
        fArray[index] = source.fArray[index];
}

template <class TElem> ValueArrayOf<TElem>::~ValueArrayOf()
{
    fMemoryManager->deallocate(fArray); //delete [] fArray;
}


// ---------------------------------------------------------------------------
//  ValueArrayOf: Public operators
// ---------------------------------------------------------------------------
template <class TElem> TElem& ValueArrayOf<TElem>::
operator[](const XMLSize_t index)
{
    if (index >= fSize)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Array_BadIndex, fMemoryManager);
    return fArray[index];
}

template <class TElem> const TElem& ValueArrayOf<TElem>::
operator[](const XMLSize_t index) const
{
    if (index >= fSize)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Array_BadIndex, fMemoryManager);
    return fArray[index];
}

template <class TElem> ValueArrayOf<TElem>& ValueArrayOf<TElem>::
operator=(const ValueArrayOf<TElem>& toAssign)
{
    if (this == &toAssign)
        return *this;

    // Reallocate if not the same size
    if (toAssign.fSize != fSize)
    {
        fMemoryManager->deallocate(fArray); //delete [] fArray;
        fSize = toAssign.fSize;
        fArray = (TElem*) fMemoryManager->allocate(fSize * sizeof(TElem)); //new TElem[fSize];
    }

    // Copy over the source elements
    for (XMLSize_t index = 0; index < fSize; index++)
        fArray[index] = toAssign.fArray[index];

    return *this;
}

template <class TElem> bool ValueArrayOf<TElem>::
operator==(const ValueArrayOf<TElem>& toCompare) const
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

template <class TElem> bool ValueArrayOf<TElem>::
operator!=(const ValueArrayOf<TElem>& toCompare) const
{
    return !operator==(toCompare);
}


// ---------------------------------------------------------------------------
//  ValueArrayOf: Copy operations
// ---------------------------------------------------------------------------
template <class TElem> XMLSize_t ValueArrayOf<TElem>::
copyFrom(const ValueArrayOf<TElem>& srcArray)
{
    //
    //  Copy over as many of the source elements as will fit into
    //  this array.
    //
    const XMLSize_t count = fSize < srcArray.fSize ?
                                fSize : srcArray.fSize;

    for (XMLSize_t index = 0; index < count; index++)
        fArray[index] = srcArray.fArray[index];

    return count;
}


// ---------------------------------------------------------------------------
//  ValueArrayOf: Getter methods
// ---------------------------------------------------------------------------
template <class TElem> XMLSize_t ValueArrayOf<TElem>::
length() const
{
    return fSize;
}

template <class TElem> TElem* ValueArrayOf<TElem>::
rawData() const
{
    return fArray;
}


// ---------------------------------------------------------------------------
//  ValueArrayOf: Miscellaneous methods
// ---------------------------------------------------------------------------
template <class TElem> void ValueArrayOf<TElem>::
resize(const XMLSize_t newSize)
{
    if (newSize == fSize)
        return;

    if (newSize < fSize)
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::Array_BadNewSize, fMemoryManager);

    // Allocate the new array
    TElem* newArray = (TElem*) fMemoryManager->allocate
    (
        newSize * sizeof(TElem)
    ); //new TElem[newSize];

    // Copy the existing values
    XMLSize_t index = 0;
    for (; index < fSize; index++)
        newArray[index] = fArray[index];

    for (; index < newSize; index++)
        newArray[index] = TElem(0);

    // Delete the old array and update our members
    fMemoryManager->deallocate(fArray); //delete [] fArray;
    fArray = newArray;
    fSize = newSize;
}



// ---------------------------------------------------------------------------
//  ValueArrayEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem> ValueArrayEnumerator<TElem>::
ValueArrayEnumerator(ValueArrayOf<TElem>* const toEnum, const bool adopt) :
    fAdopted(adopt)
    , fCurIndex(0)
    , fToEnum(toEnum)
{
}

template <class TElem> ValueArrayEnumerator<TElem>::~ValueArrayEnumerator()
{
    if (fAdopted)
        delete fToEnum;
}


// ---------------------------------------------------------------------------
//  ValueArrayEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class TElem> bool ValueArrayEnumerator<TElem>::hasMoreElements() const
{
    if (fCurIndex >= fToEnum->length())
        return false;
    return true;
}

template <class TElem> TElem& ValueArrayEnumerator<TElem>::nextElement()
{
    return (*fToEnum)[fCurIndex++];
}

template <class TElem> void ValueArrayEnumerator<TElem>::Reset()
{
    fCurIndex = 0;
}

XERCES_CPP_NAMESPACE_END
