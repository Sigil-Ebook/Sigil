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
 * $Id: ConcatToken.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/ConcatToken.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Token: Constructors and Destructors
// ---------------------------------------------------------------------------
ConcatToken::ConcatToken(Token* const tok1, Token* const tok2, MemoryManager* const manager)
    : Token(Token::T_CONCAT, manager)
    , fChild1(tok1)
    , fChild2(tok2)
{

}


ConcatToken::~ConcatToken() {

}

XERCES_CPP_NAMESPACE_END

/**
  *    End of file ConcatToken.cpp
  */
