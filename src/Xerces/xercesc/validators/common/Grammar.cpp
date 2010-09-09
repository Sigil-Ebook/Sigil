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
 * $Id: Grammar.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/validators/common/Grammar.hpp>

//since we need to dynamically created each and every derivatives 
//during deserialization by XSerializeEngine>>Derivative, we got
//to include all hpp

#include <xercesc/validators/DTD/DTDGrammar.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_NOCREATE(Grammar)

void Grammar::serialize(XSerializeEngine&)
{
    //no data
}

void Grammar::storeGrammar(XSerializeEngine&   serEng
                         , Grammar*  const     grammar)
{
    if (grammar)
    {
        serEng<<(int) grammar->getGrammarType();
        serEng<<grammar;
    }
    else
    {
        serEng<<(int) UnKnown;
    }

}

Grammar* Grammar::loadGrammar(XSerializeEngine& serEng)
{

    int type;
    serEng>>type;

    switch((GrammarType)type)
    {
    case DTDGrammarType: 
        DTDGrammar* dtdGrammar;
        serEng>>dtdGrammar;
        return dtdGrammar;
    case SchemaGrammarType:
        SchemaGrammar* schemaGrammar;
        serEng>>schemaGrammar;
        return schemaGrammar;        
    case UnKnown:
        return 0;        
    default: //we treat this same as UnKnown
        return 0;        
    }

}

XERCES_CPP_NAMESPACE_END
