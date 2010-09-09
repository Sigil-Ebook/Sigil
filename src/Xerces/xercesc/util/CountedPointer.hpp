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
 * $Id: CountedPointer.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_COUNTEDPOINTERTO_HPP)
#define XERCESC_INCLUDE_GUARD_COUNTEDPOINTERTO_HPP

#include <xercesc/util/NullPointerException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

template <class T> class CountedPointerTo : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    CountedPointerTo(const CountedPointerTo<T>& toCopy);
    CountedPointerTo(T* p = 0);
    ~CountedPointerTo();


    // -----------------------------------------------------------------------
    //  Operators
    // -----------------------------------------------------------------------
    CountedPointerTo<T>& operator=(const CountedPointerTo<T>& other);
    operator T*();
    const T* operator->() const;
    T* operator->();
    const T& operator*() const;
    T& operator*();


private:
    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fPtr
    //      The pointer that we are counting. The T type must implement the
    //      addRef() and removeRef() APIs but it doesn't have to derive from
    //      any particular type.
    // -----------------------------------------------------------------------
    T*  fPtr;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/CountedPointer.c>
#endif

#endif
