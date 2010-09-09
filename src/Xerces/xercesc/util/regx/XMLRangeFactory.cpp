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
 * $Id: XMLRangeFactory.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/XMLRangeFactory.hpp>
#include <xercesc/internal/CharTypeTables.hpp>
#include <xercesc/util/regx/RegxDefs.hpp>
#include <xercesc/util/regx/TokenFactory.hpp>
#include <xercesc/util/regx/RangeToken.hpp>
#include <xercesc/util/regx/RangeTokenMap.hpp>
#include <xercesc/util/regx/UnicodeRangeFactory.hpp>
#include <xercesc/util/Janitor.hpp>
#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local static functions
// ---------------------------------------------------------------------------
static void setupRange(XMLInt32* const rangeMap,
                       const XMLCh* const theTable,
                       unsigned int startingIndex) {

    const XMLCh* pchCur = theTable;

    // Do the ranges first
    while (*pchCur)
    {
        rangeMap[startingIndex++] = *pchCur++;
    }

    // Skip the range terminator
    pchCur++;

    // And then the singles until we hit its terminator
    while (*pchCur) {

        const XMLCh chSingle = *pchCur++;
        rangeMap[startingIndex++] = chSingle;
        rangeMap[startingIndex++] = chSingle;
    }
}

static unsigned int getTableLen(const XMLCh* const theTable) {

    XMLSize_t rangeLen = XMLString::stringLen(theTable);

    return (unsigned int)(rangeLen + 2*XMLString::stringLen(theTable + rangeLen + 1));
}

// ---------------------------------------------------------------------------
//  XMLRangeFactory: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLRangeFactory::XMLRangeFactory()
{

}

XMLRangeFactory::~XMLRangeFactory() {

}

// ---------------------------------------------------------------------------
//  XMLRangeFactory: Range creation methods
// ---------------------------------------------------------------------------
void XMLRangeFactory::buildRanges(RangeTokenMap *rangeTokMap) {

    if (fRangesCreated)
        return;

    if (!fKeywordsInitialized) {
        initializeKeywordMap(rangeTokMap);
    }

    TokenFactory* tokFactory = rangeTokMap->getTokenFactory();

    // Create space ranges
    unsigned int wsTblLen = getTableLen(gWhitespaceChars);
    RangeToken* tok = tokFactory->createRange();
    XMLInt32* wsRange = (XMLInt32*) XMLPlatformUtils::fgMemoryManager->allocate
    (
        wsTblLen * sizeof(XMLInt32)
    );//new XMLInt32[wsTblLen];

    tok->setRangeValues(wsRange, wsTblLen);
    setupRange(wsRange, gWhitespaceChars, 0);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgXMLSpace, tok);

    tok = RangeToken::complementRanges(tok, tokFactory);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgXMLSpace, tok , true);

    // Create digits ranges
    tok = tokFactory->createRange();
    unsigned int digitTblLen = getTableLen(gDigitChars);
    XMLInt32* digitRange = (XMLInt32*) XMLPlatformUtils::fgMemoryManager->allocate
    (
        digitTblLen * sizeof(XMLInt32)
    );//new XMLInt32[digitTblLen];

    tok->setRangeValues(digitRange, digitTblLen);
    setupRange(digitRange, gDigitChars, 0);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgXMLDigit, tok);

    tok = RangeToken::complementRanges(tok, tokFactory);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgXMLDigit, tok , true);

    // Build word ranges
    unsigned int baseTblLen = getTableLen(gBaseChars);
    unsigned int ideoTblLen = getTableLen(gIdeographicChars);
    unsigned int wordRangeLen = baseTblLen + ideoTblLen + digitTblLen;
    XMLInt32* wordRange = (XMLInt32*) XMLPlatformUtils::fgMemoryManager->allocate
    (
        wordRangeLen * sizeof(XMLInt32)
    );//new XMLInt32[wordRangeLen];
    ArrayJanitor<XMLInt32> janWordRange(wordRange, XMLPlatformUtils::fgMemoryManager);

    setupRange(wordRange, gBaseChars, 0);
    setupRange(wordRange, gIdeographicChars, baseTblLen);
    memcpy(wordRange + baseTblLen + ideoTblLen, digitRange, digitTblLen * sizeof(XMLInt32));

    // Create NameChar ranges
    tok = tokFactory->createRange();
    unsigned int combTblLen = getTableLen(gCombiningChars);
    unsigned int extTblLen = getTableLen(gExtenderChars);
    unsigned int nameTblLen = wordRangeLen + combTblLen + extTblLen;
    XMLInt32* nameRange = (XMLInt32*) XMLPlatformUtils::fgMemoryManager->allocate
    (
        (nameTblLen + 8) * sizeof(XMLInt32)
    );//new XMLInt32[nameTblLen + 8];

    tok->setRangeValues(nameRange, nameTblLen + 8);
    memcpy(nameRange, wordRange, wordRangeLen * sizeof(XMLInt32));
    setupRange(nameRange, gCombiningChars, wordRangeLen);
    setupRange(nameRange, gExtenderChars, wordRangeLen + combTblLen);
    nameRange[nameTblLen++] = chDash;
    nameRange[nameTblLen++] = chDash;
    nameRange[nameTblLen++] = chColon;
    nameRange[nameTblLen++] = chColon;
    nameRange[nameTblLen++] = chPeriod;
    nameRange[nameTblLen++] = chPeriod;
    nameRange[nameTblLen++] = chUnderscore;
    nameRange[nameTblLen++] = chUnderscore;
    tok->sortRanges();
    tok->compactRanges();
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgXMLNameChar, tok);

    tok = RangeToken::complementRanges(tok, tokFactory);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgXMLNameChar, tok , true);

    // Create initialNameChar ranges
    tok = tokFactory->createRange();
    unsigned int initialNameTblLen = baseTblLen + ideoTblLen;
    XMLInt32* initialNameRange = (XMLInt32*) XMLPlatformUtils::fgMemoryManager->allocate
    (
        (initialNameTblLen + 4) * sizeof(XMLInt32)
    );//new XMLInt32[initialNameTblLen + 4];

    tok->setRangeValues(initialNameRange, initialNameTblLen + 4);
    memcpy(initialNameRange, wordRange, initialNameTblLen * sizeof(XMLInt32));
    initialNameRange[initialNameTblLen++] = chColon;
    initialNameRange[initialNameTblLen++] = chColon;
    initialNameRange[initialNameTblLen++] = chUnderscore;
    initialNameRange[initialNameTblLen++] = chUnderscore;
    tok->sortRanges();
    tok->compactRanges();
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgXMLInitialNameChar, tok);

    tok = RangeToken::complementRanges(tok, tokFactory);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgXMLInitialNameChar, tok , true);

    // Create word range
    // \w = [#x0000-#x10FFFF]-[\p{P}\p{Z}\p{C}] (all characters except the set of "punctuation", "separator" and "other" characters) 
    tok = tokFactory->createRange();
    for(int i=0; i<=0xFFFF; i++)
    {
        unsigned short chType=UnicodeRangeFactory::getUniCategory(XMLUniCharacter::getType(i));
        if(chType == UnicodeRangeFactory::CHAR_PUNCTUATION || 
           chType == UnicodeRangeFactory::CHAR_SEPARATOR || 
           chType == UnicodeRangeFactory::CHAR_OTHER)
            tok->addRange(i, i);
    }
    tok->sortRanges();
    tok->compactRanges();
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgXMLWord, tok , true);

    tok = RangeToken::complementRanges(tok, tokFactory);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgXMLWord, tok);


    fRangesCreated = true;
}

// ---------------------------------------------------------------------------
//  XMLRangeFactory: Range creation methods
// ---------------------------------------------------------------------------
void XMLRangeFactory::initializeKeywordMap(RangeTokenMap *rangeTokMap) {

    if (fKeywordsInitialized)
        return;

    rangeTokMap->addKeywordMap(fgXMLSpace, fgXMLCategory);
    rangeTokMap->addKeywordMap(fgXMLDigit, fgXMLCategory);
    rangeTokMap->addKeywordMap(fgXMLWord, fgXMLCategory);
    rangeTokMap->addKeywordMap(fgXMLNameChar, fgXMLCategory);
    rangeTokMap->addKeywordMap(fgXMLInitialNameChar, fgXMLCategory);

    fKeywordsInitialized = true;
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file XMLRangeFactory.cpp
  */
