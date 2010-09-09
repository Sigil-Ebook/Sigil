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

#include "DOMConfigurationImpl.hpp"
#include "DOMStringListImpl.hpp"
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/dom/DOMException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

const unsigned short DOMConfigurationImpl::fDEFAULT_VALUES = 0x2596;

DOMConfigurationImpl::DOMConfigurationImpl(MemoryManager* const manager): featureValues(fDEFAULT_VALUES),
                                              fErrorHandler(0), fSchemaType(0), fSchemaLocation(0),
                                              fSupportedParameters(0), fMemoryManager(manager)
{
    fSupportedParameters=new (fMemoryManager) DOMStringListImpl(17, fMemoryManager);
    fSupportedParameters->add(XMLUni::fgDOMErrorHandler);
    fSupportedParameters->add(XMLUni::fgDOMSchemaType);
    fSupportedParameters->add(XMLUni::fgDOMSchemaLocation);
    fSupportedParameters->add(XMLUni::fgDOMCanonicalForm);
    fSupportedParameters->add(XMLUni::fgDOMCDATASections);
    fSupportedParameters->add(XMLUni::fgDOMComments);
    fSupportedParameters->add(XMLUni::fgDOMDatatypeNormalization);
    fSupportedParameters->add(XMLUni::fgDOMWRTDiscardDefaultContent);
    fSupportedParameters->add(XMLUni::fgDOMEntities);
    fSupportedParameters->add(XMLUni::fgDOMInfoset);
    fSupportedParameters->add(XMLUni::fgDOMNamespaces);
    fSupportedParameters->add(XMLUni::fgDOMNamespaceDeclarations);
    fSupportedParameters->add(XMLUni::fgDOMNormalizeCharacters);
    fSupportedParameters->add(XMLUni::fgDOMSplitCDATASections);
    fSupportedParameters->add(XMLUni::fgDOMValidate);
    fSupportedParameters->add(XMLUni::fgDOMValidateIfSchema);
    fSupportedParameters->add(XMLUni::fgDOMElementContentWhitespace);
}

DOMConfigurationImpl::~DOMConfigurationImpl() {
    delete fSupportedParameters;
}
                                        
void DOMConfigurationImpl::setParameter(const XMLCh* name, const void* value) {
    if(!canSetParameter(name, value)) {
        throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, fMemoryManager);
    }

    if(XMLString::compareIStringASCII(name, XMLUni::fgDOMErrorHandler)==0) {
        fErrorHandler = (DOMErrorHandler*)value;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaType)==0) {
        fSchemaType = (XMLCh*)value;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaLocation)==0) {
        fSchemaLocation = (XMLCh*)value;
    } else {  // canSetParameter above should take care of this case
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, fMemoryManager);
    }

}

void DOMConfigurationImpl::setParameter(const XMLCh* name, bool value) {
    if(!canSetParameter(name, value)) {
        throw DOMException(DOMException::NOT_SUPPORTED_ERR, 0, fMemoryManager);
    }

    DOMConfigurationFeature whichFlag = getFeatureFlag(name);
    if(value) {
        featureValues |= whichFlag;
    } else {
        featureValues &= ~whichFlag;
    }

}

// --------------------------------------
// Getter Methods
// --------------------------------------

const void* DOMConfigurationImpl::getParameter(const XMLCh* name) const {
    DOMConfigurationFeature whichFlag;
    try {
        whichFlag = getFeatureFlag(name);
        if(featureValues & whichFlag) {
            return (void*)true;
        } else {
            return (void*)false;
        }
   } catch (DOMException&) {
        // must not be a boolean parameter
        if(XMLString::compareIStringASCII(name, XMLUni::fgDOMErrorHandler)==0) {
            return fErrorHandler;
        } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaType)==0) {
            return fSchemaType;
        } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaLocation)==0) {
            return fSchemaLocation;
        } else {
            throw DOMException(DOMException::NOT_FOUND_ERR, 0, fMemoryManager);
        }
    }

}

// -----------------------------------------
// Query Methods
// -----------------------------------------

bool DOMConfigurationImpl::canSetParameter(const XMLCh* name, const void* /*value*/) const {

    /**
     * canSetParameter(name, value) returns false in two conditions:
     *  1) if a [required] feature has no supporting code, then return false in 
     *     both the true and false outcomes (This is in order to be either fully 
     *     spec compliant, or not at all)
     *  2) if an [optional] feature has no supporting code, then return false
     **/ 
    
    if(XMLString::compareIStringASCII(name, XMLUni::fgDOMErrorHandler)==0) {
        return true;                               // required //
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaType)==0) {
        return false;                            // optional //
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSchemaLocation)==0) {
        return false;                            // optional //
    } 
    return false;
}

bool DOMConfigurationImpl::canSetParameter(const XMLCh* name, bool booleanValue) const {
    /**
     * canSetParameter(name, value) returns false in two conditions:
     *  1) if a [required] feature has no supporting code, then return false in 
     *     both the true and false outcomes (This is in order to be either fully 
     *     spec compliant, or not at all)
     *  2) if an [optional] feature has no supporting code, then return false
     **/ 
    
    DOMConfigurationFeature whichFlag = getFeatureFlag(name);
    switch (whichFlag) {
        case FEATURE_CANONICAL_FORM: 
            if(booleanValue) return false;      // optional //
            else             return true;       // required // 
        case FEATURE_CDATA_SECTIONS: 
            return true;
        case FEATURE_COMMENTS:  
            return true;
        case FEATURE_DATATYPE_NORMALIZATION:  
            if(booleanValue) return false;       // required //
            else             return true;        // required //
        case FEATURE_DISCARD_DEFAULT_CONTENT:  
            if(booleanValue) return false;       // required //
            else             return true;        // required //
        case FEATURE_ENTITIES:  
            if(booleanValue) return true;       // required //
            else             return true;       // required //
        case FEATURE_INFOSET:  
            if(booleanValue) return false;       // required //
            else             return true;       // no effect//
        case FEATURE_NAMESPACES:  
            return true;       
        case FEATURE_NAMESPACE_DECLARATIONS:  
            if(booleanValue) return true;      // optional //
            else             return false;       // required //
        case FEATURE_NORMALIZE_CHARACTERS:  
            if(booleanValue) return false;      // optional //
            else             return true;       // required //
        case FEATURE_SPLIT_CDATA_SECTIONS:  
            //we dont report an error in the false case so we cant claim we do it
            if(booleanValue) return false;       // required //
            else             return false;       // required //
        case FEATURE_VALIDATE:  
            if(booleanValue) return false;      // optional //
            else             return true;       // required //
        case FEATURE_VALIDATE_IF_SCHEMA:  
            if(booleanValue) return false;      // optional //
            else             return true;       // required //
          
        case FEATURE_ELEMENT_CONTENT_WHITESPACE:  
            if(booleanValue) return true;       // required //
            else             return false;      // optional //
    }
	// should never be here
    return false;
}

const DOMStringList* DOMConfigurationImpl::getParameterNames() const
{
    return fSupportedParameters;
}

// -------------------------------------------
// Impl methods
// -------------------------------------------

DOMConfigurationImpl::DOMConfigurationFeature DOMConfigurationImpl::getFeatureFlag(const XMLCh* name) const {
    if(XMLString::compareIStringASCII(name, XMLUni::fgDOMCanonicalForm)==0) {
        return FEATURE_CANONICAL_FORM;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMCDATASections )==0) {
        return FEATURE_CDATA_SECTIONS;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMComments)==0) {
        return FEATURE_COMMENTS;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMDatatypeNormalization)==0)  {
        return FEATURE_DATATYPE_NORMALIZATION;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMWRTDiscardDefaultContent)==0) {
        return FEATURE_DISCARD_DEFAULT_CONTENT;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMEntities)==0) {
        return FEATURE_ENTITIES;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMInfoset)==0)  {
        return FEATURE_INFOSET;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMNamespaces)==0) {
        return FEATURE_NAMESPACES;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMNamespaceDeclarations)==0) {
        return FEATURE_NAMESPACE_DECLARATIONS;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMNormalizeCharacters)==0) {
        return FEATURE_NORMALIZE_CHARACTERS;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMSplitCDATASections)==0) {
        return FEATURE_SPLIT_CDATA_SECTIONS;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMValidate)==0) {
        return FEATURE_VALIDATE;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMValidateIfSchema)==0) {
        return FEATURE_VALIDATE_IF_SCHEMA;
    } else if (XMLString::compareIStringASCII(name, XMLUni::fgDOMElementContentWhitespace)==0) {
        return FEATURE_ELEMENT_CONTENT_WHITESPACE;
    } else {
        throw DOMException(DOMException::NOT_FOUND_ERR, 0, fMemoryManager);
    }
        
}

DOMErrorHandler* DOMConfigurationImpl::getErrorHandler() const {
    return fErrorHandler;
}

const XMLCh* DOMConfigurationImpl::getSchemaType() const {
    return fSchemaType;
}

const XMLCh* DOMConfigurationImpl::getSchemaLocation() const {
    return fSchemaLocation;
}

void DOMConfigurationImpl::setErrorHandler(DOMErrorHandler *erHandler) {
    fErrorHandler = erHandler;
}

void DOMConfigurationImpl::setSchemaType(const XMLCh* st) {
    fSchemaType = st;
}

void DOMConfigurationImpl::setSchemaLocation(const XMLCh* sl) {
    fSchemaLocation = sl;
}


XERCES_CPP_NAMESPACE_END


/**
 * End of file DOMConfigurationImpl.cpp
 */
