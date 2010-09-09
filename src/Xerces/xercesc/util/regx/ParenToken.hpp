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
 * $Id: ParenToken.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_PARENTOKEN_HPP)
#define XERCESC_INCLUDE_GUARD_PARENTOKEN_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/Token.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT ParenToken : public Token {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    ParenToken(const tokType tkType, Token* const tok,
               const int noParen, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~ParenToken();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    XMLSize_t size() const;
    int getNoParen() const;
    Token* getChild(const XMLSize_t index) const;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ParenToken(const ParenToken&);
    ParenToken& operator=(const ParenToken&);

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    int    fNoParen;
    Token* fChild;
};


// ---------------------------------------------------------------------------
//  ParenToken: getter methods
// ---------------------------------------------------------------------------
inline XMLSize_t ParenToken::size() const {

    return 1;
}

inline int ParenToken::getNoParen() const {

    return fNoParen;
}

inline Token* ParenToken::getChild(const XMLSize_t) const {

    return fChild;
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file ParenToken.hpp
  */
