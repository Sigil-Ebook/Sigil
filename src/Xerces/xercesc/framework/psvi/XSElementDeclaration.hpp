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
 * $Id: XSElementDeclaration.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSELEMENTDECLARATION_HPP)
#define XERCESC_INCLUDE_GUARD_XSELEMENTDECLARATION_HPP

#include <xercesc/framework/psvi/XSObject.hpp>
#include <xercesc/framework/psvi/XSNamedMap.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class describes all properties of a Schema Element Declaration
 * component.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  
 */

// forward declarations
class XSAnnotation;
class XSComplexTypeDefinition;
class XSIDCDefinition;
class XSTypeDefinition;
class SchemaElementDecl;

class XMLPARSER_EXPORT XSElementDeclaration : public XSObject
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  schemaElementDecl
      * @param  typeDefinition
      * @param  substitutionGroupAffiliation
      * @param  annot
      * @param  identityConstraints
      * @param  xsModel
      * @param  elemScope
      * @param  enclosingTypeDefinition
      * @param  manager     The configurable memory manager
      */
    XSElementDeclaration
    (
        SchemaElementDecl* const             schemaElementDecl
        , XSTypeDefinition* const            typeDefinition
        , XSElementDeclaration* const        substitutionGroupAffiliation
        , XSAnnotation* const                annot
        , XSNamedMap<XSIDCDefinition>* const identityConstraints
        , XSModel* const                     xsModel
        , XSConstants::SCOPE                 elemScope = XSConstants::SCOPE_ABSENT
        , XSComplexTypeDefinition* const     enclosingTypeDefinition = 0
        , MemoryManager* const               manager = XMLPlatformUtils::fgMemoryManager
    );

    //@};

    /** @name Destructor */
    //@{
    ~XSElementDeclaration();
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
    /** @name XSElementDeclaration methods */

    //@{

    /**
     * [type definition]: either a simple type definition or a complex type 
     * definition. 
     */
    XSTypeDefinition *getTypeDefinition() const;

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
    XSComplexTypeDefinition *getEnclosingCTDefinition() const;

    /**
     * [Value constraint]: one of <code>VC_NONE, VC_DEFAULT, VC_FIXED</code>. 
     */
    XSConstants::VALUE_CONSTRAINT getConstraintType() const;

    /**
     * [Value constraint]: the actual value with respect to the [type 
     * definition]. 
     */
    const XMLCh *getConstraintValue();

    /**
     * If nillable is true, then an element may also be valid if it carries 
     * the namespace qualified attribute with local name <code>nil</code> 
     * from namespace <code>http://www.w3.org/2001/XMLSchema-instance</code> 
     * and value <code>true</code> (xsi:nil) even if it has no text or 
     * element content despite a <code>content type</code> which would 
     * otherwise require content. 
     */
    bool getNillable() const;

    /**
     * identity-constraint definitions: a set of constraint definitions. 
     */
    XSNamedMap <XSIDCDefinition> *getIdentityConstraints();

    /**
     * [substitution group affiliation]: optional. A top-level element 
     * definition. 
     */
    XSElementDeclaration *getSubstitutionGroupAffiliation() const;

    /**
     * Convenience method. Check if <code>exclusion</code> is a substitution 
     * group exclusion for this element declaration. 
     * @param exclusion  
     *   <code>DERIVATION_EXTENSION, DERIVATION_RESTRICTION</code> or 
     *   <code>DERIVATION_NONE</code>. Represents final set for the element.
     * @return True if <code>exclusion</code> is a part of the substitution 
     *   group exclusion subset. 
     */
    bool isSubstitutionGroupExclusion(XSConstants::DERIVATION_TYPE exclusion);

    /**
     * [substitution group exclusions]: the returned value is a bit 
     * combination of the subset of {
     * <code>DERIVATION_EXTENSION, DERIVATION_RESTRICTION</code>} or 
     * <code>DERIVATION_NONE</code>. 
     */
    short getSubstitutionGroupExclusions() const;

    /**
     * Convenience method. Check if <code>disallowed</code> is a disallowed 
     * substitution for this element declaration. 
     * @param disallowed {
     *   <code>DERIVATION_SUBSTITUTION, DERIVATION_EXTENSION, DERIVATION_RESTRICTION</code>
     *   } or <code>DERIVATION_NONE</code>. Represents a block set for the 
     *   element.
     * @return True if <code>disallowed</code> is a part of the substitution 
     *   group exclusion subset. 
     */
    bool isDisallowedSubstitution(XSConstants::DERIVATION_TYPE disallowed);

    /**
     * [disallowed substitutions]: the returned value is a bit combination of 
     * the subset of {
     * <code>DERIVATION_SUBSTITUTION, DERIVATION_EXTENSION, DERIVATION_RESTRICTION</code>
     * } corresponding to substitutions disallowed by this 
     * <code>XSElementDeclaration</code> or <code>DERIVATION_NONE</code>. 
     */
    short getDisallowedSubstitutions() const;

    /**
     * {abstract} A boolean. 
     */
    bool getAbstract() const;

    /**
     * Optional. Annotation. 
     */
    XSAnnotation *getAnnotation() const;

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{

    void setTypeDefinition(XSTypeDefinition* typeDefinition);

    //@}
private:

    void setEnclosingCTDefinition(XSComplexTypeDefinition* const toSet);
    friend class XSObjectFactory;

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSElementDeclaration(const XSElementDeclaration&);
    XSElementDeclaration & operator=(const XSElementDeclaration &);

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    short                         fDisallowedSubstitutions;
    short                         fSubstitutionGroupExclusions;    
    XSConstants::SCOPE            fScope;
    SchemaElementDecl*            fSchemaElementDecl;
    XSTypeDefinition*             fTypeDefinition;
    XSComplexTypeDefinition*      fEnclosingTypeDefinition;
    XSElementDeclaration*         fSubstitutionGroupAffiliation;
    XSAnnotation*                 fAnnotation;
    XSNamedMap<XSIDCDefinition>*  fIdentityConstraints;
};

inline XSTypeDefinition* XSElementDeclaration::getTypeDefinition() const
{
    return fTypeDefinition;
}

inline XSNamedMap<XSIDCDefinition>* XSElementDeclaration::getIdentityConstraints()
{
    return fIdentityConstraints;
}

inline XSElementDeclaration* XSElementDeclaration::getSubstitutionGroupAffiliation() const
{
    return fSubstitutionGroupAffiliation;
}

inline short XSElementDeclaration::getSubstitutionGroupExclusions() const
{
    return fSubstitutionGroupExclusions;
}

inline short XSElementDeclaration::getDisallowedSubstitutions() const
{
    return fDisallowedSubstitutions;
}

inline XSAnnotation *XSElementDeclaration::getAnnotation() const
{
    return fAnnotation;
}

inline XSConstants::SCOPE XSElementDeclaration::getScope() const
{
    return fScope;
}

inline XSComplexTypeDefinition *XSElementDeclaration::getEnclosingCTDefinition() const
{
    return fEnclosingTypeDefinition;
}

inline void XSElementDeclaration::setTypeDefinition(XSTypeDefinition* typeDefinition)
{
    fTypeDefinition = typeDefinition;
}

inline void XSElementDeclaration::setEnclosingCTDefinition(XSComplexTypeDefinition* const toSet)
{
    fEnclosingTypeDefinition = toSet;
}

XERCES_CPP_NAMESPACE_END

#endif
