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
 * $Id: XMLChar.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLCHAR_HPP)
#define XERCESC_INCLUDE_GUARD_XMLCHAR_HPP

#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  This file defines Char and utility that conforms to XML 1.0 and XML 1.1
// ---------------------------------------------------------------------------
// Masks for the fgCharCharsTable1_0 array
const XMLByte   gNCNameCharMask             = 0x1;
const XMLByte   gFirstNameCharMask          = 0x2;
const XMLByte   gNameCharMask               = 0x4;
const XMLByte   gPlainContentCharMask       = 0x8;
const XMLByte   gSpecialStartTagCharMask    = 0x10;
const XMLByte   gControlCharMask            = 0x20;
const XMLByte   gXMLCharMask                = 0x40;
const XMLByte   gWhitespaceCharMask         = 0x80;

// ---------------------------------------------------------------------------
//  This class is for XML 1.0
// ---------------------------------------------------------------------------
class XMLUTIL_EXPORT XMLChar1_0
{
public:
    // -----------------------------------------------------------------------
    //  Public, static methods, check the string
    // -----------------------------------------------------------------------
    static bool isAllSpaces
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    );

    static bool containsWhiteSpace
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    );

    static bool isValidNmtoken
    (
        const   XMLCh*        const    toCheck
      , const   XMLSize_t              count
    );

    static bool isValidName
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    );

    static bool isValidName
    (
        const   XMLCh* const    toCheck
    );

    static bool isValidNCName
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    );

    static bool isValidQName
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    );

    // -----------------------------------------------------------------------
    //  Public, static methods, check the XMLCh
    //  surrogate pair is assumed if second parameter is not null
    // -----------------------------------------------------------------------
    static bool isXMLLetter(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isFirstNameChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isNameChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isPlainContentChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isSpecialStartTagChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isXMLChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isWhitespace(const XMLCh toCheck);
    static bool isWhitespace(const XMLCh toCheck, const XMLCh toCheck2);
    static bool isControlChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);

    static bool isPublicIdChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isFirstNCNameChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isNCNameChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);

    // -----------------------------------------------------------------------
    //  Special Non-conformant Public, static methods
    // -----------------------------------------------------------------------
    /**
      * Return true if NEL (0x85) and LSEP (0x2028) to be treated as white space char.
      */
    static bool isNELRecognized();

    /**
      * Method to enable NEL (0x85) and LSEP (0x2028) to be treated as white space char.
      */
    static void enableNELWS();

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLChar1_0();

    // -----------------------------------------------------------------------
    //  Static data members
    //
    //  fgCharCharsTable1_0
    //      The character characteristics table. Bits in each byte, represent
    //      the characteristics of each character. It is generated via some
    //      code and then hard coded into the cpp file for speed.
    //
    //  fNEL
    //      Flag to represents whether NEL and LSEP newline recognition is enabled
    //      or disabled
    // -----------------------------------------------------------------------
    static XMLByte  fgCharCharsTable1_0[0x10000];
    static bool     enableNEL;

    friend class XMLReader;
};


// ---------------------------------------------------------------------------
//  XMLReader: Public, static methods
// ---------------------------------------------------------------------------
inline bool XMLChar1_0::isXMLLetter(const XMLCh toCheck, const XMLCh toCheck2)
{
    // An XML letter is a FirstNameChar minus ':' and '_'.
    if (!toCheck2) {
        return (((fgCharCharsTable1_0[toCheck] & gFirstNameCharMask) != 0)
                && (toCheck != chColon) && (toCheck != chUnderscore));
    }
    return false;
}

inline bool XMLChar1_0::isFirstNameChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_0[toCheck] & gFirstNameCharMask) != 0);
    return false;
}

inline bool XMLChar1_0::isFirstNCNameChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2) {
        return (((fgCharCharsTable1_0[toCheck] & gFirstNameCharMask) != 0) && (toCheck != chColon));
    }

    return false;
}

inline bool XMLChar1_0::isNameChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_0[toCheck] & gNameCharMask) != 0);
    return false;
}

inline bool XMLChar1_0::isNCNameChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_0[toCheck] & gNCNameCharMask) != 0);
    return false;
}

inline bool XMLChar1_0::isPlainContentChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_0[toCheck] & gPlainContentCharMask) != 0);
    else {
        if ((toCheck >= 0xD800) && (toCheck <= 0xDBFF))
           if ((toCheck2 >= 0xDC00) && (toCheck2 <= 0xDFFF))
               return true;
    }
    return false;
}


inline bool XMLChar1_0::isSpecialStartTagChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_0[toCheck] & gSpecialStartTagCharMask) != 0);
    return false;
}

inline bool XMLChar1_0::isXMLChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_0[toCheck] & gXMLCharMask) != 0);
    else {
        if ((toCheck >= 0xD800) && (toCheck <= 0xDBFF))
           if ((toCheck2 >= 0xDC00) && (toCheck2 <= 0xDFFF))
               return true;
    }
    return false;
}

inline bool XMLChar1_0::isWhitespace(const XMLCh toCheck)
{
    return ((fgCharCharsTable1_0[toCheck] & gWhitespaceCharMask) != 0);
}

inline bool XMLChar1_0::isWhitespace(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_0[toCheck] & gWhitespaceCharMask) != 0);
    return false;
}

inline bool XMLChar1_0::isControlChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_0[toCheck] & gControlCharMask) != 0);
    return false;
}

inline bool XMLChar1_0::isNELRecognized() {

    return enableNEL;
}


// ---------------------------------------------------------------------------
//  This class is for XML 1.1
// ---------------------------------------------------------------------------
class XMLUTIL_EXPORT XMLChar1_1
{
public:
    // -----------------------------------------------------------------------
    //  Public, static methods, check the string
    // -----------------------------------------------------------------------
    static bool isAllSpaces
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    );

    static bool containsWhiteSpace
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    );

    static bool isValidNmtoken
    (
        const   XMLCh*        const    toCheck
      , const   XMLSize_t              count
    );

    static bool isValidName
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    );

    static bool isValidName
    (
        const   XMLCh* const    toCheck
    );

    static bool isValidNCName
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    );

    static bool isValidQName
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    );

    // -----------------------------------------------------------------------
    //  Public, static methods, check the XMLCh
    // -----------------------------------------------------------------------
    static bool isXMLLetter(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isFirstNameChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isNameChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isPlainContentChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isSpecialStartTagChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isXMLChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isWhitespace(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isControlChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);

    static bool isPublicIdChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isFirstNCNameChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);
    static bool isNCNameChar(const XMLCh toCheck, const XMLCh toCheck2 = 0);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLChar1_1();

    // -----------------------------------------------------------------------
    //  Static data members
    //
    //  fgCharCharsTable1_1
    //      The character characteristics table. Bits in each byte, represent
    //      the characteristics of each character. It is generated via some
    //      code and then hard coded into the cpp file for speed.
    //
    // -----------------------------------------------------------------------
    static XMLByte  fgCharCharsTable1_1[0x10000];

    friend class XMLReader;
};


// ---------------------------------------------------------------------------
//  XMLReader: Public, static methods
// ---------------------------------------------------------------------------
inline bool XMLChar1_1::isXMLLetter(const XMLCh toCheck, const XMLCh toCheck2)
{
    /** XML 1.1 does not define a letter, so we use the 1.0 definition */
    return XMLChar1_0::isXMLLetter(toCheck, toCheck2);
}

inline bool XMLChar1_1::isFirstNameChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_1[toCheck] & gFirstNameCharMask) != 0);
    else {
        if ((toCheck >= 0xD800) && (toCheck <= 0xDB7F))
           if ((toCheck2 >= 0xDC00) && (toCheck2 <= 0xDFFF))
               return true;
    }
    return false;
}

inline bool XMLChar1_1::isFirstNCNameChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2) {
        return (((fgCharCharsTable1_1[toCheck] & gFirstNameCharMask) != 0) && (toCheck != chColon));
    }
    else {
        if ((toCheck >= 0xD800) && (toCheck <= 0xDB7F))
           if ((toCheck2 >= 0xDC00) && (toCheck2 <= 0xDFFF))
               return true;
    }
    return false;
}

inline bool XMLChar1_1::isNameChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_1[toCheck] & gNameCharMask) != 0);
    else {
        if ((toCheck >= 0xD800) && (toCheck <= 0xDB7F))
           if ((toCheck2 >= 0xDC00) && (toCheck2 <= 0xDFFF))
               return true;
    }
    return false;
}

inline bool XMLChar1_1::isNCNameChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_1[toCheck] & gNCNameCharMask) != 0);
    else {
        if ((toCheck >= 0xD800) && (toCheck <= 0xDB7F))
           if ((toCheck2 >= 0xDC00) && (toCheck2 <= 0xDFFF))
               return true;
    }
    return false;
}

inline bool XMLChar1_1::isPlainContentChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_1[toCheck] & gPlainContentCharMask) != 0);
    else {
        if ((toCheck >= 0xD800) && (toCheck <= 0xDBFF))
           if ((toCheck2 >= 0xDC00) && (toCheck2 <= 0xDFFF))
               return true;
    }
    return false;
}


inline bool XMLChar1_1::isSpecialStartTagChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_1[toCheck] & gSpecialStartTagCharMask) != 0);
    return false;
}

inline bool XMLChar1_1::isXMLChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_1[toCheck] & gXMLCharMask) != 0);
    else {
        if ((toCheck >= 0xD800) && (toCheck <= 0xDBFF))
           if ((toCheck2 >= 0xDC00) && (toCheck2 <= 0xDFFF))
               return true;
    }
    return false;
}

inline bool XMLChar1_1::isWhitespace(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_1[toCheck] & gWhitespaceCharMask) != 0);
    return false;
}

inline bool XMLChar1_1::isControlChar(const XMLCh toCheck, const XMLCh toCheck2)
{
    if (!toCheck2)
        return ((fgCharCharsTable1_1[toCheck] & gControlCharMask) != 0);
    return false;
}


XERCES_CPP_NAMESPACE_END

#endif
