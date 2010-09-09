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
 * $Id: ElemStack.cpp 830538 2009-10-28 13:41:11Z amassari $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <string.h>
#include <xercesc/util/EmptyStackException.hpp>
#include <xercesc/util/NoSuchElementException.hpp>
#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/internal/ElemStack.hpp>
#include <xercesc/validators/common/Grammar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  ElemStack: Constructors and Destructor
// ---------------------------------------------------------------------------
ElemStack::ElemStack(MemoryManager* const manager) :

    fEmptyNamespaceId(0)
    , fGlobalPoolId(0)
    , fPrefixPool(109, manager)
    , fGlobalNamespaces(0)
    , fStack(0)
    , fStackCapacity(32)
    , fStackTop(0)
    , fUnknownNamespaceId(0)
    , fXMLNamespaceId(0)
    , fXMLPoolId(0)
    , fXMLNSNamespaceId(0)
    , fXMLNSPoolId(0)
    , fNamespaceMap(0)
    , fMemoryManager(manager)
{
    // Do an initial allocation of the stack and zero it out
    fStack = (StackElem**) fMemoryManager->allocate
    (
        fStackCapacity * sizeof(StackElem*)
    );//new StackElem*[fStackCapacity];
    memset(fStack, 0, fStackCapacity * sizeof(StackElem*));

    fNamespaceMap = new (fMemoryManager) ValueVectorOf<PrefMapElem*>(16, fMemoryManager);
}

ElemStack::~ElemStack()
{
    if(fGlobalNamespaces)
    {
        fMemoryManager->deallocate(fGlobalNamespaces->fMap);
        delete fGlobalNamespaces;
    }

    //
    //  Start working from the bottom of the stack and clear it out as we
    //  go up. Once we hit an uninitialized one, we can break out.
    //
    for (XMLSize_t stackInd = 0; stackInd < fStackCapacity; stackInd++)
    {
        // If this entry has been set, then lets clean it up
        if (!fStack[stackInd])
            break;

        fMemoryManager->deallocate(fStack[stackInd]->fChildren);//delete [] fStack[stackInd]->fChildren;
        fMemoryManager->deallocate(fStack[stackInd]->fMap);//delete [] fStack[stackInd]->fMap;
        fMemoryManager->deallocate(fStack[stackInd]->fSchemaElemName);
        delete fStack[stackInd];
    }

    // Delete the stack array itself now
    fMemoryManager->deallocate(fStack);//delete [] fStack;
    delete fNamespaceMap;
}


// ---------------------------------------------------------------------------
//  ElemStack: Stack access
// ---------------------------------------------------------------------------
XMLSize_t ElemStack::addLevel()
{
    // See if we need to expand the stack
    if (fStackTop == fStackCapacity)
        expandStack();

    // If this element has not been initialized yet, then initialize it
    if (!fStack[fStackTop])
    {
        fStack[fStackTop] = new (fMemoryManager) StackElem;
        fStack[fStackTop]->fChildCapacity = 0;
        fStack[fStackTop]->fChildren = 0;
        fStack[fStackTop]->fMapCapacity = 0;
        fStack[fStackTop]->fMap = 0;
        fStack[fStackTop]->fSchemaElemName = 0;
        fStack[fStackTop]->fSchemaElemNameMaxLen = 0;
    }

    // Set up the new top row
    fStack[fStackTop]->fThisElement = 0;
    fStack[fStackTop]->fReaderNum = 0xFFFFFFFF;
    fStack[fStackTop]->fChildCount = 0;
    fStack[fStackTop]->fMapCount = 0;
    fStack[fStackTop]->fValidationFlag = false;
    fStack[fStackTop]->fCommentOrPISeen = false;
    fStack[fStackTop]->fReferenceEscaped = false;
    fStack[fStackTop]->fCurrentURI = fUnknownNamespaceId;
    fStack[fStackTop]->fCurrentScope = Grammar::TOP_LEVEL_SCOPE;
    fStack[fStackTop]->fCurrentGrammar = 0;

    // Bump the top of stack
    fStackTop++;

    return fStackTop-1;
}


XMLSize_t ElemStack::addLevel(XMLElementDecl* const toSet, const XMLSize_t readerNum)
{
    // See if we need to expand the stack
    if (fStackTop == fStackCapacity)
        expandStack();

    // If this element has not been initialized yet, then initialize it
    if (!fStack[fStackTop])
    {
        fStack[fStackTop] = new (fMemoryManager) StackElem;
        fStack[fStackTop]->fChildCapacity = 0;
        fStack[fStackTop]->fChildren = 0;
        fStack[fStackTop]->fMapCapacity = 0;
        fStack[fStackTop]->fMap = 0;
        fStack[fStackTop]->fSchemaElemName = 0;
        fStack[fStackTop]->fSchemaElemNameMaxLen = 0;
    }

    // Set up the new top row
    fStack[fStackTop]->fThisElement = toSet;
    fStack[fStackTop]->fReaderNum = readerNum;
    fStack[fStackTop]->fChildCount = 0;
    fStack[fStackTop]->fMapCount = 0;
    fStack[fStackTop]->fValidationFlag = false;
    fStack[fStackTop]->fCommentOrPISeen = false;
    fStack[fStackTop]->fReferenceEscaped = false;
    fStack[fStackTop]->fCurrentURI = fUnknownNamespaceId;
    fStack[fStackTop]->fCurrentScope = Grammar::TOP_LEVEL_SCOPE;
    fStack[fStackTop]->fCurrentGrammar = 0;

    // Bump the top of stack
    fStackTop++;

    return fStackTop-1;
}



const ElemStack::StackElem* ElemStack::popTop()
{
    // Watch for an underflow error
    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_StackUnderflow, fMemoryManager);

    fStackTop--;
    return fStack[fStackTop];
}


void
ElemStack::setElement(XMLElementDecl* const toSet, const XMLSize_t readerNum)
{
    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_EmptyStack, fMemoryManager);

    fStack[fStackTop - 1]->fThisElement = toSet;
    fStack[fStackTop - 1]->fReaderNum = readerNum;
}


// ---------------------------------------------------------------------------
//  ElemStack: Stack top access
// ---------------------------------------------------------------------------
XMLSize_t ElemStack::addChild(QName* const child, const bool toParent)
{
    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_EmptyStack, fMemoryManager);

    //
    //  If they want to add to the parent, then we have to have at least two
    //  elements on the stack.
    //
    if (toParent && (fStackTop < 2))
        ThrowXMLwithMemMgr(NoSuchElementException, XMLExcepts::ElemStack_NoParentPushed, fMemoryManager);

    // Get a convenience pointer to the stack top row
    StackElem* curRow = toParent
                        ? fStack[fStackTop - 2] : fStack[fStackTop - 1];

    // See if we need to expand this row's child array
    if (curRow->fChildCount == curRow->fChildCapacity)
    {
        // Increase the capacity by a quarter and allocate a new row
        const XMLSize_t newCapacity = curRow->fChildCapacity ?
                                      (XMLSize_t)(curRow->fChildCapacity * 1.25) :
                                      32;
        QName** newRow = (QName**) fMemoryManager->allocate
        (
            newCapacity * sizeof(QName*)
        );//new QName*[newCapacity];

        //
        //  Copy over the old contents. We don't have to initialize the new
        //  part because The current child count is used to know how much of
        //  it is valid.
        //
        //  Only both doing this if there is any current content, since
        //  this code also does the initial faulting in of the array when
        //  both the current capacity and child count are zero.
        //
        for (XMLSize_t index = 0; index < curRow->fChildCount; index++)
            newRow[index] = curRow->fChildren[index];

        // Clean up the old children and store the new info
        fMemoryManager->deallocate(curRow->fChildren);//delete [] curRow->fChildren;
        curRow->fChildren = newRow;
        curRow->fChildCapacity = newCapacity;
    }

    // Add this id to the end of the row's child id array and bump the count
    curRow->fChildren[curRow->fChildCount++] = child;

    // Return the level of the index we just filled (before the bump)
    return curRow->fChildCount - 1;
}

const ElemStack::StackElem* ElemStack::topElement() const
{
    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_EmptyStack, fMemoryManager);

    return fStack[fStackTop - 1];
}


// ---------------------------------------------------------------------------
//  ElemStack: Prefix map methods
// ---------------------------------------------------------------------------
void ElemStack::addGlobalPrefix(const XMLCh* const    prefixToAdd
                              , const unsigned int    uriId)
{
    if (!fGlobalNamespaces)
    {
        fGlobalNamespaces = new (fMemoryManager) StackElem;
        fGlobalNamespaces->fChildCapacity = 0;
        fGlobalNamespaces->fChildren = 0;
        fGlobalNamespaces->fMapCapacity = 0;
        fGlobalNamespaces->fMap = 0;
        fGlobalNamespaces->fMapCount = 0;
        fGlobalNamespaces->fSchemaElemName = 0;
        fGlobalNamespaces->fSchemaElemNameMaxLen = 0;
        fGlobalNamespaces->fThisElement = 0;
        fGlobalNamespaces->fReaderNum = 0xFFFFFFFF;
        fGlobalNamespaces->fChildCount = 0;
        fGlobalNamespaces->fValidationFlag = false;
        fGlobalNamespaces->fCommentOrPISeen = false;
        fGlobalNamespaces->fReferenceEscaped = false;
        fGlobalNamespaces->fCurrentURI = fUnknownNamespaceId;
        fGlobalNamespaces->fCurrentScope = Grammar::TOP_LEVEL_SCOPE;
        fGlobalNamespaces->fCurrentGrammar = 0;
    }

    // Map the prefix to its unique id
    const unsigned int prefId = fPrefixPool.addOrFind(prefixToAdd);

    //
    //  Add a new element to the prefix map for this element. If its full,
    //  then expand it out.
    //
    if (fGlobalNamespaces->fMapCount == fGlobalNamespaces->fMapCapacity)
        expandMap(fGlobalNamespaces);

    //
    //  And now add a new element for this prefix. Watch for the special case
    //  of xmlns=="", and force it to ""=[globalid]
    //
    fGlobalNamespaces->fMap[fGlobalNamespaces->fMapCount].fPrefId = prefId;
    if ((prefId == fGlobalPoolId) && (uriId == fEmptyNamespaceId))
        fGlobalNamespaces->fMap[fGlobalNamespaces->fMapCount].fURIId = fEmptyNamespaceId;
    else
        fGlobalNamespaces->fMap[fGlobalNamespaces->fMapCount].fURIId = uriId;

    // Bump the map count now
    fGlobalNamespaces->fMapCount++;
}

void ElemStack::addPrefix(  const   XMLCh* const    prefixToAdd
                            , const unsigned int    uriId)
{
    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_EmptyStack, fMemoryManager);

    // Get a convenience pointer to the stack top row
    StackElem* curRow = fStack[fStackTop - 1];

    // Map the prefix to its unique id
    const unsigned int prefId = fPrefixPool.addOrFind(prefixToAdd);

    //
    //  Add a new element to the prefix map for this element. If its full,
    //  then expand it out.
    //
    if (curRow->fMapCount == curRow->fMapCapacity)
        expandMap(curRow);

    //
    //  And now add a new element for this prefix. Watch for the special case
    //  of xmlns=="", and force it to ""=[globalid]
    //
    curRow->fMap[curRow->fMapCount].fPrefId = prefId;
    if ((prefId == fGlobalPoolId) && (uriId == fEmptyNamespaceId))
        curRow->fMap[curRow->fMapCount].fURIId = fEmptyNamespaceId;
    else
        curRow->fMap[curRow->fMapCount].fURIId = uriId;

    // Bump the map count now
    curRow->fMapCount++;
}


unsigned int ElemStack::mapPrefixToURI( const   XMLCh* const    prefixToMap
                                        ,       bool&           unknown) const
{
    // Assume we find it
    unknown = false;

    //
    //  Map the prefix to its unique id, from the prefix string pool. If its
    //  not a valid prefix, then its a failure.
    //
    unsigned int prefixId = (!prefixToMap || !*prefixToMap)?fGlobalPoolId : fPrefixPool.getId(prefixToMap);
    if (prefixId == 0)
    {
        unknown = true;
        return fUnknownNamespaceId;
    }
    //
    //  Check for the special prefixes 'xml' and 'xmlns' since they cannot
    //  be overridden.
    //
    else if (prefixId == fXMLPoolId)
        return fXMLNamespaceId;
    else if (prefixId == fXMLNSPoolId)
        return fXMLNSNamespaceId;

    //
    //  Start at the stack top and work backwards until we come to some
    //  element that mapped this prefix.
    //
    for (XMLSize_t index = fStackTop; index > 0; index--)
    {
        // Get a convenience pointer to the current element
        StackElem* curRow = fStack[index-1];

        // Search the map at this level for the passed prefix
        for (XMLSize_t mapIndex = 0; mapIndex < curRow->fMapCount; mapIndex++)
        {
            if (curRow->fMap[mapIndex].fPrefId == prefixId)
                return curRow->fMap[mapIndex].fURIId;
        }
    }
    //  If the prefix wasn't found, try in the global namespaces
    if(fGlobalNamespaces)
    {
        for (XMLSize_t mapIndex = 0; mapIndex < fGlobalNamespaces->fMapCount; mapIndex++)
        {
            if (fGlobalNamespaces->fMap[mapIndex].fPrefId == prefixId)
                return fGlobalNamespaces->fMap[mapIndex].fURIId;
        }
    }

    //
    //  If the prefix is an empty string, then we will return the special
    //  global namespace id. This can be overridden, but no one has or we
    //  would have not gotten here.
    //
    if (!*prefixToMap)
        return fEmptyNamespaceId;

    // Oh well, don't have a clue so return the unknown id
    unknown = true;
    return fUnknownNamespaceId;
}


ValueVectorOf<PrefMapElem*>* ElemStack::getNamespaceMap() const
{
    fNamespaceMap->removeAllElements();

    //  Start at the stack top and work backwards until we come to some
    //  element that mapped this prefix.
    for (XMLSize_t index = fStackTop; index > 0; index--)
    {
        // Get a convenience pointer to the current element
        StackElem* curRow = fStack[index-1];

        // If no prefixes mapped at this level, then go the next one
        if (!curRow->fMapCount)
            continue;

        // Search the map at this level for the passed prefix
        for (XMLSize_t mapIndex = 0; mapIndex < curRow->fMapCount; mapIndex++)
            fNamespaceMap->addElement(&(curRow->fMap[mapIndex]));
    }
    //  Add the global namespaces
    if(fGlobalNamespaces)
    {
        for (XMLSize_t mapIndex = 0; mapIndex < fGlobalNamespaces->fMapCount; mapIndex++)
            fNamespaceMap->addElement(&(fGlobalNamespaces->fMap[mapIndex]));
    }

    return fNamespaceMap;
}

// ---------------------------------------------------------------------------
//  ElemStack: Miscellaneous methods
// ---------------------------------------------------------------------------
void ElemStack::reset(  const   unsigned int    emptyId
                        , const unsigned int    unknownId
                        , const unsigned int    xmlId
                        , const unsigned int    xmlNSId)
{
    if(fGlobalNamespaces)
    {
        fMemoryManager->deallocate(fGlobalNamespaces->fMap);
        delete fGlobalNamespaces;
        fGlobalNamespaces = 0;
    }

    // Reset the stack top to clear the stack
    fStackTop = 0;

    // if first time, put in the standard prefixes
    if (fXMLPoolId == 0) {

        fGlobalPoolId = fPrefixPool.addOrFind(XMLUni::fgZeroLenString);
        fXMLPoolId = fPrefixPool.addOrFind(XMLUni::fgXMLString);
        fXMLNSPoolId = fPrefixPool.addOrFind(XMLUni::fgXMLNSString);
    }

    // And store the new special URI ids
    fEmptyNamespaceId = emptyId;
    fUnknownNamespaceId = unknownId;
    fXMLNamespaceId = xmlId;
    fXMLNSNamespaceId = xmlNSId;
}


// ---------------------------------------------------------------------------
//  ElemStack: Private helpers
// ---------------------------------------------------------------------------
void ElemStack::expandMap(StackElem* const toExpand)
{
    // For convenience get the old map size
    const XMLSize_t oldCap = toExpand->fMapCapacity;

    //
    //  Expand the capacity by 25%, or initialize it to 16 if its currently
    //  empty. Then allocate a new temp buffer.
    //
    const XMLSize_t  newCapacity = oldCap ?
                                     (XMLSize_t )(oldCap * 1.25) : 16;
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

void ElemStack::expandStack()
{
    // Expand the capacity by 25% and allocate a new buffer
    const XMLSize_t newCapacity = (XMLSize_t)(fStackCapacity * 1.25);
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



// ---------------------------------------------------------------------------
//  WFElemStack: Constructors and Destructor
// ---------------------------------------------------------------------------
WFElemStack::WFElemStack(MemoryManager* const manager) :

    fEmptyNamespaceId(0)
    , fGlobalPoolId(0)
    , fStackCapacity(32)
    , fStackTop(0)
    , fUnknownNamespaceId(0)
    , fXMLNamespaceId(0)
    , fXMLPoolId(0)
    , fXMLNSNamespaceId(0)
    , fXMLNSPoolId(0)
    , fMapCapacity(0)
    , fMap(0)
    , fStack(0)
    , fPrefixPool(109, manager)
    , fMemoryManager(manager)
{
    // Do an initial allocation of the stack and zero it out
    fStack = (StackElem**) fMemoryManager->allocate
    (
        fStackCapacity * sizeof(StackElem*)
    );//new StackElem*[fStackCapacity];
    memset(fStack, 0, fStackCapacity * sizeof(StackElem*));
}

WFElemStack::~WFElemStack()
{
    //
    //  Start working from the bottom of the stack and clear it out as we
    //  go up. Once we hit an uninitialized one, we can break out.
    //
    for (XMLSize_t stackInd = 0; stackInd < fStackCapacity; stackInd++)
    {
        // If this entry has been set, then lets clean it up
        if (!fStack[stackInd])
            break;

        fMemoryManager->deallocate(fStack[stackInd]->fThisElement);//delete [] fStack[stackInd]->fThisElement;
        delete fStack[stackInd];
    }

    if (fMap)
        fMemoryManager->deallocate(fMap);//delete [] fMap;

    // Delete the stack array itself now
    fMemoryManager->deallocate(fStack);//delete [] fStack;
}


// ---------------------------------------------------------------------------
//  WFElemStack: Stack access
// ---------------------------------------------------------------------------
XMLSize_t WFElemStack::addLevel()
{
    // See if we need to expand the stack
    if (fStackTop == fStackCapacity)
        expandStack();

    
    // If this element has not been initialized yet, then initialize it
    if (!fStack[fStackTop])
    {
        fStack[fStackTop] = new (fMemoryManager) StackElem;
        fStack[fStackTop]->fThisElement = 0;
        fStack[fStackTop]->fElemMaxLength = 0;
    }

    // Set up the new top row
    fStack[fStackTop]->fReaderNum = 0xFFFFFFFF;
    fStack[fStackTop]->fCurrentURI = fUnknownNamespaceId;
    fStack[fStackTop]->fTopPrefix = -1;

    if (fStackTop != 0)
        fStack[fStackTop]->fTopPrefix = fStack[fStackTop - 1]->fTopPrefix;

    // Bump the top of stack
    fStackTop++;

    return fStackTop-1;
}


XMLSize_t
WFElemStack::addLevel(const XMLCh* const toSet,
                      const unsigned int toSetLen,
                      const unsigned int readerNum)
{
    // See if we need to expand the stack
    if (fStackTop == fStackCapacity)
        expandStack();

    // If this element has not been initialized yet, then initialize it
    if (!fStack[fStackTop])
    {
        fStack[fStackTop] = new (fMemoryManager) StackElem;
        fStack[fStackTop]->fThisElement = 0;
        fStack[fStackTop]->fElemMaxLength = 0;
    }

    // Set up the new top row
    fStack[fStackTop]->fCurrentURI = fUnknownNamespaceId;
    fStack[fStackTop]->fTopPrefix = -1;

    // And store the new stuff
    if (toSetLen > fStack[fStackTop]->fElemMaxLength) {

        fMemoryManager->deallocate(fStack[fStackTop]->fThisElement);//delete [] fStack[fStackTop]->fThisElement;
        fStack[fStackTop]->fElemMaxLength = toSetLen;
        fStack[fStackTop]->fThisElement = (XMLCh*) fMemoryManager->allocate
        (
            (toSetLen + 1) * sizeof(XMLCh)
        );//new XMLCh[toSetLen + 1];
    }

    XMLString::moveChars(fStack[fStackTop]->fThisElement, toSet, toSetLen + 1);
    fStack[fStackTop]->fReaderNum = readerNum;

    if (fStackTop != 0)
        fStack[fStackTop]->fTopPrefix = fStack[fStackTop - 1]->fTopPrefix;

    // Bump the top of stack
    fStackTop++;

    return fStackTop-1;
}



const WFElemStack::StackElem* WFElemStack::popTop()
{
    // Watch for an underflow error
    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_StackUnderflow, fMemoryManager);

    fStackTop--;
    return fStack[fStackTop];
}


void
WFElemStack::setElement(const XMLCh* const toSet,
                      const unsigned int toSetLen,
                      const unsigned int readerNum)
{
    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_EmptyStack, fMemoryManager);

    if (toSetLen > fStack[fStackTop - 1]->fElemMaxLength) {

        fMemoryManager->deallocate(fStack[fStackTop - 1]->fThisElement);//delete [] fStack[fStackTop - 1]->fThisElement;
        fStack[fStackTop - 1]->fElemMaxLength = toSetLen;
        fStack[fStackTop - 1]->fThisElement = (XMLCh*) fMemoryManager->allocate
        (
            (toSetLen + 1) * sizeof(XMLCh)
        );//new XMLCh[toSetLen + 1];
    }

    XMLString::moveChars(fStack[fStackTop - 1]->fThisElement, toSet, toSetLen + 1);
    fStack[fStackTop - 1]->fReaderNum = readerNum;
}


// ---------------------------------------------------------------------------
//  WFElemStack: Stack top access
// ---------------------------------------------------------------------------
const WFElemStack::StackElem* WFElemStack::topElement() const
{
    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_EmptyStack, fMemoryManager);

    return fStack[fStackTop - 1];
}


// ---------------------------------------------------------------------------
//  WFElemStack: Prefix map methods
// ---------------------------------------------------------------------------
void WFElemStack::addPrefix(  const   XMLCh* const    prefixToAdd
                              , const unsigned int    uriId)
{
    if (!fStackTop)
        ThrowXMLwithMemMgr(EmptyStackException, XMLExcepts::ElemStack_EmptyStack, fMemoryManager);

    // Get a convenience pointer to the stack top row
    StackElem* curRow = fStack[fStackTop - 1];

    // Map the prefix to its unique id
    const unsigned int prefId = fPrefixPool.addOrFind(prefixToAdd);

    //
    //  Add a new element to the prefix map for this element. If its full,
    //  then expand it out.
    //
    if ((unsigned int)curRow->fTopPrefix + 1 == fMapCapacity)
        expandMap();

    //
    //  And now add a new element for this prefix. Watch for the special case
    //  of xmlns=="", and force it to ""=[globalid]
    //
    fMap[curRow->fTopPrefix + 1].fPrefId = prefId;
    if ((prefId == fGlobalPoolId) && (uriId == fEmptyNamespaceId))
        fMap[curRow->fTopPrefix + 1].fURIId = fEmptyNamespaceId;
    else
        fMap[curRow->fTopPrefix + 1].fURIId = uriId;

    // Bump the map count now
    curRow->fTopPrefix++;
}


unsigned int WFElemStack::mapPrefixToURI( const   XMLCh* const    prefixToMap
                                          ,       bool&           unknown) const
{
    // Assume we find it
    unknown = false;

    //
    //  Map the prefix to its unique id, from the prefix string pool. If its
    //  not a valid prefix, then its a failure.
    //
    unsigned int prefixId = fPrefixPool.getId(prefixToMap);
    if (!prefixId)
    {
        unknown = true;
        return fUnknownNamespaceId;
    }

    //
    //  Check for the special prefixes 'xml' and 'xmlns' since they cannot
    //  be overridden.
    //
    if (prefixId == fXMLPoolId)
        return fXMLNamespaceId;
    else if (prefixId == fXMLNSPoolId)
        return fXMLNSNamespaceId;

    //
    //  Start at the stack top and work backwards until we come to some
    //  element that mapped this prefix.
    //
    //  Get a convenience pointer to the stack top row
    StackElem* curRow = fStack[fStackTop - 1];
    for (int mapIndex = curRow->fTopPrefix; mapIndex >=0; mapIndex--)
    {
        if (fMap[mapIndex].fPrefId == prefixId)
            return fMap[mapIndex].fURIId;
    }

    //
    //  If the prefix is an empty string, then we will return the special
    //  global namespace id. This can be overridden, but no one has or we
    //  would have not gotten here.
    //
    if (!*prefixToMap)
        return fEmptyNamespaceId;

    // Oh well, don't have a clue so return the unknown id
    unknown = true;
    return fUnknownNamespaceId;
}


// ---------------------------------------------------------------------------
//  WFElemStack: Miscellaneous methods
// ---------------------------------------------------------------------------
void WFElemStack::reset(  const   unsigned int    emptyId
                          , const unsigned int    unknownId
                          , const unsigned int    xmlId
                          , const unsigned int    xmlNSId)
{
    // Reset the stack top to clear the stack
    fStackTop = 0;

    // if first time, put in the standard prefixes
    if (fXMLPoolId == 0) {

        fGlobalPoolId = fPrefixPool.addOrFind(XMLUni::fgZeroLenString);
        fXMLPoolId = fPrefixPool.addOrFind(XMLUni::fgXMLString);
        fXMLNSPoolId = fPrefixPool.addOrFind(XMLUni::fgXMLNSString);
    }

    // And store the new special URI ids
    fEmptyNamespaceId = emptyId;
    fUnknownNamespaceId = unknownId;
    fXMLNamespaceId = xmlId;
    fXMLNSNamespaceId = xmlNSId;
}


// ---------------------------------------------------------------------------
//  WFElemStack: Private helpers
// ---------------------------------------------------------------------------
void WFElemStack::expandMap()
{
    //
    //  Expand the capacity by 25%, or initialize it to 16 if its currently
    //  empty. Then allocate a new temp buffer.
    //
    const XMLSize_t newCapacity = fMapCapacity ?
                                  (XMLSize_t)(fMapCapacity * 1.25) : 16;
    PrefMapElem* newMap = (PrefMapElem*) fMemoryManager->allocate
    (
        newCapacity * sizeof(PrefMapElem)
    );//new PrefMapElem[newCapacity];

    //
    //  Copy over the old stuff. We DON'T have to zero out the new stuff
    //  since this is a by value map and the current map index controls what
    //  is relevant.
    //
    if (fMapCapacity) {

        memcpy(newMap, fMap, fMapCapacity * sizeof(PrefMapElem));
        fMemoryManager->deallocate(fMap);//delete [] fMap;
    }

    fMap = newMap;
    fMapCapacity = newCapacity;
}

void WFElemStack::expandStack()
{
    // Expand the capacity by 25% and allocate a new buffer
    const XMLSize_t newCapacity = (XMLSize_t)(fStackCapacity * 1.25);
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
