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
 * $Id: RefVectorOf.c 676911 2008-07-15 13:27:32Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/RefVectorOf.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  RefVectorOf: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TElem>
RefVectorOf<TElem>::RefVectorOf(const XMLSize_t maxElems,
                                const bool adoptElems,
                                MemoryManager* const manager)
    : BaseRefVectorOf<TElem>(maxElems, adoptElems, manager)
{
}

template <class TElem> RefVectorOf<TElem>::~RefVectorOf()
{
    if (this->fAdoptedElems)
    {
        for (XMLSize_t index = 0; index < this->fCurCount; index++)
            delete this->fElemList[index];
    }
    this->fMemoryManager->deallocate(this->fElemList);//delete [] this->fElemList;
}


XERCES_CPP_NAMESPACE_END
