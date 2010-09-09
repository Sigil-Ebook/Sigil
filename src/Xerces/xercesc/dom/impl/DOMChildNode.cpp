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
 * $Id: DOMChildNode.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// This class only adds the ability to have siblings

#include <xercesc/util/XercesDefs.hpp>
#include "DOMNodeImpl.hpp"
#include "DOMChildNode.hpp"
#include "DOMCasts.hpp"

XERCES_CPP_NAMESPACE_BEGIN


DOMChildNode::DOMChildNode()
{
    this->previousSibling  = 0;
    this->nextSibling  = 0;
}

// This only makes a shallow copy, cloneChildren must also be called for a
// deep clone
DOMChildNode::DOMChildNode(const DOMChildNode &)
{
    // Need to break the association w/ original siblings and parent
    this->previousSibling = 0;
    this->nextSibling = 0;
}

DOMChildNode::~DOMChildNode() {
}


DOMNode * DOMChildNode::getNextSibling() const {
    return nextSibling;
}

//
//  Note:  for getParentNode and getPreviousSibling(), below, an
//         extra paramter "thisNode" is required.  This is because there
//         is no way to cast from a DOMChildNode pointer back to the
//         DOMNodeImpl that it is part of.  Our "this" may or may not
//         be preceded by a fParent in the object layout, and there's no
//         practical way to tell, so we just take an extra parameter instead.

DOMNode * DOMChildNode::getParentNode(const DOMNode *thisNode) const
{
    // if we have an owner, ownerNode is our parent, otherwise it's
    // our ownerDocument and we don't have a parent
    DOMNodeImpl *thisNodeImpl = castToNodeImpl(thisNode);
    return thisNodeImpl->isOwned() ? thisNodeImpl->fOwnerNode : 0;
}

DOMNode * DOMChildNode::getPreviousSibling(const DOMNode *thisNode) const {
    // if we are the firstChild, previousSibling actually refers to our
    // parent's lastChild, but we hide that
    return castToNodeImpl(thisNode)->isFirstChild() ? 0 : previousSibling;
}

XERCES_CPP_NAMESPACE_END

