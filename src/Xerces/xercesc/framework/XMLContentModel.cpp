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

/**
 * $Id: XMLContentModel.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLContentModel.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  public static data
// ---------------------------------------------------------------------------
const unsigned int   XMLContentModel::gInvalidTrans   = 0xFFFFFFFF;
const unsigned int   XMLContentModel::gEOCFakeId      = 0xFFFFFFF1;
const unsigned int   XMLContentModel::gEpsilonFakeId  = 0xFFFFFFF2;

XERCES_CPP_NAMESPACE_END

