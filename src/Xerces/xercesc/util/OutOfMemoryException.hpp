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
 * $Id: OutOfMemoryException.hpp 673960 2008-07-04 08:50:12Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_OUT_OF_MEMORY_EXCEPTION_HPP)
#define XERCESC_INCLUDE_GUARD_OUT_OF_MEMORY_EXCEPTION_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

static const XMLCh gDefOutOfMemoryErrMsg[] =
{
        chLatin_O, chLatin_u, chLatin_t, chLatin_O
    ,   chLatin_f, chLatin_M, chLatin_e, chLatin_m
    ,   chLatin_o, chLatin_r, chLatin_y, chNull
};

class XMLUTIL_EXPORT OutOfMemoryException : public XMemory
{
public:

    OutOfMemoryException();
    ~OutOfMemoryException();
    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    XMLExcepts::Codes getCode() const;
    const XMLCh* getMessage() const;
    const XMLCh* getType() const;
    const char* getSrcFile() const;
    XMLFileLoc getSrcLine() const;

    OutOfMemoryException(const OutOfMemoryException& toCopy);
    OutOfMemoryException& operator=(const OutOfMemoryException& toAssign);
};

// constructors/destructors...
inline OutOfMemoryException::OutOfMemoryException() {}
inline OutOfMemoryException::~OutOfMemoryException() {}
inline OutOfMemoryException::OutOfMemoryException(const OutOfMemoryException& other) : XMemory(other) {}
inline OutOfMemoryException& OutOfMemoryException::operator=(const OutOfMemoryException&)
{
    return *this;
}

// ---------------------------------------------------------------------------
//  OutOfMemoryException: Getter methods
// ---------------------------------------------------------------------------
inline XMLExcepts::Codes OutOfMemoryException::getCode() const
{
    return XMLExcepts::Out_Of_Memory;
}

inline const XMLCh* OutOfMemoryException::getMessage() const
{
    return gDefOutOfMemoryErrMsg;
}

inline const XMLCh* OutOfMemoryException::getType() const
{
    return gDefOutOfMemoryErrMsg;
}

inline const char* OutOfMemoryException::getSrcFile() const
{
    return "";
}

inline XMLFileLoc OutOfMemoryException::getSrcLine() const {
    return 0;
}

XERCES_CPP_NAMESPACE_END

#endif
