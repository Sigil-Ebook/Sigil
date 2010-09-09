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
 * $Id: PSVIElement.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/framework/psvi/PSVIElement.hpp>
#include <xercesc/framework/psvi/XSElementDeclaration.hpp>
#include <xercesc/framework/psvi/XSComplexTypeDefinition.hpp>
#include <xercesc/framework/psvi/XSSimpleTypeDefinition.hpp>

XERCES_CPP_NAMESPACE_BEGIN

PSVIElement::PSVIElement(MemoryManager* const manager):  
        PSVIItem(manager),
        fElementDecl(0),
        fNotationDecl(0),
        fSchemaInfo(0)
{
}

PSVIElement::~PSVIElement()
{
    fMemoryManager->deallocate(fCanonicalValue);
}

XSTypeDefinition* PSVIElement::getTypeDefinition()
{
    return fType;
}

    /**
     * If and only if that type definition is a simple type definition
     * with {variety} union, or a complex type definition whose {content type}
     * is a simple type definition with {variety} union, 
     
     * then an item isomorphic
     * to that member of the union's {member type definitions} which actually
     * validated the element item's normalized value.
     * 
     * @return  a simple type declaration
     */
XSSimpleTypeDefinition* PSVIElement::getMemberTypeDefinition() 
{
    return fMemberType;
}

void PSVIElement::reset( const VALIDITY_STATE          validityState
                       , const ASSESSMENT_TYPE         assessmentType
                       , const XMLCh* const            validationContext
                       , bool                          isSpecified
                       , XSElementDeclaration* const   elemDecl
                       , XSTypeDefinition* const       typeDef
                       , XSSimpleTypeDefinition* const memberType
                       , XSModel* const                schemaInfo
                       , const XMLCh* const            defaultValue
                       , const XMLCh* const            normalizedValue
                       , XMLCh* const                  canonicalValue
                       , XSNotationDeclaration* const  notationDecl)
{
    fValidationContext = validationContext;
    fValidityState = validityState;
    fAssessmentType = assessmentType;
    fIsSpecified = isSpecified;
    fType = typeDef;
    fMemberType = memberType;
    fElementDecl = elemDecl;
    fNotationDecl = notationDecl;
    fSchemaInfo = schemaInfo;
    fDefaultValue = defaultValue;
    fNormalizedValue = normalizedValue;
    fMemoryManager->deallocate(fCanonicalValue);
    fCanonicalValue = canonicalValue;
}

XERCES_CPP_NAMESPACE_END


