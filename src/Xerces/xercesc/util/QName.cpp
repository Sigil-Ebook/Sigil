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
 * $Id: QName.cpp 810580 2009-09-02 15:52:22Z amassari $
 */

#include <xercesc/util/Janitor.hpp>
#include <xercesc/util/QName.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  QName: Constructors and Destructor
// ---------------------------------------------------------------------------
QName::QName(MemoryManager* const manager)
:fPrefixBufSz(0)
,fLocalPartBufSz(0)
,fRawNameBufSz(0)
,fURIId(0)
,fPrefix(0)
,fLocalPart(0)
,fRawName(0)
,fMemoryManager(manager)
{
}

typedef JanitorMemFunCall<QName>    CleanupType;

QName::QName( const XMLCh* const   prefix
            , const XMLCh* const   localPart
            , const unsigned int   uriId
            , MemoryManager* const manager)
:fPrefixBufSz(0)
,fLocalPartBufSz(0)
,fRawNameBufSz(0)
,fURIId(0)
,fPrefix(0)
,fLocalPart(0)
,fRawName(0)
,fMemoryManager(manager)
{
    CleanupType cleanup(this, &QName::cleanUp);

    try
    {
        //
        //  Just call the local setters to set up everything. Too much
        //  work is required to replicate that functionality here.
        //
        setName(prefix, localPart, uriId);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

QName::QName( const XMLCh* const rawName
            , const unsigned int uriId
            , MemoryManager* const manager)
:fPrefixBufSz(0)
,fLocalPartBufSz(0)
,fRawNameBufSz(0)
,fURIId(0)
,fPrefix(0)
,fLocalPart(0)
,fRawName(0)
,fMemoryManager(manager)
{
    CleanupType cleanup(this, &QName::cleanUp);

    try
    {
        //
        //  Just call the local setters to set up everything. Too much
        //  work is required to replicate that functionality here.
        //
        setName(rawName, uriId);
    }
    catch(const OutOfMemoryException&)
    {
        cleanup.release();

        throw;
    }

    cleanup.release();
}

QName::~QName()
{
	cleanUp();
}

// ---------------------------------------------------------------------------
//  QName: Copy Constructors
// ---------------------------------------------------------------------------
QName::QName(const QName& qname)
:XSerializable(qname)
,XMemory(qname)
,fPrefixBufSz(0)
,fLocalPartBufSz(0)
,fRawNameBufSz(0)
,fURIId(0)
,fPrefix(0)
,fLocalPart(0)
,fRawName(0)
,fMemoryManager(qname.fMemoryManager)
{
    XMLSize_t newLen;

    newLen = XMLString::stringLen(qname.getLocalPart());
    fLocalPartBufSz = newLen + 8;
    fLocalPart = (XMLCh*) fMemoryManager->allocate
    (
        (fLocalPartBufSz + 1) * sizeof(XMLCh)
    ); //new XMLCh[fLocalPartBufSz + 1];
    XMLString::moveChars(fLocalPart, qname.getLocalPart(), newLen + 1);

    newLen = XMLString::stringLen(qname.getPrefix());
    fPrefixBufSz = newLen + 8;
    fPrefix = (XMLCh*) fMemoryManager->allocate
    (
        (fPrefixBufSz + 1) * sizeof(XMLCh)
    ); //new XMLCh[fPrefixBufSz + 1];
    XMLString::moveChars(fPrefix, qname.getPrefix(), newLen + 1);

    fURIId = qname.getURI();
}

// ---------------------------------------------------------------------------
//  QName: Getter methods
// ---------------------------------------------------------------------------
const XMLCh* QName::getRawName() const
{
    //
    //  If there is no buffer, or if there is but we've not faulted in the
    //  value yet, then we have to do that now.
    //
    if (!fRawName || !*fRawName)
    {
        //
        //  If we have a prefix, then do the prefix:name version. Else, its
        //  just the name.
        //
        if (*fPrefix)
        {
            //
            //  Calculate the worst case size buffer we will need. We use the
            //  current high water marks of the prefix and name buffers, so it
            //  might be a little wasteful of memory but we don't have to do
            //  string len operations on the two strings.
            //
            const XMLSize_t neededLen = fPrefixBufSz + fLocalPartBufSz + 1;

            //
            //  If no buffer, or the current one is too small, then allocate one
            //  and get rid of any old one.
            //
            if (!fRawName || (neededLen > fRawNameBufSz))
            {
                fMemoryManager->deallocate(fRawName); //delete [] fRawName;

                ((QName*)this)->fRawName = 0;
                // We have to cast off the const'ness to do this
                ((QName*)this)->fRawNameBufSz = neededLen;
                ((QName*)this)->fRawName = (XMLCh*) fMemoryManager->allocate
                (
                    (neededLen + 1) * sizeof(XMLCh)
                ); //new XMLCh[neededLen + 1];

                // Make sure its initially empty
                *fRawName = 0;
            }

            const XMLSize_t prefixLen = XMLString::stringLen(fPrefix);

            XMLString::moveChars(fRawName, fPrefix, prefixLen);
            fRawName[prefixLen] = chColon;
            XMLString::copyString(&fRawName[prefixLen+1], fLocalPart);
        }
         else
        {
            return fLocalPart;
        }
    }
    return fRawName;
}

XMLCh* QName::getRawName()
{
    //
    //  If there is no buffer, or if there is but we've not faulted in the
    //  value yet, then we have to do that now.
    //
    if (!fRawName || !*fRawName)
    {
        //
        //  If we have a prefix, then do the prefix:name version. Else, its
        //  just the name.
        //
        if (*fPrefix)
        {
            //
            //  Calculate the worst case size buffer we will need. We use the
            //  current high water marks of the prefix and name buffers, so it
            //  might be a little wasteful of memory but we don't have to do
            //  string len operations on the two strings.
            //
            const XMLSize_t neededLen = fPrefixBufSz + fLocalPartBufSz + 1;

            //
            //  If no buffer, or the current one is too small, then allocate one
            //  and get rid of any old one.
            //
            if (!fRawName || (neededLen > fRawNameBufSz))
            {
                fMemoryManager->deallocate(fRawName); //delete [] fRawName;
                
                fRawName = 0;
                // We have to cast off the const'ness to do this
                ((QName*)this)->fRawNameBufSz = neededLen;
                ((QName*)this)->fRawName = (XMLCh*) fMemoryManager->allocate
                (
                    (neededLen + 1) * sizeof(XMLCh)
                ); //new XMLCh[neededLen + 1];

                // Make sure its initially empty
                *fRawName = 0;
            }


            const XMLSize_t prefixLen = XMLString::stringLen(fPrefix);

            XMLString::moveChars(fRawName, fPrefix, prefixLen);
            fRawName[prefixLen] = chColon;
            XMLString::copyString(&fRawName[prefixLen+1], fLocalPart);
        }
         else
        {
            return fLocalPart;
        }
    }
    return fRawName;
}

// ---------------------------------------------------------------------------
//  QName: Setter methods
// ---------------------------------------------------------------------------
void QName::setName(const XMLCh* const    prefix
                  , const XMLCh* const    localPart
                  , const unsigned int    uriId)
{
    setPrefix(prefix);
    setLocalPart(localPart);

    // And clean up any QName and leave it undone until/if asked for again
    if (fRawName)
        *fRawName = 0;

    // And finally store the URI id parameter
    fURIId = uriId;
}

void QName::setName(const XMLCh* const    rawName
                  , const unsigned int    uriId)
{
    //set the rawName
    XMLSize_t newLen = XMLString::stringLen(rawName);
    //find out the prefix and localPart from the rawName
    const int colonInd = XMLString::indexOf(rawName, chColon);

    if (colonInd >= 0)
    {
        if (!fRawNameBufSz || (newLen > fRawNameBufSz))
        {
            fMemoryManager->deallocate(fRawName); //delete [] fRawName;
            fRawName = 0;
            fRawNameBufSz = newLen + 8;
            fRawName = (XMLCh*) fMemoryManager->allocate
            (
                (fRawNameBufSz + 1) * sizeof(XMLCh)
            ); //new XMLCh[fRawNameBufSz + 1];
        }
        XMLString::moveChars(fRawName, rawName, newLen + 1);
        setNPrefix(rawName, colonInd);
    }
    else
    {
        // No colon, so we just have a name with no prefix
        setNPrefix(XMLUni::fgZeroLenString, 0);

        // And clean up any QName and leave it undone until/if asked for again
        if (fRawName)
            *fRawName = 0;
    }

    setNLocalPart(&rawName[colonInd+1], newLen-colonInd-1);

    // And finally store the URI id parameter
    fURIId = uriId;
}

void QName::setNPrefix(const XMLCh* prefix, const XMLSize_t newLen)
{
    if (!fPrefixBufSz || (newLen > fPrefixBufSz))
    {
        fMemoryManager->deallocate(fPrefix); //delete [] fPrefix;
        fPrefix = 0;
        fPrefixBufSz = newLen + 8;
        fPrefix = (XMLCh*) fMemoryManager->allocate
        (
            (fPrefixBufSz + 1) * sizeof(XMLCh)
        ); //new XMLCh[fPrefixBufSz + 1];
    }
    XMLString::moveChars(fPrefix, prefix, newLen);
    fPrefix[newLen] = chNull;
}

void QName::setNLocalPart(const XMLCh* localPart, const XMLSize_t newLen)
{
    if (!fLocalPartBufSz || (newLen > fLocalPartBufSz))
    {
        fMemoryManager->deallocate(fLocalPart); //delete [] fLocalPart;
        fLocalPart = 0;
        fLocalPartBufSz = newLen + 8;
        fLocalPart = (XMLCh*) fMemoryManager->allocate
        (
            (fLocalPartBufSz + 1) * sizeof(XMLCh)
        ); //new XMLCh[fLocalPartBufSz + 1];
    }
    XMLString::moveChars(fLocalPart, localPart, newLen);
    fLocalPart[newLen] = chNull;
}

void QName::setValues(const QName& qname)
{
    setPrefix(qname.getPrefix());
    setLocalPart(qname.getLocalPart());
    setURI(qname.getURI());
}

// -----------------------------------------------------------------------
//  comparison
// -----------------------------------------------------------------------
bool QName::operator==(const QName& qname) const
{
    // if we are an unitialized QName, check that the other is unitialized too
    if (!fLocalPart && !fPrefix)
        return !qname.fLocalPart && !qname.fPrefix;

    if (fURIId == 0) // null URI
        return (XMLString::equals(getRawName(),qname.getRawName()));

    return ((fURIId == qname.getURI()) &&
           (XMLString::equals(fLocalPart, qname.getLocalPart())));
}

// ---------------------------------------------------------------------------
//  QName: Private, helper methods
// ---------------------------------------------------------------------------
void QName::cleanUp()
{
    fMemoryManager->deallocate(fLocalPart); //delete [] fLocalPart;
    fMemoryManager->deallocate(fPrefix); //delete [] fPrefix;
    fMemoryManager->deallocate(fRawName); //delete [] fRawName;
    fLocalPart = fPrefix = fRawName = 0;
}

/***
 * Support for Serialization/De-serialization
 ***/

IMPL_XSERIALIZABLE_TOCREATE(QName)

void QName::serialize(XSerializeEngine& serEng)
{

    if (serEng.isStoring())
    {
        serEng.writeString(fPrefix, fPrefixBufSz, XSerializeEngine::toWriteBufferLen);

        serEng.writeString(fLocalPart, fLocalPartBufSz, XSerializeEngine::toWriteBufferLen);

        //do not serialize rawName

        serEng<<fURIId;
    }
    else
    {
        XMLSize_t dataLen = 0;

        serEng.readString(fPrefix, fPrefixBufSz, dataLen, XSerializeEngine::toReadBufferLen);

        serEng.readString(fLocalPart, fLocalPartBufSz, dataLen, XSerializeEngine::toReadBufferLen);

        //force raw name rebuilt
        fRawNameBufSz = 0;        
        fRawName = 0;

        serEng>>fURIId;
    }

}

XERCES_CPP_NAMESPACE_END
