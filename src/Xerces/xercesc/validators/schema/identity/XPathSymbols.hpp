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
 * $Id: XPathSymbols.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XPATHSYMBOLS_HPP)
#define XERCESC_INCLUDE_GUARD_XPATHSYMBOLS_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/*
 * Collection of symbols used to parse a Schema Grammar
 */

class VALIDATORS_EXPORT XPathSymbols
{
public :
    // -----------------------------------------------------------------------
    // Constant data
    // -----------------------------------------------------------------------
    static const XMLCh fgSYMBOL_AND[];
    static const XMLCh fgSYMBOL_OR[];
    static const XMLCh fgSYMBOL_MOD[];
    static const XMLCh fgSYMBOL_DIV[];
    static const XMLCh fgSYMBOL_COMMENT[];
    static const XMLCh fgSYMBOL_TEXT[];
    static const XMLCh fgSYMBOL_PI[];
    static const XMLCh fgSYMBOL_NODE[];
    static const XMLCh fgSYMBOL_ANCESTOR[];
    static const XMLCh fgSYMBOL_ANCESTOR_OR_SELF[];
    static const XMLCh fgSYMBOL_ATTRIBUTE[];
    static const XMLCh fgSYMBOL_CHILD[];
    static const XMLCh fgSYMBOL_DESCENDANT[];
    static const XMLCh fgSYMBOL_DESCENDANT_OR_SELF[];
    static const XMLCh fgSYMBOL_FOLLOWING[];
    static const XMLCh fgSYMBOL_FOLLOWING_SIBLING[];
    static const XMLCh fgSYMBOL_NAMESPACE[];
    static const XMLCh fgSYMBOL_PARENT[];
    static const XMLCh fgSYMBOL_PRECEDING[];
    static const XMLCh fgSYMBOL_PRECEDING_SIBLING[];
    static const XMLCh fgSYMBOL_SELF[];

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XPathSymbols();
};

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file XPathSymbols.hpp
  */

