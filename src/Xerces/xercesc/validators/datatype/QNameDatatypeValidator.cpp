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
 * $Id: QNameDatatypeValidator.cpp 676911 2008-07-15 13:27:32Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/QNameDatatypeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/internal/ValidationContextImpl.hpp>
#include <xercesc/util/XMLChar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
QNameDatatypeValidator::QNameDatatypeValidator(MemoryManager* const manager)
:AbstractStringValidator(0, 0, 0, DatatypeValidator::QName, manager)
{}

QNameDatatypeValidator::~QNameDatatypeValidator()
{}

QNameDatatypeValidator::QNameDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager)
:AbstractStringValidator(baseValidator, facets, finalSet, DatatypeValidator::QName, manager)
{
    init(enums, manager);
}

DatatypeValidator* QNameDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) QNameDatatypeValidator(this, facets, enums, finalSet, manager);
}

// ---------------------------------------------------------------------------
//  Utilities
// ---------------------------------------------------------------------------

void QNameDatatypeValidator::checkValueSpace(const XMLCh* const content
                                             , MemoryManager* const manager)
{
    //
    // check 3.2.18.c0 must: QName
    //

    if ( !XMLChar1_0::isValidQName(content, XMLString::stringLen(content)) )
    {
        ThrowXMLwithMemMgr1(InvalidDatatypeValueException
                , XMLExcepts::VALUE_QName_Invalid
                , content
                , manager);
    }
}

void QNameDatatypeValidator::checkContent( const XMLCh*             const content
                                          ,       ValidationContext* const context
                                          ,       bool                     asBase
                                          ,       MemoryManager*     const manager
                                          )
{

    //validate against base validator if any
    QNameDatatypeValidator *pBaseValidator = (QNameDatatypeValidator*) this->getBaseValidator();
    if (pBaseValidator)
        pBaseValidator->checkContent(content, context, true, manager);

    int thisFacetsDefined = getFacetsDefined();

    // we check pattern first
    if ( (thisFacetsDefined & DatatypeValidator::FACET_PATTERN ) != 0 )
    {
        if (getRegex()->matches(content, manager) ==false)
        {
            ThrowXMLwithMemMgr2(InvalidDatatypeValueException
                    , XMLExcepts::VALUE_NotMatch_Pattern
                    , content
                    , getPattern()
                    , manager);
        }
    }

    // if this is a base validator, we only need to check pattern facet
    // all other facet were inherited by the derived type
    if (asBase)
        return;

    checkValueSpace(content, manager);

    int colonPos = 0;
    XMLCh* prefix = 0;    
    ArrayJanitor<XMLCh>  jan(prefix, manager);

    if (context) {
        prefix = XMLString::replicate(content, manager);
        jan.reset(prefix, manager);
        normalizeContent(prefix, manager);

        colonPos = XMLString::indexOf(content, chColon);
        if (colonPos > 0) {                        
            prefix[colonPos] = chNull;                     
            if (context->isPrefixUnknown(prefix)) {
                ThrowXMLwithMemMgr1(InvalidDatatypeValueException
                    , XMLExcepts::VALUE_QName_Invalid2
                    , content
                    , manager);             
            }                                  
        }
    }

#if 0
    if ((thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) != 0 &&
        (getEnumeration() != 0))
    {
        XMLCh* normContent = XMLString::replicate(content, manager);
        ArrayJanitor<XMLCh>  jan(normContent, manager);
        normalizeContent(normContent, manager);

        int i=0;
        int enumLength = getEnumeration()->size();
        for ( ; i < enumLength; i++)
        {
            if (XMLString::equals(normContent, getEnumeration()->elementAt(i)))
                break;
        }

        if (i == enumLength)
            ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_NotIn_Enumeration, content, manager);
    }


#else
    if ((thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) != 0 &&
        (getEnumeration() != 0) && context)
    {
        XMLCh* localName;
        if (colonPos > 0) {
            localName = prefix + colonPos + 1;
        }
        else {
            localName = prefix;
        }

        XMLCh* enumPrefix;
        XMLCh* enumLocalName;
        XMLSize_t i=0;
        XMLSize_t enumLength = getEnumeration()->size();
        bool foundURIId = false;
        const XMLCh* normURI = 0;
		// The +=2 is because the enumeration has prefix:localname as one entry followed
		// by the URI string for the prefix as the next entry.
        for ( ; i < enumLength; i+=2)
        {            
            enumPrefix = XMLString::replicate(getEnumeration()->elementAt(i), manager);
            ArrayJanitor<XMLCh>  janEnum(enumPrefix, manager);
            colonPos = XMLString::indexOf(enumPrefix, chColon, 0, manager);
            
            if (colonPos != -1) {
                enumLocalName = enumPrefix + colonPos + 1;
                enumPrefix[colonPos] = chNull;
            }
            else {
                enumLocalName = enumPrefix;
            }
            
            if (XMLString::equals(localName, enumLocalName)) {               
				if (colonPos < 0)
					break;

                // now need to see if the prefix URI's are the same                
                if (!foundURIId) {                    
                    normURI = context->getURIForPrefix(prefix);                                       
                    foundURIId = true;
                }
				if (XMLString::equals(normURI, getEnumeration()->elementAt(i+1)))
					break;
            }        
        }

        if (i == enumLength)
            ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_NotIn_Enumeration, content, manager);
    }
#endif

    checkAdditionalFacet(content, manager);
}

//
//  Check vs base
//         check common facets
//         check enumeration
//         check Additional Facet Constraint
//
void QNameDatatypeValidator::inspectFacetBase(MemoryManager* const manager)
{

    QNameDatatypeValidator *pBaseValidator = (QNameDatatypeValidator*) getBaseValidator();
    int thisFacetsDefined = getFacetsDefined();

    if ( (!thisFacetsDefined && !getEnumeration()) ||
         (!pBaseValidator)                      )
        return;

    // check 4.3.5.c0 must: enumeration values from the value space of base
    if ( ((thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) != 0) &&
        (getEnumeration() !=0))
    {
        XMLSize_t i = 0;
        XMLSize_t enumLength = getEnumeration()->size();
		// The +=2 is because the enumeration has prefix:localname as one entry followed
		// by the URI string for the prefix as the next entry.
        for ( ; i < enumLength; i+=2)
        {
            // ask parent do a complete check
            pBaseValidator->checkContent(getEnumeration()->elementAt(i), (ValidationContext*)0, false, manager);
#if 0
// spec says that only base has to checkContent          
            // enum shall pass this->checkContent() as well.
            checkContent(getEnumeration()->elementAt(i), (ValidationContext*)0, false, manager);
#endif
        }
    }

    checkAdditionalFacetConstraints(manager);

} //end of inspectFacetBase

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(QNameDatatypeValidator)

void QNameDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    AbstractStringValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file QNameDatatypeValidator.cpp
  */
