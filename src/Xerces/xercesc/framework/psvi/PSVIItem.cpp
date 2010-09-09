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
 * $Id: PSVIItem.cpp 471747 2006-11-06 14:31:56Z amassari $
 */

#include <xercesc/framework/psvi/PSVIItem.hpp>
#include <xercesc/framework/psvi/XSValue.hpp>
#include <xercesc/framework/psvi/XSComplexTypeDefinition.hpp>
#include <xercesc/validators/datatype/DatatypeValidatorFactory.hpp>

XERCES_CPP_NAMESPACE_BEGIN

PSVIItem::PSVIItem( MemoryManager* const manager ):  
        fMemoryManager(manager),
        fValidationContext(0),
        fNormalizedValue(0),
        fDefaultValue(0),
        fCanonicalValue(0),
        fValidityState(VALIDITY_NOTKNOWN),
        fAssessmentType(VALIDATION_FULL),
        fIsSpecified(false),
        fType(0),
        fMemberType(0)
{
}

void PSVIItem::reset(
            const XMLCh* const validationContext
            , const XMLCh* const normalizedValue
            , const VALIDITY_STATE validityState
            , const ASSESSMENT_TYPE assessmentType
        )
{
    // this is just a wrapper method; fValidationContext will
    // be valid as long as and no longer than the thing to which
    // validationContext points
    fValidationContext = validationContext;
    fNormalizedValue = normalizedValue;
    fValidityState = validityState;
    fAssessmentType = assessmentType;
}

void PSVIItem::setValidationAttempted(PSVIItem::ASSESSMENT_TYPE attemptType)
{
    fAssessmentType = attemptType;
}
 
void PSVIItem::setValidity(PSVIItem::VALIDITY_STATE validity)
{
    fValidityState = validity;
}

XSValue* PSVIItem::getActualValue() const
{
    /***
     * assessment 
	 *    VALIDATION_PARTIAL 
	 *    VALIDATION_FULL 
     * validity
	 *    VALIDITY_VALID
     ***/
    if ((fAssessmentType==VALIDATION_NONE) || (fValidityState!=VALIDITY_VALID))
        return 0;
    
    /***
     *  XSSimpleType or
     *  XSComplexType's CONTENTTYPE_SIMPLE
     *  allowed
     ***/
    if ((!fType) ||
        ((fType->getTypeCategory() == XSTypeDefinition::COMPLEX_TYPE) &&
         (((XSComplexTypeDefinition*)fType)->getContentType() != XSComplexTypeDefinition::CONTENTTYPE_SIMPLE)))
        return 0;
    
    /*** 
     * Resolve dv
     *
     * 1. If fMemberType is not null, use the fMemberType->fDataTypeValidator
     * 2. If fType is XSSimpleType, use fType->fDataTypeValidator
     * 3. If fType is XSComplexType, use fType->fXSSimpleTypeDefinition-> fDataTypeValidator
     *
    ***/

    DatatypeValidator *dv = 0;

     if (fMemberType)
     {
         /***
          *  Now that fType is either XSSimpleTypeDefinition or
          *  XSComlextTypeDefinition with CONTENTTYPE_SIMPLE, the
          *  fMemberType must be XSSimpleTypeDefinition if present
         ***/
         dv=((XSSimpleTypeDefinition*) fMemberType)->getDatatypeValidator();
     }
     else if (fType->getTypeCategory() == XSTypeDefinition::SIMPLE_TYPE)
     {
         dv=((XSSimpleTypeDefinition*) fType)->getDatatypeValidator();
     }
     else
     {
         XSSimpleTypeDefinition* simType = ((XSComplexTypeDefinition*)fType)->getSimpleType();
         if (simType)
             dv = simType->getDatatypeValidator();
     }

     if (!dv) return 0;

     /***
      * Get the ultimate base dv in the datatype registry
      ***/
     DatatypeValidator *basedv = DatatypeValidatorFactory::getBuiltInBaseValidator(dv);

     if (!basedv) return 0;
    
     XSValue::Status  status=XSValue::st_Init;

     return XSValue::getActualValue(fNormalizedValue
                                  , XSValue::getDataType(basedv->getTypeLocalName())
                                  , status
                                  , XSValue::ver_10
                                  , false
                                  , fMemoryManager);


}

XERCES_CPP_NAMESPACE_END


