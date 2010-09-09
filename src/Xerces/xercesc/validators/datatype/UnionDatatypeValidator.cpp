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
 * $Id: UnionDatatypeValidator.cpp 677559 2008-07-17 11:35:27Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/UnionDatatypeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

static const unsigned int BUF_LEN = 64;

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
UnionDatatypeValidator::UnionDatatypeValidator(MemoryManager* const manager)
:DatatypeValidator(0, 0, 0, DatatypeValidator::Union, manager)
,fEnumerationInherited(false)
,fMemberTypesInherited(false)
,fEnumeration(0)
,fMemberTypeValidators(0)

{}

UnionDatatypeValidator::~UnionDatatypeValidator()
{
    cleanUp();
}

UnionDatatypeValidator::UnionDatatypeValidator(
                        RefVectorOf<DatatypeValidator>* const memberTypeValidators
                      , const int                             finalSet
                      , MemoryManager* const                  manager)
:DatatypeValidator(0, 0, finalSet, DatatypeValidator::Union, manager)
,fEnumerationInherited(false)
,fMemberTypesInherited(false)
,fEnumeration(0)
,fMemberTypeValidators(0)
{
    if ( !memberTypeValidators )
    {
        ThrowXMLwithMemMgr(InvalidDatatypeFacetException
               , XMLExcepts::FACET_Union_Null_memberTypeValidators, manager);
    }

    // no pattern, no enumeration
    fMemberTypeValidators = memberTypeValidators;
}

typedef JanitorMemFunCall<UnionDatatypeValidator>   CleanupType;

UnionDatatypeValidator::UnionDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const                manager
                        , RefVectorOf<DatatypeValidator>* const memberTypeValidators 
                        , const bool memberTypesInherited
                        )
:DatatypeValidator(baseValidator, facets, finalSet, DatatypeValidator::Union, manager)
,fEnumerationInherited(false)
,fMemberTypesInherited(memberTypesInherited)
,fEnumeration(0)
,fMemberTypeValidators(memberTypeValidators)
{
    //
    // baseValidator another UnionDTV from which,
    // this UnionDTV is derived by restriction.
    // it shall be not null
    //
    if (!baseValidator)
    {
        ThrowXMLwithMemMgr(InvalidDatatypeFacetException
               , XMLExcepts::FACET_Union_Null_baseValidator, manager);
    }

    if (baseValidator->getType() != DatatypeValidator::Union)
    {
        XMLCh value1[BUF_LEN+1];
        XMLString::binToText(baseValidator->getType(), value1, BUF_LEN, 10, manager);
        ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
                , XMLExcepts::FACET_Union_invalid_baseValidatorType
                , value1
                , manager);
    }

    CleanupType cleanup(this, &UnionDatatypeValidator::cleanUp);

    try
    {
        init(baseValidator, facets, enums, manager);
    }
    catch(const OutOfMemoryException&)
    {
        // Don't cleanup when out of memory, since executing the
        // code can cause problems.
        cleanup.release();

        throw;
    }

    cleanup.release();
}

void UnionDatatypeValidator::init(DatatypeValidator*            const baseValidator
                                , RefHashTableOf<KVStringPair>* const facets
                                , RefArrayVectorOf<XMLCh>*      const enums
                                , MemoryManager*                const manager)
{
    if (enums)
        setEnumeration(enums, false);

    // Set Facets if any defined
    if (facets)
    {
        XMLCh* key;
        XMLCh* value;
        RefHashTableOfEnumerator<KVStringPair> e(facets, false, manager);

        while (e.hasMoreElements())
        {
            KVStringPair pair = e.nextElement();
            key = pair.getKey();
            value = pair.getValue();

            if (XMLString::equals(key, SchemaSymbols::fgELT_PATTERN))
            {
                setPattern(value);
                if (getPattern())
                    setFacetsDefined(DatatypeValidator::FACET_PATTERN);
                // do not construct regex until needed
            }
            else
            {
                 ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
                         , XMLExcepts::FACET_Invalid_Tag
                         , key
                         , manager);
            }
        }//while

        /***
           Schema constraint: Part I -- self checking
        ***/
        // Nil

        /***
           Schema constraint: Part II base vs derived checking
        ***/
        // check 4.3.5.c0 must: enumeration values from the value space of base
        if ( ((getFacetsDefined() & DatatypeValidator::FACET_ENUMERATION) != 0) &&
            (getEnumeration() !=0))
        {
            XMLSize_t i = 0;
            XMLSize_t enumLength = getEnumeration()->size();
            try
            {
                for ( ; i < enumLength; i++)
                {
                    // ask parent do a complete check
                    //
                    // enum need NOT be passed this->checkContent()
                    // since there are no other facets for Union, parent
                    // checking is good enough.
                    //
                    baseValidator->validate(getEnumeration()->elementAt(i), (ValidationContext*)0, manager);

                }
            }

            catch ( XMLException& )
            {
                ThrowXMLwithMemMgr1(InvalidDatatypeFacetException
                            , XMLExcepts::FACET_enum_base
                            , getEnumeration()->elementAt(i)
                            , manager);
            }
        }

    }// End of Facet setting

    /***
        Inherit facets from base.facets

        The reason of this inheriting (or copying values) is to ease
        schema constraint checking, so that we need NOT trace back to our
        very first base validator in the hierachy. Instead, we are pretty
        sure checking against immediate base validator is enough.
    ***/

    UnionDatatypeValidator *pBaseValidator = (UnionDatatypeValidator*) baseValidator;

    // inherit enumeration
    if (((pBaseValidator->getFacetsDefined() & DatatypeValidator::FACET_ENUMERATION) !=0) &&
        ((getFacetsDefined() & DatatypeValidator::FACET_ENUMERATION) == 0))
    {
        setEnumeration(pBaseValidator->getEnumeration(), true);
    }

}

//
// 1) the bottom level UnionDTV would check against
//        pattern and enumeration as well
// 2) each UnionDTV(s) above the bottom level UnionDTV and
//        below the native UnionDTV (the top level DTV)
//        would check against pattern only.
// 3) the natvie Union DTV (the top level DTV) would invoke
//        memberTypeValidator to validate
//
void UnionDatatypeValidator::checkContent(const XMLCh*             const content
                                        ,       ValidationContext* const context
                                        ,       bool                     asBase
                                        ,       MemoryManager*     const manager)
{

    DatatypeValidator* bv = getBaseValidator();
    if (bv)
        ((UnionDatatypeValidator*)bv)->checkContent(content, context, true, manager);
    else
    {   // 3) native union type
        // check content against each member type validator in Union
        // report an error only in case content is not valid against all member datatypes.
        //
        bool memTypeValid = false;
        for ( unsigned int i = 0; i < fMemberTypeValidators->size(); ++i )
        {
            if ( memTypeValid )
                break;

            try
            {
                fMemberTypeValidators->elementAt(i)->validate(content, context, manager);
                memTypeValid = true;
                
                //set the validator of the type actually used to validate the content
                DatatypeValidator *dtv = fMemberTypeValidators->elementAt(i);                
                // context will be null during schema construction
                if(context)
                    context->setValidatingMemberType(dtv);
            }
            catch (XMLException&)
            {
                //absorbed
            }
        } // for

        if ( !memTypeValid )
        {
            ThrowXMLwithMemMgr1(InvalidDatatypeValueException
                    , XMLExcepts::VALUE_no_match_memberType
                    , content
                    , manager);
            //( "Content '"+content+"' does not match any union types" );
        }
    }

    // 1) and 2). we check pattern first
    if ( (getFacetsDefined() & DatatypeValidator::FACET_PATTERN ) != 0 )
    {
        if (getRegex()->matches(content, manager) == false)
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

    if ((getFacetsDefined() & DatatypeValidator::FACET_ENUMERATION) != 0 &&
        (getEnumeration() != 0))
    {

        // If the content match (compare equal) any enumeration with
        // any of the member types, it is considerd valid.
        //
        RefVectorOf<DatatypeValidator>* memberDTV = getMemberTypeValidators();
        RefArrayVectorOf<XMLCh>* tmpEnum = getEnumeration();
        XMLSize_t memberTypeNumber = memberDTV->size();
        XMLSize_t enumLength = tmpEnum->size();

        for ( XMLSize_t memberIndex = 0; memberIndex < memberTypeNumber; ++memberIndex)
        {
            for ( XMLSize_t enumIndex = 0; enumIndex < enumLength; ++enumIndex)
            {
                try
                {
                    if (memberDTV->elementAt(memberIndex)->compare(content, tmpEnum->elementAt(enumIndex), manager) == 0)
                        return;
                }
                catch (XMLException&)
                {
                    //absorbed
                }
            } // for enumIndex
        } // for memberIndex

        ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_NotIn_Enumeration, content, manager);

    } // enumeration

}

//
//
//
int UnionDatatypeValidator::compare(const XMLCh* const lValue
                                  , const XMLCh* const rValue
                                  , MemoryManager* const manager)
{
    RefVectorOf<DatatypeValidator>* memberDTV = getMemberTypeValidators();
    XMLSize_t memberTypeNumber = memberDTV->size();

    for ( XMLSize_t memberIndex = 0; memberIndex < memberTypeNumber; ++memberIndex)
    {
        // 'compare' can throw exceptions when the datatype is not valid, or just 
        // return -1; so attempt to validate both values to get the right validator
        try
        {
            memberDTV->elementAt(memberIndex)->validate(lValue, 0, manager);                       
            memberDTV->elementAt(memberIndex)->validate(rValue, 0, manager);                       
            if (memberDTV->elementAt(memberIndex)->compare(lValue, rValue, manager) ==0)
                return  0;
        }
        catch (XMLException&)
        {
            //absorbed
        }
    }

    //REVISIT: what does it mean for UNION1 to be <less than> or <greater than> UNION2 ?
    // As long as -1 or +1 indicates an unequality, return either of them is ok.
    return -1;
}

const RefArrayVectorOf<XMLCh>* UnionDatatypeValidator::getEnumString() const
{
	return getEnumeration();
}

/***
 * 2.5.1.3 Union datatypes
 *
 * The canonical-lexical-representation for a union datatype is defined as the lexical form 
 * in which the values have the canonical lexical representation of the appropriate memberTypes.       
 ***/
const XMLCh* UnionDatatypeValidator::getCanonicalRepresentation(const XMLCh*         const rawData
                                                              ,       MemoryManager* const memMgr
                                                              ,       bool                 toValidate) const
{
    MemoryManager* toUse = memMgr? memMgr : getMemoryManager();
    UnionDatatypeValidator* temp = (UnionDatatypeValidator*) this;

    if (toValidate)
    {
        try
        {
            temp->checkContent(rawData, 0, false, toUse);
        }
        catch (...)
        {
            return 0;
        }
    }    

    //get the native unionDv
    UnionDatatypeValidator* bdv = (UnionDatatypeValidator*) temp->getBaseValidator();
    while (bdv)
    {
        temp = bdv;
        bdv = (UnionDatatypeValidator*) temp->getBaseValidator();
    }

    //let the member dv which recognize the rawData, to return
    //us the canonical form
    for ( unsigned int i = 0; i < temp->fMemberTypeValidators->size(); ++i )
    {
        try
        {
            temp->fMemberTypeValidators->elementAt(i)->validate(rawData, 0, toUse);                       
            return temp->fMemberTypeValidators->elementAt(i)->getCanonicalRepresentation(rawData, toUse, false);
        }
        catch (XMLException&)
        {
            //absorbed
        }
    }

    //if no member dv recognize it
    return 0;
}


/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(UnionDatatypeValidator)

void UnionDatatypeValidator::serialize(XSerializeEngine& serEng)
{

    DatatypeValidator::serialize(serEng);

    if (serEng.isStoring())
    {
        serEng<<fEnumerationInherited;
        serEng<<fMemberTypesInherited;

        /***
         * Serialize RefArrayVectorOf<XMLCh>
         * Serialize RefVectorOf<DatatypeValidator>
         ***/
        XTemplateSerializer::storeObject(fEnumeration, serEng);
        XTemplateSerializer::storeObject(fMemberTypeValidators, serEng);
    }
    else
    {
        serEng>>fEnumerationInherited;
        serEng>>fMemberTypesInherited;

        /***
         * Deserialize RefArrayVectorOf<XMLCh>
         * Deserialize RefVectorOf<DatatypeValidator>
         ***/
        XTemplateSerializer::loadObject(&fEnumeration, 8, true, serEng);
        XTemplateSerializer::loadObject(&fMemberTypeValidators, 4, false, serEng);
    }
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file UnionDatatypeValidator.cpp
  */
