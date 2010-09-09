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
 * $Id: BinOutputStream.hpp 553915 2007-07-06 14:57:08Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BIN_OUTPUT_STREAM_HPP)
#define XERCESC_INCLUDE_GUARD_BIN_OUTPUT_STREAM_HPP

#include <xercesc/util/XMemory.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT BinOutputStream : public XMemory
{
public :
    // -----------------------------------------------------------------------
    //  Virtual destructor for derived classes
    // -----------------------------------------------------------------------
    virtual ~BinOutputStream();

    // -----------------------------------------------------------------------
    //  The virtual output stream interface
    // -----------------------------------------------------------------------
    virtual XMLFilePos curPos() const = 0;

    virtual void writeBytes
    (
          const XMLByte* const      toGo
        , const XMLSize_t           maxToWrite
    ) = 0;

protected :
    // -----------------------------------------------------------------------
    //  Hidden Constructors
    // -----------------------------------------------------------------------
    BinOutputStream();


private :
    // -----------------------------------------------------------------------
    //  Unimplemented Constructors
    // -----------------------------------------------------------------------
    BinOutputStream(const BinOutputStream&);
    BinOutputStream& operator=(const BinOutputStream&);
};

XERCES_CPP_NAMESPACE_END

#endif
