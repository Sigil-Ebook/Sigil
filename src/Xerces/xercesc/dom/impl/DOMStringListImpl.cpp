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
 * $Id: DOMStringListImpl.cpp 671894 2008-06-26 13:29:21Z borisk $
 */

#include "DOMStringListImpl.hpp"
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMStringListImpl::DOMStringListImpl(int nInitialSize, MemoryManager* manager)
{
    fList=new (manager) RefVectorOf<XMLCh>(nInitialSize, false, manager);
}

DOMStringListImpl::~DOMStringListImpl()
{
    delete fList;
}


void DOMStringListImpl::add(const XMLCh* str) {
    fList->addElement((XMLCh*)str);
}

XMLSize_t DOMStringListImpl::getLength() const{
    return fList->size();
}


const XMLCh* DOMStringListImpl::item(XMLSize_t index) const{
    if(index<fList->size())
        return fList->elementAt(index);
    return 0;
}

bool DOMStringListImpl::contains(const XMLCh* str) const{
    for(XMLSize_t i=0;i<fList->size();i++)
        if(XMLString::equals(fList->elementAt(i), str))
            return true;
    return false;
}

void DOMStringListImpl::release() {
    delete this;
}

XERCES_CPP_NAMESPACE_END
