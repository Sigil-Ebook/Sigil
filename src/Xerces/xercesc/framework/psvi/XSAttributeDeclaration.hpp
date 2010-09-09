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
 * $Id: XSAttributeDeclaration.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSATTRIBUTEDECLARATION_HPP)
#define XERCESC_INCLUDE_GUARD_XSATTRIBUTEDECLARATION_HPP

#include <xercesc/framework/psvi/XSObject.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class describes all properties of a Schema Attribute
 * Declaration component.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  
 */

// forward declarations
class XSAnnotation;
class XSComplexTypeDefinition;
class XSSimpleTypeDefinition;
class SchemaAttDef;

class XMLPARSER_EXPORT XSAttributeDeclaration : public XSObject
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  attDef      
      * @param  typeDef     
      * @param  annot       
      * @param  xsModel     
      * @param  scope       
      * @param  enclosingCTDefinition
      * @param  manager     The configurable memory manager
      */
    XSAttributeDeclaration
    (
        SchemaAttDef* const             attDef
        , XSSimpleTypeDefinition* const typeDef
        , XSAnnotation* const           annot
        , XSModel* const                xsModel
        , XSConstants::SCOPE            scope
        , XSComplexTypeDefinition*      enclosingCTDefinition
        , MemoryManager* const          manager = XMLPlatformUtils::fgMemoryManager
    );

    //@};

    /** @name Destructor */
    //@{
    ~XSAttributeDeclaration();
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

    /** @name XSAttributeDeclaration methods **/

    //@{

    /**
     * [type definition]: A simple type definition 
     */
    XSSimpleTypeDefinition *getTypeDefinition() const;

    /**
     * Optional. One of <code>SCOPE_GLOBAL</code>, <code>SCOPE_LOCAL</code>, 
     * or <code>SCOPE_ABSENT</code>. If the scope is local, then the 
     * <code>enclosingCTDefinition</code> is present. 
     */
    XSConstants::SCOPE getScope() const;

    /**
     * The complex type definition for locally scoped declarations (see 
     * <code>scope</code>). 
     */
    XSComplexTypeDefinition *getEnclosingCTDefinition();

    /**
     * Value constraint: one of <code>VC_NONE, VC_DEFAULT, VC_FIXED</code>. 
     */
    XSConstants::VALUE_CONSTRAINT getConstraintType() const;

    /**
     * Value constraint: The actual value with respect to the [type definition
     * ]. 
     */
    const XMLCh *getConstraintValue();

    /**
     * Optional. Annotation. 
     */
    XSAnnotation *getAnnotation() const;

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{

    bool getRequired() const;
    //@}

private:

    void setEnclosingCTDefinition(XSComplexTypeDefinition* const toSet);
    friend class XSObjectFactory;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSAttributeDeclaration(const XSAttributeDeclaration&);
    XSAttributeDeclaration & operator=(const XSAttributeDeclaration &);

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    SchemaAttDef*               fAttDef;
    XSSimpleTypeDefinition*     fTypeDefinition;
    XSAnnotation*               fAnnotation;
    XSConstants::SCOPE          fScope;
    XSComplexTypeDefinition*    fEnclosingCTDefinition;
};

// ---------------------------------------------------------------------------
//  XSAttributeDeclaration: inline methods
// ---------------------------------------------------------------------------
inline XSSimpleTypeDefinition* XSAttributeDeclaration::getTypeDefinition() const
{
    return fTypeDefinition;
}

inline XSAnnotation *XSAttributeDeclaration::getAnnotation() const
{
    return fAnnotation;
}

inline XSConstants::SCOPE XSAttributeDeclaration::getScope() const
{   
    return fScope;
}

inline XSComplexTypeDefinition *XSAttributeDeclaration::getEnclosingCTDefinition()
{
    return fEnclosingCTDefinition;
}

inline void XSAttributeDeclaration::setEnclosingCTDefinition
(
    XSComplexTypeDefinition* const toSet
)
{
    fEnclosingCTDefinition = toSet;
}

XERCES_CPP_NAMESPACE_END

#endif
