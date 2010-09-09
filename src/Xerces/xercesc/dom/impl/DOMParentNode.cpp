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
 * $Id: DOMParentNode.cpp 678709 2008-07-22 10:56:56Z borisk $
 */

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMNode.hpp>

#include "DOMDocumentImpl.hpp"
#include "DOMRangeImpl.hpp"
#include "DOMNodeIteratorImpl.hpp"
#include "DOMParentNode.hpp"
#include "DOMCasts.hpp"

XERCES_CPP_NAMESPACE_BEGIN

DOMParentNode::DOMParentNode(DOMDocument *ownerDoc)
    : fOwnerDocument(ownerDoc), fFirstChild(0), fChildNodeList(this)
{
}

// This only makes a shallow copy, cloneChildren must also be called for a
// deep clone
DOMParentNode::DOMParentNode(const DOMParentNode &other)  :
    fChildNodeList(this)
{
    this->fOwnerDocument = other.fOwnerDocument;

    // Need to break the association w/ original kids
    this->fFirstChild = 0;
}

void DOMParentNode::changed()
{
  ((DOMDocumentImpl*)fOwnerDocument)->changed();
}


int DOMParentNode::changes() const
{
    return ((DOMDocumentImpl*)fOwnerDocument)->changes();
}


DOMNode * DOMParentNode::appendChild(DOMNode *newChild)
{
    return insertBefore(newChild, 0);
}


void DOMParentNode::cloneChildren(const DOMNode *other) {
  //    for (DOMNode *mykid = other.getFirstChild();
    for (DOMNode *mykid = other->getFirstChild();
         mykid != 0;
         mykid = mykid->getNextSibling())
    {
        appendChild(mykid->cloneNode(true));
    }
}

DOMDocument * DOMParentNode::getOwnerDocument() const {
    return fOwnerDocument;
}

// unlike getOwnerDocument this is not overriden by DocumentImpl to return 0
DOMDocument * DOMParentNode::getDocument() const {
    return fOwnerDocument;
}

void DOMParentNode::setOwnerDocument(DOMDocument* doc) {
    fOwnerDocument = doc;
}

DOMNodeList *DOMParentNode::getChildNodes() const {
    const DOMNodeList *ret = &fChildNodeList;
    return (DOMNodeList *)ret;   // cast off const.
}


DOMNode * DOMParentNode::getFirstChild() const {
    return fFirstChild;
}


DOMNode * DOMParentNode::getLastChild() const
{
    return lastChild();
}

DOMNode * DOMParentNode::lastChild() const
{
    // last child is stored as the previous sibling of first child
    if (fFirstChild == 0) {
        return 0;
    }

    DOMChildNode *firstChild = castToChildImpl(fFirstChild);
    DOMNode *ret = firstChild->previousSibling;
    return ret;
}


//
//  revisit.  Is this function used anywhere?  I don't see it.
//
void DOMParentNode::lastChild(DOMNode *node) {
    // store lastChild as previous sibling of first child
    if (fFirstChild != 0) {
        DOMChildNode *firstChild = castToChildImpl(fFirstChild);
        firstChild->previousSibling = node;
    }
}


bool DOMParentNode::hasChildNodes() const
{
    return fFirstChild!=0;
}



DOMNode *DOMParentNode::insertBefore(DOMNode *newChild, DOMNode *refChild) {
    //not really in the specs, but better than nothing
    if(newChild==NULL)
        throw DOMException(DOMException::HIERARCHY_REQUEST_ERR,0, GetDOMParentNodeMemoryManager);

    DOMNodeImpl *thisNodeImpl = castToNodeImpl(this);
    if (thisNodeImpl->isReadOnly())
        throw DOMException(DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMParentNodeMemoryManager);

    if (newChild->getOwnerDocument() != fOwnerDocument)
        throw DOMException(DOMException::WRONG_DOCUMENT_ERR, 0, GetDOMParentNodeMemoryManager);

    // Prevent cycles in the tree
    //only need to do this if the node has children
    if(newChild->hasChildNodes()) {
        bool treeSafe=true;
        for(DOMNode *a=castToNode(this)->getParentNode();
            treeSafe && a!=0;
            a=a->getParentNode())
            treeSafe=(newChild!=a);
        if(!treeSafe)
            throw DOMException(DOMException::HIERARCHY_REQUEST_ERR,0, GetDOMParentNodeMemoryManager);
    }

    // refChild must in fact be a child of this node (or 0)
    if (refChild!=0 && refChild->getParentNode() != castToNode(this))
        throw DOMException(DOMException::NOT_FOUND_ERR,0, GetDOMParentNodeMemoryManager);

    // if the new node has to be placed before itself, we don't have to do anything
    // (even worse, we would crash if we continue, as we assume they are two distinct nodes)
    if (refChild!=0 && newChild->isSameNode(refChild))
        return newChild;

    if (newChild->getNodeType() == DOMNode::DOCUMENT_FRAGMENT_NODE)
    {
        // SLOW BUT SAFE: We could insert the whole subtree without
        // juggling so many next/previous pointers. (Wipe out the
        // parent's child-list, patch the parent pointers, set the
        // ends of the list.) But we know some subclasses have special-
        // case behavior they add to insertBefore(), so we don't risk it.
        // This approch also takes fewer bytecodes.

        // NOTE: If one of the children is not a legal child of this
        // node, throw HIERARCHY_REQUEST_ERR before _any_ of the children
        // have been transferred. (Alternative behaviors would be to
        // reparent up to the first failure point or reparent all those
        // which are acceptable to the target node, neither of which is
        // as robust. PR-DOM-0818 isn't entirely clear on which it
        // recommends?????

        // No need to check kids for right-document; if they weren't,
        // they wouldn't be kids of that DocFrag.
        for(DOMNode *kid=newChild->getFirstChild(); // Prescan
              kid!=0;
              kid=kid->getNextSibling())
        {
            if (!DOMDocumentImpl::isKidOK(castToNode(this), kid))
              throw DOMException(DOMException::HIERARCHY_REQUEST_ERR,0, GetDOMParentNodeMemoryManager);
        }
        while(newChild->hasChildNodes())     // Move
            castToNode(this)->insertBefore(newChild->getFirstChild(),refChild);
    }

    else if (!DOMDocumentImpl::isKidOK(castToNode(this), newChild))
        throw DOMException(DOMException::HIERARCHY_REQUEST_ERR,0, GetDOMParentNodeMemoryManager);

    else
    {
        DOMNode *oldparent=newChild->getParentNode();
        if(oldparent!=0)
            oldparent->removeChild(newChild);

        // Attach up
        castToNodeImpl(newChild)->fOwnerNode = castToNode(this);
        castToNodeImpl(newChild)->isOwned(true);

        // Attach before and after
        // Note: fFirstChild.previousSibling == lastChild!!
        if (fFirstChild == 0) {
            // this our first and only child
            fFirstChild = newChild;
            castToNodeImpl(newChild)->isFirstChild(true);
            // castToChildImpl(newChild)->previousSibling = newChild;
            DOMChildNode *newChild_ci = castToChildImpl(newChild);
            newChild_ci->previousSibling = newChild;
        } else {
            if (refChild == 0) {
                // this is an append
                DOMNode *lastChild = castToChildImpl(fFirstChild)->previousSibling;
                castToChildImpl(lastChild)->nextSibling = newChild;
                castToChildImpl(newChild)->previousSibling = lastChild;
                castToChildImpl(fFirstChild)->previousSibling = newChild;
            } else {
                // this is an insert
                if (refChild == fFirstChild) {
                    // at the head of the list
                    castToNodeImpl(fFirstChild)->isFirstChild(false);
                    castToChildImpl(newChild)->nextSibling = fFirstChild;
                    castToChildImpl(newChild)->previousSibling = castToChildImpl(fFirstChild)->previousSibling;
                    castToChildImpl(fFirstChild)->previousSibling = newChild;
                    fFirstChild = newChild;
                    castToNodeImpl(newChild)->isFirstChild(true);
                } else {
                    // somewhere in the middle
                    DOMNode *prev = castToChildImpl(refChild)->previousSibling;
                    castToChildImpl(newChild)->nextSibling = refChild;
                    castToChildImpl(prev)->nextSibling = newChild;
                    castToChildImpl(refChild)->previousSibling = newChild;
                    castToChildImpl(newChild)->previousSibling = prev;
                }
            }
        }
    }

    changed();

    if (fOwnerDocument != 0) {
        Ranges* ranges = ((DOMDocumentImpl*)fOwnerDocument)->getRanges();
        if ( ranges != 0) {
            XMLSize_t sz = ranges->size();
            if (sz != 0) {
                for (XMLSize_t i =0; i<sz; i++) {
                    ranges->elementAt(i)->updateRangeForInsertedNode(newChild);
                }
            }
        }
    }

    return newChild;
}



DOMNode *DOMParentNode::removeChild(DOMNode *oldChild)
{
    if (castToNodeImpl(this)->isReadOnly())
        throw DOMException(
        DOMException::NO_MODIFICATION_ALLOWED_ERR, 0, GetDOMParentNodeMemoryManager);

    if (oldChild == 0 || oldChild->getParentNode() != castToNode(this))
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, GetDOMParentNodeMemoryManager);

    if (fOwnerDocument !=  0) {
        //notify iterators
        NodeIterators* nodeIterators = ((DOMDocumentImpl*)fOwnerDocument)->getNodeIterators();
        if (nodeIterators != 0) {
            XMLSize_t sz = nodeIterators->size();
            if (sz != 0) {
                for (XMLSize_t i =0; i<sz; i++) {
                    if (nodeIterators->elementAt(i) != 0)
                        nodeIterators->elementAt(i)->removeNode(oldChild);
                }
            }
        }

        //fix other ranges for change before deleting the node
        Ranges* ranges = ((DOMDocumentImpl*)fOwnerDocument)->getRanges();
        if (ranges != 0) {
            XMLSize_t sz = ranges->size();
            if (sz != 0) {
                for (XMLSize_t i =0; i<sz; i++) {
                    if (ranges->elementAt(i) != 0)
                        ranges->elementAt(i)->updateRangeForDeletedNode(oldChild);
                }
            }
        }
    }


    // Patch linked list around oldChild
    // Note: lastChild == fFirstChild->previousSibling
    if (oldChild == fFirstChild) {
        // removing first child
        castToNodeImpl(oldChild)->isFirstChild(false);
        fFirstChild = castToChildImpl(oldChild)->nextSibling;
        if (fFirstChild != 0) {
            castToNodeImpl(fFirstChild)->isFirstChild(true);
            castToChildImpl(fFirstChild)->previousSibling = castToChildImpl(oldChild)->previousSibling;
        }
    } else {
        DOMNode *prev = castToChildImpl(oldChild)->previousSibling;
        DOMNode *next = castToChildImpl(oldChild)->nextSibling;
        castToChildImpl(prev)->nextSibling = next;
        if (next == 0) {
            // removing last child
            castToChildImpl(fFirstChild)->previousSibling = prev;
        } else {
            // removing some other child in the middle
            castToChildImpl(next)->previousSibling = prev;
        }
    }

    // Remove oldChild's references to tree
    castToNodeImpl(oldChild)->fOwnerNode = fOwnerDocument;
    castToNodeImpl(oldChild)->isOwned(false);
    castToChildImpl(oldChild)->nextSibling = 0;
    castToChildImpl(oldChild)->previousSibling = 0;

    changed();

    return oldChild;
}


DOMNode *DOMParentNode::replaceChild(DOMNode *newChild, DOMNode *oldChild)
{
    insertBefore(newChild, oldChild);
    // changed() already done.
    return removeChild(oldChild);
}


DOMNode * DOMParentNode::appendChildFast(DOMNode *newChild)
{
    // This function makes the following assumptions:
    //
    // - newChild != 0
    // - newChild is not read-only
    // - newChild is not a document fragment
    // - owner documents of this node and newChild are the same
    // - appending newChild to this node cannot result in a cycle
    // - DOMDocumentImpl::isKidOK (this, newChild) return true (that is,
    //   appending newChild to this node results in a valid structure)
    // - newChild->getParentNode() is 0
    // - there are no ranges set for this document
    //

    // Attach up
    castToNodeImpl(newChild)->fOwnerNode = castToNode(this);
    castToNodeImpl(newChild)->isOwned(true);

    // Attach before and after
    // Note: fFirstChild.previousSibling == lastChild!!
    if (fFirstChild != 0)
    {
        DOMNode *lastChild = castToChildImpl(fFirstChild)->previousSibling;
        castToChildImpl(lastChild)->nextSibling = newChild;
        castToChildImpl(newChild)->previousSibling = lastChild;
        castToChildImpl(fFirstChild)->previousSibling = newChild;
    }
    else
    {
        // this our first and only child
        fFirstChild = newChild;
        castToNodeImpl(newChild)->isFirstChild(true);
        // castToChildImpl(newChild)->previousSibling = newChild;
        DOMChildNode *newChild_ci = castToChildImpl(newChild);
        newChild_ci->previousSibling = newChild;
    }

    return newChild;
}


//Introduced in DOM Level 2

void DOMParentNode::normalize()
{
    DOMNode *kid, *next;
    for (kid = fFirstChild; kid != 0; kid = next)
    {
        next = castToChildImpl(kid)->nextSibling;

        // If kid and next are both Text nodes (but _not_ CDATASection,
        // which is a subclass of Text), they can be merged.
        if (next != 0 &&
            kid->getNodeType() == DOMNode::TEXT_NODE   &&
            next->getNodeType() == DOMNode::TEXT_NODE )
        {
            ((DOMTextImpl *) kid)->appendData(((DOMTextImpl *) next)->getData());
            // revisit:
            //   should I release the removed node?
            //   not released in case user still referencing it externally
            removeChild(next);
            next = kid; // Don't advance; there might be another.
        }

        // Otherwise it might be an Element, which is handled recursively
        else
            if (kid->getNodeType() == DOMNode::ELEMENT_NODE)
                kid->normalize();
    }

    // changed() will have occurred when the removeChild() was done,
    // so does not have to be reissued.
}

//Introduced in DOM Level 3

bool DOMParentNode::isEqualNode(const DOMNode* arg) const
{
    if (arg && castToNodeImpl(this)->isSameNode(arg))
        return true;

    if (arg && castToNodeImpl(this)->isEqualNode(arg))
    {
        DOMNode *kid, *argKid;
        for (kid = fFirstChild, argKid = arg->getFirstChild();
             kid != 0 && argKid != 0;
             kid = kid->getNextSibling(), argKid = argKid->getNextSibling())
        {
            if (!kid->isEqualNode(argKid))
                return false;
        }
        return (kid || argKid) ? false : true;
    }
    return false;
}


//Non-standard extension
void DOMParentNode::release()
{
    DOMNode *kid, *next;
    for (kid = fFirstChild; kid != 0; kid = next)
    {
        next = castToChildImpl(kid)->nextSibling;

        // set is Owned false before releasing its child
        castToNodeImpl(kid)->isToBeReleased(true);
        kid->release();
    }
}


XERCES_CPP_NAMESPACE_END
