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
 * $Id: RefStackOf.c 676911 2008-07-15 13:27:32Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/RefStackOf.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  RefStackOf: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem>
RefStackOf<TElem>::RefStackOf(const XMLSize_t initElems,
                              const bool adoptElems,
                              MemoryManager* const manager) :

    fVector(initElems, adoptElems, manager)
{
}

template <class TElem> RefStackOf<TElem>::~RefStackOf()
{
}


// ---------------------------------------------------------------------------
//  RefStackOf: Element management methods
// ---------------------------------------------------------------------------
template <class TElem> const TElem* RefStackOf<TElem>::
elementAt(const XMLSize_t index) const
{
    if (index >= fVector.size())
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Stack_BadIndex, fVector.getMemoryManager());
    return fVector.elementAt(index);
}

template <class TElem> TElem* RefStackOf<TElem>::popAt(const XMLSize_t index)
{
    if (index >= fVector.size())
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Stack_BadIndex, fVector.getMemoryManager());

    // Orphan off the element from the slot in the vector
    return fVector.orphanElementAt(index);
}

template <class TElem> void RefStackOf<TElem>::push(TElem* const toPush)
{
    fVector.addElement(toPush);
}

template <class TElem> const TElem* RefStackOf<TElem>::peek() const
{
    const XMLSize_t curSize = fVector.size();
    if (curSize == 0)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::Stack_EmptyStack, fVector.getMemoryManager());

    return fVector.elementAt(curSize-1);
}

template <class TElem> TElem* RefStackOf<TElem>::pop()
{
    const XMLSize_t curSize = fVector.size();
    if (curSize == 0)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::Stack_EmptyStack, fVector.getMemoryManager());

    // Orphan off the element from the last slot in the vector
    return fVector.orphanElementAt(curSize-1);
}

template <class TElem> void RefStackOf<TElem>::removeAllElements()
{
    fVector.removeAllElements();
}


// ---------------------------------------------------------------------------
//  RefStackOf: Getter methods
// ---------------------------------------------------------------------------
template <class TElem> bool RefStackOf<TElem>::empty()
{
    return (fVector.size() == 0);
}

template <class TElem> XMLSize_t RefStackOf<TElem>::curCapacity()
{
    return fVector.curCapacity();
}

template <class TElem> XMLSize_t RefStackOf<TElem>::size()
{
    return fVector.size();
}




// ---------------------------------------------------------------------------
//  RefStackEnumerator: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem> RefStackEnumerator<TElem>::
RefStackEnumerator(         RefStackOf<TElem>* const    toEnum
                    , const bool                        adopt) :
    fAdopted(adopt)
    , fCurIndex(0)
    , fToEnum(toEnum)
    , fVector(&toEnum->fVector)
{
}

template <class TElem> RefStackEnumerator<TElem>::~RefStackEnumerator()
{
    if (fAdopted)
        delete fToEnum;
}


// ---------------------------------------------------------------------------
//  RefStackEnumerator: Enum interface
// ---------------------------------------------------------------------------
template <class TElem> bool RefStackEnumerator<TElem>::hasMoreElements() const
{
    if (fCurIndex >= fVector->size())
        return false;
    return true;
}

template <class TElem> TElem& RefStackEnumerator<TElem>::nextElement()
{
    return *fVector->elementAt(fCurIndex++);
}

template <class TElem> void RefStackEnumerator<TElem>::Reset()
{
    fCurIndex = 0;
}

XERCES_CPP_NAMESPACE_END
