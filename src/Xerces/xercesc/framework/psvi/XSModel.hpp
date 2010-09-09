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
 * $Id: XSModel.hpp 674012 2008-07-04 11:18:21Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSMODEL_HPP)
#define XERCESC_INCLUDE_GUARD_XSMODEL_HPP

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/psvi/XSObject.hpp>
#include <xercesc/framework/psvi/XSNamedMap.hpp>

#include <xercesc/util/ValueVectorOf.hpp>
#include <xercesc/validators/schema/SchemaElementDecl.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 * This class contains all properties of the Schema infoitem as determined
 * after an entire validation episode.  That is, it contains all the properties
 * of all the Schema Namespace Information objects that went into
 * the validation episode.
 * Since it is not like other components, it  does not
 * inherit from the XSObject interface.
 * This is *always* owned by the validator /parser object from which
 * it is obtained.  It is designed to be subclassed; subclasses will
 * specify under what conditions it may be relied upon to have meaningful contents.
 */

// forward declarations
class Grammar;
class XMLGrammarPool;
class XSAnnotation;
class XSAttributeDeclaration;
class XSAttributeGroupDefinition;
class XSElementDeclaration;
class XSModelGroupDefinition;
class XSNamespaceItem;
class XSNotationDeclaration;
class XSTypeDefinition;
class XSObjectFactory;

class XMLPARSER_EXPORT XSModel : public XMemory
{
public:

    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The constructor to be used when a grammar pool contains all needed info
      * @param grammarPool  the grammar pool containing the underlying data structures
      * @param manager      The configurable memory manager
      */
    XSModel( XMLGrammarPool *grammarPool
                , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /**
      * The constructor to be used when the XSModel must represent all
      * components in the union of an existing XSModel and a newly-created
      * Grammar(s) from the GrammarResolver
      *
      * @param baseModel  the XSModel upon which this one is based
      * @param grammarResolver  the grammar(s) whose components are to be merged
      * @param manager     The configurable memory manager
      */
    XSModel( XSModel *baseModel
                , GrammarResolver *grammarResolver
                , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    //@};

    /** @name Destructor */
    //@{
    ~XSModel();
    //@}

    //---------------------
    /** @name XSModel methods */

    //@{

    /**
     * Convenience method. Returns a list of all namespaces that belong to
     * this schema. The value <code>null</code> is not a valid namespace
     * name, but if there are components that don't have a target namespace,
     * <code>null</code> is included in this list.
     */
    StringList *getNamespaces();

    /**
     * A set of namespace schema information information items ( of type
     * <code>XSNamespaceItem</code>), one for each namespace name which
     * appears as the target namespace of any schema component in the schema
     * used for that assessment, and one for absent if any schema component
     * in the schema had no target namespace. For more information see
     * schema information.
     */
    XSNamespaceItemList *getNamespaceItems();

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
     * Convenience method. Returns a list of top-level component declarations
     * that are defined within the specified namespace, i.e. element
     * declarations, attribute declarations, etc.
     * @param objectType The type of the declaration, i.e.
     *   <code>ELEMENT_DECLARATION</code>.
     * @param compNamespace The namespace to which declaration belongs or
     *   <code>null</code> (for components with no target namespace).
     * @return A list of top-level definitions of the specified type in
     *   <code>objectType</code> and defined in the specified
     *   <code>namespace</code> or <code>null</code>.
     */
    XSNamedMap<XSObject> *getComponentsByNamespace(XSConstants::COMPONENT_TYPE objectType,
                                               const XMLCh *compNamespace);

    /**
     *  [annotations]: a set of annotations.
     */
    XSAnnotationList *getAnnotations();

    /**
     * Convenience method. Returns a top-level element declaration.
     * @param name The name of the declaration.
     * @param compNamespace The namespace of the declaration, null if absent.
     * @return A top-level element declaration or <code>null</code> if such
     *   declaration does not exist.
     */
    XSElementDeclaration *getElementDeclaration(const XMLCh *name
            , const XMLCh *compNamespace);

    /**
     * Convenience method. Returns a top-level attribute declaration.
     * @param name The name of the declaration.
     * @param compNamespace The namespace of the declaration, null if absent.
     * @return A top-level attribute declaration or <code>null</code> if such
     *   declaration does not exist.
     */
    XSAttributeDeclaration *getAttributeDeclaration(const XMLCh *name
            , const XMLCh *compNamespace);

    /**
     * Convenience method. Returns a top-level simple or complex type
     * definition.
     * @param name The name of the definition.
     * @param compNamespace The namespace of the declaration, null if absent.
     * @return An <code>XSTypeDefinition</code> or <code>null</code> if such
     *   definition does not exist.
     */
    XSTypeDefinition *getTypeDefinition(const XMLCh *name
            , const XMLCh *compNamespace);

    /**
     * Convenience method. Returns a top-level attribute group definition.
     * @param name The name of the definition.
     * @param compNamespace The namespace of the declaration, null if absent.
     * @return A top-level attribute group definition or <code>null</code> if
     *   such definition does not exist.
     */
    XSAttributeGroupDefinition *getAttributeGroup(const XMLCh *name
            , const XMLCh *compNamespace);

    /**
     * Convenience method. Returns a top-level model group definition.
     * @param name The name of the definition.
     * @param compNamespace The namespace of the declaration, null if absent.
     * @return A top-level model group definition definition or
     *   <code>null</code> if such definition does not exist.
     */
    XSModelGroupDefinition *getModelGroupDefinition(const XMLCh *name
            , const XMLCh *compNamespace);

    /**
     * Convenience method. Returns a top-level notation declaration.
     * @param name The name of the declaration.
     * @param compNamespace The namespace of the declaration, null if absent.
     * @return A top-level notation declaration or <code>null</code> if such
     *   declaration does not exist.
     */
    XSNotationDeclaration *getNotationDeclaration(const XMLCh *name
            , const XMLCh *compNamespace);

    /**
      * Optional.  Return a component given a component type and a unique Id.
      * May not be supported for all component types.
      * @param compId unique Id of the component within its type
      * @param compType type of the component
      * @return the component of the given type with the given Id, or 0
      * if no such component exists or this is unsupported for
      * this type of component.
      */
    XSObject *getXSObjectById(XMLSize_t compId,
                              XSConstants::COMPONENT_TYPE compType);

    //@}

    //----------------------------------
    /** methods needed by implementation */

    //@{
    XMLStringPool*  getURIStringPool();

    XSNamespaceItem* getNamespaceItem(const XMLCh* const key);

    /**
      * Get the XSObject (i.e. XSElementDeclaration) that corresponds to
      * to a schema grammar component (i.e. SchemaElementDecl)
      * @param key schema component object
      *
      * @return the corresponding XSObject
      */
    XSObject* getXSObject(void* key);

    //@}
private:

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------
    void addGrammarToXSModel
    (
        XSNamespaceItem* namespaceItem
    );
    void addS4SToXSModel
    (
        XSNamespaceItem* const namespaceItem
        , RefHashTableOf<DatatypeValidator>* const builtInDV
    );
    void addComponentToNamespace
    (
         XSNamespaceItem* const namespaceItem
         , XSObject* const component
         , XMLSize_t componentIndex
         , bool addToXSModel = true
    );

    void addComponentToIdVector
    (
        XSObject* const component
        , XMLSize_t componentIndex
    );

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSModel(const XSModel&);
    XSModel & operator=(const XSModel &);

protected:
    friend class XSObjectFactory;
    friend class XSObject;

    // -----------------------------------------------------------------------
    //  data members
    // -----------------------------------------------------------------------
    // fMemoryManager:
    //  used for any memory allocations
    MemoryManager* const                    fMemoryManager;

    StringList*                             fNamespaceStringList;
    XSNamespaceItemList*                    fXSNamespaceItemList;

    RefVectorOf<XSObject>*                  fIdVector[XSConstants::MULTIVALUE_FACET];

    /* Need a XSNamedMap for each component    top-level?
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
    XMLStringPool*                          fURIStringPool;
    XSAnnotationList*                       fXSAnnotationList;
    RefHashTableOf<XSNamespaceItem>*        fHashNamespace;
    XSObjectFactory*                        fObjFactory;
    RefVectorOf<XSNamespaceItem>*           fDeleteNamespace;
    XSModel*                                fParent;
    bool                                    fDeleteParent;
    bool                                    fAddedS4SGrammar;
};

inline XMLStringPool*  XSModel::getURIStringPool()
{
    return fURIStringPool;
}

inline StringList *XSModel::getNamespaces()
{
    return fNamespaceStringList;
}

inline XSNamespaceItemList *XSModel::getNamespaceItems()
{
    return fXSNamespaceItemList;
}

XERCES_CPP_NAMESPACE_END

#endif
