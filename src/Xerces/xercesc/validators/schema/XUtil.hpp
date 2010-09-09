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
 * $Id: XUtil.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XUTIL_HPP)
#define XERCESC_INCLUDE_GUARD_XUTIL_HPP

#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class DOMNode;
class DOMElement;

/**
 * Some useful utility methods.
 */
class VALIDATORS_EXPORT XUtil
{
public:

    // Finds and returns the first child element node.
    static DOMElement* getFirstChildElement(const DOMNode* const parent);

    // Finds and returns the first child node with the given qualifiedname.
    static DOMElement* getFirstChildElementNS(const DOMNode* const parent
                                              , const XMLCh** const elemNames
                                              , const XMLCh* const uriStr
                                              , unsigned int       length);

    // Finds and returns the next sibling element node.
    static DOMElement* getNextSiblingElement(const DOMNode* const node);

    static DOMElement* getNextSiblingElementNS(const DOMNode* const node
                                               , const XMLCh** const elemNames
                                               , const XMLCh* const uriStr
                                               , unsigned int        length);

private:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

	// This class cannot be instantiated.
     XUtil() {};
	~XUtil() {};
};

XERCES_CPP_NAMESPACE_END

#endif
