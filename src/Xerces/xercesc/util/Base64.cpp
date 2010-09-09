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

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/Base64.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/internal/XMLReader.hpp>
#include <xercesc/framework/MemoryManager.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  constants
// ---------------------------------------------------------------------------
static const int BASELENGTH = 255;
static const int FOURBYTE   = 4;

// ---------------------------------------------------------------------------
//  class data member
// ---------------------------------------------------------------------------

// the base64 alphabet according to definition in RFC 2045
const XMLByte Base64::base64Alphabet[] = {
    chLatin_A, chLatin_B, chLatin_C, chLatin_D, chLatin_E,
    chLatin_F, chLatin_G, chLatin_H, chLatin_I, chLatin_J,
    chLatin_K, chLatin_L, chLatin_M, chLatin_N, chLatin_O,
    chLatin_P, chLatin_Q, chLatin_R, chLatin_S, chLatin_T,
    chLatin_U, chLatin_V, chLatin_W, chLatin_X, chLatin_Y, chLatin_Z,
    chLatin_a, chLatin_b, chLatin_c, chLatin_d, chLatin_e,
    chLatin_f, chLatin_g, chLatin_h, chLatin_i, chLatin_j,
    chLatin_k, chLatin_l, chLatin_m, chLatin_n, chLatin_o,
    chLatin_p, chLatin_q, chLatin_r, chLatin_s, chLatin_t,
    chLatin_u, chLatin_v, chLatin_w, chLatin_x, chLatin_y, chLatin_z,
    chDigit_0, chDigit_1, chDigit_2, chDigit_3, chDigit_4,
    chDigit_5, chDigit_6, chDigit_7, chDigit_8, chDigit_9,
    chPlus, chForwardSlash, chNull
};

// This is an inverse table for base64 decoding.  So, if
// base64Alphabet[17] = 'R', then base64Inverse['R'] = 17.
//
// For characters not in base64Alphabet then
// base64Inverse[ch] = 0xFF.
const XMLByte Base64::base64Inverse[BASELENGTH] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

const XMLByte Base64::base64Padding = chEqual;

/***
 *
 * Memory Management Issue:
 *
 * . For memory allocated for result returned to caller (external memory), 
 *   the plugged memory manager is used if it is provided, otherwise global 
 *   new used to retain the pre-memory-manager behaviour.
 *
 * . For memory allocated for temperary buffer (internal memory), 
 *   XMLPlatformUtils::fgMemoryManager is used.
 *
 */

static void* getExternalMemory(  MemoryManager* const allocator
                               , XMLSize_t const   sizeToAllocate)
{
   return allocator ? allocator->allocate(sizeToAllocate)
       : ::operator new(sizeToAllocate);
}

/***
 * internal memory is deallocated by janitorArray
 */ 
static void returnExternalMemory(  MemoryManager* const allocator
                                 , void*                buffer)
{
    allocator ? allocator->deallocate(buffer)
        : ::operator delete(buffer);
}

/**
 *     E2-9
 *
 *     Canonical-base64Binary ::= (B64line* B64lastline)?
 *
 *      B64line ::= B64x15 B64x15 B64x15 B64x15 B64x15 B64 #xA
 *                  76 Base64 characters followed by newline
 *
 *      B64x15  ::= B64 B64 B64 B64 B64
 *                  B64 B64 B64 B64 B64
 *                  B64 B64 B64 B64 B64
 *
 *      B64lastline ::= B64x4? B64x4? B64x4? B64x4?
 *                      B64x4? B64x4? B64x4? B64x4?
 *                      B64x4? B64x4? B64x4? B64x4?
 *                      B64x4? B64x4? B64x4? B64x4?
 *                      B64x4? B64x4?
 *                      (B64x4 | (B64 B64 B16 '=') | (B64 B04 '=='))
 *                      #xA
 *
 *      B64x4   ::= B64 B64 B64 B64
 *      B04     ::= [AQgw]
 *      B16     ::= [AEIMQUYcgkosw048]
*/

// number of quadruplets per one line ( must be >1 and <19 )
const unsigned int Base64::quadsPerLine = 15;

XMLByte* Base64::encode(const XMLByte* const inputData
                      , const XMLSize_t      inputLength
                      , XMLSize_t*           outputLength                      
                      , MemoryManager* const memMgr)
{
    if (!inputData || !outputLength)
        return 0;

    int quadrupletCount = ( (int)inputLength + 2 ) / 3;
    if (quadrupletCount == 0)
        return 0;

    // number of rows in encoded stream ( including the last one )
    int lineCount = ( quadrupletCount + quadsPerLine-1 ) / quadsPerLine;

    //
    // convert the triplet(s) to quadruplet(s)
    //
    XMLByte  b1, b2, b3, b4;  // base64 binary codes ( 0..63 )

    XMLSize_t inputIndex = 0;
    XMLSize_t outputIndex = 0;
    XMLByte *encodedData = (XMLByte*) getExternalMemory(memMgr, (quadrupletCount*FOURBYTE+lineCount+1) * sizeof(XMLByte));

    //
    // Process all quadruplet(s) except the last
    //
    int quad = 1;
    for (; quad <= quadrupletCount-1; quad++ )
    {
        // read triplet from the input stream
        split1stOctet( inputData[ inputIndex++ ], b1, b2 );
        split2ndOctet( inputData[ inputIndex++ ], b2, b3 );
        split3rdOctet( inputData[ inputIndex++ ], b3, b4 );

        // write quadruplet to the output stream
        encodedData[ outputIndex++ ] = base64Alphabet[ b1 ];
        encodedData[ outputIndex++ ] = base64Alphabet[ b2 ];
        encodedData[ outputIndex++ ] = base64Alphabet[ b3 ];
        encodedData[ outputIndex++ ] = base64Alphabet[ b4 ];

        if (( quad % quadsPerLine ) == 0 )
            encodedData[ outputIndex++ ] = chLF;
    }

    //
    // process the last Quadruplet
    //
    // first octet is present always, process it
    split1stOctet( inputData[ inputIndex++ ], b1, b2 );
    encodedData[ outputIndex++ ] = base64Alphabet[ b1 ];

    if( inputIndex < inputLength )
    {
        // second octet is present, process it
        split2ndOctet( inputData[ inputIndex++ ], b2, b3 );
        encodedData[ outputIndex++ ] = base64Alphabet[ b2 ];

        if( inputIndex < inputLength )
        {
            // third octet present, process it
            // no PAD e.g. 3cQl
            split3rdOctet( inputData[ inputIndex++ ], b3, b4 );
            encodedData[ outputIndex++ ] = base64Alphabet[ b3 ];
            encodedData[ outputIndex++ ] = base64Alphabet[ b4 ];
        }
        else
        {
            // third octet not present
            // one PAD e.g. 3cQ=
            encodedData[ outputIndex++ ] = base64Alphabet[ b3 ];
            encodedData[ outputIndex++ ] = base64Padding;
        }
    }
    else
    {
        // second octet not present
        // two PADs e.g. 3c==
        encodedData[ outputIndex++ ] = base64Alphabet[ b2 ];
        encodedData[ outputIndex++ ] = base64Padding;
        encodedData[ outputIndex++ ] = base64Padding;
    }

    // write out end of the last line
    encodedData[ outputIndex++ ] = chLF;
    // write out end of string
    encodedData[ outputIndex ] = 0;

    *outputLength = outputIndex;

    return encodedData;
}

//
// delete the buffer allocated by decode() if
// decoding is successfully done.
//
// In previous version, we use XMLString::strLen(decodedData)
// to get the length, this will fail for test case containing
// consequtive "A", such "AAFF", or "ab56AA56". Instead of
// returning 3/6, we have 0 and 3, indicating that "AA", after
// decoded, is interpreted as <null> by the strLen().
//
// Since decode() has track of length of the decoded data, we
// will get this length from decode(), instead of strLen().
//
int Base64::getDataLength(const XMLCh*         const inputData
                        ,       MemoryManager* const manager
                        ,       Conformance          conform )

{
    XMLSize_t retLen = 0;
    XMLByte* decodedData = decodeToXMLByte(inputData, &retLen, manager, conform);

    if ( !decodedData )
        return -1;
    else
    {
        returnExternalMemory(manager, decodedData);
        return (int)retLen;
    }
}

XMLByte* Base64::decode(const XMLByte*       const  inputData
                      ,       XMLSize_t*            decodedLength
                      ,       MemoryManager* const  memMgr
                      ,       Conformance           conform )
{
    XMLByte* canRepInByte = 0;
    XMLByte* retStr = decode(
                              inputData
                            , decodedLength
                            , canRepInByte
                            , memMgr
                            , conform);

    /***
     * Release the canRepData
     */ 
    if (retStr)
        returnExternalMemory(memMgr, canRepInByte);

    return retStr;
}


XMLByte* Base64::decodeToXMLByte(const XMLCh*         const   inputData
                                ,      XMLSize_t*             decodedLen
                                ,      MemoryManager* const   memMgr
                                ,      Conformance            conform )
{
    if (!inputData || !*inputData)
        return 0;

    /***
     *  Move input data to a XMLByte buffer
     */
    XMLSize_t srcLen = XMLString::stringLen(inputData);
    XMLByte *dataInByte = (XMLByte*) getExternalMemory(memMgr, (srcLen+1) * sizeof(XMLByte));
    ArrayJanitor<XMLByte> janFill(dataInByte, memMgr ? memMgr : XMLPlatformUtils::fgMemoryManager);

    for (XMLSize_t i = 0; i < srcLen; i++)
        dataInByte[i] = (XMLByte)inputData[i];

    dataInByte[srcLen] = 0;

    /***
     * Forward to the actual decoding method to do the decoding
     */
    *decodedLen = 0;
    return decode(dataInByte, decodedLen, memMgr, conform);
}

/***
* E2-54
*
* Canonical-base64Binary ::= (B64 B64 B64 B64)*((B64 B64 B16 '=')|(B64 B04 '=='))? 
* B04                    ::= [AQgw]
* B16                    ::= [AEIMQUYcgkosw048]
* B64                    ::= [A-Za-z0-9+/] 
*
***/
XMLCh* Base64::getCanonicalRepresentation(const XMLCh*         const   inputData
                                        ,       MemoryManager* const   memMgr
                                        ,       Conformance            conform)
    
{
    if (!inputData || !*inputData) 
        return 0;

    /***
     *  Move input data to a XMLByte buffer
     */
    XMLSize_t srcLen = XMLString::stringLen(inputData);
    XMLByte *dataInByte = (XMLByte*) getExternalMemory(memMgr, (srcLen+1) * sizeof(XMLByte));
    ArrayJanitor<XMLByte> janFill(dataInByte, memMgr ? memMgr : XMLPlatformUtils::fgMemoryManager);

    for (XMLSize_t i = 0; i < srcLen; i++)
        dataInByte[i] = (XMLByte)inputData[i];

    dataInByte[srcLen] = 0;

    /***
     * Forward to the actual decoding method to do the decoding
     */
    XMLSize_t decodedLength = 0;
    XMLByte*     canRepInByte = 0;
    XMLByte*     retStr = decode(
                              dataInByte
                            , &decodedLength
                            , canRepInByte
                            , memMgr
                            , conform);

    if (!retStr)
        return 0;

    /***
     * Move canonical representation to a XMLCh buffer to return
     */
    XMLSize_t canRepLen = XMLString::stringLen((char*)canRepInByte);
    XMLCh *canRepData = (XMLCh*) getExternalMemory(memMgr, (canRepLen + 1) * sizeof(XMLCh));
               
    for (XMLSize_t j = 0; j < canRepLen; j++)
        canRepData[j] = (XMLCh)canRepInByte[j];

    canRepData[canRepLen] = 0;

    /***
     * Release the memory allocated in the actual decoding method
     */ 
    returnExternalMemory(memMgr, retStr);
    returnExternalMemory(memMgr, canRepInByte);

    return canRepData;
}
// -----------------------------------------------------------------------
//  Helper methods
// -----------------------------------------------------------------------

//
// return 0(null) if invalid data found.
// return the buffer containning decoded data otherwise
// the caller is responsible for the de-allocation of the
// buffer returned.
//
// temporary data, rawInputData, is ALWAYS released by this function.
//

/***
 *     E2-9
 *
 *     Base64Binary ::= S? B64quartet* B64final?
 *
 *     B64quartet   ::= B64 S? B64 S? B64 S? B64 S?
 *
 *     B64final     ::= B64 S? B04 S? '=' S? '=' S?
 *                    | B64 S? B64 S? B16 S? '=' S?
 *
 *     B04          ::= [AQgw]
 *     B16          ::= [AEIMQUYcgkosw048]
 *     B64          ::= [A-Za-z0-9+/]
 *
 *
 *     E2-54
 *
 *     Base64Binary  ::=  ((B64S B64S B64S B64S)*
 *                         ((B64S B64S B64S B64) |
 *                          (B64S B64S B16S '=') |
 *                          (B64S B04S '=' #x20? '=')))?
 *
 *     B64S         ::= B64 #x20?
 *     B16S         ::= B16 #x20?
 *     B04S         ::= B04 #x20?
 *
 *
 *     Note that this grammar requires the number of non-whitespace characters 
 *     in the lexical form to be a multiple of four, and for equals signs to 
 *     appear only at the end of the lexical form; strings which do not meet these 
 *     constraints are not legal lexical forms of base64Binary because they 
 *     cannot successfully be decoded by base64 decoders.
 * 
 *     Note: 
 *     The above definition of the lexical space is more restrictive than that given 
 *     in [RFC 2045] as regards whitespace -- this is not an issue in practice. Any 
 *     string compatible with the RFC can occur in an element or attribute validated 
 *     by this type, because the whiteSpace facet of this type is fixed to collapse, 
 *     which means that all leading and trailing whitespace will be stripped, and all 
 *     internal whitespace collapsed to single space characters, before the above grammar 
 *     is enforced.
 *
*/

XMLByte* Base64::decode (   const XMLByte*        const   inputData
                          ,       XMLSize_t*              decodedLength
                          ,       XMLByte*&               canRepData
                          ,       MemoryManager*  const   memMgr
                          ,       Conformance             conform
                        )
{
    if ((!inputData) || (!*inputData))
        return 0;
    
    //
    // remove all XML whitespaces from the base64Data
    //
    XMLSize_t inputLength = XMLString::stringLen( (const char*)inputData );
    XMLByte* rawInputData = (XMLByte*) getExternalMemory(memMgr, (inputLength+1) * sizeof(XMLByte));
    ArrayJanitor<XMLByte> jan(rawInputData, memMgr ? memMgr : XMLPlatformUtils::fgMemoryManager);

    XMLSize_t inputIndex = 0;
    XMLSize_t rawInputLength = 0;
    bool inWhiteSpace = false;

    switch (conform)
    {
    case Conf_RFC2045:
        while ( inputIndex < inputLength )
        {
            if (!XMLChar1_0::isWhitespace(inputData[inputIndex]))
            {
                rawInputData[ rawInputLength++ ] = inputData[ inputIndex ];
            }
            // RFC2045 does not explicitly forbid more than ONE whitespace 
            // before, in between, or after base64 octects.
            // Besides, S? allows more than ONE whitespace as specified in the production 
            // [3]   S   ::=   (#x20 | #x9 | #xD | #xA)+
            // therefore we do not detect multiple ws

            inputIndex++;
        }

        break;
    case Conf_Schema:
        // no leading #x20
        if (chSpace == inputData[inputIndex])
            return 0;

        while ( inputIndex < inputLength )
        {
            if (chSpace != inputData[inputIndex])
            {
                rawInputData[ rawInputLength++ ] = inputData[ inputIndex ];
                inWhiteSpace = false;
            }
            else
            {
                if (inWhiteSpace)
                    return 0; // more than 1 #x20 encountered
                else
                    inWhiteSpace = true;
            }

            inputIndex++;
        }

        // no trailing #x20
        if (inWhiteSpace)
            return 0;

        break;

    default:
        break;
    }

    //now rawInputData contains canonical representation 
    //if the data is valid Base64
    rawInputData[ rawInputLength ] = 0;

    // the length of raw data should be divisible by four
    if (( rawInputLength % FOURBYTE ) != 0 )
        return 0;

    int quadrupletCount = (int)rawInputLength / FOURBYTE;
    if ( quadrupletCount == 0 )
        return 0;

    //
    // convert the quadruplet(s) to triplet(s)
    //
    XMLByte  d1, d2, d3, d4;  // base64 characters
    XMLByte  b1, b2, b3, b4;  // base64 binary codes ( 0..64 )

    XMLSize_t rawInputIndex  = 0;
    XMLSize_t outputIndex    = 0;
    XMLByte *decodedData = (XMLByte*) getExternalMemory(memMgr, (quadrupletCount*3+1) * sizeof(XMLByte));

    //
    // Process all quadruplet(s) except the last
    //
    int quad = 1;
    for (; quad <= quadrupletCount-1; quad++ )
    {
        // read quadruplet from the input stream
        if (!isData( (d1 = rawInputData[ rawInputIndex++ ]) ) ||
            !isData( (d2 = rawInputData[ rawInputIndex++ ]) ) ||
            !isData( (d3 = rawInputData[ rawInputIndex++ ]) ) ||
            !isData( (d4 = rawInputData[ rawInputIndex++ ]) ))
        {
            // if found "no data" just return NULL
            returnExternalMemory(memMgr, decodedData);
            return 0;
        }

        b1 = base64Inverse[ d1 ];
        b2 = base64Inverse[ d2 ];
        b3 = base64Inverse[ d3 ];
        b4 = base64Inverse[ d4 ];

        // write triplet to the output stream
        decodedData[ outputIndex++ ] = set1stOctet(b1, b2);
        decodedData[ outputIndex++ ] = set2ndOctet(b2, b3);
        decodedData[ outputIndex++ ] = set3rdOctet(b3, b4);
    }

    //
    // process the last Quadruplet
    //
    // first two octets are present always, process them
    if (!isData( (d1 = rawInputData[ rawInputIndex++ ]) ) ||
        !isData( (d2 = rawInputData[ rawInputIndex++ ]) ))
    {
        // if found "no data" just return NULL
        returnExternalMemory(memMgr, decodedData);
        return 0;
    }

    b1 = base64Inverse[ d1 ];
    b2 = base64Inverse[ d2 ];

    // try to process last two octets
    d3 = rawInputData[ rawInputIndex++ ];
    d4 = rawInputData[ rawInputIndex++ ];

    if (!isData( d3 ) || !isData( d4 ))
    {
        // check if last two are PAD characters
        if (isPad( d3 ) && isPad( d4 ))
        {
            // two PAD e.g. 3c==
            if ((b2 & 0xf) != 0) // last 4 bits should be zero
            {
                returnExternalMemory(memMgr, decodedData);
                return 0;
            }

            decodedData[ outputIndex++ ] = set1stOctet(b1, b2);
        }
        else if (!isPad( d3 ) && isPad( d4 ))
        {
            // one PAD e.g. 3cQ=
            b3 = base64Inverse[ d3 ];
            if (( b3 & 0x3 ) != 0 ) // last 2 bits should be zero
            {
                returnExternalMemory(memMgr, decodedData);
                return 0;
            }

            decodedData[ outputIndex++ ] = set1stOctet( b1, b2 );
            decodedData[ outputIndex++ ] = set2ndOctet( b2, b3 );
        }
        else
        {
            // an error like "3c[Pad]r", "3cdX", "3cXd", "3cXX" where X is non data
            returnExternalMemory(memMgr, decodedData);
            return 0;
        }
    }
    else
    {
        // no PAD e.g 3cQl
        b3 = base64Inverse[ d3 ];
        b4 = base64Inverse[ d4 ];
        decodedData[ outputIndex++ ] = set1stOctet( b1, b2 );
        decodedData[ outputIndex++ ] = set2ndOctet( b2, b3 );
        decodedData[ outputIndex++ ] = set3rdOctet( b3, b4 );
    }

    // write out the end of string
    decodedData[ outputIndex ] = 0;
    *decodedLength = outputIndex;

    //allow the caller to have access to the canonical representation
    jan.release(); 
    canRepData = rawInputData;

    return decodedData;
}

bool Base64::isData(const XMLByte& octet)
{
    return (base64Inverse[octet]!=(XMLByte)-1);
}

XERCES_CPP_NAMESPACE_END
