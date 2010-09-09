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
 * $Id: DOMXPathNSResolverImpl.hpp 657774 2008-05-19 09:59:33Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMXPATHNSRESOLVERIMPL_HPP)
#define XERCESC_INCLUDE_GUARD_DOMXPATHNSRESOLVERIMPL_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/dom/DOMXPathNSResolver.hpp>
#include <xercesc/util/KVStringPair.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMNode;

class CDOM_EXPORT DOMXPathNSResolverImpl : public XMemory,
                                           public DOMXPathNSResolver
{
public:
    DOMXPathNSResolverImpl(const DOMNode* nodeResolver = 0, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~DOMXPathNSResolverImpl();

    virtual const XMLCh*          lookupNamespaceURI(const XMLCh* prefix) const;
    virtual const XMLCh*          lookupPrefix(const XMLCh* URI) const;
    virtual void                  addNamespaceBinding(const XMLCh* prefix, const XMLCh* uri);

    virtual void                  release();

protected:
    RefHashTableOf<KVStringPair>* fNamespaceBindings;
    const DOMNode*                fResolverNode;
    MemoryManager*                fManager;
};

XERCES_CPP_NAMESPACE_END

#endif


