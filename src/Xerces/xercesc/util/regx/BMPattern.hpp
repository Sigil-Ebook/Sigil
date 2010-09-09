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
 * $Id: BMPattern.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BMPATTERN_HPP)
#define XERCESC_INCLUDE_GUARD_BMPATTERN_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT BMPattern : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * This is the constructor which takes the pattern information. A default
      * shift table size is used.
      *
      * @param  pattern     The pattern to match against.
      *
      * @param  ignoreCase  A flag to indicate whether to ignore case
      *                        matching or not.
      *
      * @param  manager     The configurable memory manager
      */
    BMPattern
    (
        const XMLCh* const pattern
        , bool ignoreCase
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
      * This is the constructor which takes all of the information
      * required to construct a BM pattern object.
      *
      * @param  pattern     The pattern to match against.
      *
      * @param    tableSize    Indicates the size of the shift table.
      *
      * @param  ignoreCase  A flag to indicate whether to ignore case
      *                        matching or not.
      *
      * @param  manager     The configurable memory manager
      */
    BMPattern
    (
        const XMLCh* const pattern
        , int tableSize
        , bool ignoreCase
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    //@}

    /** @name Destructor. */
    //@{

    /**
      * Destructor of BMPattern
      */
    ~BMPattern();

    //@}

    // -----------------------------------------------------------------------
    // Matching functions
    // -----------------------------------------------------------------------
    /** @name Matching Functions */
    //@{

    /**
      *    This method will perform a match of the given content against a
      *    predefined pattern.
      */
    int matches(const XMLCh* const content, XMLSize_t start, XMLSize_t limit) const;

    //@}

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    BMPattern();
    BMPattern(const BMPattern&);
    BMPattern& operator=(const BMPattern&);

        // -----------------------------------------------------------------------
    // This method will perform a case insensitive match
    // -----------------------------------------------------------------------
    bool matchesIgnoreCase(const XMLCh ch1, const XMLCh ch2);

    // -----------------------------------------------------------------------
    // Initialize/Clean up methods
    // -----------------------------------------------------------------------
    void initialize();
    void cleanUp();

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fPattern
    //    fUppercasePattern
    //      This is the pattern to match against, and its upper case form.
    //        
    //  fIgnoreCase
    //      This is an indicator whether cases should be ignored during
    //        matching.
    //
    //  fShiftTable
    //    fShiftTableLen
    //      This is a table of offsets for shifting purposes used by the BM
    //        search algorithm, and its length.
    // -----------------------------------------------------------------------
    bool           fIgnoreCase;
    unsigned int   fShiftTableLen;
    XMLSize_t*     fShiftTable;
    XMLCh*         fPattern;
    XMLCh*         fUppercasePattern;
    MemoryManager* fMemoryManager; 
};

XERCES_CPP_NAMESPACE_END

#endif

/*
 * End of file BMPattern.hpp
 */

