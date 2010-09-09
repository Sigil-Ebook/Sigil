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
 * $Id: TransENameMap.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_TRANSENAMEMAP_HPP)
#define XERCESC_INCLUDE_GUARD_TRANSENAMEMAP_HPP

#include <xercesc/util/TransService.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
//  This class is really private to the TransService class. However, some
//  compilers are too dumb to allow us to hide this class there in the Cpp
//  file that uses it.
//
class ENameMap : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Destructor
    // -----------------------------------------------------------------------
    virtual ~ENameMap()
    {
        //delete [] fEncodingName;
        XMLPlatformUtils::fgMemoryManager->deallocate(fEncodingName);
    }



    // -----------------------------------------------------------------------
    //  Virtual factory method
    // -----------------------------------------------------------------------
    virtual XMLTranscoder* makeNew
    (
        const   XMLSize_t       blockSize
        , MemoryManager*  const manager = XMLPlatformUtils::fgMemoryManager
    )   const = 0;


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const XMLCh* getKey() const
    {
        return fEncodingName;
    }


protected :
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    ENameMap(const XMLCh* const encodingName) :
          fEncodingName(XMLString::replicate(encodingName, XMLPlatformUtils::fgMemoryManager))
    {
    }


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ENameMap();
    ENameMap(const ENameMap&);
    ENameMap& operator=(const ENameMap&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fEncodingName
    //      This is the encoding name for the transcoder that is controlled
    //      by this map instance.
    // -----------------------------------------------------------------------
    XMLCh*  fEncodingName;
};


template <class TType> class ENameMapFor : public ENameMap
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ENameMapFor(const XMLCh* const encodingName);
    ~ENameMapFor();


    // -----------------------------------------------------------------------
    //  Implementation of virtual factory method
    // -----------------------------------------------------------------------
    virtual XMLTranscoder* makeNew(const XMLSize_t      blockSize,
                                   MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ENameMapFor();
    ENameMapFor(const ENameMapFor<TType>&);
    ENameMapFor<TType>& operator=(const ENameMapFor<TType>&);
};


template <class TType> class EEndianNameMapFor : public ENameMap
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    EEndianNameMapFor(const XMLCh* const encodingName, const bool swapped);
    ~EEndianNameMapFor();


    // -----------------------------------------------------------------------
    //  Implementation of virtual factory method
    // -----------------------------------------------------------------------
    virtual XMLTranscoder* makeNew(const XMLSize_t      blockSize,
                                   MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) const;


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    EEndianNameMapFor(const EEndianNameMapFor<TType>&);
    EEndianNameMapFor<TType>& operator=(const EEndianNameMapFor<TType>&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSwapped
    //      Indicates whether the endianness of the encoding is opposite of
    //      that of the local host.
    // -----------------------------------------------------------------------
    bool    fSwapped;
};

XERCES_CPP_NAMESPACE_END

#if !defined(XERCES_TMPLSINC)
#include <xercesc/util/TransENameMap.c>
#endif

#endif
