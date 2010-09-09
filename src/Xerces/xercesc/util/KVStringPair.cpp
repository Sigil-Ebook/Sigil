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
 * $Id: KVStringPair.cpp 554580 2007-07-09 09:09:51Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/KVStringPair.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  KVStringPair: Constructors and Destructor
// ---------------------------------------------------------------------------
KVStringPair::KVStringPair(MemoryManager* const manager)
:fKeyAllocSize(0)
,fValueAllocSize(0)
,fKey(0)
,fValue(0)
,fMemoryManager(manager)
{
}

KVStringPair::KVStringPair(const XMLCh* const key,
                           const XMLCh* const value,
                           MemoryManager* const manager)
:fKeyAllocSize(0)
,fValueAllocSize(0)
,fKey(0)
,fValue(0)
,fMemoryManager(manager)
{
   set(key, value);
}

KVStringPair::KVStringPair(const XMLCh* const key,
                           const XMLCh* const value,
                           const XMLSize_t    valueLength,
                           MemoryManager* const manager)
:fKeyAllocSize(0)
,fValueAllocSize(0)
,fKey(0)
,fValue(0)
,fMemoryManager(manager)
{
    setKey(key);
    setValue(value, valueLength);
}

KVStringPair::KVStringPair(const XMLCh* const key,
                           const XMLSize_t    keyLength,
                           const XMLCh* const value,
                           const XMLSize_t    valueLength,
                           MemoryManager* const manager)
:fKeyAllocSize(0)
,fValueAllocSize(0)
,fKey(0)
,fValue(0)
,fMemoryManager(manager)
{    
    setKey(key, keyLength);
    setValue(value, valueLength);
}

KVStringPair::KVStringPair(const KVStringPair& toCopy)
:XSerializable(toCopy)
,XMemory(toCopy)
,fKeyAllocSize(0)
,fValueAllocSize(0)
,fKey(0)
,fValue(0)
,fMemoryManager(toCopy.fMemoryManager)
{
   set(toCopy.fKey, toCopy.fValue);
}

KVStringPair::~KVStringPair()
{
    fMemoryManager->deallocate(fKey); //delete [] fKey;
    fMemoryManager->deallocate(fValue); //delete [] fValue;
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(KVStringPair)

void KVStringPair::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {

        serEng.writeString(fKey,   fKeyAllocSize,   XSerializeEngine::toWriteBufferLen);
        serEng.writeString(fValue, fValueAllocSize, XSerializeEngine::toWriteBufferLen);
    }
    else
    {
        XMLSize_t dataLen = 0;
        serEng.readString(fKey,   fKeyAllocSize,   dataLen, XSerializeEngine::toReadBufferLen);
        serEng.readString(fValue, fValueAllocSize, dataLen, XSerializeEngine::toReadBufferLen);
    }

}

XERCES_CPP_NAMESPACE_END
