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
 * $Id: XSNamespaceItem.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSNAMESPACEITEM_HPP)
#define XERCESC_INCLUDE_GUARD_XSNAMESPACEITEM_HPP

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/psvi/XSObject.hpp>
#include <xercesc/framework/psvi/XSNamedMap.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class contains all properties of the Schema Namespace Information infoitem.  
 * These items correspond to the result of processing a schema document
 * and all its included/redefined schema documents.  It corresponds to the
 * schema component discussed in the schema specifications, but since it
 * is not like other components does not inherit from the XSObject interface.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  It is designed to be subclassed; subclasses will
 * specify under what conditions it may be relied upon to have meaningful contents.
 */

// forward declarations
class XSAnnotation;
class XSAttributeDeclaration;
class XSAttributeGroupDefinition;
class XSElementDeclaration;
class XSModelGroupDefinition;
class XSNotationDeclaration;
class XSTypeDefinition;
class SchemaGrammar;
class XSModel;

class XMLPARSER_EXPORT XSNamespaceItem : public XMemory
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor 
      *
      * @param  xsModel
      * @param  grammar
      * @param  manager     The configurable memory manager
      */
    XSNamespaceItem
    (
        XSModel* const         xsModel
        , SchemaGrammar* const grammar
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    XSNamespaceItem
    (
        XSModel* const         xsModel
        , const XMLCh* const   schemaNamespace
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    //@};

    /** @name Destructor */
    //@{
    ~XSNamespaceItem();
    //@}

    //---------------------
    /** @name XSNamespaceItem methods */

    //@{

    /**
     * [schema namespace]: A namespace name or <code>null</code>
     * corresponding to the target namespace of the schema document.
     */
    const XMLCh *getSchemaNamespace() const;

    /**
     * [schema components]: a list of top-level components, i.e. element 
     * declarations, attribute declarations, etc. 
     * @param objectType The type of the declaration, i.e. 
     *   <code>ELEMENT_DECLARATION</code>, 
     *   <code>TYPE_DEFINITION</code> and any other component type that
     * may be a property of a schema component.
     * @return A list of top-level definition of the specified type in 
     *   <code>objectType</code> or <code>null</code>. 
     */
    XSNamedMap<XSObject> *getComponents(XSConstants::COMPONENT_TYPE objectType);

    /**
     *  [annotations]: a set of annotations.
     */
    XSAnnotationList *getAnnotations();
    const XSAnnotationList *getAnnotations() const;

    /**
     * Convenience method. Returns a top-level element declaration. 
     * @param name The name of the declaration.
     * @return A top-level element declaration or <code>null</code> if such 
     *   declaration does not exist. 
     */
    XSElementDeclaration *getElementDeclaration(const XMLCh *name);

    /**
     * Convenience method. Returns a top-level attribute declaration. 
     * @param name The name of the declaration.
     * @return A top-level attribute declaration or <code>null</code> if such 
     *   declaration does not exist. 
     */
    XSAttributeDeclaration *getAttributeDeclaration(const XMLCh *name);

    /**
     * Convenience method. Returns a top-level simple or complex type 
     * definition. 
     * @param name The name of the definition.
     * @return An <code>XSTypeDefinition</code> or <code>null</code> if such 
     *   definition does not exist. 
     */
    XSTypeDefinition *getTypeDefinition(const XMLCh *name);

    /**
     * Convenience method. Returns a top-level attribute group definition. 
     * @param name The name of the definition.
     * @return A top-level attribute group definition or <code>null</code> if 
     *   such definition does not exist. 
     */
    XSAttributeGroupDefinition *getAttributeGroup(const XMLCh *name);

    /**
     * Convenience method. Returns a top-level model group definition. 
     * @param name The name of the definition.
     * @return A top-level model group definition definition or 
     *   <code>null</code> if such definition does not exist. 
     */
    XSModelGroupDefinition *getModelGroupDefinition(const XMLCh *name);

    /**
     * Convenience method. Returns a top-level notation declaration. 
     * @param name The name of the declaration.
     * @return A top-level notation declaration or <code>null</code> if such 
     *   declaration does not exist. 
     */
    XSNotationDeclaration *getNotationDeclaration(const XMLCh *name);

    /**
     * [document location] - a list of locations URI for the documents that 
     * contributed to the XSModel.
     */
    const StringList *getDocumentLocations();

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{


    //@}
private:

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSNamespaceItem(const XSNamespaceItem&);
    XSNamespaceItem & operator=(const XSNamespaceItem &);

protected:
    friend class XSModel;
    friend class XSObjectFactory;
    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    // fMemoryManager:
    //  used for any memory allocations
    MemoryManager* const    fMemoryManager;
    SchemaGrammar*          fGrammar;
    XSModel*                fXSModel;

    /* Need a XSNamedMap for each component    top-level?
       that is top level.
	      ATTRIBUTE_DECLARATION     = 1,	   
	      ELEMENT_DECLARATION       = 2,	    
	      TYPE_DEFINITION           = 3,	    
	      ATTRIBUTE_USE             = 4,	   no 
	      ATTRIBUTE_GROUP_DEFINITION= 5,	    
	      MODEL_GROUP_DEFINITION    = 6,	   
	      MODEL_GROUP               = 7,	   no 
	      PARTICLE                  = 8,	   no
	      WILDCARD                  = 9,	   no
	      IDENTITY_CONSTRAINT       = 10,	   no
	      NOTATION_DECLARATION      = 11,	    
	      ANNOTATION                = 12,	   no
	      FACET                     = 13,      no
	      MULTIVALUE_FACET          = 14       no
    */
    XSNamedMap<XSObject>*                   fComponentMap[XSConstants::MULTIVALUE_FACET];
    XSAnnotationList*                       fXSAnnotationList;
    RefHashTableOf<XSObject>*               fHashMap[XSConstants::MULTIVALUE_FACET];
    const XMLCh*                            fSchemaNamespace;
};

inline XSAnnotationList* XSNamespaceItem::getAnnotations()
{
    return fXSAnnotationList;
}

inline const XSAnnotationList* XSNamespaceItem::getAnnotations() const
{
    return fXSAnnotationList;
}

inline const XMLCh *XSNamespaceItem::getSchemaNamespace() const
{
    return fSchemaNamespace;
}



XERCES_CPP_NAMESPACE_END

#endif
