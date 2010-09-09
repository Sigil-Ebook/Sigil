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
 * $Id: StringPool.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_STRINGPOOL_HPP)
#define XERCESC_INCLUDE_GUARD_STRINGPOOL_HPP

#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  This class implements a string pool, in which strings can be added and
//  given a unique id by which they can be referred. It has to provide fast
//  access both mapping from a string to its id and mapping from an id to
//  its string. This requires that it provide two separate data structures.
//  The map one is a hash table for quick storage and look up by name. The
//  other is an array ordered by unique id which maps to the element in the
//  hash table.
//
//  This works because strings cannot be removed from the pool once added,
//  other than flushing it completely, and because ids are assigned
//  sequentially from 1.
//
class XMLUTIL_EXPORT XMLStringPool : public XSerializable, public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XMLStringPool
    (
          const unsigned int   modulus = 109
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    virtual ~XMLStringPool();


    // -----------------------------------------------------------------------
    //  Pool management methods
    // -----------------------------------------------------------------------
    virtual unsigned int addOrFind(const XMLCh* const newString);
    virtual bool exists(const XMLCh* const newString) const;
    virtual bool exists(const unsigned int id) const;
    virtual void flushAll();
    virtual unsigned int getId(const XMLCh* const toFind) const;
    virtual const XMLCh* getValueForId(const unsigned int id) const;
    virtual unsigned int getStringCount() const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(XMLStringPool)

    XMLStringPool(MemoryManager* const manager);

private :
    // -----------------------------------------------------------------------
    //  Private data types
    // -----------------------------------------------------------------------
    struct PoolElem
    {
        unsigned int  fId;
        XMLCh*        fString;
    };

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLStringPool(const XMLStringPool&);
    XMLStringPool& operator=(const XMLStringPool&);


    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    unsigned int addNewEntry(const XMLCh* const newString);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fIdMap
    //      This is an array of pointers to the pool elements. It is ordered
    //      by unique id, so using an id to index it gives instant access to
    //      the string of that id. This is grown as required.
    //
    //  fHashTable
    //      This is the hash table used to store and quickly access the
    //      strings.
    //
    //  fMapCapacity
    //      The current capacity of the id map. When the current id hits this
    //      value the map must must be expanded.
    //
    // -----------------------------------------------------------------------
    MemoryManager*              fMemoryManager;
    PoolElem**                  fIdMap;
    RefHashTableOf<PoolElem>*   fHashTable;
    unsigned int                fMapCapacity;

protected:
    // protected data members
    //  fCurId
    //      This is the counter used to assign unique ids. It is just bumped
    //      up one for each new string added.
    unsigned int                fCurId;
};


// Provide inline versions of some of the simple functions to improve performance.
inline unsigned int XMLStringPool::addOrFind(const XMLCh* const newString)
{
    PoolElem* elemToFind = fHashTable->get(newString);
    if (elemToFind)
        return elemToFind->fId;

    return addNewEntry(newString);
}

inline unsigned int XMLStringPool::getId(const XMLCh* const toFind) const
{
    PoolElem* elemToFind = fHashTable->get(toFind);
    if (elemToFind)
        return elemToFind->fId;

    // Not found, so return zero, which is never a legal id
    return 0;
}

inline bool XMLStringPool::exists(const XMLCh* const newString) const
{
    return fHashTable->containsKey(newString);
}

inline bool XMLStringPool::exists(const unsigned int id) const
{
    return (id > 0 && (id < fCurId));
}

inline const XMLCh* XMLStringPool::getValueForId(const unsigned int id) const
{
    if (!id || (id >= fCurId))
        ThrowXMLwithMemMgr(IllegalArgumentException, XMLExcepts::StrPool_IllegalId, fMemoryManager);

    // Just index the id map and return that element's string
    return fIdMap[id]->fString;
}

inline unsigned int XMLStringPool::getStringCount() const
{
    return fCurId-1;
}

XERCES_CPP_NAMESPACE_END

#endif
