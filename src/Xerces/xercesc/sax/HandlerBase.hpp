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
 * $Id: HandlerBase.hpp 932887 2010-04-11 13:04:59Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_HANDLERBASE_HPP)
#define XERCESC_INCLUDE_GUARD_HANDLERBASE_HPP

#include <xercesc/sax/DocumentHandler.hpp>
#include <xercesc/sax/DTDHandler.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class Locator;
class AttributeList;

/**
  * Default base class for handlers.
  *
  * <p>This class implements the default behaviour for four SAX
  * interfaces: EntityResolver, DTDHandler, DocumentHandler,
  * and ErrorHandler.</p>
  *
  * <p>Application writers can extend this class when they need to
  * implement only part of an interface; parser writers can
  * instantiate this class to provide default handlers when the
  * application has not supplied its own.</p>
  *
  * <p>Note that the use of this class is optional.</p>
  *
  * @see EntityResolver#EntityResolver
  * @see DTDHandler#DTDHandler
  * @see DocumentHandler#DocumentHandler
  * @see ErrorHandler#ErrorHandler
  */

class SAX_EXPORT HandlerBase :

    public EntityResolver, public DTDHandler, public DocumentHandler
    , public ErrorHandler
{
public:
    /** @name Default handlers for the DocumentHandler interface */
    //@{
  /**
    * Receive notification of character data inside an element.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method to take specific actions for each chunk of character data
    * (such as adding the data to a node or buffer, or printing it to
    * a file).</p>
    *
    * @param chars The characters.
    * @param length The number of characters to use from the
    *               character array.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#characters
    */
    virtual void characters
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
    );

  /**
    * Receive notification of the end of the document.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the beginning
    * of a document (such as finalising a tree or closing an output
    * file).</p>
    *
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#endDocument
    */
    virtual void endDocument();

  /**
    * Receive notification of the end of an element.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the end of
    * each element (such as finalising a tree node or writing
    * output to a file).</p>
    *
    * @param name The element type name.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#endElement
    */
    virtual void endElement(const XMLCh* const name);

  /**
    * Receive notification of ignorable whitespace in element content.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method to take specific actions for each chunk of ignorable
    * whitespace (such as adding data to a node or buffer, or printing
    * it to a file).</p>
    *
    * @param chars The whitespace characters.
    * @param length The number of characters to use from the
    *               character array.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#ignorableWhitespace
    */
    virtual void ignorableWhitespace
    (
        const   XMLCh* const    chars
        , const XMLSize_t       length
    );

  /**
    * Receive notification of a processing instruction.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions for each
    * processing instruction, such as setting status variables or
    * invoking other methods.</p>
    *
    * @param target The processing instruction target.
    * @param data The processing instruction data, or null if
    *             none is supplied.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#processingInstruction
    */
    virtual void processingInstruction
    (
        const   XMLCh* const    target
        , const XMLCh* const    data
    );

    /**
    * Reset the Document object on its reuse
    *
    * @see DocumentHandler#resetDocument
    */
    virtual void resetDocument();
    //@}

    /** @name Default implementation of DocumentHandler interface */

    //@{
  /**
    * Receive a Locator object for document events.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass if they wish to store the locator for use
    * with other document events.</p>
    *
    * @param locator A locator for all SAX document events.
    * @see DocumentHandler#setDocumentLocator
    * @see Locator
    */
    virtual void setDocumentLocator(const Locator* const locator);

  /**
    * Receive notification of the beginning of the document.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the beginning
    * of a document (such as allocating the root node of a tree or
    * creating an output file).</p>
    *
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#startDocument
    */
    virtual void startDocument();

  /**
    * Receive notification of the start of an element.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to take specific actions at the start of
    * each element (such as allocating a new tree node or writing
    * output to a file).</p>
    *
    * @param name The element type name.
    * @param attributes The specified or defaulted attributes.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see DocumentHandler#startElement
    */
    virtual void startElement
    (
        const   XMLCh* const    name
        ,       AttributeList&  attributes
    );

    //@}

    /** @name Default implementation of the EntityResolver interface. */

    //@{
  /**
    * Resolve an external entity.
    *
    * <p>Always return null, so that the parser will use the system
    * identifier provided in the XML document.  This method implements
    * the SAX default behaviour: application writers can override it
    * in a subclass to do special translations such as catalog lookups
    * or URI redirection.</p>
    *
    * @param publicId The public identifier, or null if none is
    *                 available.
    * @param systemId The system identifier provided in the XML
    *                 document.
    * @return The new input source, or null to require the
    *         default behaviour.
    *         The returned InputSource is owned by the parser which is
    *         responsible to clean up the memory.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see EntityResolver#resolveEntity
    */
    virtual InputSource* resolveEntity
    (
        const   XMLCh* const    publicId
        , const XMLCh* const    systemId
    );

    //@}

    /** @name Default implementation of the ErrorHandler interface */
    //@{
   /**
    * Receive notification of a recoverable parser error.
    *
    * <p>The default implementation does nothing.  Application writers
    * may override this method in a subclass to take specific actions
    * for each error, such as inserting the message in a log file or
    * printing it to the console.</p>
    *
    * @param exc The warning information encoded as an exception.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see ErrorHandler#warning
    * @see SAXParseException#SAXParseException
    */
    virtual void error(const SAXParseException& exc);

  /**
    * Report a fatal XML parsing error.
    *
    * <p>The default implementation throws a SAXParseException.
    * Application writers may override this method in a subclass if
    * they need to take specific actions for each fatal error (such as
    * collecting all of the errors into a single report): in any case,
    * the application must stop all regular processing when this
    * method is invoked, since the document is no longer reliable, and
    * the parser may no longer report parsing events.</p>
    *
    * @param exc The error information encoded as an exception.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see ErrorHandler#fatalError
    * @see SAXParseException#SAXParseException
    */
    virtual void fatalError(const SAXParseException& exc);

  /**
    * Receive notification of a parser warning.
    *
    * <p>The default implementation does nothing.  Application writers
    * may override this method in a subclass to take specific actions
    * for each warning, such as inserting the message in a log file or
    * printing it to the console.</p>
    *
    * @param exc The warning information encoded as an exception.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @see ErrorHandler#warning
    * @see SAXParseException#SAXParseException
    */
    virtual void warning(const SAXParseException& exc);

    /**
    * Reset the Error handler object on its reuse
    *
    * @see ErrorHandler#resetErrors
    */
    virtual void resetErrors();

    //@}


    /** @name Default implementation of DTDHandler interface. */
    //@{

  /**
    * Receive notification of a notation declaration.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass if they wish to keep track of the notations
    * declared in a document.</p>
    *
    * @param name The notation name.
    * @param publicId The notation public identifier, or null if not
    *                 available.
    * @param systemId The notation system identifier.
    * @see DTDHandler#notationDecl
    */
    virtual void notationDecl
    (
        const   XMLCh* const    name
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
    );

    /**
    * Reset the DTD object on its reuse
    *
    * @see DTDHandler#resetDocType
    */
    virtual void resetDocType();

  /**
    * Receive notification of an unparsed entity declaration.
    *
    * <p>By default, do nothing.  Application writers may override this
    * method in a subclass to keep track of the unparsed entities
    * declared in a document.</p>
    *
    * @param name The entity name.
    * @param publicId The entity public identifier, or null if not
    *                 available.
    * @param systemId The entity system identifier.
    * @param notationName The name of the associated notation.
    * @see DTDHandler#unparsedEntityDecl
    */
    virtual void unparsedEntityDecl
    (
        const   XMLCh* const    name
        , const XMLCh* const    publicId
        , const XMLCh* const    systemId
        , const XMLCh* const    notationName
    );
    //@}

    HandlerBase() {};
    virtual ~HandlerBase() {};

private:
	// -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    HandlerBase(const HandlerBase&);
    HandlerBase& operator=(const HandlerBase&);
};


// ---------------------------------------------------------------------------
//  HandlerBase: Inline default implementations
// ---------------------------------------------------------------------------
inline void HandlerBase::characters(const   XMLCh* const
                                    , const XMLSize_t)
{
}

inline void HandlerBase::endDocument()
{
}

inline void HandlerBase::endElement(const XMLCh* const)
{
}

inline void HandlerBase::error(const SAXParseException&)
{
}

inline void HandlerBase::fatalError(const SAXParseException& exc)
{
    throw exc;
}

inline void
HandlerBase::ignorableWhitespace(   const   XMLCh* const
                                    , const XMLSize_t)
{
}

inline void HandlerBase::notationDecl(  const   XMLCh* const
                                        , const XMLCh* const
                                        , const XMLCh* const)
{
}

inline void
HandlerBase::processingInstruction( const   XMLCh* const
                                    , const XMLCh* const)
{
}

inline void HandlerBase::resetErrors()
{
}

inline void HandlerBase::resetDocument()
{
}

inline void HandlerBase::resetDocType()
{
}

inline InputSource*
HandlerBase::resolveEntity( const   XMLCh* const
                            , const XMLCh* const)
{
    return 0;
}

inline void
HandlerBase::unparsedEntityDecl(const   XMLCh* const
                                , const XMLCh* const
                                , const XMLCh* const
                                , const XMLCh* const)
{
}

inline void HandlerBase::setDocumentLocator(const Locator* const)
{
}

inline void HandlerBase::startDocument()
{
}

inline void
HandlerBase::startElement(  const   XMLCh* const
                            ,       AttributeList&)
{
}

inline void HandlerBase::warning(const SAXParseException&)
{
}

XERCES_CPP_NAMESPACE_END

#endif
