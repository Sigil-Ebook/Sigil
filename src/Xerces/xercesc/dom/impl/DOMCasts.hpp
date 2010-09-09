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
 * $Id: DOMCasts.hpp 673975 2008-07-04 09:23:56Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMCASTS_HPP)
#define XERCESC_INCLUDE_GUARD_DOMCASTS_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

//
//  Define inline casting functions to convert from
//    (DOMNode *) to DOMParentNode or DOMChildNode *.
//
//  This requires knowledge of the structure of the fields of
//   for all node types.  There are three categories -
//
//  Nodetypes that can have children and can be a child themselves.
//    e.g.  Elements
//
//       Object
//           DOMNodeImpl     fNode;
//           DOMParentNode   fParent;
//           DOMChildNode    fChild;
//             ...            // other fields, depending on node type.
//
//  Nodetypes that can not have children, e.g. TEXT
//
//       Object
//           DOMNodeImpl     fNode;
//           DOMChildNode    fChild;
//              ...            // other fields, depending on node type
//
//  Nodetypes that can not be a child of other nodes, but that can
//  have children (are a parent)  e.g. ATTR
//       Object
//           DOMNodeImpl     fNode;
//           DOMParentNode   fParent
//               ...           // other fields, depending on node type
//
//   The casting functions make these assumptions:
//      1.  The cast is possible.  Using code will not attempt to
//          cast to something that does not exist, such as the child
//          part of an ATTR
//
//      2.  The nodes belong to this implementation.
//
//    Some of the casts use the LEAFNODE flag in the common fNode part to
//    determine whether an fParent field exists, and thus the
//    position of the fChild part within the node.
//
//  These functions also cast off const.  It was either do that, or make
//  a second overloaded set that took and returned const arguements.
//

//
//	Note that using offsetof, or taking the offset of an object member at
//	a 0 address, is now undefined in C++. And gcc now warns about this behavior.
//	This is because doing do so is unreliable for some types of objects.
//		See: http://gcc.gnu.org/ml/gcc/2004-06/msg00227.html
//		   : http://gcc.gnu.org/ml/gcc-bugs/2000-03/msg00805.html
//  The casting code below works around gcc's warnings by using a dummy
//	pointer, which the compiler cannot tell is null. The defeats the warning,
//	but also masks the potential problem.
//	The gcc option -Wno-invalid-offsetof may also be used to turn off this warning.
//

#include "DOMElementImpl.hpp"
#include "DOMTextImpl.hpp"

XERCES_CPP_NAMESPACE_BEGIN


static inline DOMNodeImpl *castToNodeImpl(const DOMNode *p)
{
    DOMElementImpl *pE = (DOMElementImpl *)p;
    return &(pE->fNode);
}


static inline DOMParentNode *castToParentImpl(const DOMNode *p) {
    DOMElementImpl *pE = (DOMElementImpl *)p;
    return &(pE->fParent);
}


static inline DOMChildNode *castToChildImpl(const DOMNode *p) {
    DOMElementImpl *pE = (DOMElementImpl *)p;
    if (pE->fNode.isLeafNode())  {
        DOMTextImpl *pT = (DOMTextImpl *)p;
        return &(pT->fChild);
    }
    return &(pE->fChild);
}


static inline DOMNode *castToNode(const DOMParentNode *p ) {
	DOMElementImpl* dummy = 0;
    XMLSize_t parentOffset = (char *)&(dummy->fParent) - (char *)dummy;
    char *retPtr = (char *)p - parentOffset;
    return (DOMNode *)retPtr;
}

static inline DOMNode *castToNode(const DOMNodeImpl *p) {
	DOMElementImpl* dummy = 0;
    XMLSize_t nodeImplOffset = (char *)&(dummy->fNode) - (char *)dummy;
    char *retPtr = (char *)p - nodeImplOffset;
    return (DOMNode *)retPtr;
}


static inline DOMNodeImpl *castToNodeImpl(const DOMParentNode *p)
{
	DOMElementImpl* dummy = 0;
    XMLSize_t nodeImplOffset = (char *)&(dummy->fNode) - (char *)dummy;
    XMLSize_t parentOffset = (char *)&(dummy->fParent) - (char *)dummy;
    char *retPtr = (char *)p - parentOffset + nodeImplOffset;
    return (DOMNodeImpl *)retPtr;
}

XERCES_CPP_NAMESPACE_END

#endif
