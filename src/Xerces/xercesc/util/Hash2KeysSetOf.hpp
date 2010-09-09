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
 * $Id: Hash2KeysSetOf.hpp 883368 2009-11-23 15:28:19Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_HASH2KEYSSETOF_HPP)
#define XERCESC_INCLUDE_GUARD_HASH2KEYSSETOF_HPP


#include <xercesc/util/Hashers.hpp>
#include <xercesc/util/IllegalArgumentException.hpp>
#include <xercesc/util/NoSuchElementException.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// This hash table is similar to Hash2KeysSetOf with an additional integer as key2

//  Forward declare the enumerator so it can be our friend.
//
template <class THasher>
class Hash2KeysSetOfEnumerator;

//
//  This should really be a nested class, but some of the compilers we
//  have to support cannot deal with that!
//
struct Hash2KeysSetBucketElem
{
    Hash2KeysSetBucketElem*              fNext;
    const void*                          fKey1;
    int                                  fKey2;
};


template <class THasher>
class Hash2KeysSetOf : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    Hash2KeysSetOf(
      const XMLSize_t modulus,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    Hash2KeysSetOf(
      const XMLSize_t modulus,
      const THasher& hasher,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    ~Hash2KeysSetOf();


    // -----------------------------------------------------------------------
    //  Element management
    // -----------------------------------------------------------------------
    bool isEmpty() const;
    bool containsKey(const void* const key1, const int key2) const;
    void removeKey(const void* const key1, const int key2);
    void removeKey(const void* const key1);
    void removeAll();

    // -----------------------------------------------------------------------
    //  Getters
    // -----------------------------------------------------------------------
    MemoryManager* getMemoryManager() const;
    XMLSize_t      getHashModulus()   const;

    // -----------------------------------------------------------------------
    //  Putters
    // -----------------------------------------------------------------------
	void put(const void* key1, int key2);
	bool putIfNotPresent(const void* key1, int key2);

private :
    // -----------------------------------------------------------------------
    //  Declare our friends
    // -----------------------------------------------------------------------
    friend class Hash2KeysSetOfEnumerator<THasher>;


private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    Hash2KeysSetOf(const Hash2KeysSetOf<THasher>&);
    Hash2KeysSetOf<THasher>& operator=(const Hash2KeysSetOf<THasher>&);

    // -----------------------------------------------------------------------
    //  Private methods
    // -----------------------------------------------------------------------
    Hash2KeysSetBucketElem* findBucketElem(const void* const key1, const int key2, XMLSize_t& hashVal);
    const Hash2KeysSetBucketElem* findBucketElem(const void* const key1, const int key2, XMLSize_t& hashVal) const;
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
    //  fCount
    //      The number of elements currently in the map
    //
    //  fHash
    //      The hasher for the key1 data type.
    // -----------------------------------------------------------------------
    MemoryManager*                      fMemoryManager;
    Hash2KeysSetBucketElem**            fBucketList;
    XMLSize_t                           fHashModulus;
    XMLSize_t                           fCount;
    Hash2KeysSetBucketElem*             fAvailable;
    THasher				                fHasher;
};



//
//  An enumerator for a value array. It derives from the basic enumerator
//  class, so that value vectors can be generically enumerated.
//
template <class THasher>
class Hash2KeysSetOfEnumerator : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    Hash2KeysSetOfEnumerator(Hash2KeysSetOf<THasher>* const toEnum
                           , const bool adopt = false
                           , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual ~Hash2KeysSetOfEnumerator();


    // -----------------------------------------------------------------------
    //  Enum interface
    // -----------------------------------------------------------------------
    bool hasMoreElements() const;
    void Reset();

    // -----------------------------------------------------------------------
    //  New interface
    // -----------------------------------------------------------------------
    void nextElementKey(const void*&, int&);
    void setPrimaryKey(const void* key);

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    Hash2KeysSetOfEnumerator(const Hash2KeysSetOfEnumerator<THasher>&);
    Hash2KeysSetOfEnumerator<THasher>& operator=(const Hash2KeysSetOfEnumerator<THasher>&);

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
    //
    //  fLockPrimaryKey
    //      Indicates that we are requested to iterate over the secondary keys
    //      associated with the given primary key
    //
    // -----------------------------------------------------------------------
    bool                                    fAdopted;
    Hash2KeysSetBucketElem*                 fCurElem;
    XMLSize_t                               fCurHash;
    Hash2KeysSetOf<THasher>*                fToEnum;
    MemoryManager* const                    fMemoryManager;
    const void*                             fLockPrimaryKey;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/Hash2KeysSetOf.c>
#endif

#endif
