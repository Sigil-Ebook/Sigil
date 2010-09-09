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
 * $Id: IconvGNUTransService.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_ICONVGNUTRANSSERVICE_HPP)
#define XERCESC_INCLUDE_GUARD_ICONVGNUTRANSSERVICE_HPP

#include <xercesc/util/TransService.hpp>
#include <xercesc/util/Mutexes.hpp>

#include <iconv.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Libiconv wrapper (low-level conversion utilities collection)
// ---------------------------------------------------------------------------

class XMLUTIL_EXPORT IconvGNUWrapper
{
public:
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    IconvGNUWrapper
    (
      iconv_t		cd_from,
      iconv_t		cd_to,
      size_t		uchsize,
      unsigned int	ubo,
      MemoryManager* manager
    );
    virtual ~IconvGNUWrapper();

    // Convert "native unicode" character into XMLCh
    void	mbcToXMLCh (const char *mbc, XMLCh *toRet) const;

    // Convert XMLCh into "native unicode" character
    void	xmlChToMbc (XMLCh xch, char *mbc) const;

    // Fill array of XMLCh characters with data, supplied in the array
    // of "native unicode" characters.
    XMLCh*	mbsToXML (
      const char*	mbs_str,
      XMLCh*		xml_str,
      size_t		cnt
    ) const;


    // Fill array of "native unicode" characters with data, supplied
    // in the array of XMLCh characters.
    char*	xmlToMbs
    (
      const XMLCh*	xml_str,
      char*		mbs_str,
      size_t		cnt
    ) const;

    // Private data accessors
    inline iconv_t	cdTo () const { return fCDTo; }
    inline iconv_t	cdFrom () const { return fCDFrom; }
    inline size_t	uChSize () const { return fUChSize; }
    inline unsigned int	UBO () const { return fUBO; }

protected:
    // The following four functions should called with the fMutex
    // locked.
    //

    // Return uppercase equivalent for XMLCh
    XMLCh 	toUpper (const XMLCh ch);

    // Return uppercase equivalent for XMLCh
    XMLCh 	toLower (const XMLCh ch);

    // Wrapper around the iconv() for transcoding from the local charset
    size_t	iconvFrom
    (
      const char	*fromPtr,
      size_t		*fromLen,
      char		**toPtr,
      size_t		toLen
    );

    // Wrapper around the iconv() for transcoding to the local charset
    size_t	iconvTo
    (
      const char	*fromPtr,
      size_t		*fromLen,
      char		**toPtr,
      size_t		toLen
    );

protected:

    // Hidden constructor
    IconvGNUWrapper(MemoryManager* manager);

    // Private data accessors
    inline void	setCDTo (iconv_t cd) { fCDTo = cd; }
    inline void	setCDFrom (iconv_t cd) { fCDFrom = cd; }
    inline void	setUChSize (size_t sz) { fUChSize = sz; }
    inline void	setUBO (unsigned int u) { fUBO = u; }

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    IconvGNUWrapper(const IconvGNUWrapper&);
    IconvGNUWrapper& operator=(const IconvGNUWrapper&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fCDTo
    //	    Characterset conversion descriptor TO the local-host encoding
    //  fCDFrom
    //	    Characterset conversion descriptor FROM the local-host encoding
    //  fUChSize
    //      Sizeof the "native unicode" character in bytes
    //  fUBO
    //      "Native unicode" characters byte order
    // -----------------------------------------------------------------------
    size_t	fUChSize;
    unsigned int fUBO;
    iconv_t	fCDTo;
    iconv_t	fCDFrom;

protected:
    XMLMutex    fMutex;
};



// ---------------------------------------------------------------------------
//  FreeBSD-specific Transcoding Service implementation
// ---------------------------------------------------------------------------

class XMLUTIL_EXPORT IconvGNUTransService : public XMLTransService, IconvGNUWrapper
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    IconvGNUTransService(MemoryManager* manager);
    ~IconvGNUTransService();


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
    virtual void lowerCase(XMLCh* const toUpperCase);

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
    IconvGNUTransService(const IconvGNUTransService&);
    IconvGNUTransService& operator=(const IconvGNUTransService&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fUnicodeCP
    //      Unicode encoding schema name
    // -----------------------------------------------------------------------
    const char*	fUnicodeCP;

};


//----------------------------------------------------------------------------
// Implementation of the transcoders for arbitrary input characterset is
// supported ONLY through libiconv interface
//----------------------------------------------------------------------------

class XMLUTIL_EXPORT IconvGNUTranscoder : public XMLTranscoder, IconvGNUWrapper
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    IconvGNUTranscoder(const	XMLCh* const	encodingName
  		, const XMLSize_t	blockSize
  		,	iconv_t		cd_from
  		,	iconv_t		cd_to
  		,	size_t		uchsize
  		,	unsigned int	ubo
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    ~IconvGNUTranscoder();


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
        const   XMLCh* const	srcData
        , const XMLSize_t	    srcCount
        ,       XMLByte* const	toFill
        , const XMLSize_t	    maxBytes
        ,       XMLSize_t&	    charsEaten
        , const UnRepOpts	    options
    );

    virtual bool canTranscodeTo
    (
        const   unsigned int	toCheck
    );

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    IconvGNUTranscoder();
    IconvGNUTranscoder(const IconvGNUTranscoder&);
    IconvGNUTranscoder& operator=(const IconvGNUTranscoder&);
};


// ---------------------------------------------------------------------------
//  GNU-specific XMLCh <-> local (host) characterset transcoder
// ---------------------------------------------------------------------------

class XMLUTIL_EXPORT IconvGNULCPTranscoder : public XMLLCPTranscoder, IconvGNUWrapper
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    IconvGNULCPTranscoder
    (
      iconv_t		from,
      iconv_t		to,
      size_t		uchsize,
      unsigned int	ubo,
      MemoryManager* manager
    );

protected:
    IconvGNULCPTranscoder();	// Unimplemented

public:

    ~IconvGNULCPTranscoder();


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
    IconvGNULCPTranscoder(const IconvGNULCPTranscoder&);
    IconvGNULCPTranscoder& operator=(const IconvGNULCPTranscoder&);
};

XERCES_CPP_NAMESPACE_END

#endif /* ICONVGNUTRANSSERVICE */


