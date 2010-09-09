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
 * $Id: DOMNodeListImpl.cpp 671894 2008-06-26 13:29:21Z borisk $
 */


#include <xercesc/util/XercesDefs.hpp>
#include "DOMNodeListImpl.hpp"
#include "DOMCasts.hpp"

XERCES_CPP_NAMESPACE_BEGIN


// revisit
//   this implementation is too stupid - needs a cache of some kind.
//

DOMNodeListImpl::DOMNodeListImpl(DOMParentNode *node)
:   fNode(node)
{
}


DOMNodeListImpl:: ~DOMNodeListImpl()
{
}



XMLSize_t DOMNodeListImpl::getLength() const{
    XMLSize_t count = 0;
    if (fNode) {
        DOMNode *node = fNode->fFirstChild;
        while(node != 0){
            ++count;
            node = castToChildImpl(node)->nextSibling;
        }
    }

    return count;
}



DOMNode *DOMNodeListImpl::item(XMLSize_t index) const{
    if (fNode) {
        DOMNode *node = fNode->fFirstChild;
        for(XMLSize_t i=0; i<index && node!=0; ++i)
            node = castToChildImpl(node)->nextSibling;
        return node;
    }
    return 0;
}


XERCES_CPP_NAMESPACE_END
