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
 * $Id: DOMRangeImpl.cpp 676911 2008-07-15 13:27:32Z amassari $
 */

#include "DOMRangeImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMDocumentFragmentImpl.hpp"
#include "DOMCommentImpl.hpp"
#include "DOMProcessingInstructionImpl.hpp"
#include "DOMCasts.hpp"

#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMRangeException.hpp>
#include <xercesc/dom/DOMText.hpp>
#include <xercesc/dom/DOMProcessingInstruction.hpp>

#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/Janitor.hpp>

XERCES_CPP_NAMESPACE_BEGIN


//---------------------
// C'tor and D'tor
//---------------------

DOMRangeImpl::DOMRangeImpl(DOMDocument* doc, MemoryManager* const manager)

    :   fStartContainer(doc),     
        fStartOffset(0),
        fEndContainer(doc),
        fEndOffset(0),        
        fCollapsed(true),
        fDocument(doc),   
        fDetached(false),
        fRemoveChild(0),
        fMemoryManager(manager)
{
}

DOMRangeImpl::DOMRangeImpl(const DOMRangeImpl& other)
:   DOMRange(other),
    fStartContainer(other.fStartContainer),
    fStartOffset(other.fStartOffset),
    fEndContainer(other.fEndContainer),
    fEndOffset(other.fEndOffset),
    fCollapsed(other.fCollapsed),
    fDocument(other.fDocument),
    fDetached(other.fDetached),
    fRemoveChild(other.fRemoveChild),
    fMemoryManager(other.fMemoryManager)
{
}

DOMRangeImpl::~DOMRangeImpl()
{
}


//-------------------------------
// Public getter functions
//-------------------------------


DOMNode* DOMRangeImpl::getStartContainer() const
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    return fStartContainer;
}

XMLSize_t DOMRangeImpl::getStartOffset() const
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    return fStartOffset;
}

DOMNode* DOMRangeImpl::getEndContainer() const
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    return fEndContainer;
}

XMLSize_t DOMRangeImpl::getEndOffset() const
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    return fEndOffset;
}



bool DOMRangeImpl::getCollapsed() const
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    return ((fStartContainer == fEndContainer)
             && (fStartOffset == fEndOffset));
}

//-------------------------------
// Public setter functions
//-------------------------------

void DOMRangeImpl::setStartContainer(const DOMNode* node)
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    fStartContainer = (DOMNode*) node;
}

void DOMRangeImpl::setStartOffset(XMLSize_t offset)
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    fStartOffset = offset;
}

void DOMRangeImpl::setEndContainer(const DOMNode* node)
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    fEndContainer = (DOMNode*) node;

}

void DOMRangeImpl::setEndOffset(XMLSize_t offset)
{
    if (fDetached)
    {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    fEndOffset = offset;
}

void DOMRangeImpl::setStart(const DOMNode* refNode, XMLSize_t offset)
{
    validateNode(refNode);
    checkIndex(refNode, offset);

    // error if not the same owner document
    if (fDocument != refNode->getOwnerDocument()) {
        if ( refNode != fDocument ) {
            collapse(true); //collapse the range positions to start
            fCollapsed = true;
            throw DOMException(
                DOMException::WRONG_DOCUMENT_ERR, 0, fMemoryManager);
        }
    }

    fStartContainer = (DOMNode*) refNode;
    fStartOffset    = offset;

    // they may be of same document, but not same root container
    // collapse if not the same root container
    if (!commonAncestorOf(refNode, fEndContainer))
        collapse(true);

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(true); //collapse the range positions to start
    else
        fCollapsed = false;
}

void DOMRangeImpl::setEnd(const DOMNode* refNode, XMLSize_t offset)
{
    validateNode(refNode);
    checkIndex(refNode, offset);

    // error if not the same owner document
    if (fDocument != refNode->getOwnerDocument()) {
        if ( refNode != fDocument ) {
            collapse(false); //collapse the range positions to end
            fCollapsed = true;
            throw DOMException(
                DOMException::WRONG_DOCUMENT_ERR, 0, fMemoryManager);
        }
    }

    fEndContainer   = (DOMNode*) refNode;
    fEndOffset      = offset;

    // they may be of same document, but not same root container
    // collapse if not the same root container
    if (!commonAncestorOf(refNode, fStartContainer))
        collapse(false);

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(false); //collapse the range positions to end
    else
        fCollapsed = false;
}

void DOMRangeImpl::setStartBefore(const DOMNode* refNode)
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }
    if ( !hasLegalRootContainer(refNode) || !isLegalContainedNode(refNode)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0, fMemoryManager);
    }

    // error if not the same owner document
    if (fDocument != refNode->getOwnerDocument()) {
        if ( refNode != fDocument ) {
            collapse(true); //collapse the range positions to start
            fCollapsed = true;
            throw DOMException(
                DOMException::WRONG_DOCUMENT_ERR, 0, fMemoryManager);
        }
    }

    fStartContainer = refNode->getParentNode();
   XMLSize_t i = 0;
    for (DOMNode* n = (DOMNode*) refNode; n!=0; n = n->getPreviousSibling()) {
        i++;
    }
    if (i == 0)
        fStartOffset = 0;
    else
        fStartOffset = i-1;

    // they may be of same document, but not same root container
    // collapse if not the same root container
    if (!commonAncestorOf(refNode, fEndContainer))
        collapse(true);

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(true); //collapse the range positions to start
    else
        fCollapsed = false;
}

void DOMRangeImpl::setStartAfter(const DOMNode* refNode)
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }
    if ( !hasLegalRootContainer(refNode) || !isLegalContainedNode(refNode)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0, fMemoryManager);
    }

    // error if not the same owner document
    if (fDocument != refNode->getOwnerDocument()) {
        if ( refNode != fDocument ) {
            collapse(true); //collapse the range positions to start
            fCollapsed = true;
            throw DOMException(
                DOMException::WRONG_DOCUMENT_ERR, 0, fMemoryManager);
        }
    }

    fStartContainer = refNode->getParentNode();
    XMLSize_t i = 0;
    for (DOMNode* n = (DOMNode*) refNode; n!=0; n = n->getPreviousSibling()) {
        i++;
    }

    fStartOffset = i;

    // they may be of same document, but not same root container
    // collapse if not the same root container
    if (!commonAncestorOf(refNode, fEndContainer))
        collapse(true);

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(true); //collapse the range positions to start
    else
        fCollapsed = false;
}

void DOMRangeImpl::setEndBefore(const DOMNode* refNode)
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }
    if ( !hasLegalRootContainer(refNode) || !isLegalContainedNode(refNode)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0, fMemoryManager);
    }

    // error if not the same owner document
    if (fDocument != refNode->getOwnerDocument()) {
        if ( refNode != fDocument ) {
            collapse(false); //collapse the range positions to end
            fCollapsed = true;
            throw DOMException(
                DOMException::WRONG_DOCUMENT_ERR, 0, fMemoryManager);
        }
    }

    fEndContainer = refNode->getParentNode();
    XMLSize_t i = 0;
    for (DOMNode* n = (DOMNode*) refNode; n!=0; n = n->getPreviousSibling(), i++) ;

    if (i< 1)
        fEndOffset = 0;
    else
        fEndOffset = i-1;

    // they may be of same document, but not same root container
    // collapse if not the same root container
    if (!commonAncestorOf(refNode, fStartContainer))
        collapse(false);

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(false); //collapse the range positions to end
    else
        fCollapsed = false;
}

void DOMRangeImpl::setEndAfter(const DOMNode* refNode)
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }
    if ( !hasLegalRootContainer(refNode) || !isLegalContainedNode(refNode)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0, fMemoryManager);
    }

    // error if not the same owner document
    if (fDocument != refNode->getOwnerDocument()) {
        if ( refNode != fDocument ) {
            collapse(false); //collapse the range positions to end
            fCollapsed = true;
            throw DOMException(
                DOMException::WRONG_DOCUMENT_ERR, 0, fMemoryManager);
        }
    }

    fEndContainer = refNode->getParentNode();
    XMLSize_t i = 0;
    for (DOMNode* n = (DOMNode*) refNode; n!=0; n = n->getPreviousSibling(), i++) ;

    if (i ==0)
        fEndOffset = 0;
    else
        fEndOffset = i;

    // they may be of same document, but not same root container
    // collapse if not the same root container
    if (!commonAncestorOf(refNode, fStartContainer))
        collapse(false);

    //compare the start and end boundary point
    //collapse if start point is after the end point
    if(compareBoundaryPoints(DOMRange::END_TO_START, this) == 1)
        collapse(false); //collapse the range positions to end
    else
        fCollapsed = false;
}
//-------------------------------
// Public Misc. functions
//-------------------------------
void DOMRangeImpl::detach()
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    ((DOMDocumentImpl *)fDocument)->removeRange(this);

    fDetached = true;

    //0ify nodes
    fStartContainer = 0;
    fStartOffset    = 0;
    fEndContainer   = 0;
    fEndOffset      = 0;
    fCollapsed      = true;

    fRemoveChild    = 0;
}

void DOMRangeImpl::collapse(bool toStart)
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    if (toStart) {
        fEndContainer = fStartContainer;
        fEndOffset = fStartOffset;
    } else {
        fStartContainer = fEndContainer;
        fStartOffset = fEndOffset;
    }
    fCollapsed = true;
}

void DOMRangeImpl::selectNode(const DOMNode* refNode)
{
    validateNode(refNode);
    if ( !isLegalContainedNode(refNode)) {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0, fMemoryManager);
    }
    //First check for the text type node
    short type = refNode->getNodeType();
    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        //The node itself is the container.
        fStartContainer = (DOMNode*) refNode;
        fEndContainer   = (DOMNode*) refNode;

        //Select all the contents of the node
        fStartOffset = 0;
        if (type == DOMNode::PROCESSING_INSTRUCTION_NODE)
            fEndOffset = XMLString::stringLen(((DOMProcessingInstruction*)refNode)->getData());
        else
            fEndOffset = ((DOMText *)refNode)->getLength();
        return;
    }

    DOMNode* parent = refNode->getParentNode();
    if (parent != 0 ) // REVIST: what to do if it IS 0?
    {
        fStartContainer = parent;
        fEndContainer = parent;

        XMLSize_t i = 0;
        for (DOMNode* n = parent->getFirstChild(); n!=0 && n!=refNode; n = n->getNextSibling()) {
            i++;
        }

        fStartOffset = i;
        fEndOffset = fStartOffset+1;
    }
}

void DOMRangeImpl::selectNodeContents(const DOMNode* node)
{
    validateNode(node);

    fStartContainer = (DOMNode*) node;
    fEndContainer = (DOMNode*) node;

    fStartOffset = 0;
    short type = node->getNodeType();

    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE)) {

        fEndOffset = ((DOMText *)node)->getLength();
        return;
    }
    if (type == DOMNode::PROCESSING_INSTRUCTION_NODE) {
        fEndOffset = XMLString::stringLen(((DOMProcessingInstruction*)node)->getData());
        return;
    }

    DOMNode* first = node->getFirstChild();
    if (first == 0) {
        fEndOffset = 0;
        return;
    }
    XMLSize_t i = 0;
    for (DOMNode* n = first; n!=0; n = n->getNextSibling()) {
        i++;
    }
    fEndOffset = i;
}

void DOMRangeImpl::surroundContents(DOMNode* newParent)
{
    if (newParent==0) return;

    //check for elimination criteria
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    if (newParent->getOwnerDocument() !=fDocument) {
        throw DOMException(
            DOMException::WRONG_DOCUMENT_ERR, 0, fMemoryManager);
    }

    int type = newParent->getNodeType();
    if ( !isLegalContainedNode(newParent)
        || type == DOMNode::DOCUMENT_TYPE_NODE)
    {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0, fMemoryManager);
    }

    DOMNode* realStart = fStartContainer;
    DOMNode* realEnd = fEndContainer;

    type = fStartContainer->getNodeType();
    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE)) {
        realStart = fStartContainer->getParentNode();
    }
    type = fEndContainer->getNodeType();
    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE)) {
        realEnd = fEndContainer->getParentNode();
    }

    if (realStart != realEnd) {
        throw DOMRangeException(
            DOMRangeException::BAD_BOUNDARYPOINTS_ERR, 0, fMemoryManager);
    }

    DOMDocumentFragment* frag = (DOMDocumentFragment*) extractContents();
    insertNode(newParent);
    newParent->appendChild(frag);
    selectNode(newParent);
}


short DOMRangeImpl::compareBoundaryPoints(DOMRange::CompareHow how, const DOMRange* srcRange) const
{
    if (fDocument != ((DOMRangeImpl*)srcRange)->fDocument) {
        throw DOMException(
            DOMException::WRONG_DOCUMENT_ERR, 0, fMemoryManager);
    }
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    DOMNode* pointA;
    DOMNode* pointB;
    XMLSize_t offsetA, offsetB;

    switch (how)
    {
    case (DOMRange::START_TO_START) :
        pointB = srcRange->getStartContainer();
        pointA = fStartContainer;
        offsetB = srcRange->getStartOffset();
        offsetA = fStartOffset;
        break;
    case (DOMRange::START_TO_END) :
        pointB = srcRange->getStartContainer();
        pointA = fEndContainer;
        offsetB = srcRange->getStartOffset();
        offsetA = fEndOffset;
        break;
    case (DOMRange::END_TO_START) :
        pointB = srcRange->getEndContainer();
        pointA = fStartContainer;
        offsetB = srcRange->getEndOffset();
        offsetA = fStartOffset;
        break;
    case (DOMRange::END_TO_END) :
        pointB = srcRange->getEndContainer();
        pointA = fEndContainer;
        offsetB = srcRange->getEndOffset();
        offsetA = fEndOffset;
        break;
    default:
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    // case 1: same container
    if (pointA == pointB) {
        if (offsetA < offsetB) return -1; //A before B
        if (offsetA == offsetB) return 0; //A equal to B
        return 1; // A after B
    }
    // case 2: Child C of container A is ancestor of B
    for (DOMNode* node = pointA->getFirstChild(); node != 0; node=node->getNextSibling()) {
        if (isAncestorOf(node, pointB)) {
            XMLSize_t index = indexOf(node, pointA);
            if (offsetA <=  index) return -1;
            return 1;
        }
    }
    // case 3: Child C of container B is ancestor of A
    for (DOMNode* nd = pointB->getFirstChild(); nd != 0; nd=nd->getNextSibling()) {
        if (isAncestorOf(nd, pointA)) {
            XMLSize_t index = indexOf(nd, pointB);
            if (index < offsetB ) return -1;
            return 1; //B strictly before A
        }
    }

    // case 4: preorder traversal of context tree.
    // Instead of literally walking the context tree in pre-order,
    // we use relative node depth walking which is usually faster

    int depthDiff = 0;
    DOMNode* n = 0;
    for ( n = pointB; n != 0; n = n->getParentNode() )
        depthDiff++;
    for ( n = pointA; n != 0; n = n->getParentNode() )
        depthDiff--;
    while (depthDiff > 0) {
        pointB = pointB->getParentNode();
        depthDiff--;
    }
    while (depthDiff < 0) {
        pointA = pointA->getParentNode();
        depthDiff++;
    }
    for (DOMNode* pB = pointB->getParentNode(),
         *pA = pointA->getParentNode();
         pB != pA;
         pB = pB->getParentNode(), pA = pA->getParentNode() )
    {
        pointB = pB;
        pointA = pA;
    }
    for ( n = pointB->getNextSibling();
         n != 0;
         n = n->getNextSibling() )
    {
        if (n == pointA) {
            return 1;
        }
    }
    return -1;
}


void DOMRangeImpl:: deleteContents()
{
    traverseContents(DELETE_CONTENTS);
}

DOMDocumentFragment* DOMRangeImpl::extractContents()
{
    checkReadOnly(fStartContainer, fEndContainer, fStartOffset, fEndOffset);
    return traverseContents(EXTRACT_CONTENTS);
}

DOMDocumentFragment* DOMRangeImpl::cloneContents() const
{
    // cast off const.
    return ((DOMRangeImpl *)this)->traverseContents(CLONE_CONTENTS);
}


void DOMRangeImpl::insertNode(DOMNode* newNode)
{
    if (newNode == 0) return; //don't have to do anything

    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    int type = newNode->getNodeType();
    if (type == DOMNode::ATTRIBUTE_NODE
        || type == DOMNode::ENTITY_NODE
        || type == DOMNode::NOTATION_NODE
        || type == DOMNode::DOCUMENT_NODE)
    {
        throw DOMRangeException(
            DOMRangeException::INVALID_NODE_TYPE_ERR, 0, fMemoryManager);
    }

    // Prevent cycles in the tree.
    //isKidOK() is not checked here as its taken care by insertBefore() function
    if (isAncestorOf( newNode, fStartContainer)) {
        throw DOMException(
            DOMException::HIERARCHY_REQUEST_ERR, 0, fMemoryManager);
    }

    for (DOMNode* aNode = fStartContainer; aNode!=0; aNode = aNode->getParentNode()) {
        if (castToNodeImpl(newNode)->isReadOnly()) {
        throw DOMException(
            DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, fMemoryManager);
    }
    }

    if (fDocument != newNode->getOwnerDocument()) {
        throw DOMException(
            DOMException::WRONG_DOCUMENT_ERR, 0, fMemoryManager);
    }


    DOMNode* parent;
    DOMNode* next;

    type = fStartContainer->getNodeType();
    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE)) {

        //set 'parent' and 'next' here
        parent = fStartContainer->getParentNode();

        //split the text nodes
       if (fStartOffset > 0) {
           if (type == DOMNode::COMMENT_NODE)
               ((DOMCommentImpl*)fStartContainer)->splitText(fStartOffset);
           else if (type == DOMNode::PROCESSING_INSTRUCTION_NODE)
               ((DOMProcessingInstructionImpl*)fStartContainer)->splitText(fStartOffset);
           else
               ((DOMText*)fStartContainer)->splitText(fStartOffset);
       }

        //update the new start information later. After inserting the first newNode
        if (fStartOffset == 0)
            next = fStartContainer;
        else
            next = fStartContainer->getNextSibling();

    } // end of text handling
    else {
        parent = fStartContainer;

        next = fStartContainer->getFirstChild();
        for(XMLSize_t i = 0; (i < fStartOffset) && (next != 0); i++) {
            next=next->getNextSibling();
        }
    }

    if (parent != 0) {
        if (next != 0)
            parent->insertBefore(newNode, next);
        else
            parent->appendChild(newNode);
    }
}

DOMRange* DOMRangeImpl::cloneRange() const
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    DOMRange* range = fDocument->createRange();
    range->setStart(fStartContainer, fStartOffset);
    range->setEnd(fEndContainer, fEndOffset);

    return range;
}

const XMLCh* DOMRangeImpl::toString() const
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    if ((fStartContainer == fEndContainer) && (fEndOffset == fStartOffset))
        return XMLUni::fgZeroLenString;

    DOMNode* node = fStartContainer;
    DOMNode* stopNode = fEndContainer;

    XMLBuffer retStringBuf(1023, ((DOMDocumentImpl *)fDocument)->getMemoryManager());
    short type = fStartContainer->getNodeType();
    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE)) {
        if (fStartContainer == fEndContainer) {
            XMLCh* tempString;
            XMLCh temp[4000];
            if ((fEndOffset-fStartOffset) >= 3999)
                tempString = (XMLCh*) fMemoryManager->allocate
                (
                    (fEndOffset - fStartOffset + 1) * sizeof(XMLCh)
                );//new XMLCh[fEndOffset-fStartOffset+1];
            else
                tempString = temp;

            XMLString::subString(tempString, fStartContainer->getNodeValue(), fStartOffset, fEndOffset, ((DOMDocumentImpl *)fDocument)->getMemoryManager());
            const XMLCh* retString = ((DOMDocumentImpl *)fDocument)->getPooledString(tempString);

            if ((fEndOffset-fStartOffset) >= 3999)
                fMemoryManager->deallocate(tempString);//delete[] tempString;

            return retString;
        } else {
            XMLSize_t length = XMLString::stringLen(fStartContainer->getNodeValue());
            if (length != fStartOffset) {

                XMLCh* tempString;
                XMLCh temp[4000];
                if ((length - fStartOffset) >= 3999)
                    tempString = (XMLCh*) fMemoryManager->allocate
                    (
                        (length - fStartOffset + 1) * sizeof(XMLCh)
                    );//new XMLCh[length - fStartOffset+1];
                else
                    tempString = temp;

                XMLString::subString(tempString, fStartContainer->getNodeValue(), fStartOffset, length, ((DOMDocumentImpl *)fDocument)->getMemoryManager());
                retStringBuf.append(tempString);

                if ((length - fStartOffset) >= 3999)
                    fMemoryManager->deallocate(tempString);//delete[] tempString;
            }

            node = nextNode(node, true);
        }
    }else { //fStartContainer is not a TextNode
        node=node->getFirstChild();
        if (fStartOffset>0) { //find a first node within a range, specified by fStartOffset
            XMLSize_t counter = 0;
            while (counter<fStartOffset && node!=0) {
                node=node->getNextSibling();
                counter++;
            }
        }
        if (node == 0) {
            node = nextNode(fStartContainer,false);
        }
    }

    type = fEndContainer->getNodeType();
    if((type != DOMNode::TEXT_NODE
        && type != DOMNode::CDATA_SECTION_NODE
        && type != DOMNode::COMMENT_NODE
        && type != DOMNode::PROCESSING_INSTRUCTION_NODE)) {
        int i=(int)fEndOffset;
        stopNode = fEndContainer->getFirstChild();
        while( i>0 && stopNode!=0 ){
            --i;
            stopNode = stopNode->getNextSibling();
        }
        if ( stopNode == 0 )
            stopNode = nextNode( fEndContainer, false );
    }

    while (node != stopNode) {  //look into all kids of the Range
        if (node == 0) break;
        type = node->getNodeType();

        if((type == DOMNode::TEXT_NODE
            || type == DOMNode::CDATA_SECTION_NODE
            || type == DOMNode::COMMENT_NODE
            || type == DOMNode::PROCESSING_INSTRUCTION_NODE)) {
            retStringBuf.append(node->getNodeValue());
        }
        node = nextNode(node, true);
    }

    type = fEndContainer->getNodeType();
    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE)) {

        if (fEndOffset != 0) {

            XMLCh* tempString;
            XMLCh temp[4000];
            if (fEndOffset >= 3999)
                tempString = (XMLCh*) fMemoryManager->allocate
                (
                    (fEndOffset+1) * sizeof(XMLCh)
                );//new XMLCh[fEndOffset+1];
            else
                tempString = temp;

            XMLString::subString(tempString, fEndContainer->getNodeValue(), 0, fEndOffset, ((DOMDocumentImpl *)fDocument)->getMemoryManager());
            retStringBuf.append(tempString);

            if (fEndOffset >= 3999)
                fMemoryManager->deallocate(tempString);//delete[] tempString;
        }
    }
    return ((DOMDocumentImpl *)fDocument)->getPooledString(retStringBuf.getRawBuffer());
}

DOMDocument* DOMRangeImpl::getDocument()
{
    return fDocument;
}

const DOMNode* DOMRangeImpl::getCommonAncestorContainer() const
{
     return commonAncestorOf(fStartContainer, fEndContainer);

}

void DOMRangeImpl::release()
{
    detach();
    // for performance reason, do not recycle pointer
    // chance that this is allocated again and again is not usual
}

//---------------------
//private functions
//---------------------

bool DOMRangeImpl::isValidAncestorType(const DOMNode* node) const
{
    for (DOMNode* aNode = (DOMNode*) node; aNode!=0; aNode = aNode->getParentNode()) {
        short type = aNode->getNodeType();
        if ( type == DOMNode::ENTITY_NODE
            || type == DOMNode::NOTATION_NODE
            || type == DOMNode::DOCUMENT_TYPE_NODE)
            return false;
    }
    return true;
}

bool DOMRangeImpl::isAncestorOf(const DOMNode* a, const DOMNode* b) {
    for (DOMNode* node = (DOMNode*) b; node != 0; node=node->getParentNode()) {
        if  (node == a) return true;
    }
    return false;
}

bool DOMRangeImpl::hasLegalRootContainer(const DOMNode* node) const {
    if ( node==0 )
        return false;

    DOMNode* rootContainer = (DOMNode*)node;
    for (; rootContainer->getParentNode()!=0; rootContainer = rootContainer->getParentNode())
        ;

    switch( rootContainer->getNodeType() ) {
        case DOMNode::ATTRIBUTE_NODE:
        case DOMNode::DOCUMENT_NODE:
        case DOMNode::DOCUMENT_FRAGMENT_NODE:
            return true;
        default:
            return false;
    }
}

bool DOMRangeImpl::isLegalContainedNode(const DOMNode* node ) const {
   if ( node==0 )
       return false;
   switch( node->getNodeType() )
   {
       case DOMNode::DOCUMENT_NODE:
       case DOMNode::DOCUMENT_FRAGMENT_NODE:
       case DOMNode::ATTRIBUTE_NODE:
       case DOMNode::ENTITY_NODE:
       case DOMNode::NOTATION_NODE:
            return false;
       default:
            return true;
   }   
}

XMLSize_t DOMRangeImpl::indexOf(const DOMNode* child, const DOMNode* parent) const
{
    XMLSize_t i = 0;
    if (child->getParentNode() != parent) return (XMLSize_t)-1;
    for(DOMNode* node = child->getPreviousSibling(); node!= 0; node=node->getPreviousSibling()) {
        i++;
    }
    return i;
}

void DOMRangeImpl::validateNode(const DOMNode* node) const
{
    if( fDetached) {
        throw DOMException(
            DOMException::INVALID_STATE_ERR, 0, fMemoryManager);
    }

    if ( !isValidAncestorType(node)) {
        throw DOMRangeException(DOMRangeException::INVALID_NODE_TYPE_ERR, 0, fMemoryManager);
    }
}


const DOMNode* DOMRangeImpl::commonAncestorOf(const DOMNode* pointA, const DOMNode* pointB) const
{
    if (fDetached)
        throw DOMException(DOMException::INVALID_STATE_ERR, 0, fMemoryManager);

    //if the containers are same then it itself is its common ancestor.
    if (pointA == pointB)
        return pointA;

    typedef RefVectorOf<DOMNode> VectorNodes;
    VectorNodes startV(1, false, ((DOMDocumentImpl *)fDocument)->getMemoryManager());
    DOMNode* node;

    for (node=(DOMNode*)pointA; node != 0; node=node->getParentNode())
    {
        startV.addElement(node);
    }
    VectorNodes endV(1, false, ((DOMDocumentImpl *)fDocument)->getMemoryManager());
    for (node=(DOMNode*)pointB; node != 0; node=node->getParentNode())
    {
        endV.addElement(node);
    }

    XMLSize_t s = startV.size();
    XMLSize_t e = endV.size();

    DOMNode* commonAncestor = 0;

    while (s>0 && e>0) {
        if (startV.elementAt(s-1) == endV.elementAt(e-1)) {
            commonAncestor = startV.elementAt(s-1);
        }
        else  break;
        --s;
        --e;
    }

    return commonAncestor;
}

void DOMRangeImpl::checkIndex(const DOMNode* node, XMLSize_t offset) const
{
    short type = node->getNodeType();

    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE)) {
        if (offset > XMLString::stringLen(node->getNodeValue()))
            throw DOMException( DOMException::INDEX_SIZE_ERR, 0, fMemoryManager );
        else  return;
    }

    DOMNode* child = node->getFirstChild();
    XMLSize_t i = 0;
    for (; child != 0; i++) {
        child = child->getNextSibling();
    }
    if (i < offset) {
        throw DOMException( DOMException::INDEX_SIZE_ERR, 0, fMemoryManager );
    }

}

DOMNode* DOMRangeImpl::nextNode(const DOMNode* node, bool visitChildren) const
{

    if (node == 0) return 0;

    DOMNode* result;
    if (visitChildren) {
        result = node->getFirstChild();
        if (result != 0) {
            return result;
        }
    }

    // if hasSibling, return sibling
    result = node->getNextSibling();
    if (result != 0) {
        return result;
    }


    // return parent's 1st sibling.
    DOMNode* parent = node->getParentNode();


    while ( (parent != 0) && (parent != fDocument) )
    {
        result = parent->getNextSibling();
        if (result != 0) {
            return result;
        } else {
            parent = parent->getParentNode();
        }

    }
    // end of list, return 0
    return 0;
}


/** This is the master routine invoked to visit the nodes
*   selected by this range.  For each such node, different
*   actions are taken depending on the value of the TraversalType argument.
*/
DOMDocumentFragment* DOMRangeImpl::traverseContents(TraversalType how)
{
    if (fDetached)
            throw DOMException(DOMException::INVALID_STATE_ERR, 0, fMemoryManager);

    if (fStartContainer == 0 || fEndContainer == 0) {
        return 0; // REVIST: Throw exception?
    }

    /* Traversal is accomplished by first determining the
       relationship between the endpoints of the range.
       For each of four significant relationships, we will
       delegate the traversal call to a method that
       can make appropriate assumptions.
    */

    // case 1: same container
    if ( fStartContainer == fEndContainer )
        return traverseSameContainer( how );

    // case 2: Child C of start container is ancestor of end container
    // This can be quickly tested by walking the parent chain of
    // end container
    int endContainerDepth = 0;
    for ( DOMNode* c = fEndContainer, *p = c->getParentNode();
             p != 0;
             c = p, p = p->getParentNode())
        {
            if (p == fStartContainer)
                return traverseCommonStartContainer( c, how );
            ++endContainerDepth;
        }

    // case 3: Child C of end container  is ancestor of start container
    // This can be quickly tested by walking the parent chain of A
    int startContainerDepth = 0;
    for ( DOMNode* c2 = fStartContainer, *p2 = c2->getParentNode();
         p2 != 0;
         c2 = p2, p2 = p2->getParentNode())
    {
        if (p2 == fEndContainer)
            return traverseCommonEndContainer( c2, how );
        ++startContainerDepth;
    }

    // case 4: There is a common ancestor container.  Find the
    // ancestor siblings that are children of that container.
    int depthDiff = startContainerDepth - endContainerDepth;

    DOMNode* startNode = fStartContainer;
    while (depthDiff > 0) {
        startNode = startNode->getParentNode();
        depthDiff--;
    }

    DOMNode* endNode = fEndContainer;
    while (depthDiff < 0) {
        endNode = endNode->getParentNode();
        depthDiff++;
    }

    // ascend the ancestor hierarchy until we have a common parent.
    for( DOMNode* sp = startNode->getParentNode(), *ep = endNode->getParentNode();
         sp!=ep;
         sp = sp->getParentNode(), ep = ep->getParentNode() )
    {
        startNode = sp;
        endNode = ep;
    }
    return traverseCommonAncestors( startNode, endNode, how );
    }

/**
 * Visits the nodes selected by this range when we know
 * a-priori that the start and end containers are the same.
 *
 */
DOMDocumentFragment* DOMRangeImpl::traverseSameContainer( int how )
{
    DOMDocumentFragment* frag = 0;
    if ( how!=DELETE_CONTENTS)
        frag = fDocument->createDocumentFragment();

    // If selection is empty, just return the fragment
    if ( fStartOffset==fEndOffset )
            return frag;

    DOMNode* cloneCurrent = 0;

    // Text node needs special case handling
    short type = fStartContainer->getNodeType();
    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        cloneCurrent = fStartContainer->cloneNode(false);
        if (fEndOffset == fStartOffset) {
            cloneCurrent->setNodeValue(XMLUni::fgZeroLenString);
        }
        else {
            XMLCh* tempString;
            XMLCh temp[4000];
            if (fEndOffset >= 3999)
                tempString = (XMLCh*) fMemoryManager->allocate
                (
                    (fEndOffset+1) * sizeof(XMLCh)
                );//new XMLCh[fEndOffset+1];
            else
                tempString = temp;

            XMLString::subString(tempString, cloneCurrent->getNodeValue(), fStartOffset, fEndOffset, ((DOMDocumentImpl *)fDocument)->getMemoryManager());
            cloneCurrent->setNodeValue(((DOMDocumentImpl *)fDocument)->getPooledString(tempString));

            if (fEndOffset >= 3999)
                fMemoryManager->deallocate(tempString);//delete[] tempString;
        }

        // set the original text node to its new value
        if ( how != CLONE_CONTENTS ) {
            if(type == DOMNode::PROCESSING_INSTRUCTION_NODE) {
                ((DOMProcessingInstructionImpl*)fStartContainer)->deleteData(fStartOffset, fEndOffset-fStartOffset);
            }
            else
                ((DOMCharacterData*)fStartContainer)->deleteData(fStartOffset, fEndOffset-fStartOffset);
        }
        if ( how != DELETE_CONTENTS)
            frag->appendChild(cloneCurrent);
    }
    else {
        // Copy nodes between the start/end offsets.
        DOMNode* n = getSelectedNode( fStartContainer, (int)fStartOffset );
        int cnt = (int)fEndOffset - (int)fStartOffset;
        while( cnt > 0 && n)
        {
            DOMNode* sibling = n->getNextSibling();
            DOMNode* xferNode = traverseFullySelected( n, how );
            if ( frag!=0 )
                frag->appendChild( xferNode );
            --cnt;
            n = sibling;
        }
    }

    // Nothing is partially selected, so collapse to start point
    if ( how != CLONE_CONTENTS )
            collapse(true);
    return frag;
}

/**
 * Visits the nodes selected by this range when we know
 * a-priori that the start and end containers are not the
 * same, but the start container is an ancestor of the end container
 *
 */
DOMDocumentFragment* DOMRangeImpl::traverseCommonStartContainer( DOMNode*endAncestor, int how )
{
    DOMDocumentFragment* frag = 0;
    if ( how!=DELETE_CONTENTS)
        frag = fDocument->createDocumentFragment();
    DOMNode*n = traverseRightBoundary( endAncestor, how );
    if ( frag!=0 )
        frag->appendChild( n );

    XMLSize_t endIdx = indexOf( endAncestor, fStartContainer );
    if ( endIdx <= fStartOffset )
    {
        // Collapse to just before the endAncestor, which
        // is partially selected.
        if ( how != CLONE_CONTENTS )
        {
            setEndBefore( endAncestor );
            collapse( false );
        }
        return frag;
    }

    n = endAncestor->getPreviousSibling();
    int cnt = (int)endIdx - (int)fStartOffset;
    while( cnt > 0 )
    {
        DOMNode* sibling = n->getPreviousSibling();
        DOMNode* xferNode = traverseFullySelected( n, how );
        if ( frag!=0 )
            frag->insertBefore( xferNode, frag->getFirstChild() );
        --cnt;
        n = sibling;
    }
    // Collapse to just before the endAncestor, which
    // is partially selected.
    if ( how != CLONE_CONTENTS )
    {
        setEndBefore( endAncestor );
        collapse( false );
    }
    return frag;
}

/**
 * Visits the nodes selected by this range when we know
 * a-priori that the start and end containers are not the
 * same, but the end container is an ancestor of the start container
 *
 */
DOMDocumentFragment* DOMRangeImpl::traverseCommonEndContainer( DOMNode*startAncestor, int how )
{
    DOMDocumentFragment* frag = 0;
    if ( how!=DELETE_CONTENTS)
        frag = fDocument->createDocumentFragment();
    DOMNode* n = traverseLeftBoundary( startAncestor, how );
    if ( frag!=0 )
        frag->appendChild( n );
    XMLSize_t startIdx = indexOf( startAncestor, fEndContainer );
    ++startIdx;  // Because we already traversed it....

    int cnt = (int)fEndOffset - (int)startIdx;
    n = startAncestor->getNextSibling();
    while( cnt > 0 )
    {
        DOMNode* sibling = n->getNextSibling();
        DOMNode* xferNode = traverseFullySelected( n, how );
        if ( frag!=0 )
            frag->appendChild( xferNode );
        --cnt;
        n = sibling;
    }

    if ( how != CLONE_CONTENTS )
    {
        setStartAfter( startAncestor );
        collapse( true );
    }

    return frag;
}

/**
 * Visits the nodes selected by this range when we know
 * a-priori that the start and end containers are not
 * the same, and we also know that neither the start
 * nor end container is an ancestor of the other.
 */
DOMDocumentFragment* DOMRangeImpl::traverseCommonAncestors( DOMNode*startAncestor, DOMNode*endAncestor, int how )
{
    DOMDocumentFragment* frag = 0;
    if ( how!=DELETE_CONTENTS)
        frag = fDocument->createDocumentFragment();

    DOMNode*n = traverseLeftBoundary( startAncestor, how );
    if ( frag!=0 )
        frag->appendChild( n );

    DOMNode*commonParent = startAncestor->getParentNode();
    XMLSize_t startOffset = indexOf( startAncestor, commonParent );
    XMLSize_t endOffset = indexOf( endAncestor, commonParent );
    ++startOffset;

    int cnt = (int)endOffset - (int)startOffset;
    DOMNode* sibling = startAncestor->getNextSibling();

    while( cnt > 0 )
    {
        DOMNode* nextSibling = sibling->getNextSibling();
        n = traverseFullySelected( sibling, how );
        if ( frag!=0 )
            frag->appendChild( n );
        sibling = nextSibling;
        --cnt;
    }

    n = traverseRightBoundary( endAncestor, how );
    if ( frag!=0 )
        frag->appendChild( n );

    if ( how != CLONE_CONTENTS )
    {
        setStartAfter( startAncestor );
        collapse( true );
    }
    return frag;
}

/**
 * Traverses the "right boundary" of this range and
 * operates on each "boundary node" according to the
 * how parameter.  It is a-priori assumed
 * by this method that the right boundary does
 * not contain the range's start container.
 *
 * A "right boundary" is best visualized by thinking
 * of a sample tree:
 *                 A
 *                /|\
 *               / | \
 *              /  |  \
 *             B   C   D
 *            /|\     /|\
 *           E F G   H I J
 *
 * Imagine first a range that begins between the
 * "E" and "F" nodes and ends between the
 * "I" and "J" nodes.  The start container is
 * "B" and the end container is "D".  Given this setup,
 * the following applies:
 *
 * Partially Selected Nodes: B, D<br>
 * Fully Selected Nodes: F, G, C, H, I
 *
 * The "right boundary" is the highest subtree node
 * that contains the ending container.  The root of
 * this subtree is always partially selected.
 *
 * In this example, the nodes that are traversed
 * as "right boundary" nodes are: H, I, and D.
 *
 */
DOMNode* DOMRangeImpl::traverseRightBoundary( DOMNode*root, int how )
{
    DOMNode*next = getSelectedNode( fEndContainer, (int)fEndOffset-1 );
    bool isFullySelected = ( next!=fEndContainer );

    if ( next==root )
        return traverseNode( next, isFullySelected, false, how );

    DOMNode*parent = next->getParentNode();
    DOMNode*clonedParent = traverseNode( parent, false, false, how );

    while( parent!=0 )
    {
        while( next!=0 )
        {
            DOMNode* prevSibling = next->getPreviousSibling();
            DOMNode* clonedChild =
                traverseNode( next, isFullySelected, false, how );
            if ( how!=DELETE_CONTENTS )
            {
                clonedParent->insertBefore(
                    clonedChild,
                    clonedParent->getFirstChild()
                );
            }
            isFullySelected = true;
            next = prevSibling;
        }
        if ( parent==root )
            return clonedParent;

        next = parent->getPreviousSibling();
        parent = parent->getParentNode();
        DOMNode* clonedGrandParent = traverseNode( parent, false, false, how );
        if ( how!=DELETE_CONTENTS )
            clonedGrandParent->appendChild( clonedParent );
        clonedParent = clonedGrandParent;

    }

    // should never occur
    return 0;
}

/**
 * Traverses the "left boundary" of this range and
 * operates on each "boundary node" according to the
 * how parameter.  It is a-priori assumed
 * by this method that the left boundary does
 * not contain the range's end container.
 *
 * A "left boundary" is best visualized by thinking
 * of a sample tree:
 *
 *                 A
 *                /|\
 *               / | \
 *              /  |  \
 *             B   C   D
 *            /|\     /|\
 *           E F G   H I J
 *
 * Imagine first a range that begins between the
 * "E" and "F" nodes and ends between the
 * "I" and "J" nodes.  The start container is
 * "B" and the end container is "D".  Given this setup,
 * the following applies:
 *
 * Partially Selected Nodes: B, D<br>
 * Fully Selected Nodes: F, G, C, H, I
 *
 * The "left boundary" is the highest subtree node
 * that contains the starting container.  The root of
 * this subtree is always partially selected.
 *
 * In this example, the nodes that are traversed
 * as "left boundary" nodes are: F, G, and B.
 *
 */
DOMNode* DOMRangeImpl::traverseLeftBoundary( DOMNode*root, int how )
{
    DOMNode*next = getSelectedNode( getStartContainer(), (int)getStartOffset() );
    bool isFullySelected = ( next!=getStartContainer() );

    if ( next==root )
        return traverseNode( next, isFullySelected, true, how );

    DOMNode* parent = next->getParentNode();
    DOMNode* clonedParent = traverseNode( parent, false, true, how );

    while( parent!=0 )
    {
        while( next!=0 )
        {
            DOMNode* nextSibling = next->getNextSibling();
            DOMNode* clonedChild =
                traverseNode( next, isFullySelected, true, how );
            if ( how!=DELETE_CONTENTS )
                clonedParent->appendChild(clonedChild);
            isFullySelected = true;
            next = nextSibling;
        }
        if ( parent==root )
            return clonedParent;

        next = parent->getNextSibling();
        parent = parent->getParentNode();
        DOMNode* clonedGrandParent = traverseNode( parent, false, true, how );
        if ( how!=DELETE_CONTENTS )
            clonedGrandParent->appendChild( clonedParent );
        clonedParent = clonedGrandParent;

    }

    // should never occur
    return 0;

}

/**
 * Utility method for traversing a single node.
 * Does not properly handle a text node containing both the
 * start and end offsets.  Such nodes should
 * have been previously detected and been routed to traverseTextNode.
 *
 */
DOMNode* DOMRangeImpl::traverseNode( DOMNode* n, bool isFullySelected, bool isLeft, int how )
{
    if ( isFullySelected )
        return traverseFullySelected( n, how );

    short type = n->getNodeType();

    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
        return traverseTextNode( n, isLeft, how );

    return traversePartiallySelected( n, how );
}

/**
 * Utility method for traversing a single node when
 * we know a-priori that the node if fully
 * selected.
 *
 */
DOMNode* DOMRangeImpl::traverseFullySelected( DOMNode* n, int how )
{
    switch( how )
    {
    case CLONE_CONTENTS:
        return n->cloneNode( true );
    case EXTRACT_CONTENTS:
        return n;
    case DELETE_CONTENTS:
        // revisit:
        //   should I release the removed node?
        //   not released in case user still referencing it externally
        n->getParentNode()->removeChild(n);
        return 0;
    }
    return 0;
}

/**
 * Utility method for traversing a single node when
 * we know a-priori that the node if partially
 * selected and is not a text node.
 *
 */
DOMNode* DOMRangeImpl::traversePartiallySelected( DOMNode*n, int how )
{
    switch( how )
    {
    case DELETE_CONTENTS:
        return 0;
    case CLONE_CONTENTS:
    case EXTRACT_CONTENTS:
        return n->cloneNode( false );
    }
    return 0;
}

/**
 * Utility method for traversing a text node that we know
 * a-priori to be on a left or right boundary of the range.
 * This method does not properly handle text nodes that contain
 * both the start and end points of the range.
 *
 */
DOMNode* DOMRangeImpl::traverseTextNode( DOMNode*n, bool isLeft, int how )
{
    XMLCh* txtValue = XMLString::replicate(n->getNodeValue(), fMemoryManager);
    ArrayJanitor<XMLCh> janValue(txtValue, fMemoryManager);

    if ( isLeft )
    {
        XMLSize_t startLen = XMLString::stringLen(fStartContainer->getNodeValue());
        XMLSize_t offset = getStartOffset();

        if (offset == 0) {
            if ( how != CLONE_CONTENTS )
                n->setNodeValue(XMLUni::fgZeroLenString);
        }
        else {
            XMLCh* oldNodeValue;
            XMLCh oldTemp[4000];

            if (offset >= 3999)  {
                oldNodeValue = (XMLCh*) fMemoryManager->allocate
                (
                    (offset+1) * sizeof(XMLCh)
                );//new XMLCh[offset+1];
            }
            else {
                oldNodeValue = oldTemp;
            }
            XMLString::subString(oldNodeValue, txtValue, 0, offset, ((DOMDocumentImpl *)fDocument)->getMemoryManager());

            if ( how != CLONE_CONTENTS )
                n->setNodeValue( ((DOMDocumentImpl *)fDocument)->getPooledString(oldNodeValue) );

            if (offset>= 3999)
                fMemoryManager->deallocate(oldNodeValue);//delete[] oldNodeValue;
        }

        if ( how==DELETE_CONTENTS )
            return 0;

        DOMNode* newNode = n->cloneNode( false );

        if (startLen == offset) {
            newNode->setNodeValue(XMLUni::fgZeroLenString);
        }
        else {
            XMLCh* newNodeValue;
            XMLCh newTemp[4000];

            if (offset >= 3999)  {
                newNodeValue = (XMLCh*) fMemoryManager->allocate
                (
                    (offset+1) * sizeof(XMLCh)
                );//new XMLCh[offset+1];
            }
            else {
                newNodeValue = newTemp;
            }
            XMLString::subString(newNodeValue, txtValue, offset, startLen, ((DOMDocumentImpl *)fDocument)->getMemoryManager());
            newNode->setNodeValue( ((DOMDocumentImpl *)fDocument)->getPooledString(newNodeValue) );

            if (offset>= 3999)
                fMemoryManager->deallocate(newNodeValue);//delete[] newNodeValue;

        }
        return newNode;
    }
    else
    {
        XMLSize_t endLen = XMLString::stringLen(fEndContainer->getNodeValue());
        XMLSize_t offset = getEndOffset();

        if (endLen == offset) {
            if ( how != CLONE_CONTENTS )
                n->setNodeValue(XMLUni::fgZeroLenString);
        }
        else {
            XMLCh* oldNodeValue;
            XMLCh oldTemp[4000];

            if (offset >= 3999)  {
                oldNodeValue = (XMLCh*) fMemoryManager->allocate
                (
                    (offset+1) * sizeof(XMLCh)
                );//new XMLCh[offset+1];
            }
            else {
                oldNodeValue = oldTemp;
            }
            XMLString::subString(oldNodeValue, txtValue, offset, endLen, ((DOMDocumentImpl *)fDocument)->getMemoryManager());

            if ( how != CLONE_CONTENTS )
                n->setNodeValue( ((DOMDocumentImpl *)fDocument)->getPooledString(oldNodeValue) );

            if (offset>= 3999)
                fMemoryManager->deallocate(oldNodeValue);//delete[] oldNodeValue;
        }

        if ( how==DELETE_CONTENTS )
            return 0;

        DOMNode* newNode = n->cloneNode( false );

        if (offset == 0) {
            newNode->setNodeValue(XMLUni::fgZeroLenString);
        }
        else {
            XMLCh* newNodeValue;
            XMLCh newTemp[4000];

            if (offset >= 3999)  {
                newNodeValue = (XMLCh*) fMemoryManager->allocate
                (
                    (offset+1) * sizeof(XMLCh)
                );//new XMLCh[offset+1];
            }
            else {
                newNodeValue = newTemp;
            }
            XMLString::subString(newNodeValue, txtValue, 0, offset, ((DOMDocumentImpl *)fDocument)->getMemoryManager());
            newNode->setNodeValue( ((DOMDocumentImpl *)fDocument)->getPooledString(newNodeValue) );

            if (offset>= 3999)
                fMemoryManager->deallocate(newNodeValue);//delete[] newNodeValue;

        }
        return newNode;
    }
}

/**
 * Utility method to retrieve a child node by index.  This method
 * assumes the caller is trying to find out which node is
 * selected by the given index.  Note that if the index is
 * greater than the number of children, this implies that the
 * first node selected is the parent node itself.
 *
 */
DOMNode* DOMRangeImpl::getSelectedNode( DOMNode*container, int offset )
{
    short type = container->getNodeType();
    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
        return container;

    // This case is an important convenience for
    // traverseRightBoundary()
    if ( offset<0 )
        return container;

    DOMNode*child = container->getFirstChild();
    while( child!=0 && offset > 0 )
    {
        --offset;
        child = child->getNextSibling();
    }
    if ( child!=0 )
        return child;
    return container;
}

void DOMRangeImpl::checkReadOnly(DOMNode* start, DOMNode* end,
                              XMLSize_t startOffset, XMLSize_t endOffset)
{
    if ((start == 0) || (end == 0) ) return;
    DOMNode*sNode = 0;

    short type = start->getNodeType();
    if ( type == DOMNode::DOCUMENT_TYPE_NODE )
    {
        throw DOMException(
            DOMException::HIERARCHY_REQUEST_ERR, 0, fMemoryManager);
    }

    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        if (castToNodeImpl(start)->isReadOnly()) {
            throw DOMException(
                DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, fMemoryManager);
        }
        //if both start and end are text check and return
        if (start == end)
            return;

        sNode = start;
    } else {
        //set the start and end nodes to check
        sNode = start->getFirstChild();
        for(XMLSize_t i = 0; i<startOffset; i++)
            sNode = sNode->getNextSibling();
    }

    DOMNode* eNode;
    type = end->getNodeType();
    if ( type == DOMNode::DOCUMENT_TYPE_NODE )
    {
        throw DOMException(
            DOMException::HIERARCHY_REQUEST_ERR, 0, fMemoryManager);
    }

    if((type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        eNode = end; //need to check only till this node
    }
    else { //need to check all the kids that fall before the end offset value
        eNode = end->getFirstChild();
        if (endOffset > 0)  {
            for (XMLSize_t i = 0; i<endOffset-1; i++)
                eNode = eNode->getNextSibling();
        }
    }
    //recursivly search if any node is readonly
    recurseTreeAndCheck(sNode, eNode);
}

void DOMRangeImpl::recurseTreeAndCheck(DOMNode* start, DOMNode* end)
{
    for(DOMNode* node=start; node != 0 && node !=end; node=node->getNextSibling())
    {
        if ( node->getNodeType()== DOMNode::DOCUMENT_TYPE_NODE )
        {
            throw DOMException(
                DOMException::HIERARCHY_REQUEST_ERR, 0, fMemoryManager);
        }

        if (castToNodeImpl(node)->isReadOnly()) {
            throw DOMException(
                DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, fMemoryManager);
        }

        if (node->hasChildNodes()) {
            node = node->getFirstChild();
            recurseTreeAndCheck(node, end);
        }
    }
}


DOMNode* DOMRangeImpl::removeChild(DOMNode* parent, DOMNode* child)
{
    fRemoveChild = child; //only a precaution measure not to update this range data before removal
    DOMNode*n = parent->removeChild(child);
    fRemoveChild = 0;
    return n;
}


//
// Mutation functions
//


/* This function is called from DOM.
*  The  text has already been replaced.
*  Fix-up any offsets.
*/
void DOMRangeImpl::receiveReplacedText(DOMNode* node)
{
    if (node == 0) return;

    short type = fStartContainer->getNodeType();
    if (node == fStartContainer
        && (type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        fStartOffset = 0;
    }
    type = fEndContainer->getNodeType();
    if (node == fEndContainer
        && (type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        fEndOffset = 0;
    }
}


/** This function is called from DOM.
*  The  text has already been deleted.
*  Fix-up any offsets.
*/
void DOMRangeImpl::updateRangeForDeletedText(DOMNode* node, XMLSize_t offset, XMLSize_t count)
{
    if (node == 0) return;

    short type = fStartContainer->getNodeType();
    if (node == fStartContainer
        && (type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        if (fStartOffset > offset+count) {
            fStartOffset = fStartOffset-count;
        } else if (fStartOffset > offset) {
            fStartOffset = offset;
        }
    }
    type = fEndContainer->getNodeType();
    if (node == fEndContainer
        && (type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        if (fEndOffset > offset+count) {
            fEndOffset = fEndOffset-count;
        } else if (fEndOffset > offset) {
            fEndOffset = offset;
        }
    }
}



/** This function is called from DOM.
*  The  text has already beeen inserted.
*  Fix-up any offsets.
*/
void DOMRangeImpl::updateRangeForInsertedText(DOMNode* node, XMLSize_t offset, XMLSize_t count)
{
    if (node == 0) return;

    short type = fStartContainer->getNodeType();
    if (node == fStartContainer
        && (type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        if (fStartOffset > offset) {
            fStartOffset = offset;
        }
    }
    type = fEndContainer->getNodeType();
    if (node == fEndContainer
        && (type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        if (fEndOffset > offset) {
            fEndOffset = fEndOffset+count;
        }
    }
}



/** This function must be called by the DOM _BEFORE_
*  a node is deleted, because at that time it is
*  connected in the DOM tree, which we depend on.
*/
void DOMRangeImpl::updateRangeForDeletedNode(DOMNode* node)
{

    if (node == 0) return;
    if (fRemoveChild == node) return;

    if (node->getParentNode() == fStartContainer) {
        XMLSize_t index = indexOf(node, fStartContainer);
        if ( fStartOffset > index) {
            fStartOffset--;
        }
    }

    if (node->getParentNode() == fEndContainer) {
        XMLSize_t index = indexOf(node, fEndContainer);
        if ( fEndOffset > index) {
            fEndOffset--;
        }
    }

    if (node->getParentNode() != fStartContainer
        ||  node->getParentNode() != fEndContainer) {
        if (isAncestorOf(node, fStartContainer)) {
            DOMNode* tpNode = node->getParentNode();
            setStartContainer( tpNode );
            fStartOffset = indexOf( node, tpNode);
        }
        if (isAncestorOf(node, fEndContainer)) {
            DOMNode* tpNode = node->getParentNode();
            setEndContainer( tpNode );
            fEndOffset = indexOf( node, tpNode);
        }
    }

}

void DOMRangeImpl::updateRangeForInsertedNode(DOMNode* node) {
    if (node == 0) return;

    if (node->getParentNode() == fStartContainer) {
        XMLSize_t index = indexOf(node, fStartContainer);
        if (index < fStartOffset) {
            fStartOffset++;
        }
    }

    if (node->getParentNode() == fEndContainer) {
        XMLSize_t index = indexOf(node, fEndContainer);
        if (index < fEndOffset) {
            fEndOffset++;
        }
    }
}


void DOMRangeImpl::updateSplitInfo(DOMNode* oldNode, DOMNode* startNode, XMLSize_t offset)
{
    if (startNode == 0) return;

    short type = fStartContainer->getNodeType();
    if (oldNode == fStartContainer
        && (type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        if (fStartOffset > offset) {
            fStartOffset = fStartOffset - offset;
            fStartContainer = startNode;
        }
    }

    type = fEndContainer->getNodeType();
    if (oldNode == fEndContainer
        && (type == DOMNode::TEXT_NODE
        || type == DOMNode::CDATA_SECTION_NODE
        || type == DOMNode::COMMENT_NODE
        || type == DOMNode::PROCESSING_INSTRUCTION_NODE))
    {
        if (fEndOffset > offset) {
            fEndContainer = startNode;
           fEndOffset = fEndOffset - offset;
        }
    }
}



XERCES_CPP_NAMESPACE_END

