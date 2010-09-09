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
 * $Id: SchemaAttDefList.cpp 679359 2008-07-24 11:15:19Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/schema/SchemaAttDefList.hpp>

#include <xercesc/internal/XTemplateSerializer.hpp>
#include <xercesc/util/ArrayIndexOutOfBoundsException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  SchemaAttDefList: Constructors and Destructor
// ---------------------------------------------------------------------------
SchemaAttDefList::SchemaAttDefList(RefHash2KeysTableOf<SchemaAttDef>* const listToUse, MemoryManager* const manager)
: XMLAttDefList(manager)
,fEnum(0)
,fList(listToUse)
,fArray(0)
,fSize(0)
,fCount(0)
{
    fEnum = new (getMemoryManager()) RefHash2KeysTableOfEnumerator<SchemaAttDef>(listToUse, false, getMemoryManager());
    fArray = (SchemaAttDef **)((getMemoryManager())->allocate( sizeof(SchemaAttDef*) << 1));
    fSize = 2;
}

SchemaAttDefList::~SchemaAttDefList()
{
    delete fEnum;
    (getMemoryManager())->deallocate(fArray);
}


// ---------------------------------------------------------------------------
//  SchemaAttDefList: Implementation of the virtual interface
// ---------------------------------------------------------------------------

bool SchemaAttDefList::isEmpty() const
{
    return fList->isEmpty();
}


XMLAttDef* SchemaAttDefList::findAttDef(const  unsigned int uriID
                                    , const XMLCh* const    attName)
{
   const int colonInd = XMLString::indexOf(attName, chColon);

   // An index of 0 is really an error, but the QName class doesn't check for
   // that case either...
   const XMLCh* const localPart = colonInd >= 0 ? attName + colonInd + 1 : attName;

   return fList->get((void*)localPart, uriID);
}


const XMLAttDef*
SchemaAttDefList::findAttDef( const   unsigned int   uriID
                            , const XMLCh* const    attName) const
{
   const int colonInd = XMLString::indexOf(attName, chColon);

   // An index of 0 is really an error, but the QName class doesn't check for
   // that case either...
   const XMLCh* const localPart = colonInd >= 0 ? attName + colonInd + 1 : attName;

   return fList->get((void*)localPart, uriID);
}


XMLAttDef* SchemaAttDefList::findAttDef(   const   XMLCh* const
                                        , const XMLCh* const)
{
   //need numeric URI id to locate the attribute, that's how it was stored
   ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Pool_InvalidId, getMemoryManager());
   return 0;
}


const XMLAttDef*
SchemaAttDefList::findAttDef( const   XMLCh* const
                            , const XMLCh* const) const
{
   //need numeric URI id to locate the attribute, that's how it was stored
   ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::Pool_InvalidId, getMemoryManager());
   return 0;
}

/**
 * return total number of attributes in this list
 */
XMLSize_t SchemaAttDefList::getAttDefCount() const
{
    return fCount;
}

/**
 * return attribute at the index-th position in the list.
 */
XMLAttDef &SchemaAttDefList::getAttDef(XMLSize_t index)
{
    if(index >= fCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::AttrList_BadIndex, getMemoryManager());
    return *(fArray[index]);
}

/**
 * return attribute at the index-th position in the list.
 */
const XMLAttDef &SchemaAttDefList::getAttDef(XMLSize_t index) const
{
    if(index >= fCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::AttrList_BadIndex, getMemoryManager());
    return *(fArray[index]);
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(SchemaAttDefList)

void SchemaAttDefList::serialize(XSerializeEngine& serEng)
{

    XMLAttDefList::serialize(serEng);

    if (serEng.isStoring())
    {
        /***
         *
         * Serialize RefHash2KeysTableOf<SchemaAttDef>
         *
         ***/
        XTemplateSerializer::storeObject(fList, serEng);
        serEng.writeSize (fCount);

        // do not serialize fEnum
    }
    else
    {
        /***
         *
         * Deserialize RefHash2KeysTableOf<SchemaAttDef>
         *
         ***/
        XTemplateSerializer::loadObject(&fList, 29, true, serEng);

        // assume empty so we can size fArray just right
        serEng.readSize (fSize);
        if (!fEnum && fList)
        {
            fEnum = new (getMemoryManager()) RefHash2KeysTableOfEnumerator<SchemaAttDef>(fList, false, getMemoryManager());
        }
        if(fSize)
        {
            (getMemoryManager())->deallocate(fArray);
            fArray = (SchemaAttDef **)((getMemoryManager())->allocate( sizeof(SchemaAttDef*) * fSize));
            fCount = 0;
            while(fEnum->hasMoreElements())
            {
                fArray[fCount++] = &fEnum->nextElement();
            }
        }
    }

}

SchemaAttDefList::SchemaAttDefList(MemoryManager* const manager)
: XMLAttDefList(manager)
,fEnum(0)
,fList(0)
,fArray(0)
,fSize(0)
,fCount(0)
{
}

XERCES_CPP_NAMESPACE_END
