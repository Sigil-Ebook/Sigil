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
 * $Id: XMLEntityResolver.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLENTITYRESOLVER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLENTITYRESOLVER_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/XMemory.hpp>
#include <xercesc/util/XMLResourceIdentifier.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class InputSource;

/**
  * Revised interface for resolving entities.
  *
  * <p>If an application needs to implement customized handling
  * for external entities, it can implement this interface and
  * register an instance with the parser using the parser's
  * setXMLEntityResolver method or it can use the basic SAX interface 
  * (EntityResolver).  The difference between the two interfaces is
  * the arguments to the resolveEntity() method.  With the SAX
  * EntityResolve the arguments are systemId and publicId.  With this
  * interface the argument is a XMLResourceIdentifier object.  <i>Only
  * one EntityResolver can be set using setEntityResolver() or 
  * setXMLEntityResolver, if both are set the last one set is 
  * used.</i></p>
  *
  * <p>The parser will then allow the application to intercept any
  * external entities (including the external DTD subset and external
  * parameter entities, if any) before including them.</p>
  *
  * <p>Many applications will not need to implement this interface,
  * but it will be especially useful for applications that build
  * XML documents from databases or other specialised input sources,
  * or for applications that use URI types other than URLs.</p>
  *
  * <p>The following resolver would provide the application
  * with a special character stream for the entity with the system
  * identifier "http://www.myhost.com/today":</p>
  *
  *<code>
  * \#include <xercesc/util/XMLEntityResolver.hpp><br>
  * \#include <xercesc/sax/InputSource.hpp><br>
  *<br>
  *&nbsp;class MyResolver : public XMLEntityResolver {<br>
  *&nbsp;&nbsp;public:<br>
  *&nbsp;&nbsp;&nbsp;InputSource resolveEntity (XMLResourceIdentifier* xmlri);<br>
  *&nbsp;&nbsp;&nbsp;...<br>
  *&nbsp;&nbsp;};<br>
  *<br>
  *&nbsp;MyResolver::resolveEntity(XMLResourceIdentifier* xmlri) {<br>
  *&nbsp;&nbsp;switch(xmlri->getResourceIdentifierType()) {<br>
  *&nbsp;&nbsp;&nbsp;case XMLResourceIdentifier::SystemId:<br>
  *&nbsp;&nbsp;&nbsp;&nbsp;if (XMLString::compareString(xmlri->getSystemId(), "http://www.myhost.com/today")) {<br>
  *&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;MyReader* reader = new MyReader();<br>
  *&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return new InputSource(reader);<br>
  *&nbsp;&nbsp;&nbsp;&nbsp;} else {<br>
  *&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return null;<br>
  *&nbsp;&nbsp;&nbsp;&nbsp;}<br>
  *&nbsp;&nbsp;&nbsp;&nbsp;break;<br>
  *&nbsp;&nbsp;&nbsp;default:<br>
  *&nbsp;&nbsp;&nbsp;&nbsp;return null;<br>
  *&nbsp;&nbsp;}<br>
  *&nbsp;}</code>
  *
  * <p>The application can also use this interface to redirect system
  * identifiers to local URIs or to look up replacements in a catalog
  * (possibly by using the public identifier).</p>
  *
  * <p>The HandlerBase class implements the default behaviour for
  * this interface, which is simply always to return null (to request
  * that the parser use the default system identifier).</p>
  *
  * @see XMLResourceIdentifier
  * @see Parser#setXMLEntityResolver
  * @see InputSource#InputSource
  * @see HandlerBase#HandlerBase
  */
class XMLUTIL_EXPORT XMLEntityResolver
{
public:
    /** @name Constructors and Destructor */
    //@{


    /** Destructor */
    virtual ~XMLEntityResolver()
    {
    }

    //@}

    /** @name The XMLEntityResolver interface */
    //@{

  /**
    * Allow the application to resolve external entities.
    *
    * <p>The Parser will call this method before opening any external
    * entity except the top-level document entity (including the
    * external DTD subset, external entities referenced within the
    * DTD, and external entities referenced within the document
    * element): the application may request that the parser resolve
    * the entity itself, that it use an alternative URI, or that it
    * use an entirely different input source.</p>
    *
    * <p>Application writers can use this method to redirect external
    * system identifiers to secure and/or local URIs, to look up
    * public identifiers in a catalogue, or to read an entity from a
    * database or other input source (including, for example, a dialog
    * box).</p>
    *
    * <p>If the system identifier is a URL, the SAX parser must
    * resolve it fully before reporting it to the application.</p>
    *
    * @param resourceIdentifier An object containing the type of
    *        resource to be resolved and the associated data members
    *        corresponding to this type.
    * @return An InputSource object describing the new input source,
    *         or null to request that the parser open a regular
    *         URI connection to the system identifier.
    *         The returned InputSource is owned by the parser which is
    *         responsible to clean up the memory.
    * @exception SAXException Any SAX exception, possibly
    *            wrapping another exception.
    * @exception IOException An IO exception,
    *            possibly the result of creating a new InputStream
    *            or Reader for the InputSource.
    *
    * @see InputSource#InputSource
    * @see XMLResourceIdentifier
    */
    virtual InputSource* resolveEntity
    (
        XMLResourceIdentifier* resourceIdentifier
    ) = 0;

    //@}
protected: 
    /** Default Constructor */
    XMLEntityResolver()
    {
    }

private :
    /* Unimplemented constructors and operators */

    /* Copy constructor */
    XMLEntityResolver(const XMLEntityResolver&);

    /* Assignment operator */
    XMLEntityResolver& operator=(const XMLEntityResolver&);

};

XERCES_CPP_NAMESPACE_END

#endif
