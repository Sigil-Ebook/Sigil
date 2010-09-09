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
 * $Id: XSMultiValueFacet.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSMULTIVALUEFACET_HPP)
#define XERCESC_INCLUDE_GUARD_XSMULTIVALUEFACET_HPP

#include <xercesc/framework/psvi/XSObject.hpp>
#include <xercesc/framework/psvi/XSSimpleTypeDefinition.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class represents all Schema Facets which may possess multiple
 * lexical values/annotations (i.e., Pattern and Enumeration facets).
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  
 */

// forward declarations
class XSAnnotation;

class XMLPARSER_EXPORT XSMultiValueFacet : public XSObject
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  facetKind
      * @param  lexicalValues
      * @param  isFixed
      * @param  headAnnot
      * @param  xsModel
      * @param  manager     The configurable memory manager
      */
    XSMultiValueFacet
    (
        XSSimpleTypeDefinition::FACET facetKind
        , StringList*                 lexicalValues
        , bool                        isFixed
        , XSAnnotation* const         headAnnot
        , XSModel* const              xsModel
        , MemoryManager* const        manager = XMLPlatformUtils::fgMemoryManager
    );

    //@};

    /** @name Destructor */
    //@{
    ~XSMultiValueFacet();
    //@}

    //---------------------
    /** @name XSMultiValueFacet methods */

    //@{

    /**
     * @return An indication as to the facet's type; see <code>XSSimpleTypeDefinition::FACET</code>
     */
    XSSimpleTypeDefinition::FACET getFacetKind() const;

    /**
     * @return Returns the values of a constraining facet. 
     */
    StringList *getLexicalFacetValues();   

    /**
     * Check whether a facet value is fixed. 
     */
    bool isFixed() const;

    /**
     * @return the annotations belonging to this facet's values
     */
    XSAnnotationList *getAnnotations();

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{

    //@}
private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSMultiValueFacet(const XSMultiValueFacet&);
    XSMultiValueFacet & operator=(const XSMultiValueFacet &);

protected:

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    XSSimpleTypeDefinition::FACET fFacetKind;
    bool                          fIsFixed;
    StringList*                   fLexicalValues;  // not owned by this class
    XSAnnotationList*             fXSAnnotationList;
};


inline XSSimpleTypeDefinition::FACET XSMultiValueFacet::getFacetKind() const
{
    return fFacetKind;
}

inline bool XSMultiValueFacet::isFixed() const
{
    return fIsFixed;
}

inline StringList *XSMultiValueFacet::getLexicalFacetValues()
{
    return fLexicalValues; 
}

inline XSAnnotationList *XSMultiValueFacet::getAnnotations()
{
    return fXSAnnotationList;
}


XERCES_CPP_NAMESPACE_END

#endif
