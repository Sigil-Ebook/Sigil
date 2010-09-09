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
 * $Id: XUtil.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/XUtil.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/IllegalArgumentException.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMNode.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// Finds and returns the first child element node.
DOMElement* XUtil::getFirstChildElement(const DOMNode* const parent)
{
    // search for node
    DOMNode* child = parent->getFirstChild();

    while (child != 0)
	{
        if (child->getNodeType() == DOMNode::ELEMENT_NODE)
            return (DOMElement*)child;

        child = child->getNextSibling();
    }

    // not found
    return 0;
}

// Finds and returns the first child node with the given name.
DOMElement* XUtil::getFirstChildElementNS(const DOMNode* const parent
                                          , const XMLCh** const elemNames
                                          , const XMLCh* const uriStr
                                          , unsigned int        length)
{
    // search for node
    DOMNode* child = parent->getFirstChild();
    while (child != 0)
	{
        if (child->getNodeType() == DOMNode::ELEMENT_NODE)
		{
            for (unsigned int i = 0; i < length; i++)
			{
                if (XMLString::equals(child->getNamespaceURI(), uriStr) &&
                    XMLString::equals(child->getLocalName(), elemNames[i]))
                    return (DOMElement*)child;
			}
		}
        child = child->getNextSibling();
    }

    // not found
    return 0;
}

// Finds and returns the last child element node.
DOMElement* XUtil::getNextSiblingElement(const DOMNode* const node)
{
    // search for node
    DOMNode* sibling = node->getNextSibling();

    while (sibling != 0)
	{
        if (sibling->getNodeType() == DOMNode::ELEMENT_NODE)
            return (DOMElement*)sibling;

        sibling = sibling->getNextSibling();
    }

    // not found
    return 0;
}

// Finds and returns the next sibling element node with the give name.
DOMElement* XUtil::getNextSiblingElementNS(const DOMNode* const node
                                           , const XMLCh** const elemNames
                                           , const XMLCh* const uriStr
									       , unsigned int        length)
{
    // search for node
    DOMNode* sibling = node->getNextSibling();
    while (sibling != 0)
	{
        if (sibling->getNodeType() == DOMNode::ELEMENT_NODE)
		{
            for (unsigned int i = 0; i < length; i++)
			{
                if (XMLString::equals(sibling->getNamespaceURI(), uriStr) &&
                    XMLString::equals(sibling->getLocalName(), elemNames[i]))
                    return (DOMElement*)sibling;
			}
		}
        sibling = sibling->getNextSibling();
    }

    // not found
    return 0;
}

XERCES_CPP_NAMESPACE_END
