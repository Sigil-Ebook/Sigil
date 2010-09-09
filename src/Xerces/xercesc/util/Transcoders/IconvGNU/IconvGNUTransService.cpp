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
 * $Id: IconvGNUTransService.cpp 901107 2010-01-20 08:45:02Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <ctype.h>

#include <locale.h>
#include <errno.h>

#if HAVE_ENDIAN_H
  #include <endian.h>
#elif HAVE_MACHINE_ENDIAN_H
  #include <machine/endian.h>
#elif HAVE_ARPA_NAMESER_COMPAT_H
  #include <arpa/nameser_compat.h>
#endif

#define MAX_UCHSIZE 4

//--------------------------------------------------
// Macro-definitions to translate "native unicode"
// characters <-> XMLCh with different host byte order
// and encoding schemas.

# if BYTE_ORDER == LITTLE_ENDIAN
#  define IXMLCh2WC16(x,w)            \
    *(w) = ((*(x)) >> 8) & 0xFF;        \
    *((w)+1) = (*(x)) & 0xFF
#  define IWC162XMLCh(w,x)    *(x) = ((*(w)) << 8) | (*((w)+1))
#  define XMLCh2WC16(x,w)            \
    *(w) = (*(x)) & 0xFF;            \
    *((w)+1) = ((*(x)) >> 8) & 0xFF
#  define WC162XMLCh(w,x)    *(x) = ((*((w)+1)) << 8) | (*(w))

#  define IXMLCh2WC32(x,w)            \
    *(w) = ((*(x)) >> 24) & 0xFF;        \
    *((w)+1) = ((*(x)) >> 16) & 0xFF;    \
    *((w)+2) = ((*(x)) >> 8) & 0xFF;    \
    *((w)+3) = (*(x)) & 0xFF
#  define IWC322XMLCh(w,x)                \
      *(x) = ((*(w)) << 24) | ((*((w)+1)) << 16) |    \
          ((*((w)+2)) << 8) | (*((w)+3))
#  define XMLCh2WC32(x,w)            \
    *((w)+3) = ((*(x)) >> 24) & 0xFF;    \
    *((w)+2) = ((*(x)) >> 16) & 0xFF;    \
    *((w)+1) = ((*(x)) >> 8) & 0xFF;    \
    *(w) = (*(x)) & 0xFF
#  define WC322XMLCh(w,x)                    \
      *(x) = ((*((w)+3)) << 24) | ((*((w)+2)) << 16) |    \
        ((*((w)+1)) << 8) | (*(w))

# else /* BYTE_ORDER != LITTLE_ENDIAN */

#  define XMLCh2WC16(x,w)            \
    *(w) = ((*(x)) >> 8) & 0xFF;        \
    *((w)+1) = (*(x)) & 0xFF
#  define WC162XMLCh(w,x)    *(x) = ((*(w)) << 8) | (*((w)+1))
#  define IXMLCh2WC16(x,w)            \
    *(w) = (*(x)) & 0xFF;            \
    *((w)+1) = ((*(x)) >> 8) & 0xFF
#  define IWC162XMLCh(w,x)    *(x) = ((*((w)+1)) << 8) | (*(w))

#  define XMLCh2WC32(x,w)            \
    *(w) = ((*(x)) >> 24) & 0xFF;        \
    *((w)+1) = ((*(x)) >> 16) & 0xFF;    \
    *((w)+2) = ((*(x)) >> 8) & 0xFF;    \
    *((w)+3) = (*(x)) & 0xFF
#  define WC322XMLCh(w,x)                \
      *(x) = ((*(w)) << 24) | ((*((w)+1)) << 16) |    \
          ((*((w)+2)) << 8) | (*((w)+3))
#  define IXMLCh2WC32(x,w)            \
    *((w)+3) = ((*(x)) >> 24) & 0xFF;    \
    *((w)+2) = ((*(x)) >> 16) & 0xFF;    \
    *((w)+1) = ((*(x)) >> 8) & 0xFF;    \
    *(w) = (*(x)) & 0xFF
#  define IWC322XMLCh(w,x)                    \
      *(x) = ((*((w)+3)) << 24) | ((*((w)+2)) << 16) |    \
        ((*((w)+1)) << 8) | (*(w))
# endif /* BYTE_ORDER == LITTLE_ENDIAN */

#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/Janitor.hpp>
#include "IconvGNUTransService.hpp"


XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
// Description of encoding schemas, supported by iconv()
// ---------------------------------------------------------------------------
typedef struct __IconvGNUEncoding {
    const char*    fSchema;    // schema name
    size_t    fUChSize;    // size of the character
    unsigned int fUBO;        // byte order, relative to the host
} IconvGNUEncoding;

static const IconvGNUEncoding    gIconvGNUEncodings[] = {
    { "UTF-16LE",        2,    LITTLE_ENDIAN },
    { "UTF-16BE",        2,    BIG_ENDIAN },
    { "UCS-2LE",         2,    LITTLE_ENDIAN },
    { "UCS-2BE",         2,    BIG_ENDIAN },
    { "UCS-2-INTERNAL",  2,    BYTE_ORDER },
    { NULL,              0,    0 }
};

// ---------------------------------------------------------------------------
//  Local, const data
// ---------------------------------------------------------------------------
static const unsigned int    gTempBuffArraySize = 4096;
static const XMLCh        gMyServiceId[] =
{
    chLatin_I, chLatin_C, chLatin_o, chLatin_n, chLatin_v, chNull
};


// ---------------------------------------------------------------------------
//  Local methods
// ---------------------------------------------------------------------------
static XMLSize_t getWideCharLength(const XMLCh* const src)
{
    if (!src)
        return 0;

    XMLSize_t len = 0;
    const XMLCh* pTmp = src;
    while (*pTmp++)
        len++;
    return len;
}


//----------------------------------------------------------------------------
// There is implementation of the libiconv for FreeBSD (available through the
// ports collection). The following is a wrapper around the iconv().
//----------------------------------------------------------------------------

IconvGNUWrapper::IconvGNUWrapper (MemoryManager* manager)
    : fUChSize(0), fUBO(LITTLE_ENDIAN),
      fCDTo((iconv_t)-1), fCDFrom((iconv_t)-1), fMutex(manager)
{
}

IconvGNUWrapper::IconvGNUWrapper ( iconv_t    cd_from,
               iconv_t    cd_to,
               size_t    uchsize,
               unsigned int    ubo,
               MemoryManager* manager)
    : fUChSize(uchsize), fUBO(ubo),
      fCDTo(cd_to), fCDFrom(cd_from), fMutex(manager)
{
    if (fCDFrom == (iconv_t) -1 || fCDTo == (iconv_t) -1) {
        XMLPlatformUtils::panic (PanicHandler::Panic_NoTransService);
    }
}

IconvGNUWrapper::~IconvGNUWrapper()
{
}

// Convert "native unicode" character into XMLCh
void    IconvGNUWrapper::mbcToXMLCh (const char *mbc, XMLCh *toRet) const
{
    if (fUBO == BYTE_ORDER) {
        if (fUChSize == sizeof(XMLCh))
            *toRet = *((XMLCh*) mbc);
        else if (fUChSize == 2) {
            WC162XMLCh( mbc, toRet );
        } else {
            WC322XMLCh( mbc, toRet );
        }
    } else {
        if (fUChSize == 2) {
            IWC162XMLCh( mbc, toRet );
        } else {
            IWC322XMLCh( mbc, toRet );
        }
    }
}

// Convert XMLCh into "native unicode" character
void    IconvGNUWrapper::xmlChToMbc (XMLCh xch, char *mbc) const
{
    if (fUBO == BYTE_ORDER) {
        if (fUChSize == sizeof(XMLCh)) {
            memcpy (mbc, &xch, fUChSize);
            return;
        }
        if (fUChSize == 2) {
            XMLCh2WC16( &xch, mbc );
        } else {
            XMLCh2WC32( &xch, mbc );
        }
    } else {
        if (fUChSize == 2) {
            IXMLCh2WC16( &xch, mbc );
        } else {
            IXMLCh2WC32( &xch, mbc );
        }
    }
}

// Return uppercase equivalent for XMLCh
XMLCh IconvGNUWrapper::toUpper (const XMLCh ch)
{
    if (ch <= 0x7F)
        return toupper(ch);

    char    wcbuf[MAX_UCHSIZE * 2];
    xmlChToMbc (ch, wcbuf);

    char    tmpArr[4];
#if ICONV_USES_CONST_POINTER
    const char* ptr = wcbuf;
#else
    char* ptr = wcbuf;
#endif
    size_t    len = fUChSize;
    char    *pTmpArr = tmpArr;
    size_t    bLen = 2;

    if (::iconv (fCDTo, &ptr, &len, &pTmpArr, &bLen) == (size_t) -1)
        return 0;
    tmpArr[1] = toupper (*((unsigned char *)tmpArr));
    *tmpArr = tmpArr[1];
    len = 1;
    pTmpArr = wcbuf;
    bLen = fUChSize;
    ptr = tmpArr;
    if (::iconv (fCDFrom, &ptr, &len, &pTmpArr, &bLen) == (size_t) -1)
        return 0;
    mbcToXMLCh (wcbuf, (XMLCh*) &ch);
    return ch;
}

// Return lowercase equivalent for XMLCh
XMLCh IconvGNUWrapper::toLower (const XMLCh ch)
{
    if (ch <= 0x7F)
        return tolower(ch);

    char    wcbuf[MAX_UCHSIZE * 2];
    xmlChToMbc (ch, wcbuf);

    char    tmpArr[4];
#if ICONV_USES_CONST_POINTER
    const char* ptr = wcbuf;
#else
    char* ptr = wcbuf;
#endif
    size_t    len = fUChSize;
    char    *pTmpArr = tmpArr;
    size_t    bLen = 2;

    if (::iconv (fCDTo, &ptr, &len, &pTmpArr, &bLen) == (size_t) -1)
        return 0;
    tmpArr[1] = tolower (*((unsigned char*)tmpArr));
    *tmpArr = tmpArr[1];
    len = 1;
    pTmpArr = wcbuf;
    bLen = fUChSize;
    ptr = tmpArr;
    if (::iconv (fCDFrom, &ptr, &len, &pTmpArr, &bLen) == (size_t) -1)
        return 0;
    mbcToXMLCh (wcbuf, (XMLCh*) &ch);
    return ch;
}

// Fill array of XMLCh characters with data, supplyed in the array
// of "native unicode" characters.
XMLCh*    IconvGNUWrapper::mbsToXML
(
    const char*      mbs_str
    ,      XMLCh*    xml_str
    ,      size_t    cnt
) const
{
    if (mbs_str == NULL || xml_str == NULL || cnt == 0)
        return NULL;
    if (fUBO == BYTE_ORDER) {
        if (fUChSize == sizeof(XMLCh)) {
            // null-transformation
            memcpy (xml_str, mbs_str, fUChSize * cnt);
            return xml_str;
        }
        if (fUChSize == 2)
            for (size_t i = 0; i < cnt; i++, mbs_str += fUChSize) {
                WC162XMLCh( mbs_str, xml_str + i);
            }
        else
            for (size_t i = 0; i < cnt; i++, mbs_str += fUChSize) {
                WC322XMLCh( mbs_str, xml_str + i );
            }
    } else {
        if (fUChSize == 2)
            for (size_t i = 0; i < cnt; i++, mbs_str += fUChSize) {
                IWC162XMLCh( mbs_str, xml_str + i );
            }
        else
            for (size_t i = 0; i < cnt; i++, mbs_str += fUChSize) {
                IWC322XMLCh( mbs_str, xml_str + i );
            }
    }
    return xml_str;
}

// Fill array of "native unicode" characters with data, supplyed
// in the array of XMLCh characters.
char*    IconvGNUWrapper::xmlToMbs
(
    const XMLCh*     xml_str
    ,      char*     mbs_str
    ,      size_t    cnt
) const
{
    if (mbs_str == NULL || xml_str == NULL || cnt == 0)
        return NULL;
    char    *toReturn = mbs_str;
    if (fUBO == BYTE_ORDER) {
        if (fUChSize == sizeof(XMLCh)) {
            // null-transformation
            memcpy (mbs_str, xml_str, fUChSize * cnt);
            return toReturn;
        }
        if (fUChSize == 2)
            for (size_t i = 0; i < cnt; i++, mbs_str += fUChSize, xml_str++) {
                XMLCh2WC16( xml_str, mbs_str );
            }
        else
            for (size_t i = 0; i < cnt; i++, mbs_str += fUChSize, xml_str++) {
                XMLCh2WC32( xml_str, mbs_str );
            }
    } else {
        if (fUChSize == 2)
            for (size_t i = 0; i < cnt; i++, mbs_str += fUChSize, xml_str++) {
                IXMLCh2WC16( xml_str, mbs_str );
            }
        else
            for (size_t i = 0; i < cnt; i++, mbs_str += fUChSize, xml_str++) {
                IXMLCh2WC32( xml_str, mbs_str );
            }
    }
    return toReturn;
}

size_t    IconvGNUWrapper::iconvFrom ( const char    *fromPtr,
                 size_t        *fromLen,
                 char        **toPtr,
                 size_t        toLen )
{
#if ICONV_USES_CONST_POINTER
    const char ** tmpPtr = &fromPtr;
#else
    char ** tmpPtr = (char**)&fromPtr;
#endif
    return ::iconv (fCDFrom, tmpPtr, fromLen, toPtr, &toLen);
}

size_t    IconvGNUWrapper::iconvTo ( const char    *fromPtr,
                   size_t        *fromLen,
                   char        **toPtr,
                   size_t        toLen )
{
#if ICONV_USES_CONST_POINTER
    const char ** tmpPtr = &fromPtr;
#else
    char ** tmpPtr = (char**)&fromPtr;
#endif
    return ::iconv (fCDTo, tmpPtr, fromLen, toPtr, &toLen);
}


// ---------------------------------------------------------------------------
//  IconvGNUTransService: Constructors and Destructor
// ---------------------------------------------------------------------------

IconvGNUTransService::IconvGNUTransService(MemoryManager* manager)
    : IconvGNUWrapper(manager), fUnicodeCP(0)
{
    // Try to obtain local (host) characterset from the setlocale
    // and through the environment. Do not call setlocale(LC_*, "")!
    // Using an empty string instead of NULL, will modify the libc
    // behavior.
    //
    const char* fLocalCP = setlocale (LC_CTYPE, NULL);
    if (fLocalCP == NULL || *fLocalCP == 0 ||
        strcmp (fLocalCP, "C") == 0 ||
        strcmp (fLocalCP, "POSIX") == 0) {
      fLocalCP = getenv ("LC_ALL");
      if (fLocalCP == NULL) {
        fLocalCP = getenv ("LC_CTYPE");
        if (fLocalCP == NULL)
          fLocalCP = getenv ("LANG");
      }
    }

    if (fLocalCP == NULL || *fLocalCP == 0 ||
        strcmp (fLocalCP, "C") == 0 ||
        strcmp (fLocalCP, "POSIX") == 0)
        fLocalCP = "iso-8859-1";    // fallback locale
    else {
        const char *ptr = strchr (fLocalCP, '.');
        if (ptr == NULL)
            fLocalCP = "iso-8859-1";    // fallback locale
        else
            fLocalCP = ptr + 1;
    }

    // Select the native unicode characters encoding schema
    const IconvGNUEncoding    *eptr;
    // first - try to use the schema with character size equal to XMLCh, and same endianness
    for (eptr = gIconvGNUEncodings; eptr->fSchema; eptr++)
    {
        if (eptr->fUChSize != sizeof(XMLCh) || eptr->fUBO != BYTE_ORDER)
            continue;

        // try to create conversion descriptor
        iconv_t    cd_to = iconv_open(fLocalCP, eptr->fSchema);
        if (cd_to == (iconv_t)-1)
            continue;
        iconv_t    cd_from = iconv_open(eptr->fSchema, fLocalCP);
        if (cd_from == (iconv_t)-1) {
            iconv_close (cd_to);
            continue;
        }

        // got it
        setUChSize(eptr->fUChSize);
        setUBO(eptr->fUBO);
        setCDTo(cd_to);
        setCDFrom(cd_from);
        fUnicodeCP = eptr->fSchema;
        break;
    }
    if (fUnicodeCP == NULL)
        // try to use any known schema
        for (eptr = gIconvGNUEncodings; eptr->fSchema; eptr++)
        {
            // try to create conversion descriptor
            iconv_t    cd_to = iconv_open(fLocalCP, eptr->fSchema);
            if (cd_to == (iconv_t)-1)
                continue;
            iconv_t    cd_from = iconv_open(eptr->fSchema, fLocalCP);
            if (cd_from == (iconv_t)-1) {
                iconv_close (cd_to);
                continue;
            }

            // got it
            setUChSize(eptr->fUChSize);
            setUBO(eptr->fUBO);
            setCDTo(cd_to);
            setCDFrom(cd_from);
            fUnicodeCP = eptr->fSchema;
            break;
        }

    if (fUnicodeCP == NULL || cdTo() == (iconv_t)-1 || cdFrom() == (iconv_t)-1)
        XMLPlatformUtils::panic (PanicHandler::Panic_NoTransService);
}

IconvGNUTransService::~IconvGNUTransService()
{
    if (cdTo() != (iconv_t) -1) {
        iconv_close (cdTo());
        setCDTo ((iconv_t)-1);
    }
    if (cdFrom() != (iconv_t) -1) {
        iconv_close (cdFrom());
        setCDFrom ((iconv_t)-1);
    }
}

// ---------------------------------------------------------------------------
//  IconvGNUTransService: The virtual transcoding service API
// ---------------------------------------------------------------------------
int IconvGNUTransService::compareIString(const XMLCh* const    comp1
                                        , const XMLCh* const    comp2)
{
    const XMLCh* cptr1 = comp1;
    const XMLCh* cptr2 = comp2;

    XMLMutexLock lockConverter(&fMutex);

    XMLCh    c1 = toUpper(*cptr1);
    XMLCh    c2 = toUpper(*cptr2);
    while ( (*cptr1 != 0) && (*cptr2 != 0) ) {
        if (c1 != c2)
            break;
        c1 = toUpper(*(++cptr1));
        c2 = toUpper(*(++cptr2));

    }
    return (int) ( c1 - c2 );
}


int IconvGNUTransService::compareNIString(const XMLCh* const     comp1
                                         , const XMLCh* const    comp2
                                         , const XMLSize_t       maxChars)
{
    unsigned int  n = 0;
    const XMLCh* cptr1 = comp1;
    const XMLCh* cptr2 = comp2;

    XMLMutexLock lockConverter(&fMutex);

    while (true && maxChars)
    {
        XMLCh    c1 = toUpper(*cptr1);
        XMLCh    c2 = toUpper(*cptr2);

        if (c1 != c2)
            return (int) (c1 - c2);

        // If either ended, then both ended, so equal
        if (!*cptr1 || !*cptr2)
            break;

        cptr1++;
        cptr2++;

        //  Bump the count of chars done. If it equals the count then we
        //  are equal for the requested count, so break out and return
        //  equal.
        n++;
        if (n == maxChars)
            break;
    }

    return 0;
}


const XMLCh* IconvGNUTransService::getId() const
{
    return gMyServiceId;
}

XMLLCPTranscoder* IconvGNUTransService::makeNewLCPTranscoder(MemoryManager* manager)
{
    return new (manager) IconvGNULCPTranscoder (cdFrom(), cdTo(), uChSize(), UBO(), manager);
}

bool IconvGNUTransService::supportsSrcOfs() const
{
    return true;
}

// ---------------------------------------------------------------------------
//  IconvGNUTransService: The protected virtual transcoding service API
// ---------------------------------------------------------------------------
XMLTranscoder*
IconvGNUTransService::makeNewXMLTranscoder
(
    const    XMLCh* const    encodingName
    ,    XMLTransService::Codes&    resValue
    , const    XMLSize_t    blockSize
    ,        MemoryManager* const    manager
)
{
    resValue = XMLTransService::UnsupportedEncoding;
    IconvGNUTranscoder    *newTranscoder = NULL;

    char    *encLocal = XMLString::transcode(encodingName, manager);
    ArrayJanitor<char> janBuf(encLocal, manager);
    iconv_t    cd_from, cd_to;

    cd_from = iconv_open (fUnicodeCP, encLocal);
    if (cd_from == (iconv_t)-1) {
        resValue = XMLTransService::SupportFilesNotFound;
        return NULL;
    }
    cd_to = iconv_open (encLocal, fUnicodeCP);
    if (cd_to == (iconv_t)-1) {
        resValue = XMLTransService::SupportFilesNotFound;
        iconv_close (cd_from);
        return NULL;
    }
    newTranscoder = new (manager) IconvGNUTranscoder (encodingName,
                         blockSize,
                         cd_from, cd_to,
                         uChSize(), UBO(), manager);
    if (newTranscoder)
        resValue = XMLTransService::Ok;
    return newTranscoder;
}

void IconvGNUTransService::upperCase(XMLCh* const toUpperCase)
{
    XMLCh* outPtr = toUpperCase;

    XMLMutexLock lockConverter(&fMutex);

    while (*outPtr)
    {
        *outPtr = toUpper(*outPtr);
        outPtr++;
    }
}

void IconvGNUTransService::lowerCase(XMLCh* const toLowerCase)
{
    XMLCh* outPtr = toLowerCase;

    XMLMutexLock lockConverter(&fMutex);

    while (*outPtr)
    {
        *outPtr = toLower(*outPtr);
        outPtr++;
    }
}

// ---------------------------------------------------------------------------
//  IconvGNULCPTranscoder: The virtual transcoder API
// ---------------------------------------------------------------------------
XMLSize_t IconvGNULCPTranscoder::calcRequiredSize (const char* const srcText
                                         , MemoryManager* const manager)
{
    if (!srcText)
        return 0;

    size_t len, srcLen;
    len = srcLen = strlen(srcText);
    if (len == 0)
        return 0;

    char tmpWideArr[gTempBuffArraySize];
    size_t totalLen = 0;

    XMLMutexLock lockConverter(&fMutex);

    for (;;) {
        char        *pTmpArr = tmpWideArr;
        const char    *ptr = srcText + srcLen - len;
        size_t    rc = iconvFrom(ptr, &len, &pTmpArr, gTempBuffArraySize);
        if (rc == (size_t) -1 && errno != E2BIG) {
            ThrowXMLwithMemMgr(TranscodingException, XMLExcepts::Trans_BadSrcSeq, manager);
            /* return 0; */
        }
        rc = pTmpArr - (char *) tmpWideArr;
        totalLen += rc;
        if (rc == 0 || len == 0)
            break;
    }
    return totalLen / uChSize();
}


XMLSize_t IconvGNULCPTranscoder::calcRequiredSize(const XMLCh* const srcText
                                        , MemoryManager* const manager)
{
    if (!srcText)
        return 0;
    XMLSize_t  wLent = getWideCharLength(srcText);
    if (wLent == 0)
        return 0;

    char    tmpWBuff[gTempBuffArraySize];
    char    *wBuf = 0;
    char    *wBufPtr = 0;
    ArrayJanitor<char>  janBuf(wBufPtr, manager);
    size_t      len = wLent * uChSize();
    if (uChSize() != sizeof(XMLCh) || UBO() != BYTE_ORDER) {
        if (len > gTempBuffArraySize) {
            wBufPtr = (char*) manager->allocate(len * sizeof(char));//new char[len];
            janBuf.reset(wBufPtr, manager);
            wBuf = wBufPtr;
        } else
            wBuf = tmpWBuff;
        xmlToMbs (srcText, wBuf, wLent);
    } else
        wBuf = (char *) srcText;

    char    tmpBuff[gTempBuffArraySize];
    size_t    totalLen = 0;
    char    *srcEnd = wBuf + wLent * uChSize();

    XMLMutexLock lockConverter(&fMutex);

    for (;;) {
        char        *pTmpArr = tmpBuff;
        const char    *ptr = srcEnd - len;
        size_t    rc = iconvTo(ptr, &len, &pTmpArr, gTempBuffArraySize);
        if (rc == (size_t) -1 && errno != E2BIG) {
            ThrowXMLwithMemMgr(TranscodingException, XMLExcepts::Trans_BadSrcSeq, manager);
            /* return 0; */
        }
        rc = pTmpArr - tmpBuff;
        totalLen += rc;
        if (rc == 0 || len == 0)
            break;
    }
    return totalLen;
}


char* IconvGNULCPTranscoder::transcode(const XMLCh* const toTranscode,
                                       MemoryManager* const manager)
{
    if (!toTranscode)
        return 0;

    char* retVal = 0;
    if (!*toTranscode) {
        retVal = (char*) manager->allocate(sizeof(char));//new char[1];
        retVal[0] = 0;
        return retVal;
    }

    XMLSize_t wLent = getWideCharLength(toTranscode);

    // Calc needed size.
    XMLSize_t neededLen = calcRequiredSize (toTranscode, manager);
    if (neededLen == 0)
        return 0;
    // allocate output buffer
    retVal = (char*) manager->allocate((neededLen + 1) * sizeof(char));//new char[neededLen + 1];
    // prepare the original
    char    tmpWBuff[gTempBuffArraySize];
    char    *wideCharBuf = 0;
    char    *wBufPtr = 0;
    ArrayJanitor<char>  janBuf(wBufPtr, manager);
    size_t  len = wLent * uChSize();

    if (uChSize() != sizeof(XMLCh) || UBO() != BYTE_ORDER) {
        if (len > gTempBuffArraySize) {
            wBufPtr = (char*) manager->allocate(len * sizeof(char));//new char[len];
            janBuf.reset(wBufPtr, manager);
            wideCharBuf = wBufPtr;
        } else
            wideCharBuf = tmpWBuff;
        xmlToMbs (toTranscode, wideCharBuf, wLent);
    } else
        wideCharBuf = (char *) toTranscode;

    // perform conversion
    char* ptr = retVal;
    size_t rc;

    {
      XMLMutexLock lockConverter(&fMutex);
      rc = iconvTo(wideCharBuf, &len, &ptr, neededLen);
    }

    if (rc == (size_t)-1) {
        return 0;
    }
    retVal[neededLen] = 0;

    return retVal;
}


bool IconvGNULCPTranscoder::transcode( const   XMLCh* const    toTranscode
                    , char* const        toFill
                    , const XMLSize_t       maxBytes
                    , MemoryManager* const  manager)
{
    // Watch for a couple of pyscho corner cases
    if (!toTranscode || !maxBytes) {
        toFill[0] = 0;
        return true;
    }
    if (!*toTranscode) {
        toFill[0] = 0;
        return true;
    }

    XMLSize_t wLent = getWideCharLength(toTranscode);
    if (wLent > maxBytes)
        wLent = maxBytes;

    // Fill the "unicode" string
    char    tmpWBuff[gTempBuffArraySize];
    char    *wideCharBuf = 0;
    char    *wBufPtr = 0;
    ArrayJanitor<char>  janBuf(wBufPtr, manager);
    size_t  len = wLent * uChSize();

    if (uChSize() != sizeof(XMLCh) || UBO() != BYTE_ORDER) {
        if (len > gTempBuffArraySize) {
            wBufPtr = (char*) manager->allocate(len * sizeof(char));//new char[len];
            janBuf.reset(wBufPtr, manager);
            wideCharBuf = wBufPtr;
        } else
            wideCharBuf = tmpWBuff;
        xmlToMbs (toTranscode, wideCharBuf, wLent);
    } else
        wideCharBuf = (char *) toTranscode;

    // Ok, go ahead and try the transcoding. If it fails, then ...
    char    *ptr = toFill;
    size_t rc;

    {
      XMLMutexLock lockConverter(&fMutex);
      rc = iconvTo(wideCharBuf, &len, &ptr, maxBytes);
    }

    if (rc == (size_t)-1) {
        return false;
    }

    // Cap it off
    *ptr = 0;
    return true;
}


XMLCh* IconvGNULCPTranscoder::transcode(const char* const toTranscode,
                                        MemoryManager* const manager)
{
    if (!toTranscode)
        return 0;

    XMLCh* retVal = 0;
    if (!*toTranscode) {
        retVal = (XMLCh*) manager->allocate(sizeof(XMLCh));//new XMLCh[1];
        retVal[0] = 0;
        return retVal;
    }

    XMLSize_t wLent = calcRequiredSize(toTranscode, manager);
    if (wLent == 0) {
        retVal = (XMLCh*) manager->allocate(sizeof(XMLCh));//new XMLCh[1];
        retVal[0] = 0;
        return retVal;
    }

    char    tmpWBuff[gTempBuffArraySize];
    char    *wideCharBuf = 0;
    char    *wBufPtr = 0;
    ArrayJanitor<char>  janBuf(wBufPtr, manager);
    size_t  len = wLent * uChSize();

    retVal = (XMLCh*) manager->allocate((wLent + 1) * sizeof(XMLCh));//new XMLCh[wLent + 1];
    if (uChSize() != sizeof(XMLCh) || UBO() != BYTE_ORDER) {
        if (len > gTempBuffArraySize) {
            wBufPtr = (char*) manager->allocate(len * sizeof(char));//new char[len];
            janBuf.reset(wBufPtr, manager);
            wideCharBuf = wBufPtr;
        } else
            wideCharBuf = tmpWBuff;
    } else
        wideCharBuf = (char *) retVal;

    size_t    flen = strlen(toTranscode);
    char    *ptr = wideCharBuf;
    size_t rc;

    {
      XMLMutexLock lockConverter(&fMutex);
      rc = iconvFrom(toTranscode, &flen, &ptr, len);
    }

    if (rc == (size_t) -1) {
        return NULL;
    }
    if (uChSize() != sizeof(XMLCh) || UBO() != BYTE_ORDER)
        mbsToXML (wideCharBuf, retVal, wLent);
    retVal[wLent] = 0x00;

    return retVal;
}


bool IconvGNULCPTranscoder::transcode(const   char* const    toTranscode
                       ,       XMLCh* const    toFill
                       , const XMLSize_t       maxChars
                       , MemoryManager* const  manager)
{
    // Check for a couple of psycho corner cases
    if (!toTranscode || !maxChars)
    {
        toFill[0] = 0;
        return true;
    }

    if (!*toTranscode)
    {
        toFill[0] = 0;
        return true;
    }

    XMLSize_t wLent = calcRequiredSize(toTranscode);
    if (wLent > maxChars)
        wLent = maxChars;

    char    tmpWBuff[gTempBuffArraySize];
    char    *wideCharBuf = 0;
    char    *wBufPtr = 0;
    ArrayJanitor<char>  janBuf(wBufPtr, manager);
    size_t    len = wLent * uChSize();

    if (uChSize() != sizeof(XMLCh) || UBO() != BYTE_ORDER) {
        if (len > gTempBuffArraySize) {
            wBufPtr = (char*) manager->allocate(len * sizeof(char));//new char[len];
            janBuf.reset(wBufPtr, manager);
            wideCharBuf = wBufPtr;
        } else
            wideCharBuf = tmpWBuff;
    } else
        wideCharBuf = (char *) toFill;

    size_t    flen = strlen(toTranscode); // wLent;
    char    *ptr = wideCharBuf;
    size_t rc;

    {
      XMLMutexLock lockConverter(&fMutex);
      rc = iconvFrom(toTranscode, &flen, &ptr, len);
    }

    if (rc == (size_t)-1) {
        return false;
    }

    if (uChSize() != sizeof(XMLCh) || UBO() != BYTE_ORDER)
        mbsToXML (wideCharBuf, toFill, wLent);

    toFill[wLent] = 0x00;
    return true;
}


// ---------------------------------------------------------------------------
//  IconvGNULCPTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------


IconvGNULCPTranscoder::IconvGNULCPTranscoder (iconv_t        cd_from,
                        iconv_t        cd_to,
                        size_t        uchsize,
                        unsigned int    ubo,
                        MemoryManager* manager)
    : IconvGNUWrapper (cd_from, cd_to, uchsize, ubo, manager)
{
}


IconvGNULCPTranscoder::~IconvGNULCPTranscoder()
{
}


// ---------------------------------------------------------------------------
//  IconvGNUTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
IconvGNUTranscoder::IconvGNUTranscoder (const    XMLCh* const    encodingName
                      , const XMLSize_t    blockSize
                      ,    iconv_t        cd_from
                      ,    iconv_t        cd_to
                      ,    size_t        uchsize
                      ,    unsigned int    ubo
                      , MemoryManager* const manager
    )
    : XMLTranscoder(encodingName, blockSize, manager)
    , IconvGNUWrapper (cd_from, cd_to, uchsize, ubo, manager)
{
}

IconvGNUTranscoder::~IconvGNUTranscoder()
{
    if (cdTo() != (iconv_t)-1) {
        iconv_close (cdTo());
        setCDTo ((iconv_t)-1);
    }
    if (cdFrom() != (iconv_t)-1) {
        iconv_close (cdFrom());
        setCDFrom ((iconv_t)-1);
    }
}

// ---------------------------------------------------------------------------
//  IconvGNUTranscoder: Implementation of the virtual transcoder API
// ---------------------------------------------------------------------------
XMLSize_t    IconvGNUTranscoder::transcodeFrom
(
    const   XMLByte* const          srcData
    , const XMLSize_t               srcCount
    ,       XMLCh* const            toFill
    , const XMLSize_t               maxChars
    ,       XMLSize_t&              bytesEaten
    ,       unsigned char* const    charSizes )
{
    // Transcode TO XMLCh
    const char*  startSrc = (const char*) srcData;
    const char*  endSrc = (const char*) srcData + srcCount;

    char    tmpWBuff[gTempBuffArraySize];
    char    *startTarget = 0;
    char    *wBufPtr = 0;
    ArrayJanitor<char>  janBuf(wBufPtr, getMemoryManager());
    size_t    len = maxChars * uChSize();

    if (uChSize() != sizeof(XMLCh) || UBO() != BYTE_ORDER) {
        if (len > gTempBuffArraySize) {
            wBufPtr = (char*) getMemoryManager()->allocate(len * sizeof(char));//new char[len];
            janBuf.reset(wBufPtr, getMemoryManager());
            startTarget = wBufPtr;
        } else
            startTarget = tmpWBuff;
    } else
        startTarget = (char *) toFill;

    // Do character-by-character transcoding
    char    *orgTarget = startTarget;
    size_t    srcLen = srcCount;
    size_t    prevSrcLen = srcLen;
    unsigned int toReturn = 0;
    bytesEaten = 0;

    XMLMutexLock lockConverter(&fMutex);

    for (size_t cnt = 0; cnt < maxChars && srcLen; cnt++) {
        size_t    rc = iconvFrom(startSrc, &srcLen, &orgTarget, uChSize());
        if (rc == (size_t)-1) {
            if (errno != E2BIG || prevSrcLen == srcLen) {
                ThrowXMLwithMemMgr(TranscodingException, XMLExcepts::Trans_BadSrcSeq, getMemoryManager());
            }
        }
        charSizes[cnt] = prevSrcLen - srcLen;
        prevSrcLen = srcLen;
        bytesEaten += charSizes[cnt];
        startSrc = endSrc - srcLen;
        toReturn++;
    }
    if (uChSize() != sizeof(XMLCh) || UBO() != BYTE_ORDER)
        mbsToXML (startTarget, toFill, toReturn);
    return toReturn;
}

XMLSize_t    IconvGNUTranscoder::transcodeTo
(
    const   XMLCh* const     srcData
    , const XMLSize_t        srcCount
    ,       XMLByte* const   toFill
    , const XMLSize_t        maxBytes
    ,       XMLSize_t&       charsEaten
    , const UnRepOpts        /*options*/ )
{
    // Transcode FROM XMLCh
    char    tmpWBuff[gTempBuffArraySize];
    char    *startSrc = tmpWBuff;
    char    *wBufPtr = 0;
    ArrayJanitor<char>  janBuf(wBufPtr, getMemoryManager());
    size_t    len = srcCount * uChSize();

    if (uChSize() != sizeof(XMLCh) || UBO() != BYTE_ORDER) {
        if (len > gTempBuffArraySize) {
            wBufPtr = (char*) getMemoryManager()->allocate(len * sizeof(char));//new char[len];
            janBuf.reset(wBufPtr, getMemoryManager());
            startSrc = wBufPtr;
        } else
            startSrc = tmpWBuff;
        xmlToMbs (srcData, startSrc, srcCount);
    } else
        startSrc = (char *) srcData;

    char* startTarget = (char *) toFill;
    size_t srcLen = len;

    size_t rc;

    {
      XMLMutexLock lockConverter(&fMutex);
      rc = iconvTo (startSrc, &srcLen, &startTarget, maxBytes);
    }

    if (rc == (size_t)-1 && errno != E2BIG) {
        ThrowXMLwithMemMgr(TranscodingException, XMLExcepts::Trans_BadSrcSeq, getMemoryManager());
    }
    charsEaten = srcCount - srcLen / uChSize();
    return startTarget - (char *)toFill;
}

bool IconvGNUTranscoder::canTranscodeTo
(
    const unsigned int toCheck
)
{
    //
    //  If the passed value is really a surrogate embedded together, then
    //  we need to break it out into its two chars. Else just one.
    //
    char        srcBuf[MAX_UCHSIZE * 2];
    unsigned int    srcCount = 1;
    if (toCheck & 0xFFFF0000) {
        XMLCh    ch1 = (toCheck >> 10) + 0xD800;
        XMLCh    ch2 = (toCheck & 0x3FF) + 0xDC00;
        xmlToMbs(&ch1, srcBuf, 1);
        xmlToMbs(&ch2, srcBuf + uChSize(), 1);
        srcCount++;
    } else
        xmlToMbs((const XMLCh*) &toCheck, srcBuf, 1);
    size_t    len = srcCount * uChSize();
    char    tmpBuf[64];
    char*    pTmpBuf = tmpBuf;

    XMLMutexLock lockConverter(&fMutex);
    size_t rc = iconvTo( srcBuf, &len, &pTmpBuf, 64);

    return (rc != (size_t)-1) && (len == 0);
}

XERCES_CPP_NAMESPACE_END
