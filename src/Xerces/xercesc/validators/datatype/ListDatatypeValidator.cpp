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
 * $Id: ListDatatypeValidator.cpp 695949 2008-09-16 15:57:44Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/datatype/ListDatatypeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

static const int BUF_LEN = 64;

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
ListDatatypeValidator::ListDatatypeValidator(MemoryManager* const manager)
:AbstractStringValidator(0, 0, 0, DatatypeValidator::List, manager)
,fContent(0)
{}

ListDatatypeValidator::ListDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*           const enums
                        , const int                           finalSet
                        , MemoryManager* const manager)
:AbstractStringValidator(baseValidator, facets, finalSet, DatatypeValidator::List, manager)
,fContent(0)
{
    //
    // baseValidator shall either
    // an atomic DTV which servers as itemType, or
    // another ListDTV from which, this ListDTV is derived by restriction.
    //
    // In either case, it shall be not null
    //
    if (!baseValidator)
        ThrowXMLwithMemMgr(InvalidDatatypeFacetException, XMLExcepts::FACET_List_Null_baseValidator, manager);

    init(enums, manager);
}

ListDatatypeValidator::~ListDatatypeValidator()
{}

DatatypeValidator* ListDatatypeValidator::newInstance
(
      RefHashTableOf<KVStringPair>* const facets
    , RefArrayVectorOf<XMLCh>* const      enums
    , const int                           finalSet
    , MemoryManager* const                manager
)
{
    return (DatatypeValidator*) new (manager) ListDatatypeValidator(this, facets, enums, finalSet, manager);
}


int ListDatatypeValidator::compare(const XMLCh*     const lValue
                                 , const XMLCh*     const rValue
                                 , MemoryManager*   const manager)
{
    DatatypeValidator* theItemTypeDTV = getItemTypeDTV();
    BaseRefVectorOf<XMLCh>* lVector = XMLString::tokenizeString(lValue, manager);
    Janitor<BaseRefVectorOf<XMLCh> > janl(lVector);
    BaseRefVectorOf<XMLCh>* rVector = XMLString::tokenizeString(rValue, manager);
    Janitor<BaseRefVectorOf<XMLCh> > janr(rVector);

    XMLSize_t lNumberOfTokens = lVector->size();
    XMLSize_t rNumberOfTokens = rVector->size();

    if (lNumberOfTokens < rNumberOfTokens)
        return -1;
    else if (lNumberOfTokens > rNumberOfTokens)
        return 1;
    else
    { //compare each token
        for ( XMLSize_t i = 0; i < lNumberOfTokens; i++)
        {
            int returnValue = theItemTypeDTV->compare(lVector->elementAt(i), rVector->elementAt(i), manager);
            if (returnValue != 0)
                return returnValue; //REVISIT: does it make sense to return -1 or +1..?
        }
        return 0;
    }

}

void ListDatatypeValidator::validate( const XMLCh*             const content
                                    ,       ValidationContext* const context
                                    ,       MemoryManager*     const manager)
{
    setContent(content);
    BaseRefVectorOf<XMLCh>* tokenVector = XMLString::tokenizeString(content, manager);
    Janitor<BaseRefVectorOf<XMLCh> > janName(tokenVector);
    checkContent(tokenVector, content, context, false, manager);
}

void ListDatatypeValidator::checkContent( const XMLCh*             const content
                                         ,      ValidationContext* const context
                                         ,      bool                     asBase
                                         ,      MemoryManager*     const manager)
{
    setContent(content);
    BaseRefVectorOf<XMLCh>* tokenVector = XMLString::tokenizeString(content, manager);
    Janitor<BaseRefVectorOf<XMLCh> > janName(tokenVector);
    checkContent(tokenVector, content, context, asBase, manager);
}

//
// here content is a list of items
//
void ListDatatypeValidator::checkContent(       BaseRefVectorOf<XMLCh>*       tokenVector
                                        , const XMLCh*                  const content
                                        ,       ValidationContext*      const context
                                        ,       bool                          asBase
                                        ,       MemoryManager*          const manager)
{
    DatatypeValidator* bv = getBaseValidator();

    if (bv->getType() == DatatypeValidator::List)
        ((ListDatatypeValidator*)bv)->checkContent(tokenVector, content, context, true, manager);
    else
    {   // the ultimate itemType DTV
        for (unsigned int i = 0; i < tokenVector->size(); i++)
            bv->validate(tokenVector->elementAt(i), context, manager);
    }

    int thisFacetsDefined = getFacetsDefined();

    // we check pattern first
    if ( (thisFacetsDefined & DatatypeValidator::FACET_PATTERN ) != 0 )
    {
        //check every item in the list as a whole
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

    XMLSize_t tokenNumber = tokenVector->size();

    if (((thisFacetsDefined & DatatypeValidator::FACET_MAXLENGTH) != 0) &&
        (tokenNumber > getMaxLength()))
    {
        XMLCh value1[BUF_LEN+1];
        XMLCh value2[BUF_LEN+1];
        XMLString::sizeToText(tokenNumber, value1, BUF_LEN, 10, manager);
        XMLString::sizeToText(getMaxLength(), value2, BUF_LEN, 10, manager);

        ThrowXMLwithMemMgr3(InvalidDatatypeValueException
                , XMLExcepts::VALUE_GT_maxLen
                , getContent()
                , value1
                , value2
                , manager);
    }

    if (((thisFacetsDefined & DatatypeValidator::FACET_MINLENGTH) != 0) &&
        (tokenNumber < getMinLength()))
    {
        XMLCh value1[BUF_LEN+1];
        XMLCh value2[BUF_LEN+1];
        XMLString::sizeToText(tokenNumber, value1, BUF_LEN, 10, manager);
        XMLString::sizeToText(getMinLength(), value2, BUF_LEN, 10, manager);

        ThrowXMLwithMemMgr3(InvalidDatatypeValueException
                , XMLExcepts::VALUE_LT_minLen
                , getContent()
                , value1
                , value2
                , manager);
    }

    if (((thisFacetsDefined & DatatypeValidator::FACET_LENGTH) != 0) &&
        (tokenNumber != AbstractStringValidator::getLength()))
    {
        XMLCh value1[BUF_LEN+1];
        XMLCh value2[BUF_LEN+1];
        XMLString::sizeToText(tokenNumber, value1, BUF_LEN, 10, manager);
        XMLString::sizeToText(AbstractStringValidator::getLength(), value2, BUF_LEN, 10, manager);

        ThrowXMLwithMemMgr3(InvalidDatatypeValueException
                , XMLExcepts::VALUE_NE_Len
                , getContent()
                , value1
                , value2
                , manager);
    }

    if ((thisFacetsDefined & DatatypeValidator::FACET_ENUMERATION) != 0 &&
        (getEnumeration() != 0))
    {
        XMLSize_t i;
        XMLSize_t enumLength = getEnumeration()->size();

        for ( i = 0; i < enumLength; i++)
        {
            //optimization: we do a lexical comparision first
            // this may be faster for string and its derived
            if (XMLString::equals(getEnumeration()->elementAt(i), getContent()))
                break; // a match found

            // do a value space check
            // this is needed for decimal (and probably other types
            // such as datetime related)
            // eg.
            // tokenVector = "1 2 3.0 4" vs enumeration = "1 2 3 4.0"
            //
            if (valueSpaceCheck(tokenVector, getEnumeration()->elementAt(i), manager))
                break;
        }

        if (i == enumLength)
            ThrowXMLwithMemMgr1(InvalidDatatypeValueException, XMLExcepts::VALUE_NotIn_Enumeration, getContent(), manager);

    } // enumeration

}

bool ListDatatypeValidator::valueSpaceCheck(BaseRefVectorOf<XMLCh>* tokenVector
                                          , const XMLCh*    const  enumStr
                                          , MemoryManager*  const  manager) const
{
    DatatypeValidator* theItemTypeDTV = getItemTypeDTV();
    BaseRefVectorOf<XMLCh>* enumVector = XMLString::tokenizeString(enumStr, manager);
    Janitor<BaseRefVectorOf<XMLCh> > janName(enumVector);

    if (tokenVector->size() != enumVector->size())
        return false;

    for ( unsigned int j = 0; j < tokenVector->size(); j++ )
    {
        if (theItemTypeDTV->compare(tokenVector->elementAt(j), enumVector->elementAt(j), manager) != 0)
            return false;
    }

    return true;
}

DatatypeValidator* ListDatatypeValidator::getItemTypeDTV() const
{
    DatatypeValidator* bdv = this->getBaseValidator();

    while (bdv->getType() == DatatypeValidator::List)
        bdv = bdv->getBaseValidator();

    return bdv;
}

// ---------------------------------------------------------------------------
//  Utilities
// ---------------------------------------------------------------------------

void ListDatatypeValidator::checkValueSpace(const XMLCh* const
                                            , MemoryManager* const)
{}

XMLSize_t ListDatatypeValidator::getLength(const XMLCh* const content
                                     , MemoryManager* const manager) const
{
    BaseRefVectorOf<XMLCh>* tokenVector = XMLString::tokenizeString(content, manager);
    Janitor<BaseRefVectorOf<XMLCh> > janName(tokenVector);

    return tokenVector->size();
}

void ListDatatypeValidator::inspectFacetBase(MemoryManager* const manager)
{

    //
    // we are pretty sure baseValidator is not null
    //

    if (getBaseValidator()->getType() == DatatypeValidator::List)
    {
        AbstractStringValidator::inspectFacetBase(manager);
    }
    else
    {
        // the first level ListDTV
        // check 4.3.5.c0 must: enumeration values from the value space of base
        if ( ((getFacetsDefined() & DatatypeValidator::FACET_ENUMERATION) != 0) &&
             (getEnumeration() !=0)                                              )
        {
            XMLSize_t i;
            XMLSize_t enumLength = getEnumeration()->size();
            try
            {
                for ( i = 0; i < enumLength; i++)
                {
                    // ask the itemType for a complete check
                    BaseRefVectorOf<XMLCh>* tempList = XMLString::tokenizeString(getEnumeration()->elementAt(i), manager);
                    Janitor<BaseRefVectorOf<XMLCh> >    jan(tempList);
                    XMLSize_t tokenNumber = tempList->size();

                    try
                    {
                        for ( XMLSize_t j = 0; j < tokenNumber; j++)
                            getBaseValidator()->validate(tempList->elementAt(j), (ValidationContext*)0, manager);
                    }
                    catch(const OutOfMemoryException&)
                    {
                        jan.release();

                        throw;
                    }
#if 0
// spec says that only base has to checkContent                    
                    // enum shall pass this->checkContent() as well.
                    checkContent(getEnumeration()->elementAt(i), (ValidationContext*)0, false, manager);
#endif
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

    }

}// End of inspectFacetBase()

void ListDatatypeValidator::inheritFacet()
{

    //iff the base validator is List, then we inherit
    //
    if (getBaseValidator()->getType() == DatatypeValidator::List)
    {
        AbstractStringValidator::inheritFacet();
    }

}

/***
 * 2.5.1.2 List datatypes   
 *   
 * The canonical-lexical-representation for the list datatype is defined as 
 * the lexical form in which each item in the list has the canonical 
 * lexical representation of its itemType.
 ***/
const XMLCh* ListDatatypeValidator::getCanonicalRepresentation(const XMLCh*         const rawData
                                                             ,       MemoryManager* const memMgr
                                                             ,       bool                 toValidate) const
{
    MemoryManager* toUse = memMgr? memMgr : getMemoryManager();
    ListDatatypeValidator* temp = (ListDatatypeValidator*) this;
    temp->setContent(rawData);
    BaseRefVectorOf<XMLCh>* tokenVector = XMLString::tokenizeString(rawData, toUse);
    Janitor<BaseRefVectorOf<XMLCh> > janName(tokenVector);    

    if (toValidate)
    {
        try
        {
            temp->checkContent(tokenVector, rawData, 0, false, toUse);
        }
        catch (...)
        {
            return 0;
        }
    }
   
    XMLSize_t retBufSize = 2 * XMLString::stringLen(rawData);
    XMLCh* retBuf = (XMLCh*) toUse->allocate(retBufSize * sizeof(XMLCh));
    retBuf[0] = 0;
    XMLCh* retBufPtr = retBuf;
    DatatypeValidator* itemDv = this->getItemTypeDTV();

    try 
    {
        for (unsigned int i = 0; i < tokenVector->size(); i++)
        {
            XMLCh* itemCanRep = (XMLCh*) itemDv->getCanonicalRepresentation(tokenVector->elementAt(i), toUse, false);
            XMLSize_t itemLen = XMLString::stringLen(itemCanRep); 

            if(retBufPtr+itemLen+2 >= retBuf+retBufSize)
            {
                // need to resize
                XMLCh * oldBuf = retBuf;
                retBuf = (XMLCh*) toUse->allocate(retBufSize * sizeof(XMLCh) * 4);
                memcpy(retBuf, oldBuf, retBufSize * sizeof(XMLCh ));
                retBufPtr = (retBufPtr - oldBuf) + retBuf;
                toUse->deallocate(oldBuf);
                retBufSize <<= 2;
            }

            XMLString::catString(retBufPtr, itemCanRep);
            retBufPtr = retBufPtr + itemLen;
            *(retBufPtr++) = chSpace;
            *(retBufPtr) = chNull;
            toUse->deallocate(itemCanRep);
        }

        return retBuf;

    }
    catch (...)
    {
        return 0;
    }
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(ListDatatypeValidator)

void ListDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    AbstractStringValidator::serialize(serEng);

    //don't serialize fContent, since it is NOT owned and 
    //will be reset each time validate()/checkContent() invoked.
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file ListDatatypeValidator.cpp
  */
