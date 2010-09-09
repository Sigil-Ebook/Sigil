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
 * $Id: PSVIAttributeList.cpp 674012 2008-07-04 11:18:21Z borisk $
 */

#include <xercesc/framework/psvi/PSVIAttributeList.hpp>
#include <xercesc/framework/psvi/XSAttributeDeclaration.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

PSVIAttributeList::PSVIAttributeList( MemoryManager* const manager ):
    fMemoryManager(manager)
  , fAttrList(0)
  , fAttrPos(0)
{
    fAttrList= new (fMemoryManager) RefVectorOf<PSVIAttributeStorage> (10, true, fMemoryManager);
}

/*
 * Get the number of attributes whose PSVI contributions
 * are contained in this list.
 */
XMLSize_t PSVIAttributeList::getLength() const
{
    return fAttrPos;
}

/*
 * Get the PSVI contribution of attribute at position i
 * in this list.  Indeces start from 0.
 * @param index index from which the attribute PSVI contribution
 * is to come.
 * @return PSVIAttribute containing the attributes PSVI contributions;
 * null is returned if the index is out of range.
 */
PSVIAttribute *PSVIAttributeList::getAttributePSVIAtIndex(const XMLSize_t index)
{
    if(index >= fAttrPos)
        return 0;
    return fAttrList->elementAt(index)->fPSVIAttribute;
}

/*
 * Get local part of attribute name at position index in the list.
 * Indeces start from 0.
 * @param index index from which the attribute name
 * is to come.
 * @return local part of the attribute's name; null is returned if the index
 * is out of range.
 */
const XMLCh *PSVIAttributeList::getAttributeNameAtIndex(const XMLSize_t index)
{

    if(index >= fAttrPos)
        return 0;
    return fAttrList->elementAt(index)->fAttributeName;
}

/*
 * Get namespace of attribute at position index in the list.
 * Indeces start from 0.
 * @param index index from which the attribute namespace
 * is to come.
 * @return namespace of the attribute;
 * null is returned if the index is out of range.
 */
const XMLCh *PSVIAttributeList::getAttributeNamespaceAtIndex(const XMLSize_t index)
{
    if(index >= fAttrPos)
        return 0;
    return fAttrList->elementAt(index)->fAttributeNamespace;
}

/*
 * Get the PSVI contribution of attribute with given
 * local name and namespace.
 * @param attrName  local part of the attribute's name
 * @param attrNamespace  namespace of the attribute
 * @return null if the attribute PSVI does not exist
 */
PSVIAttribute *PSVIAttributeList::getAttributePSVIByName(const XMLCh *attrName
                , const XMLCh * attrNamespace)
{
    for (XMLSize_t index=0; index < fAttrPos; index++) {
        PSVIAttributeStorage* storage = fAttrList->elementAt(index);
        if (XMLString::equals(attrName,storage->fAttributeName) &&
            XMLString::equals(attrNamespace,storage->fAttributeNamespace))
            return storage->fPSVIAttribute;
    }
    return 0;
}

XERCES_CPP_NAMESPACE_END
