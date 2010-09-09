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

#include "DOMXPathResultImpl.hpp"
#include <xercesc/dom/DOMNode.hpp>
#include <xercesc/dom/DOMXPathException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

DOMXPathResultImpl::DOMXPathResultImpl(ResultType type,
                                       MemoryManager* const manager)
    : fType(type),
      fMemoryManager(manager),
      fIndex (0)
{
    fSnapshot = new (fMemoryManager) RefVectorOf<DOMNode>(13, false, fMemoryManager);
}

DOMXPathResultImpl::~DOMXPathResultImpl()
{
    delete fSnapshot;
}

//
//
DOMXPathResult::ResultType DOMXPathResultImpl::getResultType() const
{
    return fType;
}

const DOMTypeInfo* DOMXPathResultImpl::getTypeInfo() const
{
    throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

bool DOMXPathResultImpl::isNode() const
{
    throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

bool DOMXPathResultImpl::getBooleanValue() const
{
    throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

int DOMXPathResultImpl::getIntegerValue() const
{
    throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

double DOMXPathResultImpl::getNumberValue() const
{
    throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

const XMLCh* DOMXPathResultImpl::getStringValue() const
{
    throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

DOMNode* DOMXPathResultImpl::getNodeValue() const
{
  if(fType == ANY_UNORDERED_NODE_TYPE || fType == FIRST_ORDERED_NODE_TYPE)
  {
    return fSnapshot->size() > 0 ? fSnapshot->elementAt(0) : 0;
  }
  else if (fType==UNORDERED_NODE_SNAPSHOT_TYPE || fType==ORDERED_NODE_SNAPSHOT_TYPE)
  {
    return fIndex < fSnapshot->size() ? fSnapshot->elementAt(fIndex) : 0;
  }
  else
    throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

bool DOMXPathResultImpl::iterateNext()
{
    throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

bool DOMXPathResultImpl::getInvalidIteratorState() const
{
    throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

bool DOMXPathResultImpl::snapshotItem(XMLSize_t index)
{
    if(fType==UNORDERED_NODE_SNAPSHOT_TYPE || fType==ORDERED_NODE_SNAPSHOT_TYPE)
    {
        fIndex = index;
        return fIndex < fSnapshot->size();
    }
    else
      throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

XMLSize_t DOMXPathResultImpl::getSnapshotLength() const
{
    if(fType==UNORDERED_NODE_SNAPSHOT_TYPE || fType==ORDERED_NODE_SNAPSHOT_TYPE)
        return fSnapshot->size();
    else
        throw DOMXPathException(DOMXPathException::TYPE_ERR, 0, fMemoryManager);
}

void DOMXPathResultImpl::release()
{
    DOMXPathResultImpl* me = this;
    delete me;
}

//
//
void DOMXPathResultImpl::reset(ResultType type)
{
    fType = type;
    fSnapshot->removeAllElements();
    fIndex = 0;
}

void DOMXPathResultImpl::addResult(DOMNode* node)
{
    fSnapshot->addElement(node);
}

XERCES_CPP_NAMESPACE_END
