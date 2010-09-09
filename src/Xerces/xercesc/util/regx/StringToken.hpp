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
 * $Id: StringToken.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_STRINGTOKEN_HPP)
#define XERCESC_INCLUDE_GUARD_STRINGTOKEN_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/Token.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT StringToken : public Token {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    StringToken(const tokType tkType,
                const XMLCh* const literal,
                const int refNo,
                MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~StringToken();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    int getReferenceNo() const;
    const XMLCh* getString() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setString(const XMLCh* const literal);

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    StringToken(const StringToken&);
    StringToken& operator=(const StringToken&);

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
    int    fRefNo;
    XMLCh* fString;
    MemoryManager* fMemoryManager;
};


// ---------------------------------------------------------------------------
//  StringToken: getter methods
// ---------------------------------------------------------------------------
inline int StringToken::getReferenceNo() const {

    return fRefNo;
}

inline const XMLCh* StringToken::getString() const {

    return fString;
}

// ---------------------------------------------------------------------------
//  StringToken: Setter methods
// ---------------------------------------------------------------------------
inline void StringToken::setString(const XMLCh* const literal) {

    fMemoryManager->deallocate(fString);//delete [] fString;
    fString = 0;
    fString = XMLString::replicate(literal, fMemoryManager);
}

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file StringToken.hpp
  */
