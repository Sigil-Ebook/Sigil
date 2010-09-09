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
 * $Id: XMLDateTime.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XML_DATETIME_HPP)
#define XERCESC_INCLUDE_GUARD_XML_DATETIME_HPP

#include <xercesc/util/XMLNumber.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/SchemaDateTimeException.hpp>
#include <xercesc/util/XMLChar.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XSValue;

class XMLUTIL_EXPORT XMLDateTime : public XMLNumber
{
public:

	enum valueIndex
    {
        CentYear   = 0,
        Month      ,
        Day        ,
        Hour       ,
        Minute     ,
        Second     ,
        MiliSecond ,  //not to be used directly
        utc        ,
        TOTAL_SIZE
    };

    enum utcType
    {
        UTC_UNKNOWN = 0,
        UTC_STD        ,          // set in parse() or normalize()
        UTC_POS        ,          // set in parse()
        UTC_NEG                   // set in parse()
    };

    // -----------------------------------------------------------------------
    // ctors and dtor
    // -----------------------------------------------------------------------

    XMLDateTime(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    XMLDateTime(const XMLCh* const,
                MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    ~XMLDateTime();

    inline void           setBuffer(const XMLCh* const);

    // -----------------------------------------------------------------------
    // Copy ctor and Assignment operators
    // -----------------------------------------------------------------------

    XMLDateTime(const XMLDateTime&);

    XMLDateTime&          operator=(const XMLDateTime&);

    // -----------------------------------------------------------------------
    // Implementation of Abstract Interface
    // -----------------------------------------------------------------------

    virtual XMLCh*        getRawData() const;

    virtual const XMLCh*  getFormattedString() const;

    virtual int           getSign() const;

    // -----------------------------------------------------------------------
    // Canonical Representation
    // -----------------------------------------------------------------------

    XMLCh*                getDateTimeCanonicalRepresentation(MemoryManager* const memMgr) const;

    XMLCh*                getTimeCanonicalRepresentation(MemoryManager* const memMgr)     const;

    XMLCh*                getDateCanonicalRepresentation(MemoryManager* const memMgr)     const;

    // -----------------------------------------------------------------------
    // parsers
    // -----------------------------------------------------------------------

    void                  parseDateTime();       //DateTime

    void                  parseDate();           //Date

    void                  parseTime();           //Time

    void                  parseDay();            //gDay

    void                  parseMonth();          //gMonth

    void                  parseYear();           //gYear

    void                  parseMonthDay();       //gMonthDay

    void                  parseYearMonth();      //gYearMonth

    void                  parseDuration();       //duration

    // -----------------------------------------------------------------------
    // Comparison
    // -----------------------------------------------------------------------
    static int            compare(const XMLDateTime* const
                                , const XMLDateTime* const);

    static int            compare(const XMLDateTime* const
                                , const XMLDateTime* const
                                , bool                    );

    static int            compareOrder(const XMLDateTime* const
                                     , const XMLDateTime* const);

    /***
     * Support for Serialization/De-serialization
     ***/
    DECL_XSERIALIZABLE(XMLDateTime)

private:

    // -----------------------------------------------------------------------
    // Constant data
    // -----------------------------------------------------------------------
	//

    enum timezoneIndex
    {
        hh = 0,
        mm ,
        TIMEZONE_ARRAYSIZE
    };

    // -----------------------------------------------------------------------
    // Comparison
    // -----------------------------------------------------------------------
    static int            compareResult(int
                                      , int
                                      , bool);

    static void           addDuration(XMLDateTime*             pDuration
                                    , const XMLDateTime* const pBaseDate
                                    , int                      index);


    static int            compareResult(const XMLDateTime* const
                                      , const XMLDateTime* const
                                      , bool
                                      , int);

    static inline int     getRetVal(int, int);

    // -----------------------------------------------------------------------
    // helper
    // -----------------------------------------------------------------------

    inline  void          reset();

    inline  void          assertBuffer()               const;

    inline  void          copy(const XMLDateTime&);

    // allow multiple parsing
    inline  bool          initParser();

    inline  bool          isNormalized()               const;

    // -----------------------------------------------------------------------
    // scaners
    // -----------------------------------------------------------------------

    void                  getDate();

    void                  getTime();

    void                  getYearMonth();

    void                  getTimeZone(const XMLSize_t);

    void                  parseTimeZone();

    // -----------------------------------------------------------------------
    // locator and converter
    // -----------------------------------------------------------------------

    int                   findUTCSign(const XMLSize_t start);

    int                   indexOf(const XMLSize_t start
                                , const XMLSize_t end
                                , const XMLCh ch)     const;

    int                   parseInt(const XMLSize_t start
                                 , const XMLSize_t end)     const;

    int                   parseIntYear(const XMLSize_t end) const;

    double                parseMiliSecond(const XMLSize_t start
                                        , const XMLSize_t end) const;

    // -----------------------------------------------------------------------
    // validator and normalizer
    // -----------------------------------------------------------------------

    void                  validateDateTime()          const;

    void                  normalize();

    void                  fillString(XMLCh*& ptr, int value, XMLSize_t expLen) const;

    int                   fillYearString(XMLCh*& ptr, int value) const;

    void                  searchMiliSeconds(XMLCh*& miliStartPtr, XMLCh*& miliEndPtr) const;

    // -----------------------------------------------------------------------
    // Unimplemented operator ==
    // -----------------------------------------------------------------------
	bool operator==(const XMLDateTime& toCompare) const;


    // -----------------------------------------------------------------------
    //  Private data members
    //
    //     fValue[]
    //          object representation of date time.
    //
    //     fTimeZone[]
    //          temporary storage for normalization
    //
    //     fStart, fEnd
    //          pointers to the portion of fBuffer being parsed
    //
    //     fBuffer
    //          raw data to be parsed, own it.
    //
    // -----------------------------------------------------------------------

    int          fValue[TOTAL_SIZE];
    int          fTimeZone[TIMEZONE_ARRAYSIZE];
    XMLSize_t    fStart;
    XMLSize_t    fEnd;
    XMLSize_t    fBufferMaxLen;

    double       fMilliSecond;
    bool         fHasTime;

    XMLCh*       fBuffer;
    MemoryManager* fMemoryManager;

    friend class XSValue;
};

inline void XMLDateTime::setBuffer(const XMLCh* const aString)
{
    reset();

    fEnd = XMLString::stringLen(aString);

    for (; fEnd > 0; fEnd--)
    {
        if (!XMLChar1_0::isWhitespace(aString[fEnd - 1]))
            break;
    }

    if (fEnd > 0) {

        if (fEnd > fBufferMaxLen)
        {
            fMemoryManager->deallocate(fBuffer);
            fBufferMaxLen = fEnd + 8;
            fBuffer = (XMLCh*) fMemoryManager->allocate((fBufferMaxLen+1) * sizeof(XMLCh));
        }

        memcpy(fBuffer, aString, (fEnd) * sizeof(XMLCh));
        fBuffer[fEnd] = '\0';
    }
}

inline void XMLDateTime::reset()
{
    for ( int i=0; i < TOTAL_SIZE; i++ )
        fValue[i] = 0;

    fMilliSecond   = 0;
    fHasTime      = false;
    fTimeZone[hh] = fTimeZone[mm] = 0;
    fStart = fEnd = 0;

    if (fBuffer)
        *fBuffer = 0;
}

inline void XMLDateTime::copy(const XMLDateTime& rhs)
{
    for ( int i = 0; i < TOTAL_SIZE; i++ )
        fValue[i] = rhs.fValue[i];

    fMilliSecond   = rhs.fMilliSecond;
    fHasTime      = rhs.fHasTime;
    fTimeZone[hh] = rhs.fTimeZone[hh];
    fTimeZone[mm] = rhs.fTimeZone[mm];
    fStart = rhs.fStart;
    fEnd   = rhs.fEnd;

    if (fEnd > 0)
    {
        if (fEnd > fBufferMaxLen)
        {
            fMemoryManager->deallocate(fBuffer);//delete[] fBuffer;
            fBufferMaxLen = rhs.fBufferMaxLen;
            fBuffer = (XMLCh*) fMemoryManager->allocate((fBufferMaxLen+1) * sizeof(XMLCh));
        }

        memcpy(fBuffer, rhs.fBuffer, (fEnd+1) * sizeof(XMLCh));
    }
}

inline bool XMLDateTime::initParser()
{
    if (!fBuffer || fBuffer[0] == chNull)
        return false;

    fStart = 0;   // to ensure scan from the very first beginning
                  // in case the pointer is updated accidentally by
                  // someone else.
    return true;
}

inline bool XMLDateTime::isNormalized() const
{
    return ( fValue[utc] == UTC_STD ? true : false );
}

inline int XMLDateTime::getRetVal(int c1, int c2)
{
    if ((c1 == LESS_THAN    && c2 == GREATER_THAN) ||
        (c1 == GREATER_THAN && c2 == LESS_THAN)      )
    {
        return INDETERMINATE;
    }

    return ( c1 != INDETERMINATE ) ? c1 : c2;
}

XERCES_CPP_NAMESPACE_END

#endif
