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
 * $Id: TokenFactory.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/TokenFactory.hpp>
#include <xercesc/util/regx/TokenInc.hpp>
#include <xercesc/util/regx/XMLRangeFactory.hpp>
#include <xercesc/util/regx/ASCIIRangeFactory.hpp>
#include <xercesc/util/regx/UnicodeRangeFactory.hpp>
#include <xercesc/util/regx/BlockRangeFactory.hpp>
#include <xercesc/util/regx/RangeTokenMap.hpp>
#include <xercesc/util/regx/RegxDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------
//  TokenFactory: Constructors and Destructor
// ---------------------------------------------------------------------------
TokenFactory::TokenFactory(MemoryManager* const manager) :
    fTokens(new (manager) RefVectorOf<Token> (16, true, manager))
    , fEmpty(0)
    , fLineBegin(0)
    , fLineEnd(0)
    , fDot(0)
    , fMemoryManager(manager)
{

}

TokenFactory::~TokenFactory() {

    delete fTokens;
    fTokens = 0;
}

// ---------------------------------------------------------------------------
//  TokenFactory - Factory methods
// ---------------------------------------------------------------------------
Token* TokenFactory::createToken(const Token::tokType tkType) {

    if (tkType == Token::T_EMPTY && fEmpty != 0)
        return fEmpty;

    Token* tmpTok = new (fMemoryManager) Token(tkType, fMemoryManager);

    if (tkType == Token::T_EMPTY) {
        fEmpty = tmpTok;
    }

    fTokens->addElement(tmpTok);

    return tmpTok;
}


ParenToken* TokenFactory::createParenthesis(Token* const token,
                                            const int noGroups) {

    ParenToken* tmpTok = new (fMemoryManager) ParenToken(Token::T_PAREN, token, noGroups, fMemoryManager);

    fTokens->addElement(tmpTok);
    return tmpTok;
}

ClosureToken* TokenFactory::createClosure(Token* const token,
                                          bool isNonGreedy) {

    ClosureToken* tmpTok = isNonGreedy ? new (fMemoryManager) ClosureToken(Token::T_NONGREEDYCLOSURE, token, fMemoryManager)
                                       : new (fMemoryManager) ClosureToken(Token::T_CLOSURE, token, fMemoryManager);
    
    fTokens->addElement(tmpTok);
    return tmpTok;
}

ConcatToken* TokenFactory::createConcat(Token* const token1,
                                        Token* const token2) {

    ConcatToken* tmpTok = new (fMemoryManager) ConcatToken(token1, token2, fMemoryManager);
    
    fTokens->addElement(tmpTok);
    return tmpTok;
}

UnionToken* TokenFactory::createUnion(const bool isConcat) {

    UnionToken* tmpTok = isConcat ? new (fMemoryManager) UnionToken(Token::T_CONCAT, fMemoryManager)
                                  : new (fMemoryManager) UnionToken(Token::T_UNION, fMemoryManager);

    fTokens->addElement(tmpTok);
    return tmpTok;
}

RangeToken* TokenFactory::createRange(const bool isNegRange){


    RangeToken* tmpTok = isNegRange ? new (fMemoryManager) RangeToken(Token::T_NRANGE, fMemoryManager)
                                   : new (fMemoryManager) RangeToken(Token::T_RANGE, fMemoryManager);

    fTokens->addElement(tmpTok);
    return tmpTok;
}

CharToken* TokenFactory::createChar(const XMLUInt32 ch, const bool isAnchor) {

    CharToken* tmpTok = isAnchor ? new (fMemoryManager) CharToken(Token::T_ANCHOR, ch, fMemoryManager)
                                : new (fMemoryManager) CharToken(Token::T_CHAR, ch, fMemoryManager);

    fTokens->addElement(tmpTok);
    return tmpTok;
}

StringToken* TokenFactory::createBackReference(const int noRefs) {

    StringToken* tmpTok = new (fMemoryManager) StringToken(Token::T_BACKREFERENCE, 0, noRefs, fMemoryManager);

    fTokens->addElement(tmpTok);
    return tmpTok;
}

StringToken* TokenFactory::createString(const XMLCh* const literal) {

    StringToken* tmpTok = new (fMemoryManager) StringToken(Token::T_STRING, literal, 0, fMemoryManager);

    fTokens->addElement(tmpTok);
    return tmpTok;
}

// ---------------------------------------------------------------------------
//  TokenFactory - Getter methods
// ---------------------------------------------------------------------------
RangeToken* TokenFactory::staticGetRange(const XMLCh* const keyword,
                                   const bool complement) {

    return RangeTokenMap::instance()->getRange(keyword, complement);
}

Token* TokenFactory::getLineBegin() {

    if (fLineBegin == 0)
        fLineBegin = createChar(chCaret, true);

    return fLineBegin;
}

Token* TokenFactory::getLineEnd() {

    if (fLineEnd == 0)
        fLineEnd = createChar(chDollarSign, true);

    return fLineEnd;
}

Token* TokenFactory::getDot() {

    if (fDot == 0)
        fDot = createToken(Token::T_DOT);

    return fDot;
}

/*
#if HAVE_CONFIG_H
#    include <config.h>
#endif

#if XERCES_USE_TRANSCODER_ICU
   #include <unicode/uchar.h>
#endif

#include <stdio.h>
void TokenFactory::printUnicode() {

#if XERCES_USE_TRANSCODER_ICU
    //
    //  Write it out to a temp file to be read back into this source later.
    //
    printf("Printing\n");
    //sprintf(msg, "Printing\n");
    FILE* outFl = fopen("table.out", "wt+");
    fprintf(outFl, "const XMLByte fgUniCharsTable[0x10000] =\n{    ");
    for (unsigned int index = 0; index <= 0xFFFF; index += 16)
    {
        fprintf(outFl
                , "    , 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n"
                , (unsigned int)u_charType(index)
                , (unsigned int)u_charType(index+1)
                , (unsigned int)u_charType(index+2)
                , (unsigned int)u_charType(index+3)
                , (unsigned int)u_charType(index+4)
                , (unsigned int)u_charType(index+5)
                , (unsigned int)u_charType(index+6)
                , (unsigned int)u_charType(index+7)
                , (unsigned int)u_charType(index+8)
                , (unsigned int)u_charType(index+9)
                , (unsigned int)u_charType(index+10)
                , (unsigned int)u_charType(index+11)
                , (unsigned int)u_charType(index+12)
                , (unsigned int)u_charType(index+13)
                , (unsigned int)u_charType(index+14)
                , (unsigned int)u_charType(index+15));
    }
    fprintf(outFl, "};\n");

    fclose(outFl);
#endif
}
*/

XERCES_CPP_NAMESPACE_END

/**
  * End of file TokenFactory.cpp
  */
