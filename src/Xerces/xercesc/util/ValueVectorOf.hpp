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
 * $Id: ValueVectorOf.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_VALUEVECTOROF_HPP)
#define XERCESC_INCLUDE_GUARD_VALUEVECTOROF_HPP

#include <xercesc/util/ArrayIndexOutOfBoundsException.hpp>
#include <xercesc/util/XMLEnumerator.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemoryManager.hpp>

XERCES_CPP_NAMESPACE_BEGIN

template <class TElem> class ValueVectorOf : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ValueVectorOf
    (
        const XMLSize_t maxElems
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
        , const bool toCallDestructor = false
    );
    ValueVectorOf(const ValueVectorOf<TElem>& toCopy);
    ~ValueVectorOf();


    // -----------------------------------------------------------------------
    //  Operators
    // -----------------------------------------------------------------------
    ValueVectorOf<TElem>& operator=(const ValueVectorOf<TElem>& toAssign);


    // -----------------------------------------------------------------------
    //  Element management
    // -----------------------------------------------------------------------
    void addElement(const TElem& toAdd);
    void setElementAt(const TElem& toSet, const XMLSize_t setAt);
    void insertElementAt(const TElem& toInsert, const XMLSize_t insertAt);
    void removeElementAt(const XMLSize_t removeAt);
    void removeAllElements();
    bool containsElement(const TElem& toCheck, const XMLSize_t startIndex = 0);


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const TElem& elementAt(const XMLSize_t getAt) const;
    TElem& elementAt(const XMLSize_t getAt);
    XMLSize_t curCapacity() const;
    XMLSize_t size() const;
    MemoryManager* getMemoryManager() const;


    // -----------------------------------------------------------------------
    //  Miscellaneous
    // -----------------------------------------------------------------------
    void ensureExtraCapacity(const XMLSize_t length);
    const TElem* rawData() const;


private:
    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fCurCount
    //      The count of values current added to the vector, which may be
    //      less than the internal capacity.
    //
    //  fMaxCount
    //      The current capacity of the vector.
    //
    //  fElemList
    //      The list of elements, which is dynamically allocated to the needed
    //      size.
    // -----------------------------------------------------------------------
    bool            fCallDestructor;
    XMLSize_t       fCurCount;
    XMLSize_t       fMaxCount;
    TElem*          fElemList;
    MemoryManager*  fMemoryManager;
};


//
//  An enumerator for a value vector. It derives from the basic enumerator
//  class, so that value vectors can be generically enumerated.
//
template <class TElem> class ValueVectorEnumerator : public XMLEnumerator<TElem>, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ValueVectorEnumerator
    (
                ValueVectorOf<TElem>* const toEnum
        , const bool                        adopt = false
    );
    virtual ~ValueVectorEnumerator();


    // -----------------------------------------------------------------------
    //  Enum interface
    // -----------------------------------------------------------------------
    bool hasMoreElements() const;
    TElem& nextElement();
    void Reset();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------    
    ValueVectorEnumerator(const ValueVectorEnumerator<TElem>&);
    ValueVectorEnumerator<TElem>& operator=(const ValueVectorEnumerator<TElem>&);

    // -----------------------------------------------------------------------
    //  Data Members
    //
    //  fAdopted
    //      Indicates whether we have adopted the passed vector. If so then
    //      we delete the vector when we are destroyed.
    //
    //  fCurIndex
    //      This is the current index into the vector.
    //
    //  fToEnum
    //      The value vector being enumerated.
    // -----------------------------------------------------------------------
    bool                    fAdopted;
    XMLSize_t               fCurIndex;
    ValueVectorOf<TElem>*   fToEnum;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/ValueVectorOf.c>
#endif

#endif
