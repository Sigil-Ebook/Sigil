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
 * $Id: ASCIIRangeFactory.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/ASCIIRangeFactory.hpp>
#include <xercesc/util/regx/RegxDefs.hpp>
#include <xercesc/util/regx/TokenFactory.hpp>
#include <xercesc/util/regx/RangeToken.hpp>
#include <xercesc/util/regx/RangeTokenMap.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  ASCIIRangeFactory: Constructors and Destructor
// ---------------------------------------------------------------------------
ASCIIRangeFactory::ASCIIRangeFactory()
{
}

ASCIIRangeFactory::~ASCIIRangeFactory() {

}

// ---------------------------------------------------------------------------
//  ASCIIRangeFactory: Range creation methods
// ---------------------------------------------------------------------------
void ASCIIRangeFactory::buildRanges(RangeTokenMap *rangeTokMap) {

    if (fRangesCreated)
        return;

    if (!fKeywordsInitialized) {
        initializeKeywordMap(rangeTokMap);
    }

    TokenFactory* tokFactory = rangeTokMap->getTokenFactory();

    // Create space ranges
    RangeToken* tok = tokFactory->createRange();
    tok->addRange(chHTab, chHTab);
    tok->addRange(chLF, chLF);
    tok->addRange(chFF, chFF);
    tok->addRange(chCR, chCR);
    tok->addRange(chSpace, chSpace);

    // Build the internal map.
    tok->createMap();

    rangeTokMap->setRangeToken(fgASCIISpace, tok);

    tok = RangeToken::complementRanges(tok, tokFactory);

    // Build the internal map.
    tok->createMap();

    rangeTokMap->setRangeToken(fgASCIISpace, tok , true);

    // Create digits ranges
    tok = tokFactory->createRange();
    tok->addRange(chDigit_0, chDigit_9);

    // Build the internal map.
    tok->createMap();

    rangeTokMap->setRangeToken(fgASCIIDigit, tok);

    tok = RangeToken::complementRanges(tok, tokFactory);

    // Build the internal map.
    tok->createMap();

    rangeTokMap->setRangeToken(fgASCIIDigit, tok , true);

    // Create word ranges
    tok = tokFactory->createRange();
    tok->addRange(chDigit_0, chDigit_9);
    tok->addRange(chLatin_A, chLatin_Z);
    tok->addRange(chUnderscore, chUnderscore);
    tok->addRange(chLatin_a, chLatin_z);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgASCIIWord, tok);

    tok = RangeToken::complementRanges(tok, tokFactory);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgASCIIWord, tok , true);

    // Create xdigit ranges
    tok = tokFactory->createRange();
    tok->addRange(chDigit_0, chDigit_9);
    tok->addRange(chLatin_A, chLatin_F);
    tok->addRange(chLatin_a, chLatin_a);
    // Build the internal map.
    tok->createMap();

    rangeTokMap->setRangeToken(fgASCIIXDigit, tok);

    tok = RangeToken::complementRanges(tok, tokFactory);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgASCIIXDigit, tok , true);

    // Create ascii ranges
    tok = tokFactory->createRange();
    tok->addRange(0x00, 0x7F);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgASCII, tok);

    tok = RangeToken::complementRanges(tok, tokFactory);
    // Build the internal map.
    tok->createMap();
    rangeTokMap->setRangeToken(fgASCII, tok , true);

    fRangesCreated = true;
}

// ---------------------------------------------------------------------------
//  ASCIIRangeFactory: Range creation methods
// ---------------------------------------------------------------------------
void ASCIIRangeFactory::initializeKeywordMap(RangeTokenMap *rangeTokMap) {

    if (fKeywordsInitialized)
        return;

    rangeTokMap->addKeywordMap(fgASCIISpace, fgASCIICategory);
    rangeTokMap->addKeywordMap(fgASCIIDigit, fgASCIICategory);
    rangeTokMap->addKeywordMap(fgASCIIWord, fgASCIICategory);
    rangeTokMap->addKeywordMap(fgASCIIXDigit, fgASCIICategory);
    rangeTokMap->addKeywordMap(fgASCII, fgASCIICategory);

    fKeywordsInitialized = true;
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file ASCIIRangeFactory.cpp
  */
