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
 * $Id: ICUTransService.cpp 901107 2010-01-20 08:45:02Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include "ICUTransService.hpp"
#include <string.h>
#include <unicode/uloc.h>
#include <unicode/uchar.h>
#include <unicode/ucnv.h>
#include <unicode/ucnv_err.h>
#include <unicode/ustring.h>
#include <unicode/udata.h>
#if (U_ICU_VERSION_MAJOR_NUM >= 2)
    #include <unicode/uclean.h>
#endif

#if !defined(XML_OS390) && !defined(XML_AS400) && !defined(XML_HPUX) && !defined(XML_PTX)
// Forward reference the symbol which points to the ICU converter data.
#if (U_ICU_VERSION_MAJOR_NUM < 2)
extern "C" const uint8_t U_IMPORT icudata_dat[];
#endif
#endif

#if !defined(U16_NEXT_UNSAFE) && defined(UTF16_NEXT_CHAR_UNSAFE)
    #define U16_NEXT_UNSAFE UTF16_NEXT_CHAR_UNSAFE
#endif

#if !defined(U16_APPEND_UNSAFE) && defined(UTF16_APPEND_CHAR_UNSAFE)
    #define U16_APPEND_UNSAFE UTF16_APPEND_CHAR_UNSAFE
#endif

#if !defined(U_IS_BMP) && defined(UTF16_CHAR_LENGTH)
    #define U_IS_BMP(c) (UTF16_CHAR_LENGTH(c)==1)
#endif


XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local, const data
// ---------------------------------------------------------------------------
static const XMLCh gMyServiceId[] =
{
    chLatin_I, chLatin_C, chLatin_U, chNull
};

static const XMLCh gS390Id[] =
{
    chLatin_S, chDigit_3, chDigit_9, chDigit_0, chNull
};

static const XMLCh gs390Id[] =
{
    chLatin_s, chDigit_3, chDigit_9, chDigit_0, chNull
};

static const XMLCh gswaplfnlId[] =
{
    chComma, chLatin_s, chLatin_w, chLatin_a, chLatin_p,
    chLatin_l, chLatin_f, chLatin_n, chLatin_l, chNull
};
// ---------------------------------------------------------------------------
//  Local functions
// ---------------------------------------------------------------------------

//
//  When XMLCh and ICU's UChar are not the same size, we have to do a temp
//  conversion of all strings. These local helper methods make that easier.
//
static UChar* convertToUChar( const XMLCh* const   toConvert
                            , const XMLSize_t      srcLen = 0
                            , MemoryManager* const manager = 0)
{
    const XMLSize_t actualLen = srcLen
                                   ? srcLen : XMLString::stringLen(toConvert);

    UChar* tmpBuf = (manager)
        ? (UChar*) manager->allocate((actualLen + 1) * sizeof(UChar))
		: new UChar[actualLen + 1];
    const XMLCh* srcPtr = toConvert;
    UChar* outPtr = tmpBuf;
    while (*srcPtr)
        *outPtr++ = UChar(*srcPtr++);
    *outPtr = 0;

    return tmpBuf;
}


static XMLCh* convertToXMLCh( const UChar* const toConvert,
                            MemoryManager* const manager = 0)
{
    const unsigned int srcLen = u_strlen(toConvert);
    XMLCh* retBuf = (manager)
        ? (XMLCh*) manager->allocate((srcLen+1) * sizeof(XMLCh))
        : new XMLCh[srcLen + 1];

    XMLCh* outPtr = retBuf;
    const UChar* srcPtr = toConvert;
    while (*srcPtr)
        *outPtr++ = XMLCh(*srcPtr++);
    *outPtr = 0;

    return retBuf;
}




// ---------------------------------------------------------------------------
//  ICUTransService: Constructors and Destructor
// ---------------------------------------------------------------------------
ICUTransService::ICUTransService(MemoryManager*)
{
  // Starting with ICU 3.4 we don't need to call init anymore.
  //
#if (U_ICU_VERSION_MAJOR_NUM > 2 || (U_ICU_VERSION_MAJOR_NUM == 2 && U_ICU_VERSION_MINOR_NUM >= 6)) && \
  (U_ICU_VERSION_MAJOR_NUM < 3 || (U_ICU_VERSION_MAJOR_NUM == 3 && U_ICU_VERSION_MINOR_NUM < 4))
    UErrorCode errorCode=U_ZERO_ERROR;
    u_init(&errorCode);
    if(U_FAILURE(errorCode)) {
        XMLPlatformUtils::panic(PanicHandler::Panic_NoTransService);
    }
#endif

#if !defined(XML_OS390) && !defined(XML_AS400) && !defined(XML_HPUX) && !defined(XML_PTX)
#if (U_ICU_VERSION_MAJOR_NUM < 2)
    // Starting with ICU 2.0, ICU itself includes a static reference to the data
    // entrypoint symbol.
    //
    // ICU 1.8 (and previous) did not include a static reference, but would
    // dynamically load the data dll when it was first needed, however this dynamic
    // loading proved unreliable in some of the odd environments that Xerces needed
    // to run in.  Hence, the static reference.

    // Pass the location of the converter data to ICU. By doing so, we are
    // forcing the load of ICU converter data DLL, after the Xerces-C DLL is
    // loaded. This implies that Xerces-C, now has to explicitly link with the
    // ICU converter dll. However, the advantage is that we no longer depend
    // on the code which does demand dynamic loading of DLL's. The demand
    // loading is highly system dependent and was a constant source of support
    // calls.
    UErrorCode uerr = U_ZERO_ERROR;
    udata_setCommonData((void *) icudata_dat, &uerr);
#endif
#endif
}

ICUTransService::~ICUTransService()
{
    /*
     * commented out the following clean up code
     * in case users use ICU outside of the parser
     * if we clean up here, users' code may crash
     *
    #if (U_ICU_VERSION_MAJOR_NUM >= 2)
        // release all lazily allocated data
        u_cleanup();
    #endif
    */
}


// ---------------------------------------------------------------------------
//  ICUTransService: The virtual transcoding service API
// ---------------------------------------------------------------------------
int ICUTransService::compareIString(const   XMLCh* const    comp1
                                    , const XMLCh* const    comp2)
{
    size_t  i = 0;
    size_t  j = 0;

    for(;;)
    {
        UChar32 ch1;
        UChar32 ch2;

        U16_NEXT_UNSAFE(comp1, i, ch1);
        U16_NEXT_UNSAFE(comp2, j, ch2);

        const UChar32   folded1 =
            u_foldCase(ch1, U_FOLD_CASE_DEFAULT);

        const UChar32   folded2 =
            u_foldCase(ch2, U_FOLD_CASE_DEFAULT);

        if (folded1 !=
            folded2)
        {
            return folded1 - folded2;
        }
        else if (ch1 == 0)
        {
            // If ch1 is 0, the ch2 must also be
            // 0.  Otherwise, the previous if
            // would have failed.
            break;
        }
    }

    return 0;
}


int ICUTransService::compareNIString(const  XMLCh* const    comp1
                                    , const XMLCh* const    comp2
                                    , const XMLSize_t       maxChars)
{
    if (maxChars > 0)
    {
        // Note that this function has somewhat broken semantics, as it's
        // possible for two strings of different lengths to compare as equal
        // in a case-insensitive manner, since one character could be
        // represented as a surrogate pair.
        size_t  i = 0;
        size_t  j = 0;

        for(;;)
        {
            UChar32 ch1;
            UChar32 ch2;

            U16_NEXT_UNSAFE(comp1, i, ch1);
            U16_NEXT_UNSAFE(comp2, j, ch2);

            const UChar32   folded1 =
                u_foldCase(ch1, U_FOLD_CASE_DEFAULT);

            const UChar32   folded2 =
                u_foldCase(ch2, U_FOLD_CASE_DEFAULT);

            if (folded1 != folded2)
            {
                return folded1 - folded2;
            }
            else if (i == maxChars)
            {
                // If we're at the end of both strings, return 0.
                // Otherwise, we've run out of characters in the
                // left string, so return -1.
                return j == maxChars ? 0 : -1;
            }
            else if (j == maxChars)
            {
                // We've run out of characters in the right string,
                // but not the left, so return 1.
                return 1;
            }
        }
    }

    return 0;
}


const XMLCh* ICUTransService::getId() const
{
    return gMyServiceId;
}

XMLLCPTranscoder* ICUTransService::makeNewLCPTranscoder(MemoryManager* manager)
{
    //
    //  Try to create a default converter. If it fails, return a null
    //  pointer which will basically cause the system to give up because
    //  we really can't do anything without one.
    //
    UErrorCode uerr = U_ZERO_ERROR;
    UConverter* converter = ucnv_open(NULL, &uerr);
    if (!converter)
        return 0;

    // That went ok, so create an ICU LCP transcoder wrapper and return it
    return new (manager) ICULCPTranscoder(converter);
}


bool ICUTransService::supportsSrcOfs() const
{
    // This implementation supports source offset information
    return true;
}


template <class FunctionType>
static void doCaseConvert(XMLCh*          convertString,
                          FunctionType    caseFunction)
{
    // Note the semantics of this function are broken, since it's
    // possible that changing the case of a string could increase
    // its length, but there's no way to handle such a situation.
    const XMLSize_t len = XMLString::stringLen(convertString);

    size_t  readPos = 0;
    size_t  writePos = 0;

    while(readPos < len)
    {
        UChar32     original;

        // Get the next Unicode code point.
        U16_NEXT_UNSAFE(convertString, readPos, original);

        // Convert the code point
        const UChar32   converted = caseFunction(original);

        // OK, now here's where it gets ugly.
        if (!U_IS_BMP(converted) && U_IS_BMP(original) &&
            readPos - writePos == 1)
        {
            // We do not have room to convert the
            // character without overwriting the next
            // character, so we will just stop.
            break;
        }
        else
        {
            U16_APPEND_UNSAFE(convertString, writePos, converted);
        }
    }

    convertString[writePos] = 0;
}



void ICUTransService::upperCase(XMLCh* const toUpperCase)
{
    doCaseConvert(toUpperCase, u_toupper);
}

void ICUTransService::lowerCase(XMLCh* const toLowerCase)
{
    doCaseConvert(toLowerCase, u_tolower);
}



// ---------------------------------------------------------------------------
//  ICUTransService: The protected virtual transcoding service API
// ---------------------------------------------------------------------------
XMLTranscoder* ICUTransService::
makeNewXMLTranscoder(const  XMLCh* const            encodingName
                    ,       XMLTransService::Codes& resValue
                    , const XMLSize_t               blockSize
                    ,       MemoryManager* const    manager)
{
    //
    //  For encodings that end with "s390" we need to strip off the "s390"
    //  from the encoding name and add ",swaplfnl" to the encoding name
    //  that we pass into ICU on the ucnv_openU.
    //
    XMLCh* encodingNameToUse = (XMLCh*) encodingName;
    XMLCh* workBuffer = 0;

    if ( (XMLString::endsWith(encodingNameToUse, gs390Id)) ||
         (XMLString::endsWith(encodingNameToUse, gS390Id)) )
    {
       XMLSize_t workBufferSize = (XMLString::stringLen(encodingNameToUse) + XMLString::stringLen(gswaplfnlId) - XMLString::stringLen(gS390Id) + 1);
       workBuffer = (XMLCh*) manager->allocate(workBufferSize * sizeof(XMLCh));
       XMLSize_t moveSize = XMLString::stringLen(encodingNameToUse) - XMLString::stringLen(gS390Id);
       XMLString::moveChars(workBuffer, encodingNameToUse, moveSize);
       XMLString::moveChars((workBuffer + moveSize), gswaplfnlId, XMLString::stringLen(gswaplfnlId));
       encodingNameToUse = workBuffer;
    }

    //
    //  If UChar and XMLCh are not the same size, then we have premassage the
    //  encoding name into a UChar type string.
    //
    const UChar* actualName;
    UChar* tmpName = 0;
    if (sizeof(UChar) == sizeof(XMLCh))
    {
        actualName = (const UChar*)encodingNameToUse;
    }
    else
    {
        tmpName = convertToUChar(encodingNameToUse, 0, manager);
        actualName = tmpName;
    }

    ArrayJanitor<UChar> janTmp(tmpName, manager);
    ArrayJanitor<XMLCh> janTmp1(workBuffer, manager);

    UErrorCode uerr = U_ZERO_ERROR;
    UConverter* converter = ucnv_openU(actualName, &uerr);
    if (!converter)
    {
        resValue = XMLTransService::UnsupportedEncoding;
        return 0;
    }

    return new (manager) ICUTranscoder(encodingName, converter, blockSize, manager);
}




// ---------------------------------------------------------------------------
//  ICUTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
ICUTranscoder::ICUTranscoder(const  XMLCh* const        encodingName
                            ,       UConverter* const   toAdopt
                            , const XMLSize_t           blockSize
                            , MemoryManager* const      manager) :

    XMLTranscoder(encodingName, blockSize, manager)
    , fConverter(toAdopt)
    , fFixed(false)
    , fSrcOffsets(0)
{
    // If there is a block size, then allocate our source offset array
    if (blockSize)
        fSrcOffsets = (XMLUInt32*) manager->allocate
        (
            blockSize * sizeof(XMLUInt32)
        );//new XMLUInt32[blockSize];

    // Remember if its a fixed size encoding
    fFixed = (ucnv_getMaxCharSize(fConverter) == ucnv_getMinCharSize(fConverter));
}

ICUTranscoder::~ICUTranscoder()
{
    getMemoryManager()->deallocate(fSrcOffsets);//delete [] fSrcOffsets;

    // If there is a converter, ask ICU to clean it up
    if (fConverter)
    {
        // <TBD> Does this actually delete the structure???
        ucnv_close(fConverter);
        fConverter = 0;
    }
}


// ---------------------------------------------------------------------------
//  ICUTranscoder: The virtual transcoder API
// ---------------------------------------------------------------------------
XMLSize_t
ICUTranscoder::transcodeFrom(const  XMLByte* const          srcData
                            , const XMLSize_t               srcCount
                            ,       XMLCh* const            toFill
                            , const XMLSize_t               maxChars
                            ,       XMLSize_t&              bytesEaten
                            ,       unsigned char* const    charSizes)
{
    // Set up pointers to the start and end of the source buffer
    const XMLByte*  startSrc = srcData;
    const XMLByte*  endSrc = srcData + srcCount;

    //
    //  And now do the target buffer. This works differently according to
    //  whether XMLCh and UChar are the same size or not.
    //
    UChar* startTarget;
    if (sizeof(XMLCh) == sizeof(UChar))
        startTarget = (UChar*)toFill;
     else
        startTarget = (UChar*) getMemoryManager()->allocate
        (
            maxChars * sizeof(UChar)
        );//new UChar[maxChars];
    UChar* orgTarget = startTarget;

    //
    //  Transcode the buffer.  Buffer overflow errors are normal, occuring
    //  when the raw input buffer holds more characters than will fit in
    //  the Unicode output buffer.
    //
    UErrorCode  err = U_ZERO_ERROR;
    ucnv_toUnicode
    (
        fConverter
        , &startTarget
        , startTarget + maxChars
        , (const char**)&startSrc
        , (const char*)endSrc
        , (fFixed ? 0 : (int32_t*)fSrcOffsets)
        , false
        , &err
    );

    if ((err != U_ZERO_ERROR) && (err != U_BUFFER_OVERFLOW_ERROR))
    {
        if (orgTarget != (UChar*)toFill)
            getMemoryManager()->deallocate(orgTarget);//delete [] orgTarget;

        if (fFixed)
        {
            XMLCh tmpBuf[17];
            XMLString::binToText((unsigned int)(*startTarget), tmpBuf, 16, 16, getMemoryManager());
            ThrowXMLwithMemMgr2
            (
                TranscodingException
                , XMLExcepts::Trans_BadSrcCP
                , tmpBuf
                , getEncodingName()
                , getMemoryManager()
            );
        }
        else
        {
            ThrowXMLwithMemMgr(TranscodingException, XMLExcepts::Trans_BadSrcSeq, getMemoryManager());
        }
    }

    // Calculate the bytes eaten and store in caller's param
    bytesEaten = startSrc - srcData;

    // And the characters decoded
    const XMLSize_t charsDecoded = startTarget - orgTarget;

    //
    //  Translate the array of char offsets into an array of character
    //  sizes, which is what the transcoder interface semantics requires.
    //  If its fixed, then we can optimize it.
    //
    if (fFixed)
    {
        const unsigned char fillSize = (unsigned char)ucnv_getMaxCharSize(fConverter);
        memset(charSizes, fillSize, maxChars);
    }
     else
    {
        //
        //  We have to convert the series of offsets into a series of
        //  sizes. If just one char was decoded, then its the total bytes
        //  eaten. Otherwise, do a loop and subtract out each element from
        //  its previous element.
        //
        if (charsDecoded == 1)
        {
            charSizes[0] = (unsigned char)bytesEaten;
        }
         else
        {
            //  ICU does not return an extra element to allow us to figure
            //  out the last char size, so we have to compute it from the
            //  total bytes used.
            unsigned int index;
            for (index = 0; index < charsDecoded - 1; index++)
            {
                charSizes[index] = (unsigned char)(fSrcOffsets[index + 1]
                                                    - fSrcOffsets[index]);
            }
            if( charsDecoded > 0 ) {
                charSizes[charsDecoded - 1] = (unsigned char)(bytesEaten
                                              - fSrcOffsets[charsDecoded - 1]);
            }
        }
    }

    //
    //  If XMLCh and UChar are not the same size, then we need to copy over
    //  the temp buffer to the new one.
    //
    if (sizeof(UChar) != sizeof(XMLCh))
    {
        XMLCh* outPtr = toFill;
        startTarget = orgTarget;
        for (unsigned int index = 0; index < charsDecoded; index++)
            *outPtr++ = XMLCh(*startTarget++);

        // And delete the temp buffer
        getMemoryManager()->deallocate(orgTarget);//delete [] orgTarget;
    }

    // Return the chars we put into the target buffer
    return charsDecoded;
}


XMLSize_t
ICUTranscoder::transcodeTo( const   XMLCh* const    srcData
                            , const XMLSize_t       srcCount
                            ,       XMLByte* const  toFill
                            , const XMLSize_t       maxBytes
                            ,       XMLSize_t&      charsEaten
                            , const UnRepOpts       options)
{
    //
    //  Get a pointer to the buffer to transcode. If UChar and XMLCh are
    //  the same size here, then use the original. Else, create a temp
    //  one and put a janitor on it.
    //
    const UChar* srcPtr;
    UChar* tmpBufPtr = 0;
    if (sizeof(XMLCh) == sizeof(UChar))
    {
        srcPtr = (const UChar*)srcData;
    }
    else
    {
        tmpBufPtr = convertToUChar(srcData, srcCount, getMemoryManager());
        srcPtr = tmpBufPtr;
    }
    ArrayJanitor<UChar> janTmpBuf(tmpBufPtr, getMemoryManager());

    //
    //  Set the appropriate callback so that it will either fail or use
    //  the rep char. Remember the old one so we can put it back.
    //
    UErrorCode  err = U_ZERO_ERROR;
    UConverterFromUCallback oldCB = NULL;
    #if (U_ICU_VERSION_MAJOR_NUM < 2)
    void* orgContent;
    #else
    const void* orgContent;
    #endif
    ucnv_setFromUCallBack
    (
        fConverter
        , (options == UnRep_Throw) ? UCNV_FROM_U_CALLBACK_STOP
                                   : UCNV_FROM_U_CALLBACK_SUBSTITUTE
        , NULL
        , &oldCB
        , &orgContent
        , &err
    );

    //
    //  Ok, lets transcode as many chars as we we can in one shot. The
    //  ICU API gives enough info not to have to do this one char by char.
    //
    XMLByte*        startTarget = toFill;
    const UChar*    startSrc = srcPtr;
    err = U_ZERO_ERROR;
    ucnv_fromUnicode
    (
        fConverter
        , (char**)&startTarget
        , (char*)(startTarget + maxBytes)
        , &startSrc
        , srcPtr + srcCount
        , 0
        , false
        , &err
    );

    // Rememember the status before we possibly overite the error code
    const bool res = (err == U_ZERO_ERROR);

    // Put the old handler back
    err = U_ZERO_ERROR;
    UConverterFromUCallback orgAction = NULL;

    ucnv_setFromUCallBack(fConverter, oldCB, NULL, &orgAction, &orgContent, &err);

    if (!res)
    {
        XMLCh tmpBuf[17];
        XMLString::binToText((unsigned int)*startSrc, tmpBuf, 16, 16, getMemoryManager());
        ThrowXMLwithMemMgr2
        (
            TranscodingException
            , XMLExcepts::Trans_Unrepresentable
            , tmpBuf
            , getEncodingName()
            , getMemoryManager()
        );
    }

    // Fill in the chars we ate from the input
    charsEaten = startSrc - srcPtr;

    // Return the chars we stored
    return startTarget - toFill;
}


bool ICUTranscoder::canTranscodeTo(const unsigned int toCheck)
{
    //
    //  If the passed value is really a surrogate embedded together, then
    //  we need to break it out into its two chars. Else just one. While
    //  we are ate it, convert them to UChar format if required.
    //
    UChar           srcBuf[2];
    unsigned int    srcCount = 1;
    if (toCheck & 0xFFFF0000)
    {
        srcBuf[0] = UChar((toCheck >> 10) + 0xD800);
        srcBuf[1] = UChar(toCheck & 0x3FF) + 0xDC00;
        srcCount++;
    }
     else
    {
        srcBuf[0] = UChar(toCheck);
    }

    //
    //  Set the callback so that it will fail instead of using the rep char.
    //  Remember the old one so we can put it back.
    //
     UErrorCode  err = U_ZERO_ERROR;
     UConverterFromUCallback oldCB = NULL;
     #if (U_ICU_VERSION_MAJOR_NUM < 2)
     void* orgContent;
     #else
     const void* orgContent;
     #endif

     ucnv_setFromUCallBack
         (
         fConverter
         , UCNV_FROM_U_CALLBACK_STOP
         , NULL
         , &oldCB
         , &orgContent
         , &err
         );

    // Set upa temp buffer to format into. Make it more than big enough
    char            tmpBuf[64];
    char*           startTarget = tmpBuf;
    const UChar*    startSrc = srcBuf;

    err = U_ZERO_ERROR;
    ucnv_fromUnicode
    (
        fConverter
        , &startTarget
        , startTarget + 64
        , &startSrc
        , srcBuf + srcCount
        , 0
        , false
        , &err
    );

    // Save the result before we overight the error code
    const bool res = (err == U_ZERO_ERROR);

    // Put the old handler back
    err = U_ZERO_ERROR;
    UConverterFromUCallback orgAction = NULL;

    ucnv_setFromUCallBack(fConverter, oldCB, NULL, &orgAction, &orgContent, &err);

    return res;
}



// ---------------------------------------------------------------------------
//  ICULCPTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
ICULCPTranscoder::ICULCPTranscoder(UConverter* const toAdopt) :

    fConverter(toAdopt)
{
}

ICULCPTranscoder::~ICULCPTranscoder()
{
    // If there is a converter, ask ICU to clean it up
    if (fConverter)
    {
        // <TBD> Does this actually delete the structure???
        ucnv_close(fConverter);
        fConverter = 0;
    }
}


// ---------------------------------------------------------------------------
//  ICULCPTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLSize_t ICULCPTranscoder::calcRequiredSize(const XMLCh* const srcText
                                                , MemoryManager* const manager)
{
    if (!srcText)
        return 0;

    //
    //  We do two different versions of this, according to whether XMLCh
    //  is the same size as UChar or not.
    //
    UErrorCode err = U_ZERO_ERROR;
    int32_t targetCap;
    if (sizeof(XMLCh) == sizeof(UChar))
    {
        // Use a faux scope to synchronize while we do this
        {
            XMLMutexLock lockConverter(&fMutex);

            targetCap = ucnv_fromUChars
            (
                fConverter
                , 0
                , 0
                , (const UChar*)srcText
                , -1
                , &err
            );
        }
    }
    else
    {
        // Copy the source to a local temp
        UChar* tmpBuf = convertToUChar(srcText, 0, manager);
        ArrayJanitor<UChar> janTmp(tmpBuf, manager);

        // Use a faux scope to synchronize while we do this
        {
            XMLMutexLock lockConverter(&fMutex);

            targetCap = ucnv_fromUChars
            (
                fConverter
                , 0
                , 0
                , tmpBuf
                , -1
                , &err
            );
        }
    }

    if (err != U_BUFFER_OVERFLOW_ERROR)
        return 0;

    return (XMLSize_t)targetCap;
}

XMLSize_t ICULCPTranscoder::calcRequiredSize(const char* const srcText
                                                , MemoryManager* const /*manager*/)
{
    if (!srcText)
        return 0;

    int32_t targetCap;
    UErrorCode err = U_ZERO_ERROR;

    // Use a faux scope to synchronize while we do this
    {
        XMLMutexLock lockConverter(&fMutex);
        targetCap = ucnv_toUChars
        (
            fConverter
            , 0
            , 0
            , srcText
            , (int32_t)strlen(srcText)
            , &err
        );
    }

    if (err != U_BUFFER_OVERFLOW_ERROR)
        return 0;

#if (U_ICU_VERSION_MAJOR_NUM < 2)
    // Subtract one since it includes the terminator space
    return (XMLSize_t)(targetCap - 1);
#else
    // Starting ICU 2.0, this is fixed and all ICU String functions have consistent NUL-termination behavior.
    // The returned length is always the number of output UChar's, not counting an additional, terminating NUL.
    return (XMLSize_t)(targetCap);
#endif
}


char* ICULCPTranscoder::transcode(const XMLCh* const toTranscode,
                                  MemoryManager* const manager)
{
    char* retBuf = 0;

    // Check for a couple of special cases
    if (!toTranscode)
        return retBuf;

    if (!*toTranscode)
    {
        retBuf = (char*) manager->allocate(sizeof(char));//new char[1];
        retBuf[0] = 0;
        return retBuf;
    }

    //
    //  Get the length of the source string since we'll have to use it in
    //  a couple places below.
    //
    const XMLSize_t srcLen = XMLString::stringLen(toTranscode);

    //
    //  If XMLCh and UChar are not the same size, then we have to make a
    //  temp copy of the text to pass to ICU.
    //
    const UChar* actualSrc;
    UChar* ncActual = 0;
    if (sizeof(XMLCh) == sizeof(UChar))
    {
        actualSrc = (const UChar*)toTranscode;
    }
     else
    {
        // Allocate a non-const temp buf, but store it also in the actual
        ncActual = convertToUChar(toTranscode, 0, manager);
        actualSrc = ncActual;
    }

    // Insure that the temp buffer, if any, gets cleaned up via the nc pointer
    ArrayJanitor<UChar> janTmp(ncActual, manager);

    // Caculate a return buffer size not too big, but less likely to overflow
    int32_t targetLen = (int32_t)(srcLen * 1.25);

    // Allocate the return buffer
    retBuf = (char*) manager->allocate((targetLen + 1) * sizeof(char));//new char[targetLen + 1];

    //
    //  Lock now while we call the converter. Use a faux block to do the
    //  lock so that it unlocks immediately afterwards.
    //
    UErrorCode err = U_ZERO_ERROR;
    int32_t targetCap;
    {
        XMLMutexLock lockConverter(&fMutex);

        targetCap = ucnv_fromUChars
        (
            fConverter
            , retBuf
            , targetLen + 1
            , actualSrc
            , -1
            , &err
        );
    }

    // If targetLen is not enough then buffer overflow might occur
    if ((err == U_BUFFER_OVERFLOW_ERROR) || (err == U_STRING_NOT_TERMINATED_WARNING))
    {
        //
        //  Reset the error, delete the old buffer, allocate a new one,
        //  and try again.
        //
        err = U_ZERO_ERROR;
        manager->deallocate(retBuf);//delete [] retBuf;
        retBuf = (char*) manager->allocate((targetCap + 1) * sizeof(char));//new char[targetCap + 1];

        // Lock again before we retry
        XMLMutexLock lockConverter(&fMutex);
        targetCap = ucnv_fromUChars
        (
            fConverter
            , retBuf
            , targetCap + 1
            , actualSrc
            , -1
            , &err
        );
    }

    if (U_FAILURE(err))
    {
        manager->deallocate(retBuf);//delete [] retBuf;
        return 0;
    }

    return retBuf;
}

XMLCh* ICULCPTranscoder::transcode(const char* const toTranscode,
                                   MemoryManager* const manager)
{
    // Watch for a few pyscho corner cases
    if (!toTranscode)
        return 0;

    if (!*toTranscode)
    {
        XMLCh* retVal = (XMLCh*) manager->allocate(sizeof(XMLCh));//new XMLCh[1];
        retVal[0] = 0;
        return retVal;
    }

    //
    //  Get the length of the string to transcode. The Unicode string will
    //  almost always be no more chars than were in the source, so this is
    //  the best guess as to the storage needed.
    //
    const int32_t srcLen = (int32_t)strlen(toTranscode);

    // We need a target buffer of UChars to fill in
    UChar* targetBuf = 0;

    // Now lock while we do these calculations
    UErrorCode err = U_ZERO_ERROR;
    int32_t targetCap;
    {
        XMLMutexLock lockConverter(&fMutex);

        //
        //  Here we don't know what the target length will be so use 0 and
        //  expect an U_BUFFER_OVERFLOW_ERROR in which case it'd get resolved
        //  by the correct capacity value.
        //
        targetCap = ucnv_toUChars
        (
            fConverter
            , 0
            , 0
            , toTranscode
            , srcLen
            , &err
        );

        if (err != U_BUFFER_OVERFLOW_ERROR)
            return 0;

        err = U_ZERO_ERROR;
        targetBuf = (UChar*) manager->allocate((targetCap+1) * sizeof(UChar));//new UChar[targetCap + 1];
        ucnv_toUChars
        (
            fConverter
            , targetBuf
            , targetCap + 1
            , toTranscode
            , srcLen
            , &err
        );
    }

    if (U_FAILURE(err))
    {
        // Clean up if we got anything allocated
        manager->deallocate(targetBuf);//delete [] targetBuf;
        return 0;
    }

    // Cap it off to make sure
    targetBuf[targetCap] = 0;

    //
    //  If XMLCh and UChar are the same size, then we can return retVal
    //  as is. Else, we have to allocate another buffer and copy the data
    //  over to it.
    //
    XMLCh* actualRet;
    if (sizeof(XMLCh) == sizeof(UChar))
    {
        actualRet = (XMLCh*)targetBuf;
    }
     else
    {
        actualRet = convertToXMLCh(targetBuf, manager);
        manager->deallocate(targetBuf);//delete [] targetBuf;
    }
    return actualRet;
}


bool ICULCPTranscoder::transcode(const  char* const     toTranscode
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

    // We'll need this in a couple of places below
    const XMLSize_t srcLen = strlen(toTranscode);

    //
    //  Set up the target buffer. If XMLCh and UChar are not the same size
    //  then we have to use a temp buffer and convert over.
    //
    UChar* targetBuf;
    if (sizeof(XMLCh) == sizeof(UChar))
        targetBuf = (UChar*)toFill;
    else
        targetBuf = (UChar*) manager->allocate
        (
            (maxChars + 1) * sizeof(UChar)
        );//new UChar[maxChars + 1];

    //
    //  Use a faux block to enforce a lock on the converter, which will
    //  unlock immediately after its completed.
    //
    UErrorCode err = U_ZERO_ERROR;
    {
        XMLMutexLock lockConverter(&fMutex);
        ucnv_toUChars
        (
            fConverter
            , targetBuf
            , (int32_t)maxChars + 1
            , toTranscode
            , (int32_t)srcLen
            , &err
        );
    }

    if (U_FAILURE(err))
    {
        if (targetBuf != (UChar*)toFill)
            manager->deallocate(targetBuf);//delete [] targetBuf;
        return false;
    }

    // If the sizes are not the same, then copy the data over
    if (sizeof(XMLCh) != sizeof(UChar))
    {
        UChar* srcPtr = targetBuf;
        XMLCh* outPtr = toFill;
        while (*srcPtr)
            *outPtr++ = XMLCh(*srcPtr++);
        *outPtr = 0;

        // And delete the temp buffer
        manager->deallocate(targetBuf);//delete [] targetBuf;
    }

    return true;
}


bool ICULCPTranscoder::transcode(   const   XMLCh* const    toTranscode
                                    ,       char* const     toFill
                                    , const XMLSize_t       maxChars
                                    , MemoryManager* const  manager)
{
    // Watch for a few psycho corner cases
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

    //
    //  If XMLCh and UChar are not the same size, then we have to make a
    //  temp copy of the text to pass to ICU.
    //
    const UChar* actualSrc;
    UChar* ncActual = 0;
    if (sizeof(XMLCh) == sizeof(UChar))
    {
        actualSrc = (const UChar*)toTranscode;
    }
     else
    {
        // Allocate a non-const temp buf, but store it also in the actual
        ncActual = convertToUChar(toTranscode, 0, manager);
        actualSrc = ncActual;
    }

    // Insure that the temp buffer, if any, gets cleaned up via the nc pointer
    ArrayJanitor<UChar> janTmp(ncActual, manager);

    //
    //  Use a faux block to enforce a lock on the converter while we do this.
    //  It will be released immediately after its done.
    //
    UErrorCode err = U_ZERO_ERROR;
    int32_t targetCap;
    {
        XMLMutexLock lockConverter(&fMutex);
        targetCap = ucnv_fromUChars
        (
            fConverter
            , toFill
            , (int32_t)maxChars
            , actualSrc
            , -1
            , &err
        );
    }

    if (U_FAILURE(err))
        return false;

    toFill[targetCap] = 0;
    return true;
}

XERCES_CPP_NAMESPACE_END
