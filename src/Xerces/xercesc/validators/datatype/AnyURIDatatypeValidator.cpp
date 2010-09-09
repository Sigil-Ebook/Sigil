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
 * $Id: AnyURIDatatypeValidator.cpp 676796 2008-07-15 05:04:13Z dbertoni $
 */

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <stdio.h>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLUTF8Transcoder.hpp>
#include <xercesc/framework/XMLBuffer.hpp>
#include <xercesc/validators/datatype/AnyURIDatatypeValidator.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeFacetException.hpp>
#include <xercesc/validators/datatype/InvalidDatatypeValueException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
AnyURIDatatypeValidator::AnyURIDatatypeValidator(MemoryManager* const manager)
:AbstractStringValidator(0, 0, 0, DatatypeValidator::AnyURI, manager)
{}

AnyURIDatatypeValidator::~AnyURIDatatypeValidator()
{  
}

AnyURIDatatypeValidator::AnyURIDatatypeValidator(
                          DatatypeValidator*            const baseValidator
                        , RefHashTableOf<KVStringPair>* const facets
                        , RefArrayVectorOf<XMLCh>*      const enums
                        , const int                           finalSet
                        , MemoryManager* const manager)
:AbstractStringValidator(baseValidator, facets, finalSet, DatatypeValidator::AnyURI, manager)
{
    init(enums, manager);
}

DatatypeValidator* AnyURIDatatypeValidator::newInstance(
                                      RefHashTableOf<KVStringPair>* const facets
                                    , RefArrayVectorOf<XMLCh>*           const enums
                                    , const int                           finalSet
                                    , MemoryManager* const manager)
{
    return (DatatypeValidator*) new (manager) AnyURIDatatypeValidator(this, facets, enums, finalSet, manager);
}

// ---------------------------------------------------------------------------
//  Utilities
// ---------------------------------------------------------------------------

void AnyURIDatatypeValidator::checkValueSpace(const XMLCh* const content
                                              , MemoryManager* const manager)
{
    bool validURI = true;

    // check 3.2.17.c0 must: URI (rfc 2396/2723)
    try
    {
        // Support for relative URLs
        // According to Java 1.1: URLs may also be specified with a
        // String and the URL object that it is related to.
        //
        XMLSize_t len = XMLString::stringLen(content);
        if (len)
        {          
            // Encode special characters using XLink 5.4 algorithm
			XMLBuffer encoded((len*3)+1, manager);
            encode(content, len, encoded, manager);
            validURI = XMLUri::isValidURI(true, encoded.getRawBuffer(), true);            
        }
    }
    catch(const OutOfMemoryException&)
    {
        throw;
    }
    catch (...)
    {
        ThrowXMLwithMemMgr1(InvalidDatatypeValueException
                , XMLExcepts::VALUE_URI_Malformed
                , content
                , manager);
    }
    
    if (!validURI) {
        ThrowXMLwithMemMgr1(InvalidDatatypeValueException
                    , XMLExcepts::VALUE_URI_Malformed
                    , content
                    , manager);
    }
}

/***
 * To encode special characters in anyURI, by using %HH to represent
 * special ASCII characters: 0x00~0x1F, 0x7F, ' ', '<', '>', etc.
 * and non-ASCII characters (whose value >= 128).
 ***/
void AnyURIDatatypeValidator::encode(const XMLCh* const content, const XMLSize_t len, XMLBuffer& encoded, MemoryManager* const manager)
{
    static const bool needEscapeMap[] = {
        true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 0x00 to 0x0F need escape */
        true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , true , /* 0x10 to 0x1F need escape */
        true , false, true , false, false, false, false, false, false, false, false, false, false, false, false, false, /* 0x20:' ', 0x22:'"' */
        false, false, false, false, false, false, false, false, false, false, false, false, true , false, true , false, /* 0x3C:'<', 0x3E:'>' */
        false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
        false, false, false, false, false, false, false, false, false, false, false, false, true , false, true , false, /* 0x5C:'\\', 0x5E:'^' */
        true , false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, /* 0x60:'`' */
        false, false, false, false, false, false, false, false, false, false, false, true , true , true , true , true   /* 0x7B:'{', 0x7C:'|', 0x7D:'}', 0x7E:'~', 0x7F:DEL */
    };

    // For each character in content
    XMLSize_t i;
    for (i = 0; i < len; i++)
    {
        int ch = (int)content[i];
        // If it's not an ASCII character, break here, and use UTF-8 encoding
        if (ch >= 128)
            break;

        if (needEscapeMap[ch])
        {
            char tempStr[3] = "\0";
            sprintf(tempStr, "%02X", ch);
            encoded.append('%');
            encoded.append((XMLCh)tempStr[0]);
            encoded.append((XMLCh)tempStr[1]);
        }
        else
        {
            encoded.append((XMLCh)ch);
        }
    }

    // we saw some non-ascii character
    if (i < len) {
        // get UTF-8 bytes for the remaining sub-string
        const XMLCh* remContent = (XMLCh*)&content[i];
        const XMLSize_t remContentLen = len - i;
        XMLByte* UTF8Byte = (XMLByte*)manager->allocate((remContentLen*4+1) * sizeof(XMLByte));
        XMLSize_t charsEaten;

        XMLUTF8Transcoder transcoder(XMLUni::fgUTF8EncodingString, remContentLen*4+1, manager);
        XMLSize_t utf8Len = transcoder.transcodeTo(remContent, remContentLen, UTF8Byte, remContentLen*4, charsEaten, XMLTranscoder::UnRep_RepChar);
        assert(charsEaten == remContentLen);

        XMLSize_t j;
        for (j = 0; j < utf8Len; j++) {
            XMLByte b = UTF8Byte[j];
            if (b >= 128 || needEscapeMap[b])
            {
                char tempStr[3] = "\0";
                sprintf(tempStr, "%02X", b);
                encoded.append('%');
                encoded.append((XMLCh)tempStr[0]);
                encoded.append((XMLCh)tempStr[1]);
            }
            else
            {
                encoded.append((XMLCh)b);
            }
        }
        manager->deallocate(UTF8Byte);
    }
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(AnyURIDatatypeValidator)

void AnyURIDatatypeValidator::serialize(XSerializeEngine& serEng)
{
    AbstractStringValidator::serialize(serEng);
}

XERCES_CPP_NAMESPACE_END

/**
  * End of file AnyURIDatatypeValidator.cpp
  */
