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
 * $Id: RefHash3KeysIdPool.hpp 883368 2009-11-23 15:28:19Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_REFHASH3KEYSIDPOOL_HPP)
#define XERCESC_INCLUDE_GUARD_REFHASH3KEYSIDPOOL_HPP


#include <xercesc/util/Hashers.hpp>
#include <xercesc/util/IllegalArgumentException.hpp>
#include <xercesc/util/NoSuchElementException.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// This hash table is a combination of RefHash2KeyTableOf (with an additional integer as key3)
// and NameIdPool with an id as index

//  Forward declare the enumerator so it can be our friend.
//
template <class TVal, class THasher = StringHasher>
class RefHash3KeysIdPoolEnumerator;


//
//  This should really be a nested class, but some of the compilers we
//  have to support cannot deal with that!
//
template <class TVal>
struct RefHash3KeysTableBucketElem
{
    RefHash3KeysTableBucketElem(
              void* key1
              , int key2
              , int key3
              , TVal* const value
              , RefHash3KeysTableBucketElem<TVal>* next) :
		fData(value)
    , fNext(next)
    , fKey1(key1)
    , fKey2(key2)
    , fKey3(key3)
    {
    }

    RefHash3KeysTableBucketElem() {};
    ~RefHash3KeysTableBucketElem() {};

    TVal*  fData;
    RefHash3KeysTableBucketElem<TVal>*   fNext;
    void*  fKey1;
    int    fKey2;
    int    fKey3;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefHash3KeysTableBucketElem(const RefHash3KeysTableBucketElem<TVal>&);
    RefHash3KeysTableBucketElem<TVal>& operator=(const RefHash3KeysTableBucketElem<TVal>&);
};


template <class TVal, class THasher = StringHasher>
class RefHash3KeysIdPool : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    RefHash3KeysIdPool(
      const XMLSize_t modulus,
      const XMLSize_t initSize = 128,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    RefHash3KeysIdPool(
      const XMLSize_t modulus,
      const THasher& hasher,
      const XMLSize_t initSize = 128,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    RefHash3KeysIdPool(
      const XMLSize_t modulus,
      const bool adoptElems,
      const XMLSize_t initSize = 128,
      MemoryManager* const manager =  XMLPlatformUtils::fgMemoryManager);

    RefHash3KeysIdPool(
      const XMLSize_t modulus,
      const bool adoptElems,
      const THasher& hasher,
      const XMLSize_t initSize = 128,
      MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    ~RefHash3KeysIdPool();

    // -----------------------------------------------------------------------
    //  Element management
    // -----------------------------------------------------------------------
    bool isEmpty() const;
    bool containsKey(const void* const key1, const int key2, const int key3) const;
    void removeAll();


    // -----------------------------------------------------------------------
    //  Getters
    // -----------------------------------------------------------------------
    TVal* getByKey(const void* const key1, const int key2, const int key3);
    const TVal* getByKey(const void* const key1, const int key2, const int key3) const;

    TVal* getById(const unsigned int elemId);
    const TVal* getById(const unsigned int elemId) const;

    MemoryManager* getMemoryManager() const;
    XMLSize_t      getHashModulus()   const;

    // -----------------------------------------------------------------------
    //  Putters
    // -----------------------------------------------------------------------
    XMLSize_t put(void* key1, int key2, int key3, TVal* const valueToAdopt);


private :
    // -----------------------------------------------------------------------
    //  Declare our friends
    // -----------------------------------------------------------------------
    friend class RefHash3KeysIdPoolEnumerator<TVal, THasher>;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefHash3KeysIdPool(const RefHash3KeysIdPool<TVal, THasher>&);
    RefHash3KeysIdPool<TVal, THasher>& operator=(const RefHash3KeysIdPool<TVal, THasher>&);

    // -----------------------------------------------------------------------
    //  Private methods
    // -----------------------------------------------------------------------
    RefHash3KeysTableBucketElem<TVal>* findBucketElem(const void* const key1, const int key2, const int key3, XMLSize_t& hashVal);
    const RefHash3KeysTableBucketElem<TVal>* findBucketElem(const void* const key1, const int key2, const int key3, XMLSize_t& hashVal) const;
    void initialize(const XMLSize_t modulus);


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
    //      The hasher for the key1 data type.
    //
    //  fIdPtrs
    //  fIdPtrsCount
    //      This is the array of pointers to the bucket elements in order of
    //      their assigned ids. So taking id N and referencing this array
    //      gives you the element with that id. The count field indicates
    //      the current size of this list. When fIdCounter+1 reaches this
    //      value the list must be expanded.
    //
    //  fIdCounter
    //      This is used to give out unique ids to added elements. It starts
    //      at zero (which means empty), and is bumped up for each newly added
    //      element. So the first element is 1, the next is 2, etc... This
    //      means that this value is set to the top index of the fIdPtrs array.
    // -----------------------------------------------------------------------
    MemoryManager*                      fMemoryManager;
    bool                                fAdoptedElems;
    RefHash3KeysTableBucketElem<TVal>** fBucketList;
    XMLSize_t                           fHashModulus;
    TVal**                              fIdPtrs;
    XMLSize_t                           fIdPtrsCount;
    XMLSize_t                           fIdCounter;
    THasher                             fHasher;
};



//
//  An enumerator for a value array. It derives from the basic enumerator
//  class, so that value vectors can be generically enumerated.
//
template <class TVal, class THasher>
class RefHash3KeysIdPoolEnumerator : public XMLEnumerator<TVal>, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    RefHash3KeysIdPoolEnumerator(RefHash3KeysIdPool<TVal, THasher>* const toEnum
        , const bool adopt = false
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    virtual ~RefHash3KeysIdPoolEnumerator();

    RefHash3KeysIdPoolEnumerator(const RefHash3KeysIdPoolEnumerator<TVal, THasher>&);
    // -----------------------------------------------------------------------
    //  Enum interface
    // -----------------------------------------------------------------------
    bool hasMoreElements() const;
    TVal& nextElement();
    void Reset();
    XMLSize_t size() const;

    // -----------------------------------------------------------------------
    //  New interface
    // -----------------------------------------------------------------------
    void resetKey();
    void nextElementKey(void*&, int&, int&);
    bool hasMoreKeys()   const;

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    RefHash3KeysIdPoolEnumerator<TVal, THasher>&
    operator=(const RefHash3KeysIdPoolEnumerator<TVal, THasher>&);

    // -----------------------------------------------------------------------
    //  Private methods
    // -----------------------------------------------------------------------
    void findNext();

    // -----------------------------------------------------------------------
    //  Data Members
    //  fAdoptedElems
    //      Indicates whether the values added are adopted or just referenced.
    //      If adopted, then they are deleted when they are removed from the
    //      hash table
    //
    //  fCurIndex
    //      This is the current index into the pool's id mapping array. This
    //      is now we enumerate it.
    //
    //  fToEnum
    //      The name id pool that is being enumerated.
    // -----------------------------------------------------------------------
    bool                                fAdoptedElems;
    XMLSize_t                           fCurIndex;
    RefHash3KeysIdPool<TVal, THasher>*  fToEnum;
    RefHash3KeysTableBucketElem<TVal>*  fCurElem;
    XMLSize_t                           fCurHash;
    MemoryManager* const                fMemoryManager;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/RefHash3KeysIdPool.c>
#endif

#endif
