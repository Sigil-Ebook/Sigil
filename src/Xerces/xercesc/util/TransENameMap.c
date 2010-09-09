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


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/util/TransENameMap.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  ENameMapFor: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TType>
ENameMapFor<TType>::ENameMapFor(const XMLCh* const encodingName) :

    ENameMap(encodingName)
{
}

template <class TType> ENameMapFor<TType>::~ENameMapFor()
{
}


// ---------------------------------------------------------------------------
//  ENameMapFor: Implementation of virtual factory method
// ---------------------------------------------------------------------------
template <class TType> XMLTranscoder*
ENameMapFor<TType>::makeNew(const XMLSize_t      blockSize,
                            MemoryManager* const manager) const
{
    return new (manager) TType(getKey(), blockSize, manager);
}




// ---------------------------------------------------------------------------
//  ENameMapFor: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TType> EEndianNameMapFor<TType>::EEndianNameMapFor(const XMLCh* const encodingName, const bool swapped) :

    ENameMap(encodingName)
    , fSwapped(swapped)
{
}

template <class TType> EEndianNameMapFor<TType>::~EEndianNameMapFor()
{
}


// ---------------------------------------------------------------------------
//  ENameMapFor: Implementation of virtual factory method
// ---------------------------------------------------------------------------
template <class TType> XMLTranscoder*
EEndianNameMapFor<TType>::makeNew(const XMLSize_t      blockSize,
                                  MemoryManager* const manager) const
{
    return new (manager) TType(getKey(), blockSize, fSwapped, manager);
}

XERCES_CPP_NAMESPACE_END
