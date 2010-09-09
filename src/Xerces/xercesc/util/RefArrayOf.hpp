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
 * $Id: RefArrayOf.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_REFARRAY_HPP)
#define XERCESC_INCLUDE_GUARD_REFARRAY_HPP

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/ArrayIndexOutOfBoundsException.hpp>
#include <xercesc/util/IllegalArgumentException.hpp>
#include <xercesc/util/XMLEnumerator.hpp>
#include <xercesc/framework/MemoryManager.hpp>

XERCES_CPP_NAMESPACE_BEGIN

template <class TElem> class RefArrayOf : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    RefArrayOf
    (
          const XMLSize_t size
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
	RefArrayOf
    (
          TElem* values[]
        , const XMLSize_t size
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
	RefArrayOf(const RefArrayOf<TElem>& source);
	~RefArrayOf();


    // -----------------------------------------------------------------------
    //  Public operators
    // -----------------------------------------------------------------------
	TElem*& operator[](const XMLSize_t index);
	const TElem* operator[](const XMLSize_t index) const;
	RefArrayOf<TElem>& operator=(const RefArrayOf<TElem>& toAssign);
	bool operator==(const RefArrayOf<TElem>& toCompare) const;
	bool operator!=(const RefArrayOf<TElem>& toCompare) const;


    // -----------------------------------------------------------------------
    //  Copy operations
    // -----------------------------------------------------------------------
    XMLSize_t copyFrom(const RefArrayOf<TElem>& srcArray);


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
	XMLSize_t length() const;
	TElem** rawData() const;


    // -----------------------------------------------------------------------
    //  Element management methods
    // -----------------------------------------------------------------------
    void deleteAt(const XMLSize_t index);
    void deleteAllElements();
    void resize(const XMLSize_t newSize);


private :
    // -----------------------------------------------------------------------
    //  Data members
    // -----------------------------------------------------------------------
	XMLSize_t       fSize;
	TElem**         fArray;
    MemoryManager*  fMemoryManager;
};


//
//  An enumerator for a reference array. It derives from the basic enumerator
//  class, so that value vectors can be generically enumerated.
//
template <class TElem> class RefArrayEnumerator : public XMLEnumerator<TElem>, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    RefArrayEnumerator
    (
                RefArrayOf<TElem>* const    toEnum
        , const bool                        adopt = false
    );
    virtual ~RefArrayEnumerator();


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
    RefArrayEnumerator(const RefArrayEnumerator<TElem>&);
    RefArrayEnumerator<TElem>& operator=(const RefArrayEnumerator<TElem>&);

    // -----------------------------------------------------------------------
    //  Data Members
    //
    //  fAdopted
    //      Indicates whether we have adopted the passed array. If so then
    //      we delete it when we are destroyed.
    //
    //  fCurIndex
    //      This is the current index into the array.
    //
    //  fToEnum
    //      The reference array being enumerated.
    // -----------------------------------------------------------------------
    bool                fAdopted;
    XMLSize_t           fCurIndex;
    RefArrayOf<TElem>*  fToEnum;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/RefArrayOf.c>
#endif

#endif
