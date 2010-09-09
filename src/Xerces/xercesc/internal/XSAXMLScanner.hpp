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
 * $Id: XSAXMLScanner.hpp 676911 2008-07-15 13:27:32Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSAXMLSCANNER_HPP)
#define XERCESC_INCLUDE_GUARD_XSAXMLSCANNER_HPP

#include <xercesc/internal/SGXMLScanner.hpp>


XERCES_CPP_NAMESPACE_BEGIN

//
//  This is a scanner class, which processes/validates contents of XML Schema
//  Annotations. It's intended for internal use only.
//
class XMLPARSER_EXPORT XSAXMLScanner : public SGXMLScanner
{
public :
    // -----------------------------------------------------------------------
    //  Destructor
    // -----------------------------------------------------------------------
    virtual ~XSAXMLScanner();

    // -----------------------------------------------------------------------
    //  XMLScanner public virtual methods
    // -----------------------------------------------------------------------
    virtual const XMLCh* getName() const;

protected:
    // -----------------------------------------------------------------------
    //  Constructors
    // -----------------------------------------------------------------------
    /**
     * The grammar representing the XML Schema annotation (xsaGrammar) is
     * passed in by the caller. The scanner will own it and is responsible
     * for deleting it.
     */
    XSAXMLScanner
    (
        GrammarResolver* const grammarResolver
        , XMLStringPool* const   uriStringPool
        , SchemaGrammar* const   xsaGrammar
        , MemoryManager* const   manager = XMLPlatformUtils::fgMemoryManager
    );
    friend class TraverseSchema;

    // -----------------------------------------------------------------------
    //  XMLScanner virtual methods
    // -----------------------------------------------------------------------
    virtual void scanReset(const InputSource& src);

    // -----------------------------------------------------------------------
    //  SGXMLScanner virtual methods
    // -----------------------------------------------------------------------
    virtual bool scanStartTag(bool& gotData);
    virtual void scanEndTag(bool& gotData);

private :
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XSAXMLScanner();
    XSAXMLScanner(const XSAXMLScanner&);
    XSAXMLScanner& operator=(const XSAXMLScanner&);

    // -----------------------------------------------------------------------
    //  Private helper methods
    // -----------------------------------------------------------------------
    void scanRawAttrListforNameSpaces(XMLSize_t attCount);
    void switchGrammar(const XMLCh* const newGrammarNameSpace, bool laxValidate);
};

inline const XMLCh* XSAXMLScanner::getName() const
{
    return XMLUni::fgXSAXMLScanner;
}

XERCES_CPP_NAMESPACE_END

#endif
