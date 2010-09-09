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

/**
 * $Id: StdInInputSource.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/StdInInputSource.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  StdInInputSource: Implementation of the input source interface
// ---------------------------------------------------------------------------
BinInputStream* StdInInputSource::makeStream() const
{
    // Open a binary file stream for the standard input file handle
    BinFileInputStream* retStream = new (getMemoryManager()) BinFileInputStream
    (
        XMLPlatformUtils::openStdInHandle(getMemoryManager())
    );

    if (!retStream->getIsOpen())
    {
        delete retStream;
        return 0;
    }
    return retStream;
}

XERCES_CPP_NAMESPACE_END

