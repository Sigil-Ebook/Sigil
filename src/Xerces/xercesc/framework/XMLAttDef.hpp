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
 * $Id: XMLAttDef.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLATTDEF_HPP)
#define XERCESC_INCLUDE_GUARD_XMLATTDEF_HPP

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMemory.hpp>
#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLAttr;

/** Represents the core information of an attribute definition
 *
 *  This class defines the basic characteristics of an attribute, no matter
 *  what type of validator is used. If a particular schema associates more
 *  information with an attribute it will create a derivative of this class.
 *  So this class provides an abstract way to get basic information on
 *  attributes from any type of validator.
 *
 *  This class supports keyed collection semantics on the fully qualified
 *  attribute name, by providing a getKey() method to extract the key string.
 *  getKey(), in this case, just calls the virtual method getFullName() to
 *  get the fully qualified name, as defined by the derived class.
 *
 *  Note that the 'value' of an attribute type definition is the default or
 *  of fixed value given to it in its definition. If the attribute is of the
 *  enumerated or notation type, it will have an 'enumeration value' as well
 *  which is a space separated list of its possible vlaues.
 */
class XMLPARSER_EXPORT XMLAttDef : public XSerializable, public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Class specific types
    //
    //  AttTypes
    //      The list of possible types that an attribute can have, according
    //      to the XML 1.0 spec and schema.
    //
    //  DefAttTypes
    //      The modifiers that an attribute decl can have, which indicates
    //      whether instances of that attributes are required, implied, etc..
    //
    //  CreateReasons
    //      This type is used to store how an attribute declaration got into
    //      the elementdecl's attribute pool.
    //
    // -----------------------------------------------------------------------
	enum AttTypes
    {
        CData               = 0
        , ID                = 1
        , IDRef             = 2
        , IDRefs            = 3
        , Entity            = 4
        , Entities          = 5
        , NmToken           = 6
        , NmTokens          = 7
        , Notation          = 8
        , Enumeration       = 9
        , Simple            = 10
        , Any_Any           = 11
        , Any_Other         = 12
        , Any_List          = 13

        , AttTypes_Count
        , AttTypes_Min      = 0
        , AttTypes_Max      = 13
        , AttTypes_Unknown  = -1
	};

    enum DefAttTypes
    {
        Default                  = 0
        , Fixed                  = 1
        , Required               = 2
        , Required_And_Fixed     = 3
        , Implied                = 4
        , ProcessContents_Skip   = 5
        , ProcessContents_Lax    = 6
        , ProcessContents_Strict = 7
        , Prohibited             = 8

        , DefAttTypes_Count
        , DefAttTypes_Min   = 0
        , DefAttTypes_Max   = 8
        , DefAttTypes_Unknown = -1
	};

    enum CreateReasons
    {
        NoReason
        , JustFaultIn
    };

    // -----------------------------------------------------------------------
    //  Public static data members
    // -----------------------------------------------------------------------
    static const unsigned int fgInvalidAttrId;


    // -----------------------------------------------------------------------
    //  Public, static methods
    // -----------------------------------------------------------------------

    /** @name Public, static methods */
    //@{

    /** Get a string representation of the passed attribute type enum
      *
      * This method allows you to get a textual representation of an attribute
      * type, mostly for debug or display.
      *
      * @param attrType The attribute type value to get the string for.
      * @param manager The MemoryManager to use to allocate objects
      * @return A const pointer to the static string that holds the text
      *         description of the passed type.
      */
    static const XMLCh* getAttTypeString(const AttTypes attrType
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /** Get a string representation of the passed def attribute type enum
      *
      * This method allows you to get a textual representation of an default
      * attributetype, mostly for debug or display.
      *
      * @param attrType The default attribute type value to get the string for.
      * @param manager The MemoryManager to use to allocate objects
      * @return A const pointer to the static string that holds the text
      *         description of the passed default type.
      */
    static const XMLCh* getDefAttTypeString(const DefAttTypes attrType
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    //@}


    // -----------------------------------------------------------------------
    //  Destructor
    // -----------------------------------------------------------------------

    /** @name Destructor */
    //@{

    /**
      *  Destructor
      */
    virtual ~XMLAttDef();
    //@}


    // -----------------------------------------------------------------------
    //  The virtual attribute def interface
    // -----------------------------------------------------------------------

    /** @name Virtual interface */
    //@{

    /** Get the full name of this attribute type
      *
      * The derived class should return a const pointer to the full name of
      * this attribute. This will vary depending on the type of validator in
      * use.
      *
      * @return A const pointer to the full name of this attribute type.
      */
    virtual const XMLCh* getFullName() const = 0;

    /**
     * The derived class should implement any cleaning up required between
     * each use of an instance of this class for validation
     */
    virtual void reset() = 0;

    //@}


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    /** @name Getter methods */
    //@{

    /** Get the default type of this attribute type
      *
      * This method returns the 'default type' of the attribute. Default
      * type in this case refers to the XML concept of a default type for
      * an attribute, i.e. \#FIXED, \#IMPLIED, etc...
      *
      * @return The default type enum for this attribute type.
      */
    DefAttTypes getDefaultType() const;

    /** Get the enumeration value (if any) of this attribute type
      *
      * If the attribute is of an enumeration or notation type, then this
      * method will return a const reference to a string that contains the
      * space separated values that can the attribute can have.
      *
      * @return A const pointer to a string that contains the space separated
      *         legal values for this attribute.
      */
    const XMLCh* getEnumeration() const;

    /** Get the pool id of this attribute type
      *
      * This method will return the id of this attribute in the validator's
      * attribute pool. It was set by the validator when this attribute was
      * created.
      *
      * @return The pool id of this attribute type.
      */
    XMLSize_t getId() const;

    /** Get the type of this attribute
      *
      * Gets the type of this attribute. This type is represented by an enum
      * that converts the types of attributes allowed by XML, e.g. CDATA, NMTOKEN,
      * NOTATION, etc...
      *
      * @return The attribute type enumeration value for this type of
      *         attribute.
      */
    AttTypes getType() const;

    /** Get the default/fixed value of this attribute (if any.)
      *
      * If the attribute defined a default/fixed value, then it is stored
      * and this method will retrieve it. If it has non, then a null pointer
      * is returned.
      *
      * @return A const pointer to the default/fixed value for this attribute
      *         type.
      */
    const XMLCh* getValue() const;

    /** Get the create reason for this attribute
      *
      * This method returns an enumeration which indicates why this attribute
      * declaration exists.
      *
      * @return An enumerated value that indicates the reason why this attribute
      * was added to the attribute table.
      */
    CreateReasons getCreateReason() const;

    /** Indicate whether this attribute has been declared externally
      *
      * This method returns a boolean that indicates whether this attribute
      * has been declared externally.
      *
      * @return true if this attribute has been declared externally, else false.
      */
    bool isExternal() const;

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

    /** Set the default attribute type
      *
      * This method sets the default attribute type for this attribute.
      * This setting controls whether the attribute is required, fixed,
      * implied, etc...
      *
      * @param  newValue The new default attribute to set
      */
    void setDefaultType(const XMLAttDef::DefAttTypes newValue);

    /** Set the pool id for this attribute type.
      *
      * This method sets the pool id of this attribute type. This is usually
      * called by the validator that creates the actual instance (which is of
      * a derived type known only by the validator.)
      *
      * @param  newId The new pool id to set.
      */
    void setId(const XMLSize_t newId);

    /** Set the type of this attribute type.
      *
      * This method will set the type of the attribute. The type of an attribute
      * controls how it is normalized and what kinds of characters it can hold.
      *
      * @param  newValue The new attribute type to set
      */
    void setType(const XMLAttDef::AttTypes newValue);

    /** Set the default/fixed value of this attribute type.
      *
      * This method set the fixed/default value for the attribute. This value
      * will be used when instances of this attribute type are faulted in. It
      * <b>must</b> be a valid value for the type set by setType(). If the
      * type is enumeration or notation, this must be one of the valid values
      * set in the setEnumeration() call.
      *
      * @param  newValue The new fixed/default value to set.
      */
    void setValue(const XMLCh* const newValue);

    /** Set the enumerated value of this attribute type.
      *
      * This method sets the enumerated/notation value list for this attribute
      * type. It is a space separated set of possible values. These values must
      * meet the constrains of the XML spec for such values of this type of
      * attribute. This should only be set if the setType() method is used to
      * set the type to the enumeration or notation types.
      *
      * @param  newValue The new enumerated/notation value list to set.
      */
    void setEnumeration(const XMLCh* const newValue);

    /** Update the create reason for this attribute type.
      *
      * This method will update the 'create reason' field for this attribute
      * decl object.
      *
      * @param  newReason The new create reason.
      */
    void setCreateReason(const CreateReasons newReason);

    /**
      * Set the attribute decl to indicate external declaration
      *
      * @param  aValue The new value to indicate external declaration.
      */
    void setExternalAttDeclaration(const bool aValue);

    //@}

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(XMLAttDef)

protected :
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    XMLAttDef
    (
        const   AttTypes       type = CData
        , const DefAttTypes    defType= Implied
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    XMLAttDef
    (
        const   XMLCh* const        attValue
        , const AttTypes            type
        , const DefAttTypes         defType
        , const XMLCh* const        enumValues = 0
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLAttDef(const XMLAttDef&);
    XMLAttDef& operator=(const XMLAttDef&);


    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void cleanUp();


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fDefaultType
    //      Indicates what, if any, default stuff this attribute has.
    //
    //  fEnumeration
    //      If its an enumeration, this is the list of values as space
    //      separated values.
    //
    //  fId
    //      This is the unique id of this attribute, given to it when its put
    //      into the validator's attribute decl pool. It defaults to the
    //      special value XMLAttrDef::fgInvalidAttrId.
    //
    //  fType
    //      The type of attribute, which is one of the AttTypes values.
    //
    //  fValue
    //      This is the value of the attribute, which is the default value
    //      given in the attribute declaration.
    //
    //  fCreateReason
    //      This flag tells us how this attribute got created.  Sometimes even
    //      the attribute was not declared for the element, we want to fault
    //      fault it into the pool to avoid lots of redundant errors.
    //
    //  fExternalAttribute
    //      This flag indicates whether or not the attribute was declared externally.
    // -----------------------------------------------------------------------
    DefAttTypes     fDefaultType;
    AttTypes        fType;
    CreateReasons   fCreateReason;   
    bool            fExternalAttribute;
    XMLSize_t       fId;
    XMLCh*          fValue;
    XMLCh*          fEnumeration;
    MemoryManager*  fMemoryManager;
};


// ---------------------------------------------------------------------------
//  Getter methods
// ---------------------------------------------------------------------------
inline XMLAttDef::DefAttTypes XMLAttDef::getDefaultType() const
{
    return fDefaultType;
}

inline const XMLCh* XMLAttDef::getEnumeration() const
{
    return fEnumeration;
}

inline XMLSize_t XMLAttDef::getId() const
{
    return fId;
}

inline XMLAttDef::AttTypes XMLAttDef::getType() const
{
    return fType;
}

inline const XMLCh* XMLAttDef::getValue() const
{
    return fValue;
}

inline XMLAttDef::CreateReasons XMLAttDef::getCreateReason() const
{
    return fCreateReason;
}

inline bool XMLAttDef::isExternal() const
{
    return fExternalAttribute;
}

inline MemoryManager* XMLAttDef::getMemoryManager() const
{
    return fMemoryManager;
}

// ---------------------------------------------------------------------------
//  XMLAttDef: Setter methods
// ---------------------------------------------------------------------------
inline void XMLAttDef::setDefaultType(const XMLAttDef::DefAttTypes newValue)
{
    fDefaultType = newValue;
}

inline void XMLAttDef::setEnumeration(const XMLCh* const newValue)
{
    if (fEnumeration)
        fMemoryManager->deallocate(fEnumeration);

    fEnumeration = XMLString::replicate(newValue, fMemoryManager);
}

inline void XMLAttDef::setId(const XMLSize_t newId)
{
    fId = newId;
}

inline void XMLAttDef::setType(const XMLAttDef::AttTypes newValue)
{
    fType = newValue;
}

inline void XMLAttDef::setValue(const XMLCh* const newValue)
{
    if (fValue)
       fMemoryManager->deallocate(fValue);

    fValue = XMLString::replicate(newValue, fMemoryManager);
}

inline void
XMLAttDef::setCreateReason(const XMLAttDef::CreateReasons newReason)
{
    fCreateReason = newReason;
}

inline void XMLAttDef::setExternalAttDeclaration(const bool aValue)
{
    fExternalAttribute = aValue;
}

XERCES_CPP_NAMESPACE_END

#endif
