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
 * $Id: Win32TransService.cpp 676954 2008-07-15 16:29:19Z dbertoni $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if HAVE_CONFIG_H
#	include <config.h>
#endif

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/XMLException.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/RefHashTableOf.hpp>
#include "Win32TransService.hpp"

XERCES_CPP_NAMESPACE_BEGIN


// ---------------------------------------------------------------------------
//  Local, const data
// ---------------------------------------------------------------------------
static const XMLCh gMyServiceId[] =
{
    chLatin_W, chLatin_i, chLatin_n, chDigit_3, chDigit_2, chNull
};


#if !HAVE_WCSUPR
void _wcsupr(LPWSTR str)
{
    int nLen=XMLString::stringLen(str);
    ::LCMapStringW( GetThreadLocale(), LCMAP_UPPERCASE, str, nLen, str, nLen);
}
#endif

#if !HAVE_WCSLWR
void _wcslwr(LPWSTR str)
{
    int nLen=XMLString::stringLen(str);
    ::LCMapStringW( GetThreadLocale(), LCMAP_LOWERCASE, str, nLen, str, nLen);
}
#endif

#if !HAVE_WCSNICMP
int _wcsnicmp(LPCWSTR comp1, LPCWSTR comp2, unsigned int nLen)
{
    unsigned int len = XMLString::stringLen( comp1);
    unsigned int otherLen = XMLString::stringLen( comp2);
    unsigned int countChar = 0;
    unsigned int maxChars;
    int          theResult = 0;

    // Determine at what string index the comparison stops.
    len = ( len > nLen ) ? nLen : len;
    otherLen = ( otherLen > nLen ) ? nLen : otherLen;
    maxChars = ( len > otherLen ) ? otherLen : len;

    // Handle situation when one argument or the other is NULL
    // by returning +/- string length of non-NULL argument (inferred
    // from XMLString::CompareNString).

    // Obs. Definition of stringLen(XMLCh*) implies NULL ptr and ptr
    // to Empty String are equivalent.  It handles NULL args, BTW.

    if ( !comp1 )
    {
        // Negative because null ptr (c1) less than string (c2).
        return ( 0 - otherLen );
    }

    if ( !comp2 )
    {
        // Positive because string (c1) still greater than null ptr (c2).
        return len;
    }

    // Copy const parameter strings (plus terminating nul) into locals.
    XMLCh* firstBuf = (XMLCh*) XMLPlatformUtils::fgMemoryManager->allocate( (++len) * sizeof(XMLCh) );//new XMLCh[ ++len];
    XMLCh* secondBuf = (XMLCh*) XMLPlatformUtils::fgMemoryManager->allocate( (++otherLen) * sizeof(XMLCh) );//new XMLCh[ ++otherLen];
    memcpy( firstBuf, comp1, len * sizeof(XMLCh));
    memcpy( secondBuf, comp2, otherLen * sizeof(XMLCh));

    // Then uppercase both strings, losing their case info.
    ::LCMapStringW( GetThreadLocale(), LCMAP_UPPERCASE, (LPWSTR)firstBuf, len, (LPWSTR)firstBuf, len);
    ::LCMapStringW( GetThreadLocale(), LCMAP_UPPERCASE, (LPWSTR)secondBuf, otherLen, (LPWSTR)secondBuf, otherLen);

    // Strings are equal until proven otherwise.
    while ( ( countChar < maxChars ) && ( !theResult ) )
    {
        theResult = (int)(firstBuf[countChar]) - (int)(secondBuf[countChar]);
        ++countChar;
    }

    XMLPlatformUtils::fgMemoryManager->deallocate(firstBuf);//delete [] firstBuf;
    XMLPlatformUtils::fgMemoryManager->deallocate(secondBuf);//delete [] secondBuf;

    return theResult;
}
#endif

#if !HAVE_WCSICMP
int _wcsicmp(LPCWSTR comp1, LPCWSTR comp2)
{
    unsigned int len = XMLString::stringLen( comp1);
    unsigned int otherLen = XMLString::stringLen( comp2);
    // Must compare terminating NUL to return difference if one string is shorter than the other.
    unsigned int maxChars = ( len > otherLen ) ? otherLen : len;
    return _wcsnicmp(comp1, comp2, maxChars+1);
}
#endif

// it's a local function (instead of a static function) so that we are not 
// forced to include <windows.h> in the header
bool isAlias(const   HKEY            encodingKey
             ,       char* const     aliasBuf = 0
             , const unsigned int    nameBufSz = 0)
{
    unsigned long theType;
    unsigned long theSize = nameBufSz;
    return (::RegQueryValueExA
    (
        encodingKey
        , "AliasForCharset"
        , 0
        , &theType
        , (unsigned char*)aliasBuf
        , &theSize
    ) == ERROR_SUCCESS);
}

// ---------------------------------------------------------------------------
//  This is the simple CPMapEntry class. It just contains an encoding name
//  and a code page for that encoding.
// ---------------------------------------------------------------------------
class CPMapEntry : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    CPMapEntry
    (
        const   XMLCh* const    encodingName
        , const unsigned int    ieId
        , MemoryManager*        manager
    );

    CPMapEntry
    (
        const   char* const     encodingName
        , const unsigned int    ieId
        , MemoryManager*        manager
    );

    ~CPMapEntry();


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    const XMLCh* getEncodingName() const;
    const XMLCh* getKey() const;
    unsigned int getIEEncoding() const;


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    CPMapEntry();
    CPMapEntry(const CPMapEntry&);
    CPMapEntry& operator=(const CPMapEntry&);


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fEncodingName
    //      This is the encoding name for the code page that this instance
    //      represents.
    //
    //  fIEId
    //      This is the code page id.
    // -----------------------------------------------------------------------
    XMLCh*          fEncodingName;
    unsigned int    fIEId;
    MemoryManager*  fManager;
};

// ---------------------------------------------------------------------------
//  CPMapEntry: Constructors and Destructor
// ---------------------------------------------------------------------------
CPMapEntry::CPMapEntry( const   char* const     encodingName
                        , const unsigned int    ieId
                        , MemoryManager*        manager) :
    fEncodingName(0)
    , fIEId(ieId)
    , fManager(manager)
{
    // Transcode the name to Unicode and store that copy
    int targetLen=::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, encodingName, -1, NULL, 0);
    if(targetLen!=0)
    {
        fEncodingName = (XMLCh*) fManager->allocate
        (
            (targetLen + 1) * sizeof(XMLCh)
        );//new XMLCh[targetLen + 1];
        ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, encodingName, -1, (LPWSTR)fEncodingName, targetLen);
        fEncodingName[targetLen] = 0;

        //
        //  Upper case it because we are using a hash table and need to be
        //  sure that we find all case combinations.
        //
        _wcsupr(fEncodingName);
  }
}

CPMapEntry::CPMapEntry( const   XMLCh* const    encodingName
                        , const unsigned int    ieId
                        , MemoryManager*        manager) :

    fEncodingName(0)
    , fIEId(ieId)
    , fManager(manager)
{
    fEncodingName = XMLString::replicate(encodingName, fManager);

    //
    //  Upper case it because we are using a hash table and need to be
    //  sure that we find all case combinations.
    //
    _wcsupr(fEncodingName);
}

CPMapEntry::~CPMapEntry()
{
    fManager->deallocate(fEncodingName);//delete [] fEncodingName;
}


// ---------------------------------------------------------------------------
//  CPMapEntry: Getter methods
// ---------------------------------------------------------------------------
const XMLCh* CPMapEntry::getEncodingName() const
{
    return fEncodingName;
}

unsigned int CPMapEntry::getIEEncoding() const
{
    return fIEId;
}


static bool onXPOrLater = false;


//---------------------------------------------------------------------------
//
//  class Win32TransService Implementation ...
//
//---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  Win32TransService: Constructors and Destructor
// ---------------------------------------------------------------------------
Win32TransService::Win32TransService(MemoryManager* manager) :
    fCPMap(NULL)
    , fManager(manager)
{
    // Figure out if we are on XP or later and save that flag for later use.
    // We need this because of certain code page conversion calls.
    OSVERSIONINFO   OSVer;
    OSVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    ::GetVersionEx(&OSVer);

    if ((OSVer.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
        ((OSVer.dwMajorVersion == 5) && (OSVer.dwMinorVersion > 0)))
    {
        onXPOrLater = true;
    }

    fCPMap = new RefHashTableOf<CPMapEntry>(109);

    //
    //  Open up the registry key that contains the info we want. Note that,
    //  if this key does not exist, then we just return. It will just mean
    //  that we don't have any support except for intrinsic encodings supported
    //  by the parser itself (and the LCP support of course.
    //
    HKEY charsetKey;
    if (::RegOpenKeyExA
    (
        HKEY_CLASSES_ROOT
        , "MIME\\Database\\Charset"
        , 0
        , KEY_READ
        , &charsetKey))
    {
        return;
    }

    //
    //  Read in the registry keys that hold the code page ids. Skip for now
    //  those entries which indicate that they are aliases for some other
    //  encodings. We'll come back and do a second round for those and look
    //  up the original name and get the code page id.
    //
    //  Note that we have to use A versions here so that this will run on
    //  98, and transcode the strings to Unicode.
    //
    const unsigned int nameBufSz = 1024;
    char nameBuf[nameBufSz + 1];
    unsigned int subIndex;
    unsigned long theSize;
    for (subIndex = 0;;++subIndex)
    {
        // Get the name of the next key
        theSize = nameBufSz;
        if (::RegEnumKeyExA
        (
            charsetKey
            , subIndex
            , nameBuf
            , &theSize
            , 0, 0, 0, 0) == ERROR_NO_MORE_ITEMS)
        {
            break;
        }

        // Open this subkey
        HKEY encodingKey;
        if (::RegOpenKeyExA
        (
            charsetKey
            , nameBuf
            , 0
            , KEY_READ
            , &encodingKey))
        {
            continue;
        }

        //
        //  Lts see if its an alias. If so, then ignore it in this first
        //  loop. Else, we'll add a new entry for this one.
        //
        if (!isAlias(encodingKey))
        {
            //
            //  Lets get the two values out of this key that we are
            //  interested in. There should be a code page entry and an
            //  IE entry.
            //
            //  The Codepage entry is the default code page for a computer using that charset
            //  while the InternetEncoding holds the code page that represents that charset
            //
            unsigned long theType;
            unsigned int CPId;
            unsigned int IEId;

            theSize = sizeof(unsigned int);
            if (::RegQueryValueExA
            (
                encodingKey
                , "Codepage"
                , 0
                , &theType
                , (unsigned char*)&CPId
                , &theSize) != ERROR_SUCCESS)
            {
                ::RegCloseKey(encodingKey);
                continue;
            }

            //
            //  If this is not a valid Id, and it might not be because its
            //  not loaded on this system, then don't take it.
            //
            if (::IsValidCodePage(CPId))
            {
                theSize = sizeof(unsigned int);
                if (::RegQueryValueExA
                (
                    encodingKey
                    , "InternetEncoding"
                    , 0
                    , &theType
                    , (unsigned char*)&IEId
                    , &theSize) != ERROR_SUCCESS)
                {
                    ::RegCloseKey(encodingKey);
                    continue;
                }

                CPMapEntry* newEntry = new (fManager) CPMapEntry(nameBuf, IEId, fManager);
                fCPMap->put((void*)newEntry->getEncodingName(), newEntry);
            }
        }

        // And close the subkey handle
        ::RegCloseKey(encodingKey);
    }

    //
    //  Now loop one more time and this time we do just the aliases. For
    //  each one we find, we look up that name in the map we've already
    //  built and add a new entry with this new name and the same id
    //  values we stored for the original.
    //
    char aliasBuf[nameBufSz + 1];
    for (subIndex = 0;;++subIndex)
    {
        // Get the name of the next key
        theSize = nameBufSz;
        if (::RegEnumKeyExA
        (
            charsetKey
            , subIndex
            , nameBuf
            , &theSize
            , 0, 0, 0, 0) == ERROR_NO_MORE_ITEMS)
        {
            break;
        }

        // Open this subkey
        HKEY encodingKey;
        if (::RegOpenKeyExA
        (
            charsetKey
            , nameBuf
            , 0
            , KEY_READ
            , &encodingKey))
        {
            continue;
        }

        //
        //  If its an alias, look up the name in the map. If we find it,
        //  then construct a new one with the new name and the aliased
        //  ids.
        //
        if (isAlias(encodingKey, aliasBuf, nameBufSz))
        {
            int targetLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, aliasBuf, -1, NULL, 0);
            if(targetLen!=0)
            {
                XMLCh* uniAlias = (XMLCh*) fManager->allocate
                (
                    (targetLen + 1) * sizeof(XMLCh)
                );//new XMLCh[targetLen + 1];
                ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, aliasBuf, -1, (LPWSTR)uniAlias, targetLen);
                uniAlias[targetLen] = 0;
                _wcsupr(uniAlias);

                // Look up the alias name
                CPMapEntry* aliasedEntry = fCPMap->get(uniAlias);
                if (aliasedEntry)
                {
                    int targetLen = ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, nameBuf, -1, NULL, 0);
                    if(targetLen!=0)
                    {
                        XMLCh* uniName = (XMLCh*) fManager->allocate
                        (
                            (targetLen + 1) * sizeof(XMLCh)
                        );//new XMLCh[targetLen + 1];
                        ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, nameBuf, -1, (LPWSTR)uniName, targetLen);
                        uniName[targetLen] = 0;
                        _wcsupr(uniName);

                        //
                        //  If the name is actually different, then take it.
                        //  Otherwise, don't take it. They map aliases that are
                        //  just different case.
                        //
						if (!XMLString::equals(uniName, aliasedEntry->getEncodingName()))
                        {
                            CPMapEntry* newEntry = new (fManager) CPMapEntry(uniName, aliasedEntry->getIEEncoding(), fManager);
                            fCPMap->put((void*)newEntry->getEncodingName(), newEntry);
                        }

                        fManager->deallocate(uniName);//delete [] uniName;
                    }
                }
                fManager->deallocate(uniAlias);//delete [] uniAlias;
            }
        }

        // And close the subkey handle
        ::RegCloseKey(encodingKey);
    }

    // And close the main key handle
    ::RegCloseKey(charsetKey);
}

Win32TransService::~Win32TransService()
{
    delete fCPMap;
}


// ---------------------------------------------------------------------------
//  Win32TransService: The virtual transcoding service API
// ---------------------------------------------------------------------------
int Win32TransService::compareIString(  const   XMLCh* const    comp1
                                        , const XMLCh* const    comp2)
{
    return _wcsicmp(comp1, comp2);
}


int Win32TransService::compareNIString( const   XMLCh* const    comp1
                                        , const XMLCh* const    comp2
                                        , const XMLSize_t       maxChars)
{
    return _wcsnicmp(comp1, comp2, maxChars);
}


const XMLCh* Win32TransService::getId() const
{
    return gMyServiceId;
}

XMLLCPTranscoder* Win32TransService::makeNewLCPTranscoder(MemoryManager* manager)
{
    // Just allocate a new LCP transcoder of our type
    return new (manager) Win32LCPTranscoder;
}


bool Win32TransService::supportsSrcOfs() const
{
    //
    //  Since the only mechanism we have to translate XML text in this
    //  transcoder basically require us to do work that allows us to support
    //  source offsets, we might as well do it.
    //
    return true;
}


void Win32TransService::upperCase(XMLCh* const toUpperCase)
{
    _wcsupr(toUpperCase);
}

void Win32TransService::lowerCase(XMLCh* const toLowerCase)
{
    _wcslwr(toLowerCase);
}

XMLTranscoder*
Win32TransService::makeNewXMLTranscoder(const   XMLCh* const            encodingName
                                        ,       XMLTransService::Codes& resValue
                                        , const XMLSize_t               blockSize
                                        ,       MemoryManager* const    manager)
{
    const XMLSize_t upLen = 1024;
    XMLCh upEncoding[upLen + 1];

    //
    //  Get an upper cased copy of the encoding name, since we use a hash
    //  table and we store them all in upper case.
    //
    XMLString::copyNString(upEncoding, encodingName, upLen);
    _wcsupr(upEncoding);

    // Now to try to find this guy in the CP map
    CPMapEntry* theEntry = fCPMap->get(upEncoding);

    // If not found, then return a null pointer
    if (!theEntry)
    {
        resValue = XMLTransService::UnsupportedEncoding;
        return 0;
    }

    // We found it, so return a Win32 transcoder for this encoding
    return new (manager) Win32Transcoder
    (
        encodingName
        , theEntry->getIEEncoding()
        , blockSize
        , manager
    );
}








//---------------------------------------------------------------------------
//
//  class Win32Transcoder Implementation ...
//
//---------------------------------------------------------------------------


inline DWORD
getFlagsValue(
            UINT    idCP,
            DWORD   desiredFlags)
{
    if (idCP == 50220 ||
        idCP == 50227 ||
        (idCP >= 57002 &&
         idCP <= 57011))
    {
        // These code pages do not support any
        // flag options.
        return 0;
    }
    else if (idCP == 65001)
    {
        // UTF-8 only supports MB_ERR_INVALID_CHARS on
        // versions of Windows since XP
        if (!onXPOrLater)
        {
            return 0;
        }
        else
        {
            return desiredFlags & MB_ERR_INVALID_CHARS ?
                        MB_ERR_INVALID_CHARS : 0;
        }
    }
    else
    {
        return desiredFlags;
    }
}



// ---------------------------------------------------------------------------
//  Win32Transcoder: Constructors and Destructor
// ---------------------------------------------------------------------------
Win32Transcoder::Win32Transcoder(const  XMLCh* const   encodingName
                                , const unsigned int   ieCP
                                , const XMLSize_t      blockSize
                                , MemoryManager* const manager) :

    XMLTranscoder(encodingName, blockSize, manager)
    , fIECP(ieCP)
    , fUsedDef(FALSE)
    , fPtrUsedDef(0)
    , fFromFlags(getFlagsValue(ieCP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS))
#if defined(WC_NO_BEST_FIT_CHARS)
    , fToFlags(getFlagsValue(ieCP, WC_COMPOSITECHECK | WC_SEPCHARS | WC_NO_BEST_FIT_CHARS))
#else
    , fToFlags(getFlagsValue(ieCP, WC_COMPOSITECHECK | WC_SEPCHARS))
#endif
{
    // Some code pages require that MultiByteToWideChar and WideCharToMultiByte
    // be passed 0 for their second parameters (dwFlags).  If that's the case,
    // it's also necessary to pass null pointers for the last two parameters
    // to WideCharToMultiByte.  This is apparently because it's impossible to
    // determine whether or not a substitution (replacement) character was used.
    if (fToFlags)
    {
        fPtrUsedDef = &fUsedDef;
    }
}

Win32Transcoder::~Win32Transcoder()
{
}


// ---------------------------------------------------------------------------
//  Win32Transcoder: The virtual transcoder API
// ---------------------------------------------------------------------------
XMLSize_t
Win32Transcoder::transcodeFrom( const   XMLByte* const      srcData
                                , const XMLSize_t           srcCount
                                ,       XMLCh* const        toFill
                                , const XMLSize_t           maxChars
                                ,       XMLSize_t&          bytesEaten
                                ,       unsigned char* const charSizes)
{
    // Get temp pointers to the in and out buffers, and the chars sizes one
    XMLCh*          outPtr = toFill;
    const XMLByte*  inPtr  = srcData;
    unsigned char*  sizesPtr = charSizes;

    // Calc end pointers for each of them
    XMLCh*          outEnd = toFill + maxChars;
    const XMLByte*  inEnd  = srcData + srcCount;

    //
    //  Now loop until we either get our max chars, or cannot get a whole
    //  character from the input buffer.
    //
    bytesEaten = 0;
    while ((outPtr < outEnd) && (inPtr < inEnd))
    {
        //
        //  If we are looking at a leading byte of a multibyte sequence,
        //  then we are going to eat 2 bytes, else 1.
        //
        unsigned char toEat = ::IsDBCSLeadByteEx(fIECP, *inPtr) ?
                                    2 : 1;

        // Make sure a whole char is in the source
        if (inPtr + toEat > inEnd)
            break;

        // Try to translate this next char and check for an error
        const unsigned int converted = ::MultiByteToWideChar
        (
            fIECP
            , fFromFlags
            , (const char*)inPtr
            , toEat
            , outPtr
            , 1
        );

        if (converted != 1)
        {
            if (toEat == 1)
            {
                XMLCh tmpBuf[17];
                XMLString::binToText((unsigned int)(*inPtr), tmpBuf, 16, 16, getMemoryManager());
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

        // Update the char sizes array for this round
        *sizesPtr++ = toEat;

        // And update the bytes eaten count
        bytesEaten += toEat;

        // And update our in/out ptrs
        inPtr += toEat;
        outPtr++;
    }

    // Return the chars we output
    return (outPtr - toFill);
}


XMLSize_t
Win32Transcoder::transcodeTo(const  XMLCh* const    srcData
                            , const XMLSize_t       srcCount
                            ,       XMLByte* const  toFill
                            , const XMLSize_t       maxBytes
                            ,       XMLSize_t&      charsEaten
                            , const UnRepOpts       options)
{
    // Get pointers to the start and end of each buffer
    const XMLCh*    srcPtr = srcData;
    const XMLCh*    srcEnd = srcData + srcCount;
    XMLByte*        outPtr = toFill;
    XMLByte*        outEnd = toFill + maxBytes;

    //
    //  Now loop until we either get our max chars, or cannot get a whole
    //  character from the input buffer.
    //
    //  NOTE: We have to use a loop for this unfortunately because the
    //  conversion API is too dumb to tell us how many chars it converted if
    //  it couldn't do the whole source.
    //
    fUsedDef = FALSE;
    while ((outPtr < outEnd) && (srcPtr < srcEnd))
    {
        //
        //  Do one char and see if it made it.
        const int bytesStored = ::WideCharToMultiByte
        (
            fIECP
            , fToFlags
            , srcPtr
            , 1
            , (char*)outPtr
            , (int)(outEnd - outPtr)
            , 0
            , fPtrUsedDef
        );

        // If we didn't transcode anything, then we are done
        if (!bytesStored)
            break;

        //
        //  If the defaault char was used and the options indicate that
        //  this isn't allowed, then throw.
        //
        if (fUsedDef && (options == UnRep_Throw))
        {
            XMLCh tmpBuf[17];
            XMLString::binToText((unsigned int)*srcPtr, tmpBuf, 16, 16, getMemoryManager());
            ThrowXMLwithMemMgr2
            (
                TranscodingException
                , XMLExcepts::Trans_Unrepresentable
                , tmpBuf
                , getEncodingName()
                , getMemoryManager()
            );
        }

        // Update our pointers
        outPtr += bytesStored;
        srcPtr++;
    }

    // Update the chars eaten
    charsEaten = srcPtr - srcData;

    // And return the bytes we stored
    return outPtr - toFill;
}


bool Win32Transcoder::canTranscodeTo(const unsigned int toCheck)
{
    //
    //  If the passed value is really a surrogate embedded together, then
    //  we need to break it out into its two chars. Else just one.
    //
    XMLCh           srcBuf[2];
    unsigned int    srcCount = 1;
    if (toCheck & 0xFFFF0000)
    {
        srcBuf[0] = XMLCh((toCheck >> 10) + 0xD800);
        srcBuf[1] = XMLCh((toCheck & 0x3FF) + 0xDC00);
        srcCount++;
    }
     else
    {
        srcBuf[0] = XMLCh(toCheck);
    }

    //
    //  Use a local temp buffer that would hold any sane multi-byte char
    //  sequence and try to transcode this guy into it.
    //
    char tmpBuf[64];

    fUsedDef = FALSE;

    const unsigned int bytesStored = ::WideCharToMultiByte
    (
        fIECP
        , fToFlags
        , srcBuf
        , srcCount
        , tmpBuf
        , 64
        , 0
        , fPtrUsedDef
    );

    if (!bytesStored || fUsedDef)
        return false;

    return true;
}




//---------------------------------------------------------------------------
//
//  class Win32Transcoder Implementation ...
//
//---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Win32LCPTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
Win32LCPTranscoder::Win32LCPTranscoder()
{
}

Win32LCPTranscoder::~Win32LCPTranscoder()
{
}


// ---------------------------------------------------------------------------
//  Win32LCPTranscoder: Implementation of the virtual transcoder interface
// ---------------------------------------------------------------------------
XMLSize_t Win32LCPTranscoder::calcRequiredSize(const char* const srcText
                                                  , MemoryManager* const /*manager*/)
{
    if (!srcText)
        return 0;

    return ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, srcText, -1, NULL, 0);
}


XMLSize_t Win32LCPTranscoder::calcRequiredSize(const XMLCh* const srcText
                                                  , MemoryManager* const /*manager*/)
{
    if (!srcText)
        return 0;

    return ::WideCharToMultiByte(CP_ACP, 0, srcText, -1, NULL, 0, NULL, NULL);
}

char* Win32LCPTranscoder::transcode(const XMLCh* const toTranscode,
                                    MemoryManager* const manager)
{
    if (!toTranscode)
        return 0;

    char* retVal = 0;
    if (*toTranscode)
    {
        // Calc the needed size
        const XMLSize_t neededLen = calcRequiredSize(toTranscode, manager);

        // Allocate a buffer of that size plus one for the null and transcode
        retVal = (char*) manager->allocate((neededLen + 1) * sizeof(char)); //new char[neededLen + 1];
        ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)toTranscode, -1, retVal, (int)neededLen+1, NULL, NULL);

        // And cap it off anyway just to make sure
        retVal[neededLen] = 0;
    }
     else
    {
        retVal = (char*) manager->allocate(sizeof(char)); //new char[1];
        retVal[0] = 0;
    }
    return retVal;
}

XMLCh* Win32LCPTranscoder::transcode(const char* const toTranscode,
                                     MemoryManager* const manager)
{
    if (!toTranscode)
        return 0;

    XMLCh* retVal = 0;
    if (*toTranscode)
    {
        // Calculate the buffer size required
        const XMLSize_t neededLen = calcRequiredSize(toTranscode, manager);
        if (neededLen == 0)
        {
            retVal = (XMLCh*) manager->allocate(sizeof(XMLCh)); //new XMLCh[1];
            retVal[0] = 0;
            return retVal;
        }

        // Allocate a buffer of that size plus one for the null and transcode
        retVal = (XMLCh*) manager->allocate((neededLen + 1) * sizeof(XMLCh)); //new XMLCh[neededLen + 1];
        ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, toTranscode, -1, (LPWSTR)retVal, (int)neededLen + 1);

        // Cap it off just to make sure. We are so paranoid!
        retVal[neededLen] = 0;
    }
     else
    {
        retVal = (XMLCh*) manager->allocate(sizeof(XMLCh)); //new XMLCh[1];
        retVal[0] = 0;
    }
    return retVal;
}


bool Win32LCPTranscoder::transcode( const   char* const     toTranscode
                                    ,       XMLCh* const    toFill
                                    , const XMLSize_t       maxChars
                                    , MemoryManager* const  /*manager*/)
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

    // This one has a fixed size output, so try it and if it fails it fails
    if ( 0 == ::MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, toTranscode, -1, (LPWSTR)toFill, (int)(maxChars + 1)) )
        return false;
    return true;
}


bool Win32LCPTranscoder::transcode( const   XMLCh* const    toTranscode
                                    ,       char* const     toFill
                                    , const XMLSize_t       maxBytes
                                    , MemoryManager* const  /*manager*/)
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

    // This one has a fixed size output, so try it and if it fails it fails
    if ( 0 == ::WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)toTranscode, -1, toFill, (int)(maxBytes + 1), NULL, NULL) )
        return false;

    // Cap it off just in case
    toFill[maxBytes] = 0;
    return true;
}


XERCES_CPP_NAMESPACE_END
