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
 * $Id: ValueStackOf.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_VALUESTACKOF_HPP)
#define XERCESC_INCLUDE_GUARD_VALUESTACKOF_HPP

#include <xercesc/util/EmptyStackException.hpp>
#include <xercesc/util/ValueVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  Forward declare the enumerator so he can be our friend. Can you say
//  friend? Sure...
//
template <class TElem> class ValueStackEnumerator;


template <class TElem> class ValueStackOf : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ValueStackOf
    (
          const XMLSize_t fInitCapacity
          , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
          , const bool toCallDestructor = false
    );
    ~ValueStackOf();


    // -----------------------------------------------------------------------
    //  Element management methods
    // -----------------------------------------------------------------------
    void push(const TElem& toPush);
    const TElem& peek() const;
    TElem pop();
    void removeAllElements();


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool empty();
    XMLSize_t curCapacity();
    XMLSize_t size();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------    
    ValueStackOf(const ValueStackOf<TElem>&);
    ValueStackOf<TElem>& operator=(const ValueStackOf<TElem>&);

    // -----------------------------------------------------------------------
    //  Declare our friends
    // -----------------------------------------------------------------------
    friend class ValueStackEnumerator<TElem>;


    // -----------------------------------------------------------------------
    //  Data Members
    //
    //  fVector
    //      The vector that is used as the backing data structure for the
    //      stack.
    // -----------------------------------------------------------------------
    ValueVectorOf<TElem>    fVector;
};



//
//  An enumerator for a value stack. It derives from the basic enumerator
//  class, so that value stacks can be generically enumerated.
//
template <class TElem> class ValueStackEnumerator : public XMLEnumerator<TElem>, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ValueStackEnumerator
    (
                ValueStackOf<TElem>* const  toEnum
        , const bool                        adopt = false
    );
    virtual ~ValueStackEnumerator();


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
    ValueStackEnumerator(const ValueStackEnumerator<TElem>&);
    ValueStackEnumerator<TElem>& operator=(const ValueStackEnumerator<TElem>&);

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
    bool                    fAdopted;
    XMLSize_t               fCurIndex;
    ValueVectorOf<TElem>*   fVector;
    ValueStackOf<TElem>*    fToEnum;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/ValueStackOf.c>
#endif

#endif
