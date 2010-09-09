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
 * $Id: XMLReader.hpp 833045 2009-11-05 13:21:27Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLREADER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLREADER_HPP

#include <xercesc/util/XMLChar.hpp>
#include <xercesc/framework/XMLRecognizer.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/util/TranscodingException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class InputSource;
class BinInputStream;
class ReaderMgr;
class XMLScanner;
class XMLTranscoder;


// ---------------------------------------------------------------------------
//  Instances of this class are used to manage the content of entities. The
//  scanner maintains a stack of these, one for each entity (this means entity
//  in the sense of any parsed file or internal entity) currently being
//  scanned. This class, given a binary input stream will handle reading in
//  the data and decoding it from its external decoding into the internal
//  Unicode format. Once internallized, this class provides the access
//  methods to read in the data in various ways, maintains line and column
//  information, and provides high performance character attribute checking
//  methods.
//
//  This is NOT to be derived from.
//
// ---------------------------------------------------------------------------
class XMLPARSER_EXPORT XMLReader : public XMemory
{
public:
    // -----------------------------------------------------------------------
    //  Public types
    // -----------------------------------------------------------------------
    enum Types
    {
        Type_PE
        , Type_General
    };

    enum Sources
    {
        Source_Internal
        , Source_External
    };

    enum RefFrom
    {
        RefFrom_Literal
        , RefFrom_NonLiteral
    };

    enum XMLVersion
    {
        XMLV1_0
        , XMLV1_1
        , XMLV_Unknown
    };


    // -----------------------------------------------------------------------
    //  Public, query methods
    // -----------------------------------------------------------------------
    bool isAllSpaces
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    ) const;

    bool containsWhiteSpace
    (
        const   XMLCh* const    toCheck
        , const XMLSize_t       count
    ) const;


    bool isXMLLetter(const XMLCh toCheck) const;
    bool isFirstNameChar(const XMLCh toCheck) const;
    bool isNameChar(const XMLCh toCheck) const;
    bool isPlainContentChar(const XMLCh toCheck) const;
    bool isSpecialStartTagChar(const XMLCh toCheck) const;
    bool isXMLChar(const XMLCh toCheck) const;
    bool isWhitespace(const XMLCh toCheck) const;
    bool isControlChar(const XMLCh toCheck) const;
    bool isPublicIdChar(const XMLCh toCheck) const;
    bool isFirstNCNameChar(const XMLCh toCheck) const;
    bool isNCNameChar(const XMLCh toCheck) const;

    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    XMLReader
    (
        const   XMLCh* const          pubId
        , const XMLCh* const          sysId
        ,       BinInputStream* const streamToAdopt
        , const RefFrom               from
        , const Types                 type
        , const Sources               source
        , const bool                  throwAtEnd = false
        , const bool                  calculateSrcOfs = true
        ,       XMLSize_t             lowWaterMark = 100
        , const XMLVersion            xmlVersion = XMLV1_0
        ,       MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    XMLReader
    (
        const   XMLCh* const          pubId
        , const XMLCh* const          sysId
        ,       BinInputStream* const streamToAdopt
        , const XMLCh* const          encodingStr
        , const RefFrom               from
        , const Types                 type
        , const Sources               source
        , const bool                  throwAtEnd = false
        , const bool                  calculateSrcOfs = true
        ,       XMLSize_t             lowWaterMark = 100
        , const XMLVersion            xmlVersion = XMLV1_0
        ,       MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    XMLReader
    (
        const   XMLCh* const          pubId
        , const XMLCh* const          sysId
        ,       BinInputStream* const streamToAdopt
        , XMLRecognizer::Encodings    encodingEnum
        , const RefFrom               from
        , const Types                 type
        , const Sources               source
        , const bool                  throwAtEnd = false
        , const bool                  calculateSrcOfs = true
        ,       XMLSize_t             lowWaterMark = 100
        , const XMLVersion            xmlVersion = XMLV1_0
        ,       MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    ~XMLReader();


    // -----------------------------------------------------------------------
    //  Character buffer management methods
    // -----------------------------------------------------------------------
    XMLSize_t charsLeftInBuffer() const;
    bool refreshCharBuffer();


    // -----------------------------------------------------------------------
    //  Scanning methods
    // -----------------------------------------------------------------------
    bool getName(XMLBuffer& toFill, const bool token);
    bool getQName(XMLBuffer& toFill, int* colonPosition);
    bool getNCName(XMLBuffer& toFill);
    bool getNextChar(XMLCh& chGotten);
    bool getNextCharIfNot(const XMLCh chNotToGet, XMLCh& chGotten);
    void movePlainContentChars(XMLBuffer &dest);
    bool getSpaces(XMLBuffer& toFill);
    bool getUpToCharOrWS(XMLBuffer& toFill, const XMLCh toCheck);
    bool peekNextChar(XMLCh& chGotten);
    bool skipIfQuote(XMLCh& chGotten);
    bool skipSpaces(bool& skippedSomething, bool inDecl = false);
    bool skippedChar(const XMLCh toSkip);
    bool skippedSpace();
    bool skippedString(const XMLCh* const toSkip);
    bool skippedStringLong(const XMLCh* toSkip);
    bool peekString(const XMLCh* const toPeek);


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    XMLFileLoc getColumnNumber() const;
    const XMLCh* getEncodingStr() const;
    XMLFileLoc getLineNumber() const;
    bool getNoMoreFlag() const;
    const XMLCh* getPublicId() const;
    XMLSize_t getReaderNum() const;
    RefFrom getRefFrom() const;
    Sources getSource() const;
    XMLFilePos getSrcOffset() const;
    const XMLCh* getSystemId() const;
    bool getThrowAtEnd() const;
    Types getType() const;


    // -----------------------------------------------------------------------
    //  Setter methods
    // -----------------------------------------------------------------------
    bool setEncoding
    (
        const   XMLCh* const    newEncoding
    );
    void setReaderNum(const XMLSize_t newNum);
    void setThrowAtEnd(const bool newValue);
    void setXMLVersion(const XMLVersion version);


private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLReader(const XMLReader&);
    XMLReader& operator=(const XMLReader&);

    // ---------------------------------------------------------------------------
    //  Class Constants
    //
    //  kCharBufSize
    //      The size of the character spool buffer that we use. Its not terribly
    //      large because its just getting filled with data from a raw byte
    //      buffer as we go along. We don't want to decode all the text at
    //      once before we find out that there is an error.
    //
    //      NOTE: This is a size in characters, not bytes.
    //
    //  kRawBufSize
    //      The size of the raw buffer from which raw bytes are spooled out
    //      as we transcode chunks of data. As it is emptied, it is filled back
    //      in again from the source stream.
    // ---------------------------------------------------------------------------
    enum Constants
    {
        kCharBufSize        = 16 * 1024
        , kRawBufSize       = 48 * 1024
    };


    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void checkForSwapped();

    void doInitCharSizeChecks();

    void doInitDecode();

    XMLByte getNextRawByte
    (
        const   bool            eoiOk
    );

    void refreshRawBuffer();

    void setTranscoder
    (
        const   XMLCh* const    newEncoding
    );

    XMLSize_t xcodeMoreChars
    (
                XMLCh* const            bufToFill
        ,       unsigned char* const    charSizes
        , const XMLSize_t               maxChars
    );

    void handleEOL
    (
              XMLCh&   curCh
            , bool     inDecl = false
    );

    // -----------------------------------------------------------------------
    //  Data members
    //
    //  fCharIndex
    //      The index into the character buffer. When this hits fCharsAvail
    //      then its time to refill.
    //
    //  fCharBuf
    //      A buffer that the reader manager fills up with transcoded
    //      characters a small amount at a time.
    //
    //  fCharsAvail
    //      The characters currently available in the character buffer.
    //
    //  fCharSizeBuf
    //      This buffer is an array that contains the number of source chars
    //      eaten to create each char in the fCharBuf buffer. So the entry
    //      fCharSizeBuf[x] is the number of source chars that were eaten
    //      to make the internalized char fCharBuf[x]. This only contains
    //      useful data if fSrcOfsSupported is true.
    //
    //  fCharOfsBuf
    //      This buffer is an array that contains the offset in the
    //      fRawByteBuf buffer of each char in the fCharBuf buffer. It
    //      only contains useful data if fSrcOfsSupported is true.
    //
    //  fCurCol
    //  fCurLine
    //      The current line and column that we are in within this reader's
    //      text.
    //
    //  fEncoding
    //      This is the rough encoding setting. This enum is set during
    //      construction and just tells us the rough family of encoding that
    //      we are doing.
    //
    //  fEncodingStr
    //      This is the name of the encoding we are using. It will be
    //      provisionally set during construction, from the auto-sensed
    //      encoding. But it might be overridden when the XMLDecl is finally
    //      seen by the scanner. It can also be forced to a particular
    //      encoding, in which case fForcedEncoding is set.
    //
    //  fForcedEncoding
    //      If the encoding if forced then this is set and all other
    //      information will be ignored. This encoding will be taken as
    //      gospel. This is done by calling an alternate constructor.
    //
    //  fNoMore
    //      This is set when the source text is exhausted. It lets us know
    //      quickly that no more text is available.
    //
    //  fRawBufIndex
    //      The current index into the raw byte buffer. When its equal to
    //      fRawBytesAvail then we need to read another buffer.
    //
    //  fRawByteBuf
    //      This is the raw byte buffer that is used to spool out bytes
    //      from into the fCharBuf buffer, as we transcode in blocks.
    //
    //  fRawBytesAvail
    //      The number of bytes currently available in the raw buffer. This
    //      helps deal with the last buffer's worth, which will usually not
    //      be a full one.
    //
    //  fLowWaterMark
    //      The low water mark for the raw byte buffer.
    //
    //
    //  fReaderNum
    //      Each reader from a particular reader manager (which means from a
    //      particular document) is given a unique number. The reader manager
    //      sets these numbers. They are used to catch things like partial
    //      markup errors.
    //
    //  fRefFrom
    //      This flag is provided in the ctor, and tells us if we represent
    //      some entity being expanded inside a literal. Sometimes things
    //      happen differently inside and outside literals.
    //
    //  fPublicId
    //  fSystemId
    //      These are the system and public ids of the source that this
    //      reader is reading.
    //
    //  fSentTrailingSpace
    //      If we are a PE entity being read and we not referenced from a
    //      literal, then a leading and trailing space must be faked into the
    //      data. This lets us know we've done the trailing space already (so
    //      we don't just keep doing it again and again.)
    //
    //  fSource
    //      Indicates whether the content this reader is spooling as already
    //      been internalized. This will prevent multiple processing of
    //      whitespace when an already internalized entity is being spooled
    //      out.
    //
    //  fSpareChar
    //      Some encodings can create two chars in an atomic way, e.g.
    //      surrogate pairs. We might not be able to store both, so we store
    //      it here until the next buffer transcoding operation.
    //
    //  fSrcOfsBase
    //      This is the base offset within the source of this entity. Values
    //      in the curent fCharSizeBuf array are relative to this value.
    //
    //  fSrcOfsSupported
    //      This flag is set to indicate whether source byte offset info
    //      is supported. For intrinsic encodings, its always set since we
    //      can always support it. For transcoder based encodings, we ask
    //      the transcoder if it supports it or not.
    //
    //  fStream
    //      This is the input stream that provides the data for the reader.
    //      Its always treated as a raw byte stream. The derived class will
    //      ask for buffers of text from it and will handle making some
    //      sense of it.
    //
    //  fSwapped
    //      If the encoding is one of the ones we do intrinsically, and its
    //      in a different byte order from our native order, then this is
    //      set to remind us to byte swap it during transcoding.
    //
    //  fThrowAtEnd
    //      Indicates whether the reader manager should throw an end of entity
    //      exception at the end of this reader instance. This is usually
    //      set for top level external entity references. It overrides the
    //      reader manager's global flag that controls throwing at the end
    //      of entities. Defaults to false.
    //
    //  fTranscoder
    //      If the encoding is not one that we handle intrinsically, then
    //      we use an an external transcoder to do it. This class is an
    //      abstraction that allows us to use pluggable external transcoding
    //      services (via XMLTransService in util.)
    //
    //  fType
    //      Indicates whether this reader represents a PE or not. If this
    //      flag is true and the fInLiteral flag is false, then we will put
    //      out an extra space at the end.
    //
    //  fgCharCharsTable;
    //      Pointer to XMLChar table, depends on XML version
    //
    //  fNEL
    //      Boolean indicates if NEL and LSEP should be recognized as NEL
    //
    //  fXMLVersion
    //      Enum to indicate if this Reader is conforming to XML 1.0 or XML 1.1
    // -----------------------------------------------------------------------
    XMLSize_t                   fCharIndex;
    XMLCh                       fCharBuf[kCharBufSize];
    XMLSize_t                   fCharsAvail;
    unsigned char               fCharSizeBuf[kCharBufSize];
    unsigned int                fCharOfsBuf[kCharBufSize];
    XMLFileLoc                  fCurCol;
    XMLFileLoc                  fCurLine;
    XMLRecognizer::Encodings    fEncoding;
    XMLCh*                      fEncodingStr;
    bool                        fForcedEncoding;
    bool                        fNoMore;
    XMLCh*                      fPublicId;
    XMLSize_t                   fRawBufIndex;
    XMLByte                     fRawByteBuf[kRawBufSize];
    XMLSize_t                   fRawBytesAvail;
    XMLSize_t                   fLowWaterMark;
    XMLSize_t                   fReaderNum;
    RefFrom                     fRefFrom;
    bool                        fSentTrailingSpace;
    Sources                     fSource;
    XMLFilePos                  fSrcOfsBase;
    bool                        fSrcOfsSupported;
    bool                        fCalculateSrcOfs;
    XMLCh*                      fSystemId;
    BinInputStream*             fStream;
    bool                        fSwapped;
    bool                        fThrowAtEnd;
    XMLTranscoder*              fTranscoder;
    Types                       fType;
    XMLByte*                    fgCharCharsTable;
    bool                        fNEL;
    XMLVersion                  fXMLVersion;
    MemoryManager*              fMemoryManager;
};


// ---------------------------------------------------------------------------
//  XMLReader: Public, query methods
// ---------------------------------------------------------------------------
inline bool XMLReader::isNameChar(const XMLCh toCheck) const
{
    return ((fgCharCharsTable[toCheck] & gNameCharMask) != 0);
}

inline bool XMLReader::isNCNameChar(const XMLCh toCheck) const
{
    return ((fgCharCharsTable[toCheck] & gNCNameCharMask) != 0);
}

inline bool XMLReader::isPlainContentChar(const XMLCh toCheck) const
{
    return ((fgCharCharsTable[toCheck] & gPlainContentCharMask) != 0);
}


inline bool XMLReader::isFirstNameChar(const XMLCh toCheck) const
{
    return ((fgCharCharsTable[toCheck] & gFirstNameCharMask) != 0);
}

inline bool XMLReader::isFirstNCNameChar(const XMLCh toCheck) const
{
    return (((fgCharCharsTable[toCheck] & gFirstNameCharMask) != 0)
            && (toCheck != chColon));
}

inline bool XMLReader::isSpecialStartTagChar(const XMLCh toCheck) const
{
    return ((fgCharCharsTable[toCheck] & gSpecialStartTagCharMask) != 0);
}

inline bool XMLReader::isXMLChar(const XMLCh toCheck) const
{
    return ((fgCharCharsTable[toCheck] & gXMLCharMask) != 0);
}

inline bool XMLReader::isXMLLetter(const XMLCh toCheck) const
{
    return (((fgCharCharsTable[toCheck] & gFirstNameCharMask) != 0)
            && (toCheck != chColon) && (toCheck != chUnderscore));
}

inline bool XMLReader::isWhitespace(const XMLCh toCheck) const
{
    return ((fgCharCharsTable[toCheck] & gWhitespaceCharMask) != 0);
}

inline bool XMLReader::isControlChar(const XMLCh toCheck) const
{
    return ((fgCharCharsTable[toCheck] & gControlCharMask) != 0);
}

// ---------------------------------------------------------------------------
//  XMLReader: Buffer management methods
// ---------------------------------------------------------------------------
inline XMLSize_t XMLReader::charsLeftInBuffer() const
{
    return fCharsAvail - fCharIndex;
}


// ---------------------------------------------------------------------------
//  XMLReader: Getter methods
// ---------------------------------------------------------------------------
inline XMLFileLoc XMLReader::getColumnNumber() const
{
    return fCurCol;
}

inline const XMLCh* XMLReader::getEncodingStr() const
{
    return fEncodingStr;
}

inline XMLFileLoc XMLReader::getLineNumber() const
{
    return fCurLine;
}

inline bool XMLReader::getNoMoreFlag() const
{
    return fNoMore;
}

inline const XMLCh* XMLReader::getPublicId() const
{
    return fPublicId;
}

inline XMLSize_t XMLReader::getReaderNum() const
{
    return fReaderNum;
}

inline XMLReader::RefFrom XMLReader::getRefFrom() const
{
    return fRefFrom;
}

inline XMLReader::Sources XMLReader::getSource() const
{
    return fSource;
}

inline const XMLCh* XMLReader::getSystemId() const
{
    return fSystemId;
}

inline bool XMLReader::getThrowAtEnd() const
{
    return fThrowAtEnd;
}

inline XMLReader::Types XMLReader::getType() const
{
    return fType;
}

// ---------------------------------------------------------------------------
//  XMLReader: Setter methods
// ---------------------------------------------------------------------------
inline void XMLReader::setReaderNum(const XMLSize_t newNum)
{
    fReaderNum = newNum;
}

inline void XMLReader::setThrowAtEnd(const bool newValue)
{
    fThrowAtEnd = newValue;
}

inline void XMLReader::setXMLVersion(const XMLVersion version)
{
    fXMLVersion = version;
    if (version == XMLV1_1) {
        fNEL = true;
        fgCharCharsTable = XMLChar1_1::fgCharCharsTable1_1;
    }
    else {
        fNEL = XMLChar1_0::enableNEL;
        fgCharCharsTable = XMLChar1_0::fgCharCharsTable1_0;
    }

}



// ---------------------------------------------------------------------------
//
//  XMLReader: movePlainContentChars()
//
//       Move as many plain (no special handling of any sort required) content
//       characters as possible from this reader to the supplied destination buffer.
//
//       This is THE hottest performance spot in the parser.
//
// ---------------------------------------------------------------------------
inline void XMLReader::movePlainContentChars(XMLBuffer &dest)
{
    const XMLSize_t chunkSize = fCharsAvail - fCharIndex;
    const XMLCh* cursor = &fCharBuf[fCharIndex];
    XMLSize_t count=0;
    for(;count<chunkSize && (fgCharCharsTable[*cursor++] & gPlainContentCharMask) != 0;++count) /*noop*/ ;

    if (count!=0)
    {
        dest.append(&fCharBuf[fCharIndex], count);
        fCharIndex += count;
        fCurCol    += (XMLFileLoc)count;
    }
}


// ---------------------------------------------------------------------------
//  XMLReader: getNextCharIfNot() method inlined for speed
// ---------------------------------------------------------------------------
inline bool XMLReader::getNextCharIfNot(const XMLCh chNotToGet, XMLCh& chGotten)
{
    //
    //  See if there is at least a char in the buffer. Else, do the buffer
    //  reload logic.
    //
    if (fCharIndex >= fCharsAvail)
    {
        // If fNoMore is set, then we have nothing else to give
        if (fNoMore)
            return false;

        // Try to refresh
        if (!refreshCharBuffer())
            return false;
    }

    // Check the next char
    if (fCharBuf[fCharIndex] == chNotToGet)
        return false;

    // Its not the one we want to skip so bump the index
    chGotten = fCharBuf[fCharIndex++];

    // Handle end of line normalization and line/col member maintenance.
    //
    // we can have end-of-line combinations with a leading
    // chCR(xD), chLF(xA), chNEL(x85), or chLineSeparator(x2028)
    //
    // 0000000000001101 chCR
    // 0000000000001010 chLF
    // 0000000010000101 chNEL
    // 0010000000101000 chLineSeparator
    // -----------------------
    // 1101111101010000 == ~(chCR|chLF|chNEL|chLineSeparator)
    //
    // if the result of the logical-& operation is
    // true  : 'curCh' can not be chCR, chLF, chNEL or chLineSeparator
    // false : 'curCh' can be chCR, chLF, chNEL or chLineSeparator
    //
    if ( chGotten & (XMLCh) ~(chCR|chLF|chNEL|chLineSeparator) )
    {
        fCurCol++;
    } else
    {
        handleEOL(chGotten, false);
    }

    return true;
}

// ---------------------------------------------------------------------------
//  XMLReader: getNextChar() method inlined for speed
// ---------------------------------------------------------------------------
inline bool XMLReader::getNextChar(XMLCh& chGotten)
{
    //
    //  See if there is at least a char in the buffer. Else, do the buffer
    //  reload logic.
    //
    if (fCharIndex >= fCharsAvail)
    {
        // If fNoMore is set, then we have nothing else to give
        if (fNoMore)
            return false;

        // Try to refresh
        if (!refreshCharBuffer())
            return false;
    }

    chGotten = fCharBuf[fCharIndex++];

    // Handle end of line normalization and line/col member maintenance.
    //
    // we can have end-of-line combinations with a leading
    // chCR(xD), chLF(xA), chNEL(x85), or chLineSeparator(x2028)
    //
    // 0000000000001101 chCR
    // 0000000000001010 chLF
    // 0000000010000101 chNEL
    // 0010000000101000 chLineSeparator
    // -----------------------
    // 1101111101010000 == ~(chCR|chLF|chNEL|chLineSeparator)
    //
    // if the result of the logical-& operation is
    // true  : 'curCh' can not be chCR, chLF, chNEL or chLineSeparator
    // false : 'curCh' can be chCR, chLF, chNEL or chLineSeparator
    //
    if ( chGotten & (XMLCh) ~(chCR|chLF|chNEL|chLineSeparator) )
    {
        fCurCol++;
    } else
    {
        handleEOL(chGotten, false);
    }

    return true;
}


// ---------------------------------------------------------------------------
//  XMLReader: peekNextChar() method inlined for speed
// ---------------------------------------------------------------------------
inline bool XMLReader::peekNextChar(XMLCh& chGotten)
{
    //
    //  If there is something still in the buffer, get it. Else do the reload
    //  scenario.
    //
    if (fCharIndex >= fCharsAvail)
    {
        // Try to refresh the buffer
        if (!refreshCharBuffer())
        {
            chGotten = chNull;
            return false;
        }
    }

    chGotten = fCharBuf[fCharIndex];

    //
    //  Even though we are only peeking, we have to act the same as the
    //  normal char get method in regards to newline normalization, though
    //  its not as complicated as the actual character getting method's.
    //
    if ((chGotten == chCR || (fNEL && (chGotten == chNEL || chGotten == chLineSeparator)))
        && (fSource == Source_External))
        chGotten = chLF;

    return true;
}

XERCES_CPP_NAMESPACE_END

#endif
