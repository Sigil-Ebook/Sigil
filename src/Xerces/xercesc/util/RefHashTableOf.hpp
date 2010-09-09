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
 * $Id: RefHashTableOf.hpp 679340 2008-07-24 10:28:29Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_REFHASHTABLEOF_HPP)
#define XERCESC_INCLUDE_GUARD_REFHASHTABLEOF_HPP

#include <xercesc/util/Hashers.hpp>
#include <xercesc/util/IllegalArgumentException.hpp>
#include <xercesc/util/NoSuchElementException.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/MemoryManager.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//  Forward declare the enumerator so it can be our friend.
//
template <class TVal, class THasher = StringHasher>
class RefHashTableOfEnumerator;

//
//  This should really be a nested class, but some of the compilers we
//  have to support cannot deal with that!
//
template <class TVal>
struct RefHashTableBucketElem
{
  RefHashTableBucketElem(void* key, TVal* const value, RefHashTableBucketElem<TVal>* next)
      : fData(value), fNext(next), fKey(key)
  {
  }

  RefHashTableBucketElem(){};
  ~RefHashTableBucketElem(){};

  TVal*                           fData;
  RefHashTableBucketElem<TVal>*   fNext;
  void*                           fKey;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
  RefHashTableBucketElem(const RefHashTableBucketElem<TVal>&);
  RefHashTableBucketElem<TVal>& operator=(const RefHashTableBucketElem<TVal>&);
};


template <class TVal, class THasher = StringHasher>
class RefHashTableOf : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    RefHashTableOf(
      const XMLSize_t modulus,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    RefHashTableOf(
      const XMLSize_t modulus,
      const THasher& hasher,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    RefHashTableOf(
      const XMLSize_t modulus,
      const bool adoptElems,
      MemoryManager* const manager =  XMLPlatformUtils::fgMemoryManager);

    RefHashTableOf(
      const XMLSize_t modulus,
      const bool adoptElems,
      const THasher& hasher,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    ~RefHashTableOf();


    // -----------------------------------------------------------------------
    //  Element management
    // -----------------------------------------------------------------------
    bool isEmpty() const;
    bool containsKey(const void* const key) const;
    void removeKey(const void* const key);
    void removeAll();
    void cleanup();
    void reinitialize(const THasher& hasher);
    void transferElement(const void* const key1, void* key2);
    TVal* orphanKey(const void* const key);

    // -----------------------------------------------------------------------
    //  Getters
    // -----------------------------------------------------------------------
    TVal* get(const void* const key);
    const TVal* get(const void* const key) const;
    MemoryManager* getMemoryManager() const;
    XMLSize_t      getHashModulus()   const;
    XMLSize_t      getCount() const;

    // -----------------------------------------------------------------------
    //  Setters
    // -----------------------------------------------------------------------
    void setAdoptElements(const bool aValue);


    // -----------------------------------------------------------------------
    //  Putters
    // -----------------------------------------------------------------------
    void put(void* key, TVal* const valueToAdopt);


private :
    // -----------------------------------------------------------------------
    //  Declare our friends
    // -----------------------------------------------------------------------
    friend class RefHashTableOfEnumerator<TVal, THasher>;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefHashTableOf(const RefHashTableOf<TVal, THasher>&);
    RefHashTableOf<TVal, THasher>& operator=(const RefHashTableOf<TVal, THasher>&);

    // -----------------------------------------------------------------------
    //  Private methods
    // -----------------------------------------------------------------------
    RefHashTableBucketElem<TVal>* findBucketElem(const void* const key, XMLSize_t& hashVal);
    const RefHashTableBucketElem<TVal>* findBucketElem(const void* const key, XMLSize_t& hashVal) const;
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
    //  fHash
    //      The hasher for the key data type.
    // -----------------------------------------------------------------------
    MemoryManager*                 fMemoryManager;
    bool                           fAdoptedElems;
    RefHashTableBucketElem<TVal>** fBucketList;
    XMLSize_t                      fHashModulus;
    XMLSize_t                      fInitialModulus;
    XMLSize_t                      fCount;
    THasher                        fHasher;
};



//
//  An enumerator for a value array. It derives from the basic enumerator
//  class, so that value vectors can be generically enumerated.
//
template <class TVal, class THasher>
class RefHashTableOfEnumerator : public XMLEnumerator<TVal>, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    RefHashTableOfEnumerator(RefHashTableOf<TVal, THasher>* const toEnum
        , const bool adopt = false
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual ~RefHashTableOfEnumerator();

    RefHashTableOfEnumerator(const RefHashTableOfEnumerator<TVal, THasher>&);
    // -----------------------------------------------------------------------
    //  Enum interface
    // -----------------------------------------------------------------------
    bool hasMoreElements() const;
    TVal& nextElement();
    void Reset();

    // -----------------------------------------------------------------------
    //  New interface specific for key used in RefHashable
    // -----------------------------------------------------------------------
    void* nextElementKey();

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefHashTableOfEnumerator<TVal, THasher>&
    operator=(const RefHashTableOfEnumerator<TVal, THasher>&);

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
    //      The current hash buck that we are working on. Once we hit the
    //      end of the bucket that fCurElem is in, then we have to start
    //      working this one up to the next non-empty bucket.
    //
    //  fToEnum
    //      The value array being enumerated.
    // -----------------------------------------------------------------------
    bool                                  fAdopted;
    RefHashTableBucketElem<TVal>*         fCurElem;
    XMLSize_t                             fCurHash;
    RefHashTableOf<TVal, THasher>*        fToEnum;
    MemoryManager* const                  fMemoryManager;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/RefHashTableOf.c>
#endif

#endif
