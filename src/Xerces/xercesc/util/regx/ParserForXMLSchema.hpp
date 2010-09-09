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
 * $Id: ParserForXMLSchema.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_PARSERFORXMLSCHEMA_HPP)
#define XERCESC_INCLUDE_GUARD_PARSERFORXMLSCHEMA_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/RegxParser.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Forward Declaration
// ---------------------------------------------------------------------------
class Token;
class RangeToken;

class XMLUTIL_EXPORT ParserForXMLSchema : public RegxParser {
public:
    // -----------------------------------------------------------------------
    //  Public Constructors and Destructor
    // -----------------------------------------------------------------------
    ParserForXMLSchema(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~ParserForXMLSchema();

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

protected:
    // -----------------------------------------------------------------------
    //  Parsing/Processing methods
    // -----------------------------------------------------------------------
    Token*      processCaret();
    Token*      processDollar();
    Token*      processStar(Token* const tok);
    Token*      processPlus(Token* const tok);
    Token*      processQuestion(Token* const tok);
    Token*      processParen();
    Token*      processBackReference();

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    bool checkQuestion(const XMLSize_t off);
    XMLInt32 decodeEscaped();

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ParserForXMLSchema(const ParserForXMLSchema&);
    ParserForXMLSchema& operator=(const ParserForXMLSchema&);

    // -----------------------------------------------------------------------
    //  Private data members
    // -----------------------------------------------------------------------
};

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file ParserForXMLSchema.hpp
  */
