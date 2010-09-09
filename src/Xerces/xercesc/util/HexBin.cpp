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
#include <xercesc/util/HexBin.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/Janitor.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  constants
// ---------------------------------------------------------------------------
static const int BASELENGTH = 255;

// ---------------------------------------------------------------------------
//  class data member
// ---------------------------------------------------------------------------
const XMLByte HexBin::hexNumberTable[BASELENGTH] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


int HexBin::getDataLength(const XMLCh* const hexData)
{
    if (!isArrayByteHex(hexData))
        return -1;

    return (int)XMLString::stringLen(hexData)/2;
}

bool HexBin::isArrayByteHex(const XMLCh* const hexData)
{
    if (( hexData == 0 ) || ( *hexData == 0 )) // zero length
        return true;

    XMLSize_t strLen = XMLString::stringLen(hexData);
    if ( strLen%2 != 0 )
        return false;

    for ( XMLSize_t i = 0; i < strLen; i++ )
        if( !isHex(hexData[i]) )
            return false;

    return true;
}

XMLCh* HexBin::getCanonicalRepresentation(const XMLCh*          const hexData
                                        ,       MemoryManager*  const manager)
{

    if (getDataLength(hexData) == -1)
        return 0;

    XMLCh* retStr = XMLString::replicate(hexData, manager);
    XMLString::upperCaseASCII(retStr);

    return retStr;
}

XMLByte* HexBin::decodeToXMLByte(const XMLCh*          const   hexData
                    ,       MemoryManager*  const   manager)
{
    if (( hexData == 0 ) || ( *hexData == 0 )) // zero length
        return 0;

    XMLSize_t strLen = XMLString::stringLen(hexData);
    if ( strLen%2 != 0 )
        return 0;

    //prepare the return string
    int decodeLength = (int)strLen/2;
    XMLByte *retVal = (XMLByte*) manager->allocate( (decodeLength + 1) * sizeof(XMLByte));
    ArrayJanitor<XMLByte> janFill(retVal, manager);
    
    XMLByte temp1, temp2;
    for( int i = 0; i<decodeLength; i++ ) {
        temp1 = hexNumberTable[hexData[i*2]];
        if (temp1 == (XMLByte) -1)
            return 0;
        temp2 = hexNumberTable[hexData[i*2+1]];
        if (temp2 == (XMLByte) -1)
            return 0;
        retVal[i] = ((temp1 << 4) | temp2);
    }

    janFill.release();
    retVal[decodeLength] = 0;
    return retVal;
}


// -----------------------------------------------------------------------
//  Helper methods
// -----------------------------------------------------------------------
bool HexBin::isHex(const XMLCh& octet)
{
    // sanity check to avoid out-of-bound index
    if ( octet >= BASELENGTH )
        return false;

    return (hexNumberTable[octet] != (XMLByte) -1);
}

XERCES_CPP_NAMESPACE_END
