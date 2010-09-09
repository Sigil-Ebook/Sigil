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
 * $Id: XSerializeEngine.hpp 679296 2008-07-24 08:13:42Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XSERIALIZE_ENGINE_HPP)
#define XERCESC_INCLUDE_GUARD_XSERIALIZE_ENGINE_HPP

#include <xercesc/util/RefHashTableOf.hpp>
#include <xercesc/util/ValueVectorOf.hpp>
#include <xercesc/util/XMLExceptMsgs.hpp>

#include <xercesc/internal/XSerializationException.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XSerializable;
class XProtoType;
class MemoryManager;
class XSerializedObjectId;
class BinOutputStream;
class BinInputStream;
class XMLGrammarPool;
class XMLGrammarPoolImpl;
class XMLStringPool;

class XMLUTIL_EXPORT XSerializeEngine
{
public:

    enum { mode_Store
         , mode_Load
    };


    static const bool toReadBufferLen;

    typedef unsigned int   XSerializedObjectId_t;

    /***
      *
      *  Destructor
      *
      ***/
    ~XSerializeEngine();

    /***
      *
      *  Constructor for de-serialization(loading)
      *
      *  Application needs to make sure that the instance of
      *  BinInputStream, persists beyond the life of this
      *  SerializeEngine.
      *
      *  Param
      *     inStream         input stream
      *     gramPool         Grammar Pool
      *     bufSize          the size of the internal buffer
      *
      ***/
    XSerializeEngine(BinInputStream*         inStream
                   , XMLGrammarPool* const   gramPool
                   , XMLSize_t               bufSize = 8192 );


    /***
      *
      *  Constructor for serialization(storing)
      *
      *  Application needs to make sure that the instance of
      *  BinOutputStream, persists beyond the life of this
      *  SerializeEngine.
      *
      *  Param
      *     outStream        output stream
      *     gramPool         Grammar Pool
      *     bufSize          the size of the internal buffer
      *
      ***/
    XSerializeEngine(BinOutputStream*        outStream
                   , XMLGrammarPool* const   gramPool
                   , XMLSize_t               bufSize = 8192 );

    /***
      *
      *  When serialization, flush out the internal buffer
      *
      *  Return:
      *
      ***/
    void flush();

    /***
      *
      *  Checking if the serialize engine is doing serialization(storing)
      *
      *  Return: true, if it is
      *          false, otherwise
      *
      ***/
    inline bool isStoring() const;

    /***
      *
      *  Checking if the serialize engine is doing de-serialization(loading)
      *
      *  Return: true, if it is
      *          false, otherwise
      *
      ***/
    inline bool isLoading() const;

    /***
      *
      *  Get the GrammarPool
      *
      *  Return: XMLGrammarPool
      *
      ***/
    XMLGrammarPool* getGrammarPool() const;

    /***
      *
      *  Get the StringPool
      *
      *  Return: XMLStringPool
      *
      ***/
    XMLStringPool* getStringPool() const;

    /***
      *
      *  Get the embeded Memory Manager
      *
      *  Return: MemoryManager
      *
      ***/
    MemoryManager* getMemoryManager() const;

    /***
      *
      *  Get the storer level (the level of the serialize engine
      *  which created the binary stream that this serialize engine
      *  is loading).
      *
      *  The level returned is meaningful only when
      *  the engine isLoading.
      *
      *  Return: level
      *
      ***/
    inline unsigned int getStorerLevel() const;

    /***
      *
      *  Write object to the internal buffer.
      *
      *  Param
      *     objectToWrite:    the object to be serialized
      *
      *  Return:
      *
      ***/
           void           write(XSerializable* const objectToWrite);

    /***
      *
      *  Write prototype info to the internal buffer.
      *
      *  Param
      *     protoType:    instance of prototype
      *
      *  Return:
      *
      ***/
           void           write(XProtoType* const protoType);

    /***
      *
      *  Write a stream of XMLByte to the internal buffer.
      *
      *  Param
      *     toWrite:   the stream of XMLByte to write
      *     writeLen:  the length of the stream
      *
      *  Return:
      *
      ***/
           void           write(const XMLByte* const toWrite
                               ,      XMLSize_t      writeLen);

    /***
      *
      *  Write a stream of XMLCh to the internal buffer.
      *
      *  Param
      *     toWrite:   the stream of XMLCh to write
      *     writeLen:  the length of the stream
      *
      *  Return:
      *
      ***/
           void           write(const XMLCh* const toWrite
                               ,      XMLSize_t    writeLen);

    /***
      *
      *  Write a stream of XMLCh to the internal buffer.
      *
      *  Write the bufferLen first if requested, then the length
      *  of the stream followed by the stream.
      *
      *  Param
      *     toWrite:        the stream of XMLCh to write
      *     bufferLen:      the maximum size of the buffer
      *     toWriteBufLen:  specify if the bufferLen need to be written or not
      *
      *  Return:
      *
      ***/
           void           writeString(const XMLCh* const toWrite
                                    , const XMLSize_t    bufferLen = 0
                                    , bool               toWriteBufLen = false);

    /***
      *
      *  Write a stream of XMLByte to the internal buffer.
      *
      *  Write the bufferLen first if requested, then the length
      *  of the stream followed by the stream.
      *
      *  Param
      *     toWrite:        the stream of XMLByte to write
      *     bufferLen:      the maximum size of the buffer
      *     toWriteBufLen:  specify if the bufferLen need to be written or not
      *
      *  Return:
      *
      ***/
           void           writeString(const XMLByte* const toWrite
                                    , const XMLSize_t      bufferLen = 0
                                    , bool                 toWriteBufLen = false);

    static const bool toWriteBufferLen;

    /***
      *
      *  Read/Create object from the internal buffer.
      *
      *  Param
      *     protoType:    an instance of prototype of the object anticipated
      *
      *  Return:          to object read/created
      *
      ***/
	       XSerializable* read(XProtoType* const protoType);

    /***
      *
      *  Read prototype object from the internal buffer.
      *  Verify if the same prototype object found in buffer.
      *
      *  Param
      *     protoType:    an instance of prototype of the object anticipated
      *     objTag:       the object Tag to an existing object
      *
      *  Return:          true  : if matching found
      *                   false : otherwise
      *
      ***/
           bool           read(XProtoType* const    protoType
		                     , XSerializedObjectId_t*       objTag);

    /***
      *
      *  Read XMLByte stream from the internal buffer.
      *
      *  Param
      *     toRead:   the buffer to hold the XMLByte stream
      *     readLen:  the length of the XMLByte to read in
      *
      *  Return:
      *
      ***/
           void           read(XMLByte* const toRead
                             , XMLSize_t      readLen);

    /***
      *
      *  Read XMLCh stream from the internal buffer.
      *
      *  Param
      *     toRead:   the buffer to hold the XMLCh stream
      *     readLen:  the length of the XMLCh to read in
      *
      *  Return:
      *
      ***/
           void           read(XMLCh* const toRead
                             , XMLSize_t    readLen);

    /***
      *
      *  Read a stream of XMLCh from the internal buffer.
      *
      *  Read the bufferLen first if requested, then the length
      *  of the stream followed by the stream.
      *
      *  Param
      *     toRead:       the pointer to the buffer to hold the XMLCh stream
      *     bufferLen:    the size of the buffer created
      *     dataLen:       the length of the stream
      *     toReadBufLen: specify if the bufferLen need to be read or not
      *
      *  Return:
      *
      ***/
           void           readString(XMLCh*&        toRead
                                   , XMLSize_t&     bufferLen
                                   , XMLSize_t&     dataLen
                                   , bool           toReadBufLen = false);

     /***
       *
       *  Read a stream of XMLCh from the internal buffer.
       *
       *  Read the bufferLen first if requested, then the length
       *  of the stream followed by the stream.
       *
       *  Param
       *     toRead:       the pointer to the buffer to hold the XMLCh stream
       *     bufferLen:    the size of the buffer created
       *
       *  Return:
       *
       ***/
            inline void     readString(XMLCh*&        toRead
                                    , XMLSize_t&      bufferLen);

     /***
       *
       *  Read a stream of XMLCh from the internal buffer.
       *
       *  Param
       *     toRead:       the pointer to the buffer to hold the XMLCh stream
       *
       *  Return:
       *
       ***/
            inline void      readString(XMLCh*&        toRead);

    /***
      *
      *  Read a stream of XMLByte from the internal buffer.
      *
      *  Read the bufferLen first if requested, then the length
      *  of the stream followed by the stream.
      *
      *  Param
      *     toRead:       the pointer to the buffer to hold the XMLByte stream
      *     bufferLen:    the size of the buffer created
      *     dataLen:       the length of the stream
      *     toReadBufLen: specify if the bufferLen need to be read or not
      *
      *  Return:
      *
      ***/
           void           readString(XMLByte*&      toRead
                                   , XMLSize_t&     bufferLen
                                   , XMLSize_t&     dataLen
                                   , bool           toReadBufLen = false);


     /***
       *
       *  Read a stream of XMLByte from the internal buffer.
       *
       *  Read the bufferLen first if requested, then the length
       *  of the stream followed by the stream.
       *
       *  Param
       *     toRead:       the pointer to the buffer to hold the XMLByte stream
       *     bufferLen:    the size of the buffer created
       *
       *  Return:
       *
       ***/
            inline void       readString(XMLByte*&      toRead
                                       , XMLSize_t&     bufferLen);

     /***
       *
       *  Read a stream of XMLByte from the internal buffer.
       *
       *  Read the bufferLen first if requested, then the length
       *  of the stream followed by the stream.
       *
       *  Param
       *     toRead:       the pointer to the buffer to hold the XMLByte stream
       *     bufferLen:    the size of the buffer created
       *     dataLen:       the length of the stream
       *     toReadBufLen: specify if the bufferLen need to be read or not
       *
       *  Return:
       *
       ***/
            inline void       readString(XMLByte*&      toRead);

    /***
      *
      *  Check if the template object has been stored or not
      *
      *  Param
      *    objectPtr:     the template object pointer
      *
      *  Return:          true  : the object has NOT been stored yet
      *                   false : otherwise
      *
      ***/
           bool           needToStoreObject(void* const templateObjectToWrite);

    /***
      *
      *  Check if the template object has been loaded or not
      *
      *  Param
      *    objectPtr:     the address of the template object pointer
      *
      *  Return:          true  : the object has NOT been loaded yet
      *                   false : otherwise
      *
      ***/
           bool           needToLoadObject(void**       templateObjectToRead);

    /***
      *
      *  In the case of needToLoadObject() return true, the client
      *  application needs to instantiate an expected template object, and
      *  register the address to the engine.
      *
      *  Param
      *    objectPtr:     the template object pointer newly instantiated
      *
      *  Return:
      *
      ***/
           void           registerObject(void* const templateObjectToRegister);

    /***
      *
      *  Insertion operator for serializable classes
      *
      ***/

	friend XSerializeEngine& operator<<(XSerializeEngine&
                                      , XSerializable* const );

    /***
      *
      *  Insertion operators for
      *     . basic Xerces data types
      *     . built-in types
      *
      ***/
           XSerializeEngine& operator<<(XMLByte);
           XSerializeEngine& operator<<(XMLCh);

           XSerializeEngine& operator<<(char);
           XSerializeEngine& operator<<(short);
           XSerializeEngine& operator<<(int);
           XSerializeEngine& operator<<(unsigned int);
           XSerializeEngine& operator<<(long);
           XSerializeEngine& operator<<(unsigned long);
           XSerializeEngine& operator<<(float);
           XSerializeEngine& operator<<(double);
           XSerializeEngine& operator<<(bool);

    // These cannot be done as operators since on some platforms they
    // may collide with int/long types.
    //
    void writeSize (XMLSize_t);
    void writeInt64 (XMLInt64);
    void writeUInt64 (XMLUInt64);


    /***
      *
      *  Extraction operators for
      *     . basic Xerces data types
      *     . built-in types
      *
      ***/
           XSerializeEngine& operator>>(XMLByte&);
           XSerializeEngine& operator>>(XMLCh&);

           XSerializeEngine& operator>>(char&);
           XSerializeEngine& operator>>(short&);
           XSerializeEngine& operator>>(int&);
           XSerializeEngine& operator>>(unsigned int&);
           XSerializeEngine& operator>>(long&);
           XSerializeEngine& operator>>(unsigned long&);
           XSerializeEngine& operator>>(float&);
           XSerializeEngine& operator>>(double&);
           XSerializeEngine& operator>>(bool&);

    void readSize (XMLSize_t&);
    void readInt64 (XMLInt64&);
    void readUInt64 (XMLUInt64&);

    /***
      *
      *  Getters
      *
      ***/
    inline
    XMLSize_t       getBufSize()    const;

    inline
    XMLSize_t       getBufCur()     const;

    inline
    XMLSize_t       getBufCurAccumulated()     const;

    inline
    unsigned long   getBufCount()    const;

    void                  trace(char*)     const;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
	XSerializeEngine();
    XSerializeEngine(const XSerializeEngine&);
	XSerializeEngine& operator=(const XSerializeEngine&);

    /***
      *
      *   Store Pool Opertions
      *
      ***/
           XSerializedObjectId_t  lookupStorePool(void* const objectPtr) const;
           void                   addStorePool(void* const objectPtr);

    /***
      *
      *   Load Pool Opertions
      *
      ***/
           XSerializable* lookupLoadPool(XSerializedObjectId_t objectTag) const;
           void           addLoadPool(void* const objectPtr);

    /***
      *
      *    Intenal Buffer Operations
      *
      ***/
    inline void           checkAndFillBuffer(XMLSize_t bytesNeedToRead);

    inline void           checkAndFlushBuffer(XMLSize_t bytesNeedToWrite);

           void           fillBuffer();

           void           flushBuffer();

           void           pumpCount();

    inline void           resetBuffer();

    /***
      *
      *    Helper
      *
      ***/
    inline void            ensureStoring()                          const;

    inline void            ensureLoading()                          const;

    inline void            ensureStoreBuffer()                      const;

    inline void            ensureLoadBuffer()                       const;

    inline void            ensurePointer(void* const)               const;

    inline void            Assert(bool  toEval
                                , const XMLExcepts::Codes toThrow)  const;


    inline XMLSize_t       calBytesNeeded(XMLSize_t)  const;

    inline XMLSize_t       alignAdjust(XMLSize_t)     const;

    inline void            alignBufCur(XMLSize_t);

    // Make XTemplateSerializer friend of XSerializeEngine so that
    // we can call lookupStorePool and lookupLoadPool in the case of
    // annotations.
    friend class XTemplateSerializer;

    // -------------------------------------------------------------------------------
    //  data
    //
    //  fStoreLoad:
    //               Indicator: storing(serialization) or loading(de-serialization)
    //
    //  fStorerLevel:
    //              The level of the serialize engine which created the binary
    //              stream that this serialize engine is loading
    //
    //              It is set by GrammarPool when loading
    //
    //  fGrammarPool:
    //               Thw owning GrammarPool which instantiate this SerializeEngine
    //               instance
    //
    //  fInputStream:
    //               Binary stream to read from (de-serialization), provided
    //               by client application, not owned.
    //
    //  fOutputStream:
    //               Binary stream to write to (serialization), provided
    //               by client application, not owned.
    //
    //  fBufSize:
    //               The size of the internal buffer
    //
    //  fBufStart/fBufEnd:
    //
    //               The internal buffer.
    //  fBufEnd:
    //               one beyond the last valid cell
    //               fBufEnd === (fBufStart + fBufSize)
    //
    //  fBufCur:
    //               The cursor of the buffer
    //
    //  fBufLoadMax:
    //               Indicating the end of the valid content in the buffer
    //
    //  fStorePool:
    //                Object collection for storing
    //
    //  fLoadPool:
    //                Object collection for loading
    //
    //  fMapCount:
    // -------------------------------------------------------------------------------
    const short                            fStoreLoad;
    unsigned int                           fStorerLevel;

    XMLGrammarPool*  const                 fGrammarPool;
    BinInputStream*  const                 fInputStream;
    BinOutputStream* const                 fOutputStream;

    unsigned long                          fBufCount;

    //buffer
    const XMLSize_t                        fBufSize;
	XMLByte* const                         fBufStart;
	XMLByte* const                         fBufEnd;
    XMLByte*                               fBufCur;
    XMLByte*                               fBufLoadMax;



    /***
     *   Map for storing object
     *
     *   key:   XSerializable*
     *          XProtoType*
     *
     *   value: XMLInteger*, owned
     *
     ***/
    RefHashTableOf<XSerializedObjectId, PtrHasher>*   fStorePool;

    /***
     *   Vector for loading object, objects are NOT owned
     *
     *   data:   XSerializable*
     *           XProtoType*
     *
     ***/
    ValueVectorOf<void*>*                  fLoadPool;

    /***
     *   object counter
     ***/
	XSerializedObjectId_t                  fObjectCount;

    //to allow grammar pool to set storer level when loading
    friend class XMLGrammarPoolImpl;
};

inline bool XSerializeEngine::isStoring() const
{
    return (fStoreLoad == mode_Store);
}

inline bool XSerializeEngine::isLoading() const
{
    return (fStoreLoad == mode_Load);
}

inline XSerializeEngine& operator<<(XSerializeEngine&       serEng
                                  , XSerializable* const    serObj)
{
	serEng.write(serObj);
    return serEng;
}

inline void XSerializeEngine::ensureStoring() const
{
	Assert(isStoring(), XMLExcepts::XSer_Storing_Violation);
}

inline void XSerializeEngine::ensureLoading() const
{
	Assert(isLoading(), XMLExcepts::XSer_Loading_Violation);
}



inline void XSerializeEngine::Assert(bool toEval
                                   , const XMLExcepts::Codes toThrow) const
{
    if (!toEval)
    {
        ThrowXMLwithMemMgr(XSerializationException, toThrow, getMemoryManager());
    }

}

inline void XSerializeEngine::readString(XMLCh*&        toRead
                                       , XMLSize_t&     bufferLen)
{
    XMLSize_t dummyDataLen;
    readString(toRead, bufferLen, dummyDataLen);
}

inline void XSerializeEngine::readString(XMLCh*&        toRead)
{
    XMLSize_t dummyBufferLen;
    XMLSize_t dummyDataLen;
    readString(toRead, dummyBufferLen, dummyDataLen);
}

inline void XSerializeEngine::readString(XMLByte*&      toRead
                                       , XMLSize_t&     bufferLen)
{
    XMLSize_t dummyDataLen;
    readString(toRead, bufferLen, dummyDataLen);
}

inline void XSerializeEngine::readString(XMLByte*&      toRead)
{
    XMLSize_t dummyBufferLen;
    XMLSize_t dummyDataLen;
    readString(toRead, dummyBufferLen, dummyDataLen);
}

inline
XMLSize_t XSerializeEngine::getBufSize() const
{
    return fBufSize;
}

inline
XMLSize_t XSerializeEngine::getBufCur() const
{
    return (fBufCur-fBufStart);
}

inline
XMLSize_t XSerializeEngine::getBufCurAccumulated() const
{
    return (fBufCount - (isStoring() ? 0: 1)) * fBufSize + (fBufCur-fBufStart);
}

inline
unsigned long XSerializeEngine::getBufCount() const
{
    return fBufCount;
}

inline
unsigned int XSerializeEngine::getStorerLevel() const
{
    return fStorerLevel;
}

/***
 *  Ought to be nested class
 ***/
class XSerializedObjectId : public XMemory
{
public:

    ~XSerializedObjectId(){};

private:

    inline XSerializedObjectId(XSerializeEngine::XSerializedObjectId_t val):
        fData(val) { };

    inline XSerializeEngine::XSerializedObjectId_t getValue() const {return fData; };

    friend class XSerializeEngine;

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
	XSerializedObjectId();
    XSerializedObjectId(const XSerializedObjectId&);
	XSerializedObjectId& operator=(const XSerializedObjectId&);

    XSerializeEngine::XSerializedObjectId_t    fData;

};


XERCES_CPP_NAMESPACE_END

#endif
