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
 * $Id: XMLUniCharacter.cpp 678879 2008-07-22 20:05:05Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if HAVE_CONFIG_H
#    include <config.h>
#endif

#include <xercesc/util/regx/XMLUniCharacter.hpp>

#if XERCES_USE_TRANSCODER_ICU
   #include <unicode/uchar.h>
#else
   #include <xercesc/util/regx/UniCharTable.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XMLUniCharacter: Public static methods
// ---------------------------------------------------------------------------
unsigned short XMLUniCharacter::getType(const XMLCh ch) {

#if XERCES_USE_TRANSCODER_ICU
    return (unsigned short) u_charType(ch);
#else
    return (unsigned short) fgUniCharsTable[ch];
#endif
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file XMLUniCharacter.cpp
  */

