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

/**
 * $Id: XSNamedMap.c 674012 2008-07-04 11:18:21Z borisk $
 */


// ---------------------------------------------------------------------------
//  Include
// ---------------------------------------------------------------------------
#if defined(XERCES_TMPLSINC)
#include <xercesc/framework/psvi/XSNamedMap.hpp>
#endif

#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/util/StringPool.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  XSNamedMap: Constructors and Destructor
// ---------------------------------------------------------------------------
template <class TVal>
XSNamedMap<TVal>::XSNamedMap(const XMLSize_t maxElems,
                             const XMLSize_t modulus,
                             XMLStringPool* uriStringPool,
                             const bool adoptElems,
                             MemoryManager* const manager)
    : fMemoryManager(manager)
    , fURIStringPool(uriStringPool)
{
    // allow one of the Vector or Hash to own the data... but not both...
    fVector = new (manager) RefVectorOf<TVal> (maxElems, false, manager);
    fHash = new (manager) RefHash2KeysTableOf<TVal> (modulus, adoptElems, manager);
}
template <class TVal> XSNamedMap<TVal>::~XSNamedMap()
{
    delete fVector;
    delete fHash;
}


/**
 * The number of <code>XSObjects</code> in the <code>XSObjectList</code>.
 * The range of valid child object indices is 0 to
 * <code>mapLength-1</code> inclusive.
 */
template <class TVal>
XMLSize_t XSNamedMap<TVal>::getLength() const
{
    return fVector->size();
}

/**
 * Returns the <code>index</code>th item in the collection. The index
 * starts at 0. If <code>index</code> is greater than or equal to the
 * number of objects in the list, this returns <code>null</code>.
 * @param index  index into the collection.
 * @return  The <code>XSObject</code> at the <code>index</code>th
 *   position in the <code>XSObjectList</code>, or <code>null</code> if
 *   that is not a valid index.
 */
template <class TVal>
TVal* XSNamedMap<TVal>::item(XMLSize_t index)
{
    if (index >= fVector->size())
    {
        return 0;
    }
    return fVector->elementAt(index);
}

template <class TVal>
const TVal* XSNamedMap<TVal>::item(XMLSize_t index) const
{
    if (index >= fVector->size())
    {
        return 0;
    }
    return fVector->elementAt(index);
}

/**
 * Retrieves a component specified by local name and namespace URI.
 * <br>applications must use the value null as the
 * <code>compNamespace</code> parameter for components whose targetNamespace property
 * is absent.
 * @param compNamespace The namespace URI of the component to retrieve.
 * @param localName The local name of the component to retrieve.
 * @return A component (of any type) with the specified local
 *   name and namespace URI, or <code>null</code> if they do not
 *   identify any node in this map.
 */
template <class TVal>
TVal *XSNamedMap<TVal>::itemByName(const XMLCh *compNamespace,
                          const XMLCh *localName)
{
    return fHash->get((void*)localName, fURIStringPool->getId(compNamespace));
}


template <class TVal>
void XSNamedMap<TVal>::addElement(TVal* const toAdd, const XMLCh* key1, const XMLCh* key2)
{
    fVector->addElement(toAdd);
    fHash->put((void*)key1, fURIStringPool->getId(key2), toAdd);
}

XERCES_CPP_NAMESPACE_END
