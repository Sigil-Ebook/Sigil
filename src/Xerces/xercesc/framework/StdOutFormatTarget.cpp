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
 * $Id: StdOutFormatTarget.cpp 881225 2009-11-17 10:19:57Z borisk $
 */

#include <xercesc/framework/StdOutFormatTarget.hpp>
#include <stdio.h>

XERCES_CPP_NAMESPACE_BEGIN

StdOutFormatTarget::StdOutFormatTarget()
{}

StdOutFormatTarget::~StdOutFormatTarget()
{
  flush ();
}

void StdOutFormatTarget::flush()
{
    fflush(stdout);
}

void StdOutFormatTarget::writeChars(const XMLByte* const  toWrite
                                  , const XMLSize_t       count
                                  , XMLFormatter* const)
{
    XMLSize_t written=fwrite(toWrite, sizeof(XMLByte), count, stdout);
    if(written!=count)
        ThrowXML(XMLPlatformUtilsException, XMLExcepts::File_CouldNotWriteToFile);
    fflush(stdout);
}

XERCES_CPP_NAMESPACE_END

