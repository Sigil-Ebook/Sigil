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
 * $Id: UnionToken.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_UNIONTOKEN_HPP)
#define XERCESC_INCLUDE_GUARD_UNIONTOKEN_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/Token.hpp>
#include <xercesc/util/RefVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT UnionToken : public Token {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    UnionToken(const tokType tkType
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~UnionToken();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    XMLSize_t size() const;
    Token* getChild(const XMLSize_t index) const;

    // -----------------------------------------------------------------------
    //  Children manipulation methods
    // -----------------------------------------------------------------------
    void addChild(Token* const child, TokenFactory* const tokFactory);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    UnionToken(const UnionToken&);
    UnionToken& operator=(const UnionToken&);

    // -----------------------------------------------------------------------
    //  Private Constants
    // -----------------------------------------------------------------------
    static const unsigned short INITIALSIZE;

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    RefVectorOf<Token>* fChildren;
};


// ---------------------------------------------------------------------------
//  UnionToken: getter methods
// ---------------------------------------------------------------------------
inline XMLSize_t UnionToken::size() const {

    return fChildren == 0 ? 0 : fChildren->size();
}

inline Token* UnionToken::getChild(const XMLSize_t index) const {

    return fChildren->elementAt(index);
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file UnionToken.hpp
  */
