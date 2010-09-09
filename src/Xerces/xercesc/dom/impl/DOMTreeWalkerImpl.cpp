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
 * $Id: DOMTreeWalkerImpl.cpp 671894 2008-06-26 13:29:21Z borisk $
 */

#include "DOMTreeWalkerImpl.hpp"
#include "DOMDocumentImpl.hpp"

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/** constructor */
DOMTreeWalkerImpl::DOMTreeWalkerImpl (
                                DOMNode* root,
                                DOMNodeFilter::ShowType whatToShow,
                                DOMNodeFilter* nodeFilter,
                                bool expandEntityRef)
:   fWhatToShow(whatToShow),
    fNodeFilter(nodeFilter),
    fCurrentNode(root),
    fRoot(root),
    fExpandEntityReferences(expandEntityRef)
{
}


DOMTreeWalkerImpl::DOMTreeWalkerImpl (const DOMTreeWalkerImpl& twi)
:   DOMTreeWalker(twi),
    fWhatToShow(twi.fWhatToShow),
    fNodeFilter(twi.fNodeFilter),
    fCurrentNode(twi.fCurrentNode),
    fRoot(twi.fRoot),
    fExpandEntityReferences(twi.fExpandEntityReferences)
{
}


DOMTreeWalkerImpl& DOMTreeWalkerImpl::operator= (const DOMTreeWalkerImpl& twi) {
    if (this != &twi)
    {
        fCurrentNode            = twi.fCurrentNode;
        fRoot                   = twi.fRoot;
        fWhatToShow             = twi.fWhatToShow;
        fNodeFilter             = twi.fNodeFilter;
		fExpandEntityReferences = twi.fExpandEntityReferences;
    }

    return *this;
}



/** Return the root node */
DOMNode* DOMTreeWalkerImpl::getRoot () {
    return fRoot;
}


/** Return the whatToShow value */
DOMNodeFilter::ShowType DOMTreeWalkerImpl::getWhatToShow () {
    return fWhatToShow;
}


/** Return the NodeFilter */
DOMNodeFilter* DOMTreeWalkerImpl::getFilter () {
    return fNodeFilter;
}

/** Get the expandEntity reference flag. */
bool DOMTreeWalkerImpl::getExpandEntityReferences() {
    return fExpandEntityReferences;
}



/** Return the current Node. */
DOMNode* DOMTreeWalkerImpl::getCurrentNode () {

    return fCurrentNode;
}


/** Return the current Node. */
void DOMTreeWalkerImpl::setCurrentNode (DOMNode* node) {

    if (!node)
        throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, GetDOMTreeWalkerMemoryManager);

    fCurrentNode = node;
}


/** Return the parent Node from the current node,
 *  after applying filter, whatToshow.
 *  If result is not null, set the current Node.
 */
DOMNode* DOMTreeWalkerImpl::parentNode () {

    if (!fCurrentNode) return 0;

    DOMNode* node = getParentNode(fCurrentNode);
    if (node != 0) {
        fCurrentNode = node;
    }
    return node;

}


/** Return the first child Node from the current node,
 *  after applying filter, whatToshow.
 *  If result is not null, set the current Node.
 */
DOMNode* DOMTreeWalkerImpl::firstChild () {

    if (!fCurrentNode) return 0;

    if(!fExpandEntityReferences && fCurrentNode->getNodeType()==DOMNode::ENTITY_REFERENCE_NODE)
        return 0;

    DOMNode* node = getFirstChild(fCurrentNode);

    if (node != 0) {
        fCurrentNode = node;
    }
    return node;
}


/** Return the last child Node from the current node,
 *  after applying filter, whatToshow.
 *  If result is not null, set the current Node.
 */
DOMNode* DOMTreeWalkerImpl::lastChild () {

    if (!fCurrentNode) return 0;

    if(!fExpandEntityReferences && fCurrentNode->getNodeType()==DOMNode::ENTITY_REFERENCE_NODE)
        return 0;

    DOMNode* node = getLastChild(fCurrentNode);
    if (node != 0) {
        fCurrentNode = node;
    }
    return node;
}


/** Return the previous sibling Node from the current node,
 *  after applying filter, whatToshow.
 *  If result is not null, set the current Node.
 */

DOMNode* DOMTreeWalkerImpl::previousSibling () {

    if (!fCurrentNode) return 0;

    DOMNode* node = getPreviousSibling(fCurrentNode);
    if (node != 0) {
        fCurrentNode = node;
    }
    return node;
}


/** Return the next sibling Node from the current node,
 *  after applying filter, whatToshow.
 *  If result is not null, set the current Node.
 */

DOMNode* DOMTreeWalkerImpl::nextSibling () {

    if (!fCurrentNode) return 0;

    DOMNode* node = getNextSibling(fCurrentNode);
    if (node != 0) {
        fCurrentNode = node;
    }
    return node;
}


/** Return the previous Node from the current node,
 *  after applying filter, whatToshow.
 *  If result is not null, set the current Node.
 */

DOMNode* DOMTreeWalkerImpl::previousNode () {

    if (!fCurrentNode) return 0;

    // get sibling
    DOMNode* node = getPreviousSibling(fCurrentNode);
    if (node == 0) {
        node = getParentNode(fCurrentNode);
        if ( node != 0) {
            fCurrentNode = node;
        }
        return node;
    }
    else {

        // get the lastChild of result.
        DOMNode* lastChild  = getLastChild(node);

        // if there is a lastChild which passes filters return it.
        if (lastChild != 0) {
            fCurrentNode = lastChild;
        }
        else {
            fCurrentNode = node;
        }
        return fCurrentNode;
    }
}


/** Return the next Node from the current node,
 *  after applying filter, whatToshow.
 *  If result is not null, set the current Node.
 */

DOMNode* DOMTreeWalkerImpl::nextNode () {

    if (!fCurrentNode) return 0;

    DOMNode* node = getFirstChild(fCurrentNode);

    if (node != 0) {
        fCurrentNode = node;
        return node;
    }
    else {

        node = getNextSibling(fCurrentNode);

        if (node != 0) {
            fCurrentNode = node;
            return node;
        }
        else {

            // return parent's 1st sibling.
            DOMNode* parent = getParentNode(fCurrentNode);
            while ( parent != 0) {
                node = getNextSibling(parent);
                if (node != 0) {
                    fCurrentNode = node;
                    return node;
                } else {
                    parent = getParentNode(parent);
                }
            }
            return node;
        }
    }
}


/** Internal function.
 *  Return the parent Node, from the input node
 *  after applying filter, whatToshow.
 *  The current node is not consulted or set.
 */

DOMNode* DOMTreeWalkerImpl::getParentNode (DOMNode* node) {

    if (!node || node == fRoot) return 0;

    DOMNode* newNode = node->getParentNode();
    if (!newNode)  return 0;

    short accept = acceptNode(newNode);

    if (accept == DOMNodeFilter::FILTER_ACCEPT)
        return newNode;

    return getParentNode(newNode);

}


/** Internal function.
 *  Return the nextSibling Node, from the input node
 *  after applying filter, whatToshow.
 *  The current node is not consulted or set.
 */

DOMNode* DOMTreeWalkerImpl::getNextSibling (DOMNode* node) {

    if (!node || node == fRoot) return 0;

    DOMNode* newNode = node->getNextSibling();
    if (!newNode) {

        newNode = node->getParentNode();

        if (!newNode || node == fRoot)  return 0;

        short parentAccept = acceptNode(newNode);

        if (parentAccept == DOMNodeFilter::FILTER_SKIP) {
            return getNextSibling(newNode);
        }

        return 0;
    }

    short accept = acceptNode(newNode);

    if (accept == DOMNodeFilter::FILTER_ACCEPT)
        return newNode;
    else
    if (accept == DOMNodeFilter::FILTER_SKIP) {
        DOMNode* fChild =  getFirstChild(newNode);
        if (!fChild && !newNode->hasChildNodes()) {
            return getNextSibling(newNode);
        }
        return fChild;
    }
    return getNextSibling(newNode);

}


/** Internal function.
 *  Return the previous sibling Node, from the input node
 *  after applying filter, whatToshow.
 *  The current node is not consulted or set.
 */

DOMNode* DOMTreeWalkerImpl::getPreviousSibling (DOMNode* node) {

    if (!node || node == fRoot) return 0;

    DOMNode* newNode = node->getPreviousSibling();
    if (!newNode) {

        newNode = node->getParentNode();
        if (!newNode || node == fRoot)  return 0;

        short parentAccept = acceptNode(newNode);

        if (parentAccept == DOMNodeFilter::FILTER_SKIP) {
            return getPreviousSibling(newNode);
        }

        return 0;
    }

    short accept = acceptNode(newNode);

    if (accept == DOMNodeFilter::FILTER_ACCEPT)
        return newNode;
    else
    if (accept == DOMNodeFilter::FILTER_SKIP) {
        DOMNode* fChild =  getLastChild(newNode);
        if (!fChild && !newNode->hasChildNodes()) {
            return getPreviousSibling(newNode);
        }
        return fChild;
    }
    return getPreviousSibling(newNode);

}


/** Internal function.
 *  Return the first child Node, from the input node
 *  after applying filter, whatToshow.
 *  The current node is not consulted or set.
 */

DOMNode* DOMTreeWalkerImpl::getFirstChild (DOMNode* node) {

    if (!node) return 0;

    if(!fExpandEntityReferences && node->getNodeType()==DOMNode::ENTITY_REFERENCE_NODE)
        return 0;

    DOMNode* newNode = node->getFirstChild();
    if (!newNode)  return 0;

    short accept = acceptNode(newNode);

    if (accept == DOMNodeFilter::FILTER_ACCEPT)
        return newNode;
    else
    if (accept == DOMNodeFilter::FILTER_SKIP
        && newNode->hasChildNodes())
    {
        return getFirstChild(newNode);
    }
    return getNextSibling(newNode);

}


/** Internal function.
 *  Return the last child Node, from the input node
 *  after applying filter, whatToshow.
 *  The current node is not consulted or set.
 */

DOMNode* DOMTreeWalkerImpl::getLastChild (DOMNode* node) {

    if (!node) return 0;

    if(!fExpandEntityReferences && node->getNodeType()==DOMNode::ENTITY_REFERENCE_NODE)
        return 0;

    DOMNode* newNode = node->getLastChild();
    if (!newNode)  return 0;

    short accept = acceptNode(newNode);

    if (accept == DOMNodeFilter::FILTER_ACCEPT)
        return newNode;
    else
    if (accept == DOMNodeFilter::FILTER_SKIP
        && newNode->hasChildNodes())
    {
        return getLastChild(newNode);
    }
    return getPreviousSibling(newNode);

}


/** The node is accepted if it passes the whatToShow and the filter. */

short DOMTreeWalkerImpl::acceptNode (DOMNode* node) {

    if (fNodeFilter == 0) {
        if ( ( fWhatToShow & (1 << (node->getNodeType() - 1))) != 0)
        {
            return DOMNodeFilter::FILTER_ACCEPT;
        }
        else
        {
            return DOMNodeFilter::FILTER_SKIP;
        }
    } else {
        // REVISIT: This logic is unclear from the spec!
        if ((fWhatToShow & (1 << (node->getNodeType() - 1))) != 0 ) {
            return fNodeFilter->acceptNode(node);
        } else {
            // what to show has failed!
            if (fNodeFilter->acceptNode(node) == DOMNodeFilter::FILTER_REJECT) {
                return DOMNodeFilter::FILTER_REJECT;
            } else {
                return DOMNodeFilter::FILTER_SKIP;
            }
        }
    }
}


void DOMTreeWalkerImpl::release()
{
    // for performance reason, do not recycle pointer
    // chance that this is allocated again and again is not usual
}

XERCES_CPP_NAMESPACE_END
