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
 * $Id: XSAttributeGroupDefinition.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSATTRIBUTEGROUPDEFINITION_HPP)
#define XERCESC_INCLUDE_GUARD_XSATTRIBUTEGROUPDEFINITION_HPP

#include <xercesc/framework/psvi/XSObject.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class describes all properties of a Schema Attribute
 * Group Definition component.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  
 */

// forward declarations
class XSAnnotation;
class XSAttributeUse;
class XSWildcard;
class XercesAttGroupInfo;

class XMLPARSER_EXPORT XSAttributeGroupDefinition : public XSObject
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  xercesAttGroupInfo
      * @param  xsAttList
      * @param  xsWildcard
      * @param  xsAnnot
      * @param  xsModel
      * @param  manager     The configurable memory manager
      */
    XSAttributeGroupDefinition
    (
        XercesAttGroupInfo* const   xercesAttGroupInfo
        , XSAttributeUseList* const xsAttList
        , XSWildcard* const         xsWildcard
        , XSAnnotation* const       xsAnnot
        , XSModel* const            xsModel
        , MemoryManager* const      manager = XMLPlatformUtils::fgMemoryManager
    );

    //@};

    /** @name Destructor */
    //@{
    ~XSAttributeGroupDefinition();
    //@}

    //---------------------
    /** @name overridden XSObject methods */
    //@{

    /**
     * The name of type <code>NCName</code> of this declaration as defined in 
     * XML Namespaces.
     */
    const XMLCh* getName() const;

    /**
     *  The [target namespace] of this object, or <code>null</code> if it is 
     * unspecified. 
     */
    const XMLCh* getNamespace();

    /**
     * A namespace schema information item corresponding to the target 
     * namespace of the component, if it's globally declared; or null 
     * otherwise.
     */
    XSNamespaceItem* getNamespaceItem();

    //@}

    //--------------------- 
    /** @name XSAttributeGroupDefinition methods */

    //@{

    /**
     * A set of [attribute uses]. 
     */
    XSAttributeUseList *getAttributeUses();

    /**
     * Optional. A [wildcard]. 
     */
    XSWildcard *getAttributeWildcard() const;

    /**
     * Optional. An [annotation]. 
     */
    XSAnnotation *getAnnotation() const;

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{

    //@}
private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSAttributeGroupDefinition(const XSAttributeGroupDefinition&);
    XSAttributeGroupDefinition & operator=(const XSAttributeGroupDefinition &);

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    XercesAttGroupInfo*     fXercesAttGroupInfo;
    XSAttributeUseList*     fXSAttributeUseList;
    XSWildcard*             fXSWildcard;
    XSAnnotation*           fAnnotation;
};

inline XSAttributeUseList* XSAttributeGroupDefinition::getAttributeUses()
{
    return fXSAttributeUseList;
}

inline XSWildcard* XSAttributeGroupDefinition::getAttributeWildcard() const
{
    return fXSWildcard;
}

inline XSAnnotation* XSAttributeGroupDefinition::getAnnotation() const
{
    return fAnnotation;
}

XERCES_CPP_NAMESPACE_END

#endif
