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
 *	$Id: MacOSUnicodeConverter.cpp 695759 2008-09-16 08:04:55Z borisk $
 */
 
 
// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XercesDefs.hpp>

#include <algorithm>
#include <cstddef>
#include <cstring>

#if defined(__APPLE__)
    //	Framework includes
    #include <CoreServices/CoreServices.h>
#else
    //	Classic includes otherwise
    #include <MacErrors.h>
    #include <Script.h>
    #include <TextUtils.h>
    #include <TextEncodingConverter.h>
    #include <TextCommon.h>
    #include <CodeFragments.h>
    #include <UnicodeConverter.h>
    #include <UnicodeUtilities.h>
    #include <CFCharacterSet.h>
    #include <CFString.h>
#endif

#include <xercesc/util/Transcoders/MacOSUnicodeConverter/MacOSUnicodeConverter.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/Janitor.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//	Typedefs
// ---------------------------------------------------------------------------

//	TempBufs are used for cases where we need a temporary buffer while processing.
const std::size_t kTempBufCount = 512;
typedef char	TempCharBuf[kTempBufCount];
typedef UniChar	TempUniBuf[kTempBufCount];
typedef XMLCh	TempXMLBuf[kTempBufCount];


// ---------------------------------------------------------------------------
//  Local, const data
// ---------------------------------------------------------------------------
const XMLCh MacOSUnicodeConverter::fgMyServiceId[] =
{
    chLatin_M, chLatin_a, chLatin_c, chLatin_O, chLatin_S, chNull
};


const XMLCh MacOSUnicodeConverter::fgMacLCPEncodingName[] =
{
        chLatin_M, chLatin_a, chLatin_c, chLatin_O, chLatin_S, chLatin_L
    ,   chLatin_C, chLatin_P, chLatin_E, chLatin_n, chLatin_c, chLatin_o
    ,   chLatin_d, chLatin_i, chLatin_n, chLatin_g, chNull
};



// ---------------------------------------------------------------------------
//  MacOSUnicodeConverter: Constructors and Destructor
// ---------------------------------------------------------------------------
MacOSUnicodeConverter::MacOSUnicodeConverter(MemoryManager* manager)
  : fCollator(NULL)
{
	//	Test for presense of unicode collation functions
	fHasUnicodeCollation = (UCCompareText != NULL);
    
    //  Create a unicode collator for doing string comparisons
    if (fHasUnicodeCollation)
    {
		//  Configure collation options
        UCCollateOptions collateOptions =
								kUCCollateComposeInsensitiveMask
								| kUCCollateWidthInsensitiveMask
								| kUCCollateCaseInsensitiveMask
								| kUCCollatePunctuationSignificantMask
								;
						
        OSStatus status = UCCreateCollator(NULL, 0, collateOptions, &fCollator);
    }
}


MacOSUnicodeConverter::~MacOSUnicodeConverter()
{
    //  Dispose our collator
    if (fCollator != NULL)
        UCDisposeCollator(&fCollator);
}


// ---------------------------------------------------------------------------
//  MacOSUnicodeConverter: The virtual transcoding service API
// ---------------------------------------------------------------------------
int MacOSUnicodeConverter::compareIString(  const XMLCh* const    comp1
                                          , const XMLCh* const    comp2)
{
	//	If unicode collation routines are available, use them.
	//	This should be the case on Mac OS 8.6 and later,
	//	with Carbon 1.0.2 or later, and under Mac OS X.
	//
	//	Otherwise, but only for Metrowerks, since only Metrowerks
	//	has a c library with a valid set of wchar routines,
	//	fall back to the standard library.

	if (fHasUnicodeCollation && fCollator != NULL)
	{
		std::size_t cnt1 = XMLString::stringLen(comp1);
		std::size_t cnt2 = XMLString::stringLen(comp2);
		
        Boolean equivalent = false;
        SInt32 order = 0;
        OSStatus status = UCCompareText(
                                fCollator,
                                reinterpret_cast<const UniChar*>(comp1),
                                cnt1,
                                reinterpret_cast<const UniChar*>(comp2),
                                cnt2,
                                &equivalent,
                                &order
                                );
									
        return ((status != noErr) || equivalent) ? 0 : order;
	}
	else
	{
		//	For some reason there is no platform utils available
		//	where we expect it. Bail.
		XMLPlatformUtils::panic(PanicHandler::Panic_NoTransService);
		return 0;
	}
}


int MacOSUnicodeConverter::compareNIString( const XMLCh* const  comp1
                                        , const XMLCh* const    comp2
                                        , const XMLSize_t       maxChars)
{
	//	If unicode collation routines are available, use them.
	//	This should be the case on Mac OS 8.6 and later,
	//	with Carbon 1.0.2 or later, and under Mac OS X.
	//
	//	Otherwise, but only for Metrowerks, since only Metrowerks
	//	has a c library with a valid set of wchar routines,
	//	fall back to the standard library.

	if (fHasUnicodeCollation && fCollator != NULL)
	{
		std::size_t cnt1 = XMLString::stringLen(comp1);
		std::size_t cnt2 = XMLString::stringLen(comp2);
		
		//	Restrict view of source characters to first {maxChars}
		if (cnt1 > maxChars)
			cnt1 = maxChars;
			
		if (cnt2 > maxChars)
			cnt2 = maxChars;
		
        Boolean equivalent = false;
        SInt32 order = 0;
        OSStatus status = UCCompareText(
                                fCollator,	
                                reinterpret_cast<const UniChar*>(comp1),
                                cnt1,
                                reinterpret_cast<const UniChar*>(comp2),
                                cnt2,
                                &equivalent,
                                &order
                                );
                                
        return ((status != noErr) || equivalent) ? 0 : order;
	}
	else
	{
		//	For some reason there is no platform utils available
		//	where we expect it. Bail.
		XMLPlatformUtils::panic(PanicHandler::Panic_NoTransService);
		return 0;
	}
}


const XMLCh* MacOSUnicodeConverter::getId() const
{
    return fgMyServiceId;
}

TextEncoding
MacOSUnicodeConverter::discoverLCPEncoding()
{
	TextEncoding encoding = 0;
	
    //  Ask the OS for the best text encoding for this application
    //  We would call GetApplicationTextEncoding(), but it's available only in
    //  Carbon (not CarbonCore), and we try to link with frameworks only in CoreServices.
    //      encoding = GetApplicationTextEncoding();
    
	//	Get TextEncoding for the current Mac System Script, falling back to Mac Roman
	if (noErr != UpgradeScriptInfoToTextEncoding(
					smSystemScript, kTextLanguageDontCare, kTextRegionDontCare,
					NULL, &encoding))
		encoding = CreateTextEncoding(kTextEncodingMacRoman,
									kTextEncodingDefaultVariant,
									kTextEncodingDefaultFormat);

	//  Traditionally, the Mac transcoder has used the current system script
	//  as the LCP text encoding.
	//
	//  As of Xerces 2.6, this continues to be the case if XML_MACOS_LCP_TRADITIONAL
	//  is defined.
	//
	//  Otherwise, but only for Mac OS X,  utf-8 will be used instead.
	//  Since posix paths are utf-8 encoding on OS X, and the OS X
	//  terminal uses utf-8 by default, this seems to make the most sense.
	#if !defined(XML_MACOS_LCP_TRADITIONAL)
	if (true /*gMacOSXOrBetter*/)
	{
		//  Manufacture a text encoding for UTF8
		encoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
									kTextEncodingDefaultVariant,
									kUnicodeUTF8Format);
	}
	#endif
	
	return encoding;
}


XMLLCPTranscoder* MacOSUnicodeConverter::makeNewLCPTranscoder(MemoryManager* manager)
{
	XMLLCPTranscoder* result = NULL;
	OSStatus status = noErr;
	
	//  Discover the text encoding to use for the LCP
	TextEncoding lcpTextEncoding = discoverLCPEncoding();

    //  We implement the LCP transcoder in terms of the XMLTranscoder.
	//  Create an XMLTranscoder for this encoding
	XMLTransService::Codes resValue;
    XMLTranscoder* xmlTrans = makeNewXMLTranscoder(fgMacLCPEncodingName,
                                resValue, kTempBufCount,
								lcpTextEncoding, manager);
    
    if (xmlTrans)
    {
        //  Pass the XMLTranscoder over to the LPC transcoder
        if (resValue == XMLTransService::Ok)
            result = new (manager) MacOSLCPTranscoder(xmlTrans, manager);
        else
            delete xmlTrans;
    }
	
    return result;
}


bool MacOSUnicodeConverter::supportsSrcOfs() const
{
	// For now, we don't support source offsets
    return false;
}


void MacOSUnicodeConverter::upperCase(XMLCh* const toUpperCase)
{
#if TARGET_API_MAC_CARBON

   // If we're targeting carbon, use the CFString conversion to uppercase
   int len = XMLString::stringLen(toUpperCase);
   CFMutableStringRef cfString = CFStringCreateMutableWithExternalCharactersNoCopy(
        kCFAllocatorDefault,
        (UniChar*)toUpperCase,
        len,		// length
        len,		// capacity
        kCFAllocatorNull);
   CFStringUppercase(cfString, NULL);
   CFRelease(cfString);

#elif (__GNUC__ >= 3 && _GLIBCPP_USE_WCHAR_T)

	// Use this if there's a reasonable c library available.
	// Metrowerks does this reasonably
	wchar_t c;
	for (XMLCh* p = (XMLCh*)toUpperCase; ((c = *p) != 0); )
		*p++ = std::towupper(c);

#else
	#error Sorry, no support for upperCase
#endif
}


void MacOSUnicodeConverter::lowerCase(XMLCh* const toLowerCase)
{
#if TARGET_API_MAC_CARBON

   // If we're targeting carbon, use the CFString conversion to uppercase
   int len = XMLString::stringLen(toLowerCase);
   CFMutableStringRef cfString = CFStringCreateMutableWithExternalCharactersNoCopy(
        kCFAllocatorDefault,
        (UniChar*)toLowerCase,
        len,		// length
        len,		// capacity
        kCFAllocatorNull);
   CFStringLowercase(cfString, NULL);
   CFRelease(cfString);

#elif (__GNUC__ >= 3 && _GLIBCPP_USE_WCHAR_T)

	// Use this if there's a reasonable c library available.
	// Metrowerks does this reasonably
	wchar_t c;
	for (XMLCh* p = (XMLCh*)toLowerCase; ((c = *p) != 0); )
		*p++ = std::towlower(c);

#else
	#error Sorry, no support for lowerCase
#endif
}


void
MacOSUnicodeConverter::ConvertWideToNarrow(const XMLCh* wide, char* narrow, std::size_t maxChars)
{
	while (maxChars-- > 0)
		if ((*narrow++ = *wide++) == 0)
			break;
}


void
MacOSUnicodeConverter::CopyCStringToPascal(const char* c, Str255 pas)
{
	int len = strlen(c);
	if (len > sizeof(pas)-1)
		len = sizeof(pas)-1;
	memmove(&pas[1], c, len);
	pas[0] = len;
}


// ---------------------------------------------------------------------------
//  MacOSTransService: The protected virtual transcoding service API
// ---------------------------------------------------------------------------
XMLTranscoder*
MacOSUnicodeConverter::makeNewXMLTranscoder(const   XMLCh* const		encodingName
                                        ,       XMLTransService::Codes& resValue
                                        , const XMLSize_t               blockSize
                                        ,       MemoryManager* const    manager)
{
	XMLTranscoder* result = NULL;
	resValue = XMLTransService::Ok;
	
	TextToUnicodeInfo textToUnicodeInfo = NULL;
	UnicodeToTextInfo unicodeToTextInfo = NULL;

	//	Map the encoding to a Mac OS Encoding value
	Str255 pasEncodingName;
	char cEncodingName[256];
	ConvertWideToNarrow(encodingName, cEncodingName, sizeof(cEncodingName));
	CopyCStringToPascal(cEncodingName, pasEncodingName);
	
	TextEncoding textEncoding = 0;
	OSStatus status = TECGetTextEncodingFromInternetName (
							&textEncoding,
							pasEncodingName);
                            
    //  Make a transcoder for that encoding
	if (status == noErr)
		result = makeNewXMLTranscoder(encodingName, resValue, blockSize, textEncoding, manager);
	else
		resValue = XMLTransService::UnsupportedEncoding;
	
	return result;
}


XMLTranscoder*
MacOSUnicodeConverter::makeNewXMLTranscoder(const   XMLCh* const		encodingName
                                        ,       XMLTransService::Codes& resValue
                                        , const XMLSize_t               blockSize
										,		TextEncoding            textEncoding
                                        ,       MemoryManager* const    manager)
{
    XMLTranscoder* result = NULL;
	resValue = XMLTransService::Ok;
    OSStatus status = noErr;
    
    TECObjectRef textToUnicode = NULL;
    TECObjectRef unicodeToText = NULL;
    
    //  We convert to and from utf16
    TextEncoding utf16Encoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
                                        kTextEncodingDefaultVariant,
                                        kUnicode16BitFormat);

    //  Create a TEC from our encoding to utf16
    if (status == noErr)
        status = TECCreateConverter(&textToUnicode, textEncoding, utf16Encoding);

    //  Create a TEC from utf16 to our encoding
    if (status == noErr)
        status = TECCreateConverter(&unicodeToText, utf16Encoding, textEncoding);

	if (status != noErr)
	{
        //  Clean up on error
		if (textToUnicode != NULL)
            TECDisposeConverter(textToUnicode);
			
		if (unicodeToText != NULL)
            TECDisposeConverter(unicodeToText);

		resValue = XMLTransService::UnsupportedEncoding;
	}
	else
    {
        //  Create our transcoder, passing in the converters
		result = new (manager) MacOSTranscoder(encodingName, textToUnicode, unicodeToText, blockSize, manager);
    }
	
    return result;
}


// ---------------------------------------------------------------------------
//  IsMacOSUnicodeConverterSupported
// ---------------------------------------------------------------------------
bool
MacOSUnicodeConverter::IsMacOSUnicodeConverterSupported(void)
{
    return UpgradeScriptInfoToTextEncoding != (void*)NULL
        && CreateTextToUnicodeInfoByEncoding != (void*)NULL
        ;
}


// ---------------------------------------------------------------------------
//  MacOSTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
MacOSTranscoder::MacOSTranscoder(const  XMLCh* const    encodingName
								, TECObjectRef          textToUnicode
								, TECObjectRef          unicodeToText
                                , const XMLSize_t       blockSize
                                , MemoryManager* const  manager) :
    XMLTranscoder(encodingName, blockSize, manager),
    mTextToUnicode(textToUnicode),
    mUnicodeToText(unicodeToText)
{
}


MacOSTranscoder::~MacOSTranscoder()
{
	//	Dispose our text encoding converters
	TECDisposeConverter(mTextToUnicode);
	TECDisposeConverter(mUnicodeToText);
}


// ---------------------------------------------------------------------------
//  MacOSTranscoder: The virtual transcoder API
// ---------------------------------------------------------------------------

XMLSize_t
MacOSTranscoder::transcodeFrom(  const  XMLByte* const          srcData
                                , const XMLSize_t               srcCount
                                ,       XMLCh* const            toFill
                                , const XMLSize_t               maxChars
                                ,       XMLSize_t&              bytesEaten
                                ,       unsigned char* const    charSizes)
{
	//  Reset the tec state (since we don't know that we're part of a
	//  larger run of text).
	TECClearConverterContextInfo(mTextToUnicode);
	
    //  Do the conversion
    ByteCount bytesConsumed = 0;
    ByteCount bytesProduced = 0;
    OSStatus status = TECConvertText(mTextToUnicode,
                (ConstTextPtr) srcData,
                srcCount,                   // inputBufferLength
                &bytesConsumed,				// actualInputLength
                (TextPtr) toFill,           // outputBuffer
                maxChars * sizeof(XMLCh),	// outputBufferLength
                &bytesProduced);			// actualOutputLength

    //  Ignorable error codes
    if(    status == kTECUsedFallbacksStatus
        || status == kTECOutputBufferFullStatus
        || status == kTECPartialCharErr
		)
        status = noErr;
    	
    if (status != noErr)
        ThrowXML(TranscodingException, XMLExcepts::Trans_BadSrcSeq);
	
	std::size_t charsProduced = bytesProduced / sizeof(XMLCh);
	
    bytesEaten = bytesConsumed;
    return charsProduced;
}


XMLSize_t
MacOSTranscoder::transcodeTo(const  XMLCh* const    srcData
                            , const XMLSize_t       srcCount
                            ,       XMLByte* const  toFill
                            , const XMLSize_t       maxBytes
                            ,       XMLSize_t&      charsEaten
                            , const UnRepOpts       options)
{
	//  Reset the tec state (since we don't know that we're part of a
	//  larger run of text).
	TECClearConverterContextInfo(mUnicodeToText);
	
    //  Do the conversion
    ByteCount bytesConsumed = 0;
    ByteCount bytesProduced = 0;
    OSStatus status = TECConvertText(mUnicodeToText,
                (ConstTextPtr) srcData,
                srcCount * sizeof(XMLCh),   // inputBufferLength
                &bytesConsumed,				// actualInputLength
                (TextPtr) toFill,           // outputBuffer
                maxBytes,                   // outputBufferLength
                &bytesProduced);			// actualOutputLength

    //  Ignorable error codes
    if(    status == kTECUsedFallbacksStatus
        || status == kTECOutputBufferFullStatus
        || status == kTECPartialCharErr
		)
        status = noErr;
        
    std::size_t charsConsumed = bytesConsumed / sizeof(XMLCh);
    
    //  Deal with errors
    if (status != noErr)
    {
    	if (status == kTECUnmappableElementErr && options == UnRep_Throw)
    	{
    		XMLCh tmpBuf[17];
            XMLString::binToText(srcData[charsConsumed], tmpBuf, 16, 16);
            ThrowXML2
            (
                TranscodingException
                , XMLExcepts::Trans_Unrepresentable
                , tmpBuf
                , getEncodingName()
            );
    	}
    }
	
    charsEaten = charsConsumed;
    return bytesProduced;
}


bool
MacOSTranscoder::canTranscodeTo(const unsigned int toCheck)
{
	//
    //  If the passed value is really a surrogate embedded together, then
    //  we need to break it out into its two chars. Else just one.
    //
    unsigned int    srcCnt = 0;
    UniChar         srcBuf[2];

    if (toCheck & 0xFFFF0000)
    {
        srcBuf[srcCnt++] = XMLCh(toCheck >> 10)   + 0xD800;
        srcBuf[srcCnt++] = XMLCh(toCheck & 0x3FF) + 0xDC00;
    }
    else
    {
        srcBuf[srcCnt++] = XMLCh(toCheck);
    }

	//  Clear the converter state: we're in a new run of text
	TECClearConverterContextInfo(mUnicodeToText);

    //
    //  Use a local temp buffer that would hold any sane multi-byte char
    //  sequence and try to transcode this guy into it.
    //
    char tmpBuf[64];

    ByteCount bytesConsumed = 0;
    ByteCount bytesProduced = 0;
    OSStatus status = TECConvertText(mUnicodeToText,
                (ConstTextPtr) srcBuf,
                srcCnt * sizeof(XMLCh),     // inputBufferLength
                &bytesConsumed,				// actualInputLength
                (TextPtr) tmpBuf,           // outputBuffer
                sizeof(tmpBuf),             // outputBufferLength
                &bytesProduced);			// actualOutputLength

    std::size_t charsConsumed = bytesConsumed / sizeof(XMLCh);
	
	//	Return true if we transcoded the character(s)
	//	successfully
	return status == noErr && charsConsumed == srcCnt;
}


// ---------------------------------------------------------------------------
//  MacOSLCPTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
MacOSLCPTranscoder::MacOSLCPTranscoder(XMLTranscoder* const transcoder, MemoryManager* const manager)
 : mTranscoder(transcoder),
   mManager(manager),
   mMutex (manager)
{
}


MacOSLCPTranscoder::~MacOSLCPTranscoder()
{
	//	Dispose the XMLTranscoder we're using
    delete mTranscoder;
}


// ---------------------------------------------------------------------------
//  MacOSLCPTranscoder: Implementation of the virtual transcoder interface
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//	In order to implement calcRequiredSize we have to go ahead and do the
//	conversion, which seems quite painful. The Mac Unicode converter has
//	no way of saying "don't actually do the conversion." So we end up
//	converting twice. It would be nice if the calling code could do some
//	extra buffering to avoid this result.
// ---------------------------------------------------------------------------
XMLSize_t MacOSLCPTranscoder::calcRequiredSize(const char* const srcText
                                     , MemoryManager* const manager)
{
	if (!srcText)
		return 0;
		
	//  Lock our mutex to gain exclusive access to the transcoder
	//  since the lcp transcoders are used globally.
	XMLMutexLock lock(&mMutex);

	std::size_t totalCharsProduced = 0;

	const char* src = srcText;
	XMLSize_t srcCnt = std::strlen(src);
    
    //  Iterate over the characters, converting into a temporary buffer which we'll discard.
    //  All this to get the size required.
	while (srcCnt > 0)
    {
        TempXMLBuf tmpBuf;
        XMLSize_t bytesConsumed = 0;
		XMLSize_t charsProduced = mTranscoder->transcodeFrom((XMLByte*)src, srcCnt,
														tmpBuf, kTempBufCount,
														bytesConsumed,
														NULL);
		
        src     += bytesConsumed;
        srcCnt  -= bytesConsumed;

        totalCharsProduced += charsProduced;
        
        //  Bail out if nothing more was produced
        if (charsProduced == 0)
            break;
    }

	//	Return number of XMLCh characters required (not counting terminating NULL!)
	return totalCharsProduced;
}


// ---------------------------------------------------------------------------
//	In order to implement calcRequiredSize we have to go ahead and do the
//	conversion, which seems quite painful. The Mac Unicode converter has
//	no way of saying "don't actually do the conversion." So we end up
//	converting twice. It would be nice if the calling code could do some
//	extra buffering to avoid this result.
// ---------------------------------------------------------------------------
XMLSize_t MacOSLCPTranscoder::calcRequiredSize(const XMLCh* const srcText
                                     , MemoryManager* const manager)
{
	if (!srcText)
		return 0;

	//  Lock our mutex to gain exclusive access to the transcoder
	//  since the lcp transcoders are used globally.
	XMLMutexLock lock(&mMutex);
	std::size_t     totalBytesProduced = 0;

	const XMLCh*    src     = srcText;
	XMLSize_t    srcCnt  = XMLString::stringLen(src);
    
    //  Iterate over the characters, converting into a temporary buffer which we'll discard.
    //  All this to get the size required.
    while (srcCnt > 0)
    {
        TempCharBuf tmpBuf;
        XMLSize_t charsConsumed = 0;
		XMLSize_t bytesProduced = mTranscoder->transcodeTo(src, srcCnt,
                                            (XMLByte*)tmpBuf, kTempBufCount,
                                            charsConsumed,
                                            XMLTranscoder::UnRep_RepChar);
        
        src     += charsConsumed;
        srcCnt  -= charsConsumed;

        totalBytesProduced += bytesProduced;
        
        //  Bail out if nothing more was produced
        if (bytesProduced == 0)
            break;
    }

	//	Return number of characters required (not counting terminating NULL!)
	return totalBytesProduced;
}


char*
MacOSLCPTranscoder::transcode(const XMLCh* const srcText,
                              MemoryManager* const manager)
{
	if (!srcText)
		return NULL;

	//  Lock our mutex to gain exclusive access to the transcoder
	//  since the lcp transcoders are used globally.
	XMLMutexLock lock(&mMutex);

	ArrayJanitor<char> result(0);
	const XMLCh* src		= srcText;
	XMLSize_t srcCnt		= XMLString::stringLen(src);
	std::size_t resultCnt	= 0;

    //  Iterate over the characters, buffering into a local temporary
    //  buffer, which we dump into an allocated (and reallocated, as necessary)
    //  string for return.
    while (srcCnt > 0)
    {
		//  Transcode some characters
        TempCharBuf tmpBuf;
        XMLSize_t charsConsumed = 0;
        XMLSize_t bytesProduced = mTranscoder->transcodeTo(src, srcCnt,
                                            (XMLByte*)tmpBuf, kTempBufCount,
                                            charsConsumed,
                                            XMLTranscoder::UnRep_RepChar);
        src     += charsConsumed;
        srcCnt  -= charsConsumed;

		//	Move the data to result buffer, reallocating as needed
		if (bytesProduced > 0)
		{
			//	Allocate space for result
			std::size_t newCnt = resultCnt + bytesProduced;
			ArrayJanitor<char> newResult
            (
                (char*) manager->allocate((newCnt + 1) * sizeof(char)) //new char[newCnt + 1]
                , manager
            );
			if (newResult.get() != NULL)
			{
				//	Incorporate previous result
				if (result.get() != NULL)
					std::memcpy(newResult.get(), result.get(), resultCnt);
				result.reset(newResult.release());

				//	Copy in new data
				std::memcpy(result.get() + resultCnt, tmpBuf, bytesProduced);
				resultCnt = newCnt;
				
                //  Terminate the result
				result[resultCnt] = '\0';					
			}
		}
        else
            break;
    }

    if (!result.get())
	{
		//	No error, and no result: we probably processed a zero length
		//	input, in which case we want a valid zero length output.
		result.reset
        (
            (char*) manager->allocate(sizeof(char))//new char[1]
            , manager
        );
		result[0] = '\0';
	}

	return result.release();
}


XMLCh*
MacOSLCPTranscoder::transcode(const char* const srcText,
                              MemoryManager* const manager)
{
	if (!srcText)
		return NULL;

	//  Lock our mutex to gain exclusive access to the transcoder
	//  since the lcp transcoders are used globally.
	XMLMutexLock lock(&mMutex);

	ArrayJanitor<XMLCh> result(0);
	const char* src			= srcText;
	std::size_t srcCnt		= std::strlen(src);
	std::size_t resultCnt	= 0;

    //  Iterate over the characters, buffering into a local temporary
    //  buffer, which we dump into an allocated (and reallocated, as necessary)
    //  string for return.
    while (srcCnt > 0)
    {
        //  Transcode some characters
		TempXMLBuf tmpBuf;
        XMLSize_t bytesConsumed = 0;
		XMLSize_t charsProduced = mTranscoder->transcodeFrom((XMLByte*)src, srcCnt,
												tmpBuf, kTempBufCount,
												bytesConsumed,
												NULL);
        src     += bytesConsumed;
        srcCnt  -= bytesConsumed;

		//	Move the data to result buffer, reallocating as needed
		if (charsProduced > 0)
		{
			//	Allocate space for result
			std::size_t newCnt = resultCnt + charsProduced;
			ArrayJanitor<XMLCh> newResult
            (
                (XMLCh*) manager->allocate((newCnt + 1) * sizeof(XMLCh)) //new XMLCh[newCnt + 1]
                , manager
            );
			if (newResult.get() != NULL)
			{
				//	Incorporate previous result
				if (result.get() != NULL)
					std::memcpy(newResult.get(), result.get(), resultCnt * sizeof(XMLCh));
				result.reset(newResult.release());

				//	Copy in new data
				std::memcpy(result.get() + resultCnt, tmpBuf, charsProduced * sizeof(XMLCh));
				resultCnt = newCnt;
				
				result[resultCnt] = 0;			
			}
		}
        else
            break;
    }

    if (!result.get())
	{
		//	No error, and no result: we probably processed a zero length
		//	input, in which case we want a valid zero length output.
		result.reset
        (
            (XMLCh*) manager->allocate(sizeof(XMLCh))//new XMLCh[1]
            , manager
        );
		result[0] = '\0';
	}
	
	return result.release();
}


bool
MacOSLCPTranscoder::transcode( 		 const   char* const	toTranscode
                                    ,       XMLCh* const    toFill
                                    , const XMLSize_t       maxChars
                                    , MemoryManager* const  manager)
{
    // toFill must contain space for maxChars XMLCh characters + 1 (for terminating NULL).

    // Check for a couple of psycho corner cases
    if (!toTranscode || !maxChars || !*toTranscode)
    {
        toFill[0] = 0;
        return true;
    }

	//  Lock our mutex to gain exclusive access to the transcoder
	//  since the lcp transcoders are used globally.
	XMLMutexLock lock(&mMutex);

    //  Call the transcoder to do the work
    XMLSize_t srcLen = std::strlen(toTranscode);
    XMLSize_t bytesConsumed = 0;
    XMLSize_t charsProduced = mTranscoder->transcodeFrom((XMLByte*)toTranscode, srcLen,
                                            toFill, maxChars,
											bytesConsumed,
											NULL);

    //	Zero terminate the output string
    toFill[charsProduced] = L'\0';
    
    //  Return true if we consumed all of the characters
    return (bytesConsumed == srcLen);
}


bool
MacOSLCPTranscoder::transcode( 		const   XMLCh* const    toTranscode
                                    ,       char* const     toFill
                                    , const XMLSize_t       maxChars
                                    , MemoryManager* const  manager)
{
    //	toFill must contain space for maxChars bytes + 1 (for terminating NULL).

    // Check for a couple of psycho corner cases
    if (!toTranscode || !maxChars || !*toTranscode)
    {
        toFill[0] = 0;
        return true;
    }

	//  Lock our mutex to gain exclusive access to the transcoder
	//  since the lcp transcoders are used globally.
	XMLMutexLock lock(&mMutex);

    //  Call the transcoder to do the work
    XMLSize_t srcLen = XMLString::stringLen(toTranscode);
    XMLSize_t charsConsumed = 0;
    XMLSize_t bytesProduced = mTranscoder->transcodeTo(toTranscode, srcLen,
                                            (XMLByte*)toFill, maxChars,
                                            charsConsumed,
                                            XMLTranscoder::UnRep_RepChar);

    //	Zero terminate the output string
    toFill[bytesProduced] = '\0';
    
    //  Return true if we consumed all of the characters
    return (charsConsumed == srcLen);
}


XERCES_CPP_NAMESPACE_END
