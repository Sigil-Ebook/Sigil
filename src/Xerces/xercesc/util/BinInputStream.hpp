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
 * $Id: BinInputStream.hpp 670359 2008-06-22 13:43:45Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BININPUTSTREAM_HPP)
#define XERCESC_INCLUDE_GUARD_BININPUTSTREAM_HPP

#include <xercesc/util/XMemory.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT BinInputStream : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Virtual destructor for derived classes
    // -----------------------------------------------------------------------
    virtual ~BinInputStream();


    // -----------------------------------------------------------------------
    //  The virtual input stream interface
    // -----------------------------------------------------------------------
    virtual XMLFilePos curPos() const = 0;

    virtual XMLSize_t readBytes
    (
                XMLByte* const      toFill
        , const XMLSize_t           maxToRead
    ) = 0;

    /**
     * Return the "out-of-band" content type for the data supplied by this
     * input stream in the form of the media-type production (mime type
     * with optional parameters such as encoding) as defined by the HTTP 1.1
     * specification. If no such content type is provided for the data, 0 is
     * returned. This function is expected to return the correct value at
     * any time after the construction of the stream.
     *
     * An example of the stream that may return non-0 from this function is
     * an HTTP stream with the value returned taken from the "Content-Type"
     * HTTP header. Note also that if the encoding of the data is known
     * to the application by some other means then the setEncoding function
     * in the InputSource object should be used instead. The getContentType
     * function should only be used to return information that is intrinsic
     * to the stream.
     *
     * @return The content type, or 0 if one is not available.
     */
    virtual const XMLCh* getContentType() const = 0;


protected :
    // -----------------------------------------------------------------------
    //  Hidden Constructors
    // -----------------------------------------------------------------------
    BinInputStream();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented Constructors
    // -----------------------------------------------------------------------
    BinInputStream(const BinInputStream&);
    BinInputStream& operator=(const BinInputStream&);
};

XERCES_CPP_NAMESPACE_END

#endif
