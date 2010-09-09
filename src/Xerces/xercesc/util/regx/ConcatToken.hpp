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
 * $Id: ConcatToken.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_CONCATTOKEN_HPP)
#define XERCESC_INCLUDE_GUARD_CONCATTOKEN_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/Token.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT ConcatToken : public Token {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    ConcatToken(Token* const tok1, Token* const tok2
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~ConcatToken();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    Token* getChild(const XMLSize_t index) const;
    XMLSize_t size() const;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ConcatToken(const ConcatToken&);
    ConcatToken& operator=(const ConcatToken&);

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    Token* fChild1;
    Token* fChild2;
};


// ---------------------------------------------------------------------------
//  StringToken: getter methods
// ---------------------------------------------------------------------------
inline XMLSize_t ConcatToken::size() const {

    return 2;
}

inline Token* ConcatToken::getChild(const XMLSize_t index) const {

    return index == 0 ? fChild1 : fChild2;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file ConcatToken.hpp
  */
