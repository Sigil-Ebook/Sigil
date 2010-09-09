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
 * $Id: RefVectorOf.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_REFVECTOROF_HPP)
#define XERCESC_INCLUDE_GUARD_REFVECTOROF_HPP

#include <xercesc/util/BaseRefVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/** 
 * Class with implementation for vectors of References - implements from the 
 * Abstract class Vector
 */ 
template <class TElem> class RefVectorOf : public BaseRefVectorOf<TElem>
{
public :
    // -----------------------------------------------------------------------
    //  Constructor
    // -----------------------------------------------------------------------
    RefVectorOf(const XMLSize_t maxElems,
                const bool adoptElems = true,
                MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    // -----------------------------------------------------------------------
    //  Destructor
    // -----------------------------------------------------------------------
    ~RefVectorOf();

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefVectorOf(const RefVectorOf<TElem>&);
    RefVectorOf<TElem>& operator=(const RefVectorOf<TElem>&);
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/RefVectorOf.c>
#endif

#endif
