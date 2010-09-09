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
 * $Id: DOMImplementationLS.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMIMPLEMENTATIONLS_HPP)
#define XERCESC_INCLUDE_GUARD_DOMIMPLEMENTATIONLS_HPP

#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMLSParser;
class DOMLSSerializer;
class DOMLSInput;
class DOMLSOutput;
class MemoryManager;
class XMLGrammarPool;

/**
  * <p><code>DOMImplementationLS</code> contains the factory methods for
  * creating Load and Save objects.</p>
  *
  * <p>An object that implements DOMImplementationLS is obtained by doing a
  * binding specific cast from DOMImplementation to DOMImplementationLS.
  * Implementations supporting the Load and Save feature must implement the
  * DOMImplementationLS interface on whatever object implements the
  * DOMImplementation interface.</p>
  *
  * @since DOM Level 3
  */
class CDOM_EXPORT DOMImplementationLS
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
    DOMImplementationLS() {};
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
    DOMImplementationLS(const DOMImplementationLS &);
    DOMImplementationLS & operator = (const DOMImplementationLS &);
    //@}

public:
    // -----------------------------------------------------------------------
    //  All constructors are hidden, just the destructor is available
    // -----------------------------------------------------------------------
    /** @name Destructor */
    //@{
    /**
     * Destructor
     *
     */
    virtual ~DOMImplementationLS() {};
    //@}

    // -----------------------------------------------------------------------
    //  Public constants
    // -----------------------------------------------------------------------
    /** @name Public constants */
    //@{
    /**
     * Create a synchronous or an asynchronous <code>DOMLSParser</code>.
     * @see createLSParser(const DOMImplementationLSMode mode, const XMLCh* const schemaType)
     * @since DOM Level 3
     *
     */
    enum DOMImplementationLSMode
    {
        MODE_SYNCHRONOUS = 1,
        MODE_ASYNCHRONOUS = 2
    };
    //@}

    // -----------------------------------------------------------------------
    // Virtual DOMImplementationLS interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
    // -----------------------------------------------------------------------
    //  Factory create methods
    // -----------------------------------------------------------------------
    /**
     * Create a new DOMLSParser. The newly constructed parser may then be configured
     * by means of its DOMConfiguration object, and used to parse documents by
     * means of its parse method.
     *
     * @param mode      The mode argument is either <code>MODE_SYNCHRONOUS</code>
     * or <code>MODE_ASYNCHRONOUS</code>, if mode is <code>MODE_SYNCHRONOUS</code>
     * then the <code>DOMLSParser</code> that is created will operate in synchronous
     * mode, if it's <code>MODE_ASYNCHRONOUS</code> then the <code>DOMLSParser</code>
     * that is created will operate in asynchronous mode.
     * @param schemaType An absolute URI representing the type of the schema
     * language used during the load of a <code>DOMDocument</code> using the newly
     * created <code>DOMLSParser</code>. Note that no lexical checking is done on
     * the absolute URI. In order to create a <code>DOMLSParser</code> for any kind
     * of schema types (i.e. the <code>DOMLSParser</code> will be free to use any
     * schema found), use the value <code>NULL</code>.
     * <b>Note</b>: For W3C XML Schema [XML Schema Part 1], applications must use
     * the value "http://www.w3.org/2001/XMLSchema". For XML DTD [XML 1.0],
     * applications must use the value "http://www.w3.org/TR/REC-xml".
     * Other Schema languages are outside the scope of the W3C and therefore should
     * recommend an absolute URI in order to use this method.
     * @param manager    Pointer to the memory manager to be used to allocate objects.
     * @param gramPool   The collection of cached grammars.
     * @return The newly created <code>DOMLSParser</code> object. This
     * <code>DOMLSParser</code> is either synchronous or asynchronous depending
     * on the value of the <code>mode</code> argument.
     * @exception DOMException NOT_SUPPORTED_ERR: Raised if the requested mode
     * or schema type is not supported.
     *
     * @see DOMLSParser
     * @since DOM Level 3
     */
    virtual DOMLSParser* createLSParser(const DOMImplementationLSMode mode,
                                        const XMLCh* const     schemaType,
                                        MemoryManager* const   manager = XMLPlatformUtils::fgMemoryManager,
                                        XMLGrammarPool*  const gramPool = 0) = 0;


    /**
     * Create a new DOMLSSerializer. DOMLSSerializer is used to serialize a DOM tree
     * back into an XML document.
     *
     * @return The newly created <code>DOMLSSerializer</code> object.
     *
     * @see DOMLSSerializer
     * @since DOM Level 3
     */
    virtual DOMLSSerializer* createLSSerializer(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) = 0;

    /**
     * Create a new "empty" DOMLSInput.
     *
     * @return The newly created <code>DOMLSInput</code> object.
     *
     * @see DOMLSInput
     * @since DOM Level 3
     */
    virtual DOMLSInput* createLSInput(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) = 0;

    /**
     * Create a new "empty" LSOutput.
     *
     * @return The newly created <code>LSOutput</code> object.
     *
     * @see LSOutput
     * @since DOM Level 3
     */
    virtual DOMLSOutput* createLSOutput(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) = 0;
    //@}
};


XERCES_CPP_NAMESPACE_END

#endif
