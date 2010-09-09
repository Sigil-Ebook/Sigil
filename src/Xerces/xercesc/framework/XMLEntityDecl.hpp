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
 * $Id: XMLEntityDecl.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLENTITYDECL_HPP)
#define XERCESC_INCLUDE_GUARD_XMLENTITYDECL_HPP

#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 *  This class defines that core information that defines an XML entity, no
 *  matter what validator is used. Each validator will create a derivative
 *  of this class which adds any extra information it requires.
 *
 *  This class supports keyed collection semantics via the getKey() method
 *  which extracts the key field, the entity name in this case. The name will
 *  have whatever form is deemed appropriate for the type of validator in
 *  use.
 *
 *  When setting the fields of this class, you must make sure that you do
 *  not set conflicting values. For instance, an internal entity cannot have
 *  a notation name. And an external entity cannot have a value string.
 *  These rules are defined by the XML specification. In most cases, these
 *  objects are created by validator objects as they parse a DTD or Schema
 *  or whatever, at which time they confirm the correctness of the data before
 *  creating the entity decl object.
 */
class XMLPARSER_EXPORT XMLEntityDecl : public XSerializable, public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    /** @name Constructors */
    //@{

    /**
      *  Default Constructor
      */
    XMLEntityDecl(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /** Constructor with a const entity name
      *
      * @param  entName The new name to give to this entity.
      * @param  manager Pointer to the memory manager to be used to
      *                 allocate objects.
      */
    XMLEntityDecl
    (
        const   XMLCh* const    entName
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
      * Constructor with a const entity name and value
      *
      * @param  entName The new name to give to this entity.
      * @param  value   The new value to give to this entity name.
      * @param  manager Pointer to the memory manager to be used to
      *                 allocate objects.
      */
    XMLEntityDecl
    (
        const   XMLCh* const    entName
        , const XMLCh* const    value
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
      * Constructor with a const entity name and single XMLCh value
      *
      * @param  entName The new name to give to this entity.
      * @param  value   The new value to give to this entity name.
      * @param manager  Pointer to the memory manager to be used to
      *                 allocate objects.
      */
    XMLEntityDecl
    (
        const   XMLCh* const    entName
        , const XMLCh           value
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    //@}

    /** @name Destructor */
    //@{

    /**
      *  Default destructor
      */
    virtual ~XMLEntityDecl();

    //@}


    // -----------------------------------------------------------------------
    //  Virtual entity decl interface
    // -----------------------------------------------------------------------

    /** @name The pure virtual methods in this interface. */
    //@{

    /** Get the 'declared in internal subset' flag
      *
      * Gets the state of the flag which indicates whether the entity was
      * declared in the internal or external subset. Some structural
      * description languages might not have an internal subset concept, in
      * which case this will always return false.
      */
    virtual bool getDeclaredInIntSubset() const = 0;

    /** Get the 'is parameter entity' flag
      *
      * Gets the state of the flag which indicates whether this entity is
      * a parameter entity. If not, then its a general entity.
      */
    virtual bool getIsParameter() const = 0;

    /** Get the 'is special char entity' flag
      *
      * Gets the state of the flag that indicates whether this entity is
      * one of the special, intrinsically supported character entities.
      */
    virtual bool getIsSpecialChar() const = 0;

    //@}


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    /** @name Getter methods */
    //@{

    /**
      * Gets the pool id of this entity. Validators maintain all decls in
      * pools, from which they can be quickly extracted via id.
      */
    XMLSize_t getId() const;

    /**
      * Returns a const pointer to the name of this entity decl. This name
      * will be in whatever format is appropriate for the type of validator
      * in use.
      */
    const XMLCh* getName() const;

    /**
      * Gets the notation name, if any, declared for this entity. If this
      * entity is not a notation type entity, it will be a null pointer.
      */
    const XMLCh* getNotationName() const;

    /**
      * Gets the public id declared for this entity. Public ids are optional
      * so it can be a null pointer.
      */
    const XMLCh* getPublicId() const;

    /**
      * Gets the system id declared for this entity. The system id is required
      * so this method should never return a null pointers.
      */
    const XMLCh* getSystemId() const;

    /**
      * Gets the base URI for this entity.
      */
    const XMLCh* getBaseURI() const;

    /**
      * This method returns the value of an internal entity. If this is not
      * an internal entity (i.e. its external), then this will be a null
      * pointer.
      */
    const XMLCh* getValue() const;

    /**
     *  This method returns the number of characters in the value returned
     *  by getValue(). If this entity is external, this will be zero since
     *  an external entity has no internal value.
     */
    XMLSize_t getValueLen() const;

    /**
      * Indicates that this entity is an external entity. If not, then it is
      * assumed to be an internal entity, surprise.
      */
    bool isExternal() const;

    /**
      * Indicates whether this entity is unparsed. This is meaningless for
      * internal entities. Some external entities are unparsed in that they
      * refer to something other than XML source.
      */
    bool isUnparsed() const;

    /** Get the plugged-in memory manager
      *
      * This method returns the plugged-in memory manager user for dynamic
      * memory allocation/deallocation.
      *
      * @return the plugged-in memory manager
      */
    MemoryManager* getMemoryManager() const;

    //@}


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------

    /** @name Setter methods */
    //@{

    /**
     *  This method will set the entity name. The format of this name is
     *  defined by the particular validator in use, since it will be the
     *  one who creates entity definitions as it parses the DTD, Schema,
     *  ect...
     *
     *  @param  entName   The new name to give to this entity.
     */
    void setName
    (
        const   XMLCh* const    entName
    );

    /**
     *  This method will mark whether the entity is external.
     *
     *  @param  value   The new value for the 'is external' flag.
     */
    void setIsExternal(bool value);

    /**
     *  This method will set the notation name for this entity. By setting
     *  this, you are indicating that this is an unparsed external entity.
     *
     *  @param  newName   The new notation name to give to this entity.
     */
    void setNotationName(const XMLCh* const newName);

    /**
     *  This method will set a new public id on this entity. The public id
     *  has no particular form and is purely for client consumption.
     *
     *  @param  newId     The new public id to give to this entity.
     */
    void setPublicId(const XMLCh* const newId);

    /**
     *  This method will set a new sysetm id on this entity. This will
     *  then control where the source for this entity lives. If it is
     *  an internal entity, then the system id is only for bookkeeping
     *  purposes, and to allow any external entities referenced from
     *  within the entity to be correctly resolved.
     *
     *  @param  newId     The new system id to give to the entity.
     */
    void setSystemId(const XMLCh* const newId);

    /**
     *  This method will set a new baseURI on this entity. This will
     *  then control the URI used to resolve the relative system Id.
     *
     *  @param  newId     The new base URI to give to the entity.
     */
    void setBaseURI(const XMLCh* const newId);

    /**
     *  This method will set a new value for this entity. This is only
     *  valid if the entity is to be an internal entity. By setting this
     *  field, you are indicating that the entity is internal.
     *
     *  @param  newValue  The new value to give to this entity.
     */
    void setValue(const XMLCh* const newValue);

    //@}

    /* For internal use only */
    void setId(const XMLSize_t newId);


    // -----------------------------------------------------------------------
    //  Support named pool syntax
    // -----------------------------------------------------------------------

    /** @name Setter methods */
    //@{

    /**
      * This method allows objects of this class to be used within a standard
      * keyed collection used commonly within the parser system. The collection
      * calls this method to get the key (usually to hash it) by which the
      * object is to be stored.
      */
    const XMLCh* getKey() const;

    //@}

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(XMLEntityDecl)

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLEntityDecl(const XMLEntityDecl&);
    XMLEntityDecl& operator=(XMLEntityDecl&);


    // -----------------------------------------------------------------------
    //  XMLEntityDecl: Private helper methods
    // -----------------------------------------------------------------------
    void cleanUp();


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fId
    //      This is the unique id given to this entity decl.
    //
    //  fName
    //      The name of the entity. Entity names are never namespace based.
    //
    //  fNotationName
    //      The optional notation of the entity. If there was none, then its
    //      empty.
    //
    //  fPublicId
    //      The public id of the entity, which can be empty.
    //
    //  fSystemId
    //      The system id of the entity.
    //
    //  fValue
    //  fValueLen
    //      The entity's value and length, which is only valid if its an
    //      internal style entity.
    //
    //  fBaseURI
    //      The base URI of the entity.   According to XML InfoSet, such value
    //      is the URI where it is declared (NOT referenced).
    // -----------------------------------------------------------------------
    XMLSize_t       fId;
    XMLSize_t       fValueLen;
    XMLCh*          fValue;
    XMLCh*          fName;
    XMLCh*          fNotationName;
    XMLCh*          fPublicId;
    XMLCh*          fSystemId;
    XMLCh*          fBaseURI;
    bool            fIsExternal;
    MemoryManager*  fMemoryManager;
};


// ---------------------------------------------------------------------------
//  XMLEntityDecl: Getter methods
// ---------------------------------------------------------------------------
inline XMLSize_t XMLEntityDecl::getId() const
{
    return fId;
}

inline const XMLCh* XMLEntityDecl::getName() const
{
    return fName;
}

inline const XMLCh* XMLEntityDecl::getNotationName() const
{
    return fNotationName;
}

inline const XMLCh* XMLEntityDecl::getPublicId() const
{
    return fPublicId;
}

inline const XMLCh* XMLEntityDecl::getSystemId() const
{
    return fSystemId;
}

inline const XMLCh* XMLEntityDecl::getBaseURI() const
{
    return fBaseURI;
}

inline const XMLCh* XMLEntityDecl::getValue() const
{
    return fValue;
}

inline XMLSize_t XMLEntityDecl::getValueLen() const
{
    return fValueLen;
}

inline bool XMLEntityDecl::isExternal() const
{
    return fIsExternal;
}

inline bool XMLEntityDecl::isUnparsed() const
{
    // If it has a notation, its unparsed
    return (fNotationName != 0);
}

inline MemoryManager* XMLEntityDecl::getMemoryManager() const
{
    return fMemoryManager;
}

// ---------------------------------------------------------------------------
//  XMLEntityDecl: Setter methods
// ---------------------------------------------------------------------------
inline void XMLEntityDecl::setId(const XMLSize_t newId)
{
    fId = newId;
}

inline void XMLEntityDecl::setIsExternal(bool value)
{
    fIsExternal = value;
}

inline void XMLEntityDecl::setNotationName(const XMLCh* const newName)
{
    if (fNotationName)
        fMemoryManager->deallocate(fNotationName);

    fNotationName = XMLString::replicate(newName, fMemoryManager);
}

inline void XMLEntityDecl::setPublicId(const XMLCh* const newId)
{
    if (fPublicId)
        fMemoryManager->deallocate(fPublicId);

    fPublicId = XMLString::replicate(newId, fMemoryManager);
}

inline void XMLEntityDecl::setSystemId(const XMLCh* const newId)
{
    if (fSystemId)
        fMemoryManager->deallocate(fSystemId);

    fSystemId = XMLString::replicate(newId, fMemoryManager);
}

inline void XMLEntityDecl::setBaseURI(const XMLCh* const newId)
{
    if (fBaseURI)
        fMemoryManager->deallocate(fBaseURI);

    fBaseURI = XMLString::replicate(newId, fMemoryManager);
}

inline void XMLEntityDecl::setValue(const XMLCh* const newValue)
{
    if (fValue)
        fMemoryManager->deallocate(fValue);

    fValue = XMLString::replicate(newValue, fMemoryManager);
    fValueLen = XMLString::stringLen(newValue);
}


// ---------------------------------------------------------------------------
//  XMLEntityDecl: Support named pool syntax
// ---------------------------------------------------------------------------
inline const XMLCh* XMLEntityDecl::getKey() const
{
    return fName;
}

XERCES_CPP_NAMESPACE_END

#endif
