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
 * $Id: DOMNodeVector.hpp 676796 2008-07-15 05:04:13Z dbertoni $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMNODEVECTOR_HPP)
#define XERCESC_INCLUDE_GUARD_DOMNODEVECTOR_HPP

//
//  This file is part of the internal implementation of the C++ XML DOM.
//  It should NOT be included or used directly by application programs.
//
//  Applications should include the file <xercesc/dom/DOM.hpp> for the entire
//  DOM API, or xercesc/dom/DOM*.hpp for individual DOM classes, where the class
//  name is substituded for the *.
//

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMNode;
class DOMDocument;


class  DOMNodeVector {
private:
    DOMNode        **data;
    XMLSize_t       allocatedSize;
    XMLSize_t       nextFreeSlot;
    void           init(DOMDocument *doc, XMLSize_t size);
    void           checkSpace();

    // unimplemented
    DOMNodeVector ( const DOMNodeVector& toCopy);
    DOMNodeVector& operator= (const DOMNodeVector& other);

public:
    DOMNodeVector(DOMDocument *doc);
    DOMNodeVector(DOMDocument *doc, XMLSize_t size);
    ~DOMNodeVector();

    XMLSize_t      size();
    DOMNode*       elementAt(XMLSize_t index);
    DOMNode*       lastElement();
    void           addElement(DOMNode *);
    void           insertElementAt(DOMNode *, XMLSize_t index);
    void           setElementAt(DOMNode *val, XMLSize_t index);
    void           removeElementAt(XMLSize_t index);
    void           reset();
};

inline DOMNode *DOMNodeVector::elementAt(XMLSize_t index) {
    if (index >= nextFreeSlot)
        return 0;
	return data[index];
}

inline DOMNode *DOMNodeVector::lastElement() {
	if (nextFreeSlot == 0)
		return 0;
	return data[nextFreeSlot-1];
}

inline XMLSize_t DOMNodeVector::size() {
	return nextFreeSlot;
}

XERCES_CPP_NAMESPACE_END

#endif
