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
 * $Id: OpFactory.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_OPFACTORY_HPP)
#define XERCESC_INCLUDE_GUARD_OPFACTORY_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/RefVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class Op;
class CharOp;
class UnionOp;
class ChildOp;
class RangeOp;
class StringOp;
class ModifierOp;
class Token;

/*
 * A Factory class used by 'RegularExpression' to create different types of
 * operations (Op) objects. The class will keep track of all objects created
 * for cleanup purposes. Each 'RegularExpression' object will have its own
 * instance of OpFactory and when a 'RegularExpression' object is deleted
 * all associated Op objects will be deleted.
 */

class XMLUTIL_EXPORT OpFactory : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and destructors
    // -----------------------------------------------------------------------
    OpFactory(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~OpFactory();

    // -----------------------------------------------------------------------
    //  Factory methods
    // -----------------------------------------------------------------------
    Op* createDotOp();
    CharOp* createCharOp(XMLInt32 data);
    CharOp* createAnchorOp(XMLInt32 data);
    CharOp* createCaptureOp(int number, const Op* const next);
    UnionOp* createUnionOp(XMLSize_t size);
    ChildOp* createClosureOp(int id);
    ChildOp* createNonGreedyClosureOp();
    ChildOp* createQuestionOp(bool nonGreedy);
    RangeOp* createRangeOp(const Token* const token);
    CharOp* createBackReferenceOp(int refNo);
    StringOp* createStringOp(const XMLCh* const literal);

    // -----------------------------------------------------------------------
    //  Reset methods
    // -----------------------------------------------------------------------
    /*
     *    Remove all created Op objects from Vector
     */
    void reset();

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    OpFactory(const OpFactory&);
    OpFactory& operator=(const OpFactory&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fOpVector
    //      Contains Op objects. Used for memory cleanup.
    // -----------------------------------------------------------------------
    RefVectorOf<Op>* fOpVector;
    MemoryManager*   fMemoryManager;
};

// ---------------------------------------------------------------------------
//  OpFactory - Factory methods
// ---------------------------------------------------------------------------
inline void OpFactory::reset() {

    fOpVector->removeAllElements();
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  *    End file OpFactory
  */
