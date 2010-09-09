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
 * $Id: DOMImplementationListImpl.cpp 671894 2008-06-26 13:29:21Z borisk $
 */

#include "DOMImplementationListImpl.hpp"
#include <xercesc/dom/DOMImplementation.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMImplementationListImpl::DOMImplementationListImpl()
{
    fList=new RefVectorOf<DOMImplementation>(3, false);
}


DOMImplementationListImpl:: ~DOMImplementationListImpl()
{
    delete fList;
}


void DOMImplementationListImpl::add(DOMImplementation* impl) {
    fList->addElement(impl);
}

XMLSize_t DOMImplementationListImpl::getLength() const{
    return fList->size();
}


DOMImplementation *DOMImplementationListImpl::item(XMLSize_t index) const
{
    if(index<fList->size())
        return fList->elementAt(index);
    return 0;
}

void DOMImplementationListImpl::release() {
    delete this;
}

XERCES_CPP_NAMESPACE_END
