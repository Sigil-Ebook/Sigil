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
 * $Id: UnicodeRangeFactory.hpp 671870 2008-06-26 12:19:31Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_UNICODERANGEFACTORY_HPP)
#define XERCESC_INCLUDE_GUARD_UNICODERANGEFACTORY_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/RangeFactory.hpp>
#include <xercesc/util/regx/XMLUniCharacter.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT UnicodeRangeFactory: public RangeFactory {

public:
    // -----------------------------------------------------------------------
    //  Public Constants
    // -----------------------------------------------------------------------
    // Unicode categories 
    enum {
        CHAR_LETTER      = XMLUniCharacter::FINAL_PUNCTUATION+1,
        CHAR_MARK,
        CHAR_NUMBER,
        CHAR_SEPARATOR,
        CHAR_OTHER,
        CHAR_PUNCTUATION,
        CHAR_SYMBOL,
        UNICATEGSIZE
    };

    // -----------------------------------------------------------------------
    //  Constructors and operators
    // -----------------------------------------------------------------------
    UnicodeRangeFactory();
    ~UnicodeRangeFactory();

    // -----------------------------------------------------------------------
    //  Initialization methods
    // -----------------------------------------------------------------------
    void initializeKeywordMap(RangeTokenMap *rangeTokMap = 0);

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    static unsigned short getUniCategory(const unsigned short type);

protected:
    // -----------------------------------------------------------------------
    //  Private Helper methods
    // -----------------------------------------------------------------------
    void buildRanges(RangeTokenMap *rangeTokMap = 0);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    UnicodeRangeFactory(const UnicodeRangeFactory&);
    UnicodeRangeFactory& operator=(const UnicodeRangeFactory&);

};

XERCES_CPP_NAMESPACE_END

#endif

/**
  *    End file UnicodeRangeFactory.hpp
  */
