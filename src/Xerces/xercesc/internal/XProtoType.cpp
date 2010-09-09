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
 * $Id: XProtoType.cpp 834826 2009-11-11 10:03:53Z borisk $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/XProtoType.hpp>
#include <xercesc/internal/XSerializeEngine.hpp>
#include <xercesc/util/XMLString.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/***
 *
 *  write the length of the class name
 *  write the class name
 *
 ***/
void XProtoType::store(XSerializeEngine& serEng) const
{
    XMLSize_t strLen = XMLString::stringLen((char*)fClassName);
	serEng << (unsigned long)strLen;
	serEng.write(fClassName, strLen * sizeof(XMLByte));
}

/***
 *
 *  To verify that the content in the binary stream
 *  is the same as this class
 *
 ***/
void XProtoType::load(XSerializeEngine& serEng
                    , XMLByte* const    inName
                    , MemoryManager* const manager)
{
    if (!inName)
    {       
        ThrowXMLwithMemMgr(XSerializationException
               , XMLExcepts::XSer_ProtoType_Null_ClassName, manager);
    }

    // read and check class name length
    XMLSize_t inNameLen = XMLString::stringLen((char*)inName); 
    XMLSize_t classNameLen = 0;
    serEng >> (unsigned long&)classNameLen;

	if (classNameLen != inNameLen)
    {
        XMLCh value1[17];
        XMLCh value2[17];
        XMLString::sizeToText(inNameLen,    value1, 16, 10, manager);
        XMLString::sizeToText(classNameLen, value2, 16, 10, manager);

        ThrowXMLwithMemMgr2(XSerializationException
                , XMLExcepts::XSer_ProtoType_NameLen_Dif
                , value1
                , value2
                , manager);  
    }

    // read and check class name
	XMLByte  className[256];
    serEng.read(className, classNameLen*sizeof(XMLByte));
    className[classNameLen] = '\0';

    if ( !XMLString::equals((char*)className, (char*)inName))
    {
        //we don't have class name exceed this length in xerces
        XMLCh name1[256];
        XMLCh name2[256];
        XMLCh *tmp = XMLString::transcode((char*)inName, manager);
        XMLString::copyNString(name1, tmp, 255);
        manager->deallocate(tmp);
        tmp = XMLString::transcode((char*)className, manager);
        XMLString::copyNString(name2, tmp, 255);
        manager->deallocate(tmp);

        ThrowXMLwithMemMgr2(XSerializationException
                , XMLExcepts::XSer_ProtoType_Name_Dif
                , name1
                , name2
                , manager);
    }

    return;

}

XERCES_CPP_NAMESPACE_END

