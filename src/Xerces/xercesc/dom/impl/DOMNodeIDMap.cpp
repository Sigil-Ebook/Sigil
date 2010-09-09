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
 * $Id: DOMNodeIDMap.cpp 678144 2008-07-19 12:08:55Z borisk $
 */

#include "DOMAttrImpl.hpp"
#include "DOMDocumentImpl.hpp"
#include "DOMNodeIDMap.hpp"

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <stdio.h>

XERCES_CPP_NAMESPACE_BEGIN


static const XMLSize_t gPrimes[] = {997, 9973, 99991, 999983, 0 };  // To do - add a few more.

static const float gMaxFill = 0.8f;   // The maximum fraction of the total
                                    // table entries to consume before exanding.

DOMNodeIDMap::DOMNodeIDMap(XMLSize_t initialSize, DOMDocument *doc)
: fNumEntries(0)
, fDoc(doc)
{
    for (fSizeIndex = 0; gPrimes[fSizeIndex] < initialSize; fSizeIndex++)
    {
        if (gPrimes[fSizeIndex] == 0)
        {
            // We need a bigger size than the largest available one.
            //   Big trouble.
            fSizeIndex--;
            ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::NodeIDMap_GrowErr, ((DOMDocumentImpl *)fDoc)->getMemoryManager());
        }
    }

    fSize = gPrimes[fSizeIndex];
    fMaxEntries = (XMLSize_t)(float(fSize) * gMaxFill);

    //fTable = new (fDoc) DOMAttr*[fSize];
    fTable = (DOMAttr**) ((DOMDocumentImpl *)fDoc)->allocate(sizeof(DOMAttr*) * fSize);
    XMLSize_t i;
    for (i=0; i<fSize; i++)
        fTable[i] = 0;
}


DOMNodeIDMap::~DOMNodeIDMap()
{
    // don't delete - the document owns the storage.
    fTable = 0;
}



void DOMNodeIDMap::add(DOMAttr *attr)
{
	//
	//  If the table is getting too full, grow it.  We arbitrarily limit
	//   the table to 80 full, which should limit the average number of
	//   rehashes to a reasonable value.
	//
	if (fNumEntries >= fMaxEntries)
		growTable();

        fNumEntries++;

	//
	// Hash the value string from the ID attribute being added to the table
	//      0 < Initial hash value < table size.
	//      An initial hash of zero would cause the rehash to fail.
	//
	const XMLCh *id=attr->getValue();
        XMLSize_t initalHash = XMLString::hash(id, fSize-1);
	initalHash++;
	XMLSize_t currentHash = initalHash;

	//
	// Loop looking for an empty slot for this ID.
	//   Don't even bother checking to see if the ID is already there -
	//   the table is only filled by the parser from valid documents, which
	//   can not have duplicates.  Behavior of invalid docs is not defined.
	//
    while (fTable[currentHash]!=0 && fTable[currentHash]!=(DOMAttr *)-1)
	{
		currentHash += initalHash;  // rehash
        if (currentHash >= fSize)
            currentHash = currentHash % fSize;
    }

    //
    // We've found our slot.  Stick the pointer to the attr into it.
    //
    fTable[currentHash] = attr;

}


void DOMNodeIDMap::remove(DOMAttr *attr)
{
    //
	// Hash the value string from the ID attribute being added to the table
	//      0 < Initial hash value < table size.
	//      An initial hash of zero would cause the rehash to fail.
	//
	const XMLCh *id=attr->getValue();
        XMLSize_t initalHash = XMLString::hash(id, fSize-1);
	initalHash++;
	XMLSize_t currentHash = initalHash;

	//
	// Loop looking for a slot pointing to an attr with this id.
    //
	DOMAttr *tableSlot;
    while ((tableSlot= fTable[currentHash])!=0)
	{
        if (tableSlot == attr)
        {
            //  Found the attribute.  Set the slot to -1 to indicate
            //   that it was once used, meaning that lookups, while never
            //   matching here, can not stop either, but must rehash again
            //   and continue searching.
            fTable[currentHash] = (DOMAttr *)-1;
            return;
        }

        currentHash += initalHash;  // rehash.
        if (currentHash >= fSize)
            currentHash = currentHash % fSize;
    }
    // There is no matching entry in the table
}


DOMAttr *DOMNodeIDMap::find(const XMLCh *id)
{
    //
    //  Get the hashcode for the supplied string.
    //
    XMLSize_t initalHash = XMLString::hash(id, fSize-1);
	initalHash++;
	XMLSize_t currentHash = initalHash;

	//
	// Loop looking for a slot pointing to an attr with this id.
    //
	DOMAttr *tableSlot;
    while ((tableSlot= fTable[currentHash])!=0)
	{
        if ((tableSlot != (DOMAttr *)-1) && XMLString::equals(tableSlot->getValue(), id))
            return tableSlot;

        currentHash += initalHash;  // rehash
        if (currentHash >= fSize)
            currentHash = currentHash % fSize;
    }
    // There is no matching entry in the table
    return 0;
}


//
//  Grow the table to the next larger size.
//      It has gotten too full for efficient operation.
//     (We never fill it all the way)
//
void DOMNodeIDMap::growTable()
{
    DOMAttr     **oldTable = fTable;
    XMLSize_t oldSize  = fSize;

    //
    //  Figure the new table size.
    //
#if defined(XERCES_DEBUG)
    fprintf(stderr, "growing...\n");
#endif
    fSizeIndex++;
    fSize = gPrimes[fSizeIndex];
    if (fSize == 0)
    {
        // We need to grow bigger than the largest available size.
        //   Big trouble.
        fSizeIndex--;
        ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::NodeIDMap_GrowErr, ((DOMDocumentImpl *)fDoc)->getMemoryManager());
    }

    //
    //  Allocate the new table.
    //
    //fTable = new (fDoc) DOMAttr *[fSize];
    fTable = (DOMAttr**) ((DOMDocumentImpl *)fDoc)->allocate(sizeof(DOMAttr*) * fSize);
    XMLSize_t i;
    for (i=0; i<fSize; i++)
        fTable[i] = 0;

    fMaxEntries = (XMLSize_t)(float(fSize) * gMaxFill);

    //
    // Move entries over from the old table to the new one.
    //
    for (i=0; i<oldSize; i++)
    {
        if ((oldTable[i] != 0)  &&  (oldTable[i] != (DOMAttr *)-1))
            add(oldTable[i]);
    }

    // delete [] oldTable;   (The document owns the storage.  The old table will just
    //                        need to leak until until the document is discarded.)

}


XERCES_CPP_NAMESPACE_END
