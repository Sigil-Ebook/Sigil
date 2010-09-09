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



// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/XTemplateSerializer.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/validators/common/Grammar.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

XERCES_CPP_NAMESPACE_BEGIN

/***
 * internal class meant to be comsumed by XTemplateSerializer only
 * the size can not grow
 ***/
#ifdef XERCES_DEBUG_SORT_GRAMMAR

class KeySet : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Contructors and Destructor
    // -----------------------------------------------------------------------
    KeySet
    (
      const XMLCh* const         strKey
    , const int                  intKey1
    , const int                  intKey2
    ,       MemoryManager* const manager
    );

    ~KeySet();

    // -----------------------------------------------------------------------
    //  Public operators
    // -----------------------------------------------------------------------
    inline void getKeys(const XMLCh*&, int&, int&) const;
           void print() const;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    KeySet(const KeySet&);
    KeySet& operator=(const KeySet&);

    // -----------------------------------------------------------------------
    //  Data members
    //  reference only
    // -----------------------------------------------------------------------
    const XMLCh* const    fStrKey;
    const int             fIntKey1;
    const int             fIntKey2;
          MemoryManager*  fMemoryManager;

};

KeySet::KeySet(const XMLCh* const         strKey
             , const int                  intKey1
             , const int                  intKey2
             ,       MemoryManager* const manager)
:fStrKey(strKey)
,fIntKey1(intKey1)
,fIntKey2(intKey2)
,fMemoryManager(manager)
{
}

KeySet::~KeySet()
{
}

inline
void KeySet::getKeys(const XMLCh*& strKey, int& intKey1, int& intKey2) const
{
    strKey  = fStrKey;
    intKey1 = fIntKey1;
    intKey2 = fIntKey2;
}

void KeySet::print() const
{
    char* tmpStr = XMLString::transcode(fStrKey, fMemoryManager);
    printf("tmpStr=<%s>, intKey1=<%d>, intKey2=<%d>\n", tmpStr, fIntKey1, fIntKey2);
    XMLString::release(&tmpStr, fMemoryManager);
}

static int compareKeySet(const void* keyl, const void* keyr)
{
    const KeySet* pairl=*(const KeySet**)keyl;
    const KeySet* pairr=*(const KeySet**)keyr;

    const XMLCh* strKeyl   = 0;
    int          intKeyl_1 = 0;
    int          intKeyl_2 = 0;
    pairl->getKeys(strKeyl, intKeyl_1, intKeyl_2);

    const XMLCh* strKeyr   = 0;
    int          intKeyr_1 = 0;
    int          intKeyr_2 = 0;
    pairr->getKeys(strKeyr, intKeyr_1, intKeyr_2);

    int compareValue = XMLString::compareString(strKeyl, strKeyr);

    if (compareValue !=0)
        return compareValue;

    compareValue = intKeyl_1 - intKeyr_1;
    if (compareValue !=0)
        return compareValue;

    return (intKeyl_2 - intKeyr_2);

}

class SortArray : public XMemory
{
private :
    // -----------------------------------------------------------------------
    //  Contructors and Destructor
    // -----------------------------------------------------------------------
    SortArray
    (
          const XMLSize_t            size
        ,       MemoryManager* const manager
    );

	~SortArray();

    // -----------------------------------------------------------------------
    //  Public operators
    // -----------------------------------------------------------------------
    inline const KeySet* elementAt(const XMLSize_t index)       const;
           void  addElement(const KeySet* const keySet);
           void  sort();
           void  dump() const;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    SortArray(const SortArray&);
    SortArray& operator=(const SortArray&);

    // -----------------------------------------------------------------------
    //  Data members
    // -----------------------------------------------------------------------
          XMLSize_t       fCur;      //point to the current available slot
          XMLSize_t       fSize;
    const KeySet**        fElemList;  //owning objects
          MemoryManager*  fMemoryManager;

    friend class XTemplateSerializer;

};

SortArray::SortArray(const XMLSize_t            size
                   ,       MemoryManager* const manager )
:fCur(0)
,fSize(size)
,fMemoryManager(manager)
{
    fElemList = (const KeySet**) fMemoryManager->allocate(size * sizeof (KeySet*));
}

SortArray::~SortArray()
{
    for (XMLSize_t i=0; i< fSize; i++)
        delete fElemList[i];

    fMemoryManager->deallocate(fElemList);
}

inline
const KeySet* SortArray::elementAt(const XMLSize_t index) const
{
    assert(index < fCur);
    return fElemList[index];
}

void SortArray::addElement(const KeySet* const keySet)
{
    assert(fCur < fSize);
    fElemList[fCur++]=keySet;
}

void SortArray::sort()
{
    assert(fCur == fSize);
    //dump();
    qsort(fElemList, fSize, sizeof (KeySet*), compareKeySet);
    //dump();
}

void  SortArray::dump() const
{
    printf("\n fSize=<%d>, fCur=<%d>\n", fSize, fCur);
    for (XMLSize_t i = 0; i < fCur; i++)
        fElemList[i]->print();
}

#define GET_NEXT_KEYSET()                            \
    const KeySet* keySet  = sortArray.elementAt(i);  \
    const XMLCh*  strKey  = 0;                       \
    int           intKey1 = 0;                       \
    int           intKey2 = 0;                       \
    keySet->getKeys(strKey, intKey1, intKey2);

#define SORT_KEYSET_ONEKEY(MM)                                   \
   SortArray sortArray(itemNumber, MM);                          \
   while (e.hasMoreElements())                                   \
   {                                                             \
       KeySet* keySet = new (MM) KeySet((XMLCh*) e.nextElementKey(), 0, 0, MM); \
       sortArray.addElement(keySet);                             \
   }                                                             \
   sortArray.sort();

#define SORT_KEYSET_TWOKEYS(MM)                                  \
   SortArray sortArray(itemNumber, MM);                          \
   while (e.hasMoreElements())                                   \
   {                                                             \
       XMLCh*     strKey;                                        \
       int        intKey;                                        \
       e.nextElementKey((void*&)strKey, intKey);                 \
       KeySet* keySet = new (MM) KeySet(strKey, intKey, 0, MM);  \
       sortArray.addElement(keySet);                             \
   }                                                             \
   sortArray.sort();

#endif

/**********************************************************
 *
 * ValueVectorOf
 *
 *   SchemaElementDecl*
 *   unsigned int
 *
 ***********************************************************/
void XTemplateSerializer::storeObject(ValueVectorOf<SchemaElementDecl*>* const objToStore
                                    , XSerializeEngine&                        serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        XMLSize_t vectorSize = objToStore->size();
        serEng.writeSize (vectorSize);

        for ( XMLSize_t i = 0; i < vectorSize; i++)
        {
            SchemaElementDecl*& data = objToStore->elementAt(i);
            serEng<<data;
        }
    }
}

void XTemplateSerializer::loadObject(ValueVectorOf<SchemaElementDecl*>**       objToLoad
                                   , int                                       initSize
                                   , bool                                      toCallDestructor
                                   , XSerializeEngine&                         serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             ValueVectorOf<SchemaElementDecl*>(
                                                               initSize
                                                             , serEng.getMemoryManager()
                                                             , toCallDestructor
                                                             );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorSize = 0;
        serEng.readSize (vectorSize);

        for (XMLSize_t i = 0; i < vectorSize; i++)
        {
            SchemaElementDecl* data;
            serEng>>data;
            (*objToLoad)->addElement(data);
        }
    }
}

void XTemplateSerializer::storeObject(ValueVectorOf<unsigned int>* const objToStore
                                    , XSerializeEngine&                  serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        XMLSize_t vectorSize = objToStore->size();
        serEng.writeSize (vectorSize);

        for (XMLSize_t i = 0; i < vectorSize; i++)
        {
            unsigned int& data = objToStore->elementAt(i);
            serEng<<data;
        }
    }
}

void XTemplateSerializer::loadObject(ValueVectorOf<unsigned int>**       objToLoad
                                   , int                                 initSize
                                   , bool                                toCallDestructor
                                   , XSerializeEngine&                   serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             ValueVectorOf<unsigned int>(
                                                         initSize
                                                       , serEng.getMemoryManager()
                                                       , toCallDestructor
                                                       );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorSize = 0;
        serEng.readSize (vectorSize);

        for (XMLSize_t i = 0; i < vectorSize; i++)
        {
            unsigned int data;
            serEng>>data;
            (*objToLoad)->addElement(data);
        }
    }

}

/**********************************************************
 *
 * RefArrayVectorOf
 *
 *   XMLCh
 *
 ***********************************************************/

void XTemplateSerializer::storeObject(RefArrayVectorOf<XMLCh>* const objToStore
                                    , XSerializeEngine&              serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        XMLSize_t vectorSize = objToStore->size();
        serEng.writeSize (vectorSize);

        for (XMLSize_t i = 0; i < vectorSize; i++)
        {
            serEng.writeString(objToStore->elementAt(i));
        }
    }

}

void XTemplateSerializer::loadObject(RefArrayVectorOf<XMLCh>**  objToLoad
                                   , int                        initSize
                                   , bool                       toAdopt
                                   , XSerializeEngine&          serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             RefArrayVectorOf<XMLCh>(
                                                     initSize
                                                   , toAdopt
                                                   , serEng.getMemoryManager()
                                                   );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorSize = 0;
        serEng.readSize (vectorSize);

        for (XMLSize_t i = 0; i < vectorSize; i++)
        {
            XMLCh* data;
            serEng.readString(data);
            (*objToLoad)->addElement(data);
        }
    }

}

/**********************************************************
 *
 * RefVectorOf
 *
 *   SchemaAttDef
 *   SchemaElementDecl
 *   ContentSpecNode
 *   IC_Field
 *   DatatypeValidator
 *   IdentityConstraint
 *   XMLNumber
 *   XercesLocationPath
 *   XercesStep
 *
 ***********************************************************/

void XTemplateSerializer::storeObject(RefVectorOf<SchemaAttDef>* const objToStore
                                    , XSerializeEngine&                serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {

        XMLSize_t vectorLength = objToStore->size();
        serEng.writeSize (vectorLength);

        for ( XMLSize_t i = 0; i < vectorLength; i++)
        {
            SchemaAttDef* data = objToStore->elementAt(i);
            serEng<<data;
        }

    }

}

void XTemplateSerializer::loadObject(RefVectorOf<SchemaAttDef>** objToLoad
                                   , int                         initSize
                                   , bool                        toAdopt
                                   , XSerializeEngine&           serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {

        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             RefVectorOf<SchemaAttDef>(
                                                       initSize
                                                     , toAdopt
                                                     , serEng.getMemoryManager()
                                                     );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorLength = 0;
        serEng.readSize (vectorLength);
        for (XMLSize_t i = 0 ; i < vectorLength; i++)
        {
            SchemaAttDef* data;
            serEng>>data;
            (*objToLoad)->addElement(data);
        }

    }

}

void XTemplateSerializer::storeObject(RefVectorOf<SchemaElementDecl>* const objToStore
                                    , XSerializeEngine&                     serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {

        XMLSize_t vectorLength = objToStore->size();
        serEng.writeSize (vectorLength);

        for ( XMLSize_t i = 0; i < vectorLength; i++)
        {
            SchemaElementDecl* data = objToStore->elementAt(i);
            serEng<<data;
        }

    }

}

void XTemplateSerializer::loadObject(RefVectorOf<SchemaElementDecl>** objToLoad
                                   , int                              initSize
                                   , bool                             toAdopt
                                   , XSerializeEngine&                serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             RefVectorOf<SchemaElementDecl>(
                                                            initSize
                                                          , toAdopt
                                                          , serEng.getMemoryManager()
                                                          );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorLength = 0;
        serEng.readSize (vectorLength);
        for (XMLSize_t i = 0 ; i < vectorLength; i++)
        {
            SchemaElementDecl* data;
            serEng>>data;
            (*objToLoad)->addElement(data);
        }
    }

}

void XTemplateSerializer::storeObject(RefVectorOf<ContentSpecNode>* const objToStore
                                    , XSerializeEngine&                   serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {
        XMLSize_t vectorLength = objToStore->size();
        serEng.writeSize (vectorLength);

        for ( XMLSize_t i = 0; i < vectorLength; i++)
        {
            ContentSpecNode* data = objToStore->elementAt(i);
            serEng<<data;
        }
    }

}

void XTemplateSerializer::loadObject(RefVectorOf<ContentSpecNode>** objToLoad
                                   , int                            initSize
                                   , bool                           toAdopt
                                   , XSerializeEngine&              serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             RefVectorOf<ContentSpecNode>(
                                                          initSize
                                                        , toAdopt
                                                        , serEng.getMemoryManager()
                                                        );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorLength = 0;
        serEng.readSize (vectorLength);
        for (XMLSize_t i = 0 ; i < vectorLength; i++)
        {
            ContentSpecNode* data;
            serEng>>data;
            (*objToLoad)->addElement(data);
        }
    }

}

void XTemplateSerializer::storeObject(RefVectorOf<IC_Field>* const objToStore
                                    , XSerializeEngine&            serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {
        XMLSize_t vectorLength = objToStore->size();
        serEng.writeSize (vectorLength);

        for ( XMLSize_t i = 0; i < vectorLength; i++)
        {
            IC_Field* data = objToStore->elementAt(i);
            serEng<<data;
        }
    }

}

void XTemplateSerializer::loadObject(RefVectorOf<IC_Field>** objToLoad
                                   , int                     initSize
                                   , bool                    toAdopt
                                   , XSerializeEngine&       serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             RefVectorOf<IC_Field>(
                                                   initSize
                                                 , toAdopt
                                                 , serEng.getMemoryManager()
                                                 );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorLength = 0;
        serEng.readSize (vectorLength);
        for (XMLSize_t i = 0 ; i < vectorLength; i++)
        {
            IC_Field* data;
            serEng>>data;
            (*objToLoad)->addElement(data);
        }
    }

}

void XTemplateSerializer::storeObject(RefVectorOf<DatatypeValidator>* const objToStore
                                    , XSerializeEngine&                      serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {
        XMLSize_t vectorLength = objToStore->size();
        serEng.writeSize (vectorLength);

        for ( XMLSize_t i = 0; i < vectorLength; i++)
        {
            DatatypeValidator* data = objToStore->elementAt(i);
            DatatypeValidator::storeDV(serEng, data);
        }
    }

}

void XTemplateSerializer::loadObject(RefVectorOf<DatatypeValidator>** objToLoad
                                   , int                               initSize
                                   , bool                              toAdopt
                                   , XSerializeEngine&                 serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             RefVectorOf<DatatypeValidator>(
                                                            initSize
                                                          , toAdopt
                                                          , serEng.getMemoryManager()
                                                           );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorLength = 0;
        serEng.readSize (vectorLength);
        for (XMLSize_t i = 0 ; i < vectorLength; i++)
        {
            DatatypeValidator*  data;
            data = DatatypeValidator::loadDV(serEng);
            (*objToLoad)->addElement(data);
        }
    }

}

void XTemplateSerializer::storeObject(RefVectorOf<IdentityConstraint>* const objToStore
                                    , XSerializeEngine&                       serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {
        XMLSize_t vectorLength = objToStore->size();
        serEng.writeSize (vectorLength);

        for ( XMLSize_t i = 0; i < vectorLength; i++)
        {
            IdentityConstraint* data = objToStore->elementAt(i);
            IdentityConstraint::storeIC(serEng, data);
        }
    }

}

void XTemplateSerializer::loadObject(RefVectorOf<IdentityConstraint>** objToLoad
                                   , int                                initSize
                                   , bool                               toAdopt
                                   , XSerializeEngine&                  serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             RefVectorOf<IdentityConstraint>(
                                                             initSize
                                                           , toAdopt
                                                           , serEng.getMemoryManager()
                                                            );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorLength = 0;
        serEng.readSize (vectorLength);
        for (XMLSize_t i = 0 ; i < vectorLength; i++)
        {
            IdentityConstraint*  data;
            data = IdentityConstraint::loadIC(serEng);
            (*objToLoad)->addElement(data);
        }
    }

}

void XTemplateSerializer::storeObject(RefVectorOf<XMLNumber>* const objToStore
                                    , XSerializeEngine&             serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {
        XMLSize_t vectorLength = objToStore->size();
        serEng.writeSize (vectorLength);

        for ( XMLSize_t i = 0; i < vectorLength; i++)
        {
            XMLNumber* data = objToStore->elementAt(i);
            serEng<<data;
        }
    }

}

void XTemplateSerializer::loadObject(RefVectorOf<XMLNumber>** objToLoad
                                   , int                       initSize
                                   , bool                      toAdopt
                                   , XMLNumber::NumberType     numType
                                   , XSerializeEngine&         serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             RefVectorOf<XMLNumber>(
                                                    initSize
                                                  , toAdopt
                                                  , serEng.getMemoryManager()
                                                   );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorLength = 0;
        serEng.readSize (vectorLength);
        for (XMLSize_t i = 0 ; i < vectorLength; i++)
        {
            XMLNumber*  data;
            data = XMLNumber::loadNumber(numType , serEng);
            (*objToLoad)->addElement(data);
        }
    }

}

void XTemplateSerializer::storeObject(RefVectorOf<XercesLocationPath>* const objToStore
                                    , XSerializeEngine&                      serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {
        XMLSize_t vectorLength = objToStore->size();
        serEng.writeSize (vectorLength);

        for ( XMLSize_t i = 0; i < vectorLength; i++)
        {
            XercesLocationPath* data = objToStore->elementAt(i);
            serEng<<data;
        }
    }

}

void XTemplateSerializer::loadObject(RefVectorOf<XercesLocationPath>** objToLoad
                                   , int                               initSize
                                   , bool                              toAdopt
                                   , XSerializeEngine&                 serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             RefVectorOf<XercesLocationPath>(
                                                             initSize
                                                           , toAdopt
                                                           , serEng.getMemoryManager()
                                                            );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorLength = 0;
        serEng.readSize (vectorLength);
        for (XMLSize_t i = 0 ; i < vectorLength; i++)
        {
            XercesLocationPath*  data;
            serEng>>data;
            (*objToLoad)->addElement(data);
        }
    }

}

void XTemplateSerializer::storeObject(RefVectorOf<XercesStep>* const objToStore
                                    , XSerializeEngine&              serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {
        XMLSize_t vectorLength = objToStore->size();
        serEng.writeSize (vectorLength);

        for (XMLSize_t i = 0; i < vectorLength; i++)
        {
            XercesStep* data = objToStore->elementAt(i);
            serEng<<data;
        }
    }

}

void XTemplateSerializer::loadObject(RefVectorOf<XercesStep>** objToLoad
                                   , int                       initSize
                                   , bool                      toAdopt
                                   , XSerializeEngine&         serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             RefVectorOf<XercesStep>(
                                                     initSize
                                                   , toAdopt
                                                   , serEng.getMemoryManager()
                                                    );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t vectorLength = 0;
        serEng.readSize (vectorLength);
        for (XMLSize_t i = 0 ; i < vectorLength; i++)
        {
            XercesStep*  data;
            serEng>>data;
            (*objToLoad)->addElement(data);
        }
    }

}

/**********************************************************
 *
 * RefHashTableOf
 *
 *   KVStringPair
 *   XMLAttDef
 *   DTDAttDef
 *   ComplexTypeInfo
 *   XercesGroupInfo
 *   XercesAttGroupInfo
 *   XMLRefInfo
 *   DatatypeValidator
 *   Grammar
 *   XSAnnotation
 *
 ***********************************************************/
void XTemplateSerializer::storeObject(RefHashTableOf<KVStringPair>* const objToStore
                                    , XSerializeEngine&                    serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        RefHashTableOfEnumerator<KVStringPair> e(objToStore, false, objToStore->getMemoryManager());
        XMLSize_t itemNumber = 0;

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //to sort the key
        SORT_KEYSET_ONEKEY(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            KVStringPair* data = objToStore->get(strKey);
            serEng<<data;
        }
#else
        while (e.hasMoreElements())
        {
            KVStringPair* data = objToStore->get(e.nextElementKey());
            serEng<<data;
        }
#endif
    }
}

void XTemplateSerializer::loadObject(RefHashTableOf<KVStringPair>** objToLoad
                                   , int
                                   , bool                           toAdopt
                                   , XSerializeEngine&              serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHashTableOf<KVStringPair>(
                                                          hashModulus
                                                        , toAdopt
                                                        , serEng.getMemoryManager()
                                                         );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            KVStringPair*  data;
            serEng>>data;

            (*objToLoad)->put((void*)data->getKey(), data);
        }
    }
}

void XTemplateSerializer::storeObject(RefHashTableOf<XMLAttDef>* const objToStore
                                    , XSerializeEngine&                serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        RefHashTableOfEnumerator<XMLAttDef> e(objToStore, false, objToStore->getMemoryManager());
        XMLSize_t itemNumber = 0;

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //to sort the key
        SORT_KEYSET_ONEKEY(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            XMLAttDef* data = objToStore->get(strKey);
            serEng<<data;
        }
#else
        while (e.hasMoreElements())
        {
            XMLAttDef* data = objToStore->get(e.nextElementKey());
            serEng<<data;
        }
#endif
    }
}

void XTemplateSerializer::loadObject(RefHashTableOf<XMLAttDef>** objToLoad
                                   , int
                                   , bool                        toAdopt
                                   , XSerializeEngine&           serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {

        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHashTableOf<XMLAttDef>(
                                                       hashModulus
                                                     , toAdopt
                                                     , serEng.getMemoryManager()
                                                      );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            //This is used solely by SchemaGrammar and by all means it must be
            //SchemaAttDef, ideally we may change it to RefHashTableOf<SchemaAttDef>
            //later on.
            //Affected files IGXMLScanner, SGXMLScanner, SchemaGrammar, TraverseSchema
            //XMLAttDef*  data;
            SchemaAttDef*  data;
            serEng>>data;

            (*objToLoad)->put((void*)data->getAttName()->getLocalPart(), data);
        }
    }
}

void XTemplateSerializer::storeObject(RefHashTableOf<DTDAttDef>* const objToStore
                                    , XSerializeEngine&                serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        RefHashTableOfEnumerator<DTDAttDef> e(objToStore, false, objToStore->getMemoryManager());
        XMLSize_t itemNumber = 0;

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //to sort the key
        SORT_KEYSET_ONEKEY(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            DTDAttDef* data = objToStore->get(strKey);
            serEng<<data;
        }
#else
        while (e.hasMoreElements())
        {
            DTDAttDef* data = objToStore->get(e.nextElementKey());
            serEng<<data;
        }
#endif
    }
}

void XTemplateSerializer::loadObject(RefHashTableOf<DTDAttDef>** objToLoad
                                   , int
                                   , bool                        toAdopt
                                   , XSerializeEngine&           serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {

        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHashTableOf<DTDAttDef>(
                                                       hashModulus
                                                     , toAdopt
                                                     , serEng.getMemoryManager()
                                                      );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            DTDAttDef*  data;
            serEng>>data;

            (*objToLoad)->put((void*)data->getFullName(), data);
        }
    }
}

void XTemplateSerializer::storeObject(RefHashTableOf<ComplexTypeInfo>* const objToStore
                                    , XSerializeEngine&                      serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        RefHashTableOfEnumerator<ComplexTypeInfo> e(objToStore, false, objToStore->getMemoryManager());
        XMLSize_t itemNumber = 0;

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //to sort the key
        SORT_KEYSET_ONEKEY(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            ComplexTypeInfo* data = objToStore->get(strKey);
            serEng<<data;
        }
#else
        while (e.hasMoreElements())
        {
            ComplexTypeInfo* data = objToStore->get(e.nextElementKey());
            serEng<<data;
        }
#endif
    }
}

void XTemplateSerializer::loadObject(RefHashTableOf<ComplexTypeInfo>** objToLoad
                                   , int
                                   , bool                              toAdopt
                                   , XSerializeEngine&                 serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHashTableOf<ComplexTypeInfo>(
                                                             hashModulus
                                                           , toAdopt
                                                           , serEng.getMemoryManager()
                                                           );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            ComplexTypeInfo*  data;
            serEng>>data;

           (*objToLoad)->put((void*)data->getTypeName(), data);
        }
    }
}

void XTemplateSerializer::storeObject(RefHashTableOf<XercesGroupInfo>* const objToStore
                                    , XSerializeEngine&                      serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        RefHashTableOfEnumerator<XercesGroupInfo> e(objToStore, false, objToStore->getMemoryManager());
        XMLSize_t itemNumber = 0;

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //to sort the key
        SORT_KEYSET_ONEKEY(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            unsigned int id  = serEng.getStringPool()->getId(strKey);
            serEng<<id;

            XercesGroupInfo* data = objToStore->get(strKey);
            serEng<<data;
        }
#else
        while (e.hasMoreElements())
        {
            XMLCh*       key = (XMLCh*) e.nextElementKey();
            unsigned int id  = serEng.getStringPool()->getId(key);

           // key = StringPool->getValueForId(XercesGroupInfo::getNameSpaceId())
           //     + chComma
           //     + StringPool->getValueForId(XercesGroupInfo::getNameId())
           //
           // and the key is guranteed in the StringPool
           //
           //
           //  if (id == 0)
           //  {
           //      throw exception
           //   }
           //

            serEng<<id;

            XercesGroupInfo* data = objToStore->get(key);
            serEng<<data;
        }
#endif
    }
}

void XTemplateSerializer::loadObject(RefHashTableOf<XercesGroupInfo>** objToLoad
                                   , int
                                   , bool                              toAdopt
                                   , XSerializeEngine&                 serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHashTableOf<XercesGroupInfo>(
                                                             hashModulus
                                                           , toAdopt
                                                           , serEng.getMemoryManager()
                                                           );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            unsigned int id;
            serEng>>id;

            XMLCh* key = (XMLCh*) serEng.getStringPool()->getValueForId(id);

            XercesGroupInfo*  data;
            serEng>>data;

            (*objToLoad)->put((void*)key, data);
        }
    }
}

void XTemplateSerializer::storeObject(RefHashTableOf<XercesAttGroupInfo>* const objToStore
                                    , XSerializeEngine&                         serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        RefHashTableOfEnumerator<XercesAttGroupInfo> e(objToStore, false, objToStore->getMemoryManager());
        XMLSize_t itemNumber = 0;

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //to sort the key
        SORT_KEYSET_ONEKEY(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            XercesAttGroupInfo* data = objToStore->get(strKey);
            serEng<<data;
        }
#else
        while (e.hasMoreElements())
        {
            XercesAttGroupInfo* data = objToStore->get(e.nextElementKey());
            serEng<<data;
        }
#endif
    }
}

void XTemplateSerializer::loadObject(RefHashTableOf<XercesAttGroupInfo>** objToLoad
                                   , int
                                   , bool                                 toAdopt
                                   , XSerializeEngine&                    serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHashTableOf<XercesAttGroupInfo>(
                                                                hashModulus
                                                              , toAdopt
                                                              , serEng.getMemoryManager()
                                                              );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            XercesAttGroupInfo*  data;
            serEng>>data;

            XMLCh* key = (XMLCh*) serEng.getStringPool()->getValueForId(data->getNameId());
            (*objToLoad)->put((void*)key, data);
        }
    }
}

void XTemplateSerializer::storeObject(RefHashTableOf<XMLRefInfo>* const objToStore
                                    , XSerializeEngine&                 serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        RefHashTableOfEnumerator<XMLRefInfo> e(objToStore, false, objToStore->getMemoryManager());
        XMLSize_t itemNumber = 0;

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //to sort the key
        SORT_KEYSET_ONEKEY(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            serEng.writeString(strKey);

            XMLRefInfo* data = objToStore->get(strKey);
            serEng<<data;
        }
#else
        while (e.hasMoreElements())
        {
            XMLCh*     key  = (XMLCh*) e.nextElementKey();
            serEng.writeString(key);

            XMLRefInfo* data = objToStore->get(key);
            serEng<<data;
        }
#endif
    }
}

void XTemplateSerializer::loadObject(RefHashTableOf<XMLRefInfo>** objToLoad
                                   , int
                                   , bool                         toAdopt
                                   , XSerializeEngine&            serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHashTableOf<XMLRefInfo>(
                                                        hashModulus
                                                      , toAdopt
                                                      , serEng.getMemoryManager()
                                                      );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            XMLCh*      key;
            serEng.readString(key);

            XMLRefInfo*  data;
            serEng>>data;

            (*objToLoad)->put((void*)key, data);
        }
    }
}

void XTemplateSerializer::storeObject(RefHashTableOf<DatatypeValidator>* const objToStore
                                    , XSerializeEngine&                        serEng)
{

    if (serEng.needToStoreObject(objToStore))
    {

        serEng.writeSize (objToStore->getHashModulus());

        RefHashTableOfEnumerator<DatatypeValidator> e(objToStore, false, objToStore->getMemoryManager());
        XMLSize_t itemNumber = 0;

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //to sort the key
        SORT_KEYSET_ONEKEY(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            DatatypeValidator* data = objToStore->get(strKey);
            DatatypeValidator::storeDV(serEng, data);
        }
#else
        while (e.hasMoreElements())
        {
            /***
             * to do: verify valid id
             *
             *    XMLCh*  key = (XMLCh*) e.nextElementKey();
             *    unsigned int id = serEng.getStringPool()->getId(key);
             *    if (id == 0)
             *        throw exception
             ***/
            DatatypeValidator* data = objToStore->get(e.nextElementKey());
            DatatypeValidator::storeDV(serEng, data);
        }
#endif
    }
}

void XTemplateSerializer::loadObject(RefHashTableOf<DatatypeValidator>** objToLoad
                                   , int
                                   , bool                                toAdopt
                                   , XSerializeEngine&                   serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHashTableOf<DatatypeValidator>(
                                                               hashModulus
                                                             , toAdopt
                                                             , serEng.getMemoryManager()
                                                             );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            DatatypeValidator*  data;
            data = DatatypeValidator::loadDV(serEng);

            /***
             *   restore the key
             ***/
            XMLCh*       typeUri   = (XMLCh*) data->getTypeUri();
            XMLCh*       typeLocal = (XMLCh*) data->getTypeLocalName();
            XMLSize_t    uriLen    = XMLString::stringLen(typeUri);
            XMLSize_t    localLen  = XMLString::stringLen(typeLocal);
            XMLCh*       typeKey   = (XMLCh*) serEng.getMemoryManager()->allocate
                                     (
                                       (uriLen + localLen + 2) * sizeof(XMLCh)
                                     );
            // "typeuri,typeLocal"
            XMLString::moveChars(typeKey, typeUri, uriLen+1);
            typeKey[uriLen] = chComma;
            XMLString::moveChars(&typeKey[uriLen+1], typeLocal, localLen+1);
            typeKey[uriLen + localLen + 1] = chNull;
            ArrayJanitor<XMLCh> janName(typeKey, serEng.getMemoryManager());

            /*
             * get the string from string pool
             *
             *  to do:
             ***/
            unsigned int id = serEng.getStringPool()->getId(typeKey);
            XMLCh* refKey = (XMLCh*) serEng.getStringPool()->getValueForId(id);

            (*objToLoad)->put((void*)refKey, data);
        }
    }
}

void XTemplateSerializer::storeObject(RefHashTableOf<Grammar>* const objToStore
                                    , XSerializeEngine&              serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        RefHashTableOfEnumerator<Grammar> e(objToStore, false, objToStore->getMemoryManager());
        XMLSize_t itemNumber = 0;

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //to sort the key
        SORT_KEYSET_ONEKEY(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            Grammar* data = objToStore->get(strKey);
            Grammar::storeGrammar(serEng, data);
        }
#else
        while (e.hasMoreElements())
        {
            Grammar* data = objToStore->get(e.nextElementKey());
            Grammar::storeGrammar(serEng, data);
        }
#endif

    }
}

void XTemplateSerializer::loadObject(RefHashTableOf<Grammar>** objToLoad
                                   , int
                                   , bool                      toAdopt
                                   , XSerializeEngine&         serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHashTableOf<Grammar>(
                                                     hashModulus
                                                   , toAdopt
                                                   , serEng.getMemoryManager()
                                                   );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            Grammar*  data;
            data = Grammar::loadGrammar(serEng);

            XMLCh* key = (XMLCh*) data->getGrammarDescription()->getGrammarKey();
            (*objToLoad)->put(key, data);
        }
    }
}


void XTemplateSerializer::storeObject(RefHashTableOf<XSAnnotation, PtrHasher>* const objToStore
                                    , XSerializeEngine&              serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        RefHashTableOfEnumerator<XSAnnotation, PtrHasher> e(objToStore, false, objToStore->getMemoryManager());

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //get the total item number
        unsigned int   itemNumber = 0;
        while (e.hasMoreElements())
        {
            void* key = e.nextElementKey();
            XSerializeEngine::XSerializedObjectId_t keyId = serEng.lookupStorePool(key);

            if (keyId)
                itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

        //to sort the key
        //though keyId is not supposed to be involved in compare
        //we merely use the KeySet to encap both the string key and keyid
        SortArray sortArray(itemNumber, objToStore->getMemoryManager());
        while (e.hasMoreElements())
        {
            void* key = e.nextElementKey();
            XSerializeEngine::XSerializedObjectId_t keyId = serEng.lookupStorePool(key);

            if (keyId)
            {
                KeySet* keySet =
                    new (objToStore->getMemoryManager()) KeySet((XMLCh*)key, keyId, 0, objToStore->getMemoryManager());
                sortArray.addElement(keySet);
            }

        }

        sortArray.sort();

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            XSerializeEngine::XSerializedObjectId_t keyId = (XSerializeEngine::XSerializedObjectId_t)intKey1;
            XSAnnotation* data = objToStore->get(strKey);

            serEng<<keyId;
            serEng<<data;
        }
#else
        ValueVectorOf<XSerializeEngine::XSerializedObjectId_t> ids(16, serEng.getMemoryManager());
        ValueVectorOf<void*> keys(16, serEng.getMemoryManager());

        while (e.hasMoreElements())
        {
            void* key = e.nextElementKey();
            XSerializeEngine::XSerializedObjectId_t keyId = serEng.lookupStorePool(key);

            if (keyId)
            {
                ids.addElement(keyId);
                keys.addElement(key);
            }
        }

        XMLSize_t itemNumber = ids.size();
        serEng.writeSize (itemNumber);

        for (XMLSize_t i=0; i<itemNumber; i++)
        {
            XSerializeEngine::XSerializedObjectId_t keyId = ids.elementAt(i);
            XSAnnotation* data = objToStore->get(keys.elementAt(i));
            serEng<<keyId;
            serEng<<data;
        }
#endif
    }
}

void XTemplateSerializer::loadObject(RefHashTableOf<XSAnnotation, PtrHasher>** objToLoad
                                   , int
                                   , bool                           toAdopt
                                   , XSerializeEngine&              serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                              RefHashTableOf<XSAnnotation, PtrHasher>(
                              hashModulus
                            , toAdopt
                            , serEng.getMemoryManager()
                              );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        XMLSize_t itemIndex;
        XSerializeEngine::XSerializedObjectId_t keyId;
        void* key;
        XSAnnotation*  data;
        if (!serEng.fGrammarPool->getIgnoreSerializedAnnotations()) {
            for (itemIndex = 0; itemIndex < itemNumber; itemIndex++)
            {
                serEng>>keyId;
                key = serEng.lookupLoadPool(keyId);
                serEng>>data;
                (*objToLoad)->put(key, data);
            }
        }
        else {
            for (itemIndex = 0; itemIndex < itemNumber; itemIndex++)
            {
                serEng>>keyId;
                key = serEng.lookupLoadPool(keyId);
                serEng>>data;
                delete data;
            }
        }
    }
}

/**********************************************************
 *
 * RefHash2KeysTableOf
 *
 *   SchemaAttDef
 *   ElemVector
 *
 ***********************************************************/
void XTemplateSerializer::storeObject(RefHash2KeysTableOf<SchemaAttDef>* const objToStore
                                    , XSerializeEngine&                        serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        XMLSize_t itemNumber = 0;

        RefHash2KeysTableOfEnumerator<SchemaAttDef> e(objToStore, false, objToStore->getMemoryManager());

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR
        //to sort the key
        SORT_KEYSET_TWOKEYS(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            SchemaAttDef* data = objToStore->get(strKey, intKey1);
            serEng<<data;

        }
#else

        while (e.hasMoreElements())
        {
            void*       key1;
            int        key2;
            e.nextElementKey(key1, key2);

            SchemaAttDef* data = objToStore->get(key1, key2);
            serEng<<data;

        }
#endif
    }

}

void XTemplateSerializer::loadObject(RefHash2KeysTableOf<SchemaAttDef>** objToLoad
                                   , int
                                   , bool                                toAdopt
                                   , XSerializeEngine&                   serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHash2KeysTableOf<SchemaAttDef>(
                                                               hashModulus
                                                             , toAdopt
                                                             , serEng.getMemoryManager()
                                                             );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            SchemaAttDef*  data;
            serEng>>data;

            XMLCh* key1 = data->getAttName()->getLocalPart();
            int    key2 = data->getAttName()->getURI();
            //key2==data->getId()
            (*objToLoad)->put((void*)key1, key2, data);

        }

    }

}

void XTemplateSerializer::storeObject(RefHash2KeysTableOf<ElemVector>* const objToStore
                                    , XSerializeEngine&                      serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        XMLSize_t itemNumber = 0;

        RefHash2KeysTableOfEnumerator<ElemVector> e(objToStore, false, objToStore->getMemoryManager());

        while (e.hasMoreElements())
        {
            e.nextElement();
            itemNumber++;
        }

        serEng.writeSize (itemNumber);
        e.Reset();

#ifdef XERCES_DEBUG_SORT_GRAMMAR

        //to sort the key
        SORT_KEYSET_TWOKEYS(serEng.getMemoryManager())

        //to store the data
        for (XMLSize_t i=0; i < itemNumber; i++)
        {
            GET_NEXT_KEYSET()

            serEng.writeString(strKey);
            serEng<<intKey1;

            ElemVector* data = objToStore->get(strKey, intKey1);
            storeObject(data, serEng);
        }
#else

        while (e.hasMoreElements())
        {
            void*      key1;
            int        key2;

            e.nextElementKey(key1, key2);
            serEng.writeString((const XMLCh*)key1);
            serEng<<key2;

            ElemVector* data = objToStore->get(key1, key2);
            storeObject(data, serEng);

        }
#endif
    }

}

void XTemplateSerializer::loadObject(RefHash2KeysTableOf<ElemVector>**      objToLoad
                                   , int
                                   , bool                                    toAdopt
                                   , XSerializeEngine&                       serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHash2KeysTableOf<ElemVector>(
                                                               hashModulus
                                                             , toAdopt
                                                             , serEng.getMemoryManager()
                                                             );
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            XMLCh*      key1;
            serEng.readString(key1);

            int         key2;
            serEng>>key2;

            ElemVector*  data = 0;

            //don't call destructor
            loadObject(&data, 8, false, serEng);

            /***
             *
             *  There must be one element in the vector whose
             *  susbititutionGroupElem matches the (key1,key2)
             *
             ***/

            // bool FOUND=false;

            XMLSize_t vectorSize = data->size();
            for ( XMLSize_t i = 0; i < vectorSize; i++)
            {
                SchemaElementDecl*& elem   = data->elementAt(i);
                SchemaElementDecl*  subElem = elem->getSubstitutionGroupElem();
                XMLCh* elemName = subElem->getBaseName();
                int    uri      = subElem->getURI();
                if (XMLString::equals(elemName, key1) &&
                    (uri == key2)                       )
                {
                    //release the temp
                    serEng.getMemoryManager()->deallocate(key1);
                    key1 = elemName;
                    //FOUND=true;
                    break;
                }
            }

            /***
             * if (!FOUND)
             * {
             *     throw exception
             * }
             ***/

            (*objToLoad)->put((void*)key1, key2, data);

        }

    }
}

/**********************************************************
 *
 * RefHash3KeysIdPool
 *
 *   SchemaElementDecl
 *
 *   maintain the same order through id
 ***********************************************************/
void XTemplateSerializer::storeObject(RefHash3KeysIdPool<SchemaElementDecl>* const objToStore
                                    , XSerializeEngine&                            serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        serEng.writeSize (objToStore->getHashModulus());

        RefHash3KeysIdPoolEnumerator<SchemaElementDecl> e(objToStore, false, objToStore->getMemoryManager());

        serEng.writeSize (e.size());

        void*  strkey;
        int    key1;
        int    key2;
        /* Update to store key2 separately as for the putGroupElemDecl the key is not the
           enclosing scope but another value. */
        while (e.hasMoreKeys())
        {
            e.nextElementKey(strkey, key1, key2);
            serEng<<key2;
            SchemaElementDecl* data = objToStore->getByKey(strkey, key1, key2);
            serEng<<data;
        }
    }

}

void XTemplateSerializer::loadObject(RefHash3KeysIdPool<SchemaElementDecl>** objToLoad
                                   , int
                                   , bool                                    toAdopt
                                   , int                                     initSize2
                                   , XSerializeEngine&                       serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        XMLSize_t hashModulus;
        serEng.readSize (hashModulus);

        if (!*objToLoad)
        {
            *objToLoad = new (serEng.getMemoryManager())
                             RefHash3KeysIdPool<SchemaElementDecl>(
                                                                   hashModulus
                                                                 , toAdopt
                                                                 , initSize2
                                                                 , serEng.getMemoryManager());
        }

        serEng.registerObject(*objToLoad);

        XMLSize_t itemNumber = 0;
        serEng.readSize (itemNumber);

        int scopeKey;
        SchemaElementDecl*  elemDecl;
        for (XMLSize_t itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            serEng>>scopeKey;
            serEng>>elemDecl;

           (*objToLoad)->put(elemDecl->getBaseName()
                            , elemDecl->getURI()
                            , scopeKey
                            , elemDecl);
        }

    }

}

/**********************************************************
 *
 * NameIdPool
 *    no NameIdPool::nextElementKey()
 *
 *   DTDElementDecl
 *   DTDEntityDecl
 *   XMLNotationDecl
 *
 *   maintain the same order through id
 ***********************************************************/
void XTemplateSerializer::storeObject(NameIdPool<DTDElementDecl>* const objToStore
                                    , XSerializeEngine&                 serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        NameIdPoolEnumerator<DTDElementDecl> e(objToStore, objToStore->getMemoryManager());

        serEng<<(unsigned int)e.size();

        while (e.hasMoreElements())
        {
            DTDElementDecl& data = e.nextElement();
            data.serialize(serEng);
        }
    }

}

void XTemplateSerializer::loadObject(NameIdPool<DTDElementDecl>** objToLoad
                                   , int                          initSize
                                   , int                          initSize2
                                   , XSerializeEngine&            serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             NameIdPool<DTDElementDecl>(
                                                        initSize
                                                      , initSize2
                                                      , serEng.getMemoryManager()
                                                      );
        }

        serEng.registerObject(*objToLoad);

        unsigned int itemNumber = 0;
        serEng >> itemNumber;

        for (unsigned int itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            DTDElementDecl*  data = new (serEng.getMemoryManager())
                                    DTDElementDecl(serEng.getMemoryManager());
            data->serialize(serEng);
            (*objToLoad)->put(data);
        }
    }
}

void XTemplateSerializer::storeObject(NameIdPool<DTDEntityDecl>* const objToStore
                                    , XSerializeEngine&                 serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        NameIdPoolEnumerator<DTDEntityDecl> e(objToStore, objToStore->getMemoryManager());

        serEng<<(unsigned int)e.size();

        while (e.hasMoreElements())
        {
            DTDEntityDecl& data = e.nextElement();
            data.serialize(serEng);
        }
    }
}

void XTemplateSerializer::loadObject(NameIdPool<DTDEntityDecl>** objToLoad
                                   , int                          initSize
                                   , int                          initSize2
                                   , XSerializeEngine&            serEng)
{
    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             NameIdPool<DTDEntityDecl>(
                                                       initSize
                                                     , initSize2
                                                     , serEng.getMemoryManager()
                                                     );
        }

        serEng.registerObject(*objToLoad);

        unsigned int itemNumber = 0;
        serEng >> itemNumber;

        for (unsigned int itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            DTDEntityDecl*  data = new (serEng.getMemoryManager())
                                   DTDEntityDecl(serEng.getMemoryManager());
            data->serialize(serEng);
            (*objToLoad)->put(data);
        }
    }
}

void XTemplateSerializer::storeObject(NameIdPool<XMLNotationDecl>* const objToStore
                                    , XSerializeEngine&                  serEng)
{
    if (serEng.needToStoreObject(objToStore))
    {
        NameIdPoolEnumerator<XMLNotationDecl> e(objToStore, objToStore->getMemoryManager());

        serEng<<(unsigned int)e.size();

        while (e.hasMoreElements())
        {
            XMLNotationDecl& data = e.nextElement();
            data.serialize(serEng);
        }
    }
}

void XTemplateSerializer::loadObject(NameIdPool<XMLNotationDecl>** objToLoad
                                   , int                          initSize
                                   , int                          initSize2
                                   , XSerializeEngine&            serEng)
{

    if (serEng.needToLoadObject((void**)objToLoad))
    {
        if (!*objToLoad)
        {
            if (initSize < 0)
                initSize = 16;

            *objToLoad = new (serEng.getMemoryManager())
                             NameIdPool<XMLNotationDecl>(
                                                         initSize
                                                       , initSize2
                                                       , serEng.getMemoryManager()
                                                       );
        }

        serEng.registerObject(*objToLoad);

        unsigned int itemNumber = 0;
        serEng >> itemNumber;

        for (unsigned int itemIndex = 0; itemIndex < itemNumber; itemIndex++)
        {
            XMLNotationDecl*  data = new (serEng.getMemoryManager())
                                     XMLNotationDecl(serEng.getMemoryManager());
            data->serialize(serEng);
            (*objToLoad)->put(data);
        }
    }
}

XERCES_CPP_NAMESPACE_END
