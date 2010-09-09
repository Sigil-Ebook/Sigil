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
 * $Id: XMLNotationDecl.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLNOTATIONDECL_HPP)
#define XERCESC_INCLUDE_GUARD_XMLNOTATIONDECL_HPP

#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/internal/XSerializable.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
 *  This class represents the core information about a notation declaration
 *  that all validators must at least support. Each validator will create a
 *  derivative of this class which adds any information it requires for its
 *  own extra needs.
 *
 *  At this common level, the information supported is the notation name
 *  and the public and sysetm ids indicated in the notation declaration.
 */
class XMLPARSER_EXPORT XMLNotationDecl : public XSerializable, public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    /** @name Constructors */
    //@{
    XMLNotationDecl(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    XMLNotationDecl
    (
        const   XMLCh* const    notName
        , const XMLCh* const    pubId
        , const XMLCh* const    sysId
        , const XMLCh* const    baseURI = 0
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );
    //@}

    /** @name Destructor */
    //@{
    ~XMLNotationDecl();
    //@}


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    XMLSize_t getId() const;
    const XMLCh* getName() const;
    const XMLCh* getPublicId() const;
    const XMLCh* getSystemId() const;
    const XMLCh* getBaseURI() const;
    unsigned int getNameSpaceId() const;
    MemoryManager* getMemoryManager() const;


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    void setId(const XMLSize_t newId);
    void setName
    (
        const   XMLCh* const    notName
    );
    void setPublicId(const XMLCh* const newId);
    void setSystemId(const XMLCh* const newId);
    void setBaseURI(const XMLCh* const newId);
    void setNameSpaceId(const unsigned int newId);

    // -----------------------------------------------------------------------
    //  Support named collection element semantics
    // -----------------------------------------------------------------------
    const XMLCh* getKey() const;

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(XMLNotationDecl)

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLNotationDecl(const XMLNotationDecl&);
    XMLNotationDecl& operator=(const XMLNotationDecl&);


    // -----------------------------------------------------------------------
    //  XMLNotationDecl: Private helper methods
    // -----------------------------------------------------------------------
    void cleanUp();


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fId
    //      This is the unique id given to this notation decl.
    //
    //  fName
    //      The notation's name, which identifies the type of notation it
    //      applies to.
    //
    //  fPublicId
    //      The text of the notation's public id, if any.
    //
    //  fSystemId
    //      The text of the notation's system id, if any.
    //
    //  fBaseURI
    //      The text of the notation's base URI
    // -----------------------------------------------------------------------
    XMLSize_t       fId;
    unsigned int    fNameSpaceId;
	XMLCh*          fName;
    XMLCh*          fPublicId;
    XMLCh*          fSystemId;
    XMLCh*          fBaseURI;
    MemoryManager*  fMemoryManager;
};


// -----------------------------------------------------------------------
//  Getter methods
// -----------------------------------------------------------------------
inline XMLSize_t XMLNotationDecl::getId() const
{
    return fId;
}

inline const XMLCh* XMLNotationDecl::getName() const
{
    return fName;
}

inline unsigned int XMLNotationDecl::getNameSpaceId() const
{
    return fNameSpaceId;
}

inline const XMLCh* XMLNotationDecl::getPublicId() const
{
    return fPublicId;
}

inline const XMLCh* XMLNotationDecl::getSystemId() const
{
    return fSystemId;
}

inline const XMLCh* XMLNotationDecl::getBaseURI() const
{
    return fBaseURI;
}

inline MemoryManager* XMLNotationDecl::getMemoryManager() const
{
    return fMemoryManager;
}

// -----------------------------------------------------------------------
//  Setter methods
// -----------------------------------------------------------------------
inline void XMLNotationDecl::setId(const XMLSize_t newId)
{
    fId = newId;
}

inline void XMLNotationDecl::setNameSpaceId(const unsigned int newId)
{
    fNameSpaceId = newId;
}

inline void XMLNotationDecl::setPublicId(const XMLCh* const newId)
{
    if (fPublicId)
        fMemoryManager->deallocate(fPublicId);

    fPublicId = XMLString::replicate(newId, fMemoryManager);
}

inline void XMLNotationDecl::setSystemId(const XMLCh* const newId)
{
    if (fSystemId)
        fMemoryManager->deallocate(fSystemId);

    fSystemId = XMLString::replicate(newId, fMemoryManager);
}

inline void XMLNotationDecl::setBaseURI(const XMLCh* const newId)
{
    if (fBaseURI)
        fMemoryManager->deallocate(fBaseURI);

    fBaseURI = XMLString::replicate(newId, fMemoryManager);
}


// ---------------------------------------------------------------------------
//  XMLNotationDecl: Support named pool element semantics
// ---------------------------------------------------------------------------
inline const XMLCh* XMLNotationDecl::getKey() const
{
    return fName;
}

XERCES_CPP_NAMESPACE_END

#endif
