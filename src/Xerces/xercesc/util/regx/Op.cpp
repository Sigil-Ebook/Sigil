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
 * $Id: Op.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/Op.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Op: Constructors and Destructors
// ---------------------------------------------------------------------------
Op::Op(const Op::opType type, MemoryManager* const manager) 
    : fMemoryManager(manager)
    , fOpType(type)
    , fNextOp(0)
{
}

// ---------------------------------------------------------------------------
//  Op: Getter methods
// ---------------------------------------------------------------------------
XMLSize_t Op::getSize() const {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
    return 0; // for compilers that complain about no return value
}

XMLInt32 Op::getData() const {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
    return 0; // for compilers that complain about no return value
}

XMLInt32 Op::getData2() const {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
    return 0; // for compilers that complain about no return value
}

const Op* Op::elementAt(XMLSize_t) const {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
    return 0; // for compilers that complain about no return value
}

const Op* Op::getChild() const {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
    return 0; // for compilers that complain about no return value
}

const XMLCh* Op::getLiteral() const {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
    return 0; // for compilers that complain about no return value
}
    
const Token* Op::getToken() const {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
    return 0; // for compilers that complain about no return value
}


// ---------------------------------------------------------------------------
//  CharOp: Constructors and Destuctors
// ---------------------------------------------------------------------------
CharOp::CharOp(const Op::opType type, const XMLInt32 charData
               , MemoryManager* const manager)
    : Op(type, manager)
      , fCharData(charData) {
}

// ---------------------------------------------------------------------------
//  CharOp: Getter methods
// ---------------------------------------------------------------------------
XMLInt32 CharOp::getData() const {

    return fCharData;
}

// ---------------------------------------------------------------------------
//  UnionOp: Constructors and Destuctors
// ---------------------------------------------------------------------------
UnionOp::UnionOp(const Op::opType type, const XMLSize_t size, MemoryManager* const manager)
    : Op(type, manager)
      , fBranches(new (manager) RefVectorOf<Op> (size, false, manager)) {

}

// ---------------------------------------------------------------------------
//  UnionOp: Getter/Setter methods
// ---------------------------------------------------------------------------
XMLSize_t UnionOp::getSize() const {

    return fBranches->size();
}

const Op* UnionOp::elementAt(XMLSize_t index) const {

    return fBranches->elementAt(index);
}

void UnionOp::addElement(Op* const op) {

    fBranches->addElement(op);
}

// ---------------------------------------------------------------------------
//  ChildOp: Constructors and Destuctors
// ---------------------------------------------------------------------------
ChildOp::ChildOp(const Op::opType type, MemoryManager* const manager)
    : Op(type, manager)
      , fChild(0) {

}

// ---------------------------------------------------------------------------
//  ChildOp: Getter/Setter methods
// ---------------------------------------------------------------------------
const Op* ChildOp::getChild() const {

    return fChild;
}

void ChildOp::setChild(const Op* const child) {

    fChild = child;
}

// ---------------------------------------------------------------------------
//  ModifierOp: Constructors and Destuctors
// ---------------------------------------------------------------------------
ModifierOp::ModifierOp(const Op::opType type, const XMLInt32 v1, const XMLInt32 v2
                       , MemoryManager* const manager)
    : ChildOp(type, manager)
      , fVal1(v1)
      , fVal2(v2) {

}

// ---------------------------------------------------------------------------
//  ModifierOp: Getter methods
// ---------------------------------------------------------------------------
XMLInt32 ModifierOp::getData() const {

    return fVal1;
}

XMLInt32 ModifierOp::getData2() const {

    return fVal2;
}

// ---------------------------------------------------------------------------
//  RangeOp: Constructors and Destuctors
// ---------------------------------------------------------------------------
RangeOp::RangeOp(const Op::opType type, const Token* const token, MemoryManager* const manager)
    : Op (type, manager)
      , fToken(token) {

}

// ---------------------------------------------------------------------------
//  RangeOp: Getter methods
// ---------------------------------------------------------------------------
const Token* RangeOp::getToken() const {

    return fToken;
}


// ---------------------------------------------------------------------------
//  StringOp: Constructors and Destuctors
// ---------------------------------------------------------------------------
StringOp::StringOp(const Op::opType type, const XMLCh* const literal
                   , MemoryManager* const manager)
    : Op (type, manager)
      , fLiteral(XMLString::replicate(literal, manager)) {

}

// ---------------------------------------------------------------------------
//  StringOp: Getter methods
// ---------------------------------------------------------------------------
const XMLCh* StringOp::getLiteral() const {

    return fLiteral;
}

XERCES_CPP_NAMESPACE_END

/**
  * End file Op.cpp
  */
