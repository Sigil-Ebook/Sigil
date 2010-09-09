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
 * $Id: BMPattern.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/BMPattern.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/framework/MemoryManager.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  BMPattern: Constructors
// ---------------------------------------------------------------------------

typedef JanitorMemFunCall<BMPattern>    CleanupType;

BMPattern::BMPattern( const XMLCh*         const pattern
                    ,       bool                 ignoreCase
                    ,       MemoryManager* const manager) :

    fIgnoreCase(ignoreCase)
    , fShiftTableLen(256)
    , fShiftTable(0)
    , fPattern(0)
    , fUppercasePattern(0)
    , fMemoryManager(manager)
{
    CleanupType cleanup(this, &BMPattern::cleanUp);

    try {
        fPattern = XMLString::replicate(pattern, fMemoryManager);
        initialize();
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

BMPattern::BMPattern( const XMLCh*         const pattern
                    ,       int                  tableSize
                    ,       bool                 ignoreCase
                    ,       MemoryManager* const manager) :

    fIgnoreCase(ignoreCase)
    , fShiftTableLen(tableSize)
    , fShiftTable(0)
    , fPattern(0)
    , fUppercasePattern(0)
    , fMemoryManager(manager)
{
    CleanupType cleanup(this, &BMPattern::cleanUp);

    try {
        fPattern = XMLString::replicate(pattern, fMemoryManager);
        initialize();
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

BMPattern::~BMPattern() {

    cleanUp();
}

// ---------------------------------------------------------------------------
//  BMPattern: matches methods
// ---------------------------------------------------------------------------
int BMPattern::matches(const XMLCh* const content, XMLSize_t start, XMLSize_t limit) const {

    const XMLSize_t patternLen = XMLString::stringLen(fPattern);
    // Uppercase Content
    XMLCh* ucContent = 0;

    if (patternLen == 0)
        return (int)start;

    if (fIgnoreCase) {
        
        ucContent = XMLString::replicate(content, fMemoryManager);
        XMLString::upperCase(ucContent);
    }

    ArrayJanitor<XMLCh> janUCContent(ucContent, fMemoryManager);

    XMLSize_t index = start + patternLen;

    while (index <= limit) {

        XMLSize_t patternIndex = patternLen;
        XMLSize_t nIndex = index + 1;
        XMLCh ch = 0;

        while (patternIndex > 0) {

            ch = content[--index];

            if (ch != fPattern[--patternIndex]) {

                // No match, so we will break. But first we have
                // to check the ignore case flag. If it is set, then
                // we try to match with the case ignored
                if (!fIgnoreCase ||
                    (fUppercasePattern[patternIndex] != ucContent[index]))
                    break;
            }

            if (patternIndex == 0)
                return (int)index;
        }

        index += fShiftTable[ch % fShiftTableLen] + 1;

        if (index < nIndex)
            index = nIndex;
    }

    return -1;
}

// ---------------------------------------------------------------------------
//  BMPattern: private helpers methods
// ---------------------------------------------------------------------------
void BMPattern::initialize() {

    const XMLSize_t patternLen = XMLString::stringLen(fPattern);
    XMLCh* lowercasePattern = 0;

    fShiftTable = (XMLSize_t*) fMemoryManager->allocate(fShiftTableLen*sizeof(XMLSize_t)); //new XMLSize_t[fShiftTableLen];

    if (fIgnoreCase) {

        fUppercasePattern = XMLString::replicate(fPattern, fMemoryManager);
        lowercasePattern = XMLString::replicate(fPattern, fMemoryManager);
        XMLString::upperCase(fUppercasePattern);
        XMLString::lowerCase(lowercasePattern);
    }

    ArrayJanitor<XMLCh> janLowercase(lowercasePattern, fMemoryManager);

    for (unsigned int i=0; i< fShiftTableLen; i++)
        fShiftTable[i] = patternLen;

    for (unsigned int k=0; k< patternLen; k++) {

        XMLCh      ch = fPattern[k];
        XMLSize_t diff = patternLen - k - 1;
        int          index = ch % fShiftTableLen;

        if (diff < fShiftTable[index])
            fShiftTable[index] = diff;

        if (fIgnoreCase) {

            for (int j=0; j< 2; j++) {

                ch = (j == 0) ? fUppercasePattern[k] : lowercasePattern[k];
                index = ch % fShiftTableLen;

                if (diff < fShiftTable[index])
                    fShiftTable[index] = diff;
            }
        }
    }
}

// ---------------------------------------------------------------------------
//  BMPattern: Cleanup
// ---------------------------------------------------------------------------
void BMPattern::cleanUp() {

    fMemoryManager->deallocate(fPattern);//delete [] fPattern;
    fMemoryManager->deallocate(fUppercasePattern);//delete [] fUppercasePattern;
    fMemoryManager->deallocate(fShiftTable);
}

XERCES_CPP_NAMESPACE_END

/**
  *    End of file BMPattern.cpp
  */
