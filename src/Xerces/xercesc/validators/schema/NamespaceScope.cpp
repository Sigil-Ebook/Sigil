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
 * $Id: NamespaceScope.cpp 729944 2008-12-29 17:03:32Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <string.h>
#include <xercesc/util/EmptyStackException.hpp>
#include <xercesc/validators/schema/NamespaceScope.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  NamespaceScope: Constructors and Destructor
// ---------------------------------------------------------------------------
NamespaceScope::NamespaceScope(MemoryManager* const manager /*= XMLPlatformUtils::fgMemoryManager*/) :
    fEmptyNamespaceId(0)
    , fStackCapacity(8)
    , fStackTop(0)
    , fPrefixPool(109, manager)
    , fStack(0)
    , fMemoryManager(manager)
{
    // Do an initial allocation of the stack and zero it out
    fStack = (StackElem**) fMemoryManager->allocate
    (
        fStackCapacity * sizeof(StackElem*)
    );//new StackElem*[fStackCapacity];
    memset(fStack, 0, fStackCapacity * sizeof(StackElem*));
}

NamespaceScope::NamespaceScope(const NamespaceScope* const initialize, MemoryManager* const manager /*= XMLPlatformUtils::fgMemoryManager*/) :
    fEmptyNamespaceId(0)
    , fStackCapacity(8)
    , fStackTop(0)
    , fPrefixPool(109, manager)
    , fStack(0)
    , fMemoryManager(manager)
{
    // Do an initial allocation of the stack and zero it out
    fStack = (StackElem**) fMemoryManager->allocate
    (
        fStackCapacity * sizeof(StackElem*)
    );//new StackElem*[fStackCapacity];
    memset(fStack, 0, fStackCapacity * sizeof(StackElem*));

    if(initialize)
    {
        reset(initialize->fEmptyNamespaceId);

        // copy the existing bindings
        for (unsigned int index = initialize->fStackTop; index > 0; index--)
        {
            // Get a convenience pointer to the current element
            StackElem* curRow = initialize->fStack[index-1];

            // If no prefixes mapped at this level, then go the next one
            if (!curRow->fMapCount)
                continue;

            for (unsigned int mapIndex = 0; mapIndex < curRow->fMapCount; mapIndex++)
            {
                // go from the id to the prefix
                const XMLCh* prefix = initialize->fPrefixPool.getValueForId(curRow->fMap[mapIndex].fPrefId);

                // if the prefix is not already known, add it
                if(getNamespaceForPrefix(prefix)==fEmptyNamespaceId)
                    addPrefix(prefix, curRow->fMap[mapIndex].fURIId);
            }
        }
    }
}

NamespaceScope::~NamespaceScope()
{
    //
    //  Start working from the bottom of the stack and clear it out as we
    //  go up. Once we hit an uninitialized one, we can break out.
    //
    for (unsigned int stackInd = 0; stackInd < fStackCapacity; stackInd++)
    {
        // If this entry has been set, then lets clean it up
        if (!fStack[stackInd])
            break;

        // Delete the row for this entry, then delete the row structure
        fMemoryManager->deallocate(fStack[stackInd]->fMap);//delete [] fStack[stackInd]->fMap;
        delete fStack[stackInd];
    }

    // Delete the stack array itself now
    fMemoryManager->deallocate(fStack);//delete [] fStack;
}


// ---------------------------------------------------------------------------
//  NamespaceScope: Stack access
// ---------------------------------------------------------------------------
unsigned int NamespaceScope::increaseDepth()
{
    // See if we need to expand the stack
    if (fStackTop == fStackCapacity)
        expandStack();

    // If this element has not been initialized yet, then initialize it
    if (!fStack[fStackTop])
    {
        fStack[fStackTop] = new (fMemoryManager) StackElem;
        fStack[fStackTop]->fMapCapacity = 0;
        fStack[fStackTop]->fMap = 0;
    }

    // Set up the new top row
    fStack[fStackTop]->fMapCount = 0;

    // Bump the top of stack
    fStackTop++;

    return fStackTop-1;
}

unsigned int NamespaceScope::decreaseDepth()
{
    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_StackUnderflow, fMemoryManager);

    fStackTop--;

    return fStackTop;
}


// ---------------------------------------------------------------------------
//  NamespaceScope: Prefix map methods
// ---------------------------------------------------------------------------
void NamespaceScope::addPrefix(const XMLCh* const prefixToAdd,
                               const unsigned int uriId) {

    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_EmptyStack, fMemoryManager);

    // Get a convenience pointer to the stack top row
    StackElem* curRow = fStack[fStackTop - 1];

    // Map the prefix to its unique id
    const unsigned int prefId = fPrefixPool.addOrFind(prefixToAdd);

    // Search the map at this level for the passed prefix
    for (unsigned int mapIndex = 0; mapIndex < curRow->fMapCount; mapIndex++)
    {
        if (curRow->fMap[mapIndex].fPrefId == prefId)
        {
            curRow->fMap[mapIndex].fURIId = uriId;
            return;
        }
    }

    //
    //  Add a new element to the prefix map for this element. If its full,
    //  then expand it out.
    //
    if (curRow->fMapCount == curRow->fMapCapacity)
        expandMap(curRow);

    //
    //  And now add a new element for this prefix.
    //
    curRow->fMap[curRow->fMapCount].fPrefId = prefId;
    curRow->fMap[curRow->fMapCount].fURIId = uriId;

    // Bump the map count now
    curRow->fMapCount++;
}

unsigned int
NamespaceScope::getNamespaceForPrefix(const XMLCh* const prefixToMap) const {

    //
    //  Map the prefix to its unique id, from the prefix string pool. If its
    //  not a valid prefix, then its a failure.
    //
    unsigned int prefixId = fPrefixPool.getId(prefixToMap);

    if (!prefixId){
        return fEmptyNamespaceId;
    }

    //
    //  Start at the stack top and work backwards until we come to some
    //  element that mapped this prefix.
    //
    for (unsigned int index = fStackTop; index > 0; index--)
    {
        // Get a convenience pointer to the current element
        StackElem* curRow = fStack[index-1];

        // If no prefixes mapped at this level, then go the next one
        if (!curRow->fMapCount)
            continue;

        // Search the map at this level for the passed prefix
        for (unsigned int mapIndex = 0; mapIndex < curRow->fMapCount; mapIndex++)
        {
            if (curRow->fMap[mapIndex].fPrefId == prefixId)
                return curRow->fMap[mapIndex].fURIId;
        }
    }

    return fEmptyNamespaceId;
}


// ---------------------------------------------------------------------------
//  NamespaceScope: Miscellaneous methods
// ---------------------------------------------------------------------------
void NamespaceScope::reset(const unsigned int emptyId)
{
    // Flush the prefix pool and put back in the standard prefixes
    fPrefixPool.flushAll();

    // Reset the stack top to clear the stack
    fStackTop = 0;

    // And store the new special URI ids
    fEmptyNamespaceId = emptyId;

    // add the first storage
    increaseDepth();
}


// ---------------------------------------------------------------------------
//  Namespace: Private helpers
// ---------------------------------------------------------------------------
void NamespaceScope::expandMap(StackElem* const toExpand)
{
    // For convenience get the old map size
    const unsigned int oldCap = toExpand->fMapCapacity;

    //
    //  Expand the capacity by 25%, or initialize it to 16 if its currently
    //  empty. Then allocate a new temp buffer.
    //
    const unsigned int newCapacity = oldCap ?
                                     (unsigned int)(oldCap * 1.25) : 16;
    PrefMapElem* newMap = (PrefMapElem*) fMemoryManager->allocate
    (
        newCapacity * sizeof(PrefMapElem)
    );//new PrefMapElem[newCapacity];

    //
    //  Copy over the old stuff. We DON'T have to zero out the new stuff
    //  since this is a by value map and the current map index controls what
    //  is relevant.
    //
    memcpy(newMap, toExpand->fMap, oldCap * sizeof(PrefMapElem));

    // Delete the old map and store the new stuff
    fMemoryManager->deallocate(toExpand->fMap);//delete [] toExpand->fMap;
    toExpand->fMap = newMap;
    toExpand->fMapCapacity = newCapacity;
}

void NamespaceScope::expandStack()
{
    // Expand the capacity by 25% and allocate a new buffer
    const unsigned int newCapacity = (unsigned int)(fStackCapacity * 1.25);
    StackElem** newStack = (StackElem**) fMemoryManager->allocate
    (
        newCapacity * sizeof(StackElem*)
    );//new StackElem*[newCapacity];

    // Copy over the old stuff
    memcpy(newStack, fStack, fStackCapacity * sizeof(StackElem*));

    //
    //  And zero out the new stuff. Though we use a stack top, we reuse old
    //  stack contents so we need to know if elements have been initially
    //  allocated or not as we push new stuff onto the stack.
    //
    memset
    (
        &newStack[fStackCapacity]
        , 0
        , (newCapacity - fStackCapacity) * sizeof(StackElem*)
    );

    // Delete the old array and update our members
    fMemoryManager->deallocate(fStack);//delete [] fStack;
    fStack = newStack;
    fStackCapacity = newCapacity;
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file NamespaceScope.cpp
  */

