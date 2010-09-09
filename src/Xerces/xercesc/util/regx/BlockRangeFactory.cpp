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
 * $Id: BlockRangeFactory.cpp 678879 2008-07-22 20:05:05Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/regx/BlockRangeFactory.hpp>
#include <xercesc/util/regx/RangeToken.hpp>
#include <xercesc/util/regx/RegxDefs.hpp>
#include <xercesc/util/regx/TokenFactory.hpp>
#include <xercesc/util/regx/RangeTokenMap.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local static data
// ---------------------------------------------------------------------------
const int   BLOCKNAMESIZE = 93;

// Block Names IsX
// only define Specials as FEFF..FEFF, missing Specials as FFF0..FFFD, add manually
// only define private use as E000..F8FF,
//    missing 2 private use (F0000..FFFFD and 100000..10FFFD), add manually
const XMLCh fgBlockNames[][50] =
{
    { chLatin_I, chLatin_s, chLatin_B, chLatin_a, chLatin_s, chLatin_i, chLatin_c, chLatin_L, chLatin_a,
      chLatin_t, chLatin_i, chLatin_n,  chNull },
    { chLatin_I, chLatin_s, chLatin_L, chLatin_a, chLatin_t, chLatin_i, chLatin_n, chDash, chDigit_1,
      chLatin_S, chLatin_u, chLatin_p, chLatin_p, chLatin_l, chLatin_e, chLatin_m, chLatin_e,
      chLatin_n, chLatin_t,  chNull },
    { chLatin_I, chLatin_s, chLatin_L, chLatin_a, chLatin_t, chLatin_i, chLatin_n, chLatin_E, chLatin_x,
      chLatin_t, chLatin_e, chLatin_n, chLatin_d, chLatin_e, chLatin_d, chDash, chLatin_A,
       chNull },
    { chLatin_I, chLatin_s, chLatin_L, chLatin_a, chLatin_t, chLatin_i, chLatin_n, chLatin_E, chLatin_x,
      chLatin_t, chLatin_e, chLatin_n, chLatin_d, chLatin_e, chLatin_d, chDash, chLatin_B,
       chNull },
    { chLatin_I, chLatin_s, chLatin_I, chLatin_P, chLatin_A, chLatin_E, chLatin_x, chLatin_t, chLatin_e,
      chLatin_n, chLatin_s, chLatin_i, chLatin_o, chLatin_n, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_S, chLatin_p, chLatin_a, chLatin_c, chLatin_i, chLatin_n, chLatin_g,
      chLatin_M, chLatin_o, chLatin_d, chLatin_i, chLatin_f, chLatin_i, chLatin_e, chLatin_r,
      chLatin_L, chLatin_e, chLatin_t, chLatin_t, chLatin_e, chLatin_r, chLatin_s,
       chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_o, chLatin_m, chLatin_b, chLatin_i, chLatin_n, chLatin_i, chLatin_n,
      chLatin_g, chLatin_D, chLatin_i, chLatin_a, chLatin_c, chLatin_r, chLatin_i,
      chLatin_t, chLatin_i, chLatin_c, chLatin_a, chLatin_l, chLatin_M, chLatin_a,
      chLatin_r, chLatin_k, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_G, chLatin_r, chLatin_e, chLatin_e, chLatin_k,  chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_y, chLatin_r, chLatin_i, chLatin_l, chLatin_l, chLatin_i, chLatin_c,
       chNull },
    { chLatin_I, chLatin_s, chLatin_A, chLatin_r, chLatin_m, chLatin_e, chLatin_n, chLatin_i, chLatin_a, chLatin_n,
       chNull },
    { chLatin_I, chLatin_s, chLatin_H, chLatin_e, chLatin_b, chLatin_r, chLatin_e, chLatin_w,  chNull },
    { chLatin_I, chLatin_s, chLatin_A, chLatin_r, chLatin_a, chLatin_b, chLatin_i, chLatin_c,  chNull },
    { chLatin_I, chLatin_s, chLatin_S, chLatin_y, chLatin_r, chLatin_i, chLatin_a, chLatin_c,  chNull },
    { chLatin_I, chLatin_s, chLatin_T, chLatin_h, chLatin_a, chLatin_a, chLatin_n, chLatin_a,  chNull },
    { chLatin_I, chLatin_s, chLatin_D, chLatin_e, chLatin_v, chLatin_a, chLatin_n, chLatin_a, chLatin_g, chLatin_a,
      chLatin_r, chLatin_i,  chNull },
    { chLatin_I, chLatin_s, chLatin_B, chLatin_e, chLatin_n, chLatin_g, chLatin_a, chLatin_l, chLatin_i,  chNull },
    { chLatin_I, chLatin_s, chLatin_G, chLatin_u, chLatin_r, chLatin_m, chLatin_u, chLatin_k, chLatin_h, chLatin_i,
       chNull },
    { chLatin_I, chLatin_s, chLatin_G, chLatin_u, chLatin_j, chLatin_a, chLatin_r, chLatin_a, chLatin_t, chLatin_i,
       chNull },
    { chLatin_I, chLatin_s, chLatin_O, chLatin_r, chLatin_i, chLatin_y, chLatin_a,  chNull },
    { chLatin_I, chLatin_s, chLatin_T, chLatin_a, chLatin_m, chLatin_i, chLatin_l,  chNull },
    { chLatin_I, chLatin_s, chLatin_T, chLatin_e, chLatin_l, chLatin_u, chLatin_g, chLatin_u,  chNull },
    { chLatin_I, chLatin_s, chLatin_K, chLatin_a, chLatin_n, chLatin_n, chLatin_a, chLatin_d, chLatin_a,  chNull },
    { chLatin_I, chLatin_s, chLatin_M, chLatin_a, chLatin_l, chLatin_a, chLatin_y, chLatin_a, chLatin_l, chLatin_a,
      chLatin_m,  chNull },
    { chLatin_I, chLatin_s, chLatin_S, chLatin_i, chLatin_n, chLatin_h, chLatin_a, chLatin_l, chLatin_a,  chNull },
    { chLatin_I, chLatin_s, chLatin_T, chLatin_h, chLatin_a, chLatin_i,  chNull },
    { chLatin_I, chLatin_s, chLatin_L, chLatin_a, chLatin_o,  chNull },
    { chLatin_I, chLatin_s, chLatin_T, chLatin_i, chLatin_b, chLatin_e, chLatin_t, chLatin_a, chLatin_n,  chNull },
    { chLatin_I, chLatin_s, chLatin_M, chLatin_y, chLatin_a, chLatin_n, chLatin_m, chLatin_a, chLatin_r,  chNull },
    { chLatin_I, chLatin_s, chLatin_G, chLatin_e, chLatin_o, chLatin_r, chLatin_g, chLatin_i, chLatin_a, chLatin_n,
       chNull },
    { chLatin_I, chLatin_s, chLatin_H, chLatin_a, chLatin_n, chLatin_g, chLatin_u, chLatin_l, chLatin_J,
      chLatin_a, chLatin_m, chLatin_o,  chNull },
    { chLatin_I, chLatin_s, chLatin_E, chLatin_t, chLatin_h, chLatin_i, chLatin_o, chLatin_p, chLatin_i,  chLatin_c,
       chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_h, chLatin_e, chLatin_r, chLatin_o, chLatin_k, chLatin_e,  chLatin_e,
       chNull },
    { chLatin_I, chLatin_s, chLatin_U, chLatin_n, chLatin_i, chLatin_f, chLatin_i, chLatin_e, chLatin_d,
      chLatin_C, chLatin_a, chLatin_n, chLatin_a, chLatin_d, chLatin_i, chLatin_a, chLatin_n,
      chLatin_A, chLatin_b, chLatin_o, chLatin_r, chLatin_i, chLatin_g, chLatin_i, chLatin_n, chLatin_a, chLatin_l,
      chLatin_S, chLatin_y, chLatin_l, chLatin_l, chLatin_a, chLatin_b, chLatin_i, chLatin_c, chLatin_s, chNull },
    { chLatin_I, chLatin_s, chLatin_O, chLatin_g, chLatin_h, chLatin_a, chLatin_m, chNull },
    { chLatin_I, chLatin_s, chLatin_R, chLatin_u, chLatin_n, chLatin_i, chLatin_c, chNull },
    { chLatin_I, chLatin_s, chLatin_K, chLatin_h, chLatin_m, chLatin_e, chLatin_r, chNull },
    { chLatin_I, chLatin_s, chLatin_M, chLatin_o, chLatin_n, chLatin_g, chLatin_o, chLatin_l, chLatin_i,
      chLatin_a, chLatin_n, chNull },
    { chLatin_I, chLatin_s, chLatin_L, chLatin_a, chLatin_t, chLatin_i, chLatin_n, chLatin_E, chLatin_x,
      chLatin_t, chLatin_e, chLatin_n, chLatin_d, chLatin_e, chLatin_d, chLatin_A,
      chLatin_d, chLatin_d, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_a,
      chLatin_l,  chNull },
    { chLatin_I, chLatin_s, chLatin_G, chLatin_r, chLatin_e, chLatin_e, chLatin_k, chLatin_E, chLatin_x,
      chLatin_t, chLatin_e, chLatin_n, chLatin_d, chLatin_e, chLatin_d,  chNull },
    { chLatin_I, chLatin_s, chLatin_G, chLatin_e, chLatin_n, chLatin_e, chLatin_r, chLatin_a, chLatin_l,
      chLatin_P, chLatin_u, chLatin_n, chLatin_c, chLatin_t, chLatin_u, chLatin_a, chLatin_t,
      chLatin_i, chLatin_o, chLatin_n,  chNull },
    { chLatin_I, chLatin_s, chLatin_S, chLatin_u, chLatin_p, chLatin_e, chLatin_r, chLatin_s, chLatin_c, chLatin_r,
      chLatin_i, chLatin_p, chLatin_t, chLatin_s, chLatin_a, chLatin_n, chLatin_d,
      chLatin_S, chLatin_u, chLatin_b, chLatin_s, chLatin_c, chLatin_r, chLatin_i,
      chLatin_p, chLatin_t, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_u, chLatin_r, chLatin_r, chLatin_e, chLatin_n, chLatin_c, chLatin_y,
      chLatin_S, chLatin_y, chLatin_m, chLatin_b, chLatin_o, chLatin_l, chLatin_s,
       chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_o, chLatin_m, chLatin_b, chLatin_i, chLatin_n, chLatin_i, chLatin_n,
      chLatin_g, chLatin_M, chLatin_a, chLatin_r, chLatin_k, chLatin_s,
      chLatin_f, chLatin_o, chLatin_r, chLatin_S, chLatin_y, chLatin_m, chLatin_b,
      chLatin_o, chLatin_l, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_L, chLatin_e, chLatin_t, chLatin_t, chLatin_e, chLatin_r, chLatin_l, chLatin_i,
      chLatin_k, chLatin_e, chLatin_S, chLatin_y, chLatin_m, chLatin_b, chLatin_o,
      chLatin_l, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_N, chLatin_u, chLatin_m, chLatin_b, chLatin_e, chLatin_r, chLatin_F,
      chLatin_o, chLatin_r, chLatin_m, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_A, chLatin_r, chLatin_r, chLatin_o, chLatin_w, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_M, chLatin_a, chLatin_t, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_t,
      chLatin_i, chLatin_c, chLatin_a, chLatin_l, chLatin_O, chLatin_p, chLatin_e,
      chLatin_r, chLatin_a, chLatin_t, chLatin_o, chLatin_r, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_M, chLatin_i, chLatin_s, chLatin_c, chLatin_e, chLatin_l, chLatin_l, chLatin_a,
      chLatin_n, chLatin_e, chLatin_o, chLatin_u, chLatin_s, chLatin_T, chLatin_e,
      chLatin_c, chLatin_h, chLatin_n, chLatin_i, chLatin_c, chLatin_a, chLatin_l,  chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_o, chLatin_n, chLatin_t, chLatin_r, chLatin_o, chLatin_l,
      chLatin_P, chLatin_i, chLatin_c, chLatin_t, chLatin_u, chLatin_r, chLatin_e, chLatin_s,
       chNull },
    { chLatin_I, chLatin_s, chLatin_O, chLatin_p, chLatin_t, chLatin_i, chLatin_c, chLatin_a, chLatin_l,
      chLatin_C, chLatin_h, chLatin_a, chLatin_r, chLatin_a, chLatin_c, chLatin_t, chLatin_e,
      chLatin_r, chLatin_R, chLatin_e, chLatin_c, chLatin_o, chLatin_g, chLatin_n,
      chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n,  chNull },
    { chLatin_I, chLatin_s, chLatin_E, chLatin_n, chLatin_c, chLatin_l, chLatin_o, chLatin_s, chLatin_e, chLatin_d,
      chLatin_A, chLatin_l, chLatin_p, chLatin_h, chLatin_a, chLatin_n, chLatin_u,
      chLatin_m, chLatin_e, chLatin_r, chLatin_i, chLatin_c, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_B, chLatin_o, chLatin_x, chLatin_D, chLatin_r, chLatin_a, chLatin_w,
      chLatin_i, chLatin_n, chLatin_g,  chNull },
    { chLatin_I, chLatin_s, chLatin_B, chLatin_l, chLatin_o, chLatin_c, chLatin_k, chLatin_E, chLatin_l,
      chLatin_e, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_G, chLatin_e, chLatin_o, chLatin_m, chLatin_e, chLatin_t, chLatin_r, chLatin_i,
      chLatin_c, chLatin_S, chLatin_h, chLatin_a, chLatin_p, chLatin_e, chLatin_s,
       chNull },
    { chLatin_I, chLatin_s, chLatin_M, chLatin_i, chLatin_s, chLatin_c, chLatin_e, chLatin_l, chLatin_l, chLatin_a,
      chLatin_n, chLatin_e, chLatin_o, chLatin_u, chLatin_s, chLatin_S, chLatin_y,
      chLatin_m, chLatin_b, chLatin_o, chLatin_l, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_D, chLatin_i, chLatin_n, chLatin_g, chLatin_b, chLatin_a, chLatin_t, chLatin_s,
       chNull },
    { chLatin_I, chLatin_s, chLatin_B, chLatin_r, chLatin_a, chLatin_i, chLatin_l, chLatin_l, chLatin_e,
      chLatin_P, chLatin_a, chLatin_t, chLatin_t, chLatin_e, chLatin_r, chLatin_n, chLatin_s, chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_J, chLatin_K, chLatin_R, chLatin_a, chLatin_d, chLatin_i, chLatin_c,
      chLatin_a, chLatin_l, chLatin_s, chLatin_S, chLatin_u, chLatin_p, chLatin_p, chLatin_l, chLatin_e, chLatin_m,
      chLatin_e, chLatin_n, chLatin_t,  chNull },
    { chLatin_I, chLatin_s, chLatin_K, chLatin_a, chLatin_n, chLatin_g, chLatin_x, chLatin_i,
      chLatin_R, chLatin_a, chLatin_d, chLatin_i, chLatin_c, chLatin_a, chLatin_l, chLatin_s, chNull },
    { chLatin_I, chLatin_s, chLatin_I, chLatin_d, chLatin_e, chLatin_o, chLatin_g, chLatin_r, chLatin_a, chLatin_p,
      chLatin_h, chLatin_i, chLatin_c, chLatin_D, chLatin_e, chLatin_s, chLatin_c, chLatin_r, chLatin_i, chLatin_p,
      chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_C, chLatin_h, chLatin_a, chLatin_r, chLatin_a, chLatin_c,
      chLatin_t, chLatin_e, chLatin_r, chLatin_s, chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_J, chLatin_K, chLatin_S, chLatin_y, chLatin_m, chLatin_b,
      chLatin_o, chLatin_l, chLatin_s, chLatin_a, chLatin_n, chLatin_d,
      chLatin_P, chLatin_u, chLatin_n, chLatin_c, chLatin_t, chLatin_u, chLatin_a, chLatin_t,
      chLatin_i, chLatin_o, chLatin_n,  chNull },
    { chLatin_I, chLatin_s, chLatin_H, chLatin_i, chLatin_r, chLatin_a, chLatin_g, chLatin_a, chLatin_n, chLatin_a,
       chNull },
    { chLatin_I, chLatin_s, chLatin_K, chLatin_a, chLatin_t, chLatin_a, chLatin_k, chLatin_a, chLatin_n, chLatin_a,
       chNull },
    { chLatin_I, chLatin_s, chLatin_B, chLatin_o, chLatin_p, chLatin_o, chLatin_m, chLatin_o, chLatin_f, chLatin_o,
       chNull },
    { chLatin_I, chLatin_s, chLatin_H, chLatin_a, chLatin_n, chLatin_g, chLatin_u, chLatin_l, chLatin_C,
      chLatin_o, chLatin_m, chLatin_p, chLatin_a, chLatin_t, chLatin_i, chLatin_b, chLatin_i,
      chLatin_l, chLatin_i, chLatin_t, chLatin_y, chLatin_J, chLatin_a, chLatin_m,
      chLatin_o,  chNull },
    { chLatin_I, chLatin_s, chLatin_K, chLatin_a, chLatin_n, chLatin_b, chLatin_u, chLatin_n,  chNull },
    { chLatin_I, chLatin_s, chLatin_B, chLatin_o, chLatin_p, chLatin_o, chLatin_m, chLatin_o, chLatin_f, chLatin_o,
      chLatin_E, chLatin_x, chLatin_t, chLatin_e, chLatin_n, chLatin_d, chLatin_e, chLatin_d, chNull },
    { chLatin_I, chLatin_s, chLatin_E, chLatin_n, chLatin_c, chLatin_l, chLatin_o, chLatin_s, chLatin_e, chLatin_d,
      chLatin_C, chLatin_J, chLatin_K, chLatin_L, chLatin_e, chLatin_t,
      chLatin_t, chLatin_e, chLatin_r, chLatin_s, chLatin_a, chLatin_n, chLatin_d,
      chLatin_M, chLatin_o, chLatin_n, chLatin_t, chLatin_h, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_J, chLatin_K, chLatin_C, chLatin_o, chLatin_m, chLatin_p,
      chLatin_a, chLatin_t, chLatin_i, chLatin_b, chLatin_i, chLatin_l, chLatin_i, chLatin_t,
      chLatin_y,  chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_J, chLatin_K, chLatin_U, chLatin_n, chLatin_i, chLatin_f,
      chLatin_i, chLatin_e, chLatin_d, chLatin_I, chLatin_d, chLatin_e, chLatin_o,
      chLatin_g, chLatin_r, chLatin_a, chLatin_p, chLatin_h, chLatin_s,
      chLatin_E, chLatin_x, chLatin_t, chLatin_e, chLatin_n, chLatin_s, chLatin_i, chLatin_o, chLatin_n, chLatin_A,
      chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_J, chLatin_K, chLatin_U, chLatin_n, chLatin_i, chLatin_f,
      chLatin_i, chLatin_e, chLatin_d, chLatin_I, chLatin_d, chLatin_e, chLatin_o,
      chLatin_g, chLatin_r, chLatin_a, chLatin_p, chLatin_h, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_Y, chLatin_i, chLatin_S, chLatin_y, chLatin_l, chLatin_l, chLatin_a,
      chLatin_b, chLatin_l, chLatin_e, chLatin_s, chNull },
    { chLatin_I, chLatin_s, chLatin_Y, chLatin_i, chLatin_R, chLatin_a, chLatin_d, chLatin_i, chLatin_c,
      chLatin_a, chLatin_l, chLatin_s, chNull },
    { chLatin_I, chLatin_s, chLatin_H, chLatin_a, chLatin_n, chLatin_g, chLatin_u, chLatin_l, chLatin_S,
      chLatin_y, chLatin_l, chLatin_l, chLatin_a, chLatin_b, chLatin_l, chLatin_e, chLatin_s,
       chNull },
    { chLatin_I, chLatin_s, chLatin_P, chLatin_r, chLatin_i, chLatin_v, chLatin_a, chLatin_t, chLatin_e,
      chLatin_U, chLatin_s, chLatin_e,  chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_J, chLatin_K, chLatin_C, chLatin_o, chLatin_m, chLatin_p,
      chLatin_a, chLatin_t, chLatin_i, chLatin_b, chLatin_i, chLatin_l, chLatin_i, chLatin_t,
      chLatin_y, chLatin_I, chLatin_d, chLatin_e, chLatin_o, chLatin_g, chLatin_r,
      chLatin_a, chLatin_p, chLatin_h, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_A, chLatin_l, chLatin_p, chLatin_h, chLatin_a, chLatin_b, chLatin_e, chLatin_t,
      chLatin_i, chLatin_c, chLatin_P, chLatin_r, chLatin_e, chLatin_s, chLatin_e,
      chLatin_n, chLatin_t, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n,
      chLatin_F, chLatin_o, chLatin_r, chLatin_m, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_A, chLatin_r, chLatin_a, chLatin_b, chLatin_i, chLatin_c, chLatin_P,
      chLatin_r, chLatin_e, chLatin_s, chLatin_e, chLatin_n, chLatin_t, chLatin_a, chLatin_t,
      chLatin_i, chLatin_o, chLatin_n, chLatin_F, chLatin_o, chLatin_r, chLatin_m,
      chLatin_s, chDash, chLatin_A,  chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_o, chLatin_m, chLatin_b, chLatin_i, chLatin_n, chLatin_i, chLatin_n,
      chLatin_g, chLatin_H, chLatin_a, chLatin_l, chLatin_f, chLatin_M,
      chLatin_a, chLatin_r, chLatin_k, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_J, chLatin_K, chLatin_C, chLatin_o, chLatin_m, chLatin_p,
      chLatin_a, chLatin_t, chLatin_i, chLatin_b, chLatin_i, chLatin_l, chLatin_i, chLatin_t,
      chLatin_y, chLatin_F, chLatin_o, chLatin_r, chLatin_m, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_S, chLatin_m, chLatin_a, chLatin_l, chLatin_l, chLatin_F, chLatin_o,
      chLatin_r, chLatin_m, chLatin_V, chLatin_a, chLatin_r, chLatin_i, chLatin_a,
      chLatin_n, chLatin_t, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_A, chLatin_r, chLatin_a, chLatin_b, chLatin_i, chLatin_c, chLatin_P,
      chLatin_r, chLatin_e, chLatin_s, chLatin_e, chLatin_n, chLatin_t, chLatin_a, chLatin_t,
      chLatin_i, chLatin_o, chLatin_n, chLatin_F, chLatin_o, chLatin_r, chLatin_m,
      chLatin_s, chDash, chLatin_B,  chNull },
    { chLatin_I, chLatin_s, chLatin_S, chLatin_p, chLatin_e, chLatin_c, chLatin_i, chLatin_a, chLatin_l, chLatin_s,
       chNull },
    { chLatin_I, chLatin_s, chLatin_H, chLatin_a, chLatin_l, chLatin_f, chLatin_w, chLatin_i, chLatin_d, chLatin_t,
      chLatin_h, chLatin_a, chLatin_n, chLatin_d, chLatin_F, chLatin_u,
      chLatin_l, chLatin_l, chLatin_w, chLatin_i, chLatin_d, chLatin_t, chLatin_h,
      chLatin_F, chLatin_o, chLatin_r, chLatin_m, chLatin_s,  chNull },
    { chLatin_I, chLatin_s, chLatin_O, chLatin_l, chLatin_d, chLatin_I, chLatin_t, chLatin_a, chLatin_l, chLatin_i,
      chLatin_c, chNull },
    { chLatin_I, chLatin_s, chLatin_G, chLatin_o, chLatin_t, chLatin_h, chLatin_i, chLatin_c, chNull },
    { chLatin_I, chLatin_s, chLatin_D, chLatin_e, chLatin_s, chLatin_e, chLatin_r, chLatin_e, chLatin_t, chNull },
    { chLatin_I, chLatin_s, chLatin_B, chLatin_y, chLatin_z, chLatin_a, chLatin_n, chLatin_t, chLatin_i, chLatin_n, chLatin_e,
      chLatin_M, chLatin_u, chLatin_s, chLatin_i, chLatin_c, chLatin_a, chLatin_l,
      chLatin_S, chLatin_y, chLatin_m, chLatin_b, chLatin_o, chLatin_l, chLatin_s, chNull },
    { chLatin_I, chLatin_s, chLatin_M, chLatin_u, chLatin_s, chLatin_i, chLatin_c, chLatin_a, chLatin_l,
      chLatin_S, chLatin_y, chLatin_m, chLatin_b, chLatin_o, chLatin_l, chLatin_s, chNull },
    { chLatin_I, chLatin_s, chLatin_M, chLatin_a, chLatin_t, chLatin_h, chLatin_e,
      chLatin_m, chLatin_a, chLatin_t, chLatin_i, chLatin_c, chLatin_a, chLatin_l,
      chLatin_A, chLatin_l, chLatin_p, chLatin_h, chLatin_a,
      chLatin_n, chLatin_u, chLatin_m, chLatin_e, chLatin_r, chLatin_i, chLatin_c,
      chLatin_S, chLatin_y, chLatin_m, chLatin_b, chLatin_o, chLatin_l, chLatin_s, chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_J, chLatin_K, chLatin_U, chLatin_n, chLatin_i, chLatin_f,
      chLatin_i, chLatin_e, chLatin_d, chLatin_I, chLatin_d, chLatin_e, chLatin_o,
      chLatin_g, chLatin_r, chLatin_a, chLatin_p, chLatin_h, chLatin_s,
      chLatin_E, chLatin_x, chLatin_t, chLatin_e, chLatin_n, chLatin_s, chLatin_i, chLatin_o, chLatin_n, chLatin_B,
      chNull },
    { chLatin_I, chLatin_s, chLatin_C, chLatin_J, chLatin_K, chLatin_C, chLatin_o, chLatin_m, chLatin_p,
      chLatin_a, chLatin_t, chLatin_i, chLatin_b, chLatin_i, chLatin_l, chLatin_i, chLatin_t, chLatin_y,
      chLatin_I, chLatin_d, chLatin_e, chLatin_o, chLatin_g, chLatin_r, chLatin_a, chLatin_p, chLatin_h, chLatin_s,
      chLatin_S, chLatin_u, chLatin_p, chLatin_p, chLatin_l, chLatin_e, chLatin_m, chLatin_e, chLatin_n, chLatin_t,  chNull },
    { chLatin_I, chLatin_s, chLatin_T, chLatin_a, chLatin_g, chLatin_s, chNull },
};

const XMLInt32 blockRanges[] =
{
    0x0000,0x007F,0x0080,0x00FF,0x0100,0x017F,0x0180,0x024F,0x0250,0x02AF,0x02B0,0x02FF,
    0x0300,0x036F,0x0370,0x03FF,0x0400,0x04FF,0x0530,0x058F,0x0590,0x05FF,0x0600,0x06FF,
    0x0700,0x074F,0x0780,0x07BF,0x0900,0x097F,0x0980,0x09FF,0x0A00,0x0A7F,0x0A80,0x0AFF,
    0x0B00,0x0B7F,0x0B80,0x0BFF,0x0C00,0x0C7F,0x0C80,0x0CFF,0x0D00,0x0D7F,0x0D80,0x0DFF,
    0x0E00,0x0E7F,0x0E80,0x0EFF,0x0F00,0x0FFF,0x1000,0x109F,0x10A0,0x10FF,0x1100,0x11FF,
    0x1200,0x137F,0x13A0,0x13FF,0x1400,0x167F,0x1680,0x169F,0x16A0,0x16FF,0x1780,0x17FF,
    0x1800,0x18AF,0x1E00,0x1EFF,0x1F00,0x1FFF,0x2000,0x206F,0x2070,0x209F,0x20A0,0x20CF,
    0x20D0,0x20FF,0x2100,0x214F,0x2150,0x218F,0x2190,0x21FF,0x2200,0x22FF,0x2300,0x23FF,
    0x2400,0x243F,0x2440,0x245F,0x2460,0x24FF,0x2500,0x257F,0x2580,0x259F,0x25A0,0x25FF,
    0x2600,0x26FF,0x2700,0x27BF,0x2800,0x28FF,0x2E80,0x2EFF,0x2F00,0x2FDF,0x2FF0,0x2FFF,
    0x3000,0x303F,0x3040,0x309F,0x30A0,0x30FF,0x3100,0x312F,0x3130,0x318F,0x3190,0x319F,
    0x31A0,0x31BF,0x3200,0x32FF,0x3300,0x33FF,0x3400,0x4DB5,0x4E00,0x9FFF,0xA000,0xA48F,
    0xA490,0xA4CF,0xAC00,0xD7A3,0xE000,0xF8FF,0xF900,0xFAFF,0xFB00,0xFB4F,0xFB50,0xFDFF,
    0xFE20,0xFE2F,0xFE30,0xFE4F,0xFE50,0xFE6F,0xFE70,0xFEFE,0xFEFF,0xFEFF,0xFF00,0xFFEF,
    0x10300,0x1032F,0x10330,0x1034F,0x10400,0x1044F,0x1D000,0x1D0FF,0x1D100,0x1D1FF,
    0x1D400,0x1D7FF,0x20000,0x2A6D6,0x2F800,0x2FA1F,0xE0000,0xE007F, chNull
};

// ---------------------------------------------------------------------------
//  BlockRangeFactory: Constructors and Destructor
// ---------------------------------------------------------------------------
BlockRangeFactory::BlockRangeFactory()
{

}

BlockRangeFactory::~BlockRangeFactory() {

}

// ---------------------------------------------------------------------------
//  BlockRangeFactory: Range creation methods
// ---------------------------------------------------------------------------
void BlockRangeFactory::buildRanges(RangeTokenMap *rangeTokMap) {

    if (fRangesCreated)
        return;

    if (!fKeywordsInitialized) {
        initializeKeywordMap(rangeTokMap);
    }

    TokenFactory* tokFactory = rangeTokMap->getTokenFactory();

    //for performance, once the desired specials and private use are found
    //don't need to compareString anymore
    bool foundSpecial = false;
    bool foundPrivate = false;

    for (int i=0; i < BLOCKNAMESIZE; i++) {
        RangeToken* tok = tokFactory->createRange();
        tok->addRange(blockRanges[i*2], blockRanges[(i*2)+1]);

        if (!foundSpecial && XMLString::equals((XMLCh*)fgBlockNames[i] , (XMLCh*) fgBlockIsSpecials)) {
            tok->addRange(0xFFF0, 0xFFFD);
            foundSpecial = true;
        }
        if (!foundPrivate && XMLString::equals((XMLCh*)fgBlockNames[i] , (XMLCh*) fgBlockIsPrivateUse)) {
            tok->addRange(0xF0000, 0xFFFFD);
            tok->addRange(0x100000, 0x10FFFD);
            foundPrivate = true;
        }

        // Build the internal map.
        tok->createMap();
        rangeTokMap->setRangeToken(fgBlockNames[i], tok);
        tok = RangeToken::complementRanges(tok, tokFactory);
        // Build the internal map.
        tok->createMap();
        rangeTokMap->setRangeToken(fgBlockNames[i], tok , true);
    }

    fRangesCreated = true;
}

// ---------------------------------------------------------------------------
//  BlockRangeFactory: Range creation methods
// ---------------------------------------------------------------------------
void BlockRangeFactory::initializeKeywordMap(RangeTokenMap *rangeTokMap) {

    if (fKeywordsInitialized)
        return;

    for (int i=0; i< BLOCKNAMESIZE; i++) {
        rangeTokMap->addKeywordMap(fgBlockNames[i], fgBlockCategory);
    }

    fKeywordsInitialized = true;
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file BlockRangeFactory.cpp
  */
