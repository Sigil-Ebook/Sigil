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
 * $Id: ICUTransService.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_ICUTRANSSERVICE_HPP)
#define XERCESC_INCLUDE_GUARD_ICUTRANSSERVICE_HPP

#include <xercesc/util/Mutexes.hpp>
#include <xercesc/util/TransService.hpp>

struct UConverter;

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT ICUTransService : public XMLTransService
{
public :
    friend class Uniconv390TransService;
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ICUTransService(MemoryManager* manager);
    ~ICUTransService();


    // -----------------------------------------------------------------------
    //  Implementation of the virtual transcoding service API
    // -----------------------------------------------------------------------
    virtual int compareIString
    (
        const   XMLCh* const    comp1
        , const XMLCh* const    comp2
    );

    virtual int compareNIString
    (
        const   XMLCh* const    comp1
        , const XMLCh* const    comp2
        , const XMLSize_t       maxChars
    );

    virtual const XMLCh* getId() const;

    virtual XMLLCPTranscoder* makeNewLCPTranscoder(MemoryManager* manager);

    virtual bool supportsSrcOfs() const;

    virtual void upperCase(XMLCh* const toUpperCase);
    virtual void lowerCase(XMLCh* const toLowerCase);


protected :
    // -----------------------------------------------------------------------
    //  Protected virtual methods
    // -----------------------------------------------------------------------
    virtual XMLTranscoder* makeNewXMLTranscoder
    (
        const   XMLCh* const            encodingName
        ,       XMLTransService::Codes& resValue
        , const XMLSize_t               blockSize
        ,       MemoryManager* const    manager
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ICUTransService(const ICUTransService&);
    ICUTransService& operator=(const ICUTransService&);
};



class XMLUTIL_EXPORT ICUTranscoder : public XMLTranscoder
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ICUTranscoder
    (
        const   XMLCh* const        encodingName
        ,       UConverter* const   toAdopt
        , const XMLSize_t           blockSize
        , MemoryManager* const      manager = XMLPlatformUtils::fgMemoryManager
    );
    ~ICUTranscoder();


    // -----------------------------------------------------------------------
    //  Implementation of the virtual transcoder interface
    // -----------------------------------------------------------------------
    virtual XMLSize_t transcodeFrom
    (
        const   XMLByte* const          srcData
        , const XMLSize_t               srcCount
        ,       XMLCh* const            toFill
        , const XMLSize_t               maxChars
        ,       XMLSize_t&              bytesEaten
        ,       unsigned char* const    charSizes
    );

    virtual XMLSize_t transcodeTo
    (
        const   XMLCh* const    srcData
        , const XMLSize_t       srcCount
        ,       XMLByte* const  toFill
        , const XMLSize_t       maxBytes
        ,       XMLSize_t&      charsEaten
        , const UnRepOpts       options
    );

    virtual bool canTranscodeTo
    (
        const   unsigned int    toCheck
    );



private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ICUTranscoder();
    ICUTranscoder(const ICUTranscoder&);
    ICUTranscoder& operator=(const ICUTranscoder&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fConverter
    //      This is a pointer to the ICU converter that this transcoder
    //      uses.
    //
    //  fFixed
    //      This is set to true if the encoding is a fixed size one. This
    //      can be used to optimize some operations.
    //
    //  fSrcOffsets
    //      This is an array of longs, which are allocated to the size of
    //      the transcoding block (if any) indicated in the ctor. It is used
    //      to get the character offsets from ICU, which are then translated
    //      into an array of char sizes for return.
    // -----------------------------------------------------------------------
    UConverter*     fConverter;
    bool            fFixed;
    XMLUInt32*      fSrcOffsets;
};


class XMLUTIL_EXPORT ICULCPTranscoder : public XMLLCPTranscoder
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    ICULCPTranscoder(UConverter* const toAdopt);
    ~ICULCPTranscoder();


    // -----------------------------------------------------------------------
    //  Implementation of the virtual transcoder interface
    // -----------------------------------------------------------------------
    virtual char* transcode(const XMLCh* const toTranscode,
                            MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    virtual XMLCh* transcode(const char* const toTranscode,
                             MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);


    // -----------------------------------------------------------------------
    //  DEPRECATED old transcode interface
    // -----------------------------------------------------------------------
    virtual XMLSize_t calcRequiredSize(const char* const srcText
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    virtual XMLSize_t calcRequiredSize(const XMLCh* const srcText
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

    virtual bool transcode
    (
        const   char* const     toTranscode
        ,       XMLCh* const    toFill
        , const XMLSize_t       maxChars
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual bool transcode
    (
        const   XMLCh* const    toTranscode
        ,       char* const     toFill
        , const XMLSize_t       maxChars
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );



private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    ICULCPTranscoder();
    ICULCPTranscoder(const ICULCPTranscoder&);
    ICULCPTranscoder& operator=(const ICULCPTranscoder&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fConverter
    //      This is a pointer to the ICU converter that this transcoder
    //      uses.
    //
    //  fMutex
    //      We have to synchronize threaded calls to the converter.
    // -----------------------------------------------------------------------
    UConverter*     fConverter;
    XMLMutex        fMutex;
};

XERCES_CPP_NAMESPACE_END

#endif
