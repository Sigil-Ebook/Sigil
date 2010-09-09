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
 * $Id: DOMImplementation.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_DOMIMPLEMENTATION_HPP)
#define XERCESC_INCLUDE_GUARD_DOMIMPLEMENTATION_HPP

#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMLSException.hpp>
#include <xercesc/dom/DOMRangeException.hpp>
#include <xercesc/util/PlatformUtils.hpp>

XERCES_CPP_NAMESPACE_BEGIN


class DOMDocument;
class DOMDocumentType;

/**
 * The <code>DOMImplementation</code> interface provides a number of methods
 * for performing operations that are independent of any particular instance
 * of the document object model.
 */

class CDOM_EXPORT DOMImplementation : public DOMImplementationLS
{
protected:
    // -----------------------------------------------------------------------
    //  Hidden constructors
    // -----------------------------------------------------------------------
    /** @name Hidden constructors */
    //@{
        DOMImplementation() {};                                      // no plain constructor
    //@}

private:
    // -----------------------------------------------------------------------
    // Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    /** @name Unimplemented constructors and operators */
    //@{
        DOMImplementation(const DOMImplementation &);   // no copy constructor.
        DOMImplementation & operator = (const DOMImplementation &);  // No Assignment
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
    virtual ~DOMImplementation() {};
    //@}

    // -----------------------------------------------------------------------
    // Virtual DOMImplementation interface
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 1 */
    //@{
    /**
     * Test if the DOM implementation implements a specific feature.
     * @param feature The name of the feature to test (case-insensitive). The
     *   values used by DOM features are defined throughout the DOM Level 2
     *   specifications and listed in the  section. The name must be an XML
     *   name. To avoid possible conflicts, as a convention, names referring
     *   to features defined outside the DOM specification should be made
     *   unique.
     * @param version This is the version number of the feature to test. In
     *   Level 2, the string can be either "2.0" or "1.0". If the version is
     *   not specified, supporting any version of the feature causes the
     *   method to return <code>true</code>.
     * @return <code>true</code> if the feature is implemented in the
     *   specified version, <code>false</code> otherwise.
     * @since DOM Level 1
     */
    virtual bool  hasFeature(const XMLCh *feature,  const XMLCh *version) const = 0;
    //@}

    // -----------------------------------------------------------------------
    // Functions introduced in DOM Level 2
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 2 */
    //@{
    /**
     * Creates an empty <code>DOMDocumentType</code> node. Entity declarations
     * and notations are not made available. Entity reference expansions and
     * default attribute additions do not occur. It is expected that a
     * future version of the DOM will provide a way for populating a
     * <code>DOMDocumentType</code>.
     * @param qualifiedName The qualified name of the document type to be
     *   created.
     * @param publicId The external subset public identifier.
     * @param systemId The external subset system identifier.
     * @return A new <code>DOMDocumentType</code> node with
     *   <code>ownerDocument</code> set to <code>null</code>.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified qualified name
     *   contains an illegal character.
     *   <br>NAMESPACE_ERR: Raised if the <code>qualifiedName</code> is
     *   malformed.
     *   <br>NOT_SUPPORTED_ERR: May be raised by DOM implementations which do
     *   not support the <code>"XML"</code> feature, if they choose not to
     *   support this method. Other features introduced in the future, by
     *   the DOM WG or in extensions defined by other groups, may also
     *   demand support for this method; please consult the definition of
     *   the feature to see if it requires this method.
     * @since DOM Level 2
     */
    virtual  DOMDocumentType *createDocumentType(const XMLCh *qualifiedName,
                                                 const XMLCh *publicId,
                                                 const XMLCh *systemId) = 0;

    /**
     * Creates a DOMDocument object of the specified type with its document
     * element.
     * @param namespaceURI The namespace URI of the document element to
     *   create.
     * @param qualifiedName The qualified name of the document element to be
     *   created.
     * @param doctype The type of document to be created or <code>null</code>.
     *   When <code>doctype</code> is not <code>null</code>, its
     *   <code>ownerDocument</code> attribute is set to the document
     *   being created.
     * @param manager    Pointer to the memory manager to be used to
     *                   allocate objects.
     * @return A new <code>DOMDocument</code> object.
     * @exception DOMException
     *   INVALID_CHARACTER_ERR: Raised if the specified qualified name
     *   contains an illegal character.
     *   <br>NAMESPACE_ERR: Raised if the <code>qualifiedName</code> is
     *   malformed, if the <code>qualifiedName</code> has a prefix and the
     *   <code>namespaceURI</code> is <code>null</code>, or if the
     *   <code>qualifiedName</code> has a prefix that is "xml" and the
     *   <code>namespaceURI</code> is different from "
     *   http://www.w3.org/XML/1998/namespace" , or if the DOM
     *   implementation does not support the <code>"XML"</code> feature but
     *   a non-null namespace URI was provided, since namespaces were
     *   defined by XML.
     *   <br>WRONG_DOCUMENT_ERR: Raised if <code>doctype</code> has already
     *   been used with a different document or was created from a different
     *   implementation.
     *   <br>NOT_SUPPORTED_ERR: May be raised by DOM implementations which do
     *   not support the "XML" feature, if they choose not to support this
     *   method. Other features introduced in the future, by the DOM WG or
     *   in extensions defined by other groups, may also demand support for
     *   this method; please consult the definition of the feature to see if
     *   it requires this method.
     * @since DOM Level 2
     */

    virtual DOMDocument *createDocument(const XMLCh *namespaceURI,
                                        const XMLCh *qualifiedName,
                                        DOMDocumentType *doctype,
                                        MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) = 0;

    //@}
    // -----------------------------------------------------------------------
    // Functions introduced in DOM Level 3
    // -----------------------------------------------------------------------
    /** @name Functions introduced in DOM Level 3 */
    //@{
    /**
     * This method returns a specialized object which implements the specialized APIs
     * of the specified feature and version, as specified in DOM Features.
     * This method also allow the implementation to provide specialized objects which
     * do not support the <code>DOMImplementation</code> interface.
     *
     * @param feature The name of the feature requested (case-insensitive).
     *        Note that any plus sign "+" prepended to the name of the feature will
     *        be ignored since it is not significant in the context of this method.
     * @param version This is the version number of the feature to test.
     * @return Returns an object which implements the specialized APIs of the specified
     *         feature and version, if any, or null if there is no object which implements
     *         interfaces associated with that feature.
     * @since DOM Level 3
     */
    virtual void* getFeature(const XMLCh* feature, const XMLCh* version) const = 0;

    //@}

    // -----------------------------------------------------------------------
    // Non-standard extension
    // -----------------------------------------------------------------------
    /** @name Non-standard extension */
    //@{
    /**
     * Non-standard extension
     *
     * Create a completely empty document that has neither a root element or a doctype node.
     */
    virtual DOMDocument *createDocument(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) = 0;

    /**
     * Non-standard extension
     *
     *  Factory method for getting a DOMImplementation object.
     *     The DOM implementation retains ownership of the returned object.
     *     Application code should NOT delete it.
     */
    static DOMImplementation *getImplementation();

    /**
     * Non-standard extension
     *
     * Load the default error text message for DOMException.
     * @param msgToLoad The DOM ExceptionCode id to be processed
     * @param toFill    The buffer that will hold the output on return. The
     *         size of this buffer should at least be 'maxChars + 1'.
     * @param maxChars  The maximum number of output characters that can be
     *         accepted. If the result will not fit, it is an error.
     * @return <code>true</code> if the message is successfully loaded
     */
    static bool loadDOMExceptionMsg
    (
          const   short  msgToLoad
        ,       XMLCh* const                 toFill
        , const XMLSize_t                    maxChars
    );

    //@}

};

XERCES_CPP_NAMESPACE_END

#endif
