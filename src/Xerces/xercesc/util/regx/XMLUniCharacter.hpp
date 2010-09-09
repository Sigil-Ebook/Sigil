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
 * $Id: XMLUniCharacter.hpp 671870 2008-06-26 12:19:31Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLUNICHARACTER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLUNICHARACTER_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
  * Class for representing unicode characters
  */
class XMLUTIL_EXPORT XMLUniCharacter
{
public:
    // -----------------------------------------------------------------------
    //  Public Constants
    // -----------------------------------------------------------------------
    // Unicode char types
    enum {
        UNASSIGNED              = 0,
        UPPERCASE_LETTER        = 1,
        LOWERCASE_LETTER        = 2,
        TITLECASE_LETTER        = 3,
        MODIFIER_LETTER         = 4,
        OTHER_LETTER            = 5,
        NON_SPACING_MARK        = 6,
        ENCLOSING_MARK          = 7,
        COMBINING_SPACING_MARK  = 8,
        DECIMAL_DIGIT_NUMBER    = 9,
        LETTER_NUMBER           = 10,
        OTHER_NUMBER            = 11,
        SPACE_SEPARATOR         = 12,
        LINE_SEPARATOR          = 13,
        PARAGRAPH_SEPARATOR     = 14,
        CONTROL                 = 15,
        FORMAT                  = 16,
        PRIVATE_USE             = 17,
        SURROGATE               = 18,
        DASH_PUNCTUATION        = 19,
        START_PUNCTUATION       = 20,
        END_PUNCTUATION         = 21,
        CONNECTOR_PUNCTUATION   = 22,
        OTHER_PUNCTUATION       = 23,
        MATH_SYMBOL             = 24,
        CURRENCY_SYMBOL         = 25,
        MODIFIER_SYMBOL         = 26,
        OTHER_SYMBOL            = 27,
        INITIAL_PUNCTUATION     = 28,
        FINAL_PUNCTUATION       = 29
    };

    /** destructor */
    ~XMLUniCharacter() {}

    /* Static methods for getting unicode character type */
    /** @name Getter functions */
    //@{

    /** Gets the unicode type of a given character
      *
      * @param ch The character we want to get its unicode type
      */
    static unsigned short getType(const XMLCh ch);
    //@}

private :

    /** @name Constructors and Destructor */
    //@{
    /** Unimplemented default constructor */
    XMLUniCharacter();
    //@}
};

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file XMLUniCharacter.hpp
  */
