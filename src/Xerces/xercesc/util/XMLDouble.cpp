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
 * $Id: XMLDouble.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLDouble.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/NumberFormatException.hpp>
#include <xercesc/util/Janitor.hpp>

#include <string.h>
#include <stdlib.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  ctor/dtor
// ---------------------------------------------------------------------------
XMLDouble::XMLDouble(const XMLCh* const strValue,
                     MemoryManager* const manager)
:XMLAbstractDoubleFloat(manager)
{
    init(strValue);
}

XMLDouble::~XMLDouble()
{
}

void XMLDouble::checkBoundary(char* const strValue)
{
    convert(strValue);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(XMLDouble)

XMLDouble::XMLDouble(MemoryManager* const manager)
:XMLAbstractDoubleFloat(manager)
{
}

void XMLDouble::serialize(XSerializeEngine& serEng)
{
    XMLAbstractDoubleFloat::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END
