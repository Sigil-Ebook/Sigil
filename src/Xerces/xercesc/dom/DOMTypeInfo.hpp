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

#if !defined(XERCESC_INCLUDE_GUARD_DOMTYPEINFO_HPP)
#define XERCESC_INCLUDE_GUARD_DOMTYPEINFO_HPP

//------------------------------------------------------------------------------------
//  Includes
//------------------------------------------------------------------------------------
#include <xercesc/util/XMLString.hpp>


XERCES_CPP_NAMESPACE_BEGIN

/**
  * The <code>DOMTypeInfo</code> interface represent a type used by
  * <code>DOMElement</code> or <code>DOMAttr</code> nodes, specified in the
  * schemas associated with the document. The type is a pair of a namespace URI
  * and name properties, and depends on the document's schema.
  */
class CDOM_EXPORT DOMTypeInfo
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMTypeInfo() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMTypeInfo(const DOMTypeInfo &);
    DOMTypeInfo & operator = (const DOMTypeInfo &);
    //@}

public:

    // -----------------------------------------------------------------------
    //  All constructors are hidden, just the destructor is available
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    /**
     * Destructor
     *
     */
    virtual ~DOMTypeInfo() {};
    //@}

    // -----------------------------------------------------------------------
    //  Class Types
    // -----------------------------------------------------------------------
    /** @name Public Contants */
    //@{
    /**
     * These are the available values for the derivationMethod parameter used by the
     * method <code>DOMTypeInfo::isDerivedFrom()</code>. It is a set of possible types
     * of derivation, and the values represent bit positions. If a bit in the derivationMethod
     * parameter is set to 1, the corresponding type of derivation will be taken into account
     * when evaluating the derivation between the reference type definition and the other type
     * definition. When using the isDerivedFrom method, combining all of them in the
     * derivationMethod parameter is equivalent to invoking the method for each of them separately
     * and combining the results with the OR boolean function. This specification only defines
     * the type of derivation for XML Schema.
     *
     * In addition to the types of derivation listed below, please note that:
     *  - any type derives from xsd:anyType.
     *  - any simple type derives from xsd:anySimpleType by restriction.
     *  - any complex type does not derive from xsd:anySimpleType by restriction.
     *
     * <p><code>DERIVATION_EXTENSION:</code>
     * If the document's schema is an XML Schema [XML Schema Part 1], this constant represents the
     * derivation by extension. The reference type definition is derived by extension from the other
     * type definition if the other type definition can be reached recursively following the
     * {base type definition} property from the reference type definition, and at least one of the
     * derivation methods involved is an extension.</p>
     *
     * <p><code>DERIVATION_LIST:</code>
     * If the document's schema is an XML Schema [XML Schema Part 1], this constant represents the list.
     * The reference type definition is derived by list from the other type definition if there exists
     * two type definitions T1 and T2 such as the reference type definition is derived from T1 by
     * DERIVATION_RESTRICTION or DERIVATION_EXTENSION, T2 is derived from the other type definition by
     * DERIVATION_RESTRICTION, T1 has {variety} list, and T2 is the {item type definition}. Note that
     * T1 could be the same as the reference type definition, and T2 could be the same as the other
     * type definition.</p>
     *
     * <p><code>DERIVATION_RESTRICTION:</code>
     * If the document's schema is an XML Schema [XML Schema Part 1], this constant represents the
     * derivation by restriction if complex types are involved, or a restriction if simple types are
     * involved.
     * The reference type definition is derived by restriction from the other type definition if the
     * other type definition is the same as the reference type definition, or if the other type definition
     * can be reached recursively following the {base type definition} property from the reference type
     * definition, and all the derivation methods involved are restriction.</p>
     *
     * <p><code>DERIVATION_UNION:</code>
     * If the document's schema is an XML Schema [XML Schema Part 1], this constant represents the union
     * if simple types are involved.
     * The reference type definition is derived by union from the other type definition if there exists
     * two type definitions T1 and T2 such as the reference type definition is derived from T1 by
     * DERIVATION_RESTRICTION or DERIVATION_EXTENSION, T2 is derived from the other type definition by
     * DERIVATION_RESTRICTION, T1 has {variety} union, and one of the {member type definitions} is T2.
     * Note that T1 could be the same as the reference type definition, and T2 could be the same as the
     * other type definition.</p>
     *
     * @since DOM Level 3
     *
     */
    enum DerivationMethods {
         DERIVATION_RESTRICTION = 0x001,
         DERIVATION_EXTENSION   = 0x002,
         DERIVATION_UNION       = 0x004,
         DERIVATION_LIST        = 0x008
        };
    //@}

    //@{
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    /**
     * Returns The name of a type declared for the associated <code>DOMElement</code>
     * or <code>DOMAttr</code>, or null if unknown.
     *
     * @return The name of a type declared for the associated <code>DOMElement</code>
     * or <code>DOMAttribute</code>, or null if unknown.
     * @since DOM level 3
     */
    virtual const XMLCh* getTypeName() const = 0;

    /**
     * The namespace of the type declared for the associated <code>DOMElement</code>
     * or <code>DOMAttr</code> or null if the <code>DOMElement</code> does not have
     * declaration or if no namespace information is available.
     *
     * @return The namespace of the type declared for the associated <code>DOMElement</code>
     * or <code>DOMAttr</code> or null if the <code>DOMElement</code> does not have
     * declaration or if no namespace information is available.
     * @since DOM level 3
     */
    virtual const XMLCh* getTypeNamespace() const = 0;
    //@}

    //@{
    /**
     * This method returns if there is a derivation between the reference type definition,
     * i.e. the DOMTypeInfo on which the method is being called, and the other type definition,
     * i.e. the one passed as parameters.
     *
     * @param typeNamespaceArg The namespace of the other type definition.
     * @param typeNameArg The name of the other type definition.
     * @param derivationMethod The type of derivation and conditions applied between two types,
     *                         as described in the list of constants provided in this interface.
     * @return If the document's schema is a DTD or no schema is associated with the document,
     *         this method will always return false.
     *         If the document's schema is an XML Schema, the method will true if the reference
     *         type definition is derived from the other type definition according to the derivation
     *         parameter. If the value of the parameter is 0 (no bit is set to 1 for the
     *         derivationMethod parameter), the method will return true if the other type definition
     *         can be reached by recursing any combination of {base type definition},
     *         {item type definition}, or {member type definitions} from the reference type definition.
     * @since DOM level 3
     */
    virtual bool isDerivedFrom(const XMLCh* typeNamespaceArg,
                               const XMLCh* typeNameArg,
                               DerivationMethods derivationMethod) const = 0;
    //@}
};

XERCES_CPP_NAMESPACE_END

#endif

/**
 * End of file DOMTypeInfo.hpp
 */
