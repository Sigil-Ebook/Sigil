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
 * $Id: DOMStringListImpl.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMSTRINGLISTIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMSTRINGLISTIMPL_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/dom/DOMStringList.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class CDOM_EXPORT DOMStringListImpl: public XMemory,
                                     public DOMStringList
{
protected:
    RefVectorOf<XMLCh>   *fList;

private:
    // Unused, and unimplemented constructors, operators, etc.
    DOMStringListImpl(const DOMStringListImpl & other);
    DOMStringListImpl & operator = (const DOMStringListImpl & other);

public:
    DOMStringListImpl(int nInitialSize, MemoryManager* manager);
    void add(const XMLCh* impl);

    virtual ~DOMStringListImpl();
    virtual const XMLCh* item(XMLSize_t index) const;
    virtual XMLSize_t    getLength() const;
    virtual bool         contains(const XMLCh* str) const;
    virtual void         release();
};

XERCES_CPP_NAMESPACE_END

#endif
