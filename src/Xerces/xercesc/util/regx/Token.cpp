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
 * $Id: Token.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/RangeToken.hpp>
#include <xercesc/util/regx/RegularExpression.hpp>
#include <xercesc/util/regx/RegxUtil.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Static member data initialization
// ---------------------------------------------------------------------------
const XMLInt32         Token::UTF16_MAX = 0x10FFFF;

// ---------------------------------------------------------------------------
//  Token: Constructors and Destructors
// ---------------------------------------------------------------------------
Token::Token(const Token::tokType tkType
             , MemoryManager* const manager) 
             : fTokenType(tkType) 
             , fMemoryManager(manager)
{

}


Token::~Token() {

}

// ---------------------------------------------------------------------------
//  Token: Getter mthods
// ---------------------------------------------------------------------------
XMLSize_t Token::getMinLength() const {

    switch (fTokenType) {

    case T_CONCAT:
        {
            XMLSize_t sum = 0;
            XMLSize_t childSize = size();

            for (XMLSize_t i=0; i<childSize; i++) {
                sum += getChild(i)->getMinLength();
            }
            return sum;
        }
    case T_UNION:
        {
            XMLSize_t childSize = size();

            if (childSize == 0) {
                return 0;
            }
            XMLSize_t ret = getChild(0)->getMinLength();

            for (XMLSize_t i=1; i < childSize; i++) {

                XMLSize_t min = getChild(i)->getMinLength();
                if (min < ret)
                    ret = min;
            }
            return ret;
        }
    case T_CLOSURE:
    case T_NONGREEDYCLOSURE:
        if (getMin() >= 0)
            return getMin() * getChild(0)->getMinLength();

        return 0;
    case T_EMPTY:
    case T_ANCHOR:
        return 0;
    case T_DOT:
    case T_CHAR:
    case T_RANGE:
    case T_NRANGE:
        return 1;
    case T_PAREN:
        return getChild(0)->getMinLength();
    case T_BACKREFERENCE:
        return 0; // *****  - REVISIT
    case T_STRING:
        return XMLString::stringLen(getString());
//    default:
//        throw;
    }

    // We should not get here, but we have it to make some compilers happy
    return (XMLSize_t)-1;
}


int Token::getMaxLength() const {

    switch (fTokenType) {

    case T_CONCAT:
        {
            int sum = 0;
            XMLSize_t childSize = size();

            for (XMLSize_t i=0; i<childSize; i++) {

                int val = getChild(i)->getMaxLength();

                if (val < 0){
                    return -1;
                }
                sum += val;
            }
            return sum;
        }
    case T_UNION:
        {
            XMLSize_t childSize = size();

            if (childSize == 0)
                return 0;

            int ret = getChild(0)->getMaxLength();

            for (XMLSize_t i = 1; ret > 0 && i < childSize; i++) {

                int max = getChild(i)->getMaxLength();

                if (max < 0) {

                    ret = -1;
                    break;
                }

                if (max > ret)
                    ret = max;
            }
            return ret;
        }
    case T_CLOSURE:
    case T_NONGREEDYCLOSURE:
        if (getMax() >= 0) {
            return getMax() * getChild(0)->getMaxLength();
        }
        return -1;
    case T_EMPTY:
    case T_ANCHOR:
        return 0;
    case T_CHAR:
        return 1;
    case T_DOT:
    case T_RANGE:
    case T_NRANGE:
        return 2;
    case T_PAREN:
        return getChild(0)->getMaxLength();
    case T_BACKREFERENCE:
        return -1; // REVISIT
    case T_STRING:
        return (int)XMLString::stringLen(getString());
//    default:
//        throw; //ThrowXML(RuntimeException, ...)
    } // end switch

    return -1;
}

// ---------------------------------------------------------------------------
//  Token: Helper mthods
// ---------------------------------------------------------------------------
Token::firstCharacterOptions Token::analyzeFirstCharacter(RangeToken* const rangeTok,
                                                          const int options,
                                                          TokenFactory* const tokFactory)
{
    switch(fTokenType) {
    case T_CONCAT:
        {
            firstCharacterOptions ret = FC_CONTINUE;
            for (XMLSize_t i=0; i<size(); i++) {

                Token* tok = getChild(i);
                if (tok
                    && (ret=tok->analyzeFirstCharacter(rangeTok,
                                    options, tokFactory))!= FC_CONTINUE)
                    break;
            }
            return ret;
        }
    case T_UNION:
        {
            XMLSize_t childSize = size();
            if (childSize == 0)
                return FC_CONTINUE;

            firstCharacterOptions ret = FC_CONTINUE;
            bool hasEmpty = false;

            for (XMLSize_t i=0; i < childSize; i++) {

                ret = getChild(i)->analyzeFirstCharacter(rangeTok, options, tokFactory);

                if (ret == FC_ANY)
                    break;
                else
                    hasEmpty = true;
            }
            return hasEmpty ? FC_CONTINUE : ret;
        }
    case T_CLOSURE:
    case T_NONGREEDYCLOSURE:
        {
            Token* tok = getChild(0);
            if (tok)
                tok->analyzeFirstCharacter(rangeTok, options, tokFactory);
            return FC_CONTINUE;
        }
    case T_DOT:
    return FC_ANY;
    case T_EMPTY:
    case T_ANCHOR:
        return FC_CONTINUE;
    case T_CHAR:
        {
            XMLInt32 ch = getChar();
            rangeTok->addRange(ch, ch);
            if (ch < 0x1000 && isSet(options,RegularExpression::IGNORE_CASE)) {
                //REVISIT
            }
        }
        return FC_TERMINAL;
    case T_RANGE:
        {
            if (isSet(options, RegularExpression::IGNORE_CASE)) {
                rangeTok->mergeRanges(((RangeToken*)
                                         this)->getCaseInsensitiveToken(tokFactory));
            }
            else {
                rangeTok->mergeRanges(this);
            }
            return FC_TERMINAL;
        }
    case T_NRANGE:
        {
            if (isSet(options, RegularExpression::IGNORE_CASE)) {

                RangeToken* caseITok = (((RangeToken*)
                                           this)->getCaseInsensitiveToken(tokFactory));
                rangeTok->mergeRanges(RangeToken::complementRanges(caseITok, tokFactory, fMemoryManager));
            }
            else {
                rangeTok->mergeRanges(
                    RangeToken::complementRanges((RangeToken*) this, tokFactory, fMemoryManager));
            }
        }
    case T_PAREN:
        {
            Token* tok = getChild(0);
            if (tok)
                return tok->analyzeFirstCharacter(rangeTok,options, tokFactory);
        }
    case T_BACKREFERENCE:
        rangeTok->addRange(0, UTF16_MAX);
        return FC_ANY;
    case T_STRING:
        {
            const XMLCh* str = getString();
            XMLInt32 ch = str[0];

            if (RegxUtil::isHighSurrogate((XMLCh) ch)) {
            }

            rangeTok->addRange(ch, ch);
            if (ch<0x10000 && isSet(options,RegularExpression::IGNORE_CASE)) {
                //REVISIT
            }
        }
        return FC_TERMINAL;
//    default:
//        throw;
    }

    return FC_CONTINUE;
}


Token* Token::findFixedString(int options, int& outOptions) {

    switch(fTokenType) {

    case T_CHAR:
        return 0;
    case T_STRING:
        outOptions = options;
        return this;
    case T_UNION:
    case T_CLOSURE:
    case T_NONGREEDYCLOSURE:
    case T_EMPTY:
    case T_ANCHOR:
    case T_RANGE:
    case T_NRANGE:
    case T_DOT:
    case T_BACKREFERENCE:
        return 0;
    case T_PAREN:
        return getChild(0)->findFixedString(options, outOptions);
    case T_CONCAT:
        {
            Token* prevTok = 0;
            int prevOptions = 0;

            for (XMLSize_t i=0; i<size(); i++) {

                Token* tok = getChild(i)->findFixedString(options, outOptions);

                if (prevTok == 0 || prevTok->isShorterThan(tok)) {

                    prevTok = tok;
                    prevOptions = outOptions;
                }
            }

            outOptions = prevOptions;
            return prevTok;
        }
    } // end switch

    return 0;
}


bool Token::isShorterThan(Token* const tok) {

    if (tok == 0)
        return false;

    if (getTokenType() != T_STRING && tok->getTokenType() != T_STRING)
        return false; //Should we throw an exception?

    XMLSize_t length = XMLString::stringLen(getString());
    XMLSize_t tokLength = XMLString::stringLen(tok->getString());

    return length < tokLength;
}

XERCES_CPP_NAMESPACE_END

/**
  *    End of file Token.cpp
  */
