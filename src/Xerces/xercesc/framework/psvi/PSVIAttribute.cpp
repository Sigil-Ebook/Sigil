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
 * $Id: PSVIAttribute.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/framework/psvi/PSVIAttribute.hpp>

XERCES_CPP_NAMESPACE_BEGIN

PSVIAttribute::PSVIAttribute( MemoryManager* const manager ):  
        PSVIItem(manager)
        , fAttributeDecl(0)
        , fDV(0)
{
}

void PSVIAttribute::reset(
            const XMLCh * const         valContext
            , PSVIItem::VALIDITY_STATE  state
            , PSVIItem::ASSESSMENT_TYPE assessmentType
            , XSSimpleTypeDefinition *  validatingType
            , XSSimpleTypeDefinition *  memberType
            , const XMLCh * const       defaultValue
            , const bool                isSpecified
            , XSAttributeDeclaration *  attrDecl
            , DatatypeValidator *dv
        )
{
    fValidationContext = valContext;
    fValidityState = state;
    fAssessmentType = assessmentType;
    fType = validatingType;
    fMemberType = memberType;
    fDefaultValue = defaultValue;
    fIsSpecified = isSpecified;
    fMemoryManager->deallocate((void *)fCanonicalValue);
    fCanonicalValue = 0;
    fNormalizedValue = 0;
    fAttributeDecl = attrDecl;
    fDV = dv;
}

void PSVIAttribute::setValue(const XMLCh * const       normalizedValue)
{ 
    if(normalizedValue)
    {
        fNormalizedValue = normalizedValue;
        if(fDV && fValidityState == PSVIItem::VALIDITY_VALID)
            fCanonicalValue = (XMLCh *)fDV->getCanonicalRepresentation(normalizedValue, fMemoryManager);
    }
}

XERCES_CPP_NAMESPACE_END


