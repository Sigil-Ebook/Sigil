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
 * $Id: DTDGrammar.cpp 676911 2008-07-15 13:27:32Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/validators/DTD/DTDGrammar.hpp>
#include <xercesc/validators/DTD/XMLDTDDescriptionImpl.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  DTDGrammar: Static member data
// ---------------------------------------------------------------------------
NameIdPool<DTDEntityDecl>* DTDGrammar::fDefaultEntities = 0;

void XMLInitializer::initializeDTDGrammar()
{
    DTDGrammar::fDefaultEntities = new NameIdPool<DTDEntityDecl>(11, 12);

    // Add the default entity entries for the character refs that must
    // always be present. We indicate that they are from the internal
    // subset. They aren't really, but they have to look that way so
    // that they are still valid for use within a standalone document.
    //
    // We also mark them as special char entities, which allows them
    // to be used in places whether other non-numeric general entities
    // cannot.
    //
    if (DTDGrammar::fDefaultEntities)
    {
        DTDGrammar::fDefaultEntities->put(new DTDEntityDecl(XMLUni::fgAmp, chAmpersand, true, true));
        DTDGrammar::fDefaultEntities->put(new DTDEntityDecl(XMLUni::fgLT, chOpenAngle, true, true));
        DTDGrammar::fDefaultEntities->put(new DTDEntityDecl(XMLUni::fgGT, chCloseAngle, true, true));
        DTDGrammar::fDefaultEntities->put(new DTDEntityDecl(XMLUni::fgQuot, chDoubleQuote, true, true));
        DTDGrammar::fDefaultEntities->put(new DTDEntityDecl(XMLUni::fgApos, chSingleQuote, true, true));
    }
}

void XMLInitializer::terminateDTDGrammar()
{
  delete DTDGrammar::fDefaultEntities;
  DTDGrammar::fDefaultEntities = 0;
}

//---------------------------------------------------------------------------
//  DTDGrammar: Constructors and Destructor
// ---------------------------------------------------------------------------
DTDGrammar::DTDGrammar(MemoryManager* const manager) :
    fMemoryManager(manager)
    , fElemDeclPool(0)
    , fElemNonDeclPool(0)
    , fEntityDeclPool(0)
    , fNotationDeclPool(0)
    , fGramDesc(0)
    , fValidated(false)
{
    //
    //  Init all the pool members.
    //
    //  <TBD> Investigate what the optimum values would be for the various
    //  pools.
    //
    fElemDeclPool = new (fMemoryManager) NameIdPool<DTDElementDecl>(109, 128, fMemoryManager);
    // should not need this in the common situation where grammars
    // are built once and then read - NG
    //fElemNonDeclPool = new (fMemoryManager) NameIdPool<DTDElementDecl>(29, 128, fMemoryManager);
    fEntityDeclPool = new (fMemoryManager) NameIdPool<DTDEntityDecl>(109, 128, fMemoryManager);
    fNotationDeclPool = new (fMemoryManager) NameIdPool<XMLNotationDecl>(109, 128, fMemoryManager);

    //REVISIT: use grammarPool to create
    fGramDesc = new (fMemoryManager) XMLDTDDescriptionImpl(XMLUni::fgDTDEntityString, fMemoryManager);
}

DTDGrammar::~DTDGrammar()
{
    delete fElemDeclPool;
    if(fElemNonDeclPool)
    {
        delete fElemNonDeclPool;
    }
    delete fEntityDeclPool;
    delete fNotationDeclPool;
    delete fGramDesc;
}

// -----------------------------------------------------------------------
//  Virtual methods
// -----------------------------------------------------------------------
XMLElementDecl* DTDGrammar::findOrAddElemDecl (const   unsigned int    uriId
        , const XMLCh* const    baseName
        , const XMLCh* const
        , const XMLCh* const    qName
        , unsigned int          scope
        ,       bool&           wasAdded )
{
    // See it it exists
    DTDElementDecl* retVal = (DTDElementDecl*) getElemDecl(uriId, baseName, qName, scope);

    // if not, then add this in
    if (!retVal)
    {
        retVal = new (fMemoryManager) DTDElementDecl
        (
            qName
            , uriId
            , DTDElementDecl::Any
            , fMemoryManager
        );
        if(!fElemNonDeclPool)
            fElemNonDeclPool = new (fMemoryManager) NameIdPool<DTDElementDecl>(29, 128, fMemoryManager);
        const XMLSize_t elemId = fElemNonDeclPool->put(retVal);
        retVal->setId(elemId);
        wasAdded = true;
    }
     else
    {
        wasAdded = false;
    }
    return retVal;
}

XMLElementDecl* DTDGrammar::putElemDecl (const   unsigned int    uriId
        , const XMLCh* const
        , const XMLCh* const
        , const XMLCh* const    qName
        , unsigned int
        , const bool            notDeclared)
{
    DTDElementDecl* retVal = new (fMemoryManager) DTDElementDecl
    (
        qName
        , uriId
        , DTDElementDecl::Any
        , fMemoryManager
    );
    if(notDeclared)
    {
        if(!fElemNonDeclPool)
            fElemNonDeclPool = new (fMemoryManager) NameIdPool<DTDElementDecl>(29, 128, fMemoryManager);
        retVal->setId(fElemNonDeclPool->put(retVal));
    } else
    {
        retVal->setId(fElemDeclPool->put(retVal));
    }
    return retVal;
}

void DTDGrammar::reset()
{
    //
    //  We need to reset all of the pools.
    //
    fElemDeclPool->removeAll();
    // now that we have this, no point in deleting it...
    if(fElemNonDeclPool)
        fElemNonDeclPool->removeAll();
    fNotationDeclPool->removeAll();
    fEntityDeclPool->removeAll();
    fValidated = false;
}

void DTDGrammar::setGrammarDescription( XMLGrammarDescription* gramDesc)
{
    if ((!gramDesc) ||
        (gramDesc->getGrammarType() != Grammar::DTDGrammarType))
        return;

    if (fGramDesc)
        delete fGramDesc;

    //adopt the grammar Description
    fGramDesc = (XMLDTDDescription*) gramDesc;
}

XMLGrammarDescription*  DTDGrammar::getGrammarDescription() const
{
    return fGramDesc;
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(DTDGrammar)

void DTDGrammar::serialize(XSerializeEngine& serEng)
{

    Grammar::serialize(serEng);

    //don't serialize fDefaultEntities

    if (serEng.isStoring())
    {
        /***
         *
         * Serialize NameIdPool<DTDElementDecl>*       fElemDeclPool;
         * Serialize NameIdPool<DTDEntityDecl>*        fEntityDeclPool;
         * Serialize NameIdPool<XMLNotationDecl>*      fNotationDeclPool;
         ***/
        XTemplateSerializer::storeObject(fElemDeclPool, serEng);
        XTemplateSerializer::storeObject(fEntityDeclPool, serEng);
        XTemplateSerializer::storeObject(fNotationDeclPool, serEng);

        /***
         * serialize() method shall be used to store object
         * which has been created in ctor
         ***/
        fGramDesc->serialize(serEng);

        serEng<<fValidated;
    }
    else
    {

       /***
         *
         * Deserialize NameIdPool<DTDElementDecl>*       fElemDeclPool;
         * Deserialize NameIdPool<DTDEntityDecl>*        fEntityDeclPool;
         * Deerialize NameIdPool<XMLNotationDecl>*       fNotationDeclPool;
         ***/
        XTemplateSerializer::loadObject(&fElemDeclPool, 109, 128, serEng);
        fElemNonDeclPool = 0;
        XTemplateSerializer::loadObject(&fEntityDeclPool, 109, 128, serEng);
        XTemplateSerializer::loadObject(&fNotationDeclPool, 109, 128, serEng);

        /***
         * serialize() method shall be used to load object
         * which has been created in ctor
         ***/
        fGramDesc->serialize(serEng);

        serEng>>fValidated;
    }

}

XERCES_CPP_NAMESPACE_END
