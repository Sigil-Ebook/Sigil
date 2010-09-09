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
 * $Id: RegxParser.hpp 711369 2008-11-04 20:03:14Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_REGXPARSER_HPP)
#define XERCESC_INCLUDE_GUARD_REGXPARSER_HPP

/*
 *    A regular expression parser
 */
// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/regx/Token.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class Token;
class RangeToken;
class TokenFactory;

class XMLUTIL_EXPORT RegxParser : public XMemory
{
public:

    // -----------------------------------------------------------------------
    //  Public constant data
    // -----------------------------------------------------------------------
    // Parse tokens
    typedef enum {
        REGX_T_CHAR                     = 0,
        REGX_T_EOF                      = 1,
        REGX_T_OR                       = 2,
        REGX_T_STAR                     = 3,
        REGX_T_PLUS                     = 4,
        REGX_T_QUESTION                 = 5,
        REGX_T_LPAREN                   = 6,
        REGX_T_RPAREN                   = 7,
        REGX_T_DOT                      = 8,
        REGX_T_LBRACKET                 = 9,
        REGX_T_BACKSOLIDUS              = 10,
        REGX_T_CARET                    = 11,
        REGX_T_DOLLAR                   = 12,
        REGX_T_XMLSCHEMA_CC_SUBTRACTION    = 13
    } parserState;

    typedef enum {
        regexParserStateNormal = 0,
        regexParserStateInBrackets = 1
    } parserStateContext;

    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    RegxParser(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual ~RegxParser();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    parserStateContext  getParseContext() const;
    parserState         getState() const;
    XMLInt32            getCharData() const;
    int                 getNoParen() const;
    XMLSize_t           getOffset() const;
    bool                hasBackReferences() const;
    TokenFactory*       getTokenFactory() const;
    int                 getOptions() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setParseContext(const parserStateContext value);
    void setTokenFactory(TokenFactory* const tokFactory);
    void setOptions(const int options);

    // -----------------------------------------------------------------------
    //  Public Parsing methods
    // -----------------------------------------------------------------------
    Token* parse(const XMLCh* const regxStr, const int options);

protected:
    // -----------------------------------------------------------------------
    //  Protected Helper methods
    // -----------------------------------------------------------------------
    virtual bool        checkQuestion(const XMLSize_t off);
    virtual XMLInt32    decodeEscaped();
    MemoryManager*      getMemoryManager() const;
    // -----------------------------------------------------------------------
    //  Protected Parsing/Processing methods
    // -----------------------------------------------------------------------
    void                processNext();

    Token*              parseRegx(const bool matchingRParen = false);
    virtual Token*      processCaret();
    virtual Token*      processDollar();
    virtual Token*      processBackReference();
    virtual Token*      processStar(Token* const tok);
    virtual Token*      processPlus(Token* const tok);
    virtual Token*      processQuestion(Token* const tok);
    virtual Token*      processParen();

    RangeToken*         parseCharacterClass(const bool useNRange);
    RangeToken*         processBacksolidus_pP(const XMLInt32 ch);

    // -----------------------------------------------------------------------
    //  Protected PreCreated RangeToken access methods
    // -----------------------------------------------------------------------
    RangeToken*         getTokenForShorthand(const XMLInt32 ch);

    bool isSet(const int flag);
private:
    // -----------------------------------------------------------------------
    //  Private parsing/processing methods
    // -----------------------------------------------------------------------
    Token* parseTerm(const bool matchingRParen = false);
    Token* parseFactor();
    Token* parseAtom();

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RegxParser(const RegxParser&);
    RegxParser& operator=(const RegxParser&);

    // -----------------------------------------------------------------------
    //  Private data types
    // -----------------------------------------------------------------------
    class ReferencePosition : public XMemory
    {
        public :
            ReferencePosition(const int refNo, const XMLSize_t position);

            int            fReferenceNo;
            XMLSize_t   fPosition;
    };

    // -----------------------------------------------------------------------
    //  Private Helper methods
    // -----------------------------------------------------------------------
    int hexChar(const XMLInt32 ch);

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    MemoryManager*                  fMemoryManager;
    bool                            fHasBackReferences;
    int                             fOptions;
    XMLSize_t                       fOffset;
    int                             fNoGroups;
    parserStateContext              fParseContext;
    XMLSize_t                       fStringLen;
    parserState                     fState;
    XMLInt32                        fCharData;
    XMLCh*                          fString;
    RefVectorOf<ReferencePosition>* fReferences;
    TokenFactory*                   fTokenFactory;
};


// ---------------------------------------------------------------------------
//  RegxParser: Getter Methods
// ---------------------------------------------------------------------------
inline RegxParser::parserStateContext RegxParser::getParseContext() const {

    return fParseContext;
}

inline RegxParser::parserState RegxParser::getState() const {

    return fState;
}

inline XMLInt32 RegxParser::getCharData() const {

    return fCharData;
}

inline int RegxParser::getNoParen() const {

    return fNoGroups;
}

inline XMLSize_t RegxParser::getOffset() const {

    return fOffset;
}

inline bool RegxParser::hasBackReferences() const {

    return fHasBackReferences;
}

inline TokenFactory* RegxParser::getTokenFactory() const {

    return fTokenFactory;
}

inline MemoryManager* RegxParser::getMemoryManager() const {
    return fMemoryManager;
}

inline int RegxParser::getOptions() const {

    return fOptions;
}

// ---------------------------------------------------------------------------
//  RegxParser: Setter Methods
// ---------------------------------------------------------------------------
inline void RegxParser::setParseContext(const RegxParser::parserStateContext value) {

    fParseContext = value;
}

inline void RegxParser::setTokenFactory(TokenFactory* const tokFactory) {

    fTokenFactory = tokFactory;
}

inline void RegxParser::setOptions(const int options) {

    fOptions = options;
}

// ---------------------------------------------------------------------------
//  RegxParser: Helper Methods
// ---------------------------------------------------------------------------
inline bool RegxParser::isSet(const int flag) {

    return (fOptions & flag) == flag;
}


inline int RegxParser::hexChar(const XMLInt32 ch) {

    if (ch < chDigit_0 || ch > chLatin_f)
        return -1;

    if (ch <= chDigit_9)
        return ch - chDigit_0;

    if (ch < chLatin_A)
        return -1;

    if (ch <= chLatin_F)
        return ch - chLatin_A + 10;

    if (ch < chLatin_a)
        return -1;

    return ch - chLatin_a + 10;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  *    End file RegxParser.hpp
  */

