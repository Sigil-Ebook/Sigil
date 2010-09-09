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
 * $Id: StdInInputSource.hpp 527149 2007-04-10 14:56:39Z amassari $
 */


#if !defined(XERCESC_INCLUDE_GUARD_STDININPUTSOURCE_HPP)
#define XERCESC_INCLUDE_GUARD_STDININPUTSOURCE_HPP

#include <xercesc/sax/InputSource.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class BinInputStream;


/**
 *  This class is a derivative of the standard InputSource class. It provides
 *  for the parser access to data via the standard input. This input source
 *  is not commonly used, but can be useful when implementing such things
 *  as pipe based tools which exchange XML data.
 *
 *  As with all InputSource derivatives. The primary objective of an input
 *  source is to create an input stream via which the parser can spool in
 *  data from the referenced source.
 */
class XMLPARSER_EXPORT StdInInputSource : public InputSource
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    /** @name Constructor */
    //@{

    /**
      * Since the standard input is a canned source, the constructor is very
      * simple. It just uses local platform services to open up the standard
      * input source as file, a new handleof which it gives to each new stream
      * it creates.
      */
    StdInInputSource(MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager);
    //@}

    /** @name Destructor */
    //@{
    ~StdInInputSource();
    //@}


    // -----------------------------------------------------------------------
    //  Virtual input source interface
    // -----------------------------------------------------------------------


    /** @name Virtual methods */
    //@{

    /**
     * This method will return a binary input stream derivative that will
     * parse from the standard input of the local host.
     *
     * @return A dynamically allocated binary input stream derivative that
     *         can parse from the standardinput.
     */
    BinInputStream* makeStream() const;

    //@}

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    StdInInputSource(const StdInInputSource&);
    StdInInputSource& operator=(const StdInInputSource&);

};

inline StdInInputSource::StdInInputSource(MemoryManager* const manager) :

    InputSource("stdin", manager)
{
}

inline StdInInputSource::~StdInInputSource()
{
}

XERCES_CPP_NAMESPACE_END

#endif
