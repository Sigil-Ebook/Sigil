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


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/framework/LocalFileInputSource.hpp>
#include <xercesc/util/BinFileInputStream.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

/***
 *
 * Originated by Chris larsson
 *
 * Issue:
 *
 * There is an inconsistency in URI resolution in the case where the file itself is a 
 * symbolic link to another path (or the path has path segment which is a symbolic
 * link to another path). So, is the base path the directory where the symbolic link resides 
 * or the directory where the real file resides? I'm sure one could argue either way, 
 * but I think that having the base path be the directory where the symbolic link resides 
 * is more intuitive. 
 *
 * Defining it this way would then make the behavior consistent with using an absolute 
 * path as well as with the java behavior. 
 *
 * Proposal:
 *
 * The URI is resolved within the parser code, and is somewhat independant of the OS. 
 * 
 * A relative path is resolved by querying the current directory and appending the 
 * relative part onto the returned current directory string to obtain the base URI. 
 * An absolute path is simply used as the base URI. 
 * Then remove all "./" and "../" path segments using an algorithm like weavepath to obtain 
 * the resolved base URI. 
 *
 * When you need to access another file such as a dtd, use the resolved base URI and add on
 * the relative URI of the dtd file. Then resolve it using the same weavepath algorithm. 
 *
 * Note:
 *
 *   Java parser behaves differently for a path containning symbolic path segment. When 
 *   it is given an absolute path, it can locate the primary instance document, while given 
 *   relative path, it might not.
 *
 *   It is because Java parser uses URI solution where "/segment/../" is required to be removed
 *   from the resultant path if a relative URI is merged to a baseURI. While this is NOT required
 *   for an absolute URI. 
 *   
 *   So if a path segment, which is symbolic link, happen to be followed by the '/../', it is 
 *   NOT removed from the path if it is given in absolute form, and the underlying file system 
 *   will locate the file, if in relative form, that symbolic link path segment together with 
 *   '../' is removed from the resultant path, and the file system may NOT be able to locate 
 *   the file, if there is a one, it is definitely not the one expected, in fact by accident.
 *
 *   Therefore, to keep consistent with Java parser, for now, we do not apply removeDotDotSlash()
 *   for absolute path.
 *  
 ***/

// ---------------------------------------------------------------------------
//  LocalFileInputSource: Constructors and Destructor
// ---------------------------------------------------------------------------
LocalFileInputSource::LocalFileInputSource( const XMLCh* const basePath
                                          , const XMLCh* const relativePath
                                          , MemoryManager* const manager)
    : InputSource(manager)
{
    //
    //  If the relative part is really relative, then weave it together
    //  with the base path. If not, just take the relative path as the
    //  entire path.
    //
    if (XMLPlatformUtils::isRelative(relativePath, manager))
    {
        XMLCh* tmpBuf = XMLPlatformUtils::weavePaths(basePath, relativePath, manager);
        setSystemId(tmpBuf);
        manager->deallocate(tmpBuf); //delete [] tmpBuf;
    }
    else
    {
        XMLCh* tmpBuf = XMLString::replicate(relativePath, manager);
        XMLPlatformUtils::removeDotSlash(tmpBuf, manager);
        setSystemId(tmpBuf);
        manager->deallocate(tmpBuf);//delete [] tmpBuf;
    }

}

LocalFileInputSource::LocalFileInputSource(const XMLCh* const filePath,
                                           MemoryManager* const manager)
    : InputSource(manager)
{

    //
    //  If the path is relative, then complete it acording to the current
    //  working directory rules of the current platform. Else, just take
    //  it as is.
    //
    if (XMLPlatformUtils::isRelative(filePath, manager))
    {
        XMLCh* curDir = XMLPlatformUtils::getCurrentDirectory(manager);

        XMLSize_t curDirLen = XMLString::stringLen(curDir);
        XMLSize_t filePathLen = XMLString::stringLen(filePath);
        XMLCh* fullDir = (XMLCh*) manager->allocate
        (
            (curDirLen + filePathLen + 2) * sizeof(XMLCh)
        );//new XMLCh [ curDirLen + filePathLen + 2];

        XMLString::copyString(fullDir, curDir);
        fullDir[curDirLen] = chForwardSlash;
        XMLString::copyString(&fullDir[curDirLen+1], filePath);
        
        XMLPlatformUtils::removeDotSlash(fullDir, manager);
        XMLPlatformUtils::removeDotDotSlash(fullDir, manager);

        setSystemId(fullDir);

        manager->deallocate(curDir);//delete [] curDir;
        manager->deallocate(fullDir);//delete [] fullDir;
    }
     else
    {
        XMLCh* tmpBuf = XMLString::replicate(filePath, manager);
        XMLPlatformUtils::removeDotSlash(tmpBuf, manager);
        setSystemId(tmpBuf);
        manager->deallocate(tmpBuf);//delete [] tmpBuf;
    }

}

LocalFileInputSource::~LocalFileInputSource()
{
}


// ---------------------------------------------------------------------------
//  LocalFileInputSource: InputSource interface implementation
// ---------------------------------------------------------------------------
BinInputStream* LocalFileInputSource::makeStream() const
{
    BinFileInputStream* retStrm = new (getMemoryManager()) BinFileInputStream(getSystemId(), getMemoryManager());
    if (!retStrm->getIsOpen())
    {
        delete retStrm;
        return 0;
    }
    return retStrm;
}

XERCES_CPP_NAMESPACE_END

