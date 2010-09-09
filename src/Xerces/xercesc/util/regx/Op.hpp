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
 * $Id: Op.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_OP_HPP)
#define XERCESC_INCLUDE_GUARD_OP_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/util/RuntimeException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class Token;


class XMLUTIL_EXPORT Op : public XMemory
{
public:

    typedef enum {
        O_DOT                       = 0,
        O_CHAR                      = 1,
        O_RANGE                     = 3,
        O_NRANGE                    = 4,
        O_ANCHOR                    = 5,
        O_STRING                    = 6,
        O_CLOSURE                   = 7,
        O_NONGREEDYCLOSURE          = 8,
        O_FINITE_CLOSURE            = 9,
        O_FINITE_NONGREEDYCLOSURE   = 10,
        O_QUESTION                  = 11,
        O_NONGREEDYQUESTION         = 12,
        O_UNION                     = 13,
        O_CAPTURE                   = 15,
        O_BACKREFERENCE             = 16
    } opType;

    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    virtual ~Op() { }

    // -----------------------------------------------------------------------
    // Getter functions
    // -----------------------------------------------------------------------
            opType       getOpType() const;
            const Op*    getNextOp() const;
    virtual XMLInt32     getData() const;
    virtual XMLInt32     getData2() const;
    virtual XMLSize_t    getSize() const;
    virtual const Op*    elementAt(XMLSize_t index) const;
    virtual const Op*    getChild() const;
    virtual const Token* getToken() const;
    virtual const XMLCh* getLiteral() const;

    // -----------------------------------------------------------------------
    // Setter functions
    // -----------------------------------------------------------------------
    void setOpType(const opType type);
    void setNextOp(const Op* const next);

protected:
    // -----------------------------------------------------------------------
    //  Protected Constructors
    // -----------------------------------------------------------------------
    Op(const opType type, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    friend class OpFactory;

    MemoryManager* const fMemoryManager;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    Op(const Op&);
    Op& operator=(const Op&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fOpType
    //      Indicates the type of operation
    //
    //  fNextOp
    //      Points to the next operation in the chain
    // -----------------------------------------------------------------------
    opType      fOpType;
    const Op*   fNextOp;
};


class XMLUTIL_EXPORT CharOp: public Op {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    CharOp(const opType type, const XMLInt32 charData, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~CharOp() {}

    // -----------------------------------------------------------------------
    // Getter functions
    // -----------------------------------------------------------------------
    XMLInt32 getData() const;

private:
    // Private data members
    XMLInt32 fCharData;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    CharOp(const CharOp&);
    CharOp& operator=(const CharOp&);
};

class XMLUTIL_EXPORT UnionOp : public Op {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    UnionOp(const opType type, const XMLSize_t size,
            MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~UnionOp() { delete fBranches; }

    // -----------------------------------------------------------------------
    // Getter functions
    // -----------------------------------------------------------------------
    XMLSize_t getSize() const;
    const Op* elementAt(XMLSize_t index) const;

    // -----------------------------------------------------------------------
    // Setter functions
    // -----------------------------------------------------------------------
    void addElement(Op* const op);

private:
    // Private Data members
    RefVectorOf<Op>* fBranches;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    UnionOp(const UnionOp&);
    UnionOp& operator=(const UnionOp&);
};


class XMLUTIL_EXPORT ChildOp: public Op {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    ChildOp(const opType type, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~ChildOp() {}

    // -----------------------------------------------------------------------
    // Getter functions
    // -----------------------------------------------------------------------
    const Op* getChild() const;

    // -----------------------------------------------------------------------
    // Setter functions
    // -----------------------------------------------------------------------
    void setChild(const Op* const child);

private:
    // Private data members
    const Op* fChild;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ChildOp(const ChildOp&);
    ChildOp& operator=(const ChildOp&);
};

class XMLUTIL_EXPORT ModifierOp: public ChildOp {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    ModifierOp(const opType type, const XMLInt32 v1, const XMLInt32 v2, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~ModifierOp() {}

    // -----------------------------------------------------------------------
    // Getter functions
    // -----------------------------------------------------------------------
    XMLInt32 getData() const;
    XMLInt32 getData2() const;

private:
    // Private data members
    XMLInt32 fVal1;
    XMLInt32 fVal2;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ModifierOp(const ModifierOp&);
    ModifierOp& operator=(const ModifierOp&);
};

class XMLUTIL_EXPORT RangeOp: public Op {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    RangeOp(const opType type, const Token* const token, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~RangeOp() {}

    // -----------------------------------------------------------------------
    // Getter functions
    // -----------------------------------------------------------------------
    const Token* getToken() const;

private:
    // Private data members
    const Token* fToken;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RangeOp(const RangeOp&);
    RangeOp& operator=(const RangeOp&);
};

class XMLUTIL_EXPORT StringOp: public Op {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    StringOp(const opType type, const XMLCh* const literal, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~StringOp() { fMemoryManager->deallocate(fLiteral);}

    // -----------------------------------------------------------------------
    // Getter functions
    // -----------------------------------------------------------------------
    const XMLCh* getLiteral() const;

private:
    // Private data members
    XMLCh* fLiteral;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    StringOp(const StringOp&);
    StringOp& operator=(const StringOp&);
};

// ---------------------------------------------------------------------------
//  Op: getter methods
// ---------------------------------------------------------------------------
inline Op::opType Op::getOpType() const {

    return fOpType;
}

inline const Op* Op::getNextOp() const {

    return fNextOp;
}

// ---------------------------------------------------------------------------
//  Op: setter methods
// ---------------------------------------------------------------------------
inline void Op::setOpType(const Op::opType type) {

    fOpType = type;
}

inline void Op::setNextOp(const Op* const nextOp) {
    
    fNextOp = nextOp;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file Op.hpp
  */
