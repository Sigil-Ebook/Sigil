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
 * $Id: RefStackOf.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_REFSTACKOF_HPP)
#define XERCESC_INCLUDE_GUARD_REFSTACKOF_HPP

#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/util/EmptyStackException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  Forward declare the enumerator so he can be our friend. Can you say
//  friend? Sure...
//
template <class TElem> class RefStackEnumerator;


template <class TElem> class RefStackOf : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    RefStackOf(const XMLSize_t initElems,
               const bool adoptElems = true,
               MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~RefStackOf();


    // -----------------------------------------------------------------------
    //  Element management methods
    // -----------------------------------------------------------------------
    const TElem* elementAt(const XMLSize_t index) const;
    TElem* popAt(const XMLSize_t index);
    void push(TElem* const toPush);
    const TElem* peek() const;
    TElem* pop();
    void removeAllElements();


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool empty();
    XMLSize_t curCapacity();
    XMLSize_t size();


private :
    // -----------------------------------------------------------------------
    //  Declare our friends
    // -----------------------------------------------------------------------
    friend class RefStackEnumerator<TElem>;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefStackOf(const RefStackOf<TElem>&);
    RefStackOf<TElem>& operator=(const RefStackOf<TElem>&);

    // -----------------------------------------------------------------------
    //  Data Members
    //
    //  fVector
    //      The vector that is used as the backing data structure for the
    //      stack.
    // -----------------------------------------------------------------------
    RefVectorOf<TElem>  fVector;
};



//
//  An enumerator for a value stack. It derives from the basic enumerator
//  class, so that value stacks can be generically enumerated.
//
template <class TElem> class RefStackEnumerator : public XMLEnumerator<TElem>, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    RefStackEnumerator
    (
                RefStackOf<TElem>* const    toEnum
        , const bool                        adopt = false
    );
    virtual ~RefStackEnumerator();


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
    RefStackEnumerator(const RefStackEnumerator<TElem>&);
    RefStackEnumerator<TElem>& operator=(const RefStackEnumerator<TElem>&);
    // -----------------------------------------------------------------------
    //  Data Members
    //
    //  fAdopted
    //      Indicates whether we have adopted the passed stack. If so then
    //      we delete the stack when we are destroyed.
    //
    //  fCurIndex
    //      This is the current index into the vector inside the stack being
    //      enumerated.
    //
    //  fToEnum
    //      The stack that is being enumerated. This is just kept for
    //      adoption purposes, since we really are enumerating the vector
    //      inside of it.
    // -----------------------------------------------------------------------
    bool                fAdopted;
    XMLSize_t           fCurIndex;
    RefVectorOf<TElem>* fVector;
    RefStackOf<TElem>*  fToEnum;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/RefStackOf.c>
#endif

#endif
