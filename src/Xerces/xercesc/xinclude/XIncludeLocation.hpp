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
 * $Id: XIncludeLocation.hpp 655706 2008-05-13 01:08:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XINCLUDELOCATION_HPP)
#define XERCESC_INCLUDE_GUARD_XINCLUDELOCATION_HPP

#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOM.hpp>

XERCES_CPP_NAMESPACE_BEGIN


/**
  * Class for representing and manipulating the XMLCh * href's used
  * by an xi:include element.
  *
  * This class is designed primarily for internal use. This class implements
  * the functionality required to calculate relative hrefs and the base URI
  * fixups required for performing XInclude functionality.
  */
class XINCLUDE_EXPORT XIncludeLocation
{
public:
     /** Create an XIncludeLocation, primed with the supplied href
      *
      * @param href the initial URI value
      *
      * @return nothing
      */
    XIncludeLocation(const XMLCh *href);

     /** Destructor
      *
      * @return nothing
      */
    ~XIncludeLocation();

     /** Prepend the supplied href to the current location and modify the current XIncludeLocation's
      * internal href field
      *
      * @param toPrepend the path to prepend
      *
      * @return the resultant compound URI
      */
    const XMLCh *prependPath(const XMLCh *toPrepend);

     /** Get the current XIncludeLocation's compound URI location
      *
      * @return the current URI
      */
    const XMLCh *getLocation(){
        return fHref;
    };

     /** Get a pointer to the end of the protocol section of a URI
      *
      * @param URI a URI to strip the protocol from
      *
      * @return a pointer into the supplied URI immediately after the last character of the protocol section
      *            the pointer points to the first character after the protocol.
      */
    static const XMLCh *findEndOfProtocol(const XMLCh *URI);

private:
    const XMLCh *fHref;
};

XERCES_CPP_NAMESPACE_END

#endif /* XINCLUDELOCATION_HPP */

