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
 * $Id: StdOutFormatTarget.hpp 553937 2007-07-06 16:02:19Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_STDOUTFORMATTARGET_HPP)
#define XERCESC_INCLUDE_GUARD_STDOUTFORMATTARGET_HPP

#include <xercesc/framework/XMLFormatter.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLPARSER_EXPORT StdOutFormatTarget : public XMLFormatTarget {
public:

    /** @name constructors and destructor */
    //@{
    StdOutFormatTarget() ;
    ~StdOutFormatTarget();
    //@}

    // -----------------------------------------------------------------------
    //  Implementations of the format target interface
    // -----------------------------------------------------------------------
    virtual void writeChars(const XMLByte* const toWrite
                          , const XMLSize_t      count
                          , XMLFormatter* const  formatter);

    virtual void flush();

private:
    // -----------------------------------------------------------------------
    //  Unimplemented methods.
    // -----------------------------------------------------------------------
    StdOutFormatTarget(const StdOutFormatTarget&);
    StdOutFormatTarget& operator=(const StdOutFormatTarget&);
};

XERCES_CPP_NAMESPACE_END

#endif
