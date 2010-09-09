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
 * $Id: XMLReaderFactory.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLREADERFACTORY_HPP)
#define XERCESC_INCLUDE_GUARD_XMLREADERFACTORY_HPP

#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax/SAXException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class MemoryManager;
class XMLGrammarPool;

/**
  * Creates a SAX2 parser (SAX2XMLReader).
  *
  * <p>Note: The parser object returned by XMLReaderFactory is owned by the
  * calling users, and it's the responsibility of the users to delete that
  * parser object, once they no longer need it.</p>
  *
  * @see SAX2XMLReader#SAX2XMLReader
  */
class SAX2_EXPORT XMLReaderFactory
{
protected:                // really should be private, but that causes compiler warnings.
	XMLReaderFactory() ;
	~XMLReaderFactory() ;

public:
	static SAX2XMLReader * createXMLReader(  MemoryManager* const  manager = XMLPlatformUtils::fgMemoryManager
                                           , XMLGrammarPool* const gramPool = 0
                                          ) ;
	static SAX2XMLReader * createXMLReader(const XMLCh* className)  ;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLReaderFactory(const XMLReaderFactory&);
    XMLReaderFactory& operator=(const XMLReaderFactory&);
};

inline SAX2XMLReader * XMLReaderFactory::createXMLReader(const XMLCh *)
{	
	throw SAXNotSupportedException();
	// unimplemented
	return 0;
}

XERCES_CPP_NAMESPACE_END

#endif
