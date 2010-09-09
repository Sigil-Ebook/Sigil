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
 * $Id: XMLNumber.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLNumber.hpp>

//since we need to dynamically created each and every derivatives 
//during deserialization by XSerializeEngine>>Derivative, we got
//to include all hpp
#include <xercesc/util/XMLDouble.hpp>
#include <xercesc/util/XMLFloat.hpp>
#include <xercesc/util/XMLDateTime.hpp>
#include <xercesc/util/XMLBigDecimal.hpp>

XERCES_CPP_NAMESPACE_BEGIN

XMLNumber::XMLNumber()
{}

XMLNumber::XMLNumber(const XMLNumber& toCopy)
: XSerializable(toCopy)
, XMemory(toCopy)
{}

XMLNumber::~XMLNumber()
{}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_NOCREATE(XMLNumber)

void XMLNumber::serialize(XSerializeEngine&)
{
    // this class has no data to serialize/de-serilize
}

XMLNumber* XMLNumber::loadNumber(XMLNumber::NumberType  numType
                               , XSerializeEngine&      serEng)
{

    switch((XMLNumber::NumberType) numType)
    {
    case XMLNumber::Float: 
        XMLFloat* floatNum;
        serEng>>floatNum;
        return floatNum;
        break;
    case XMLNumber::Double:
        XMLDouble* doubleNum;
        serEng>>doubleNum;
        return doubleNum;
        break;
    case XMLNumber::BigDecimal: 
        XMLBigDecimal* bigdecimalNum;
        serEng>>bigdecimalNum;
        return bigdecimalNum;
        break;
    case XMLNumber::DateTime: 
        XMLDateTime* datetimeNum;
        serEng>>datetimeNum;
        return datetimeNum;
        break;
    case XMLNumber::UnKnown:
        return 0;
        break;
    }
    //we treat this same as UnKnown
    return 0;
}

XERCES_CPP_NAMESPACE_END
