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
 * $Id: SAXException.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_SAXEXCEPTION_HPP)
#define XERCESC_INCLUDE_GUARD_SAXEXCEPTION_HPP

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/util/XMemory.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
  * Encapsulate a general SAX error or warning.
  *
  * <p>This class can contain basic error or warning information from
  * either the XML SAX parser or the application: a parser writer or
  * application writer can subclass it to provide additional
  * functionality.  SAX handlers may throw this exception or
  * any exception subclassed from it.</p>
  *
  * <p>If the application needs to pass through other types of
  * exceptions, it must wrap those exceptions in a SAXException
  * or an exception derived from a SAXException.</p>
  *
  * <p>If the parser or application needs to include information
  * about a specific location in an XML document, it should use the
  * SAXParseException subclass.</p>
  *
  * @see SAXParseException#SAXParseException
  */
class SAX_EXPORT SAXException : public XMemory
{
public:
    /** @name Constructors and Destructor */
    //@{
    /** Default constructor
     * @param manager    Pointer to the memory manager to be used to
     *                   allocate objects.
     */
    SAXException(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) :

        fMsg(XMLString::replicate(XMLUni::fgZeroLenString, manager))
        , fMemoryManager(manager)
    {
    }

  /**
    * Create a new SAXException.
    *
    * @param msg The error or warning message.
    * @param manager    Pointer to the memory manager to be used to
    *                   allocate objects.
    */
    SAXException(const XMLCh* const msg,
                 MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) :

        fMsg(XMLString::replicate(msg, manager))
        , fMemoryManager(manager)
    {
    }

  /**
    * Create a new SAXException.
    *
    * @param msg The error or warning message.
    * @param manager    Pointer to the memory manager to be used to
    *                   allocate objects.
    */
    SAXException(const char* const msg,
                 MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager) :

        fMsg(XMLString::transcode(msg, manager))
        , fMemoryManager(manager)
    {
    }

  /**
    * Copy constructor
    *
    * @param toCopy The exception to be copy constructed
    */
    SAXException(const SAXException& toCopy) :
        XMemory(toCopy)
        , fMsg(XMLString::replicate(toCopy.fMsg, toCopy.fMemoryManager))
        , fMemoryManager(toCopy.fMemoryManager)
    {
    }

    /** Destructor */
    virtual ~SAXException()
    {
        fMemoryManager->deallocate(fMsg);//delete [] fMsg;
    }

    //@}


    /** @name Public Operators */
    //@{
    /**
      * Assignment operator
      *
      * @param toCopy The object to be copied
      */
    SAXException& operator=(const SAXException& toCopy)
    {
        if (this == &toCopy)
            return *this;

        fMemoryManager->deallocate(fMsg);//delete [] fMsg;
        fMsg = XMLString::replicate(toCopy.fMsg, toCopy.fMemoryManager);
        fMemoryManager = toCopy.fMemoryManager;
        return *this;
    }
    //@}

    /** @name Getter Methods */
    //@{
    /**
      * Get the contents of the message
      *
      */
    virtual const XMLCh* getMessage() const
    {
        return fMsg;
    }
    //@}


protected :
    // -----------------------------------------------------------------------
    //  Protected data members
    //
    //  fMsg
    //      This is the text of the error that is being thrown.
    // -----------------------------------------------------------------------
    XMLCh*  fMsg;
    MemoryManager* fMemoryManager;
};

class SAX_EXPORT SAXNotSupportedException : public SAXException
{

public:
	SAXNotSupportedException(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

  /**
    * Create a new SAXException.
    *
    * @param msg The error or warning message.
    * @param manager    Pointer to the memory manager to be used to
    *                   allocate objects.
    */
    SAXNotSupportedException(const XMLCh* const msg,
                             MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

  /**
    * Create a new SAXException.
    *
    * @param msg The error or warning message.
    * @param manager    Pointer to the memory manager to be used to
    *                   allocate objects.
    */
    SAXNotSupportedException(const char* const msg,
                             MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

  /**
    * Copy constructor
    *
    * @param toCopy The exception to be copy constructed
    */
    SAXNotSupportedException(const SAXException& toCopy);
};

class SAX_EXPORT SAXNotRecognizedException : public SAXException
{
public:
	SAXNotRecognizedException(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

  /**
    * Create a new SAXException.
    *
    * @param msg The error or warning message.
    * @param manager    Pointer to the memory manager to be used to
    *                   allocate objects.
    */
    SAXNotRecognizedException(const XMLCh* const msg,
                              MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

  /**
    * Create a new SAXException.
    *
    * @param msg The error or warning message.
    * @param manager    Pointer to the memory manager to be used to
    *                   allocate objects.
    */
    SAXNotRecognizedException(const char* const msg,
                              MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);

  /**
    * Copy constructor
    *
    * @param toCopy The exception to be copy constructed
    */
    SAXNotRecognizedException(const SAXException& toCopy);
};

XERCES_CPP_NAMESPACE_END

#endif
