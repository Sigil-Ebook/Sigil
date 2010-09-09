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
 * $Id: LocalFileFormatTarget.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_LOCALFILEFORMATTARGET_HPP)
#define XERCESC_INCLUDE_GUARD_LOCALFILEFORMATTARGET_HPP

#include <xercesc/framework/XMLFormatter.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLPARSER_EXPORT LocalFileFormatTarget : public XMLFormatTarget {
public:

    /** @name constructors and destructor */
    //@{
    LocalFileFormatTarget
    (
        const XMLCh* const
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    LocalFileFormatTarget
    (
        const char* const
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    ~LocalFileFormatTarget();
    //@}

    // -----------------------------------------------------------------------
    //  Implementations of the format target interface
    // -----------------------------------------------------------------------
    virtual void writeChars(const XMLByte* const toWrite
                          , const XMLSize_t      count
                          , XMLFormatter* const  formatter);

    virtual void flush();

private:
    // -----------------------------------------------------------------------
    //  Unimplemented methods.
    // -----------------------------------------------------------------------
    LocalFileFormatTarget(const LocalFileFormatTarget&);
    LocalFileFormatTarget& operator=(const LocalFileFormatTarget&);

    // -----------------------------------------------------------------------
    //  Private helpers
    // -----------------------------------------------------------------------
    void ensureCapacity(const XMLSize_t extraNeeded);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSource
    //      The source file that we represent. The FileHandle type is defined
    //      per platform.
    //
    //  fDataBuf
    //      The pointer to the buffer data. Its always
    //      one larger than fCapacity, to leave room for the null terminator.
    //
    //  fIndex
    //      The current index into the buffer, as characters are appended
    //      to it. If its zero, then the buffer is empty.
    //
    //  fCapacity
    //      The current capacity of the buffer. Its actually always one
    //      larger, to leave room for the null terminator.
    // -----------------------------------------------------------------------
    FileHandle      fSource;
    XMLByte*        fDataBuf;
    XMLSize_t       fIndex;
    XMLSize_t       fCapacity;
    MemoryManager*  fMemoryManager;
};


XERCES_CPP_NAMESPACE_END

#endif
