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
 * $Id: PSVIAttributeList.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_PSVIATTRIBUTE_LIST_HPP)
#define XERCESC_INCLUDE_GUARD_PSVIATTRIBUTE_LIST_HPP

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/psvi/PSVIAttribute.hpp>
#include <xercesc/util/RefVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * A container for the PSVI contributions to attributes that occur
 * on a particular element.
 * This is always owned by the parser/validator from
 * which it is obtained.  The parser/validator will specify
 * under what conditions it may be relied upon to have meaningful contents.
 */

class XMLPARSER_EXPORT PSVIAttributeStorage : public XMemory
{
public:
    PSVIAttributeStorage() :
        fPSVIAttribute(0)
      , fAttributeName(0)
      , fAttributeNamespace(0)
    {
    }

    ~PSVIAttributeStorage()
    {
        delete fPSVIAttribute;
    }

    PSVIAttribute* fPSVIAttribute;
    const XMLCh*   fAttributeName;
    const XMLCh*   fAttributeNamespace;
};

class XMLPARSER_EXPORT PSVIAttributeList : public XMemory
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor
      *
      * @param  manager     The configurable memory manager
      */
    PSVIAttributeList( MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    //@};

    /** @name Destructor */
    //@{
    ~PSVIAttributeList();
    //@}

    //---------------------
    /** @name PSVIAttributeList methods */

    //@{

    /*
     * Get the number of attributes whose PSVI contributions
     * are contained in this list.
     */
    XMLSize_t getLength() const;

    /*
     * Get the PSVI contribution of attribute at position i
     * in this list.  Indices start from 0.
     * @param index index from which the attribute PSVI contribution
     * is to come.
     * @return PSVIAttribute containing the attributes PSVI contributions;
     * null is returned if the index is out of range.
     */
    PSVIAttribute *getAttributePSVIAtIndex(const XMLSize_t index);

    /*
     * Get local part of attribute name at position index in the list.
     * Indices start from 0.
     * @param index index from which the attribute name
     * is to come.
     * @return local part of the attribute's name; null is returned if the index
     * is out of range.
     */
    const XMLCh *getAttributeNameAtIndex(const XMLSize_t index);

    /*
     * Get namespace of attribute at position index in the list.
     * Indices start from 0.
     * @param index index from which the attribute namespace
     * is to come.
     * @return namespace of the attribute;
     * null is returned if the index is out of range.
     */
    const XMLCh *getAttributeNamespaceAtIndex(const XMLSize_t index);

    /*
     * Get the PSVI contribution of attribute with given
     * local name and namespace.
     * @param attrName  local part of the attribute's name
     * @param attrNamespace  namespace of the attribute
     * @return null if the attribute PSVI does not exist
     */
    PSVIAttribute *getAttributePSVIByName(const XMLCh *attrName
                    , const XMLCh * attrNamespace);

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{

    /**
      * returns a PSVI attribute of undetermined state and given name/namespace and
      * makes that object part of the internal list.  Intended to be called
      * during validation of an element.
      * @param attrName     name of this attribute
      * @param attrNS       URI of the attribute
      * @return             new, uninitialized, PSVIAttribute object
      */
    PSVIAttribute *getPSVIAttributeToFill(
            const XMLCh * attrName
            , const XMLCh * attrNS);

    /**
      * reset the list
      */
    void reset();

    //@}

private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    PSVIAttributeList(const PSVIAttributeList&);
    PSVIAttributeList & operator=(const PSVIAttributeList &);


    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    // fMemoryManager
    //  handler to provide dynamically-need memory
    // fAttrList
    //  list of PSVIAttributes contained by this object
    // fAttrPos
    //  current number of initialized PSVIAttributes in fAttrList
    MemoryManager*                      fMemoryManager;
    RefVectorOf<PSVIAttributeStorage>*  fAttrList;
    XMLSize_t                           fAttrPos;
};

inline PSVIAttributeList::~PSVIAttributeList()
{
    delete fAttrList;
}

inline PSVIAttribute *PSVIAttributeList::getPSVIAttributeToFill(
            const XMLCh *attrName
            , const XMLCh * attrNS)
{
    PSVIAttributeStorage* storage = 0;
    if(fAttrPos == fAttrList->size())
    {
        storage = new (fMemoryManager) PSVIAttributeStorage();
        storage->fPSVIAttribute = new (fMemoryManager) PSVIAttribute(fMemoryManager);
        fAttrList->addElement(storage);
    }
    else
    {
        storage = fAttrList->elementAt(fAttrPos);
    }
    storage->fAttributeName = attrName;
    storage->fAttributeNamespace = attrNS;
    fAttrPos++;
    return storage->fPSVIAttribute;
}

inline void PSVIAttributeList::reset()
{
    fAttrPos = 0;
}

XERCES_CPP_NAMESPACE_END

#endif
