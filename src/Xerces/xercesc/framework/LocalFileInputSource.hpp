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
 * $Id: LocalFileInputSource.hpp 527149 2007-04-10 14:56:39Z amassari $
 */


#if !defined(XERCESC_INCLUDE_GUARD_LOCALFILEINPUTSOURCE_HPP)
#define XERCESC_INCLUDE_GUARD_LOCALFILEINPUTSOURCE_HPP

#include <xercesc/sax/InputSource.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class BinInputStream;

/**
 *  This class is a derivative of the standard InputSource class. It provides
 *  for the parser access to data which is referenced via a local file path,
 *  as apposed to remote file or URL. This is the most efficacious mechanism
 *  by which local files can be parsed, since the parse knows that it refers
 *  to a local file and will make no other attempts to interpret the passed
 *  path.
 *
 *  The path provided can either be a fully qualified path or a relative path.
 *  If relative, it will be completed either relative to a passed base path
 *  or relative to the current working directory of the process.
 *
 *  As with all InputSource derivatives. The primary objective of an input
 *  source is to create an input stream via which the parser can spool in
 *  data from the referenced source.
 */
class XMLPARSER_EXPORT LocalFileInputSource : public InputSource
{
public :
    // -----------------------------------------------------------------------
    //  Constructors and Destructor
    // -----------------------------------------------------------------------

    /** @name Constructors */
    //@{

    /**
      * A local file input source requires a path to the file to load. This
      * can be provided either as a fully qualified path, a path relative to
      * the current working directly, or a path relative to a provided base
      * path.
      *
      * The completed path will become the system id of this input source.
      * The constructors don't take any public id for local files, but you
      * still set them via the parent class' setPublicId() method of course.
      *
      * This constructor takes an explicit base path and a possibly relative
      * path. If the relative path is seen to be fully qualified, it is used
      * as is. Otherwise, it is made relative to the passed base path.
      *
      * @param  basePath    The base path from which the passed relative path
      *                     will be based, if the relative part is indeed
      *                     relative.
      *
      * @param  relativePath    The relative part of the path. It can actually
      *                         be fully qualified, in which case it is taken
      *                         as is.
      *
      * @param  manager    Pointer to the memory manager to be used to
      *                    allocate objects.
      *
      * @exception XMLException If the path is relative and doesn't properly
      *            resolve to a file.
      */
    LocalFileInputSource
    (
        const   XMLCh* const   basePath
        , const XMLCh* const   relativePath
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );

    /**
      * This constructor takes a single parameter which is the fully qualified
      * or relative path. If it is fully qualified, it is taken as is. If it is
      * relative, then it is completed relative to the current working directory
      * (or the equivalent on the local host machine.)
      *
      * The completed path will become the system id of this input source.
      * The constructors don't take any public id for local files, but you
      * still set them via the parent class' setPublicId() method of course.
      *
      * @param  filePath    The relative or fully qualified path.
      *
      * @param  manager     Pointer to the memory manager to be used to
      *                     allocate objects.
      *
      * @exception XMLException If the path is relative and doesn't properly
      *            resolve to a file.
      */
    LocalFileInputSource
    (
        const   XMLCh* const   filePath
        , MemoryManager* const manager = XMLPlatformUtils::fgMemoryManager
    );
    //@}

    /** @name Destructor */
    //@{
    ~LocalFileInputSource();
    //@}


    // -----------------------------------------------------------------------
    //  Virtual input source interface
    // -----------------------------------------------------------------------

    /** @name Virtual methods */
    //@{

    /**
    * This method will return a binary input stream derivative that will
    * parse from the local file indicatedby the system id.
    *
    * @return A dynamically allocated binary input stream derivative that
    *         can parse from the file indicated by the system id.
    */
    virtual BinInputStream* makeStream() const;

    //@}
private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    LocalFileInputSource(const LocalFileInputSource&);
    LocalFileInputSource& operator=(const LocalFileInputSource&);

};

XERCES_CPP_NAMESPACE_END

#endif
