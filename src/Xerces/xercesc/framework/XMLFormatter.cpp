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

/**
 * $Id: XMLFormatter.cpp 901107 2010-01-20 08:45:02Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/TransService.hpp>
#include <xercesc/util/TranscodingException.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/framework/XMLFormatter.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/XMLChar.hpp>

#include <string.h>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local data
//
//  gXXXRef
//      These are hard coded versions of the char refs we put out for the
//      standard char refs.
//
//  gEscapeChars
//      For each style of escape, we have a list of the chars that must
//      be escaped for that style. The first null hit in each list indicates
//      no more valid entries in that list. The first entry is a dummy for
//      the NoEscapes style.
// ---------------------------------------------------------------------------
static const XMLCh  gAmpRef[] =
{
    chAmpersand, chLatin_a, chLatin_m, chLatin_p, chSemiColon, chNull
};

static const XMLCh  gAposRef[] =
{
    chAmpersand, chLatin_a, chLatin_p, chLatin_o, chLatin_s, chSemiColon, chNull
};

static const XMLCh  gGTRef[] =
{
    chAmpersand, chLatin_g, chLatin_t, chSemiColon, chNull
};

static const XMLCh  gLTRef[] =
{
    chAmpersand, chLatin_l, chLatin_t, chSemiColon, chNull
};

static const XMLCh  gQuoteRef[] =
{
    chAmpersand, chLatin_q, chLatin_u, chLatin_o, chLatin_t, chSemiColon, chNull
};

static const unsigned int kEscapeCount = 7;
static const XMLCh gEscapeChars[XMLFormatter::EscapeFlags_Count][kEscapeCount] =
{
        { chNull      , chNull       , chNull        , chNull       , chNull        , chNull    , chNull }
    ,   { chAmpersand , chCloseAngle , chDoubleQuote , chOpenAngle  , chSingleQuote , chNull    , chNull }
    ,   { chAmpersand , chOpenAngle  , chDoubleQuote , chLF         , chCR          , chHTab    , chNull }
    ,   { chAmpersand , chOpenAngle  , chCloseAngle  , chNull       , chNull        , chNull    , chNull }
};

// ---------------------------------------------------------------------------
//  Local methods
// ---------------------------------------------------------------------------
bool XMLFormatter::inEscapeList(const XMLFormatter::EscapeFlags escStyle
                              , const XMLCh                     toCheck)
{
    const XMLCh* escList = gEscapeChars[escStyle];
    while (*escList)
    {
        if (*escList++ == toCheck)
            return true;
    }

    /***
     *  XML1.1
     *
     *  Finally, there is considerable demand to define a standard representation of
     *  arbitrary Unicode characters in XML documents. Therefore, XML 1.1 allows the
     *  use of character references to the control characters #x1 through #x1F,
     *  most of which are forbidden in XML 1.0. For reasons of robustness, however,
     *  these characters still cannot be used directly in documents.
     *  In order to improve the robustness of character encoding detection, the
     *  additional control characters #x7F through #x9F, which were freely allowed in
     *  XML 1.0 documents, now must also appear only as character references.
     *  (Whitespace characters are of course exempt.) The minor sacrifice of backward
     *  compatibility is considered not significant.
     *  Due to potential problems with APIs, #x0 is still forbidden both directly and
     *  as a character reference.
     *
    ***/
    if (fIsXML11)
    {
        // for XML11
        if ( XMLChar1_1::isControlChar(toCheck, 0) &&
            !XMLChar1_1::isWhitespace(toCheck, 0)   )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }

}


// ---------------------------------------------------------------------------
//  XMLFormatter: Constructors and Destructor
// ---------------------------------------------------------------------------
XMLFormatter::XMLFormatter( const   char* const             outEncoding
                            , const char* const             docVersion
                            ,       XMLFormatTarget* const  target
                            , const EscapeFlags             escapeFlags
                            , const UnRepFlags              unrepFlags
                            ,       MemoryManager* const    manager)
    : fEscapeFlags(escapeFlags)
    , fOutEncoding(0)
    , fTarget(target)
    , fUnRepFlags(unrepFlags)
    , fXCoder(0)
    , fAposRef(0)
    , fAposLen(0)
    , fAmpRef(0)
    , fAmpLen(0)
    , fGTRef(0)
    , fGTLen(0)
    , fLTRef(0)
    , fLTLen(0)
    , fQuoteRef(0)
    , fQuoteLen(0)
    , fIsXML11(false)
    , fMemoryManager(manager)
{
    // Transcode the encoding string
    fOutEncoding = XMLString::transcode(outEncoding, fMemoryManager);

    // Try to create a transcoder for this encoding
    XMLTransService::Codes resCode;
    fXCoder = XMLPlatformUtils::fgTransService->makeNewTranscoderFor
    (
        fOutEncoding
        , resCode
        , kTmpBufSize
        , fMemoryManager
    );

    if (!fXCoder)
    {
        fMemoryManager->deallocate(fOutEncoding); //delete [] fOutEncoding;
        ThrowXMLwithMemMgr1
        (
            TranscodingException
            , XMLExcepts::Trans_CantCreateCvtrFor
            , outEncoding
            , fMemoryManager
        );
    }

    XMLCh* const tmpDocVer = XMLString::transcode(docVersion, fMemoryManager);
    ArrayJanitor<XMLCh> jname(tmpDocVer, fMemoryManager);
    fIsXML11 = XMLString::equals(tmpDocVer, XMLUni::fgVersion1_1);
}


XMLFormatter::XMLFormatter( const   XMLCh* const            outEncoding
                            , const XMLCh* const            docVersion
                            ,       XMLFormatTarget* const  target
                            , const EscapeFlags             escapeFlags
                            , const UnRepFlags              unrepFlags
                            ,       MemoryManager* const    manager)
    : fEscapeFlags(escapeFlags)
    , fOutEncoding(0)
    , fTarget(target)
    , fUnRepFlags(unrepFlags)
    , fXCoder(0)
    , fAposRef(0)
    , fAposLen(0)
    , fAmpRef(0)
    , fAmpLen(0)
    , fGTRef(0)
    , fGTLen(0)
    , fLTRef(0)
    , fLTLen(0)
    , fQuoteRef(0)
    , fQuoteLen(0)
    , fIsXML11(false)
    , fMemoryManager(manager)
{
    // Try to create a transcoder for this encoding
    XMLTransService::Codes resCode;
    fXCoder = XMLPlatformUtils::fgTransService->makeNewTranscoderFor
    (
        outEncoding
        , resCode
        , kTmpBufSize
        , fMemoryManager
    );

    if (!fXCoder)
    {
        ThrowXMLwithMemMgr1
        (
            TranscodingException
            , XMLExcepts::Trans_CantCreateCvtrFor
            , outEncoding
            , fMemoryManager
        );
    }

    // Copy the encoding string
    fOutEncoding = XMLString::replicate(outEncoding, fMemoryManager);


    fIsXML11 = XMLString::equals(docVersion, XMLUni::fgVersion1_1);
}

XMLFormatter::XMLFormatter( const   char* const             outEncoding
                            ,       XMLFormatTarget* const  target
                            , const EscapeFlags             escapeFlags
                            , const UnRepFlags              unrepFlags
                            ,       MemoryManager* const    manager)
    : fEscapeFlags(escapeFlags)
    , fOutEncoding(0)
    , fTarget(target)
    , fUnRepFlags(unrepFlags)
    , fXCoder(0)
    , fAposRef(0)
    , fAposLen(0)
    , fAmpRef(0)
    , fAmpLen(0)
    , fGTRef(0)
    , fGTLen(0)
    , fLTRef(0)
    , fLTLen(0)
    , fQuoteRef(0)
    , fQuoteLen(0)
    , fIsXML11(false)
    , fMemoryManager(manager)
{
    // this constructor uses "1.0" for the docVersion

    // Transcode the encoding string
    fOutEncoding = XMLString::transcode(outEncoding, fMemoryManager);

    // Try to create a transcoder for this encoding
    XMLTransService::Codes resCode;
    fXCoder = XMLPlatformUtils::fgTransService->makeNewTranscoderFor
    (
        fOutEncoding
        , resCode
        , kTmpBufSize
        , fMemoryManager
    );

    if (!fXCoder)
    {
        fMemoryManager->deallocate(fOutEncoding); //delete [] fOutEncoding;
        ThrowXMLwithMemMgr1
        (
            TranscodingException
            , XMLExcepts::Trans_CantCreateCvtrFor
            , outEncoding
            , fMemoryManager
        );
    }

    //XMLCh* const tmpDocVer = XMLString::transcode("1.0", fMemoryManager);
    //ArrayJanitor<XMLCh> jname(tmpDocVer, fMemoryManager);
    //fIsXML11 = XMLString::equals(tmpDocVer, XMLUni::fgVersion1_1);
    fIsXML11 = false;  // docVersion 1.0 is not 1.1!
}


XMLFormatter::XMLFormatter( const   XMLCh* const            outEncoding
                            ,       XMLFormatTarget* const  target
                            , const EscapeFlags             escapeFlags
                            , const UnRepFlags              unrepFlags
                            ,       MemoryManager* const    manager)
    : fEscapeFlags(escapeFlags)
    , fOutEncoding(0)
    , fTarget(target)
    , fUnRepFlags(unrepFlags)
    , fXCoder(0)
    , fAposRef(0)
    , fAposLen(0)
    , fAmpRef(0)
    , fAmpLen(0)
    , fGTRef(0)
    , fGTLen(0)
    , fLTRef(0)
    , fLTLen(0)
    , fQuoteRef(0)
    , fQuoteLen(0)
    , fIsXML11(false)
    , fMemoryManager(manager)
{
    // this constructor uses XMLUni::fgVersion1_0 for the docVersion

    // Try to create a transcoder for this encoding
    XMLTransService::Codes resCode;
    fXCoder = XMLPlatformUtils::fgTransService->makeNewTranscoderFor
    (
        outEncoding
        , resCode
        , kTmpBufSize
        , fMemoryManager
    );

    if (!fXCoder)
    {
        ThrowXMLwithMemMgr1
        (
            TranscodingException
            , XMLExcepts::Trans_CantCreateCvtrFor
            , outEncoding
            , fMemoryManager
        );
    }

    // Copy the encoding string
    fOutEncoding = XMLString::replicate(outEncoding, fMemoryManager);

    //fIsXML11 = XMLString::equals(docVersion, XMLUni::fgVersion1_1);
    fIsXML11 = false;  // docVersion 1.0 is not 1.1!
}

XMLFormatter::~XMLFormatter()
{
    fMemoryManager->deallocate(fAposRef); //delete [] fAposRef;
    fMemoryManager->deallocate(fAmpRef); //delete [] fAmpRef;
    fMemoryManager->deallocate(fGTRef); //delete [] fGTRef;
    fMemoryManager->deallocate(fLTRef); //delete [] fLTRef;
    fMemoryManager->deallocate(fQuoteRef); //delete [] fQuoteRef;
    fMemoryManager->deallocate(fOutEncoding); //delete [] fOutEncoding;
    delete fXCoder;

    // We DO NOT own the target object!
}


// ---------------------------------------------------------------------------
//  XMLFormatter: Formatting methods
// ---------------------------------------------------------------------------
void
XMLFormatter::formatBuf(const   XMLCh* const    toFormat
                        , const XMLSize_t       count
                        , const EscapeFlags     escapeFlags
                        , const UnRepFlags      unrepFlags)
{
    //
    //  Figure out the actual escape flag value. If the parameter is not
    //  the default, then take it. Else take the current default.
    //
    const EscapeFlags actualEsc = (escapeFlags == DefaultEscape)
                                ? fEscapeFlags : escapeFlags;

    // And do the same for the unrep flags
    const UnRepFlags  actualUnRep = (unrepFlags == DefaultUnRep)
                                    ? fUnRepFlags : unrepFlags;

    //
    //  If the actual unrep action is that they want to provide char refs
    //  for unrepresentable chars, then this one is a much more difficult
    //  one to do cleanly, and we handle it separately.
    //
    if (actualUnRep == UnRep_CharRef)
    {
        specialFormat(toFormat, count, actualEsc);
        return;
    }

    //
    //  If we don't have any escape flags set, then we can do the most
    //  efficient loop, else we have to do it the hard way.
    //
    const XMLCh*    srcPtr = toFormat;
    const XMLCh*    endPtr = toFormat + count;
    if (actualEsc == NoEscapes)
    {
        //
        //  Just do a whole buffer at a time into the temp buffer, cap
        //  it off, and send it to the target.
        //
        if (srcPtr < endPtr)
           srcPtr += handleUnEscapedChars(srcPtr, endPtr - srcPtr, actualUnRep);
    }
     else
    {
        //
        //  Escape chars that require it according tot he scale flags
        //  we were given. For the others, try to accumulate them and
        //  format them in as big as bulk as we can.
        //
        while (srcPtr < endPtr)
        {
            //
            //  Run a temp pointer up until we hit a character that we have
            //  to escape. Then we can convert all the chars between our
            //  current source pointer and here all at once.
            //
            const XMLCh* tmpPtr = srcPtr;
            while ((tmpPtr < endPtr) && !inEscapeList(actualEsc, *tmpPtr))
                tmpPtr++;

            //
            //  If we got any chars, then lets convert them and write them
            //  out.
            //
            if (tmpPtr > srcPtr)
               srcPtr += handleUnEscapedChars(srcPtr, tmpPtr - srcPtr,
                                              actualUnRep);

             else if (tmpPtr < endPtr)
            {
                //
                //  Ok, so we've hit a char that must be escaped. So do
                //  this one specially.
                //
                const XMLByte * theChars;
                switch (*srcPtr) {
                    case chAmpersand :
                        theChars = getCharRef(fAmpLen, fAmpRef, gAmpRef);
                        fTarget->writeChars(theChars, fAmpLen, this);
                        break;

                    case chSingleQuote :
                        theChars = getCharRef(fAposLen, fAposRef, gAposRef);
                        fTarget->writeChars(theChars, fAposLen, this);
                        break;

                    case chDoubleQuote :
                        theChars = getCharRef(fQuoteLen, fQuoteRef, gQuoteRef);
                        fTarget->writeChars(theChars, fQuoteLen, this);
                        break;

                    case chCloseAngle :
                        theChars = getCharRef(fGTLen, fGTRef, gGTRef);
                        fTarget->writeChars(theChars, fGTLen, this);
                        break;

                    case chOpenAngle :
                        theChars = getCharRef(fLTLen, fLTRef, gLTRef);
                        fTarget->writeChars(theChars, fLTLen, this);
                        break;

                    default:
                        // control characters
                        writeCharRef(*srcPtr);
                        break;
                }
                srcPtr++;
            }
        }
    }
}


XMLSize_t
XMLFormatter::handleUnEscapedChars(const XMLCh *                  srcPtr,
                                   const XMLSize_t                oCount,
                                   const UnRepFlags               actualUnRep)
{
   //
   //  Use that to figure out what I should pass to the transcoder. If we
   //  are doing character references or failing for unrepresentable chars,
   //  then we just throw, since we should never get a call for something
   //  we cannot represent. Else, we tell it to just use the replacement
   //  char.
   //
   const XMLTranscoder::UnRepOpts unRepOpts = (actualUnRep == UnRep_Replace)
                                             ? XMLTranscoder::UnRep_RepChar
                                             : XMLTranscoder::UnRep_Throw;

   XMLSize_t charsEaten;
   XMLSize_t count = oCount;

   while (count) {
     const XMLSize_t srcChars = (count > XMLSize_t (kTmpBufSize))
       ? XMLSize_t (kTmpBufSize) : count;

      const XMLSize_t outBytes
         = fXCoder->transcodeTo(srcPtr, srcChars,
                                fTmpBuf, kTmpBufSize,
                                charsEaten, unRepOpts);

      if (outBytes) {
         fTmpBuf[outBytes]     = 0; fTmpBuf[outBytes + 1] = 0;
         fTmpBuf[outBytes + 2] = 0; fTmpBuf[outBytes + 3] = 0;
         fTarget->writeChars(fTmpBuf, outBytes, this);
      }

      srcPtr += charsEaten;
      count  -= charsEaten;
   }

   return oCount; // This should be an assertion that count == 0.
}


XMLFormatter& XMLFormatter::operator<<(const XMLCh* const toFormat)
{
    const XMLSize_t len = XMLString::stringLen(toFormat);
    formatBuf(toFormat, len);
    return *this;
}

XMLFormatter& XMLFormatter::operator<<(const XMLCh toFormat)
{
    // Make a temp string format that
    XMLCh szTmp[2];
    szTmp[0] = toFormat;
    szTmp[1] = 0;

    formatBuf(szTmp, 1);
    return *this;
}

/**
 * the parameter, count, is needed since stringLen()
 * does not work on a BOM like "0xFE0xFF0x000x00" or
 * "0x000x000xFF0xFE"
 **/
void XMLFormatter::writeBOM(const XMLByte* const toFormat
                          , const XMLSize_t      count)
{
    fTarget->writeChars(toFormat, count, this);
}

// ---------------------------------------------------------------------------
//  XMLFormatter: Private helper methods
// ---------------------------------------------------------------------------
void XMLFormatter::writeCharRef(const XMLCh &toWrite)
{
    XMLCh tmpBuf[32];
    tmpBuf[0] = chAmpersand;
    tmpBuf[1] = chPound;
    tmpBuf[2] = chLatin_x;

    // Build a char ref for the current char
    XMLString::binToText(toWrite, &tmpBuf[3], 8, 16, fMemoryManager);
    const XMLSize_t bufLen = XMLString::stringLen(tmpBuf);
    tmpBuf[bufLen] = chSemiColon;
    tmpBuf[bufLen+1] = chNull;

    // write it out
    formatBuf(tmpBuf
            , bufLen + 1
            , XMLFormatter::NoEscapes
            , XMLFormatter::UnRep_Fail);

}

void XMLFormatter::writeCharRef(XMLSize_t toWrite)
{
    XMLCh tmpBuf[64];
    tmpBuf[0] = chAmpersand;
    tmpBuf[1] = chPound;
    tmpBuf[2] = chLatin_x;

    // Build a char ref for the current char
    XMLString::sizeToText(toWrite, &tmpBuf[3], 32, 16, fMemoryManager);
    const XMLSize_t bufLen = XMLString::stringLen(tmpBuf);
    tmpBuf[bufLen] = chSemiColon;
    tmpBuf[bufLen+1] = chNull;

    // write it out
    formatBuf(tmpBuf
            , bufLen + 1
            , XMLFormatter::NoEscapes
            , XMLFormatter::UnRep_Fail);

}


const XMLByte* XMLFormatter::getCharRef(XMLSize_t     &count,
                                        XMLByte*      &ref,
                                        const XMLCh *  stdRef)
{
   if (!ref) {

       XMLSize_t charsEaten;
       const XMLSize_t outBytes =
           fXCoder->transcodeTo(stdRef, XMLString::stringLen(stdRef),
                                fTmpBuf, kTmpBufSize, charsEaten,
                                XMLTranscoder::UnRep_Throw);

       fTmpBuf[outBytes] = 0;
       fTmpBuf[outBytes + 1] = 0;
       fTmpBuf[outBytes + 2] = 0;
       fTmpBuf[outBytes + 3] = 0;

       ref = (XMLByte*) fMemoryManager->allocate
       (
           (outBytes + 4) * sizeof(XMLByte)
       );//new XMLByte[outBytes + 4];
       memcpy(ref, fTmpBuf, outBytes + 4);
       count = outBytes;
   }

   return ref;
}

void XMLFormatter::specialFormat(const  XMLCh* const    toFormat
                                , const XMLSize_t       count
                                , const EscapeFlags     escapeFlags)
{
    //
    //  We have to check each character and see if it could be represented.
    //  As long as it can, we just keep up with where we started and how
    //  many chars we've checked. When we hit an unrepresentable one, we
    //  stop, transcode everything we've collected, then start handling
    //  the unrepresentables via char refs. We repeat this until we get all
    //  the chars done.
    //
    const XMLCh*    srcPtr = toFormat;
    const XMLCh*    endPtr = toFormat + count;

    while (srcPtr < endPtr)
    {
        const XMLCh* tmpPtr = srcPtr;
        while (tmpPtr < endPtr)
        {
            if (fXCoder->canTranscodeTo(*tmpPtr))
                tmpPtr++;
            else
                break;
        }

        if (tmpPtr > srcPtr)
        {
            // We got at least some chars that can be done normally
            formatBuf
            (
                srcPtr
                , tmpPtr - srcPtr
                , escapeFlags
                , XMLFormatter::UnRep_Fail
            );

            // Update the source pointer to our new spot
            srcPtr = tmpPtr;
        }
         else
        {

            //  We hit something unrepresentable. So continue forward doing
            //  char refs until we hit something representable again or the
            //  end of input.
            //
            while (srcPtr < endPtr)
            {
                if ((*srcPtr & 0xFC00) == 0xD800) {
                    // we have encountered a surrogate, need to recombine before printing out
                    // use writeCharRef that takes XMLSize_t to get values larger than
                    // hex 0xFFFF printed.
                    tmpPtr = srcPtr;
                    tmpPtr++; // point at low surrogate
                    writeCharRef((XMLSize_t) (0x10000+((*srcPtr-0xD800)<<10)+*tmpPtr-0xDC00));
                    srcPtr++; // advance to low surrogate (will advance again below)
                }
                else {
                    writeCharRef(*srcPtr);
                }

                // Move up the source pointer and break out if needed
                srcPtr++;
                if (fXCoder->canTranscodeTo(*srcPtr))
                    break;
            }
        }
    }
}

XERCES_CPP_NAMESPACE_END
