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
 * $Id: RegxUtil.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/RegxUtil.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

XMLCh* RegxUtil::decomposeToSurrogates(XMLInt32 ch,
                                       MemoryManager* const manager) {

    XMLCh* pszStr = (XMLCh*) manager->allocate(3 *  sizeof(XMLCh));//new XMLCh[3];

    decomposeToSurrogates(ch, pszStr[0], pszStr[1]);

    pszStr[2] = chNull;

    return pszStr;
}


XMLCh* RegxUtil::stripExtendedComment(const XMLCh* const expression,
                                      MemoryManager* const manager) {

    XMLCh* buffer = (manager) ? XMLString::replicate(expression, manager)
                              : XMLString::replicate(expression);

    if (buffer)
    {
        const XMLCh* inPtr = expression;
        XMLCh* outPtr = buffer;

        while (*inPtr) {

            XMLCh ch = *inPtr++;

            if (ch == chFF || ch == chCR || ch == chLF
                || ch == chSpace || ch == chHTab) {
                continue;
            }

            // Skips chracters between '#' and a line end.
            if (ch == chPound) {

                while (*inPtr) {

                    ch = *inPtr++;
                    if (ch == chLF || ch == chCR)
                        break;
                }

                continue;
            }

            if (ch == chBackSlash && *inPtr) {

                if ((ch = *inPtr++) == chPound || ch == chHTab || ch == chLF
                    || ch == chFF || ch == chCR || ch == chSpace) {
                    *outPtr++ = ch;
                }
                else { // Other escaped character.

                    *outPtr++ = chBackSlash;
                    *outPtr++ = ch;
                }
            }
            else { // As is.
                *outPtr++ = ch;
            }
        }

        *outPtr = chNull; // null terminate
    }

    return buffer;
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file RegxUtil.cpp
  */
