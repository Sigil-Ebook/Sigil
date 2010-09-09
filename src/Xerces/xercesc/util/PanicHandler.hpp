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
 * $Id: PanicHandler.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_PANICHANDLER_HPP)
#define XERCESC_INCLUDE_GUARD_PANICHANDLER_HPP

#include <xercesc/util/XMemory.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/**
  * Receive notification of panic.
  *
  * <p>This is the interface, through which the Xercesc reports
  *    a panic to the application. 
  * </p>
  *
  * <p>Application may implement this interface, instantiate an
  *    object of the derivative, and plug it to Xercesc in the
  *    invocation to XMLPlatformUtils::Initialize(), if it prefers 
  *    to handling panic itself rather than Xercesc doing it.
  * </p>
  *
  */

class XMLUTIL_EXPORT PanicHandler
{
public:

    /** @name Public Types */
    //@{
    enum PanicReasons
    {
          Panic_NoTransService
        , Panic_NoDefTranscoder
        , Panic_CantFindLib
        , Panic_UnknownMsgDomain
        , Panic_CantLoadMsgDomain
        , Panic_SynchronizationErr
        , Panic_SystemInit
        , Panic_AllStaticInitErr
        , Panic_MutexErr
        , PanicReasons_Count
    };
    //@}

protected:

    /** @name hidden Constructors */
    //@{
    /** Default constructor */
    PanicHandler(){};

public:

    /** Destructor */
    virtual ~PanicHandler(){};   
    //@}

    /** @name The virtual panic handler interface */
    //@{
   /**
    * Receive notification of panic
    *
    * This method is called when an unrecoverable error has occurred in the Xerces library.  
    *
    * This method must not return normally, otherwise, the results are undefined. 
    * 
    * Ways of handling this call could include throwing an exception or exiting the process.
    *
    * Once this method has been called, the results of calling any other Xerces API, 
    * or using any existing Xerces objects are undefined.    
    *
    * @param reason The reason of panic
    *
    */
    virtual void panic(const PanicHandler::PanicReasons reason) = 0;
    //@}

    static const char* getPanicReasonString(const PanicHandler::PanicReasons reason);
    
private:

    /* Unimplemented Constructors and operators */
    /* Copy constructor */
    PanicHandler(const PanicHandler&);
    
    /** Assignment operator */
    PanicHandler& operator=(const PanicHandler&);
};

XERCES_CPP_NAMESPACE_END

#endif
