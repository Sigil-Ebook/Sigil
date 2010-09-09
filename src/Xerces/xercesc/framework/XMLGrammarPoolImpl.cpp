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
 * $Id: XMLGrammarPoolImpl.cpp 676823 2008-07-15 07:57:44Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/XMLGrammarPoolImpl.hpp>
#include <xercesc/internal/XSerializeEngine.hpp>
#include <xercesc/internal/XTemplateSerializer.hpp>
#include <xercesc/validators/DTD/DTDGrammar.hpp>
#include <xercesc/validators/DTD/XMLDTDDescriptionImpl.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/validators/schema/XMLSchemaDescriptionImpl.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/SynchronizedStringPool.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// private function used to update fXSModel
void XMLGrammarPoolImpl::createXSModel()
{
    delete fXSModel;
    fXSModel = 0;
    fXSModel = new (getMemoryManager()) XSModel(this, getMemoryManager());
    fXSModelIsValid = true;
}

// ---------------------------------------------------------------------------
//  XMLGrammarPoolImpl: constructor and destructor
// ---------------------------------------------------------------------------
XMLGrammarPoolImpl::~XMLGrammarPoolImpl()
{
    delete fGrammarRegistry;
    delete fStringPool;
    if(fSynchronizedStringPool)
        delete fSynchronizedStringPool;
    if(fXSModel)
        delete fXSModel;
}

XMLGrammarPoolImpl::XMLGrammarPoolImpl(MemoryManager* const memMgr)
:XMLGrammarPool(memMgr)
,fGrammarRegistry(0)
,fStringPool(0)
,fSynchronizedStringPool(0)
,fXSModel(0)
,fLocked(false)
,fXSModelIsValid(false)
{
    fGrammarRegistry = new (memMgr) RefHashTableOf<Grammar>(29, true, memMgr);
    fStringPool = new (memMgr) XMLStringPool(109, memMgr);
}

// -----------------------------------------------------------------------
// Implementation of Grammar Pool Interface
// -----------------------------------------------------------------------
bool XMLGrammarPoolImpl::cacheGrammar(Grammar* const               gramToCache )
{
    if(fLocked || !gramToCache)
        return false;

    const XMLCh* grammarKey = gramToCache->getGrammarDescription()->getGrammarKey();

    if (fGrammarRegistry->containsKey(grammarKey))
    {
        return false;
    }

    fGrammarRegistry->put((void*) grammarKey, gramToCache);

    if (fXSModelIsValid && gramToCache->getGrammarType() == Grammar::SchemaGrammarType)
    {
        fXSModelIsValid = false;
    }
    return true;
}

Grammar* XMLGrammarPoolImpl::retrieveGrammar(XMLGrammarDescription* const gramDesc)
{
    if (!gramDesc)
        return 0;

    /***
     * This implementation simply use GrammarKey
     */
    return fGrammarRegistry->get(gramDesc->getGrammarKey());
}

Grammar* XMLGrammarPoolImpl::orphanGrammar(const XMLCh* const nameSpaceKey)
{
    if (!fLocked)
    {
        Grammar* grammar = fGrammarRegistry->orphanKey(nameSpaceKey);
        if (fXSModelIsValid && grammar && grammar->getGrammarType() == Grammar::SchemaGrammarType)
        {
            fXSModelIsValid = false;
        }
        return grammar;
    }
    return 0;
}

RefHashTableOfEnumerator<Grammar>
XMLGrammarPoolImpl::getGrammarEnumerator() const
{
    return RefHashTableOfEnumerator<Grammar>(fGrammarRegistry, false, fGrammarRegistry->getMemoryManager());
}


bool XMLGrammarPoolImpl::clear()
{
    if (!fLocked)
    {
        fGrammarRegistry->removeAll();

        fXSModelIsValid = false;
        if (fXSModel)
        {
            delete fXSModel;
            fXSModel = 0;
        }
        return true;
    }
    return false;
}

void XMLGrammarPoolImpl::lockPool()
{
    if (!fLocked)
    {
        fLocked = true;
        MemoryManager *memMgr = getMemoryManager();
        if(!fSynchronizedStringPool)
        {
            fSynchronizedStringPool = new (memMgr) XMLSynchronizedStringPool(fStringPool, 109, memMgr);
        }
        if (!fXSModelIsValid)
        {
            createXSModel();
        }
    }
}

void XMLGrammarPoolImpl::unlockPool()
{
    if (fLocked)
    {
        fLocked = false;
        if(fSynchronizedStringPool)
        {
            fSynchronizedStringPool->flushAll();
            // if user calls Lock again, need to have null fSynchronizedStringPool
            delete fSynchronizedStringPool;
            fSynchronizedStringPool = 0;
        }
        fXSModelIsValid = false;
        if (fXSModel)
        {
            delete fXSModel;
            fXSModel = 0;
        }
    }
}

// -----------------------------------------------------------------------
// Implementation of Factory Interface
// -----------------------------------------------------------------------
DTDGrammar*  XMLGrammarPoolImpl::createDTDGrammar()
{
	return new (getMemoryManager()) DTDGrammar(getMemoryManager());
}

SchemaGrammar* XMLGrammarPoolImpl::createSchemaGrammar()
{
	return new (getMemoryManager()) SchemaGrammar(getMemoryManager());
}

XMLDTDDescription*  XMLGrammarPoolImpl::createDTDDescription(const XMLCh* const systemId)
{
	return new (getMemoryManager()) XMLDTDDescriptionImpl(systemId, getMemoryManager());
}

XMLSchemaDescription* XMLGrammarPoolImpl::createSchemaDescription(const XMLCh* const targetNamespace)
{
	return new (getMemoryManager()) XMLSchemaDescriptionImpl(targetNamespace, getMemoryManager());
}

XSModel *XMLGrammarPoolImpl::getXSModel(bool& XSModelWasChanged)
{
    XSModelWasChanged = false;
    if (fLocked || fXSModelIsValid)
        return fXSModel;

    createXSModel();
    XSModelWasChanged = true;
    return fXSModel;
}

XMLStringPool *XMLGrammarPoolImpl::getURIStringPool()
{
    if(fLocked)
        return fSynchronizedStringPool;
    return fStringPool;
}

// -----------------------------------------------------------------------
// serialization and deserialization support
// -----------------------------------------------------------------------
/***
 *
 * don't serialize
 *
 *   XMLSynchronizedStringPool*  fSynchronizedStringPool;
 */

/***
 *   .non-empty gramamrRegistry
 ***/
void XMLGrammarPoolImpl::serializeGrammars(BinOutputStream* const binOut)
{
    RefHashTableOfEnumerator<Grammar> grammarEnum(fGrammarRegistry, false, getMemoryManager());
    if (!(grammarEnum.hasMoreElements()))
    {
        ThrowXMLwithMemMgr(XSerializationException, XMLExcepts::XSer_GrammarPool_Empty, getMemoryManager());
    }

    XSerializeEngine  serEng(binOut, this);

    //version information
    serEng<<(unsigned int)XERCES_GRAMMAR_SERIALIZATION_LEVEL;

    //lock status
    serEng<<fLocked;

    //StringPool, don't use <<
    fStringPool->serialize(serEng);

    /***
     * Serialize RefHashTableOf<Grammar>*    fGrammarRegistry;
     ***/
    XTemplateSerializer::storeObject(fGrammarRegistry, serEng);
}

/***
 *   .empty stringPool
 *   .empty gramamrRegistry
 ***/
void XMLGrammarPoolImpl::deserializeGrammars(BinInputStream* const binIn)
{
    MemoryManager *memMgr = getMemoryManager();
    unsigned int stringCount = fStringPool->getStringCount();
    if (stringCount)
    {
        /***
         * it contains only the four predefined one, that is ok
         * but we need to reset the string before deserialize it
         *
         ***/
        if ( stringCount <= 4 )
        {
            fStringPool->flushAll();
        }
        else
        {
            ThrowXMLwithMemMgr(XSerializationException, XMLExcepts::XSer_StringPool_NotEmpty, memMgr);
        }
    }

    RefHashTableOfEnumerator<Grammar> grammarEnum(fGrammarRegistry, false, memMgr);
    if (grammarEnum.hasMoreElements())
    {
        ThrowXMLwithMemMgr(XSerializationException, XMLExcepts::XSer_GrammarPool_NotEmpty, memMgr);
    }

    // This object will take care of cleaning up if an exception is
    // thrown during deserialization.
    JanitorMemFunCall<XMLGrammarPoolImpl>   cleanup(this, &XMLGrammarPoolImpl::cleanUp);

    try
    {
        XSerializeEngine  serEng(binIn, this);

        //version information
        unsigned int  StorerLevel;
        serEng>>StorerLevel;
        serEng.fStorerLevel = StorerLevel;

        // The storer level must match the loader level.
        //
        if (StorerLevel != (unsigned int)XERCES_GRAMMAR_SERIALIZATION_LEVEL)
        {
            XMLCh     StorerLevelChar[5];
            XMLCh     LoaderLevelChar[5];
            XMLString::binToText(StorerLevel,                          StorerLevelChar,   4, 10, memMgr);
            XMLString::binToText(XERCES_GRAMMAR_SERIALIZATION_LEVEL,   LoaderLevelChar,   4, 10, memMgr);

            ThrowXMLwithMemMgr2(XSerializationException
                    , XMLExcepts::XSer_Storer_Loader_Mismatch
                    , StorerLevelChar
                    , LoaderLevelChar
                    , memMgr);
        }

        //lock status
        serEng>>fLocked;

        //StringPool, don't use >>
        fStringPool->serialize(serEng);

        /***
         * Deserialize RefHashTableOf<Grammar>*    fGrammarRegistry;
         ***/
        XTemplateSerializer::loadObject(&fGrammarRegistry, 29, true, serEng);

    }
    catch(const OutOfMemoryException&)
    {
        // This is a special case, because we don't want
        // to execute cleanup code on out-of-memory
        // conditions.
        cleanup.release();

        throw;
    }

    // Everything is OK, so we can release the cleanup object.
    cleanup.release();

    if (fLocked)
    {
        createXSModel();
    }
}


void
XMLGrammarPoolImpl::cleanUp()
{
    fLocked = false;

    clear();
}


XERCES_CPP_NAMESPACE_END
