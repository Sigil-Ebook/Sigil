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
 * $Id: CMUnaryOp.cpp 677396 2008-07-16 19:36:20Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/validators/common/CMStateSet.hpp>
#include <xercesc/validators/common/CMUnaryOp.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  CMUnaryOp: Constructors and Destructor
// ---------------------------------------------------------------------------
CMUnaryOp::CMUnaryOp( ContentSpecNode::NodeTypes type
                    , CMNode* const              nodeToAdopt
                    , unsigned int               maxStates
                    , MemoryManager* const       manager) :
    CMNode(type, maxStates, manager)
    , fChild(nodeToAdopt)
{
    // Insure that its one of the types we require
    if ((type != ContentSpecNode::ZeroOrOne)
    &&  (type != ContentSpecNode::ZeroOrMore)
    &&  (type != ContentSpecNode::OneOrMore))
    {
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::CM_UnaryOpHadBinType, manager);
    }
    if (type == ContentSpecNode::OneOrMore)
        fIsNullable=fChild->isNullable();
    else
        fIsNullable=true;
}

CMUnaryOp::~CMUnaryOp()
{
    delete fChild;
}


// ---------------------------------------------------------------------------
//  CMUnaryOp: Getter methods
// ---------------------------------------------------------------------------
const CMNode* CMUnaryOp::getChild() const
{
    return fChild;
}

CMNode* CMUnaryOp::getChild()
{
    return fChild;
}


// ---------------------------------------------------------------------------
//  CMUnaryOp: Implementation of the public CMNode virtual interface
// ---------------------------------------------------------------------------
void CMUnaryOp::orphanChild()
{
    delete fChild;
    fChild=0;
}

// ---------------------------------------------------------------------------
//  CMUnaryOp: Implementation of the protected CMNode virtual interface
// ---------------------------------------------------------------------------
void CMUnaryOp::calcFirstPos(CMStateSet& toSet) const
{
    // Its just based on our child node's first pos
    toSet = fChild->getFirstPos();
}

void CMUnaryOp::calcLastPos(CMStateSet& toSet) const
{
    // Its just based on our child node's last pos
    toSet = fChild->getLastPos();
}

XERCES_CPP_NAMESPACE_END
