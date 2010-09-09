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
 * $Id: RegxDefs.hpp 678879 2008-07-22 20:05:05Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_REGXDEFS_HPP)
#define XERCESC_INCLUDE_GUARD_REGXDEFS_HPP

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

static const XMLCh fgXMLCategory[] =
{
    chLatin_X, chLatin_M, chLatin_L, chNull
};

static const XMLCh fgASCIICategory[] =
{
    chLatin_A, chLatin_S, chLatin_C, chLatin_I, chLatin_I, chNull
};

static const XMLCh fgUnicodeCategory[] =
{
    chLatin_U, chLatin_N, chLatin_I, chLatin_C, chLatin_O, chLatin_D,
    chLatin_E, chNull
};

static const XMLCh fgBlockCategory[] =
{
    chLatin_B, chLatin_L, chLatin_O, chLatin_C, chLatin_K, chNull
};

static const XMLCh fgXMLSpace[] =
{
    chLatin_x, chLatin_m, chLatin_l, chColon, chLatin_i, chLatin_s, chLatin_S,
    chLatin_p, chLatin_a, chLatin_c, chLatin_e, chNull
};

static const XMLCh fgXMLDigit[] =
{
    chLatin_x, chLatin_m, chLatin_l, chColon, chLatin_i, chLatin_s, chLatin_D,
    chLatin_i, chLatin_g, chLatin_i, chLatin_t, chNull
};

static const XMLCh fgXMLWord[] =
{
    chLatin_x, chLatin_m, chLatin_l, chColon, chLatin_i, chLatin_s, chLatin_W,
    chLatin_o, chLatin_r, chLatin_d, chNull
};

static const XMLCh fgXMLNameChar[] =
{
    chLatin_x, chLatin_m, chLatin_l, chColon, chLatin_i, chLatin_s, chLatin_N,
    chLatin_a, chLatin_m, chLatin_e, chLatin_C, chLatin_h, chLatin_a,
    chLatin_r, chNull
};

static const XMLCh fgXMLInitialNameChar[] =
{
    chLatin_x, chLatin_m, chLatin_l, chColon, chLatin_i, chLatin_s, chLatin_I,
    chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_a, chLatin_l,
    chLatin_N, chLatin_a, chLatin_m, chLatin_e, chLatin_C, chLatin_h,
    chLatin_a, chLatin_r, chNull
};

static const XMLCh fgASCII[] =
{
    chLatin_a, chLatin_s, chLatin_c, chLatin_i, chLatin_i, chColon, chLatin_i,
    chLatin_s, chLatin_A, chLatin_s, chLatin_c, chLatin_i, chLatin_i, chNull
};

static const XMLCh fgASCIIDigit[] =
{
    chLatin_a, chLatin_s, chLatin_c, chLatin_i, chLatin_i, chColon, chLatin_i,
    chLatin_s, chLatin_D, chLatin_i, chLatin_g, chLatin_i, chLatin_t, chNull
};

static const XMLCh fgASCIIWord[] =
{
    chLatin_a, chLatin_s, chLatin_c, chLatin_i, chLatin_i, chColon, chLatin_i,
    chLatin_s, chLatin_W, chLatin_o, chLatin_r, chLatin_d, chNull
};

static const XMLCh fgASCIISpace[] =
{
    chLatin_a, chLatin_s, chLatin_c, chLatin_i, chLatin_i, chColon, chLatin_i,
    chLatin_s, chLatin_S, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chNull
};

static const XMLCh fgASCIIXDigit[] =
{
    chLatin_a, chLatin_s, chLatin_c, chLatin_i, chLatin_i, chColon, chLatin_i,
    chLatin_s, chLatin_X, chLatin_D, chLatin_i, chLatin_g, chLatin_i,
    chLatin_t, chNull
};


static const XMLCh fgUniAll[] =
{
    chLatin_A, chLatin_L, chLatin_L, chNull
};

static const XMLCh fgUniIsAlpha[] =
{
    chLatin_I, chLatin_s, chLatin_A, chLatin_l, chLatin_p, chLatin_h,
    chLatin_a, chNull
};

static const XMLCh fgUniIsAlnum[] =
{
    chLatin_I, chLatin_s, chLatin_A, chLatin_l, chLatin_n, chLatin_u,
    chLatin_m, chNull
};

static const XMLCh fgUniIsWord[] =
{
    chLatin_I, chLatin_s, chLatin_W, chLatin_o, chLatin_r, chLatin_d,
    chNull
};


static const XMLCh fgUniIsDigit[] =
{
    chLatin_I, chLatin_s, chLatin_D, chLatin_i, chLatin_g, chLatin_i,
    chLatin_t, chNull
};

static const XMLCh fgUniIsUpper[] =
{
    chLatin_I, chLatin_s, chLatin_U, chLatin_p, chLatin_p, chLatin_e,
    chLatin_r, chNull
};

static const XMLCh fgUniIsLower[] =
{
    chLatin_I, chLatin_s, chLatin_L, chLatin_o, chLatin_w, chLatin_e,
    chLatin_r, chNull
};

static const XMLCh fgUniIsPunct[] =
{
    chLatin_I, chLatin_s, chLatin_P, chLatin_u, chLatin_n, chLatin_c,
    chLatin_t, chNull
};

static const XMLCh fgUniIsSpace[] =
{
    chLatin_I, chLatin_s, chLatin_S, chLatin_p, chLatin_a, chLatin_c,
    chLatin_e, chNull
};

static const XMLCh fgUniAssigned[] =
{
    chLatin_A, chLatin_S, chLatin_S, chLatin_I, chLatin_G, chLatin_N,
    chLatin_E, chLatin_D, chNull
};


static const XMLCh fgUniDecimalDigit[] =
{
    chLatin_N, chLatin_d, chNull
};

static const XMLCh fgBlockIsSpecials[] =
{
    chLatin_I, chLatin_s, chLatin_S, chLatin_p, chLatin_e, chLatin_c, chLatin_i, chLatin_a,
    chLatin_l, chLatin_s, chNull
};

static const XMLCh fgBlockIsPrivateUse[] =
{
    chLatin_I, chLatin_s, chLatin_P, chLatin_r, chLatin_i, chLatin_v, chLatin_a, chLatin_t, chLatin_e,
    chLatin_U, chLatin_s, chLatin_e,  chNull
};

static const XMLCh fgUniLetter[] =
{
    chLatin_L, chNull
};

static const XMLCh fgUniNumber[] =
{
    chLatin_N, chNull
};

static const XMLCh fgUniMark[] =
{
    chLatin_M, chNull
};

static const XMLCh fgUniSeparator[] =
{
    chLatin_Z, chNull
};

static const XMLCh fgUniPunctuation[] =
{
    chLatin_P, chNull
};

static const XMLCh fgUniControl[] =
{
    chLatin_C, chNull
};

static const XMLCh fgUniSymbol[] =
{
    chLatin_S, chNull
};

XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file RegxDefs.hpp
  */

