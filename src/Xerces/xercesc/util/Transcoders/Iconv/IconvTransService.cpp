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
 * $Id: IconvTransService.cpp 695885 2008-09-16 14:00:19Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------

#if HAVE_CONFIG_H
#	include <config.h>
#endif

#if HAVE_WCHAR_H
#	include <wchar.h>
#endif
#if HAVE_WCTYPE_H
#	include <wctype.h>
#endif

// Fill in for broken or missing wctype functions on some platforms
#if !HAVE_TOWUPPER
#	include <towupper.h>
#endif
#if !HAVE_TOWLOWER
#	include <towlower.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "IconvTransService.hpp"
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/framework/MemoryManager.hpp>


XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local, const data
// ---------------------------------------------------------------------------
static const int    gTempBuffArraySize = 1024;
static const XMLCh  gMyServiceId[] =
{
    chLatin_I, chLatin_C, chLatin_o, chLatin_n, chLatin_v, chNull
};

// ---------------------------------------------------------------------------
// the following is defined by 'man mbrtowc':
// ---------------------------------------------------------------------------
static const size_t TRANSCODING_ERROR = (size_t)(-1);

// ---------------------------------------------------------------------------
//  Local methods
// ---------------------------------------------------------------------------
static unsigned int getWideCharLength(const XMLCh* const src)
{
    if (!src)
        return 0;

    unsigned int len = 0;
    const XMLCh* pTmp = src;
    while (*pTmp++)
        len++;
    return len;
}



// ---------------------------------------------------------------------------
//  IconvTransService: Constructors and Destructor
// ---------------------------------------------------------------------------
IconvTransService::IconvTransService(MemoryManager* /* manager */)
{
}

IconvTransService::~IconvTransService()
{
}


// ---------------------------------------------------------------------------
//  IconvTransService: The virtual transcoding service API
// ---------------------------------------------------------------------------
int IconvTransService::compareIString(  const   XMLCh* const    comp1
                                        , const XMLCh* const    comp2)
{
    const XMLCh* cptr1 = comp1;
    const XMLCh* cptr2 = comp2;

    while ( (*cptr1 != 0) && (*cptr2 != 0) )
    {
        wint_t wch1 = towupper(*cptr1);
        wint_t wch2 = towupper(*cptr2);
        if (wch1 != wch2)
            break;

        cptr1++;
        cptr2++;
    }
    return (int) ( towupper(*cptr1) - towupper(*cptr2) );
}


int IconvTransService::compareNIString( const   XMLCh* const    comp1
                                        , const XMLCh* const    comp2
                                        , const XMLSize_t       maxChars)
{
    unsigned int  n = 0;
    const XMLCh* cptr1 = comp1;
    const XMLCh* cptr2 = comp2;

    while (true && maxChars)
    {
        wint_t wch1 = towupper(*cptr1);
        wint_t wch2 = towupper(*cptr2);

        if (wch1 != wch2)
            return (int) (wch1 - wch2);

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


const XMLCh* IconvTransService::getId() const
{
    return gMyServiceId;
}

XMLLCPTranscoder* IconvTransService::makeNewLCPTranscoder(MemoryManager* manager)
{
    // Just allocate a new transcoder of our type
    return new (manager) IconvLCPTranscoder;
}

bool IconvTransService::supportsSrcOfs() const
{
    return true;
}


// ---------------------------------------------------------------------------
//  IconvTransService: The protected virtual transcoding service API
// ---------------------------------------------------------------------------
XMLTranscoder*
IconvTransService::makeNewXMLTranscoder(const   XMLCh* const
                                        ,       XMLTransService::Codes& resValue
                                        , const XMLSize_t
                                        ,       MemoryManager* const)
{
    //
    //  NOTE: We don't use the block size here
    //
    //  This is a minimalist transcoding service, that only supports a local
    //  default transcoder. All named encodings return zero as a failure,
    //  which means that only the intrinsic encodings supported by the parser
    //  itself will work for XML data.
    //
    resValue = XMLTransService::UnsupportedEncoding;
    return 0;
}


void IconvTransService::upperCase(XMLCh* const toUpperCase)
{
    XMLCh* outPtr = toUpperCase;
    while (*outPtr)
    {
        *outPtr = towupper(*outPtr);
        outPtr++;
    }
}


void IconvTransService::lowerCase(XMLCh* const toLowerCase)
{
    XMLCh* outPtr = toLowerCase;
    while (*outPtr)
    {
        *outPtr = towlower(*outPtr);
        outPtr++;
    }
}


// ---------------------------------------------------------------------------
//  IconvLCPTranscoder: The virtual transcoder API
// ---------------------------------------------------------------------------
XMLSize_t IconvLCPTranscoder::calcRequiredSize(const char* const srcText
                                                  , MemoryManager* const)
{
    if (!srcText)
        return 0;

    XMLSize_t len = 0;
    const char *src = srcText;
#if HAVE_MBRLEN
    mbstate_t st;
    memset(&st, 0, sizeof(st));
#endif
    for ( ; *src; ++len)
    {
#if HAVE_MBRLEN
        int l=::mbrlen( src, MB_CUR_MAX, &st );
#else
        int l=::mblen( src, MB_CUR_MAX );
#endif
        if( l == TRANSCODING_ERROR )
            return 0;
        src += l;
    }
    return len;
}


XMLSize_t IconvLCPTranscoder::calcRequiredSize(const XMLCh* const srcText
                                                  , MemoryManager* const manager)
{
    if (!srcText)
        return 0;

    XMLSize_t     wLent = getWideCharLength(srcText);
    wchar_t       tmpWideCharArr[gTempBuffArraySize];
    wchar_t*      allocatedArray = 0;
    wchar_t*      wideCharBuf = 0;

    if (wLent >= gTempBuffArraySize)
        wideCharBuf = allocatedArray = (wchar_t*)
            manager->allocate
            (
                (wLent + 1) * sizeof(wchar_t)
            );//new wchar_t[wLent + 1];
    else
        wideCharBuf = tmpWideCharArr;

    for (XMLSize_t i = 0; i < wLent; i++)
    {
        wideCharBuf[i] = srcText[i];
    }
    wideCharBuf[wLent] = 0x00;

    const XMLSize_t retVal = ::wcstombs(NULL, wideCharBuf, 0);

    if (allocatedArray)
      manager->deallocate(allocatedArray);

    if (retVal == ~0)
        return 0;
    return retVal;
}


bool IconvLCPTranscoder::transcode( const   XMLCh* const    toTranscode
                                    ,       char* const     toFill
                                    , const XMLSize_t       maxBytes
                                    , MemoryManager* const  manager)
{
    // Watch for a couple of pyscho corner cases
    if (!toTranscode || !maxBytes)
    {
        toFill[0] = 0;
        return true;
    }

    if (!*toTranscode)
    {
        toFill[0] = 0;
        return true;
    }

    unsigned int  wLent = getWideCharLength(toTranscode);
    wchar_t       tmpWideCharArr[gTempBuffArraySize];
    wchar_t*      allocatedArray = 0;
    wchar_t*      wideCharBuf = 0;

    if (wLent > maxBytes) {
        wLent = maxBytes;
    }

    if (maxBytes >= gTempBuffArraySize) {
        wideCharBuf = allocatedArray = (wchar_t*)
            manager->allocate
            (
                (maxBytes + 1) * sizeof(wchar_t)
            );//new wchar_t[maxBytes + 1];
    }
    else
        wideCharBuf = tmpWideCharArr;

    for (unsigned int i = 0; i < wLent; i++)
    {
        wideCharBuf[i] = toTranscode[i];
    }
    wideCharBuf[wLent] = 0x00;

    // Ok, go ahead and try the transcoding. If it fails, then ...
    size_t mblen = ::wcstombs(toFill, wideCharBuf, maxBytes);
    if (mblen == (size_t)-1)
    {
        if (allocatedArray)
          manager->deallocate(allocatedArray);
        return false;
    }

    // Cap it off just in case
    toFill[mblen] = 0;

    if (allocatedArray)
      manager->deallocate(allocatedArray);

    return true;
}


bool IconvLCPTranscoder::transcode( const   char* const     toTranscode
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

    XMLSize_t     len = calcRequiredSize(toTranscode);
    wchar_t       tmpWideCharArr[gTempBuffArraySize];
    wchar_t*      allocatedArray = 0;
    wchar_t*      wideCharBuf = 0;

    if (len > maxChars) {
        len = maxChars;
    }

    if (maxChars >= gTempBuffArraySize)
        wideCharBuf = allocatedArray = (wchar_t*) manager->allocate
        (
            (maxChars + 1) * sizeof(wchar_t)
        );//new wchar_t[maxChars + 1];
    else
        wideCharBuf = tmpWideCharArr;

    if (::mbstowcs(wideCharBuf, toTranscode, maxChars) == (size_t)-1)
    {
        if (allocatedArray)
          manager->deallocate(allocatedArray);
        return false;
    }

    for (XMLSize_t i = 0; i < len; i++)
    {
        toFill[i] = (XMLCh) wideCharBuf[i];
    }
    toFill[len] = 0x00;

    if (allocatedArray)
      manager->deallocate(allocatedArray);

    return true;
}


template <typename T>
void reallocString(T *&ref, size_t &size, MemoryManager* const manager, bool releaseOld)
{
    T *tmp = (T*)manager->allocate(2 * size * sizeof(T));
    memcpy(tmp, ref, size * sizeof(T));
    if (releaseOld) manager->deallocate(ref);
    ref = tmp;
    size *= 2;
}


char* IconvLCPTranscoder::transcode(const XMLCh* const toTranscode,
                                    MemoryManager* const manager)
{
    if (!toTranscode)
        return 0;
    size_t srcCursor = 0, dstCursor = 0;
    size_t resultSize = gTempBuffArraySize;
    char localBuffer[gTempBuffArraySize];
    char* resultString = localBuffer;

#if HAVE_WCSRTOMBS
    mbstate_t st;
    memset(&st, 0, sizeof(st));
    wchar_t srcBuffer[gTempBuffArraySize];
    srcBuffer[gTempBuffArraySize - 1] = 0;
    const wchar_t *src = 0;

    while (toTranscode[srcCursor] || src)
    {
        if (src == 0) // copy a piece of the source string into a local
                      // buffer, converted to wchar_t and NULL-terminated.
                      // after that, src points to the beginning of the
                      // local buffer and is used for the call to ::wcsrtombs
        {
            size_t i;
            for (i=0; i<gTempBuffArraySize-1; ++i)
            {
                srcBuffer[i] = toTranscode[srcCursor];
                if (srcBuffer[i] == '\0')
                    break;
                ++srcCursor;
            }
            src = srcBuffer;
        }

        size_t len = ::wcsrtombs(resultString + dstCursor, &src, resultSize - dstCursor, &st);
        if (len == TRANSCODING_ERROR)
        {
            dstCursor = 0;
            break;
        }
        dstCursor += len;
        if (src != 0) // conversion not finished. This *always* means there
                      // was not enough room in the destination buffer.
        {
            reallocString<char>(resultString, resultSize, manager, resultString != localBuffer);
        }
    }
#else
    while (toTranscode[srcCursor])
    {
        char mbBuf[16]; // MB_CUR_MAX is not defined as a constant on some platforms
        int len = wctomb(mbBuf, toTranscode[srcCursor++]);
        if (len < 0)
        {
            dstCursor = 0;
            break;
        }
        if (dstCursor + len >= resultSize - 1)
            reallocString<char>(resultString, resultSize, manager, resultString != localBuffer);
        for (int j=0; j<len; ++j)
            resultString[dstCursor++] = mbBuf[j];
    }
#endif

    if (resultString == localBuffer)
    {
        resultString = (char*)manager->allocate((dstCursor + 1) * sizeof(char));
        memcpy(resultString, localBuffer, dstCursor * sizeof(char));
    }

    resultString[dstCursor] = '\0';
    return resultString;
}

XMLCh* IconvLCPTranscoder::transcode(const char* const toTranscode,
                                     MemoryManager* const manager)
{
    if (!toTranscode)
        return 0;
    size_t resultSize = gTempBuffArraySize;
    size_t srcCursor = 0, dstCursor = 0;

#if HAVE_MBSRTOWCS
    wchar_t localBuffer[gTempBuffArraySize];
    wchar_t *tmpString = localBuffer;

    mbstate_t st;
    memset(&st, 0, sizeof(st));
    const char *src = toTranscode;

    while(true)
    {
        size_t len = ::mbsrtowcs(tmpString + dstCursor, &src, resultSize - dstCursor, &st);
        if (len == TRANSCODING_ERROR)
        {
            dstCursor = 0;
            break;
        }
        dstCursor += len;
        if (src == 0) // conversion finished
            break;
        if (dstCursor >= resultSize - 1)
            reallocString<wchar_t>(tmpString, resultSize, manager, tmpString != localBuffer);
    }
    // make a final copy, converting from wchar_t to XMLCh:
    XMLCh* resultString = (XMLCh*)manager->allocate((dstCursor + 1) * sizeof(XMLCh));
    size_t i;
    for (i=0; i<dstCursor; ++i)
        resultString[i] = tmpString[i];
    if (tmpString != localBuffer) // did we allocate something?
        manager->deallocate(tmpString);
#else
    XMLCh localBuffer[gTempBuffArraySize];
    XMLCh* resultString = localBuffer;
    size_t srcLen = strlen(toTranscode);

    while(srcLen > srcCursor)
    {
        wchar_t wcBuf[1];
        int len = mbtowc(wcBuf, toTranscode + srcCursor, srcLen - srcCursor);
        if (len <= 0)
        {
            if (len < 0)
                dstCursor = 0;
            break;
        }
        srcCursor += len;
        if (dstCursor + 1 >= resultSize - 1)
            reallocString<XMLCh>(resultString, resultSize, manager, resultString != localBuffer);
        resultString[dstCursor++] = wcBuf[0];
    }

    if (resultString == localBuffer)
    {
        resultString = (XMLCh*)manager->allocate((dstCursor + 1) * sizeof(XMLCh));
        memcpy(resultString, localBuffer, dstCursor * sizeof(XMLCh));
    }
#endif

    resultString[dstCursor] = L'\0';
    return resultString;
}


// ---------------------------------------------------------------------------
//  IconvLCPTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
IconvLCPTranscoder::IconvLCPTranscoder()
{
}

IconvLCPTranscoder::~IconvLCPTranscoder()
{
}

XERCES_CPP_NAMESPACE_END
