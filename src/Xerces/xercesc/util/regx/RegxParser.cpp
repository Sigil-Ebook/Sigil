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
 * $Id: RegxParser.cpp 834826 2009-11-11 10:03:53Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/RegxParser.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/ParseException.hpp>
#include <xercesc/util/regx/RegularExpression.hpp>
#include <xercesc/util/regx/RegxUtil.hpp>
#include <xercesc/util/regx/RegxDefs.hpp>
#include <xercesc/util/regx/TokenInc.hpp>
#include <xercesc/framework/XMLErrorCodes.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  RegxParser::ReferencePostion: Constructors and Destructor
// ---------------------------------------------------------------------------
RegxParser::ReferencePosition::ReferencePosition(const int refNo,
                                                 const XMLSize_t position)
    :fReferenceNo(refNo)
    , fPosition(position)
{

}

// ---------------------------------------------------------------------------
//  RegxParser: Constructors and Destructors
// ---------------------------------------------------------------------------
RegxParser::RegxParser(MemoryManager* const manager)
    :fMemoryManager(manager),
     fHasBackReferences(false),
     fOptions(0),
     fOffset(0),
     fNoGroups(1),
     fParseContext(regexParserStateNormal),
     fStringLen(0),
     fState(REGX_T_EOF),
     fCharData(0),
     fString(0),
     fReferences(0),
     fTokenFactory(0)
{
}

RegxParser::~RegxParser() {

    fMemoryManager->deallocate(fString);//delete [] fString;
    delete fReferences;
}

// ---------------------------------------------------------------------------
//  RegxParser: Parsing methods
// ---------------------------------------------------------------------------
Token* RegxParser::parse(const XMLCh* const regxStr, const int options) {

    // if TokenFactory is not set do nothing.
    // REVISIT - should we throw an exception
    if (fTokenFactory == 0) {
        return 0;
    }

    fOptions = options;
    fOffset = 0;
    fNoGroups = 1;
    fHasBackReferences = false;
    setParseContext(regexParserStateNormal);
    if (fString)
        fMemoryManager->deallocate(fString);//delete [] fString;
    fString = XMLString::replicate(regxStr, fMemoryManager);

    if (isSet(RegularExpression::EXTENDED_COMMENT)) {

        if (fString)
            fMemoryManager->deallocate(fString);//delete [] fString;
        fString = RegxUtil::stripExtendedComment(regxStr, fMemoryManager);
    }

    fStringLen = XMLString::stringLen(fString);
    processNext();

    Token* retTok = parseRegx();

    if (fOffset != fStringLen) {
        XMLCh value1[65];
        XMLString::sizeToText(fOffset, value1, 64, 10, fMemoryManager);
        ThrowXMLwithMemMgr2(ParseException,XMLExcepts::Parser_Parse1, value1, fString, fMemoryManager);
    }

    if (fReferences != 0) {

        XMLSize_t refSize = fReferences->size();
        for (XMLSize_t i = 0; i < refSize; i++) {

            if (fNoGroups <= fReferences->elementAt(i)->fReferenceNo) {
                ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Parse2, fMemoryManager);
            }
        }

        fReferences->removeAllElements();
    }

    return retTok;
}


void RegxParser::processNext() {

    if (fOffset >= fStringLen) {

        fCharData = -1;
        fState = REGX_T_EOF;
        return;
    }

    parserState nextState;
    XMLCh ch = fString[fOffset++];
    fCharData = ch;

    if (fParseContext == regexParserStateInBrackets) {

        switch (ch) {
        case chBackSlash:
            nextState = REGX_T_BACKSOLIDUS;

            if (fOffset >= fStringLen) {
                ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Next1, fMemoryManager);
            }

            fCharData = fString[fOffset++];
            break;
        case chDash:
            if (fOffset < fStringLen && fString[fOffset] == chOpenSquare) {

                fOffset++;
                nextState = REGX_T_XMLSCHEMA_CC_SUBTRACTION;
            }
            else {
                nextState = REGX_T_CHAR;
            }
            break;
        default:
            if (RegxUtil::isHighSurrogate(ch) && fOffset < fStringLen) {

                XMLCh lowCh = fString[fOffset];
                if (RegxUtil::isLowSurrogate(lowCh)) {
                    fCharData = RegxUtil::composeFromSurrogate(ch, lowCh);
                    fOffset++;
                }
                else {
                    throw XMLErrs::Expected2ndSurrogateChar;
                }
            }

            nextState = REGX_T_CHAR;
        }

        fState = nextState;
        return;
    }

    switch (ch) {

    case chPipe:
        nextState = REGX_T_OR;
        break;
    case chAsterisk:
        nextState = REGX_T_STAR;
        break;
    case chPlus:
        nextState = REGX_T_PLUS;
        break;
    case chQuestion:
        nextState = REGX_T_QUESTION;
        break;
    case chCloseParen:
        nextState = REGX_T_RPAREN;
        break;
    case chPeriod:
        nextState = REGX_T_DOT;
        break;
    case chOpenSquare:
        nextState = REGX_T_LBRACKET;
        break;
    case chCaret:
        nextState = REGX_T_CARET;
        break;
    case chDollarSign:
        nextState = REGX_T_DOLLAR;
        break;
    case chOpenParen:
        nextState = REGX_T_LPAREN;
        break;
    case chBackSlash:
        nextState = REGX_T_BACKSOLIDUS;
        if (fOffset >= fStringLen) {
            ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Next1, fMemoryManager);
        }

        fCharData = fString[fOffset++];
        break;
    default:
        nextState = REGX_T_CHAR;
        if (RegxUtil::isHighSurrogate(ch) && fOffset < fStringLen) {

                XMLCh lowCh = fString[fOffset];
                if (RegxUtil::isLowSurrogate(lowCh)) {
                    fCharData = RegxUtil::composeFromSurrogate(ch, lowCh);
                    fOffset++;
                }
                else {
                    throw XMLErrs::Expected2ndSurrogateChar;
                }
            }
    }

    fState = nextState;
}


Token* RegxParser::parseRegx(const bool matchingRParen) {

    Token* tok = parseTerm(matchingRParen);
    Token* parentTok = 0;

    while (fState == REGX_T_OR) {

        processNext();
        if (parentTok == 0) {

            parentTok = fTokenFactory->createUnion();
            parentTok->addChild(tok, fTokenFactory);
            tok = parentTok;
        }

        tok->addChild(parseTerm(matchingRParen), fTokenFactory);
    }

    return tok;
}


Token* RegxParser::parseTerm(const bool matchingRParen) {

    parserState state = fState;

    if (state == REGX_T_OR || state == REGX_T_EOF
        || (state == REGX_T_RPAREN && matchingRParen)) {
        return fTokenFactory->createToken(Token::T_EMPTY);
    }
    else {

        Token* tok = parseFactor();
        Token* concatTok = 0;

        while ((state = fState) != REGX_T_OR && state != REGX_T_EOF
               && (state != REGX_T_RPAREN || !matchingRParen))
        {
            if (concatTok == 0) {

                concatTok = fTokenFactory->createUnion(true);
                concatTok->addChild(tok, fTokenFactory);
                tok = concatTok;
            }
            concatTok->addChild(parseFactor(), fTokenFactory);
        }

        return tok;
    }
}


Token* RegxParser::processCaret() {

    processNext();
    return fTokenFactory->getLineBegin();
}


Token* RegxParser::processDollar() {

    processNext();
    return fTokenFactory->getLineEnd();
}


Token* RegxParser::processStar(Token* const tok) {

    processNext();

    if (fState == REGX_T_QUESTION) {
        processNext();
        return fTokenFactory->createClosure(tok, true);
    }

    return fTokenFactory->createClosure(tok);
}


Token* RegxParser::processPlus(Token* const tok) {

    processNext();

    if (fState == REGX_T_QUESTION) {
        processNext();
        return fTokenFactory->createConcat(tok,
                           fTokenFactory->createClosure(tok,true));
    }

    return fTokenFactory->createConcat(tok,
                                fTokenFactory->createClosure(tok));
}


Token* RegxParser::processQuestion(Token* const tok) {

    processNext();

    Token* parentTok = fTokenFactory->createUnion();

    if (fState == REGX_T_QUESTION) {
        processNext();
        parentTok->addChild(fTokenFactory->createToken(Token::T_EMPTY), fTokenFactory);
        parentTok->addChild(tok, fTokenFactory);
    }
    else {
        parentTok->addChild(tok, fTokenFactory);
        parentTok->addChild(fTokenFactory->createToken(Token::T_EMPTY), fTokenFactory);
    }

    return parentTok;
}


Token* RegxParser::processParen() {

    processNext();
    int num = fNoGroups++;
    Token* tok = fTokenFactory->createParenthesis(parseRegx(true),num);

    if (fState != REGX_T_RPAREN)
        ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Factor1, fMemoryManager);

    processNext();
    return tok;
}


Token* RegxParser::processBackReference() {

    XMLSize_t position = fOffset - 2;

    // Handle multi digit back references
    int refNo = fCharData - chDigit_0;
    while(true) {
        processNext();
        if(fState != REGX_T_CHAR || fCharData < chDigit_0 || fCharData > chDigit_9)
            break;

        int nextRefNo = (refNo * 10) + fCharData - chDigit_0;
        if(nextRefNo >= fNoGroups)
            break;

        refNo = nextRefNo;
    }

    Token* tok = fTokenFactory->createBackReference(refNo);

    fHasBackReferences = true;
    if (fReferences == 0) {
        fReferences = new (fMemoryManager) RefVectorOf<ReferencePosition>(8, true, fMemoryManager);
    }

    fReferences->addElement(new (fMemoryManager) ReferencePosition(refNo, position));
    return tok;
}


Token* RegxParser::parseFactor() {

    Token* tok = parseAtom();

    switch(fState) {

    case REGX_T_STAR:
        return processStar(tok);
    case REGX_T_PLUS:
        return processPlus(tok);
    case REGX_T_QUESTION:
        return processQuestion(tok);
    case REGX_T_CHAR:
        if (fCharData == chOpenCurly && fOffset < fStringLen) {

            int min = 0;
            int max = -1;
            XMLInt32 ch = fString[fOffset++];

            if (ch >= chDigit_0 && ch <= chDigit_9) {

                min = ch - chDigit_0;
                while (fOffset < fStringLen
                       && (ch = fString[fOffset++]) >= chDigit_0
                       && ch <= chDigit_9) {

                    min = min*10 + ch - chDigit_0;
                }

                if (min < 0)
                    ThrowXMLwithMemMgr1(ParseException, XMLExcepts::Parser_Quantifier5, fString, fMemoryManager);
            }
            else {
                ThrowXMLwithMemMgr1(ParseException, XMLExcepts::Parser_Quantifier1, fString, fMemoryManager);
            }

            max = min;

            if (ch == chComma) {

                if (fOffset >= fStringLen) {
                    ThrowXMLwithMemMgr1(ParseException, XMLExcepts::Parser_Quantifier3, fString, fMemoryManager);
                }
                else if ((ch = fString[fOffset++]) >= chDigit_0 && ch <= chDigit_9) {

                    max = ch - chDigit_0;
                    while (fOffset < fStringLen
                           && (ch = fString[fOffset++]) >= chDigit_0
                           && ch <= chDigit_9) {

                        max = max*10 + ch - chDigit_0;
                    }

                    if (max < 0)
                        ThrowXMLwithMemMgr1(ParseException, XMLExcepts::Parser_Quantifier5, fString, fMemoryManager);
                    else if (min > max)
                        ThrowXMLwithMemMgr1(ParseException, XMLExcepts::Parser_Quantifier4, fString, fMemoryManager);
                }
                else {
                    max = -1;
                }
            }

            if (ch != chCloseCurly)  {
                ThrowXMLwithMemMgr1(ParseException, XMLExcepts::Parser_Quantifier2, fString, fMemoryManager);
            }

            if (checkQuestion(fOffset)) {

                tok = fTokenFactory->createClosure(tok, true);
                fOffset++;
            }
            else {
                tok = fTokenFactory->createClosure(tok);
            }

            tok->setMin(min);
            tok->setMax(max);
            processNext();
        }
        break;
    default:
        break;
    }

    return tok;
}


Token* RegxParser::parseAtom() {

    Token* tok = 0;

    switch(fState) {

    case REGX_T_LPAREN:
        return processParen();
    case REGX_T_DOT:
        processNext();
        tok = fTokenFactory->getDot();
        break;
    case REGX_T_CARET:
        return processCaret();
    case REGX_T_DOLLAR:
        return processDollar();
    case REGX_T_LBRACKET:
        return parseCharacterClass(true);
    case REGX_T_BACKSOLIDUS:
        switch(fCharData) {

        case chLatin_d:
        case chLatin_D:
        case chLatin_w:
        case chLatin_W:
        case chLatin_s:
        case chLatin_S:
        case chLatin_c:
        case chLatin_C:
        case chLatin_i:
        case chLatin_I:
            tok = getTokenForShorthand(fCharData);
            processNext();
            return tok;
        case chDigit_0:
        case chDigit_1:
        case chDigit_2:
        case chDigit_3:
        case chDigit_4:
        case chDigit_5:
        case chDigit_6:
        case chDigit_7:
        case chDigit_8:
        case chDigit_9:
            return processBackReference();
        case chLatin_p:
        case chLatin_P:
            {                
                tok = processBacksolidus_pP(fCharData);
                if (tok == 0) {
                    ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Atom5, fMemoryManager);
                }
            }
            break;
        default:
            {
                XMLInt32 ch = decodeEscaped();
                if (ch < 0x10000) {
                    tok = fTokenFactory->createChar(ch);
                }
                else {

                    XMLCh* surrogateStr = RegxUtil::decomposeToSurrogates(ch, fMemoryManager);
                    ArrayJanitor<XMLCh> janSurrogate(surrogateStr, fMemoryManager);
                    tok = fTokenFactory->createString(surrogateStr);
                }
            }
            break;
        } // end switch

        processNext();
        break;
    case REGX_T_CHAR:
        if (fCharData == chOpenCurly
            || fCharData == chCloseCurly
            || fCharData == chCloseSquare)
            ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Atom4, fMemoryManager);

        tok = fTokenFactory->createChar(fCharData);
        processNext();
        break;
    default:
        ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Atom4, fMemoryManager);
    } //end switch

    return tok;
}


RangeToken* RegxParser::processBacksolidus_pP(const XMLInt32 ch) {

    processNext();

    if (fState != REGX_T_CHAR || fCharData != chOpenCurly)
        ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Atom2, fMemoryManager);

    XMLSize_t nameStart = fOffset;
    int nameEnd = XMLString::indexOf(fString,chCloseCurly,nameStart, fMemoryManager);

    if (nameEnd < 0)
        ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Atom3, fMemoryManager);
    
    fOffset = nameEnd + 1;
    XMLCh* rangeName = (XMLCh*) fMemoryManager->allocate
    (
        (nameEnd - nameStart + 1) * sizeof(XMLCh)
    );//new XMLCh[(nameEnd - nameStart) + 1];
    ArrayJanitor<XMLCh> janRangeName(rangeName, fMemoryManager);
    XMLString::subString(rangeName, fString, nameStart, nameEnd, fMemoryManager);

    return  fTokenFactory->getRange(rangeName, !(ch == chLatin_p));
}

RangeToken* RegxParser::parseCharacterClass(const bool useNRange) {

    setParseContext(regexParserStateInBrackets);
    processNext();

    RangeToken* tok = 0;
    bool isNRange = false;

    if (getState() == REGX_T_CHAR && getCharData() == chCaret) {
        isNRange = true;
        processNext();
    }
    tok = fTokenFactory->createRange();

    parserState type;
    bool firstLoop = true;
    bool wasDecoded;

    while ( (type = getState()) != REGX_T_EOF) {

        wasDecoded = false;

        // single range | from-to-range | subtraction
        if (type == REGX_T_CHAR && getCharData() == chCloseSquare && !firstLoop)
            break;

        XMLInt32 ch = getCharData();
        bool     end = false;

        if (type == REGX_T_BACKSOLIDUS) {

            switch(ch) {
            case chLatin_d:
            case chLatin_D:
            case chLatin_w:
            case chLatin_W:
            case chLatin_s:
            case chLatin_S:
            case chLatin_i:
            case chLatin_I:
            case chLatin_c:
            case chLatin_C:
                {
                    tok->mergeRanges(getTokenForShorthand(ch));
                    end = true;
                }
                break;
            case chLatin_p:
            case chLatin_P:
                {                    
                    RangeToken* tok2 = processBacksolidus_pP(ch);

                    if (tok2 == 0) {
                        ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Atom5, getMemoryManager());
                    }

                    tok->mergeRanges(tok2);
                    end = true;
                }
                break;
            case chDash:
                wasDecoded = true;
                // fall thru to default.
            default:
                ch = decodeEscaped();
            }
        } // end if REGX_T_BACKSOLIDUS
        else if (type == REGX_T_XMLSCHEMA_CC_SUBTRACTION && !firstLoop) {

            if (isNRange)
            {
                tok = RangeToken::complementRanges(tok, fTokenFactory, fMemoryManager);
                isNRange=false;
            }
            RangeToken* rangeTok = parseCharacterClass(false);
            tok->subtractRanges(rangeTok);

            if (getState() != REGX_T_CHAR || getCharData() != chCloseSquare) {
                ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_CC5, getMemoryManager());
            }
            break;
        } // end if REGX_T_XMLSCHEMA...

        processNext();

        if (!end) {

            if (type == REGX_T_CHAR
                && (ch == chOpenSquare
                    || ch == chCloseSquare
                    || (ch == chDash && getCharData() == chCloseSquare && firstLoop))) {
                // if regex = [-] then invalid...
                // '[', ']', '-' not allowed and should be escaped
                XMLCh chStr[] = { ch, chNull };
                ThrowXMLwithMemMgr2(ParseException,XMLExcepts::Parser_CC6, chStr, chStr, getMemoryManager());
            }
            if (ch == chDash && getCharData() == chDash && getState() != REGX_T_BACKSOLIDUS && !wasDecoded) {
                XMLCh chStr[] = { ch, chNull };
                ThrowXMLwithMemMgr2(ParseException,XMLExcepts::Parser_CC6, chStr, chStr, getMemoryManager());
            }

            if (getState() != REGX_T_CHAR || getCharData() != chDash) {
                tok->addRange(ch, ch);
            }
            else {

                processNext();
                if ((type = getState()) == REGX_T_EOF)
                    ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_CC2, getMemoryManager());

                if (type == REGX_T_CHAR && getCharData() == chCloseSquare) {
                    tok->addRange(ch, ch);
                    tok->addRange(chDash, chDash);
                }
                else if (type == REGX_T_XMLSCHEMA_CC_SUBTRACTION) {

                    static const XMLCh dashStr[] = { chDash, chNull};
                    ThrowXMLwithMemMgr2(ParseException, XMLExcepts::Parser_CC6, dashStr, dashStr, getMemoryManager());
                }
                else {

                    XMLInt32 rangeEnd = getCharData();
                    XMLCh rangeEndStr[] = { rangeEnd, chNull };

                    if (type == REGX_T_CHAR) {

                        if (rangeEnd == chOpenSquare
                            || rangeEnd == chCloseSquare
                            || rangeEnd == chDash)
                            // '[', ']', '-' not allowed and should be escaped
                            ThrowXMLwithMemMgr2(ParseException, XMLExcepts::Parser_CC6, rangeEndStr, rangeEndStr, getMemoryManager());
                    }
                    else if (type == REGX_T_BACKSOLIDUS) {
                        rangeEnd = decodeEscaped();
                    }

                    processNext();

                    if (ch > rangeEnd) {
                        XMLCh chStr[] = { ch, chNull };
                        ThrowXMLwithMemMgr2(ParseException,XMLExcepts::Parser_Ope3, rangeEndStr, chStr, getMemoryManager());
                    }

                    tok->addRange(ch, rangeEnd);
                }
            }
        }
        firstLoop = false;
    }

    if (getState() == REGX_T_EOF)
        ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_CC2, getMemoryManager());

    if (isNRange)
    {
        if(useNRange)
            tok->setTokenType(Token::T_NRANGE);
        else
            tok = RangeToken::complementRanges(tok, fTokenFactory, fMemoryManager);
    }

    tok->sortRanges();
    tok->compactRanges();

    // If the case-insensitive option is enabled, we need to
    // have the new RangeToken instance build its internal
    // case-insensitive RangeToken.
    if (RegularExpression::isSet(fOptions, RegularExpression::IGNORE_CASE))
    {
        tok->getCaseInsensitiveToken(fTokenFactory);
    }

    setParseContext(regexParserStateNormal);
    processNext();

    return tok;
}


RangeToken* RegxParser::getTokenForShorthand(const XMLInt32 ch) {

    switch(ch) {
    case chLatin_d:
        return fTokenFactory->getRange(fgUniDecimalDigit);
        //return fTokenFactory->getRange(fgXMLDigit);
    case chLatin_D:
        return fTokenFactory->getRange(fgUniDecimalDigit, true);
        //return fTokenFactory->getRange(fgXMLDigit, true);
    case chLatin_w:
        return fTokenFactory->getRange(fgXMLWord);
    case chLatin_W:
        return fTokenFactory->getRange(fgXMLWord, true);
    case chLatin_s:
        return fTokenFactory->getRange(fgXMLSpace);
    case chLatin_S:
        return fTokenFactory->getRange(fgXMLSpace, true);
    case chLatin_c:
        return fTokenFactory->getRange(fgXMLNameChar);
    case chLatin_C:
        return fTokenFactory->getRange(fgXMLNameChar, true);
    case chLatin_i:
        return fTokenFactory->getRange(fgXMLInitialNameChar);
    case chLatin_I:
        return fTokenFactory->getRange(fgXMLInitialNameChar, true);
//    default:
//        ThrowXMLwithMemMgr(RuntimeException, "Invalid shorthand {0}", chAsString)
    }

    return 0;
}


XMLInt32 RegxParser::decodeEscaped() {

    if (fState != REGX_T_BACKSOLIDUS)
        ThrowXMLwithMemMgr(ParseException,XMLExcepts::Parser_Next1, getMemoryManager());

    XMLInt32 ch = fCharData;

    switch (ch) {
    case chLatin_n:
        ch = chLF;
        break;
    case chLatin_r:
        ch = chCR;
        break;
    case chLatin_t:
        ch = chHTab;
        break;
    case chBackSlash:
    case chPipe:
    case chPeriod:
    case chCaret:
    case chDash:
    case chQuestion:
    case chAsterisk:
    case chPlus:
    case chOpenCurly:
    case chCloseCurly:
    case chOpenParen:
    case chCloseParen:
    case chOpenSquare:
    case chCloseSquare:
    case chDollarSign:
        break;
    default:
    {
        XMLCh chString[] = {chBackSlash, ch, chNull};        
        ThrowXMLwithMemMgr1(ParseException,XMLExcepts::Parser_Process2, chString, getMemoryManager());
    }
    }

    return ch;
}

// ---------------------------------------------------------------------------
//  RegxParser: Helper Methods
// ---------------------------------------------------------------------------
bool RegxParser::checkQuestion(const XMLSize_t off) {

    return ((off < fStringLen) && fString[off] == chQuestion);
}

XERCES_CPP_NAMESPACE_END

/**
  *    End file RegxParser.cpp
  */
