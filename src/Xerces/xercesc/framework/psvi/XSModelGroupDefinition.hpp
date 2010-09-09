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
 * $Id: XSModelGroupDefinition.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSMODELGROUPDEFINITION_HPP)
#define XERCESC_INCLUDE_GUARD_XSMODELGROUPDEFINITION_HPP

#include <xercesc/framework/psvi/XSObject.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class describes all properties of a Schema Model Group
 * Definition component.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  
 */

// forward declarations
class XSAnnotation;
class XSModelGroup;
class XSParticle;
class XercesGroupInfo;

class XMLPARSER_EXPORT XSModelGroupDefinition : public XSObject
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  groupInfo
      * @param  groupParticle
      * @param  annot
      * @param  xsModel
      * @param  manager     The configurable memory manager
      */
    XSModelGroupDefinition
    (
        XercesGroupInfo* const groupInfo
        , XSParticle* const    groupParticle
        , XSAnnotation* const  annot
        , XSModel* const       xsModel
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    //@};

    /** @name Destructor */
    //@{
    ~XSModelGroupDefinition();
    //@}

    //---------------------
    /** @name overridden XSXSObject methods */
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
    XSNamespaceItem *getNamespaceItem();

    //@}

    //---------------------
    /** @name XSModelGroupDefinition methods */

    //@{

    /**
     * A model group. 
     */
    XSModelGroup *getModelGroup();

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
    XSModelGroupDefinition(const XSModelGroupDefinition&);
    XSModelGroupDefinition & operator=(const XSModelGroupDefinition &);

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    XercesGroupInfo* fGroupInfo;
    XSParticle*      fModelGroupParticle;
    XSAnnotation*    fAnnotation;
};

inline XSAnnotation* XSModelGroupDefinition::getAnnotation() const
{
    return fAnnotation;
}



XERCES_CPP_NAMESPACE_END

#endif
