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
 * $Id: DTDElementDecl.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DTDELEMENTDECL_HPP)
#define XERCESC_INCLUDE_GUARD_DTDELEMENTDECL_HPP

#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/QName.hpp>
#include <xercesc/framework/XMLElementDecl.hpp>
#include <xercesc/framework/XMLContentModel.hpp>
#include <xercesc/validators/DTD/DTDAttDef.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class ContentSpecNode;
class DTDAttDefList;


//
//  This class is a derivative of the basic element decl. This one implements
//  the virtuals so that they work for a DTD. The big difference is that
//  they don't live in any URL in the DTD. The names are just stored as full
//  QNames, so they are not split out and element decls don't live within
//  URL namespaces or anything like that.
//

class VALIDATORS_EXPORT DTDElementDecl : public XMLElementDecl
{
public :
    // -----------------------------------------------------------------------
    //  Class specific types
    //
    //  ModelTypes
    //      Indicates the type of content model that an element has. This
    //      indicates how the content model is represented and validated.
    // -----------------------------------------------------------------------
    enum ModelTypes
    {
        Empty
        , Any
        , Mixed_Simple
        , Children

        , ModelTypes_Count
    };


    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    DTDElementDecl(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    DTDElementDecl
    (
          const XMLCh* const   elemRawName
        , const unsigned int   uriId
        , const ModelTypes     modelType
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    DTDElementDecl
    (
          QName* const         elementName
        , const ModelTypes     modelType = Any
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    ~DTDElementDecl();


    // -----------------------------------------------------------------------
    //  The virtual element decl interface
    // -----------------------------------------------------------------------
    virtual XMLAttDefList& getAttDefList() const;
    virtual CharDataOpts getCharDataOpts() const;
    virtual bool hasAttDefs() const;   
    virtual const ContentSpecNode* getContentSpec() const;
    virtual ContentSpecNode* getContentSpec();
    virtual void setContentSpec(ContentSpecNode* toAdopt);
    virtual XMLContentModel* getContentModel();
    virtual void setContentModel(XMLContentModel* const newModelToAdopt);
    virtual const XMLCh* getFormattedContentModel ()   const;

    // -----------------------------------------------------------------------
    // Support keyed collections
    //
    // This method allows objects of this type be placed into one of the
    // standard keyed collections. This method will return the full name of
    // the element, which will vary depending upon the type of the grammar.
    // -----------------------------------------------------------------------
    const XMLCh* getKey() const;

    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const DTDAttDef* getAttDef(const XMLCh* const attName) const;
    DTDAttDef* getAttDef(const XMLCh* const attName);
    ModelTypes getModelType() const;

    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void addAttDef(DTDAttDef* const toAdd);
    void setModelType(const DTDElementDecl::ModelTypes toSet);

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(DTDElementDecl)

    virtual XMLElementDecl::objectType  getObjectType() const;

private :
    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void faultInAttDefList() const;
    XMLContentModel* createChildModel() ;
    XMLContentModel* makeContentModel() ;
    XMLCh* formatContentModel () const ;

    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    DTDElementDecl(const DTDElementDecl &);
    DTDElementDecl& operator = (const  DTDElementDecl&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fAttDefs
    //      The list of attributes that are defined for this element. Each
    //      element is its own little 'namespace' for attributes, so each
    //      element maintains its own list of owned attribute defs. It is
    //      faulted in when an attribute is actually added.
    //
    //  fAttList
    //      We have to return a view of our att defs via the abstract view
    //      that the scanner understands. It may or may not ever be asked
    //      for so we fault it in as needed.
    //
    //  fContentSpec
    //      This is the content spec for the node. It contains the original
    //      content spec that was read from the DTD, as a tree of nodes. This
    //      one is always set up, and is used to build the fContentModel
    //      version if we are validating.
    //
    //  fModelType
    //      The content model type of this element. This tells us what kind
    //      of content model to create.
    //
    //  fContentModel
    //      The content model object for this element. It is stored here via
    //      its abstract interface.
    //
    //  fFormattedModel
    //      This is a faulted in member. When the outside world asks for
    //      our content model as a string, we format it and fault it into
    //      this field (to avoid doing the formatted over and over.)
    // -----------------------------------------------------------------------
    ModelTypes                  fModelType;

    RefHashTableOf<DTDAttDef>*  fAttDefs;
    DTDAttDefList*              fAttList;
    ContentSpecNode*            fContentSpec;
    XMLContentModel*            fContentModel;
    XMLCh*                      fFormattedModel;
};

// ---------------------------------------------------------------------------
//  DTDElementDecl: XMLElementDecl virtual interface implementation
// ---------------------------------------------------------------------------
inline ContentSpecNode* DTDElementDecl::getContentSpec()
{
    return fContentSpec;
}

inline const ContentSpecNode* DTDElementDecl::getContentSpec() const
{
    return fContentSpec;
}

inline XMLContentModel* DTDElementDecl::getContentModel()
{
    if (!fContentModel)
        fContentModel = makeContentModel();
    return fContentModel;
}

inline void
DTDElementDecl::setContentModel(XMLContentModel* const newModelToAdopt)
{
    delete fContentModel;
    fContentModel = newModelToAdopt;

    // reset formattedModel
    if (fFormattedModel)
    {
        getMemoryManager()->deallocate(fFormattedModel);
        fFormattedModel = 0;
    }
}

// ---------------------------------------------------------------------------
//  DTDElementDecl: Miscellaneous methods
// ---------------------------------------------------------------------------
inline const XMLCh* DTDElementDecl::getKey() const
{
    return getFullName();
}

// ---------------------------------------------------------------------------
//  DTDElementDecl: Getter methods
// ---------------------------------------------------------------------------
inline DTDElementDecl::ModelTypes DTDElementDecl::getModelType() const
{
    return fModelType;
}

// ---------------------------------------------------------------------------
//  DTDElementDecl: Setter methods
// ---------------------------------------------------------------------------
inline void
DTDElementDecl::setModelType(const DTDElementDecl::ModelTypes toSet)
{
    fModelType = toSet;
}

XERCES_CPP_NAMESPACE_END

#endif
