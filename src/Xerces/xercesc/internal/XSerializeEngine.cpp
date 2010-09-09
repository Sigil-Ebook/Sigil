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
 * $Id: XSerializeEngine.cpp 834826 2009-11-11 10:03:53Z borisk $
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/internal/XSerializeEngine.hpp>
#include <xercesc/internal/XSerializable.hpp>
#include <xercesc/internal/XProtoType.hpp>

#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/BinOutputStream.hpp>
#include <xercesc/util/BinInputStream.hpp>

#include <stdio.h>
#include <assert.h>

XERCES_CPP_NAMESPACE_BEGIN

const bool XSerializeEngine::toWriteBufferLen = true;
const bool XSerializeEngine::toReadBufferLen  = true;

static const unsigned long noDataFollowed = (unsigned long)-1;

static const XSerializeEngine::XSerializedObjectId_t fgNullObjectTag  = 0;           // indicating null ptrs
static const XSerializeEngine::XSerializedObjectId_t fgNewClassTag    = 0xFFFFFFFF;  // indicating new class
static const XSerializeEngine::XSerializedObjectId_t fgTemplateObjTag = 0xFFFFFFFE;  // indicating template object
static const XSerializeEngine::XSerializedObjectId_t fgClassMask      = 0x80000000;  // indicates class tag
static const XSerializeEngine::XSerializedObjectId_t fgMaxObjectCount = 0x3FFFFFFD;

#define TEST_THROW_ARG1(condition, data, err_msg) \
if (condition) \
{ \
    XMLCh value1[64]; \
    XMLString::sizeToText(data, value1, 65, 10, getMemoryManager()); \
    ThrowXMLwithMemMgr1(XSerializationException \
            , err_msg  \
            , value1 \
            , getMemoryManager()); \
}

#define TEST_THROW_ARG2(condition, data1, data2, err_msg) \
if (condition) \
{ \
    XMLCh value1[64]; \
    XMLCh value2[64]; \
    XMLString::sizeToText(data1, value1, 65, 10, getMemoryManager()); \
    XMLString::sizeToText(data2, value2, 65, 10, getMemoryManager()); \
    ThrowXMLwithMemMgr2(XSerializationException \
            , err_msg  \
            , value1   \
            , value2 \
            , getMemoryManager()); \
}

// ---------------------------------------------------------------------------
//  Constructor and Destructor
// ---------------------------------------------------------------------------
XSerializeEngine::~XSerializeEngine()
{
    if (isStoring())
    {
        flush();
        delete fStorePool;
    }
    else
    {
        delete fLoadPool;
    }

    getMemoryManager()->deallocate(fBufStart);

}

XSerializeEngine::XSerializeEngine(BinInputStream*         inStream
                                 , XMLGrammarPool* const   gramPool
                                 , XMLSize_t               bufSize)
:fStoreLoad(mode_Load)
,fStorerLevel(0)
,fGrammarPool(gramPool)
,fInputStream(inStream)
,fOutputStream(0)
,fBufCount(0)
,fBufSize(bufSize)
,fBufStart( (XMLByte*) gramPool->getMemoryManager()->allocate(bufSize))
,fBufEnd(0)
,fBufCur(fBufStart)
,fBufLoadMax(fBufStart)
,fStorePool(0)
,fLoadPool( new (gramPool->getMemoryManager()) ValueVectorOf<void*>(29, gramPool->getMemoryManager(), false))
,fObjectCount(0)
{
    /***
     *  initialize buffer from the inStream
     ***/
    fillBuffer();

}

XSerializeEngine::XSerializeEngine(BinOutputStream*        outStream
                                 , XMLGrammarPool* const   gramPool
                                 , XMLSize_t               bufSize)
:fStoreLoad(mode_Store)
,fStorerLevel(0)
,fGrammarPool(gramPool)
,fInputStream(0)
,fOutputStream(outStream)
,fBufCount(0)
,fBufSize(bufSize)
,fBufStart((XMLByte*) gramPool->getMemoryManager()->allocate(bufSize))
,fBufEnd(fBufStart+bufSize)
,fBufCur(fBufStart)
,fBufLoadMax(0)
,fStorePool( new (gramPool->getMemoryManager()) RefHashTableOf<XSerializedObjectId, PtrHasher>(29, true, gramPool->getMemoryManager()) )
,fLoadPool(0)
,fObjectCount(0)
{
    resetBuffer();

    //initialize store pool
    fStorePool->put(0, new (gramPool->getMemoryManager()) XSerializedObjectId(fgNullObjectTag));

}

void XSerializeEngine::flush()
{
    if (isStoring())
        flushBuffer();

}

// ---------------------------------------------------------------------------
//  Storing
// ---------------------------------------------------------------------------
void XSerializeEngine::write(XSerializable* const objectToWrite)
{
    ensureStoring();
    //don't ensurePointer here !!!

    XSerializedObjectId_t   objIndex = 0;

	if (!objectToWrite)  // null pointer
	{
		*this << fgNullObjectTag;
	}
    else if (0 != (objIndex = lookupStorePool((void*) objectToWrite)))
	{
        // writing an object reference tag
        *this << objIndex;
	}
	else
	{
		// write protoType first
		XProtoType* protoType = objectToWrite->getProtoType();
		write(protoType);

		// put the object into StorePool
        addStorePool((void*)objectToWrite);

        // ask the object to serialize itself
		objectToWrite->serialize(*this);
	}

}

void XSerializeEngine::write(XProtoType* const protoType)
{
    ensureStoring();
    ensurePointer(protoType);

	XSerializedObjectId_t objIndex = lookupStorePool((void*)protoType);

    if (objIndex)
    {
        //protoType seen in the store pool
        *this << (fgClassMask | objIndex);
	}
	else
	{
		// store protoType
		*this << fgNewClassTag;
		protoType->store(*this);
        addStorePool((void*)protoType);
	}

}

/***
 *
***/
void XSerializeEngine::write(const XMLCh* const toWrite
                           ,       XMLSize_t    writeLen)
{
    write((XMLByte*)toWrite, (sizeof(XMLCh)/sizeof(XMLByte)) * writeLen);
}


void XSerializeEngine::write(const XMLByte* const toWrite
                           ,       XMLSize_t      writeLen)
{
    ensureStoring();
    ensurePointer((void*)toWrite);
    ensureStoreBuffer();

    if (writeLen == 0)
        return;

    /***
     *  If the available space is sufficient, write it up
     ***/
    XMLSize_t bufAvail = fBufEnd - fBufCur;

    if (writeLen <= bufAvail)
    {
        memcpy(fBufCur, toWrite, writeLen);
        fBufCur += writeLen;
        return;
    }

    const XMLByte*  tempWrite   = (const XMLByte*) toWrite;
    XMLSize_t       writeRemain = writeLen;

    // fill up the avaiable space and flush
    memcpy(fBufCur, tempWrite, bufAvail);
    tempWrite   += bufAvail;
    writeRemain -= bufAvail;
    flushBuffer();

    // write chunks of fBufSize
    while (writeRemain >= fBufSize)
    {
        memcpy(fBufCur, tempWrite, fBufSize);
        tempWrite   += fBufSize;
        writeRemain -= fBufSize;
        flushBuffer();
    }

    // write the remaining if any
    if (writeRemain)
    {
        memcpy(fBufCur, tempWrite, writeRemain);
        fBufCur += writeRemain;
    }

}

/***
 *
 *     Storage scheme (normal):
 *
 *     <
 *     1st integer:    bufferLen (optional)
 *     2nd integer:    dataLen
 *     bytes following:
 *     >
 *
 *     Storage scheme (special):
 *     <
 *     only integer:   noDataFollowed
 *     >
 */

void XSerializeEngine::writeString(const XMLCh* const toWrite
                                 , const XMLSize_t    bufferLen
                                 , bool               toWriteBufLen)
{
    if (toWrite)
    {
        if (toWriteBufLen)
            *this<<(unsigned long)bufferLen;

        XMLSize_t strLen = XMLString::stringLen(toWrite);
        *this<<(unsigned long)strLen;

        write(toWrite, strLen);
    }
    else
    {
        *this<<noDataFollowed;
    }

}

void XSerializeEngine::writeString(const XMLByte* const toWrite
                                 , const XMLSize_t      bufferLen
                                 , bool                 toWriteBufLen)
{

    if (toWrite)
    {
        if (toWriteBufLen)
            *this<<(unsigned long)bufferLen;

        XMLSize_t strLen = XMLString::stringLen((char*)toWrite);
        *this<<(unsigned long)strLen;
        write(toWrite, strLen);
    }
    else
    {
        *this<<noDataFollowed;
    }

}

// ---------------------------------------------------------------------------
//  Loading
// ---------------------------------------------------------------------------
XSerializable* XSerializeEngine::read(XProtoType* const protoType)
{
    ensureLoading();
    ensurePointer(protoType);

	XSerializedObjectId_t    objectTag;
	XSerializable*           objRet;

    if (! read(protoType, &objectTag))
	{
        /***
         * We hava a reference to an existing object in
         * load pool, get it.
         */
        objRet = lookupLoadPool(objectTag);
	}
	else
	{
		// create the object from the prototype
		objRet = protoType->fCreateObject(getMemoryManager());
        Assert((objRet != 0), XMLExcepts::XSer_CreateObject_Fail);

        // put it into load pool
        addLoadPool(objRet);

        // de-serialize it
		objRet->serialize(*this);

	}

	return objRet;
}

bool XSerializeEngine::read(XProtoType*            const    protoType
                          , XSerializedObjectId_t*          objectTagRet)
{
    ensureLoading();
    ensurePointer(protoType);

	XSerializedObjectId_t obTag;

    *this >> obTag;

    // object reference tag found
    if (!(obTag & fgClassMask))
	{
		*objectTagRet = obTag;
		return false;
	}

	if (obTag == fgNewClassTag)
	{
        // what follows fgNewClassTag is the prototype object info
        // for the object anticipated, go and verify the info
        XProtoType::load(*this, protoType->fClassName, getMemoryManager());

        addLoadPool((void*)protoType);
	}
	else
	{
        // what follows class tag is an XSerializable object
	XSerializedObjectId_t classIndex = (obTag & ~fgClassMask);
        XSerializedObjectId_t loadPoolSize = (XSerializedObjectId_t)fLoadPool->size();

        if ((classIndex == 0 ) || (classIndex > loadPoolSize))
        {
          XMLCh value1[64];
          XMLCh value2[64];
          XMLString::binToText(classIndex, value1, 65, 10, getMemoryManager());
          XMLString::binToText(loadPoolSize, value2, 65, 10, getMemoryManager());
          ThrowXMLwithMemMgr2(XSerializationException
                              , XMLExcepts::XSer_Inv_ClassIndex
                              , value1
                              , value2
                              , getMemoryManager());
        }

        ensurePointer(lookupLoadPool(classIndex));
   }

	return true;
}

void XSerializeEngine::read(XMLCh* const toRead
                          , XMLSize_t    readLen)
{
    read((XMLByte*)toRead, (sizeof(XMLCh)/sizeof(XMLByte))*readLen);
}

void XSerializeEngine::read(XMLByte* const toRead
                          , XMLSize_t      readLen)
{
    ensureLoading();
    ensurePointer(toRead);
    ensureLoadBuffer();

    if (readLen == 0)
        return;

    /***
     *  If unread is sufficient, read it up
     ***/
    XMLSize_t dataAvail = fBufLoadMax - fBufCur;

    if (readLen <= dataAvail)
    {
        memcpy(toRead, fBufCur, readLen);
        fBufCur += readLen;
        return;
    }

    /***
     *
     * fillBuffer will discard anything left in the buffer
     * before it asks the inputStream to fill in the buffer,
     * so we need to readup everything in the buffer before
     * calling fillBuffer
     *
     ***/
    XMLByte*     tempRead   = (XMLByte*) toRead;
    XMLSize_t    readRemain = readLen;

    // read the unread
    memcpy(tempRead, fBufCur, dataAvail);
    tempRead   += dataAvail;
    readRemain -= dataAvail;

    // read chunks of fBufSize
    while (readRemain >= fBufSize)
    {
        fillBuffer();
        memcpy(tempRead, fBufCur, fBufSize);
        tempRead   += fBufSize;
        readRemain -= fBufSize;
    }

    // read the remaining if any
    if (readRemain)
    {
        fillBuffer();
        memcpy(tempRead, fBufCur, readRemain);
        fBufCur += readRemain;
    }

}

/***
 *
 *     Storage scheme (normal):
 *
 *     <
 *     1st integer:    bufferLen (optional)
 *     2nd integer:    dataLen
 *     bytes following:
 *     >
 *
 *     Storage scheme (special):
 *     <
 *     only integer:   noDataFollowed
 *     >
 */
void XSerializeEngine::readString(XMLCh*&       toRead
                                , XMLSize_t&    bufferLen
                                , XMLSize_t&    dataLen
                                , bool          toReadBufLen)
{
    /***
     * Check if any data written
     ***/
    unsigned long tmp;
    *this>>tmp;
    bufferLen=tmp;

    if (bufferLen == noDataFollowed)
    {
        toRead = 0;
        bufferLen = 0;
        dataLen = 0;
        return;
    }

    if (toReadBufLen)
    {
        *this>>tmp;
        dataLen=tmp;
    }
    else
    {
        dataLen = bufferLen++;
    }

    toRead = (XMLCh*) getMemoryManager()->allocate(bufferLen * sizeof(XMLCh));
    read(toRead, dataLen);
    toRead[dataLen] = 0;
}

void XSerializeEngine::readString(XMLByte*&     toRead
                                , XMLSize_t&    bufferLen
                                , XMLSize_t&    dataLen
                                , bool          toReadBufLen)
{
    /***
     * Check if any data written
     ***/
    unsigned long tmp;
    *this>>tmp;
    bufferLen=tmp;
    if (bufferLen == noDataFollowed)
    {
        toRead = 0;
        bufferLen = 0;
        dataLen = 0;
        return;
    }

    if (toReadBufLen)
    {
        *this>>tmp;
        dataLen=tmp;
    }
    else
    {
        dataLen = bufferLen++;
    }

    toRead = (XMLByte*) getMemoryManager()->allocate(bufferLen * sizeof(XMLByte));
    read(toRead, dataLen);
    toRead[dataLen] = 0;

}

// ---------------------------------------------------------------------------
//  Insertion & Extraction
// ---------------------------------------------------------------------------

XSerializeEngine& XSerializeEngine::operator<<(XMLCh xch)
{
    checkAndFlushBuffer(calBytesNeeded(sizeof(XMLCh)));

    alignBufCur(sizeof(XMLCh));
    *(XMLCh*)fBufCur = xch;
    fBufCur += sizeof(XMLCh);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator>>(XMLCh& xch)
{
    checkAndFillBuffer(calBytesNeeded(sizeof(XMLCh)));

    alignBufCur(sizeof(XMLCh));
    xch = *(XMLCh*)fBufCur;
    fBufCur += sizeof(XMLCh);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator<<(XMLByte by)
{
    checkAndFlushBuffer(sizeof(XMLByte));

    *(XMLByte*)fBufCur = by;
    fBufCur += sizeof(XMLByte);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator>>(XMLByte& by)
{
    checkAndFillBuffer(sizeof(XMLByte));

    by = *(XMLByte*)fBufCur;
    fBufCur += sizeof(XMLByte);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator<<(bool b)
{
    checkAndFlushBuffer(sizeof(bool));

    *(bool*)fBufCur = b;
    fBufCur += sizeof(bool);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator>>(bool& b)
{
    checkAndFillBuffer(sizeof(bool));

    b = *(bool*)fBufCur;
    fBufCur += sizeof(bool);
    return *this;
}

void XSerializeEngine::writeSize (XMLSize_t t)
{
  checkAndFlushBuffer(sizeof(t));

  memcpy(fBufCur, &t, sizeof(t));
  fBufCur += sizeof(t);
}

void XSerializeEngine::writeInt64 (XMLInt64 t)
{
  checkAndFlushBuffer(sizeof(t));

  memcpy(fBufCur, &t, sizeof(t));
  fBufCur += sizeof(t);
}

void XSerializeEngine::writeUInt64 (XMLUInt64 t)
{
  checkAndFlushBuffer(sizeof(t));

  memcpy(fBufCur, &t, sizeof(t));
  fBufCur += sizeof(t);
}

void XSerializeEngine::readSize (XMLSize_t& t)
{
  checkAndFillBuffer(sizeof(t));

  memcpy(&t, fBufCur, sizeof(t));
  fBufCur += sizeof(t);
}

void XSerializeEngine::readInt64 (XMLInt64& t)
{
  checkAndFillBuffer(sizeof(t));

  memcpy(&t, fBufCur, sizeof(t));
  fBufCur += sizeof(t);
}

void XSerializeEngine::readUInt64 (XMLUInt64& t)
{
  checkAndFillBuffer(sizeof(t));

  memcpy(&t, fBufCur, sizeof(t));
  fBufCur += sizeof(t);
}

XSerializeEngine& XSerializeEngine::operator<<(char ch)
{
    return XSerializeEngine::operator<<((XMLByte)ch);
}

XSerializeEngine& XSerializeEngine::operator>>(char& ch)
{
    return XSerializeEngine::operator>>((XMLByte&)ch);
}

XSerializeEngine& XSerializeEngine::operator<<(short sh)
{
    checkAndFlushBuffer(calBytesNeeded(sizeof(short)));

    alignBufCur(sizeof(short));
    *(short*)fBufCur = sh;
    fBufCur += sizeof(short);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator>>(short& sh)
{
    checkAndFillBuffer(calBytesNeeded(sizeof(short)));

    alignBufCur(sizeof(short));
    sh = *(short*)fBufCur;
    fBufCur += sizeof(short);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator<<(int i)
{
    checkAndFlushBuffer(calBytesNeeded(sizeof(int)));

    alignBufCur(sizeof(int));
    *(int*)fBufCur = i;
    fBufCur += sizeof(int);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator>>(int& i)
{
    checkAndFillBuffer(calBytesNeeded(sizeof(int)));

    alignBufCur(sizeof(int));
    i = *(int*)fBufCur;
    fBufCur += sizeof(int);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator<<(unsigned int ui)
{

    checkAndFlushBuffer(calBytesNeeded(sizeof(unsigned int)));

    alignBufCur(sizeof(unsigned int));
    *(unsigned int*)fBufCur = ui;
    fBufCur += sizeof(unsigned int);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator>>(unsigned int& ui)
{

    checkAndFillBuffer(calBytesNeeded(sizeof(unsigned int)));

    alignBufCur(sizeof(unsigned int));
    ui = *(unsigned int*)fBufCur;
    fBufCur += sizeof(unsigned int);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator<<(long l)
{
    checkAndFlushBuffer(calBytesNeeded(sizeof(long)));

    alignBufCur(sizeof(long));
    *(long*)fBufCur = l;
    fBufCur += sizeof(long);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator>>(long& l)
{
    checkAndFillBuffer(calBytesNeeded(sizeof(long)));

    alignBufCur(sizeof(long));
    l = *(long*)fBufCur;
    fBufCur += sizeof(long);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator<<(unsigned long ul)
{
    checkAndFlushBuffer(calBytesNeeded(sizeof(unsigned long)));

    alignBufCur(sizeof(unsigned long));
    *(unsigned long*)fBufCur = ul;
    fBufCur += sizeof(unsigned long);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator>>(unsigned long& ul)
{
    checkAndFillBuffer(calBytesNeeded(sizeof(unsigned long)));

    alignBufCur(sizeof(unsigned long));
    ul = *(unsigned long*)fBufCur;
    fBufCur += sizeof(unsigned long);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator<<(float f)
{
    checkAndFlushBuffer(calBytesNeeded(sizeof(float)));

    alignBufCur(sizeof(float));
    *(float*)fBufCur = *(float*)&f;
    fBufCur += sizeof(float);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator>>(float& f)
{
    checkAndFillBuffer(calBytesNeeded(sizeof(float)));

    alignBufCur(sizeof(float));
    *(float*)&f = *(float*)fBufCur;
    fBufCur += sizeof(float);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator<<(double d)
{
    checkAndFlushBuffer(calBytesNeeded(sizeof(double)));

    alignBufCur(sizeof(double));
    *(double*)fBufCur = *(double*)&d;
    fBufCur += sizeof(double);
    return *this;
}

XSerializeEngine& XSerializeEngine::operator>>(double& d)
{
    checkAndFillBuffer(calBytesNeeded(sizeof(double)));

    alignBufCur(sizeof(double));
    *(double*)&d = *(double*)fBufCur;
    fBufCur += sizeof(double);
    return *this;
}

// ---------------------------------------------------------------------------
//  StorePool/LoadPool Opertions
// ---------------------------------------------------------------------------
XSerializeEngine::XSerializedObjectId_t
XSerializeEngine::lookupStorePool(void* const objToLookup) const
{
    //0 indicating object is not in the StorePool
    XSerializedObjectId* data = fStorePool->get(objToLookup);
    return (XSerializeEngine::XSerializedObjectId_t) (data ? data->getValue() : 0);

}

void XSerializeEngine::addStorePool(void* const objToAdd)
{
    pumpCount();
    fStorePool->put(objToAdd, new (fGrammarPool->getMemoryManager()) XSerializedObjectId(fObjectCount));
}

XSerializable* XSerializeEngine::lookupLoadPool(XSerializedObjectId_t objectTag) const
{

    /***
      *  an object tag read from the binary refering to
      *  an object beyond the upper most boundary of the load pool
      ***/

    if (objectTag > fLoadPool->size())
    {
      XMLCh value1[64];
      XMLCh value2[64];
      XMLString::binToText(objectTag, value1, 65, 10, getMemoryManager());
      XMLString::sizeToText(fLoadPool->size(), value2, 65, 10, getMemoryManager());
      ThrowXMLwithMemMgr2(XSerializationException
                          , XMLExcepts::XSer_LoadPool_UppBnd_Exceed
                          , value1
                          , value2
                          , getMemoryManager());
    }

    if (objectTag == 0)
        return 0;

    /***
     *   A non-null object tag starts from 1 while fLoadPool starts from 0
     ***/
    return (XSerializable*) fLoadPool->elementAt(objectTag - 1);
}

void XSerializeEngine::addLoadPool(void* const objToAdd)
{

    TEST_THROW_ARG2( (fLoadPool->size() != fObjectCount)
               , fObjectCount
               , fLoadPool->size()
               , XMLExcepts::XSer_LoadPool_NoTally_ObjCnt
               )

    pumpCount();
    fLoadPool->addElement(objToAdd);

}

void XSerializeEngine::pumpCount()
{
    if (fObjectCount >= fgMaxObjectCount)
    {
      XMLCh value1[64];
      XMLCh value2[64];
      XMLString::sizeToText(fObjectCount, value1, 65, 10, getMemoryManager());
      XMLString::binToText(fgMaxObjectCount, value2, 65, 10, getMemoryManager());
      ThrowXMLwithMemMgr2(XSerializationException
                          , XMLExcepts::XSer_ObjCount_UppBnd_Exceed
                          , value1
                          , value2
                          , getMemoryManager());
    }

    fObjectCount++;

}

// ---------------------------------------------------------------------------
//  Buffer Opertions
// ---------------------------------------------------------------------------
/***
 *
 *  Though client may need only miniBytesNeeded, we always request
 *  a full size reading from our inputStream.
 *
 *  Whatever possibly left in the buffer is abandoned, such as in
 *  the case of CheckAndFillBuffer()
 *
 ***/
void XSerializeEngine::fillBuffer()
{
    ensureLoading();
    ensureLoadBuffer();

    resetBuffer();

    XMLSize_t bytesRead = fInputStream->readBytes(fBufStart, fBufSize);

    /***
     * InputStream MUST fill in the exact amount of bytes as requested
     * to do: combine the checking and create a new exception code later
    ***/
    TEST_THROW_ARG2( (bytesRead < fBufSize)
               , bytesRead
               , fBufSize
               , XMLExcepts::XSer_InStream_Read_LT_Req
               )

    TEST_THROW_ARG2( (bytesRead > fBufSize)
               , bytesRead
               , fBufSize
               , XMLExcepts::XSer_InStream_Read_OverFlow
               )

    fBufLoadMax = fBufStart + fBufSize;
    fBufCur     = fBufStart;

    ensureLoadBuffer();

    fBufCount++;
}

/***
 *
 *  Flush out whatever left in the buffer, from
 *  fBufStart to fBufEnd.
 *
 ***/
void XSerializeEngine::flushBuffer()
{
    ensureStoring();
    ensureStoreBuffer();

    fOutputStream->writeBytes(fBufStart, fBufSize);
    fBufCur = fBufStart;

    resetBuffer();
    ensureStoreBuffer();

    fBufCount++;
}

inline void XSerializeEngine::checkAndFlushBuffer(XMLSize_t bytesNeedToWrite)
{
    TEST_THROW_ARG1( (bytesNeedToWrite <= 0)
                   , bytesNeedToWrite
                   , XMLExcepts::XSer_Inv_checkFlushBuffer_Size
                   )

    // fBufStart ... fBufCur ...fBufEnd
    if ((fBufCur + bytesNeedToWrite) > fBufEnd)
        flushBuffer();
}

inline void XSerializeEngine::checkAndFillBuffer(XMLSize_t bytesNeedToRead)
{

    TEST_THROW_ARG1( (bytesNeedToRead <= 0)
                   , bytesNeedToRead
                   , XMLExcepts::XSer_Inv_checkFillBuffer_Size
                   )

    // fBufStart ... fBufCur ...fBufLoadMax
    if ((fBufCur + bytesNeedToRead) > fBufLoadMax)
    {
        fillBuffer();
    }

}

inline void XSerializeEngine::ensureStoreBuffer() const
{
    XMLSize_t a = (XMLSize_t) (fBufCur - fBufStart);
    XMLSize_t b = (XMLSize_t) (fBufEnd - fBufCur);

    TEST_THROW_ARG2 ( !((fBufStart <= fBufCur) && (fBufCur <= fBufEnd))
                    , a
                    , b
                    , XMLExcepts::XSer_StoreBuffer_Violation
                    )

}

inline void XSerializeEngine::ensureLoadBuffer() const
{
    XMLSize_t a = (XMLSize_t) (fBufCur - fBufStart);
    XMLSize_t b = (XMLSize_t) (fBufLoadMax - fBufCur);

    TEST_THROW_ARG2 ( !((fBufStart <= fBufCur) && (fBufCur <= fBufLoadMax))
                    , a
                    , b
                    , XMLExcepts::XSer_LoadBuffer_Violation
                    )

}

inline void XSerializeEngine::ensurePointer(void* const ptr) const
{

    TEST_THROW_ARG1( (ptr == 0)
                   , 0
                   , XMLExcepts::XSer_Inv_Null_Pointer
                   )

}

inline void XSerializeEngine::resetBuffer()
{
    memset(fBufStart, 0, fBufSize * sizeof(XMLByte));
}

// ---------------------------------------------------------------------------
//  Template object
// ---------------------------------------------------------------------------
/***
 *
 *  Search the store pool to see if the address has been seen before or not.
 *
 *  If yes, write the corresponding object Tag to the internal buffer
 *  and return true.
 *
 *  Otherwise, add the address to the store pool and return false
 *  to notifiy the client application code to store the template object.
 *
 ***/
bool XSerializeEngine::needToStoreObject(void* const  templateObjectToWrite)
{
    ensureStoring(); //don't ensurePointer here !!!

    XSerializedObjectId_t   objIndex = 0;

	if (!templateObjectToWrite)
	{
		*this << fgNullObjectTag; // null pointer
        return false;
	}
    else if (0 != (objIndex = lookupStorePool(templateObjectToWrite)))
	{
        *this << objIndex;         // write an object reference tag
        return false;
	}
	else
	{
        *this << fgTemplateObjTag;            // write fgTemplateObjTag to denote that actual
                                              // template object follows
        addStorePool(templateObjectToWrite); // put the address into StorePool
        return true;
	}

}

bool XSerializeEngine::needToLoadObject(void**  templateObjectToRead)
{
    ensureLoading();

	XSerializedObjectId_t obTag;

    *this >> obTag;

	if (obTag == fgTemplateObjTag)
	{
        /***
         * what follows fgTemplateObjTag is the actual template object
         * We need the client application to create a template object
         * and register it through registerObject(), and deserialize
         * template object
         ***/
        return true;
	}
	else
	{
        /***
         * We hava a reference to an existing template object, get it.
         */
        *templateObjectToRead = lookupLoadPool(obTag);
        return false;
   }

}

void XSerializeEngine::registerObject(void*  const templateObjectToRegister)
{
    ensureLoading();
    addLoadPool(templateObjectToRegister);
}

XMLGrammarPool* XSerializeEngine::getGrammarPool() const
{
    return fGrammarPool;
}

XMLStringPool* XSerializeEngine::getStringPool() const
{
    return fGrammarPool->getURIStringPool();
}

MemoryManager* XSerializeEngine::getMemoryManager() const
{
    //todo: changed to return fGrammarPool->getMemoryManager()
    return fGrammarPool ? fGrammarPool->getMemoryManager() : XMLPlatformUtils::fgMemoryManager;
}

//
// Based on the current position (fBufCur), calculated the needed size
// to read/write
//
inline XMLSize_t XSerializeEngine::alignAdjust(XMLSize_t size) const
{
    XMLSize_t remainder = (XMLSize_t) fBufCur % size;
    return (remainder == 0) ? 0 : (size - remainder);
}

// Adjust the fBufCur
inline void XSerializeEngine::alignBufCur(XMLSize_t size)
{
    fBufCur+=alignAdjust(size);
    assert(((XMLSize_t) fBufCur % size)==0);
}

inline XMLSize_t XSerializeEngine::calBytesNeeded(XMLSize_t size) const
{
    return (alignAdjust(size) + size);
}

void XSerializeEngine::trace(char* /*funcName*/) const
{
    return;

/*
   if (isStoring())
        printf("\n funcName=<%s>, storing, count=<%lu>, postion=<%lu>\n", funcName, fBufCount, getBufCurAccumulated());
    else
        printf("\n funcName=<%s>, loading, count=<%lu>, postion=<%lu>\n", funcName, fBufCount, getBufCurAccumulated());
*/
}

XERCES_CPP_NAMESPACE_END
