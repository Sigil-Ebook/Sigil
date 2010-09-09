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
 * $Id: ValueHashTableOf.hpp 679340 2008-07-24 10:28:29Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_VALUEHASHTABLEOF_HPP)
#define XERCESC_INCLUDE_GUARD_VALUEHASHTABLEOF_HPP


#include <xercesc/util/Hashers.hpp>
#include <xercesc/util/IllegalArgumentException.hpp>
#include <xercesc/util/NoSuchElementException.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//  Forward declare the enumerator so it can be our friend.
//
template <class TVal, class THasher = StringHasher>
class ValueHashTableOfEnumerator;


//
//  This should really be a nested class, but some of the compilers we
//  have to support cannot deal with that!
//
template <class TVal>
struct ValueHashTableBucketElem
{
    ValueHashTableBucketElem(void* key, const TVal& value, ValueHashTableBucketElem<TVal>* next)
		: fData(value), fNext(next), fKey(key)
        {
        }
    ValueHashTableBucketElem(){};
    ~ValueHashTableBucketElem(){};

    TVal                            fData;
    ValueHashTableBucketElem<TVal>* fNext;
    void*                           fKey;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ValueHashTableBucketElem(const ValueHashTableBucketElem<TVal>&);
    ValueHashTableBucketElem<TVal>& operator=(const ValueHashTableBucketElem<TVal>&);
};


template <class TVal, class THasher = StringHasher>
class ValueHashTableOf : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ValueHashTableOf(
      const XMLSize_t modulus,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    ValueHashTableOf(
      const XMLSize_t modulus,
      const THasher& hasher,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    ~ValueHashTableOf();


    // -----------------------------------------------------------------------
    //  Element management
    // -----------------------------------------------------------------------
    bool isEmpty() const;
    bool containsKey(const void* const key) const;
    void removeKey(const void* const key);
    void removeAll();


    // -----------------------------------------------------------------------
    //  Getters
    // -----------------------------------------------------------------------
    TVal& get(const void* const key, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    const TVal& get(const void* const key) const;


    // -----------------------------------------------------------------------
    //  Putters
    // -----------------------------------------------------------------------
    void put(void* key, const TVal& valueToAdopt);


private :
    // -----------------------------------------------------------------------
    //  Declare our friends
    // -----------------------------------------------------------------------
    friend class ValueHashTableOfEnumerator<TVal, THasher>;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ValueHashTableOf(const ValueHashTableOf<TVal, THasher>&);
    ValueHashTableOf<TVal, THasher>& operator=(const ValueHashTableOf<TVal, THasher>&);

    // -----------------------------------------------------------------------
    //  Private methods
    // -----------------------------------------------------------------------
    ValueHashTableBucketElem<TVal>* findBucketElem(const void* const key, XMLSize_t& hashVal);
    const ValueHashTableBucketElem<TVal>* findBucketElem(const void* const key, XMLSize_t& hashVal) const;
    void removeBucketElem(const void* const key, XMLSize_t& hashVal);
    void initialize(const XMLSize_t modulus);
    void rehash();


    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fBucketList
    //      This is the array that contains the heads of all of the list
    //      buckets, one for each possible hash value.
    //
    //  fHashModulus
    //      The modulus used for this hash table, to hash the keys. This is
    //      also the number of elements in the bucket list.
    //
    //  fHash
    //      The hasher for the key data type.
    // -----------------------------------------------------------------------
    MemoryManager*                   fMemoryManager;
    ValueHashTableBucketElem<TVal>** fBucketList;
    XMLSize_t                        fHashModulus;
    XMLSize_t                        fInitialModulus;
    XMLSize_t                        fCount;
    THasher                          fHasher;
};



//
//  An enumerator for a value array. It derives from the basic enumerator
//  class, so that value vectors can be generically enumerated.
//
template <class TVal, class THasher>
class ValueHashTableOfEnumerator : public XMLEnumerator<TVal>, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ValueHashTableOfEnumerator(ValueHashTableOf<TVal, THasher>* const toEnum
                               , const bool adopt = false
                               , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual ~ValueHashTableOfEnumerator();


    // -----------------------------------------------------------------------
    //  Enum interface
    // -----------------------------------------------------------------------
    bool hasMoreElements() const;
    TVal& nextElement();
    void Reset();

    // -----------------------------------------------------------------------
    //  New interface specific for key used in ValueHashable
    // -----------------------------------------------------------------------
    void* nextElementKey();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ValueHashTableOfEnumerator(const ValueHashTableOfEnumerator<TVal, THasher>&);
    ValueHashTableOfEnumerator<TVal, THasher>& operator=(const ValueHashTableOfEnumerator<TVal, THasher>&);

    // -----------------------------------------------------------------------
    //  Private methods
    // -----------------------------------------------------------------------
    void findNext();


    // -----------------------------------------------------------------------
    //  Data Members
    //
    //  fAdopted
    //      Indicates whether we have adopted the passed vector. If so then
    //      we delete the vector when we are destroyed.
    //
    //  fCurElem
    //      This is the current bucket bucket element that we are on.
    //
    //  fCurHash
    //      The is the current hash buck that we are working on. Once we hit
    //      the end of the bucket that fCurElem is in, then we have to start
    //      working this one up to the next non-empty bucket.
    //
    //  fToEnum
    //      The value array being enumerated.
    // -----------------------------------------------------------------------
    bool                             fAdopted;
    ValueHashTableBucketElem<TVal>*  fCurElem;
    XMLSize_t                        fCurHash;
    ValueHashTableOf<TVal, THasher>* fToEnum;
    MemoryManager* const             fMemoryManager;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/ValueHashTableOf.c>
#endif

#endif
