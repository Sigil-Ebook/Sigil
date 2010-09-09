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
 * $Id: ContentLeafNameTypeVector.cpp 676911 2008-07-15 13:27:32Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/validators/common/ContentLeafNameTypeVector.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  ContentLeafNameTypeVector: Constructors and Destructor
// ---------------------------------------------------------------------------
ContentLeafNameTypeVector::ContentLeafNameTypeVector
(
    MemoryManager* const manager
)
: fMemoryManager(manager)
, fLeafNames(0)
, fLeafTypes(0)
, fLeafCount(0)
{
}

ContentLeafNameTypeVector::ContentLeafNameTypeVector
(
      QName** const                     names
    , ContentSpecNode::NodeTypes* const types
    , const XMLSize_t                   count
    , MemoryManager* const              manager
)
: fMemoryManager(manager)
, fLeafNames(0)
, fLeafTypes(0)
, fLeafCount(0)
{
    setValues(names, types, count);
}

/***
copy ctor
***/
ContentLeafNameTypeVector::ContentLeafNameTypeVector
(
    const ContentLeafNameTypeVector& toCopy
)
: XMemory(toCopy)
, fMemoryManager(toCopy.fMemoryManager)
, fLeafNames(0)
, fLeafTypes(0)
, fLeafCount(0)
{
    fLeafCount=toCopy.getLeafCount();
    init(fLeafCount);

    for (XMLSize_t i=0; i<this->fLeafCount; i++)
    {
        fLeafNames[i] = toCopy.getLeafNameAt(i);
        fLeafTypes[i] = toCopy.getLeafTypeAt(i);
    }
}

ContentLeafNameTypeVector::~ContentLeafNameTypeVector()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  ContentSpecType: Setter methods
// ---------------------------------------------------------------------------
void ContentLeafNameTypeVector::setValues
    (
         QName** const                      names
       , ContentSpecNode::NodeTypes* const  types
       , const XMLSize_t                    count
    )
{
    cleanUp();
    init(count);

    for (XMLSize_t i=0; i<count; i++)
    {
        fLeafNames[i] = names[i];
        fLeafTypes[i] = types[i];
    }
}

// ---------------------------------------------------------------------------
//  ContentLeafNameTypeVector: Getter methods
// ---------------------------------------------------------------------------
QName* ContentLeafNameTypeVector::getLeafNameAt(const XMLSize_t pos) const
{
    if (pos >= fLeafCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);

    return fLeafNames[pos];
}

ContentSpecNode::NodeTypes ContentLeafNameTypeVector::getLeafTypeAt
       (const XMLSize_t pos) const
{
    if (pos >= fLeafCount)
        ThrowXMLwithMemMgr(ArrayIndexOutOfBoundsException, XMLExcepts::Vector_BadIndex, fMemoryManager);

	return fLeafTypes[pos];
}

XMLSize_t ContentLeafNameTypeVector::getLeafCount() const
{
	return fLeafCount;
}

XERCES_CPP_NAMESPACE_END
