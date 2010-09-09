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
 * $Id: PanicHandler.cpp 471747 2006-11-06 14:31:56Z amassari $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/PanicHandler.hpp>

XERCES_CPP_NAMESPACE_BEGIN

const char* PanicHandler::getPanicReasonString(const PanicReasons reason)
{
    const char* reasonStr;

    switch (reason)
    {
    case Panic_NoTransService:
        reasonStr = "Could not load a transcoding service";
        break;
    case Panic_NoDefTranscoder:
        reasonStr = "Could not load a local code page transcoder";
        break;
    case Panic_CantFindLib:
        reasonStr = "Could not find the xerces-c DLL";
        break;
    case Panic_UnknownMsgDomain:
        reasonStr = "Unknown message domain";
        break;
    case Panic_CantLoadMsgDomain:
        reasonStr = "Cannot load message domain";
        break;
    case Panic_SynchronizationErr:
        reasonStr = "Cannot synchronize system or mutex";
        break;
    case Panic_SystemInit:
        reasonStr = "Cannot initialize the system or mutex";
        break;
    case Panic_MutexErr:
        reasonStr = "Cannot create, lock or unlock a mutex";
        break;
    default:
        reasonStr = "Unknown reason";
        break;
    }	

    return reasonStr;

}

XERCES_CPP_NAMESPACE_END

