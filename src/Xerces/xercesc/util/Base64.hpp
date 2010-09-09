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
 * $Id: Base64.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BASE64_HPP)
#define XERCESC_INCLUDE_GUARD_BASE64_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/framework/MemoryManager.hpp>

XERCES_CPP_NAMESPACE_BEGIN

//
// This class provides encode/decode for RFC 2045 Base64 as
// defined by RFC 2045, N. Freed and N. Borenstein.
// RFC 2045: Multipurpose Internet Mail Extensions (MIME)
// Part One: Format of Internet Message Bodies. Reference
// 1996 Available at: http://www.ietf.org/rfc/rfc2045.txt
// This class is used by XML Schema binary format validation
//
//
class XMLUTIL_EXPORT Base64
{
public :

    enum Conformance
    {
        Conf_RFC2045
      , Conf_Schema
    };

    //@{

    /**
     * Encodes octets into Base64 data
     *
     * NOTE: The returned buffer is dynamically allocated and is the
     * responsibility of the caller to delete it when not longer needed.
     * Use the memory manager to release the returned buffer or
     * operator delete() if none was provided.
     *
     * @param inputData Binary data in XMLByte stream.
     * @param inputLength Length of the XMLByte stream.
     * @param outputLength Length of the encoded Base64 byte stream.
     * @param memMgr client provided memory manager
     * @return Encoded Base64 data in XMLByte stream,
     *      or NULL if input data can not be encoded.
     */
    static XMLByte* encode(const XMLByte* const inputData
                         , const XMLSize_t      inputLength
                         , XMLSize_t*           outputLength
                         , MemoryManager* const memMgr = 0);

    /**
     * Decodes Base64 data into octets
     *
     * NOTE: The returned buffer is dynamically allocated and is the
     * responsibility of the caller to delete it when not longer needed.
     * Use the memory manager to release the returned buffer or
     * operator delete() if none was provided.
     *
     * @param inputData Base64 data in XMLByte stream.
     * @param decodedLength Length of decoded XMLByte stream.
     * @param memMgr client provided memory manager
     * @param conform conformance specified: if the input data conforms to the
     *                RFC 2045 it is allowed to have any number of whitespace
     *                characters inside; if it conforms to the XMLSchema specs,
     *                it is allowed to have at most one whitespace character
     *                between the quartets
     * @return Decoded binary data in XMLByte stream,
     *      or NULL if input data can not be decoded.
     */
    static XMLByte* decode(
                           const XMLByte*        const   inputData
                         ,       XMLSize_t*              decodedLength
                         ,       MemoryManager*  const   memMgr = 0
                         ,       Conformance             conform = Conf_RFC2045
                          );

   /**
     * Decodes Base64 data into octets
     *
     * NOTE: The returned buffer is dynamically allocated and is the
     * responsibility of the caller to delete it when not longer needed.
     * Use the memory manager to release the returned buffer or
     * operator delete() if none was provided.
     *
     * @param inputData Base64 data in XMLCh stream.
     * @param decodedLength Length of decoded XMLByte stream.
     * @param memMgr client provided memory manager
     * @param conform conformance specified: if the input data conforms to the
     *                RFC 2045 it is allowed to have any number of whitespace
     *                characters inside; if it conforms to the XMLSchema specs,
     *                it is allowed to have at most one whitespace character
     *                between the quartets
     * @return Decoded binary data in XMLByte stream,
     *      or NULL if input data can not be decoded.
     */
    static XMLByte* decodeToXMLByte(
                           const XMLCh*          const   inputData
                         ,       XMLSize_t*              decodedLength
                         ,       MemoryManager*  const   memMgr = 0
                         ,       Conformance             conform = Conf_RFC2045
                          );
    /**
     * Get data length
	 *
     * Returns length of decoded data given an array
     * containing encoded data.
     *
     * @param inputData Base64 data in XMLCh stream.
     * @param memMgr client provided memory manager
     * @param conform conformance specified
     * @return Length of decoded data,
	 *      or -1 if input data can not be decoded.
     */
    static int getDataLength(
                             const XMLCh*         const  inputData
                            ,      MemoryManager* const  memMgr = 0
                            ,      Conformance           conform = Conf_RFC2045
                             );

    //@}

     /**
     * get canonical representation
     *
     * Caller is responsible for the proper deallocation
     * of the string returned.
     *
     * @param inputData A string containing the Base64
     * @param memMgr client provided memory manager
     * @param conform conformance specified
     *
     * return: the canonical representation of the Base64
     *         if it is a valid Base64
     *         0 otherwise
     */

    static XMLCh* getCanonicalRepresentation
                  (
                      const XMLCh*          const inputData
                    ,       MemoryManager*  const memMgr = 0
                    ,       Conformance           conform = Conf_RFC2045
                  );

private :

    // -----------------------------------------------------------------------
    //  Helper methods
    // -----------------------------------------------------------------------

    static XMLByte* decode(
                           const XMLByte*        const   inputData
                         ,       XMLSize_t*              outputLength
                         ,       XMLByte*&               canRepData
                         ,       MemoryManager*  const   memMgr = 0
                         ,       Conformance             conform = Conf_RFC2045
                          );

    static bool isData(const XMLByte& octet);
    static bool isPad(const XMLByte& octet);

    static XMLByte set1stOctet(const XMLByte&, const XMLByte&);
    static XMLByte set2ndOctet(const XMLByte&, const XMLByte&);
    static XMLByte set3rdOctet(const XMLByte&, const XMLByte&);

    static void split1stOctet(const XMLByte&, XMLByte&, XMLByte&);
    static void split2ndOctet(const XMLByte&, XMLByte&, XMLByte&);
    static void split3rdOctet(const XMLByte&, XMLByte&, XMLByte&);

    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    Base64();
    Base64(const Base64&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  base64Alphabet
    //     The Base64 alphabet (see RFC 2045).
    //
    //  base64Padding
    //     Padding character (see RFC 2045).
    //
    //  base64Inverse
    //     Table used in decoding base64.
    //
    //  isInitialized
    //     Set once base64Inverse is initialized.
    //
    //  quadsPerLine
    //     Number of quadruplets per one line. The encoded output
    //     stream must be represented in lines of no more
    //     than 19 quadruplets each.
    //
    // -----------------------------------------------------------------------

    static const XMLByte  base64Alphabet[];
    static const XMLByte  base64Padding;

    static const XMLByte  base64Inverse[];

    static const unsigned int  quadsPerLine;
};

// -----------------------------------------------------------------------
//  Helper methods
// -----------------------------------------------------------------------
inline bool Base64::isPad(const XMLByte& octet)
{
    return ( octet == base64Padding );
}

inline XMLByte Base64::set1stOctet(const XMLByte& b1, const XMLByte& b2)
{
    return (( b1 << 2 ) | ( b2 >> 4 ));
}

inline XMLByte Base64::set2ndOctet(const XMLByte& b2, const XMLByte& b3)
{
    return (( b2 << 4 ) | ( b3 >> 2 ));
}

inline XMLByte Base64::set3rdOctet(const XMLByte& b3, const XMLByte& b4)
{
    return (( b3 << 6 ) | b4 );
}

inline void Base64::split1stOctet(const XMLByte& ch, XMLByte& b1, XMLByte& b2) {
    b1 = ch >> 2;
    b2 = ( ch & 0x3 ) << 4;
}

inline void Base64::split2ndOctet(const XMLByte& ch, XMLByte& b2, XMLByte& b3) {
    b2 |= ch >> 4;  // combine with previous value
    b3 = ( ch & 0xf ) << 2;
}

inline void Base64::split3rdOctet(const XMLByte& ch, XMLByte& b3, XMLByte& b4) {
    b3 |= ch >> 6;  // combine with previous value
    b4 = ( ch & 0x3f );
}

XERCES_CPP_NAMESPACE_END

#endif
