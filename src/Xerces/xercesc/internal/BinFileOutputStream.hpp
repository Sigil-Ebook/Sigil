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
 * $Id: BinFileOutputStream.hpp 553915 2007-07-06 14:57:08Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_BINFILEOUTPUTSTREAM_HPP)
#define XERCESC_INCLUDE_GUARD_BINFILEOUTPUTSTREAM_HPP

#include <xercesc/framework/BinOutputStream.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT BinFileOutputStream : public BinOutputStream
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    ~BinFileOutputStream();

    BinFileOutputStream
    (
        const   XMLCh* const    fileName
        , MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
    );

    BinFileOutputStream
    (
         const   char* const     fileName
       , MemoryManager* const    manager = XMLPlatformUtils::fgMemoryManager
    );

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

    virtual void writeBytes
    (
          const XMLByte* const      toGo
        , const XMLSize_t        maxToWrite
    );


private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    BinFileOutputStream(const BinFileOutputStream&);
    BinFileOutputStream& operator=(const BinFileOutputStream&); 

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
//  BinFileOutputStream: Getter methods
// ---------------------------------------------------------------------------
inline bool BinFileOutputStream::getIsOpen() const
{
    return (fSource != (FileHandle) XERCES_Invalid_File_Handle);
}

XERCES_CPP_NAMESPACE_END

#endif
