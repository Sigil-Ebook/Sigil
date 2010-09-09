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
 * $Id $
 */

#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>
#include <xercesc/validators/schema/SchemaGrammar.hpp>
#include <xercesc/validators/schema/XMLSchemaDescriptionImpl.hpp>
#include <xercesc/validators/DTD/XMLDTDDescriptionImpl.hpp>
#include <xercesc/framework/XMLGrammarPoolImpl.hpp>
#include <xercesc/framework/psvi/XSAnnotation.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  GrammarResolver: Constructor and Destructor
// ---------------------------------------------------------------------------
GrammarResolver::GrammarResolver(XMLGrammarPool* const gramPool
                               , MemoryManager*  const manager)
:fCacheGrammar(false)
,fUseCachedGrammar(false)
,fGrammarPoolFromExternalApplication(true)
,fStringPool(0)
,fGrammarBucket(0)
,fGrammarFromPool(0)
,fDataTypeReg(0)
,fMemoryManager(manager)
,fGrammarPool(gramPool)
,fXSModel(0)
,fGrammarPoolXSModel(0)
,fGrammarsToAddToXSModel(0)
{
    fGrammarBucket = new (manager) RefHashTableOf<Grammar>(29, true,  manager);

    /***
     * Grammars in this set are not owned
     */    
    fGrammarFromPool = new (manager) RefHashTableOf<Grammar>(29, false,  manager);

    if (!gramPool)
    {
        /***
         * We need to instantiate a default grammar pool object so that
         * all grammars and grammar components could be created through
         * the Factory methods
         */
        fGrammarPool = new (manager) XMLGrammarPoolImpl(manager);     
        fGrammarPoolFromExternalApplication=false;
    }
    fStringPool = fGrammarPool->getURIStringPool();

    // REVISIT: size
    fGrammarsToAddToXSModel = new (manager) ValueVectorOf<SchemaGrammar*> (29, manager);
}

GrammarResolver::~GrammarResolver()
{  
    delete fGrammarBucket;
    delete fGrammarFromPool;

    if (fDataTypeReg)
      delete fDataTypeReg;

   /***
    *  delete the grammar pool iff it is created by this resolver
    */
   if (!fGrammarPoolFromExternalApplication)
       delete fGrammarPool;

   if (fXSModel)
       delete fXSModel;
   // don't delete fGrammarPoolXSModel! we don't own it!
   delete fGrammarsToAddToXSModel;
}

// ---------------------------------------------------------------------------
//  GrammarResolver: Getter methods
// ---------------------------------------------------------------------------
DatatypeValidator*
GrammarResolver::getDatatypeValidator(const XMLCh* const uriStr,
                                      const XMLCh* const localPartStr) {

    DatatypeValidator* dv = 0;

    if (XMLString::equals(uriStr, SchemaSymbols::fgURI_SCHEMAFORSCHEMA)) {

        if (!fDataTypeReg) {

            fDataTypeReg = new (fMemoryManager) DatatypeValidatorFactory(fMemoryManager);
        }

        dv = fDataTypeReg->getDatatypeValidator(localPartStr);
    }
    else {

        Grammar* grammar = getGrammar(uriStr);

        if (grammar && grammar->getGrammarType() == Grammar::SchemaGrammarType) {

            XMLBuffer nameBuf(128, fMemoryManager);

            nameBuf.set(uriStr);
            nameBuf.append(chComma);
            nameBuf.append(localPartStr);

            dv = ((SchemaGrammar*) grammar)->getDatatypeRegistry()->getDatatypeValidator(nameBuf.getRawBuffer());
        }
    }

    return dv;
}

Grammar* GrammarResolver::getGrammar( const XMLCh* const namespaceKey)
{
    if (!namespaceKey)
        return 0;

    Grammar* grammar = fGrammarBucket->get(namespaceKey);

    if (grammar)
        return grammar;

    if (fUseCachedGrammar)
    {
        grammar = fGrammarFromPool->get(namespaceKey);
        if (grammar)
        {
            return grammar;
        }
        else
        {
            XMLSchemaDescription* gramDesc = fGrammarPool->createSchemaDescription(namespaceKey);
            Janitor<XMLGrammarDescription> janName(gramDesc);
            grammar = fGrammarPool->retrieveGrammar(gramDesc);
            if (grammar)
            {
                fGrammarFromPool->put((void*) grammar->getGrammarDescription()->getGrammarKey(), grammar);
            }
            return grammar;
        }
    }

    return 0;
}

Grammar* GrammarResolver::getGrammar( XMLGrammarDescription* const gramDesc)
{
    if (!gramDesc)
        return 0;

    Grammar* grammar = fGrammarBucket->get(gramDesc->getGrammarKey());

    if (grammar)
        return grammar;

    if (fUseCachedGrammar)
    {
        grammar = fGrammarFromPool->get(gramDesc->getGrammarKey());
        if (grammar)
        {
            return grammar;
        }
        else
        {
            grammar = fGrammarPool->retrieveGrammar(gramDesc);
            if (grammar)
            {
                fGrammarFromPool->put((void*) grammar->getGrammarDescription()->getGrammarKey(), grammar);
            }
            return grammar;
        }
    }

    return 0;
}

RefHashTableOfEnumerator<Grammar>
GrammarResolver::getGrammarEnumerator() const
{
    return RefHashTableOfEnumerator<Grammar>(fGrammarBucket, false, fMemoryManager);
}

RefHashTableOfEnumerator<Grammar>
GrammarResolver::getReferencedGrammarEnumerator() const
{
    return RefHashTableOfEnumerator<Grammar>(fGrammarFromPool, false, fMemoryManager);
}

RefHashTableOfEnumerator<Grammar>
GrammarResolver::getCachedGrammarEnumerator() const
{
    return fGrammarPool->getGrammarEnumerator();
}

bool GrammarResolver::containsNameSpace( const XMLCh* const nameSpaceKey )
{
    if (!nameSpaceKey)
        return false;
    if (fGrammarBucket->containsKey(nameSpaceKey))
        return true;
    if (fUseCachedGrammar)
    {
        if (fGrammarFromPool->containsKey(nameSpaceKey))
            return true;

        // Lastly, need to check in fGrammarPool        
        XMLSchemaDescription* gramDesc = fGrammarPool->createSchemaDescription(nameSpaceKey);
        Janitor<XMLGrammarDescription> janName(gramDesc);
        Grammar* grammar = fGrammarPool->retrieveGrammar(gramDesc);
        if (grammar)
            return true;
    }

    return false;
}

void GrammarResolver::putGrammar(Grammar* const grammarToAdopt)
{
    if (!grammarToAdopt)
        return;

    /***
     * the grammar will be either in the grammarpool, or in the grammarbucket
     */
    if (!fCacheGrammar || !fGrammarPool->cacheGrammar(grammarToAdopt))
    {
        // either we aren't caching or the grammar pool doesn't want it
        // so we need to look after it
        fGrammarBucket->put( (void*) grammarToAdopt->getGrammarDescription()->getGrammarKey(), grammarToAdopt );
        if (grammarToAdopt->getGrammarType() == Grammar::SchemaGrammarType)
        {
            fGrammarsToAddToXSModel->addElement((SchemaGrammar*) grammarToAdopt);
        }
    }
}

// ---------------------------------------------------------------------------
//  GrammarResolver: methods
// ---------------------------------------------------------------------------
void GrammarResolver::reset() {
    fGrammarBucket->removeAll();
    fGrammarsToAddToXSModel->removeAllElements();
    delete fXSModel;
    fXSModel = 0;
}

void GrammarResolver::resetCachedGrammar()
{
    //REVISIT: if the pool is locked this will fail... should throw an exception?
    fGrammarPool->clear();
    // Even though fXSModel and fGrammarPoolXSModel will be invalid don't touch 
    // them here as getXSModel will handle this.

    //to clear all other references to the grammars just deleted from fGrammarPool
    fGrammarFromPool->removeAll(); 

}

void GrammarResolver::cacheGrammars()
{
    RefHashTableOfEnumerator<Grammar> grammarEnum(fGrammarBucket, false, fMemoryManager);
    ValueVectorOf<XMLCh*> keys(8, fMemoryManager);
    unsigned int keyCount = 0;

    // Build key set
    while (grammarEnum.hasMoreElements()) 
    {
        XMLCh* grammarKey = (XMLCh*) grammarEnum.nextElementKey();
        keys.addElement(grammarKey);
        keyCount++;
    }

    // PSVI: assume everything will be added, if caching fails add grammar back 
    //       into vector
    fGrammarsToAddToXSModel->removeAllElements();

    // Cache
    for (unsigned int i = 0; i < keyCount; i++) 
    {
        XMLCh* grammarKey = keys.elementAt(i);    

        /***
         * It is up to the GrammarPool implementation to handle duplicated grammar
         */
        Grammar* grammar = fGrammarBucket->get(grammarKey);
        if(fGrammarPool->cacheGrammar(grammar))
        {
            // only orphan grammar if grammar pool accepts caching of it
            fGrammarBucket->orphanKey(grammarKey);
        }
        else if (grammar->getGrammarType() == Grammar::SchemaGrammarType)
        {
            // add it back to list of grammars not in grammar pool
            fGrammarsToAddToXSModel->addElement((SchemaGrammar*) grammar);           
        }
    }

}

// ---------------------------------------------------------------------------
//  GrammarResolver: Setter methods
// ---------------------------------------------------------------------------
void GrammarResolver::cacheGrammarFromParse(const bool aValue)
{
    reset();
    fCacheGrammar = aValue;
}

Grammar* GrammarResolver::orphanGrammar(const XMLCh* const nameSpaceKey)
{
    if (fCacheGrammar)
    {
        Grammar* grammar = fGrammarPool->orphanGrammar(nameSpaceKey);
        if (grammar)
        {
            if (fGrammarFromPool->containsKey(nameSpaceKey))
                fGrammarFromPool->removeKey(nameSpaceKey);
        }
        // Check to see if it's in fGrammarBucket, since
        // we put it there if the grammar pool refused to
        // cache it.
        else if (fGrammarBucket->containsKey(nameSpaceKey))
        {
            grammar = fGrammarBucket->orphanKey(nameSpaceKey);
        }

        return grammar;
    }
    else
    {
        return fGrammarBucket->orphanKey(nameSpaceKey);
    }
}

XSModel *GrammarResolver::getXSModel()
{
    XSModel* xsModel;
    if (fCacheGrammar || fUseCachedGrammar)
    {
        // We know if the grammarpool changed thru caching, orphaning and erasing
        // but NOT by other mechanisms such as lockPool() or unlockPool() so it
        // is safest to always get it.  The grammarPool XSModel will only be 
        // regenerated if something changed.
        bool XSModelWasChanged;
        // The grammarpool will always return an xsmodel, even if it is just
        // the schema for schema xsmodel...
        xsModel = fGrammarPool->getXSModel(XSModelWasChanged);
        if (XSModelWasChanged)
        {
            // we know the grammarpool XSModel has changed or this is the
            // first call to getXSModel
            if (!fGrammarPoolXSModel && (fGrammarsToAddToXSModel->size() == 0) &&
                !fXSModel)
            { 
                fGrammarPoolXSModel = xsModel;
                return fGrammarPoolXSModel;
            }
            else
            {
                fGrammarPoolXSModel = xsModel;
                // We had previously augmented the grammar pool XSModel
                // with our our grammars or we would like to upate it now
                // so we have to regenerate the XSModel
                fGrammarsToAddToXSModel->removeAllElements();
                RefHashTableOfEnumerator<Grammar> grammarEnum(fGrammarBucket, false, fMemoryManager);
                while (grammarEnum.hasMoreElements()) 
                {
                    Grammar& grammar = (Grammar&) grammarEnum.nextElement();
                    if (grammar.getGrammarType() == Grammar::SchemaGrammarType)
                        fGrammarsToAddToXSModel->addElement((SchemaGrammar*)&grammar);
                }
                delete fXSModel;
                if (fGrammarsToAddToXSModel->size())
                {                    
                    fXSModel = new (fMemoryManager) XSModel(fGrammarPoolXSModel, this, fMemoryManager);
                    fGrammarsToAddToXSModel->removeAllElements();
                    return fXSModel;
                }       
                fXSModel = 0; 
                return fGrammarPoolXSModel;                                                      
            }       
        }
        else {
            // we know that the grammar pool XSModel is the same as before
            if (fGrammarsToAddToXSModel->size())
            {
                // we need to update our fXSModel with the new grammars               
                if (fXSModel)
                {
                    xsModel = new (fMemoryManager) XSModel(fXSModel, this, fMemoryManager);                   
                    fXSModel = xsModel;
                }
                else
                {
                    fXSModel = new (fMemoryManager) XSModel(fGrammarPoolXSModel, this, fMemoryManager);
                }
                fGrammarsToAddToXSModel->removeAllElements();
                return fXSModel;
            }
            // Nothing has changed!
            if (fXSModel)
            {
                return fXSModel;
            }
            else if (fGrammarPoolXSModel)
            {
                return fGrammarPoolXSModel;
            }
            fXSModel = new (fMemoryManager) XSModel(0, this, fMemoryManager);    
            return fXSModel;    
        }
    }
    // Not Caching...
    if (fGrammarsToAddToXSModel->size())
    {      
        xsModel = new (fMemoryManager) XSModel(fXSModel, this, fMemoryManager);
        fGrammarsToAddToXSModel->removeAllElements();
        fXSModel = xsModel;             
    }
    else if (!fXSModel)
    {
        // create a new model only if we didn't have one already
        fXSModel = new (fMemoryManager) XSModel(0, this, fMemoryManager);
    }
    return fXSModel; 
}

XERCES_CPP_NAMESPACE_END
