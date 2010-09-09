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
 * $Id: XSObject.hpp 674012 2008-07-04 11:18:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSOBJECT_HPP)
#define XERCESC_INCLUDE_GUARD_XSOBJECT_HPP

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/psvi/XSConstants.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * The XSObject forms the base of the Schema Component Model.  It contains
 * all properties common to the majority of XML Schema components.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  It is designed to be subclassed; subclasses will
 * specify under what conditions it may be relied upon to have meaningful contents.
 */

// forward declarations
class XSNamespaceItem;
class XSModel;

class XMLPARSER_EXPORT XSObject : public XMemory
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor
      *
      * @param  compType
      * @param  xsModel
      * @param  manager     The configurable memory manager
      */
    XSObject
    (
        XSConstants::COMPONENT_TYPE compType
        , XSModel* const xsModel
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    //@};

    /** @name Destructor */
    //@{
    virtual ~XSObject();
    //@}

    //---------------------
    /** @name XSObject methods */

    //@{

    /**
     *  The <code>type</code> of this object, i.e.
     * <code>ELEMENT_DECLARATION</code>.
     */
    XSConstants::COMPONENT_TYPE getType() const;

    /**
     * The name of type <code>NCName</code> of this declaration as defined in
     * XML Namespaces.
     */
    virtual const XMLCh* getName() const;

    /**
     *  The [target namespace] of this object, or <code>null</code> if it is
     * unspecified.
     */
    virtual const XMLCh* getNamespace();

    /**
     * A namespace schema information item corresponding to the target
     * namespace of the component, if it's globally declared; or null
     * otherwise.
     */
    virtual XSNamespaceItem *getNamespaceItem();

    /**
      * Optional.  Return a unique identifier for a component within this XSModel, to
      * optimize querying.  May not be defined for all types of component.
      * @return id unique for this type of component within this XSModel or 0
      *     to indicate that this is not supported for this type of component.
      */
    virtual XMLSize_t getId() const;

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{
    /**
      * Set the id to be returned on getId().
      */
    void setId(XMLSize_t id);
    //@}

private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSObject(const XSObject&);
    XSObject & operator=(const XSObject &);

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    // fMemoryManager:
    //  used for any memory allocations
    // fComponentType
    //  the type of the actual component
    XSConstants::COMPONENT_TYPE fComponentType;
    XSModel*                    fXSModel;
    MemoryManager*              fMemoryManager;
    XMLSize_t                   fId;
};

inline XSConstants::COMPONENT_TYPE XSObject::getType() const
{
    return fComponentType;
}

XERCES_CPP_NAMESPACE_END

#endif
