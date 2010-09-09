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
 * $Id: DOMDeepNodeListImpl.cpp 678381 2008-07-21 10:15:01Z borisk $
 */

#include "DOMDeepNodeListImpl.hpp"
#include "DOMElementImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMCasts.hpp"
#include "DOMNodeImpl.hpp"
#include <xercesc/util/XMLUniDefs.hpp>
#include <limits.h>

XERCES_CPP_NAMESPACE_BEGIN


static const XMLCh kAstr[] = {chAsterisk, chNull};

DOMDeepNodeListImpl::DOMDeepNodeListImpl(const DOMNode *rootNode,
                                       const XMLCh *tagName)
    : fRootNode(rootNode)
    , fChanges(0)
    , fCurrentNode(0)
    , fCurrentIndexPlus1(0)
    , fNamespaceURI(0)
    , fMatchAllURI(false)
    , fMatchURIandTagname(false)
{
    fTagName = ((DOMDocumentImpl *)(castToNodeImpl(rootNode)->getOwnerDocument()))->getPooledString(tagName);
    fMatchAll = XMLString::equals(fTagName, kAstr);
}


//DOM Level 2
DOMDeepNodeListImpl::DOMDeepNodeListImpl(const DOMNode *rootNode,
                                       const XMLCh *namespaceURI,
                                       const XMLCh *localName)
    : fRootNode(rootNode)
    , fChanges(0)
    , fCurrentNode(0)
    , fCurrentIndexPlus1(0)
    , fMatchAllURI(false)
    , fMatchURIandTagname(true)
{
    DOMDocumentImpl* doc = (DOMDocumentImpl *)castToNodeImpl(rootNode)->getOwnerDocument();

    fTagName = doc->getPooledString(localName);
    fMatchAll = XMLString::equals(fTagName, kAstr);
    fMatchAllURI = XMLString::equals(namespaceURI, kAstr);
    fNamespaceURI = doc->getPooledString(namespaceURI);
}


DOMDeepNodeListImpl::~DOMDeepNodeListImpl()
{
}

XMLSize_t DOMDeepNodeListImpl::getLength() const
{
    // Reset cache to beginning of list
    item(0);

    // Preload all matching elements. (Stops when we run out of subtree!)
    item(INT_MAX);
    return fCurrentIndexPlus1;
}


DOMNode *DOMDeepNodeListImpl::item(XMLSize_t index) const
{
    return ((DOMDeepNodeListImpl*)this)->cacheItem(index);
}

// Start from the first child and count forward, 0-based. index>length-1
// should return 0.
//
// Attempts to do only work actually requested, cache work already
// done, and to flush that cache when the tree has changed.
//
// LIMITATION: ????? Unable to tell relevant tree-changes from
// irrelevant ones.  Doing so in a really useful manner would seem
// to involve a tree-walk in its own right, or maintaining our data
// in a parallel tree.
DOMNode *DOMDeepNodeListImpl::cacheItem(XMLSize_t index)
{
    XMLSize_t currentIndexPlus1 = fCurrentIndexPlus1;
    DOMNode *currentNode = fCurrentNode;

    if (castToParentImpl(fRootNode)->changes() != fChanges)
    {
        // Tree changed. Do it all from scratch!
        currentIndexPlus1 = 0;
        currentNode = (DOMNode *)fRootNode;
        fChanges = castToParentImpl(fRootNode)->changes();
    }
    else if (currentIndexPlus1 > index+1)
    {
        // Interested in something before cached node.  Do it all from scratch!
        currentIndexPlus1 = 0;
        currentNode = (DOMNode *)fRootNode;
    }
    else if (index+1 == currentIndexPlus1)
    {
        // What luck!  User is interested in cached node.
        return currentNode;
    }

    DOMNode *nextNode = 0;

// revisit - ???? How efficient is this loop? ????

    // Start at the place in the tree at which we're
    // currently pointing and count off nodes until we
    // reach the node of interest or the end of the tree.
    while (currentIndexPlus1 < index+1 && currentNode != 0)
    {
        nextNode = nextMatchingElementAfter(currentNode);
        if (nextNode == 0)
            break;
        currentNode = nextNode;
        currentIndexPlus1++;
    }

    fCurrentNode = currentNode;
    fCurrentIndexPlus1 = currentIndexPlus1;

    // If we found a node at the requested index, make that the current node
    if (nextNode != 0)
    {
        return currentNode;
    }

    // If we didn't find a node at the requested index, return 0
    return 0;
}



/* Iterative tree-walker. When you have a Parent link, there's often no
need to resort to recursion. NOTE THAT only Element nodes are matched
since we're specifically supporting getElementsByTagName().
*/
DOMNode *DOMDeepNodeListImpl::nextMatchingElementAfter(DOMNode *current)
{
    DOMNode *next;
    while (current != 0)
    {
        // Look down to first child.
        if (current->hasChildNodes())
        {
            current = current->getFirstChild();
        }
        // Look right to sibling (but not from root!)
        else
        {
            if (current != fRootNode && 0 != (next = current->getNextSibling()))
            {
                current = next;
            }
            // Look up and right (but not past root!)
            else
            {
                next = 0;
                for (;
                     current != fRootNode; // Stop on return to starting point
                     current = current->getParentNode())
                {
                    next = current->getNextSibling();
                    if (next != 0)
                        break;
                }
                current = next;
            }
        }

        // Have we found an Element with the right tagName?
        // ("*" matches anything.)
        if (current != 0 && current != fRootNode &&
            current->getNodeType() == DOMNode::ELEMENT_NODE) {
            DOMElement *currElement = (DOMElement *)current;

            if (!fMatchURIandTagname) {        //DOM Level 1
                if (fMatchAll ||
                    XMLString::equals(currElement->getTagName(), fTagName))
                    return current;
            } else {        //DOM Level 2
                if (!fMatchAllURI &&
                    !XMLString::equals(current->getNamespaceURI(), fNamespaceURI))
                    continue;

                if (fMatchAll ||
                    XMLString::equals(current->getLocalName(), fTagName))
                    return current;
            }
        }

        // Otherwise continue walking the tree
    }
    // Fell out of tree-walk; no more instances found
    return 0;
}

XERCES_CPP_NAMESPACE_END
