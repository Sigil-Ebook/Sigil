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
 * $Id: StDOMNode.hpp 570480 2007-08-28 16:36:34Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_STDOMNODE_HPP)
#define XERCESC_INCLUDE_GUARD_STDOMNODE_HPP

#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMElement.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/* This class is a smart pointer implementation over DOMNode interface and
** classes derived from it. It takes care of reference counting automatically.
** Reference counting is optional so use of this class is experimental.
*/
template <class T> class StDOMNode {
	T* m_node;

	static inline void INCREFCOUNT(T *x) { if (x != (T*)0) x->incRefCount(); }
	static inline void DECREFCOUNT(T *x) { if (x != (T*)0) x->decRefCount(); }

public:
	inline StDOMNode(T* node = (T*)0) : m_node(node) { INCREFCOUNT(m_node); }
	inline StDOMNode(const StDOMNode& stNode) : m_node(stNode.m_node) { INCREFCOUNT(m_node); }
	inline ~StDOMNode() { DECREFCOUNT(m_node); }

	inline T* operator= (T *node)
	{
		if (m_node != node) {
			DECREFCOUNT(m_node);
			m_node = node;
			INCREFCOUNT(m_node);
		}
		return (m_node);
	}

	inline bool operator!= (T* node) const { return (m_node != node); }
	inline bool operator== (T* node) const { return (m_node == node); }

	inline T& operator* () { return (*m_node); }
	inline const T& operator* () const { return (*m_node); }
	inline T* operator-> () const { return (m_node); }
	inline operator T*() const { return (m_node); }
	inline void ClearNode() { operator=((T*)(0)); }
};

#if defined(XML_DOMREFCOUNT_EXPERIMENTAL)
    typedef StDOMNode<DOMNode> DOMNodeSPtr;
#else
    typedef DOMNode* DOMNodeSPtr;
#endif

/* StDOMNode is a smart pointer implementation over DOMNode interface and
** classes derived from it. It takes care of reference counting automatically.
** Reference counting is optional so use of this class is experimental.
*/
#if defined(XML_DOMREFCOUNT_EXPERIMENTAL)
    typedef StDOMNode<DOMAttr> DOMAttrSPtr;
#else
    typedef DOMAttr* DOMAttrSPtr;
#endif

/* StDOMNode is a smart pointer implementation over DOMNode interface and
** classes derived from it. It takes care of reference counting automatically.
** Reference counting is optional so use of this class is experimental.
*/
#if defined(XML_DOMREFCOUNT_EXPERIMENTAL)
    typedef StDOMNode<DOMElement> DOMElementSPtr;
#else
    typedef DOMElement* DOMElementSPtr;
#endif

XERCES_CPP_NAMESPACE_END

#endif

