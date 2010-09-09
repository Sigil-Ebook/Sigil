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
 * $Id: KVStringPair.hpp 554580 2007-07-09 09:09:51Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_KVSTRINGPAIR_HPP)
#define XERCESC_INCLUDE_GUARD_KVSTRINGPAIR_HPP

#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  This class provides a commonly used data structure, which is that of
//  a pair of strings which represent a 'key=value' type mapping. It works
//  only in terms of XMLCh type raw strings.
//
class XMLUTIL_EXPORT KVStringPair : public XSerializable, public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    KVStringPair(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    KVStringPair
    (
        const XMLCh* const key
        , const XMLCh* const value
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    KVStringPair
    (
        const XMLCh* const key
        , const XMLCh* const value
        , const XMLSize_t    valueLength
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    KVStringPair
    (
        const XMLCh* const key
        , const XMLSize_t    keyLength
        , const XMLCh* const value
        , const XMLSize_t    valueLength
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    KVStringPair(const KVStringPair& toCopy);
    ~KVStringPair();


    // -----------------------------------------------------------------------
    //  Getters
    //
    //  We support the
    // -----------------------------------------------------------------------
    const XMLCh* getKey() const;
    XMLCh* getKey();
    const XMLCh* getValue() const;
    XMLCh* getValue();


    // -----------------------------------------------------------------------
    //  Setters
    // -----------------------------------------------------------------------
    void setKey(const XMLCh* const newKey);
    void setValue(const XMLCh* const newValue);
    void setKey
    (
        const   XMLCh* const newKey
        , const XMLSize_t    newKeyLength
    );
    void setValue
    (
        const   XMLCh* const newValue
        , const XMLSize_t    newValueLength
    );
    void set
    (
        const   XMLCh* const newKey
        , const XMLCh* const newValue
    );
    void set
    (
        const     XMLCh* const newKey
        , const   XMLSize_t    newKeyLength
        , const   XMLCh* const newValue
        , const   XMLSize_t    newValueLength
    );

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(KVStringPair)

private :
    // unimplemented:
       
    KVStringPair& operator=(const KVStringPair&);
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fKey
    //      The string that represents the key field of this object.
    //
    //  fKeyAllocSize
    //      The amount of memory allocated for fKey.
    //
    //  fValue
    //      The string that represents the value of this pair object.
    //
    //  fValueAllocSize
    //      The amount of memory allocated for fValue.
    //
    // -----------------------------------------------------------------------
    XMLSize_t      fKeyAllocSize;
    XMLSize_t      fValueAllocSize;
    XMLCh*         fKey;
    XMLCh*         fValue;
    MemoryManager* fMemoryManager;
};

// ---------------------------------------------------------------------------
//  KVStringPair: Getters
// ---------------------------------------------------------------------------
inline const XMLCh* KVStringPair::getKey() const
{
    return fKey;
}

inline XMLCh* KVStringPair::getKey()
{
    return fKey;
}

inline const XMLCh* KVStringPair::getValue() const
{
    return fValue;
}

inline XMLCh* KVStringPair::getValue()
{
    return fValue;
}

// ---------------------------------------------------------------------------
//  KVStringPair: Setters
// ---------------------------------------------------------------------------
inline void KVStringPair::setKey(const XMLCh* const newKey)
{
    setKey(newKey, XMLString::stringLen(newKey));
}

inline void KVStringPair::setValue(const XMLCh* const newValue)
{
    setValue(newValue, XMLString::stringLen(newValue));
}

inline void KVStringPair::setKey(  const XMLCh* const newKey
                                 , const XMLSize_t    newKeyLength)
{
    if (newKeyLength >= fKeyAllocSize)
    {
        fMemoryManager->deallocate(fKey); //delete [] fKey;
        fKey = 0;
        fKeyAllocSize = newKeyLength + 1;
        fKey = (XMLCh*) fMemoryManager->allocate(fKeyAllocSize * sizeof(XMLCh)); //new XMLCh[fKeyAllocSize];
    }

    memcpy(fKey, newKey, (newKeyLength+1) * sizeof(XMLCh)); // len+1 because of the 0 at the end
}

inline void KVStringPair::setValue(  const XMLCh* const newValue
                                   , const XMLSize_t    newValueLength)
{
    if (newValueLength >= fValueAllocSize)
    {
        fMemoryManager->deallocate(fValue); //delete [] fValue;
        fValue = 0;
        fValueAllocSize = newValueLength + 1;
        fValue = (XMLCh*) fMemoryManager->allocate(fValueAllocSize * sizeof(XMLCh)); //new XMLCh[fValueAllocSize];
    }

    memcpy(fValue, newValue, (newValueLength+1) * sizeof(XMLCh)); // len+1 because of the 0 at the end
}

inline void KVStringPair::set(  const   XMLCh* const    newKey
                              , const   XMLCh* const    newValue)
{
    setKey(newKey, XMLString::stringLen(newKey));
    setValue(newValue, XMLString::stringLen(newValue));
}

inline void KVStringPair::set(  const   XMLCh* const newKey
                              , const   XMLSize_t    newKeyLength
                              , const   XMLCh* const newValue
                              , const   XMLSize_t    newValueLength)
{
    setKey(newKey, newKeyLength);
    setValue(newValue, newValueLength);
}


XERCES_CPP_NAMESPACE_END

#endif
