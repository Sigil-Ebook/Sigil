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
 * $Id: XMLElementDecl.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLELEMENTDECL_HPP)
#define XERCESC_INCLUDE_GUARD_XMLELEMENTDECL_HPP

#include <xercesc/framework/XMLAttr.hpp>
#include <xercesc/framework/XMLAttDefList.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class ContentSpecNode;
class XMLContentModel;

/**
 *  This class defines the core information of an element declaration. Each
 *  validator (DTD, Schema, etc...) will have its own information that it
 *  associations with the declaration of an element, but they must all share
 *  at least this core information, i.e. they must all derive from this
 *  class. The set of info enforced at this level is driven by the needs of
 *  XML 1.0 spec validation and well formedness checks.
 *
 *  This class defines some special element id values for invalid elements
 *  and PCDATA elements, as well as a string for the special PCDATA element
 *  name. All validators must honor these special values in order to allow
 *  content models to work generically (i.e. to let code know when its dealing
 *  with invalid or PCDATA element ids without having to know what type of
 *  validator its messing with.)
 */
class XMLPARSER_EXPORT XMLElementDecl : public XSerializable, public XMemory
{
 public:
    // -----------------------------------------------------------------------
    //  Class specific types
    //
    //  CreateReasons
    //      This type is used to store how an element declaration got into
    //      the grammar's element pool. They are faulted in for various
    //      reasons.
    //
    //  LookupOpts
    //      These are the values used by the attribute lookup methods.
    //
    //  CharDataOpts
    //      This is used to indicate how this type of element reacts to
    //      character data as content.
    // -----------------------------------------------------------------------
    enum CreateReasons
    {
        NoReason
        , Declared
        , AttList
        , InContentModel
        , AsRootElem
        , JustFaultIn
    };

    enum CharDataOpts
    {
        NoCharData
        , SpacesOk
        , AllCharData
    };


    // -----------------------------------------------------------------------
    //  Public static data
    //
    //  fgInvalidElemId
    //      A value to represent an invalid element node id.
    //
    //  fgPCDataElemId
    //      This is the value to use to represent a PCDATA node when an
    //      element id is required.
    //
    //  fgPCDataElemName
    //      This is the value to use to represent a PCDATA node when an
    //      element name is required.
    // -----------------------------------------------------------------------
    static const unsigned int   fgInvalidElemId;
    static const unsigned int   fgPCDataElemId;
    static const XMLCh          fgPCDataElemName[];



    // -----------------------------------------------------------------------
    //  Destructor
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    virtual ~XMLElementDecl();
    //@}


    // -----------------------------------------------------------------------
    //  The virtual element decl interface
    // -----------------------------------------------------------------------

    /** @name Virual ElementDecl interface */
    //@{

    /** Get a list of attributes defined for this element.
      *
      * The derived class should return a reference to some member object which
      * implements the XMLAttDefList interface. This object gives the scanner the
      * ability to look through the attributes defined for this element.
      *
      * It is done this way for efficiency, though of course this is not thread
      * safe. The scanner guarantees that it won't ever call this method in any
      * nested way, but the outside world must be careful about when it calls
      * this method, and optimally never would.
      */
    virtual XMLAttDefList& getAttDefList() const = 0;

    /** The character data options for this element type
      *
      * The derived class should return an appropriate character data opts value
      * which correctly represents its tolerance towards whitespace or character
      * data inside of its instances. This allows the scanner to do all of the
      * validation of character data.
      */
    virtual CharDataOpts getCharDataOpts() const = 0;

    /** Indicate whether this element type defined any attributes
      *
      * The derived class should return a boolean that indicates whether this
      * element has any attributes defined for it or not. This is an optimization
      * that allows the scanner to skip some work if no attributes exist.
      */
    virtual bool hasAttDefs() const = 0;

    /** Get a pointer to the content spec node
      *
      * This method will return a const pointer to the content spec node object
      * of this element.
      *
      * @return A const pointer to the element's content spec node
      */
    virtual const ContentSpecNode* getContentSpec() const = 0;

    /** Get a pointer to the content spec node
      *
      * This method is identical to the previous one, except that it is non
      * const.
      */
    virtual ContentSpecNode* getContentSpec() = 0;

    /** Set the content spec node object for this element type
      *
      * This method will adopt the based content spec node object. This is called
      * by the actual validator which is parsing its DTD or Schema or whatever
      * and store it on the element decl object via this method.
      *
      * @param  toAdopt This method will adopt the passed content node spec
      *         object. Any previous object is destroyed.
      */
    virtual void setContentSpec(ContentSpecNode* toAdopt) = 0;

    /** Get a pointer to the abstract content model
      *
      * This method will return a const pointer to the content model object
      * of this element. This class is a simple abstraction that allows an
      * element to define and use multiple, specialized content model types
      * internally but still allow the outside world to do simple stuff with
      * them.
      *
      * @return A pointer to the element's content model, via the basic
      * abstract content model type.
      */
    virtual XMLContentModel* getContentModel() = 0;

    /** Set the content model object for this element type
      *
      * This method will adopt the based content model object. This is called
      * by the actual validator which is parsing its DTD or Schema or whatever
      * a creating an element decl. It will build what it feels is the correct
      * content model type object and store it on the element decl object via
      * this method.
      *
      * @param  newModelToAdopt This method will adopt the passed content model
      *         object. Any previous object is destroyed.
      */
    virtual void setContentModel(XMLContentModel* const newModelToAdopt) = 0;

    /** Geta formatted string of the content model
      *
      * This method is a convenience method which will create a formatted
      * representation of the content model of the element. It will not always
      * exactly recreate the original model, since some normalization or
      * or reformatting may occur. But, it will be a technically accurate
      * representation of the original content model.
      *
      * @return A pointer to an internal buffer which contains the formatted
      *         content model. The caller does not own this buffer and should
      *         copy it if it needs to be kept around.
      */
    virtual const XMLCh* getFormattedContentModel ()   const = 0;

    //@}


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    /** @name Getter methods */
    //@{

    /** Get the base name of this element type.
      *
      * Return the base name part of the element's name. This is the
      * same regardless of whether namespaces are enabled or not.
      *
      * @return A const pointer to the base name of the element decl.
      */
    const XMLCh* getBaseName() const;
    XMLCh* getBaseName();

    /** Get the URI id of this element type.
      *
      * Return the URI Id of this element.
      *
      * @return The URI Id of the element decl, or the emptyNamespaceId if not applicable.
      */
    unsigned int getURI() const;

    /** Get the QName of this element type.
      *
      * Return the QName part of the element's name.  This is the
      * same regardless of whether namespaces are enabled or not.
      *
      * @return A const pointer to the QName of the element decl.
      */
    const QName* getElementName() const;
    QName* getElementName();

    /** Get the full name of this element type.
      *
      * Return the full name of the element. If namespaces
      * are not enabled, then this is the qName. Else it is the {uri}baseName
      * form. For those validators that always require namespace processing, it
      * will always be in the latter form because namespace processing will always
      * be on.
      */
    const XMLCh* getFullName() const;

    /** Get the create reason for this element type
      *
      * This method returns an enumeration which indicates why this element
      * declaration exists. Elements can be used before they are actually
      * declared, so they will often be faulted into the pool and marked as
      * to why they are there.
      *
      * @return An enumerated value that indicates the reason why this element
      * was added to the element decl pool.
      */

    CreateReasons getCreateReason() const;

    /** Get the element decl pool id for this element type
      *
      * This method will return the element decl pool id of this element
      * declaration. This uniquely identifies this element type within the
      * parse event that it is declared within. This value is assigned by the
      * grammar whose decl pool this object belongs to.
      *
      * @return The element decl id of this element declaration.
      */
    XMLSize_t getId() const;

    /** Indicate whether this element type has been declared yet
      *
      * This method returns a boolean that indicates whether this element
      * has been declared yet. There are a number of reasons why an element
      * declaration can be faulted in, but eventually it must be declared or
      * its an error. See the CreateReasons enumeration.
      *
      * @return true if this element has been declared, else false.
      */
    bool isDeclared() const;

    /** Indicate whether this element type has been declared externally
      *
      * This method returns a boolean that indicates whether this element
      * has been declared externally.
      *
      * @return true if this element has been declared externally, else false.
      */

    bool isExternal() const;

    /** Get the memory manager
      *
      * This method returns the configurable memory manager used by the
      * element declaration for dynamic allocation/deallocation.
      *
      * @return the memory manager
      */
    MemoryManager* getMemoryManager() const;

    //@}


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------

    /** @name Setter methods */
    //@{

    /** Set the element name object for this element type
      *
      * This method will adopt the based content spec node object. This is called
      * by the actual validator which is parsing its DTD or Schema or whatever
      * and store it on the element decl object via this method.
      *
      * @param  prefix       Prefix of the element
      * @param  localPart    Base Name of the element
      * @param  uriId        The uriId of the element
      */
      void setElementName(const XMLCh* const       prefix
                        , const XMLCh* const       localPart
                        , const int                uriId );

    /** Set the element name object for this element type
      *
      * This method will adopt the based content spec node object. This is called
      * by the actual validator which is parsing its DTD or Schema or whatever
      * and store it on the element decl object via this method.
      *
      * @param  rawName      Full Name of the element
      * @param  uriId        The uriId of the element
      */
      void setElementName(const XMLCh* const    rawName
                        , const int             uriId );

    /** Set the element name object for this element type
      *
      * This method will adopt the based content spec node object. This is called
      * by the actual validator which is parsing its DTD or Schema or whatever
      * and store it on the element decl object via this method.
      *
      * @param  elementName  QName of the element
      */
      void setElementName(const QName* const    elementName);

    /** Update the create reason for this element type.
      *
      * This method will update the 'create reason' field for this element
      * decl object. As the validator parses its DTD, Schema, etc... it will
      * encounter various references to an element declaration, which will
      * cause the element declaration to either be declared or to be faulted
      * into the pool in preparation for some future declaration. As it does
      * so,it will update this field to indicate the current status of the
      * decl object.
      */
    void setCreateReason(const CreateReasons newReason);

    /** Set the element decl pool id for this element type
      *
      * This method will set the pool id of this element decl. This is called
      * by the grammar which created this object, and will provide this
      * decl object with a unique id within the parse event that created it.
      */
    void setId(const XMLSize_t newId);


    /** Set the element decl to indicate external declaration
      *
      */
    void setExternalElemDeclaration(const bool aValue);

    //@}


    // -----------------------------------------------------------------------
    //  Miscellaneous methods
    // -----------------------------------------------------------------------

    /** @name Miscellaneous methods */
    //@{

    //@}

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(XMLElementDecl)

    enum objectType
    {
        Schema
      , DTD
      , UnKnown
    };

    virtual XMLElementDecl::objectType  getObjectType() const = 0;

    static void            storeElementDecl(XSerializeEngine&        serEng
                                          , XMLElementDecl*    const element);

    static XMLElementDecl* loadElementDecl(XSerializeEngine& serEng);

protected :
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    XMLElementDecl(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLElementDecl(const XMLElementDecl&);
    XMLElementDecl& operator=(const XMLElementDecl&);


    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fElementName
    //      This is the name of the element decl.
    //
    //  fCreateReason
    //      We sometimes have to put an element decl object into the elem
    //      decl pool before the element's declaration is seen, such as when
    //      its used in another element's content model or an att list is
    //      seen for it. This flag tells us whether its been declared, and
    //      if not why it had to be created.
    //
    //  fId
    //      The unique id of this element. This is created by the derived
    //      class, or more accurately the grammar that owns the objects
    //      of the derived types. But, since they all have to have them, we
    //      let them all store the id here. It is defaulted to have the
    //      value fgInvalidElem until explicitly set.
    //
    //  fExternalElement
    //      This flag indicates whether or the element was declared externally.
    // -----------------------------------------------------------------------
    MemoryManager*      fMemoryManager;
    QName*              fElementName;
    CreateReasons       fCreateReason;
    XMLSize_t           fId;
    bool                fExternalElement;
};


// ---------------------------------------------------------------------------
//  XMLElementDecl: Getter methods
// ---------------------------------------------------------------------------
inline const XMLCh* XMLElementDecl::getBaseName() const
{
    return fElementName->getLocalPart();
}

inline XMLCh* XMLElementDecl::getBaseName()
{
    return fElementName->getLocalPart();
}

inline unsigned int XMLElementDecl::getURI() const
{
    return fElementName->getURI();
}

inline const QName* XMLElementDecl::getElementName() const
{
    return fElementName;
}

inline QName* XMLElementDecl::getElementName()
{
    return fElementName;
}

inline const XMLCh* XMLElementDecl::getFullName() const
{
    return fElementName->getRawName();
}

inline XMLElementDecl::CreateReasons XMLElementDecl::getCreateReason() const
{
    return fCreateReason;
}

inline XMLSize_t XMLElementDecl::getId() const
{
    return fId;
}

inline bool XMLElementDecl::isDeclared() const
{
    return (fCreateReason == Declared);
}


inline bool XMLElementDecl::isExternal() const
{
    return fExternalElement;
}

inline MemoryManager* XMLElementDecl::getMemoryManager() const
{
    return fMemoryManager;
}


// ---------------------------------------------------------------------------
//  XMLElementDecl: Setter methods
// ---------------------------------------------------------------------------
inline void
XMLElementDecl::setCreateReason(const XMLElementDecl::CreateReasons newReason)
{
    fCreateReason = newReason;
}

inline void XMLElementDecl::setId(const XMLSize_t newId)
{
    fId = newId;
}


inline void XMLElementDecl::setExternalElemDeclaration(const bool aValue)
{
    fExternalElement = aValue;
}

XERCES_CPP_NAMESPACE_END

#endif
