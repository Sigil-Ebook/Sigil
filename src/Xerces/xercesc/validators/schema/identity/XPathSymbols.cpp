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
 * $Id: XPathSymbols.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/validators/schema/identity/XPathSymbols.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  SchemaSymbols: Static data
// ---------------------------------------------------------------------------
const XMLCh XPathSymbols::fgSYMBOL_AND[] =
{
    chLatin_a, chLatin_n, chLatin_d, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_OR[] =
{
    chLatin_o, chLatin_r, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_MOD[] =
{
    chLatin_m, chLatin_o, chLatin_d, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_DIV[] =
{
    chLatin_d, chLatin_i, chLatin_v, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_COMMENT[] =
{
    chLatin_c, chLatin_o, chLatin_m, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_TEXT[] =
{
    chLatin_t, chLatin_e, chLatin_x, chLatin_t, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_PI[] =
{
    chLatin_p, chLatin_r, chLatin_o, chLatin_c, chLatin_e, chLatin_s, chLatin_s,
    chLatin_i, chLatin_n, chLatin_g, chDash, chLatin_i, chLatin_n, chLatin_s, chLatin_t,
    chLatin_r, chLatin_u, chLatin_c, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_NODE[] =
{
    chLatin_n, chLatin_o, chLatin_d, chLatin_e, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_ANCESTOR[] =
{
    chLatin_a, chLatin_n, chLatin_c, chLatin_e, chLatin_s, chLatin_t, chLatin_o,
    chLatin_r, chNull
};


const XMLCh XPathSymbols::fgSYMBOL_ANCESTOR_OR_SELF[] =
{
    chLatin_a, chLatin_n, chLatin_c, chLatin_e, chLatin_s, chLatin_t, chLatin_o,
    chLatin_r, chDash, chLatin_o, chLatin_r, chDash, chLatin_s, chLatin_e,
    chLatin_l, chLatin_f, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_ATTRIBUTE[] =
{
    chLatin_a, chLatin_t, chLatin_t, chLatin_r, chLatin_i, chLatin_b, chLatin_u,
    chLatin_t, chLatin_e, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_CHILD[] =
{
    chLatin_c, chLatin_h, chLatin_i, chLatin_l, chLatin_d, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_DESCENDANT[] =
{
    chLatin_d, chLatin_e, chLatin_s, chLatin_c, chLatin_e, chLatin_n, chLatin_d,
    chLatin_a, chLatin_n, chLatin_t, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_DESCENDANT_OR_SELF[] =
{
    chLatin_d, chLatin_e, chLatin_s, chLatin_c, chLatin_e, chLatin_n, chLatin_d,
    chLatin_a, chLatin_n, chLatin_t, chDash, chLatin_o, chLatin_r, chDash, chLatin_s,
	chLatin_e, chLatin_l, chLatin_f, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_FOLLOWING[] =
{
    chLatin_f, chLatin_o, chLatin_l, chLatin_l, chLatin_o, chLatin_w, chLatin_i,
    chLatin_n, chLatin_g, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_FOLLOWING_SIBLING[] =
{
    chLatin_f, chLatin_o, chLatin_l, chLatin_l, chLatin_o, chLatin_w, chLatin_i,
    chLatin_n, chLatin_g, chDash, chLatin_s, chLatin_i, chLatin_b, chLatin_l, chLatin_i,
    chLatin_n, chLatin_g, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_NAMESPACE[] =
{
    chLatin_n, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a,
    chLatin_c, chLatin_e, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_PARENT[] =
{
    chLatin_p, chLatin_a, chLatin_r, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_PRECEDING[] =
{
    chLatin_p, chLatin_r, chLatin_e, chLatin_c, chLatin_e, chLatin_d, chLatin_i,
    chLatin_n, chLatin_g, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_PRECEDING_SIBLING[] =
{
    chLatin_p, chLatin_r, chLatin_e, chLatin_c, chLatin_e, chLatin_d, chLatin_i,
    chLatin_n, chLatin_g, chDash, chLatin_s, chLatin_i, chLatin_b, chLatin_l, chLatin_i,
    chLatin_n, chLatin_g, chNull
};

const XMLCh XPathSymbols::fgSYMBOL_SELF[] =
{
    chLatin_s, chLatin_e, chLatin_l, chLatin_f, chNull
};

XERCES_CPP_NAMESPACE_END

/**
  * End of file XPathSymbols.cpp
  */

