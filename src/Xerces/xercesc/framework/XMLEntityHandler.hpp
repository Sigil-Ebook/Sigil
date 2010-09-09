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
 * $Id: XMLEntityHandler.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XMLENTITYHANDLER_HPP)
#define XERCESC_INCLUDE_GUARD_XMLENTITYHANDLER_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class InputSource;
class XMLBuffer;
class XMLResourceIdentifier;

/**
 *  This abstract class is a callback mechanism for the scanner. By creating
 *  a derivative of this class and plugging into the scanner, the scanner
 *  will call back on the object's methods to entity events.
 *
 *  This class is primarily for use by those writing their own parser classes.
 *  If you use the standard parser classes, DOMParser and SAXParser, you won't
 *  use this API. You will instead use a similar mechanism defined by the SAX
 *  API, called EntityResolver.
 */
class XMLPARSER_EXPORT XMLEntityHandler
{
public:
    // -----------------------------------------------------------------------
    //  Constructors are hidden, only the virtual destructor is exposed
    // -----------------------------------------------------------------------

    /** @name Destructor */
    //@{

    /**
      * Default destructor
      */
    virtual ~XMLEntityHandler()
    {
    }
    //@}


    // -----------------------------------------------------------------------
    //  The virtual entity handler interface
    // -----------------------------------------------------------------------
    /** @name The pure virtual methods in this interface. */
    //@{

    /**
      * This method get called after the scanner has finished reading from
      * the given input source while processing external entity references.
      *
      * @param inputSource The input source for the entity
      */
    virtual void endInputSource(const InputSource& inputSource) = 0;

    /**
      * This method allows the passes the scanned systemId to the entity
      * handler, thereby giving it a chance to provide any customized
      * handling like resolving relative path names. The scanner first
      * calls this method before calling <code>resolveEntity</code>.
      *
      * @param systemId The system id extracted by the scanner from the
      *                 input source.
      * @param toFill The buffer in which the fully expanded system id needs
      *               to be stored.
      */
    virtual bool expandSystemId
    (
        const   XMLCh* const    systemId
        ,       XMLBuffer&      toFill
    ) = 0;

    /**
      * This method allows the entity handler to reset itself, so that
      * it can be used again. It is called prior to a new document parse
      * operation.
      */
    virtual void resetEntities() = 0;

    /**
      * This method allows the entity handler to provide customized
      * application specific entity resolution. 
      *
      * <i>Only one resolveEntity method will be used.  If both setEntityResolver and 
      * setXMLEntityResolver are called, then the last one is used.</i>
      *
      * @param resourceIdentifier An object containing the type of
      *        resource to be resolved and the associated data members
      *        corresponding to this type.
      * @return The value returned by the resolveEntity method or
      *         NULL otherwise to indicate no processing was done.
      *         The returned InputSource is owned by the parser which is
      *         responsible to clean up the memory.
      */
    virtual InputSource* resolveEntity
    (
        XMLResourceIdentifier* resourceIdentifier
    ) = 0;

    /**
      * This method will be called before the scanner starts reading
      * from an input source while processing external entity references.
      *
      * @param inputSource The external input source.
      */
    virtual void startInputSource(const InputSource& inputSource) = 0;
    //@}


protected :
    // -----------------------------------------------------------------------
    //  Hidden Constructors
    // -----------------------------------------------------------------------
    /** @name Constructor */
    //@{

    /**
      * Protected default constructor
      */
    XMLEntityHandler()
    {
    }
    //@}



private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and destructor
    // -----------------------------------------------------------------------
    XMLEntityHandler(const XMLEntityHandler&);
    XMLEntityHandler& operator=(const XMLEntityHandler&);
};

XERCES_CPP_NAMESPACE_END

#endif
