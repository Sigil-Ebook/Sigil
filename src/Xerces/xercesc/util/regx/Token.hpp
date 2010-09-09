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
 * $Id: Token.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_TOKEN_HPP)
#define XERCESC_INCLUDE_GUARD_TOKEN_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class RangeToken;
class TokenFactory;


class XMLUTIL_EXPORT Token : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Public Constants
    // -----------------------------------------------------------------------
    // Token types
    typedef enum {
        T_CHAR = 0,
        T_CONCAT = 1,
        T_UNION = 2,
        T_CLOSURE = 3,
        T_RANGE = 4,
        T_NRANGE = 5,
        T_PAREN = 6,
        T_EMPTY = 7,
        T_ANCHOR = 8,
        T_NONGREEDYCLOSURE = 9,
        T_STRING = 10,
        T_DOT = 11,
        T_BACKREFERENCE = 12
    } tokType;

    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    Token(const tokType tkType
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
        );
    virtual ~Token();

    static const XMLInt32        UTF16_MAX;

    typedef enum {
        FC_CONTINUE = 0,
        FC_TERMINAL = 1,
        FC_ANY = 2
    } firstCharacterOptions;

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    tokType              getTokenType() const;
    XMLSize_t            getMinLength() const;
    int                  getMaxLength() const;
    virtual Token*       getChild(const XMLSize_t index) const;
    virtual XMLSize_t    size() const;
    virtual int          getMin() const;
    virtual int          getMax() const;
    virtual int          getNoParen() const;
    virtual int          getReferenceNo() const;
    virtual const XMLCh* getString() const;
    virtual XMLInt32     getChar() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setTokenType(const tokType tokType);
    virtual void setMin(const int minVal);
    virtual void setMax(const int maxVal);

    // -----------------------------------------------------------------------
    //  Range manipulation methods
    // -----------------------------------------------------------------------
    virtual void addRange(const XMLInt32 start, const XMLInt32 end);
    virtual void mergeRanges(const Token *const tok);
    virtual void sortRanges();
    virtual void compactRanges();
    virtual void subtractRanges(RangeToken* const tok);
    virtual void intersectRanges(RangeToken* const tok);

    // -----------------------------------------------------------------------
    //  Putter methods
    // -----------------------------------------------------------------------
    virtual void addChild(Token* const child, TokenFactory* const tokFactory);

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    firstCharacterOptions analyzeFirstCharacter(RangeToken* const rangeTok, const int options,
                                                TokenFactory* const tokFactory);
    Token* findFixedString(int options, int& outOptions);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    Token(const Token&);
    Token& operator=(const Token&);

    // -----------------------------------------------------------------------
    //  Private Helper methods
    // -----------------------------------------------------------------------
    bool isSet(const int options, const unsigned int flag);
    bool isShorterThan(Token* const tok);

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    tokType fTokenType;
protected:
    MemoryManager* const    fMemoryManager;
};


// ---------------------------------------------------------------------------
//  Token: getter methods
// ---------------------------------------------------------------------------
inline Token::tokType Token::getTokenType() const {

    return fTokenType;
}

inline XMLSize_t Token::size() const {

    return 0;
}

inline Token* Token::getChild(const XMLSize_t) const {

    return 0;
}

inline int Token::getMin() const {

    return -1;
}

inline int Token::getMax() const {

    return -1;
}

inline int Token::getReferenceNo() const {

    return 0;
}

inline int Token::getNoParen() const {

    return 0;
}

inline const XMLCh* Token::getString() const {

    return 0;
}

inline XMLInt32 Token::getChar() const {

    return -1;
}

// ---------------------------------------------------------------------------
//  Token: setter methods
// ---------------------------------------------------------------------------
inline void Token::setTokenType(const Token::tokType tokType) {
    
    fTokenType = tokType;
}

inline void Token::setMax(const int) {
    // ClosureToken
}

inline void Token::setMin(const int) {
    // ClosureToken
}

inline bool Token::isSet(const int options, const unsigned int flag) {

    return (options & flag) == flag;
}

// ---------------------------------------------------------------------------
//  Token: setter methods
// ---------------------------------------------------------------------------
inline void Token::addChild(Token* const, TokenFactory* const) {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
}

// ---------------------------------------------------------------------------
//  Token: Range manipulation methods
// ---------------------------------------------------------------------------
inline void Token::addRange(const XMLInt32, const XMLInt32) {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
}

inline void Token::mergeRanges(const Token *const) {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
}

inline void Token::sortRanges() {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
}

inline void Token::compactRanges() {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
}

inline void Token::subtractRanges(RangeToken* const) {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
}

inline void Token::intersectRanges(RangeToken* const) {

    ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Regex_NotSupported, fMemoryManager);
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file Token.hpp
  */

