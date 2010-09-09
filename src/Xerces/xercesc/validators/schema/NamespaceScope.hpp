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
 * $Id: NamespaceScope.hpp 729944 2008-12-29 17:03:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_NAMESPACESCOPE_HPP)
#define XERCESC_INCLUDE_GUARD_NAMESPACESCOPE_HPP

#include <xercesc/util/StringPool.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// Define a pure interface to allow XercesXPath to work on both NamespaceScope and DOMXPathNSResolver
class VALIDATORS_EXPORT XercesNamespaceResolver
{
public:
    virtual unsigned int getNamespaceForPrefix(const XMLCh* const prefix) const = 0;
};

//
// NamespaceScope provides a data structure for mapping namespace prefixes
// to their URI's. The mapping accurately reflects the scoping of namespaces
// at a particular instant in time.
//

class VALIDATORS_EXPORT NamespaceScope : public XMemory,
                                         public XercesNamespaceResolver
{
public :
    // -----------------------------------------------------------------------
    //  Class specific data types
    //
    //  These really should be private, but some of the compilers we have to
    //  support are too dumb to deal with that.
    //
    //  PrefMapElem
    //      fURIId is the id of the URI from the validator's URI map. The
    //      fPrefId is the id of the prefix from our own prefix pool. The
    //      namespace stack consists of these elements.
    //
    //  StackElem
    //      The fMapCapacity is how large fMap has grown so far. fMapCount
    //      is how many of them are valid right now.
    // -----------------------------------------------------------------------
    struct PrefMapElem : public XMemory
    {
        unsigned int        fPrefId;
        unsigned int        fURIId;
    };

    struct StackElem : public XMemory
    {
        PrefMapElem*        fMap;
        unsigned int        fMapCapacity;
        unsigned int        fMapCount;
    };


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    NamespaceScope(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    NamespaceScope(const NamespaceScope* const initialize, MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~NamespaceScope();


    // -----------------------------------------------------------------------
    //  Stack access
    // -----------------------------------------------------------------------
    unsigned int increaseDepth();
    unsigned int decreaseDepth();

    // -----------------------------------------------------------------------
    //  Prefix map methods
    // -----------------------------------------------------------------------
    void addPrefix(const XMLCh* const prefixToAdd,
                   const unsigned int uriId);

    virtual unsigned int getNamespaceForPrefix(const XMLCh* const prefixToMap) const;


    // -----------------------------------------------------------------------
    //  Miscellaneous methods
    // -----------------------------------------------------------------------
    bool isEmpty() const;
    void reset(const unsigned int emptyId);
    unsigned int getEmptyNamespaceId() const;


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    NamespaceScope(const NamespaceScope&);
    NamespaceScope& operator=(const NamespaceScope&);


    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void expandMap(StackElem* const toExpand);
    void expandStack();


    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fEmptyNamespaceId
    //      This is the special URI id for the "" namespace, which is magic
    //      because of the xmlns="" operation.
    //
    //  fPrefixPool
    //      This is the prefix pool where prefixes are hashed and given unique
    //      ids. These ids are used to track prefixes in the element stack.
    //
    //  fStack
    //  fStackCapacity
    //  fStackTop
    //      This the stack array. Its an array of pointers to StackElem
    //      structures. The capacity is the current high water mark of the
    //      stack. The top is the current top of stack (i.e. the part of it
    //      being used.)
    // -----------------------------------------------------------------------
    unsigned int  fEmptyNamespaceId;
    unsigned int  fStackCapacity;
    unsigned int  fStackTop;
    XMLStringPool fPrefixPool;
    StackElem**   fStack;
    MemoryManager* fMemoryManager;
};

// ---------------------------------------------------------------------------
//  NamespaceScope: Miscellaneous methods
// ---------------------------------------------------------------------------
inline bool NamespaceScope::isEmpty() const
{
    return (fStackTop == 0);
}

inline unsigned int NamespaceScope::getEmptyNamespaceId() const
{
    return fEmptyNamespaceId;
}


XERCES_CPP_NAMESPACE_END

#endif

/**
  * End of file NameSpaceScope.hpp
  */

