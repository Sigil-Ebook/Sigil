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
 * $Id: BinFileInputStream.hpp 670359 2008-06-22 13:43:45Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BINFILEINPUTSTREAM_HPP)
#define XERCESC_INCLUDE_GUARD_BINFILEINPUTSTREAM_HPP

#include <xercesc/util/BinInputStream.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT BinFileInputStream : public BinInputStream
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------
    BinFileInputStream
    (
        const   XMLCh* const    fileName
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    BinFileInputStream
    (
        const   char* const     fileName
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    BinFileInputStream
    (
        const   FileHandle      toUse
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    virtual ~BinFileInputStream();


    // -----------------------------------------------------------------------
    //  Getter methods
    // -----------------------------------------------------------------------
    bool getIsOpen() const;
    XMLFilePos getSize() const;
    void reset();


    // -----------------------------------------------------------------------
    //  Implementation of the input stream interface
    // -----------------------------------------------------------------------
    virtual XMLFilePos curPos() const;

    virtual XMLSize_t readBytes
    (
                XMLByte* const      toFill
        , const XMLSize_t           maxToRead
    );

    virtual const XMLCh* getContentType() const;

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    BinFileInputStream(const BinFileInputStream&);
    BinFileInputStream& operator=(const BinFileInputStream&);

    // -----------------------------------------------------------------------
    //  Private data members
    //
    //  fSource
    //      The source file that we represent. The FileHandle type is defined
    //      per platform.
    // -----------------------------------------------------------------------
    FileHandle              fSource;
    MemoryManager* const    fMemoryManager;
};


// ---------------------------------------------------------------------------
//  BinFileInputStream: Getter methods
// ---------------------------------------------------------------------------
inline bool BinFileInputStream::getIsOpen() const
{
    return (fSource != (FileHandle) XERCES_Invalid_File_Handle);
}

XERCES_CPP_NAMESPACE_END

#endif
