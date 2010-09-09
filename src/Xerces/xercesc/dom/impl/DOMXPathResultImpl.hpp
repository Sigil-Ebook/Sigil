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
 * $Id: DOMXPathResultImpl.hpp 671894 2008-06-26 13:29:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMXPATHRESULTIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMXPATHRESULTIMPL_HPP

#include <xercesc/util/XMemory.hpp>
#include <xercesc/dom/DOMXPathResult.hpp>
#include <xercesc/util/RefVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class CDOM_EXPORT DOMXPathResultImpl :  public XMemory,
                                        public DOMXPathResult
{
public:
    DOMXPathResultImpl(ResultType type, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~DOMXPathResultImpl();

    virtual ResultType getResultType() const;
    virtual const DOMTypeInfo *getTypeInfo() const;
    virtual bool isNode() const;
    virtual bool getBooleanValue() const;
    virtual int getIntegerValue() const;
    virtual double getNumberValue() const;
    virtual const XMLCh* getStringValue() const;
    virtual DOMNode* getNodeValue() const;
    virtual bool iterateNext();
    virtual bool getInvalidIteratorState() const;
    virtual bool snapshotItem(XMLSize_t);
    virtual XMLSize_t getSnapshotLength() const;

    virtual void release();

public:
    void reset(ResultType type);
    void addResult(DOMNode* node);

protected:
    ResultType              fType;
    MemoryManager* const    fMemoryManager;
    RefVectorOf<DOMNode>*   fSnapshot;
    XMLSize_t               fIndex;
};

XERCES_CPP_NAMESPACE_END

#endif
