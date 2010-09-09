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
 * $Id: XSSimpleTypeDefinition.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSSIMPLETYPEDEFINITION_HPP)
#define XERCESC_INCLUDE_GUARD_XSSIMPLETYPEDEFINITION_HPP

#include <xercesc/framework/psvi/XSTypeDefinition.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class represents a simpleType definition
 * schema component.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  
 *
 */

// forward declarations
class XSAnnotation;
class XSFacet;
class XSMultiValueFacet;
class DatatypeValidator;

class XMLPARSER_EXPORT XSSimpleTypeDefinition : public XSTypeDefinition
{
public:

    // Variety definitions
    enum VARIETY {
	    /**
	     * The variety is absent for the anySimpleType definition.
	     */
	    VARIETY_ABSENT            = 0,
	    /**
	     * <code>Atomic</code> type.
	     */
	    VARIETY_ATOMIC            = 1,
	    /**
	     * <code>List</code> type.
	     */
	    VARIETY_LIST              = 2,
	    /**
	     * <code>Union</code> type.
	     */
	    VARIETY_UNION             = 3
    };

    // Facets
    enum FACET {
	    /**
	     * No facets defined.
	     */
	    FACET_NONE                = 0,
	    /**
	     * 4.3.1 Length
	     */
	    FACET_LENGTH              = 1,
	    /**
	     * 4.3.2 minLength. 
	     */
	    FACET_MINLENGTH           = 2,
	    /**
	     * 4.3.3 maxLength.
	     */
	    FACET_MAXLENGTH           = 4,
	    /**
	     * 4.3.4 pattern.
	     */
	    FACET_PATTERN             = 8,
	    /**
	     * 4.3.5 whitespace.
	     */
	    FACET_WHITESPACE          = 16,
	    /**
	     * 4.3.7 maxInclusive.
	     */
	    FACET_MAXINCLUSIVE        = 32,
	    /**
	     * 4.3.9 maxExclusive.
	     */
	    FACET_MAXEXCLUSIVE        = 64,
	    /**
	     * 4.3.9 minExclusive.
	     */
	    FACET_MINEXCLUSIVE        = 128,
	    /**
	     * 4.3.10 minInclusive.
	     */
	    FACET_MININCLUSIVE        = 256,
	    /**
	     * 4.3.11 totalDigits .
	     */
	    FACET_TOTALDIGITS         = 512,
	    /**
	     * 4.3.12 fractionDigits.
	     */
	    FACET_FRACTIONDIGITS      = 1024,
	    /**
	     * 4.3.5 enumeration.
	     */
	    FACET_ENUMERATION         = 2048
    };

    // possible order relations
    enum ORDERING {
	    /**
	     * A constant defined for the 'ordered' fundamental facet: Not ordered.
	     */
	    ORDERED_FALSE             = 0,
	    /**
	     * A constant defined for the 'ordered' fundamental facet: partially 
	     * ordered.
	     */
	    ORDERED_PARTIAL           = 1,
	    /**
	     * A constant defined for the 'ordered' fundamental facet: total ordered.
	     */
        ORDERED_TOTAL             = 2
    };

	//  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  datatypeValidator
      * @param  stVariety
      * @param  xsBaseType
      * @param  primitiveOrItemType
      * @param  memberTypes
      * @param  headAnnot
      * @param  xsModel
      * @param  manager     The configurable memory manager
      */
    XSSimpleTypeDefinition
    (
        DatatypeValidator* const            datatypeValidator
        , VARIETY                           stVariety
        , XSTypeDefinition* const           xsBaseType
        , XSSimpleTypeDefinition* const     primitiveOrItemType
        , XSSimpleTypeDefinitionList* const memberTypes
        , XSAnnotation*                     headAnnot
        , XSModel* const                    xsModel
        , MemoryManager* const              manager = XMLPlatformUtils::fgMemoryManager
    );

    //@};

    /** @name Destructor */
    //@{
    ~XSSimpleTypeDefinition();
    //@}

    //---------------------
    /** @name XSSimpleTypeDefinition methods */

    //@{

    /**
     * [variety]: one of {atomic, list, union} or absent 
     */
    VARIETY getVariety() const;

    /**
     * If variety is <code>atomic</code> the primitive type definition (a 
     * built-in primitive datatype definition or the simple ur-type 
     * definition) is available, otherwise <code>null</code>. 
     */
    XSSimpleTypeDefinition *getPrimitiveType();

    /**
     * If variety is <code>list</code> the item type definition (an atomic or 
     * union simple type definition) is available, otherwise 
     * <code>null</code>. 
     */
    XSSimpleTypeDefinition *getItemType();

    /**
     * If variety is <code>union</code> the list of member type definitions (a 
     * non-empty sequence of simple type definitions) is available, 
     * otherwise <code>null</code>. 
     */
    XSSimpleTypeDefinitionList *getMemberTypes() const;

    /**
     * [facets]: get all facets defined on this type. The value is a bit 
     * combination of FACET_XXX constants of all defined facets. 
     */
    int getDefinedFacets() const;

    /**
     * Convenience method. [Facets]: check whether a facet is defined on this 
     * type.
     * @param facetName  The name of the facet. 
     * @return  True if the facet is defined, false otherwise.
     */
    bool isDefinedFacet(FACET facetName);

    /**
     * [facets]: get all facets defined and fixed on this type.
     */
    int getFixedFacets() const;

    /**
     * Convenience method. [Facets]: check whether a facet is defined and 
     * fixed on this type. 
     * @param facetName  The name of the facet. 
     * @return  True if the facet is fixed, false otherwise.
     */
    bool isFixedFacet(FACET facetName);

    /**
     * Convenience method. Returns a value of a single constraining facet for 
     * this simple type definition. This method must not be used to retrieve 
     * values for <code>enumeration</code> and <code>pattern</code> facets. 
     * @param facetName The name of the facet, i.e. 
     *   <code>FACET_LENGTH, FACET_TOTALDIGITS </code> (see 
     *   <code>XSConstants</code>).To retrieve value for pattern or 
     *   enumeration, see <code>enumeration</code> and <code>pattern</code>.
     * @return A value of the facet specified in <code>facetName</code> for 
     *   this simple type definition or <code>null</code>. 
     */
    const XMLCh *getLexicalFacetValue(FACET facetName);

    /**
     * Returns a list of enumeration values. 
     */
    StringList *getLexicalEnumeration();

    /**
     * Returns a list of pattern values. 
     */
    StringList *getLexicalPattern();

    /**
     *  Fundamental Facet: ordered 
     */
    ORDERING getOrdered() const;

    /**
     * Fundamental Facet: cardinality. 
     */
    bool getFinite() const;

    /**
     * Fundamental Facet: bounded. 
     */
    bool getBounded() const;

    /**
     * Fundamental Facet: numeric. 
     */
    bool getNumeric() const;

    /**
     * Optional. A set of [annotation]s. 
     */
    XSAnnotationList *getAnnotations();
    /** 
     * @return list of constraining facets.
     * This method must not be used to retrieve 
     * values for <code>enumeration</code> and <code>pattern</code> facets.
     */
    XSFacetList *getFacets();
    
    /** 
     * @return list of enumeration and pattern facets.
     */
    XSMultiValueFacetList *getMultiValueFacets();
    
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

    /**
     *  A boolean that specifies if the type definition is 
     * anonymous. Convenience attribute. 
     */
    bool getAnonymous() const;

    /**
     * {base type definition}: either a simple type definition or a complex 
     * type definition. 
     */
    XSTypeDefinition *getBaseType();

    /**
     * Convenience method: check if this type is derived from the given 
     * <code>ancestorType</code>. 
     * @param ancestorType  An ancestor type definition. 
     * @return  Return true if this type is derived from 
     *   <code>ancestorType</code>.
     */
    bool derivedFromType(const XSTypeDefinition* const ancestorType);

    /**
     * 
     */
    inline DatatypeValidator* getDatatypeValidator() const;

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{


    //@}

private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSSimpleTypeDefinition(const XSSimpleTypeDefinition&);
    XSSimpleTypeDefinition & operator=(const XSSimpleTypeDefinition &);

    /**
      * Helper method for construct
      */
    void setFacetInfo
    (
        int                            definedFacets
        , int                          fixedFacets
        , XSFacetList* const           xsFacetList
        , XSMultiValueFacetList* const xsMultiValueFacetList
        , StringList* const            patternList
    );
    void setPrimitiveType(XSSimpleTypeDefinition*  const toSet);

    friend class XSObjectFactory;

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    int                         fDefinedFacets;
    int                         fFixedFacets;
    VARIETY                     fVariety;
    DatatypeValidator*          fDatatypeValidator;
    XSFacetList*                fXSFacetList;
    XSMultiValueFacetList*      fXSMultiValueFacetList;
    StringList*                 fPatternList;
    XSSimpleTypeDefinition*     fPrimitiveOrItemType;
    XSSimpleTypeDefinitionList* fMemberTypes;
    XSAnnotationList*           fXSAnnotationList;
};

inline XSSimpleTypeDefinition::VARIETY XSSimpleTypeDefinition::getVariety() const
{
    return fVariety;
}

inline XSSimpleTypeDefinition* XSSimpleTypeDefinition::getPrimitiveType()
{
    if (fVariety == VARIETY_ATOMIC)
        return fPrimitiveOrItemType;

    return 0;
}

inline XSSimpleTypeDefinition* XSSimpleTypeDefinition::getItemType()
{
    if (fVariety == VARIETY_LIST)
        return fPrimitiveOrItemType;

    return 0;
}

inline XSSimpleTypeDefinitionList* XSSimpleTypeDefinition::getMemberTypes() const
{
    return fMemberTypes;
}

inline int XSSimpleTypeDefinition::getDefinedFacets() const
{
    return fDefinedFacets;
}

inline int XSSimpleTypeDefinition::getFixedFacets() const
{
    return fFixedFacets;
}

inline StringList* XSSimpleTypeDefinition::getLexicalPattern()
{
    return fPatternList;
}

inline XSFacetList* XSSimpleTypeDefinition::getFacets()
{
    return fXSFacetList;
}

inline XSMultiValueFacetList* XSSimpleTypeDefinition::getMultiValueFacets()
{
    return fXSMultiValueFacetList;
}

inline XSAnnotationList *XSSimpleTypeDefinition::getAnnotations()
{
    return fXSAnnotationList;
}

inline void
XSSimpleTypeDefinition::setPrimitiveType(XSSimpleTypeDefinition* const toSet)
{
    fPrimitiveOrItemType = toSet;
}

inline DatatypeValidator* 
XSSimpleTypeDefinition::getDatatypeValidator() const
{
    return fDatatypeValidator;
}

XERCES_CPP_NAMESPACE_END

#endif
