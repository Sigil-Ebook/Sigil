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
 * $Id: BitOps.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BITOPS_HPP)
#define XERCESC_INCLUDE_GUARD_BITOPS_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT BitOps
{
public:
    // -----------------------------------------------------------------------
    //  Public static methods
    // -----------------------------------------------------------------------
    static inline XMLCh swapBytes(const XMLUInt16 toSwap)
    {
        //The mask is required to overcome a compiler error on solaris
        return XMLCh(((toSwap >> 8) | (toSwap << 8)) & 0xFFFF);
    }

    static inline unsigned int swapBytes(const XMLUInt32 toSwap)
    {
        return
        (
            (toSwap >> 24)
            | (toSwap << 24)
            | ((toSwap & 0xFF00) << 8)
            | ((toSwap & 0xFF0000) >> 8)
        );
    }



protected :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators.  (These ought to be private,
    //                                  but that produces spurious compiler warnings
    //                                  on some platforms.)
    // -----------------------------------------------------------------------
    BitOps();
    BitOps(const BitOps&);
    BitOps& operator=(const BitOps&);
};

XERCES_CPP_NAMESPACE_END

#endif
