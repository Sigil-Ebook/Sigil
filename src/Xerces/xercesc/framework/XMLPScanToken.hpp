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
 * $Id: XMLPScanToken.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLPSCANTOKEN_HPP)
#define XERCESC_INCLUDE_GUARD_XMLPSCANTOKEN_HPP

#include <xercesc/util/XMemory.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLScanner;

/**
 *  This simple class is used as a sanity check when the scanner is used to
 *  do progressive parsing. It insures that things are not done out of
 *  sequence and that sequences of scan calls are made correctly to the
 *  right scanner instances.
 *
 *  To client code, it is just a magic cookie which is obtained when a
 *  progressive parse is begun, and which is passed back in on each subsequent
 *  call of the progressive parse.
 */
class XMLPARSER_EXPORT XMLPScanToken : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    /** @name Constructor */
    //@{
    XMLPScanToken();
    XMLPScanToken(const XMLPScanToken& toCopy);
    //@}

    /** @name Destructor */
    //@{
    ~XMLPScanToken();
    //@}


    // -----------------------------------------------------------------------
    //  Public operators
    // -----------------------------------------------------------------------
    XMLPScanToken& operator=(const XMLPScanToken& toCopy);


protected :
    // -----------------------------------------------------------------------
    //  XMLScanner is our friend, can you say friend? Sure...
    // -----------------------------------------------------------------------
    friend class XMLScanner;


    // -----------------------------------------------------------------------
    //  Hidden methods for use by XMLScanner
    // -----------------------------------------------------------------------
    void set
    (
        const   XMLUInt32   scannerId
        , const XMLUInt32   sequenceId
    );


private :
    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fScannerId
    //      This field is set to the id of the scanner, to catch problems
    //      where a token is gotten from one scanner and passed to another.
    //      Each scanner is assigned an incrementing id.
    //
    //  fSequenceId
    //      In order to avoid problems such as calling scanNext() without
    //      a call to scanFirst() and such, this value is set when scanFirst()
    //      is called and matches this token to the current sequence id of
    //      the scanner.
    // -----------------------------------------------------------------------
    XMLUInt32   fScannerId;
    XMLUInt32   fSequenceId;
};


// ---------------------------------------------------------------------------
//  XMLPScanToken: Constructors and Operators
// ---------------------------------------------------------------------------
inline XMLPScanToken::XMLPScanToken() :

    fScannerId(0)
    , fSequenceId(0)
{
}

inline XMLPScanToken::XMLPScanToken(const XMLPScanToken& toCopy) :
    XMemory(toCopy)
    , fScannerId(toCopy.fScannerId)
    , fSequenceId(toCopy.fSequenceId)
{
}

inline XMLPScanToken::~XMLPScanToken()
{
}


// ---------------------------------------------------------------------------
//  XMLPScanToken: Public operators
// ---------------------------------------------------------------------------
inline XMLPScanToken& XMLPScanToken::operator=(const XMLPScanToken& toCopy)
{
    if (this == &toCopy)
        return *this;

    fScannerId = toCopy.fScannerId;
    fSequenceId = toCopy.fSequenceId;

    return *this;
}


// ---------------------------------------------------------------------------
//  XMLPScanToken: Hidden methods
// ---------------------------------------------------------------------------
inline void XMLPScanToken::set( const   XMLUInt32   scannerId
                                , const XMLUInt32   sequenceId)
{
    fScannerId = scannerId;
    fSequenceId = sequenceId;
}

XERCES_CPP_NAMESPACE_END

#endif
