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

#include "DOMTypeInfoImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include <xercesc/framework/psvi/PSVIItem.hpp>
#include <xercesc/framework/psvi/XSTypeDefinition.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedElement;
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdNotValidatedAttribute;
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedCDATAAttribute(XMLUni::fgInfosetURIName, XMLUni::fgCDATAString);
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedIDAttribute(XMLUni::fgInfosetURIName, XMLUni::fgIDString);
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedIDREFAttribute(XMLUni::fgInfosetURIName, XMLUni::fgIDRefString);
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedIDREFSAttribute(XMLUni::fgInfosetURIName, XMLUni::fgIDRefsString);
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedENTITYAttribute(XMLUni::fgInfosetURIName, XMLUni::fgEntityString);
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedENTITIESAttribute(XMLUni::fgInfosetURIName, XMLUni::fgEntitiesString);
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedNMTOKENAttribute(XMLUni::fgInfosetURIName, XMLUni::fgNmTokenString);
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedNMTOKENSAttribute(XMLUni::fgInfosetURIName, XMLUni::fgNmTokensString);
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedNOTATIONAttribute(XMLUni::fgInfosetURIName, XMLUni::fgNotationString);
/*static*/ DOMTypeInfoImpl DOMTypeInfoImpl::g_DtdValidatedENUMERATIONAttribute(XMLUni::fgInfosetURIName, XMLUni::fgEnumerationString);

DOMTypeInfoImpl::DOMTypeInfoImpl(const XMLCh* namespaceUri/*=0*/, const XMLCh* name/*=0*/)
: fBitFields(0),
  fTypeName(name),
  fTypeNamespace(namespaceUri),
  fMemberTypeName(0),
  fMemberTypeNamespace(0),
  fDefaultValue(0),
  fNormalizedValue(0)
{
    // by setting the fBitField to 0 we are also setting:
    //  - [validity]=VALIDITY_NOTKNOWN
    //  - [validitation attempted]=VALIDATION_NONE
    //  - [schema specified]=false
}

DOMTypeInfoImpl::DOMTypeInfoImpl(DOMDocumentImpl* ownerDoc, const DOMPSVITypeInfo* sourcePSVI)
: fBitFields(0),
  fTypeName(0),
  fTypeNamespace(0),
  fMemberTypeName(0),
  fMemberTypeNamespace(0),
  fDefaultValue(0),
  fNormalizedValue(0)
{
    setNumericProperty(DOMPSVITypeInfo::PSVI_Validity,
        sourcePSVI->getNumericProperty(DOMPSVITypeInfo::PSVI_Validity));
    setNumericProperty(DOMPSVITypeInfo::PSVI_Validation_Attempted,
        sourcePSVI->getNumericProperty(DOMPSVITypeInfo::PSVI_Validation_Attempted));
    setNumericProperty(DOMPSVITypeInfo::PSVI_Type_Definition_Type,
        sourcePSVI->getNumericProperty(DOMPSVITypeInfo::PSVI_Type_Definition_Type));
    setNumericProperty(DOMPSVITypeInfo::PSVI_Type_Definition_Anonymous,
        sourcePSVI->getNumericProperty(DOMPSVITypeInfo::PSVI_Type_Definition_Anonymous));
    setNumericProperty(DOMPSVITypeInfo::PSVI_Nil,
        sourcePSVI->getNumericProperty(DOMPSVITypeInfo::PSVI_Nil));
    setNumericProperty(DOMPSVITypeInfo::PSVI_Member_Type_Definition_Anonymous,
        sourcePSVI->getNumericProperty(DOMPSVITypeInfo::PSVI_Member_Type_Definition_Anonymous));
    setNumericProperty(DOMPSVITypeInfo::PSVI_Schema_Specified,
        sourcePSVI->getNumericProperty(DOMPSVITypeInfo::PSVI_Schema_Specified));

    setStringProperty(DOMPSVITypeInfo::PSVI_Type_Definition_Name,
        ownerDoc->getPooledString(sourcePSVI->getStringProperty(DOMPSVITypeInfo::PSVI_Type_Definition_Name)));
    setStringProperty(DOMPSVITypeInfo::PSVI_Type_Definition_Namespace,
        ownerDoc->getPooledString(sourcePSVI->getStringProperty(DOMPSVITypeInfo::PSVI_Type_Definition_Namespace)));
    setStringProperty(DOMPSVITypeInfo::PSVI_Member_Type_Definition_Name,
        ownerDoc->getPooledString(sourcePSVI->getStringProperty(DOMPSVITypeInfo::PSVI_Member_Type_Definition_Name)));
    setStringProperty(DOMPSVITypeInfo::PSVI_Member_Type_Definition_Namespace,
        ownerDoc->getPooledString(sourcePSVI->getStringProperty(DOMPSVITypeInfo::PSVI_Member_Type_Definition_Namespace)));
    setStringProperty(DOMPSVITypeInfo::PSVI_Schema_Default,
        ownerDoc->getPooledString(sourcePSVI->getStringProperty(DOMPSVITypeInfo::PSVI_Schema_Default)));
    setStringProperty(DOMPSVITypeInfo::PSVI_Schema_Normalized_Value,
        ownerDoc->getPooledString(sourcePSVI->getStringProperty(DOMPSVITypeInfo::PSVI_Schema_Normalized_Value)));
}

const XMLCh* DOMTypeInfoImpl::getTypeName() const {
    // if it's a DTD, return the data that was stored
    if(!getNumericProperty(PSVI_Schema_Specified))
        return fTypeName;
    // if [validity] is "invalid" or "notKnown", the {target namespace} and {name} properties of the declared type if available, otherwise null.
    if(!getNumericProperty(PSVI_Validity))
        return fTypeName;
    if(fMemberTypeName)
        return fMemberTypeName;
    return fTypeName;
}

const XMLCh* DOMTypeInfoImpl::getTypeNamespace() const {
    // if it's a DTD, return the data that was stored
    if(!getNumericProperty(PSVI_Schema_Specified))
        return fTypeNamespace;
    // if [validity] is "invalid" or "notKnown", the {target namespace} and {name} properties of the declared type if available, otherwise null.
    if(!getNumericProperty(PSVI_Validity))
        return fTypeNamespace;
    if(fMemberTypeName)     // we check on the name, as the URI can be NULL
        return fMemberTypeNamespace;
    return fTypeNamespace;
}

bool DOMTypeInfoImpl::isDerivedFrom(const XMLCh* typeNamespaceArg, const XMLCh* typeNameArg, DerivationMethods) const
{
    // if it's a DTD, return false
    if(!getNumericProperty(PSVI_Schema_Specified))
        return false;
    if(XMLString::equals(typeNamespaceArg, getTypeNamespace()) && XMLString::equals(typeNameArg, getTypeName()))
        return true;
    // TODO: need a pointer to the Grammar object
    return false;
}

const XMLCh* DOMTypeInfoImpl::getStringProperty(PSVIProperty prop) const {
    switch(prop)
    {
    case PSVI_Type_Definition_Name:             return fTypeName;
    case PSVI_Type_Definition_Namespace:        return fTypeNamespace;
    case PSVI_Member_Type_Definition_Name:      return fMemberTypeName;
    case PSVI_Member_Type_Definition_Namespace: return fMemberTypeNamespace;
    case PSVI_Schema_Default:                   return fDefaultValue;
    case PSVI_Schema_Normalized_Value:          return fNormalizedValue;
    default:                                    assert(false); /* it's not a string property */
    }
    return 0;
}

int DOMTypeInfoImpl::getNumericProperty(PSVIProperty prop) const {
    switch(prop)
    {
    case PSVI_Validity:                         return (PSVIItem::VALIDITY_STATE)(fBitFields & 0x0003);
    case PSVI_Validation_Attempted:             return (PSVIItem::ASSESSMENT_TYPE)((fBitFields >> 2) & 0x0003);
    case PSVI_Type_Definition_Type:             return (fBitFields & (1 << 5))?XSTypeDefinition::COMPLEX_TYPE:XSTypeDefinition::SIMPLE_TYPE;
    case PSVI_Type_Definition_Anonymous:        return (fBitFields & (1 << 6))?true:false;
    case PSVI_Nil:                              return (fBitFields & (1 << 7))?true:false;
    case PSVI_Member_Type_Definition_Anonymous: return (fBitFields & (1 << 8))?true:false;
    case PSVI_Schema_Specified:                 return (fBitFields & (1 << 9))?true:false;
    default:                                    assert(false); /* it's not a numeric property */
    }
    return 0;
}

void DOMTypeInfoImpl::setStringProperty(PSVIProperty prop, const XMLCh* value) {
    switch(prop)
    {
    case PSVI_Type_Definition_Name:             fTypeName=value; break;
    case PSVI_Type_Definition_Namespace:        fTypeNamespace=value; break;
    case PSVI_Member_Type_Definition_Name:      fMemberTypeName=value; break;
    case PSVI_Member_Type_Definition_Namespace: fMemberTypeNamespace=value; break;
    case PSVI_Schema_Default:                   fDefaultValue=value; break;
    case PSVI_Schema_Normalized_Value:          fNormalizedValue=value; break;
    default:                                    assert(false); /* it's not a string property */
    }
}

void DOMTypeInfoImpl::setNumericProperty(PSVIProperty prop, int value) {
    switch(prop)
    {
    case PSVI_Validity:                         fBitFields |= (value & 0x0003); break;
    case PSVI_Validation_Attempted:             fBitFields |= ((value & 0x0003) << 2); break;
    case PSVI_Type_Definition_Type:             fBitFields |= (value==XSTypeDefinition::COMPLEX_TYPE)?(1 << 5):0; break;
    case PSVI_Type_Definition_Anonymous:        fBitFields |= (value!=0)?(1 << 6):0; break;
    case PSVI_Nil:                              fBitFields |= (value!=0)?(1 << 7):0; break;
    case PSVI_Member_Type_Definition_Anonymous: fBitFields |= (value!=0)?(1 << 8):0; break;
    case PSVI_Schema_Specified:                 fBitFields |= (value!=0)?(1 << 9):0; break;
    default:                                    assert(false); /* it's not a numeric property */
    }
}


XERCES_CPP_NAMESPACE_END
/**
 * End of file DOMTypeInfo.cpp
 */
