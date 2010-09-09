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
 * $Id: XMLAttr.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLATTR_HPP)
#define XERCESC_INCLUDE_GUARD_XMLATTR_HPP

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/QName.hpp>
#include <xercesc/framework/XMLAttDef.hpp>
#include <xercesc/validators/datatype/DatatypeValidator.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 *  This class defines the information about an attribute that will come out
 *  of the scanner during parsing. This information does not depend upon the
 *  type of validator because it is not tied to any scheme/DTD type info. Its
 *  just the raw XML 1.0 information that will be reported about an attribute
 *  in the startElement() callback method of the XMLDocumentHandler class.
 *  Hence it is not intended to be extended or derived from. Its designed to
 *  be used as is.
 *
 *  The 'specified' field of this class indicates whether the attribute was
 *  actually present or whether it was faulted in because it had a fixed or
 *  default value.
 *
 *  The code receiving this information can ask its validator for more info
 *  about the attribute, i.e. get its declaration from the DTD/Schema info.
 *
 *  Because of the heavy use (and reuse) of instances of this class, and the
 *  number of string members it has, this class takes pains to not reallocate
 *  string members unless it has to. It keeps up with how long each buffer
 *  is and only reallocates if the new value won't fit.
 */
class XMLPARSER_EXPORT XMLAttr : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructors */
    //@{

    /**
      * The default constructor just setsup an empty attribute to be filled
      * in the later. Though the initial state is a reasonable one, it is
      * not documented because it should not be depended on.
      *
      * @param  manager     The configurable memory manager
      */
    XMLAttr(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    /**
      * This is the primary constructor which takes all of the information
      * required to construct a complete attribute object.
      *
      * @param  uriId       The id into the validator's URI pool of the URI
      *                     that the prefix mapped to. Only used if namespaces
      *                     are enabled/supported.
      *
      * @param  attrName    The base name of the attribute, i.e. the part
      *                     after any prefix.
      *
      * @param  attrPrefix  The prefix, if any, of this attribute's name. If
      *                     this is empty, then uriID is meaningless as well.
      *
      * @param  attrValue   The value string of the attribute, which should
      *                     be fully normalized by XML rules!
      *
      * @param  type        The type of the attribute. This will indicate
      *                     the type of normalization done and constrains
      *                     the value content. Make sure that the value
      *                     set meets the constraints!
      *
      * @param  specified   Indicates whether the attribute was explicitly
      *                     specified or not. If not, then it was faulted
      *                     in from a FIXED or DEFAULT value.
      *
      * @param  manager     The configurable memory manager
      * @param datatypeValidator type used to validate the attribute, 
      *                         if it was validated by an XML Schema
      * @param isSchema         true if and only if this attribute was validated
      *                         by an XML Schema
      */
    XMLAttr
    (
          const unsigned int        uriId
        , const XMLCh* const        attrName
        , const XMLCh* const        attrPrefix
        , const XMLCh* const        attrValue
        , const XMLAttDef::AttTypes type = XMLAttDef::CData
        , const bool                specified = true
        , MemoryManager* const      manager = XMLPlatformUtils::fgMemoryManager
        , DatatypeValidator * datatypeValidator = 0
        , const bool isSchema = false
    );

    /**
      * This is the primary constructor which takes all of the information
      * required to construct a complete attribute object.
      *
      * @param  uriId       The id into the validator's URI pool of the URI
      *                     that the prefix mapped to. Only used if namespaces
      *                     are enabled/supported.
      *
      * @param  rawName     The raw name of the attribute.
      *
      * @param  attrValue   The value string of the attribute, which should
      *                     be fully normalized by XML rules!
      *
      * @param  type        The type of the attribute. This will indicate
      *                     the type of normalization done and constrains
      *                     the value content. Make sure that the value
      *                     set meets the constraints!
      *
      * @param  specified   Indicates whether the attribute was explicitly
      *                     specified or not. If not, then it was faulted
      *                     in from a FIXED or DEFAULT value.
      *
      * @param  manager     The configurable memory manager
      * @param datatypeValidator type used to validate the attribute, 
      *                         if it was validated by an XML Schema
      * @param isSchema         true if and only if this attribute was validated
      *                         by an XML Schema
      */
    XMLAttr
    (
        const unsigned int uriId
        , const XMLCh* const rawName
        , const XMLCh* const attrValue
        , const XMLAttDef::AttTypes type = XMLAttDef::CData
        , const bool specified = true
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
        , DatatypeValidator * datatypeValidator = 0
        , const bool isSchema = false
    );

    //@}

    /** @name Destructor */
    //@{
    ~XMLAttr();
    //@}


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------

    /** @name Getter methods */
    //@{

    /**
      * This method returns the attribute name in a QName format.
      */
    QName* getAttName() const;

    /**
      * This method gets a const pointer to the name of the attribute. The
      * form of this name is defined by the validator in use.
      */
    const XMLCh* getName() const;

    /**
      * This method will get a const pointer to the prefix string of this
      * attribute. Since prefixes are optional, it may be zero.
      */
    const XMLCh* getPrefix() const;

    /**
      * This method will get the QName of this attribute, which will be the
      * prefix if any, then a colon, then the base name. If there was no
      * prefix, its the same as the getName() method.
      */
    const XMLCh* getQName() const;

    /**
      * This method will get the specified flag, which indicates whether
      * the attribute was explicitly specified or just faulted in.
      */
    bool getSpecified() const;

    /**
      * This method will get the type of the attribute. The available types
      * are defined by the XML specification.
      */
    XMLAttDef::AttTypes getType() const;

    /**
      * This method will get the value of the attribute. The value can be
      * be an empty string, but never null if the object is correctly
      * set up.
      */
    const XMLCh* getValue() const;

    /**
      * This method will get the id of the URI that this attribute's prefix
      * mapped to. If namespaces are not on, then its value is meaningless.
      */
    unsigned int getURIId() const;

    //@}


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------

    /** @name Setter methods */
    //@{

    /**
      * This method is called to set up a default constructed object after
      * the fact, or to reuse a previously used object.
      *
      * @param  uriId       The id into the validator's URI pool of the URI
      *                     that the prefix mapped to. Only used if namespaces
      *                     are enabled/supported.
      *
      * @param  attrName    The base name of the attribute, i.e. the part
      *                     after any prefix.
      *
      * @param  attrPrefix  The prefix, if any, of this attribute's name. If
      *                     this is empty, then uriID is meaningless as well.
      *
      * @param  attrValue   The value string of the attribute, which should
      *                     be fully normalized by XML rules according to the
      *                     attribute type.
      *
      * @param  type        The type of the attribute. This will indicate
      *                     the type of normalization done and constrains
      *                     the value content. Make sure that the value
      *                     set meets the constraints!
      * @param datatypeValidator type used to validate the attribute, 
      *                         if it was validated by an XML Schema
      * @param isSchema         true if and only if this attribute was validated
      *                         by an XML Schema
      *
      */
    void set
    (
        const   unsigned int        uriId
        , const XMLCh* const        attrName
        , const XMLCh* const        attrPrefix
        , const XMLCh* const        attrValue
        , const XMLAttDef::AttTypes type = XMLAttDef::CData
        , DatatypeValidator * datatypeValidator = 0
        , const bool isSchema = false
    );

    /**
      * This method is called to set up a default constructed object after
      * the fact, or to reuse a previously used object.
      *
      * @param  uriId       The id into the validator's URI pool of the URI
      *                     that the prefix mapped to. Only used if namespaces
      *                     are enabled/supported.
      *
      * @param  attrRawName The raw name of the attribute.
      *
      * @param  attrValue   The value string of the attribute, which should
      *                     be fully normalized by XML rules according to the
      *                     attribute type.
      *
      * @param  type        The type of the attribute. This will indicate
      *                     the type of normalization done and constrains
      *                     the value content. Make sure that the value
      *                     set meets the constraints!
      * @param datatypeValidator type used to validate the attribute, 
      *                         if it was validated by an XML Schema
      * @param isSchema         true if and only if this attribute was validated
      *                         by an XML Schema
      */
    void set
    (
        const   unsigned int        uriId
        , const XMLCh* const        attrRawName
        , const XMLCh* const        attrValue
        , const XMLAttDef::AttTypes type = XMLAttDef::CData
        , DatatypeValidator * datatypeValidator = 0
        , const bool isSchema = false
    );

    /**
      * This method will update just the name related fields of the
      * attribute object. The other fields are left as is.
      *
      * @param  uriId       The id into the validator's URI pool of the URI
      *                     that the prefix mapped to. Only used if namespaces
      *                     are enabled/supported.
      *
      * @param  attrName    The base name of the attribute, i.e. the part
      *                     after any prefix.
      *
      * @param  attrPrefix  The prefix, if any, of this attribute's name. If
      *                     this is empty, then uriID is meaningless as well.
      */
    void setName
    (
        const   unsigned int        uriId
        , const XMLCh* const        attrName
        , const XMLCh* const        attrPrefix
    );

    /**
      * This method will update the specified state of the object.
      *
      * @param  newValue    Indicates whether the attribute was explicitly
      *                     specified or not. If not, then it was faulted
      *                     in from a FIXED or DEFAULT value.
      */
    void setSpecified(const bool newValue);

    /**
      * This method will update the attribute type of the object.
      *
      * @param  newType     The type of the attribute. This will indicate
      *                     the type of normalization done and constrains
      *                     the value content. Make sure that the value
      *                     set meets the constraints!
      */
    void setType(const XMLAttDef::AttTypes newType);

    /**
      * This method will update the value field of the attribute.
      *
      * @param  newValue    The value string of the attribute, which should
      *                     be fully normalized by XML rules according to the
      *                     attribute type.
      */
    void setValue(const XMLCh* const newValue);

    /**
      * This method will set the URI id field of this attribute. This is
      * generally only ever called internally by the parser itself during
      * the parsing process.
      *
      * @param  uriId       The uriId of the attribute.
      */
    void setURIId(const unsigned int uriId);

    //@}



private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLAttr(const XMLAttr&);
    XMLAttr& operator=(const XMLAttr&);


    // -----------------------------------------------------------------------
    //  Private, helper methods
    // -----------------------------------------------------------------------
    void cleanUp();


    // -----------------------------------------------------------------------
    //  Private instance variables
    //
    //  fAttName
    //      The Attribute Name;
    //
    //  fSpecified
    //      True if this attribute appeared in the element; else, false if
    //      it was defaulted from an AttDef.
    //
    //  fType
    //      The attribute type enum value for this attribute. Indicates what
    //      type of attribute it was.
    //
    //  fValue
    //  fValueBufSz
    //      The attribute value that was given in the attribute instance, and
    //      its current buffer size (minus one, where the null is.)
    //
    //  fMemoryManager
    //      The memory manager used for dynamic memory allocation/deallocation
    // -----------------------------------------------------------------------
    bool                fSpecified;
    XMLAttDef::AttTypes fType;
    XMLSize_t           fValueBufSz;
    XMLCh*              fValue;
    QName*              fAttName;
    MemoryManager*      fMemoryManager;   
};

// ---------------------------------------------------------------------------
//  XMLAttr: Constructors and Destructor
// ---------------------------------------------------------------------------
inline XMLAttr::~XMLAttr()
{
    cleanUp();
}


// ---------------------------------------------------------------------------
//  XMLAttr: Getter methods
// ---------------------------------------------------------------------------
inline QName* XMLAttr::getAttName() const
{
    return fAttName;
}

inline const XMLCh* XMLAttr::getName() const
{
    return fAttName->getLocalPart();
}

inline const XMLCh* XMLAttr::getPrefix() const
{
    return fAttName->getPrefix();
}

inline bool XMLAttr::getSpecified() const
{
    return fSpecified;
}

inline XMLAttDef::AttTypes XMLAttr::getType() const
{
    return fType;
}

inline const XMLCh* XMLAttr::getValue() const
{
    return fValue;
}

inline unsigned int XMLAttr::getURIId() const
{
    return fAttName->getURI();
}

// ---------------------------------------------------------------------------
//  XMLAttr: Setter methods
// ---------------------------------------------------------------------------
inline void XMLAttr::set(const  unsigned int        uriId
                        , const XMLCh* const        attrName
                        , const XMLCh* const        attrPrefix
                        , const XMLCh* const        attrValue
                        , const XMLAttDef::AttTypes type
                        , DatatypeValidator * /*datatypeValidator */
                        , const bool /*isSchema*/ )
{
    // Set the name info and the value via their respective calls
    fAttName->setName(attrPrefix, attrName, uriId);
    setValue(attrValue);

    // And store the type
    fType = type;
}

inline void XMLAttr::set(const  unsigned int        uriId
                        , const XMLCh* const        attrRawName
                        , const XMLCh* const        attrValue
                        , const XMLAttDef::AttTypes type
                        , DatatypeValidator * /*datatypeValidator */
                        , const bool /*isSchema*/ )
{
    // Set the name info and the value via their respective calls
    fAttName->setName(attrRawName, uriId);
    setValue(attrValue);

    // And store the type
    fType = type;
}

inline void XMLAttr::setType(const XMLAttDef::AttTypes newValue)
{
    fType = newValue;
}

inline void XMLAttr::setSpecified(const bool newValue)
{
    fSpecified = newValue;
}

XERCES_CPP_NAMESPACE_END

#endif
