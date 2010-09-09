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
 * $Id: RefHash2KeysTableOf.hpp 883368 2009-11-23 15:28:19Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_REFHASH2KEYSTABLEOF_HPP)
#define XERCESC_INCLUDE_GUARD_REFHASH2KEYSTABLEOF_HPP


#include <xercesc/util/Hashers.hpp>
#include <xercesc/util/IllegalArgumentException.hpp>
#include <xercesc/util/NoSuchElementException.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// This hash table is similar to RefHashTableOf with an additional integer as key2

//  Forward declare the enumerator so it can be our friend.
//
template <class TVal, class THasher = StringHasher>
class RefHash2KeysTableOfEnumerator;

//
//  This should really be a nested class, but some of the compilers we
//  have to support cannot deal with that!
//
template <class TVal>
struct RefHash2KeysTableBucketElem
{
    RefHash2KeysTableBucketElem(void* key1, int key2, TVal* const value, RefHash2KeysTableBucketElem<TVal>* next)
		: fData(value), fNext(next), fKey1(key1), fKey2(key2)
        {
        }
    ~RefHash2KeysTableBucketElem() {};

    TVal*                                fData;
    RefHash2KeysTableBucketElem<TVal>*   fNext;
    void*                                fKey1;
    int                                  fKey2;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefHash2KeysTableBucketElem(const RefHash2KeysTableBucketElem<TVal>&);
    RefHash2KeysTableBucketElem<TVal>& operator=(const RefHash2KeysTableBucketElem<TVal>&);
};


template <class TVal, class THasher = StringHasher>
class RefHash2KeysTableOf : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    RefHash2KeysTableOf(
      const XMLSize_t modulus,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    RefHash2KeysTableOf(
      const XMLSize_t modulus,
      const THasher& hasher,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    RefHash2KeysTableOf(
      const XMLSize_t modulus,
      const bool adoptElems,
      MemoryManager* const manager =  XMLPlatformUtils::fgMemoryManager);

    RefHash2KeysTableOf(
      const XMLSize_t modulus,
      const bool adoptElems,
      const THasher& hasher,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    ~RefHash2KeysTableOf();


    // -----------------------------------------------------------------------
    //  Element management
    // -----------------------------------------------------------------------
    bool isEmpty() const;
    bool containsKey(const void* const key1, const int key2) const;
    void removeKey(const void* const key1, const int key2);
    void removeKey(const void* const key1);
    void removeAll();
    void transferElement(const void* const key1, void* key2);

    // -----------------------------------------------------------------------
    //  Getters
    // -----------------------------------------------------------------------
    TVal* get(const void* const key1, const int key2);
    const TVal* get(const void* const key1, const int key2) const;

    MemoryManager* getMemoryManager() const;
    XMLSize_t      getHashModulus()   const;

    // -----------------------------------------------------------------------
    //  Putters
    // -----------------------------------------------------------------------
	void put(void* key1, int key2, TVal* const valueToAdopt);

private :
    // -----------------------------------------------------------------------
    //  Declare our friends
    // -----------------------------------------------------------------------
    friend class RefHash2KeysTableOfEnumerator<TVal, THasher>;


private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefHash2KeysTableOf(const RefHash2KeysTableOf<TVal, THasher>&);
    RefHash2KeysTableOf<TVal>& operator=(const RefHash2KeysTableOf<TVal, THasher>&);

    // -----------------------------------------------------------------------
    //  Private methods
    // -----------------------------------------------------------------------
    RefHash2KeysTableBucketElem<TVal>* findBucketElem(const void* const key1, const int key2, XMLSize_t& hashVal);
    const RefHash2KeysTableBucketElem<TVal>* findBucketElem(const void* const key1, const int key2, XMLSize_t& hashVal) const;
    void initialize(const XMLSize_t modulus);
    void rehash();


    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fAdoptedElems
    //      Indicates whether the values added are adopted or just referenced.
    //      If adopted, then they are deleted when they are removed from the
    //      hash table.
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
    bool                                fAdoptedElems;
    RefHash2KeysTableBucketElem<TVal>** fBucketList;
    XMLSize_t                           fHashModulus;
    XMLSize_t                           fCount;
    THasher                             fHasher;
};



//
//  An enumerator for a value array. It derives from the basic enumerator
//  class, so that value vectors can be generically enumerated.
//
template <class TVal, class THasher>
class RefHash2KeysTableOfEnumerator : public XMLEnumerator<TVal>, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    RefHash2KeysTableOfEnumerator(RefHash2KeysTableOf<TVal, THasher>* const toEnum
                                  , const bool adopt = false
                                  , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual ~RefHash2KeysTableOfEnumerator();


    // -----------------------------------------------------------------------
    //  Enum interface
    // -----------------------------------------------------------------------
    bool hasMoreElements() const;
    TVal& nextElement();
    void Reset();

    // -----------------------------------------------------------------------
    //  New interface
    // -----------------------------------------------------------------------
    void nextElementKey(void*&, int&);
    void setPrimaryKey(const void* key);

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefHash2KeysTableOfEnumerator(const RefHash2KeysTableOfEnumerator<TVal, THasher>&);
    RefHash2KeysTableOfEnumerator<TVal, THasher>& operator=(const RefHash2KeysTableOfEnumerator<TVal, THasher>&);

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
    RefHash2KeysTableBucketElem<TVal>*      fCurElem;
    XMLSize_t                               fCurHash;
    RefHash2KeysTableOf<TVal, THasher>*     fToEnum;
    MemoryManager* const                    fMemoryManager;
    const void*                             fLockPrimaryKey;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/RefHash2KeysTableOf.c>
#endif

#endif
