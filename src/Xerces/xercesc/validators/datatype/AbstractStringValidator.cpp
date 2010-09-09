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
 * $Id: AbstractStringValidator.cpp 834826 2009-11-11 10:03:53Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/AbstractStringValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/util/NumberFormatException.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

static const int BUF_LEN = 64;

#define  REPORT_FACET_ERROR(val1, val2, except_code, manager)    \
    XMLCh value1[BUF_LEN+1]; \
    XMLCh value2[BUF_LEN+1]; \
   XMLString::sizeToText(val1, value1, BUF_LEN, 10, manager);     \
   XMLString::sizeToText(val2, value2, BUF_LEN, 10, manager);     \
   ThrowXMLwithMemMgr2(InvalidDatatypeFacetException              \
           , except_code                                \
           , value1                                     \
           , value2                                     \
           , manager);

#define  REPORT_VALUE_ERROR(data, val1, val2, except_code, manager)       \
    XMLCh value1[BUF_LEN+1]; \
    XMLCh value2[BUF_LEN+1]; \
   XMLString::sizeToText(val1, value1, BUF_LEN, 10, manager);             \
   XMLString::sizeToText(val2, value2, BUF_LEN, 10, manager);             \
   ThrowXMLwithMemMgr3(InvalidDatatypeValueException                      \
           , except_code                                        \
           , data                                               \
           , value1                                             \
           , value2                                             \
           , manager);

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
AbstractStringValidator::~AbstractStringValidator()
{
    //~RefVectorOf will delete all adopted elements
    if ( !fEnumerationInherited && fEnumeration)
    {
        delete fEnumeration;
        fEnumeration = 0;
    }
}

AbstractStringValidator::AbstractStringValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , const int                           finalSet
                        , const ValidatorType                 type
                        , MemoryManager* const                manager)
:DatatypeValidator(baseValidator, facets, finalSet, type, manager)
,fLength(0)
,fMaxLength(SchemaSymbols::fgINT_MAX_VALUE)
,fMinLength(0)
,fEnumerationInherited(false)
,fEnumeration(0)
{
    // init() is invoked from derived class's ctor instead of from
    // here to allow correct resolution of virutal method, such as
    // assigneAdditionalFacet(), inheritAdditionalFacet().
}

void AbstractStringValidator::init(RefArrayVectorOf<XMLCh>*           const enums
                                   ,MemoryManager*                    const manager)
{

    if (enums)
    {
        setEnumeration(enums, false);
        normalizeEnumeration(manager);
    }

    assignFacet(manager);
    inspectFacet(manager);
    inspectFacetBase(manager);
    inheritFacet();

}

//
//   Assign facets
//        assign common facets
//        assign additional facet
//
void AbstractStringValidator::assignFacet(MemoryManager* const manager)
{

    RefHashTableOf<KVStringPair>* facets = getFacets();

    if (!facets)
        return;

    XMLCh* key;
    RefHashTableOfEnumerator<KVStringPair> e(facets, false, manager);

    while (e.hasMoreElements())
    {
        KVStringPair pair = e.nextElement();
        key = pair.getKey();
        XMLCh* value = pair.getValue();

        if (XMLString::equals(key, SchemaSymbols::fgELT_LENGTH))
        {
            int val;
            try
            {
                val = XMLString::parseInt(value, manager);
            }
            catch (NumberFormatException&)
            {
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_Invalid_Len, value, manager);
            }

            if ( val < 0 )
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_NonNeg_Len, value, manager);

            setLength(val);
            setFacetsDefined(DatatypeValidator::FACET_LENGTH);
        }
        else if (XMLString::equals(key, SchemaSymbols::fgELT_MINLENGTH))
        {
            int val;
            try
            {
                val = XMLString::parseInt(value, manager);
            }
            catch (NumberFormatException&)
            {
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_Invalid_minLen, value, manager);
            }

            if ( val < 0 )
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_NonNeg_minLen, value, manager);

            setMinLength(val);
            setFacetsDefined(DatatypeValidator::FACET_MINLENGTH);
        }
        else if (XMLString::equals(key, SchemaSymbols::fgELT_MAXLENGTH))
        {
            int val;
            try
            {
                val = XMLString::parseInt(value, manager);
            }
            catch (NumberFormatException&)
            {
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_Invalid_maxLen, value, manager);
            }

            if ( val < 0 )
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException, XMLExcepts::FACET_NonNeg_maxLen, value, manager);

            setMaxLength(val);
            setFacetsDefined(DatatypeValidator::FACET_MAXLENGTH);
        }
        else if (XMLString::equals(key, SchemaSymbols::fgELT_PATTERN))
        {
            setPattern(value);
            if (getPattern())
                setFacetsDefined(DatatypeValidator::FACET_PATTERN);
            // do not construct regex until needed
        }
        else if (XMLString::equals(key, SchemaSymbols::fgATT_FIXED))
        {
            unsigned int val;
            bool         retStatus;
            try
            {
                retStatus = XMLString::textToBin(value, val, fMemoryManager);
            }
            catch (RuntimeException&)
            {
                ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_internalError_fixed, manager);
            }

            if (!retStatus)
            {
                ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_internalError_fixed, manager);
            }

            setFixed(val);
            //no setFacetsDefined here
        }
        //
        // else if (XMLString::equals(key, SchemaSymbols::fgELT_SPECIAL_TOKEN))
        // TODO
        //
        // Note: whitespace is taken care of by TraverseSchema.
        //
        else
        {
            assignAdditionalFacet(key, value, manager);
        }
    }//while
}//end of assigneFacet()

//
// Check facet among self
//         check common facets
//         check Additional Facet Constraint
//
void AbstractStringValidator::inspectFacet(MemoryManager* const manager)
{

    int thisFacetsDefined = getFacetsDefined();

    if (!thisFacetsDefined)
        return;

    // check 4.3.1.c1 error: length & (maxLength | minLength)
    if ((thisFacetsDefined & DatatypeValidator::FACET_LENGTH) != 0)
    {
        if ((thisFacetsDefined & DatatypeValidator::FACET_MAXLENGTH) != 0)
            ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_Len_maxLen, manager);
        else if (((thisFacetsDefined & DatatypeValidator::FACET_MINLENGTH) != 0))
            ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_Len_minLen, manager);
    }

    // check 4.3.2.c1 must: minLength <= maxLength
    if ((thisFacetsDefined & (DatatypeValidator::FACET_MINLENGTH
        |DatatypeValidator::FACET_MAXLENGTH)) != 0)
    {
        XMLSize_t thisMinLength = getMinLength();
        XMLSize_t thisMaxLength = getMaxLength();
        if ( thisMinLength > thisMaxLength )
        {
            REPORT_FACET_ERROR(thisMaxLength
                             , thisMinLength
                             , XMLExcepts::FACET_maxLen_minLen
                             , manager)
        }
    }

}// end of inspectFacet()

//
//  Check vs base
//         check common facets
//         check enumeration
//         check Additional Facet Constraint
//
void AbstractStringValidator::inspectFacetBase(MemoryManager* const manager)
{

    AbstractStringValidator *pBaseValidator = (AbstractStringValidator*) getBaseValidator();
    int thisFacetsDefined = getFacetsDefined();

    if ( (!thisFacetsDefined && !fEnumeration) ||
         (!pBaseValidator)                      )
        return;

    int baseFacetsDefined = pBaseValidator->getFacetsDefined();

    XMLSize_t thisLength    = getLength();
    XMLSize_t thisMinLength = getMinLength();
    XMLSize_t thisMaxLength = getMaxLength();

    XMLSize_t baseLength    = pBaseValidator->getLength();
    XMLSize_t baseMinLength = pBaseValidator->getMinLength();
    XMLSize_t baseMaxLength = pBaseValidator->getMaxLength();
    int baseFixed     = pBaseValidator->getFixed();

    /***
       check facets against base.facets
       Note: later we need to check the "fix" option of the base type
            and apply that to every individual facet.
    ***/

    /***
                Non coexistence of derived' length and base'    (minLength | maxLength)
                                   base'    length and derived' (minLength | maxLength)

     E2-35
     It is an error for both length and either of minLength or maxLength to be members of {facets},
     unless they are specified in different derivation steps in which case the following must be true:
     the {value} of minLength <= the {value} of length <= the {value} of maxLength
    ***/

    // error: length > base.maxLength
    //        length < base.minLength
    if ((thisFacetsDefined & DatatypeValidator::FACET_LENGTH) !=0)
    {
        if (((baseFacetsDefined & DatatypeValidator::FACET_MAXLENGTH) !=0) &&
             (thisLength > baseMaxLength)                                   )
        {
            REPORT_FACET_ERROR(thisLength
                             , baseMaxLength
                             , XMLExcepts::FACET_Len_baseMaxLen
                             , manager)
        }

        if (((baseFacetsDefined & DatatypeValidator::FACET_MINLENGTH) !=0) &&
             (thisLength < baseMinLength)                                   )
        {
            REPORT_FACET_ERROR(thisLength
                             , baseMinLength
                             , XMLExcepts::FACET_Len_baseMinLen
                             , manager)
        }
    }

    // error: baseLength > maxLength
    //        baseLength < minLength
    if ((baseFacetsDefined & DatatypeValidator::FACET_LENGTH) !=0)
    {
        if (((thisFacetsDefined & DatatypeValidator::FACET_MAXLENGTH) !=0) &&
             (baseLength > thisMaxLength)                                   )
        {
            REPORT_FACET_ERROR(thisMaxLength
                             , baseLength
                             , XMLExcepts::FACET_maxLen_baseLen
                             , manager)
        }

        if (((thisFacetsDefined & DatatypeValidator::FACET_MINLENGTH) !=0) &&
             (baseLength < thisMinLength)                                   )
        {
            REPORT_FACET_ERROR(thisMinLength
                             , baseLength
                             , XMLExcepts::FACET_minLen_baseLen
                             , manager)
        }
    }

    // check 4.3.1.c2 error: length != base.length
    if (((thisFacetsDefined & DatatypeValidator::FACET_LENGTH) !=0) &&
        ((baseFacetsDefined & DatatypeValidator::FACET_LENGTH) !=0))
    {
        if ( thisLength != baseLength )
        {
            REPORT_FACET_ERROR(thisLength
                             , baseLength
                             , XMLExcepts::FACET_Len_baseLen
                             , manager)
        }
    }

    /***
                                   |---  derived   ---|
                base.minLength <= minLength <= maxLength <= base.maxLength
                |-------------------        base      -------------------|
    ***/

    // check 4.3.2.c1 must: minLength <= base.maxLength
    if (((thisFacetsDefined & DatatypeValidator::FACET_MINLENGTH ) != 0) &&
        ((baseFacetsDefined & DatatypeValidator::FACET_MAXLENGTH ) != 0))
    {
        if ( thisMinLength > baseMaxLength )
        {
            REPORT_FACET_ERROR(thisMinLength
                             , baseMaxLength
                             , XMLExcepts::FACET_minLen_basemaxLen
                             , manager)
        }
    }

    // check 4.3.2.c2 error: minLength < base.minLength
    if (((thisFacetsDefined & DatatypeValidator::FACET_MINLENGTH) !=0) &&
        ((baseFacetsDefined & DatatypeValidator::FACET_MINLENGTH) != 0))
    {
        if ((baseFixed & DatatypeValidator::FACET_MINLENGTH) !=0)
        {
            if ( thisMinLength != baseMinLength )
            {
                REPORT_FACET_ERROR(thisMinLength
                                 , baseMinLength
                                 , XMLExcepts::FACET_minLen_base_fixed
                                 , manager)
            }

        }
        else
        {
            if ( thisMinLength < baseMinLength )
            {
                REPORT_FACET_ERROR(thisMinLength
                                 , baseMinLength
                                 , XMLExcepts::FACET_minLen_baseminLen
                                 , manager)
            }
        }
    }

    // check 4.3.2.c1 must: base.minLength <= maxLength
    if (((baseFacetsDefined & DatatypeValidator::FACET_MINLENGTH) !=0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_MAXLENGTH) !=0))
    {
        if ( baseMinLength > thisMaxLength )
        {
            REPORT_FACET_ERROR(thisMaxLength
                             , baseMinLength
                             , XMLExcepts::FACET_maxLen_baseminLen
                             , manager)
        }
    }

    // check 4.3.3.c1 error: maxLength > base.maxLength
    if (((thisFacetsDefined & DatatypeValidator::FACET_MAXLENGTH) !=0) &&
        ((baseFacetsDefined & DatatypeValidator::FACET_MAXLENGTH) !=0))
    {
        if ((baseFixed & DatatypeValidator::FACET_MAXLENGTH) !=0)
        {
            if ( thisMaxLength != baseMaxLength )
            {
                REPORT_FACET_ERROR(thisMaxLength
                                 , baseMaxLength
                                 , XMLExcepts::FACET_maxLen_base_fixed
                                 , manager)
            }
        }
        else
        {
            if ( thisMaxLength > baseMaxLength )
            {
                REPORT_FACET_ERROR(thisMaxLength
                                 , baseMaxLength
                                 , XMLExcepts::FACET_maxLen_basemaxLen
                                 , manager)
            }
        }
    }

    // check 4.3.5.c0 must: enumeration values from the value space of base
    if ( ((thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) != 0) &&
        (getEnumeration() !=0))
    {
        XMLSize_t i = 0;
        XMLSize_t enumLength = getEnumeration()->size();
        for ( ; i < enumLength; i++)
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

//
//  Inherit facet from base
//    a. inherit common facets
//    b. inherit additional facet
//
void AbstractStringValidator::inheritFacet()
{
    /***
        P3. Inherit facets from base.facets

        The reason of this inheriting (or copying values) is to ease
        schema constraint checking, so that we need NOT trace back to our
        very first base validator in the hierachy. Instead, we are pretty
        sure checking against immediate base validator is enough.
    ***/

    AbstractStringValidator *pBaseValidator = (AbstractStringValidator*) getBaseValidator();

    if (!pBaseValidator)
        return;

    int thisFacetsDefined = getFacetsDefined();
    int baseFacetsDefined = pBaseValidator->getFacetsDefined();

    // inherit length
    if (((baseFacetsDefined & DatatypeValidator::FACET_LENGTH) != 0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_LENGTH) == 0))
    {
        setLength(pBaseValidator->getLength());
        setFacetsDefined(DatatypeValidator::FACET_LENGTH);
    }

    // inherit minLength
    if (((baseFacetsDefined & DatatypeValidator::FACET_MINLENGTH) !=0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_MINLENGTH) == 0))
    {
        setMinLength(pBaseValidator->getMinLength());
        setFacetsDefined(DatatypeValidator::FACET_MINLENGTH);
    }

    // inherit maxLength
    if (((baseFacetsDefined & DatatypeValidator::FACET_MAXLENGTH) !=0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_MAXLENGTH) == 0))
    {
        setMaxLength(pBaseValidator->getMaxLength());
        setFacetsDefined(DatatypeValidator::FACET_MAXLENGTH);
    }

    // inherit enumeration
    if (((baseFacetsDefined & DatatypeValidator::FACET_ENUMERATION) !=0) &&
        ((thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) == 0))
    {
        setEnumeration(pBaseValidator->getEnumeration(), true);
    }

    // we don't inherit pattern

    // inherit "fixed" option
    setFixed(getFixed() | pBaseValidator->getFixed());

    // inherit additional facet
    inheritAdditionalFacet();

} // end of inheritance


// -----------------------------------------------------------------------
// Compare methods
// -----------------------------------------------------------------------
int AbstractStringValidator::compare(const XMLCh* const lValue
                                   , const XMLCh* const rValue
                                   , MemoryManager*     const)
{
    return XMLString::compareString(lValue, rValue);
}

void AbstractStringValidator::validate( const XMLCh*             const content
                                      ,       ValidationContext* const context
                                      ,       MemoryManager*     const manager)
{
    checkContent(content, context, false, manager);
}

void AbstractStringValidator::checkContent( const XMLCh*             const content
                                          ,       ValidationContext* const context
                                          ,       bool                     asBase
                                          ,       MemoryManager*     const manager
                                          )
{

    //validate against base validator if any
    AbstractStringValidator *pBaseValidator = (AbstractStringValidator*) this->getBaseValidator();
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
    XMLSize_t length = getLength(content, manager);

    if (((thisFacetsDefined & DatatypeValidator::FACET_MAXLENGTH) != 0) &&
        (length > getMaxLength()))
    {
        REPORT_VALUE_ERROR(content
                         , length
                         , getMaxLength()
                         , XMLExcepts::VALUE_GT_maxLen
                         , manager)
    }

    if (((thisFacetsDefined & DatatypeValidator::FACET_MINLENGTH) != 0) &&
        (length < getMinLength()))
    {
        REPORT_VALUE_ERROR(content
                         , length
                         , getMinLength()
                         , XMLExcepts::VALUE_LT_minLen
                         , manager)
    }

    if (((thisFacetsDefined & DatatypeValidator::FACET_LENGTH) != 0) &&
        (length != getLength()))
    {
        REPORT_VALUE_ERROR(content
                         , length
                         , getLength()
                         , XMLExcepts::VALUE_NE_Len
                         , manager)
    }

    if ((thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) != 0 &&
        (getEnumeration() != 0))
    {
        XMLCh* normContent = XMLString::replicate(content, manager);
        ArrayJanitor<XMLCh>  jan(normContent, manager);
        normalizeContent(normContent, manager);

        XMLSize_t i=0;
        XMLSize_t enumLength = getEnumeration()->size();
        for ( ; i < enumLength; i++)
        {
            if (XMLString::equals(normContent, getEnumeration()->elementAt(i)))
                break;
        }

        if (i == enumLength)
            ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_NotIn_Enumeration, content, manager);
    }

    checkAdditionalFacet(content, manager);

}

const RefArrayVectorOf<XMLCh>* AbstractStringValidator::getEnumString() const
{
	return getEnumeration();
}

void AbstractStringValidator::normalizeEnumeration(MemoryManager* const manager)
{
    AbstractStringValidator *pBaseValidator = (AbstractStringValidator*) getBaseValidator();

    if (!fEnumeration || !pBaseValidator)
        return;

    int baseFacetsDefined = pBaseValidator->getFacetsDefined();
    if ((baseFacetsDefined & DatatypeValidator::FACET_WHITESPACE) == 0)
        return;

    short whiteSpace = pBaseValidator->getWSFacet();

    if ( whiteSpace == DatatypeValidator::PRESERVE )
    {
        return;
    }
    else if ( whiteSpace == DatatypeValidator::REPLACE )
    {
        XMLSize_t enumLength = getEnumeration()->size();
        for ( XMLSize_t i=0; i < enumLength; i++)
        {
            XMLString::replaceWS(getEnumeration()->elementAt(i), manager);
        }
    }
    else if ( whiteSpace == DatatypeValidator::COLLAPSE )
    {
        XMLSize_t enumLength = getEnumeration()->size();
        for ( XMLSize_t i=0; i < enumLength; i++)
        {
            XMLString::collapseWS(getEnumeration()->elementAt(i), manager);
        }
    }
}

void AbstractStringValidator::normalizeContent(XMLCh* const, MemoryManager* const) const
{
    // default implementation: do nothing
    return;
}


void AbstractStringValidator::checkAdditionalFacetConstraints(MemoryManager* const) const
{
    return;
}

void AbstractStringValidator::checkAdditionalFacet(const XMLCh* const
                                    , MemoryManager* const) const
{
    return;
}

void AbstractStringValidator::inheritAdditionalFacet()
{
    return;
}

void AbstractStringValidator::assignAdditionalFacet( const XMLCh* const key
                                                   , const XMLCh* const
                                                   , MemoryManager* const manager)
{
    ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
            , XMLExcepts::FACET_Invalid_Tag
            , key
            , manager);
}

XMLSize_t AbstractStringValidator::getLength(const XMLCh* const content
                                           , MemoryManager* const) const
{
    return XMLString::stringLen(content);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_NOCREATE(AbstractStringValidator)

void AbstractStringValidator::serialize(XSerializeEngine& serEng)
{

    DatatypeValidator::serialize(serEng);

    if (serEng.isStoring())
    {
        serEng.writeSize (fLength);
        serEng.writeSize (fMaxLength);
        serEng.writeSize (fMinLength);
        serEng<<fEnumerationInherited;

        /***
         *
         * Serialize RefArrayVectorOf<XMLCh>
         *
         ***/
        XTemplateSerializer::storeObject(fEnumeration, serEng);

    }
    else
    {
        serEng.readSize (fLength);
        serEng.readSize (fMaxLength);
        serEng.readSize (fMinLength);
        serEng>>fEnumerationInherited;

        /***
         *
         *  Deserialize RefArrayVectorOf<XMLCh>
         *
        ***/
        XTemplateSerializer::loadObject(&fEnumeration, 8, true, serEng);

    }

}

XERCES_CPP_NAMESPACE_END

/**
  * End of file AbstractStringValidator.cpp
  */
