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
 * $Id: RefArrayVectorOf.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_REFARRAYVECTOROF_HPP)
#define XERCESC_INCLUDE_GUARD_REFARRAYVECTOROF_HPP

#include <xercesc/util/BaseRefVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/** 
 * Class with implementation for vectors of pointers to arrays  - implements from 
 * the Abstract class Vector
 */ 
template <class TElem> class RefArrayVectorOf : public BaseRefVectorOf<TElem> 
{
public :
    // -----------------------------------------------------------------------
    //  Constructor
    // -----------------------------------------------------------------------
    RefArrayVectorOf( const XMLSize_t      maxElems
                    , const bool           adoptElems = true
                    , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    // -----------------------------------------------------------------------
    //  Destructor
    // -----------------------------------------------------------------------
    ~RefArrayVectorOf();

    // -----------------------------------------------------------------------
    //  Element management
    // -----------------------------------------------------------------------
    void setElementAt(TElem* const toSet, const XMLSize_t setAt);
    void removeAllElements();
    void removeElementAt(const XMLSize_t removeAt);
    void removeLastElement();
    void cleanup();
private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefArrayVectorOf(const RefArrayVectorOf<TElem>&);
    RefArrayVectorOf<TElem>& operator=(const RefArrayVectorOf<TElem>&);
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/RefArrayVectorOf.c>
#endif

#endif
