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
 * $Id: XSConstants.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSCONSTANTS_HPP)
#define XERCESC_INCLUDE_GUARD_XSCONSTANTS_HPP

#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/util/RefArrayVectorOf.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This contains constants needed in the schema component model.
 */

// forward definitions for typedefs
class XSAnnotation;
class XSAttributeUse;
class XSFacet;
class XSMultiValueFacet;
class XSNamespaceItem;
class XSParticle;
class XSSimpleTypeDefinition;

// these typedefs are intended to help hide dependence on utility
// classes, as well as to define more intuitive names for commonly
// used concepts.

typedef RefVectorOf <XSAnnotation> XSAnnotationList;
typedef RefVectorOf <XSAttributeUse> XSAttributeUseList;
typedef RefVectorOf <XSFacet> XSFacetList;
typedef RefVectorOf <XSMultiValueFacet> XSMultiValueFacetList;
typedef RefVectorOf <XSNamespaceItem> XSNamespaceItemList;
typedef RefVectorOf <XSParticle> XSParticleList;
typedef RefVectorOf <XSSimpleTypeDefinition> XSSimpleTypeDefinitionList;
typedef RefArrayVectorOf <XMLCh> StringList;

class XMLPARSER_EXPORT XSConstants 
{
public:

    // XML Schema Components
    enum COMPONENT_TYPE {
	    /**
	     * The object describes an attribute declaration.
	     */
	      ATTRIBUTE_DECLARATION     = 1,
	    /**
	     * The object describes an element declaration.
	     */
	      ELEMENT_DECLARATION       = 2,
	    /**
	     * The object describes a complex type or simple type definition.
	     */
	      TYPE_DEFINITION           = 3,
	    /**
	     * The object describes an attribute use definition.
	     */
	      ATTRIBUTE_USE             = 4,
	    /**
	     * The object describes an attribute group definition.
	     */
	      ATTRIBUTE_GROUP_DEFINITION= 5,
	    /**
	     * The object describes a model group definition.
	     */
	      MODEL_GROUP_DEFINITION    = 6,
	    /**
	     * A model group.
	     */
	      MODEL_GROUP               = 7,
	    /**
	     * The object describes a particle.
	     */
	      PARTICLE                  = 8,
	    /**
	     * The object describes a wildcard.
	     */
	      WILDCARD                  = 9,
	    /**
	     * The object describes an identity constraint definition.
	     */
	      IDENTITY_CONSTRAINT       = 10,
	    /**
	     * The object describes a notation declaration.
	     */
	      NOTATION_DECLARATION      = 11,
	    /**
	     * The object describes an annotation.
	     */
	      ANNOTATION                = 12,
	    /**
	     * The object describes a constraining facet.
	     */
	      FACET                     = 13,
	    
	    /**
	     * The object describes enumeration/pattern facets.
	     */
	      MULTIVALUE_FACET           = 14
    };

    // Derivation constants
    enum DERIVATION_TYPE {
	    /**
	     * No constraint is available.
	     */
	     DERIVATION_NONE     = 0,
	    /**
	     * <code>XSTypeDefinition</code> final set or 
	     * <code>XSElementDeclaration</code> disallowed substitution group.
	     */
	     DERIVATION_EXTENSION      = 1,
	    /**
	     * <code>XSTypeDefinition</code> final set or 
	     * <code>XSElementDeclaration</code> disallowed substitution group.
	     */
	     DERIVATION_RESTRICTION    = 2,
	    /**
	     * <code>XSTypeDefinition</code> final set.
	     */
	     DERIVATION_SUBSTITUTION   = 4,
	    /**
	     * <code>XSTypeDefinition</code> final set.
	     */
	     DERIVATION_UNION          = 8,
	    /**
	     * <code>XSTypeDefinition</code> final set.
	     */
	     DERIVATION_LIST           = 16
    };

    // Scope
    enum SCOPE {
	    /**
	     * The scope of a declaration within named model groups or attribute 
	     * groups is <code>absent</code>. The scope of such declaration is 
	     * determined when it is used in the construction of complex type 
	     * definitions. 
	     */
	     SCOPE_ABSENT              = 0,
	    /**
	     * A scope of <code>global</code> identifies top-level declarations. 
	     */
	     SCOPE_GLOBAL              = 1,
	    /**
	     * <code>Locally scoped</code> declarations are available for use only 
	     * within the complex type.
	     */
	     SCOPE_LOCAL               = 2
    };

    // Value Constraint
    enum VALUE_CONSTRAINT {
	    /**
	     * Indicates that the component does not have any value constraint.
	     */
	     VALUE_CONSTRAINT_NONE          = 0,
	    /**
	     * Indicates that there is a default value constraint.
	     */
	     VALUE_CONSTRAINT_DEFAULT       = 1,
	    /**
	     * Indicates that there is a fixed value constraint for this attribute.
	     */
	     VALUE_CONSTRAINT_FIXED         = 2
    };

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSConstants();
};

XERCES_CPP_NAMESPACE_END

#endif
