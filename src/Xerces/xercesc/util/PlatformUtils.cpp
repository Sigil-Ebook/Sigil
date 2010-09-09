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
 * $Id: PlatformUtils.cpp 932877 2010-04-11 12:17:34Z borisk $
 *
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#if HAVE_CONFIG_H
#	include <config.h>
#endif

#if HAVE_LIMITS_H
#	include <limits.h>
#endif
#if HAVE_SYS_TIME_H
#	include <sys/time.h>
#endif
#if HAVE_SYS_TIMEB_H
#	include <sys/timeb.h>
#endif
#if HAVE_CPUID_H && !XERCES_HAVE_INTRIN_H
#   include <cpuid.h>
#endif

#include <xercesc/util/Mutexes.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/RefVectorOf.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/internal/XMLReader.hpp>
#include <xercesc/util/RuntimeException.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/DefaultPanicHandler.hpp>
#include <xercesc/util/XMLInitializer.hpp>
#include <xercesc/internal/MemoryManagerImpl.hpp>

#if XERCES_HAVE_INTRIN_H
#   include <intrin.h>
#endif

#include <xercesc/util/XMLFileMgr.hpp>
#if XERCES_USE_FILEMGR_POSIX
#	include <xercesc/util/FileManagers/PosixFileMgr.hpp>
#endif
#if XERCES_USE_FILEMGR_WINDOWS
#	include <xercesc/util/FileManagers/WindowsFileMgr.hpp>
#endif

#include <xercesc/util/XMLMutexMgr.hpp>
#if XERCES_USE_MUTEXMGR_NOTHREAD
#	include <xercesc/util/MutexManagers/NoThreadMutexMgr.hpp>
#endif
#if XERCES_USE_MUTEXMGR_POSIX
#	include <xercesc/util/MutexManagers/PosixMutexMgr.hpp>
#endif
#if XERCES_USE_MUTEXMGR_WINDOWS
#	include <xercesc/util/MutexManagers/WindowsMutexMgr.hpp>
#endif

#include <xercesc/util/XMLNetAccessor.hpp>
#if XERCES_USE_NETACCESSOR_CURL
#	include <xercesc/util/NetAccessors/Curl/CurlNetAccessor.hpp>
#endif
#if XERCES_USE_NETACCESSOR_SOCKET
#	include <xercesc/util/NetAccessors/Socket/SocketNetAccessor.hpp>
#endif
#if XERCES_USE_NETACCESSOR_CFURL
#	include <xercesc/util/NetAccessors/MacOSURLAccessCF/MacOSURLAccessCF.hpp>
#endif
#if XERCES_USE_NETACCESSOR_WINSOCK
#	include <xercesc/util/NetAccessors/WinSock/WinSockNetAccessor.hpp>
#endif


#include <xercesc/util/XMLMsgLoader.hpp>
#if XERCES_USE_MSGLOADER_ICU
#	include <xercesc/util/MsgLoaders/ICU/ICUMsgLoader.hpp>
#endif
#if XERCES_USE_MSGLOADER_ICONV
#	include <xercesc/util/MsgLoaders/MsgCatalog/MsgCatalogLoader.hpp>
#endif
#if XERCES_USE_MSGLOADER_INMEMORY
#	include <xercesc/util/MsgLoaders/InMemory/InMemMsgLoader.hpp>
#endif
#if XERCES_USE_WIN32_MSGLOADER
#	include <xercesc/util/MsgLoaders/Win32/Win32MsgLoader.hpp>
#endif

#include <xercesc/util/TransService.hpp>
#if XERCES_USE_TRANSCODER_ICU
#	include <xercesc/util/Transcoders/ICU/ICUTransService.hpp>
#endif
#if XERCES_USE_TRANSCODER_GNUICONV
#	include <xercesc/util/Transcoders/IconvGNU/IconvGNUTransService.hpp>
#endif
#if XERCES_USE_TRANSCODER_ICONV
#	include <xercesc/util/Transcoders/Iconv/IconvTransService.hpp>
#endif
#if XERCES_USE_TRANSCODER_MACOSUNICODECONVERTER
#	include <xercesc/util/Transcoders/MacOSUnicodeConverter/MacOSUnicodeConverter.hpp>
#endif
#if XERCES_USE_TRANSCODER_WINDOWS
#	include <xercesc/util/Transcoders/Win32/Win32TransService.hpp>
#endif

XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local data members
//
//  gSyncMutex
//      This is a mutex that will be used to synchronize access to some of
//      the static data of the platform utilities class and here locally.
// ---------------------------------------------------------------------------
static XMLMutex*                gSyncMutex = 0;
static long                     gInitFlag = 0;


// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Static Data Members
// ---------------------------------------------------------------------------
XMLNetAccessor*         XMLPlatformUtils::fgNetAccessor = 0;
XMLTransService*        XMLPlatformUtils::fgTransService = 0;
#ifdef OS390
XMLTransService*        XMLPlatformUtils::fgTransService2 = 0;
#endif
PanicHandler*           XMLPlatformUtils::fgUserPanicHandler = 0;
PanicHandler*           XMLPlatformUtils::fgDefaultPanicHandler = 0;
MemoryManager*          XMLPlatformUtils::fgMemoryManager = 0;
bool                    XMLPlatformUtils::fgMemMgrAdopted = true;

XMLFileMgr*             XMLPlatformUtils::fgFileMgr = 0;
XMLMutexMgr*            XMLPlatformUtils::fgMutexMgr = 0;

XMLMutex*               XMLPlatformUtils::fgAtomicMutex = 0;

bool                    XMLPlatformUtils::fgXMLChBigEndian = true;
bool                    XMLPlatformUtils::fgSSE2ok = false;

// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Init/term methods
// ---------------------------------------------------------------------------
void XMLPlatformUtils::Initialize(const char*          const locale
                                , const char*          const nlsHome
                                ,       PanicHandler*  const panicHandler
                                ,       MemoryManager* const memoryManager)
{
    //
    //  Effects of overflow:
    //  . resouce re-allocations
    //  . consequently resource leaks
    //  . potentially terminate() may never get executed
    //
    //  We got to prevent overflow from happening.
    //  no error or exception
    //
    if (gInitFlag == LONG_MAX)
        return;

    //
    //  Make sure we haven't already been initialized. Note that this is not
    //  thread safe and is not intended for that. Its more for those COM
    //  like processes that cannot keep up with whether they have initialized
    //  us yet or not.
    //
    gInitFlag++;

    if (gInitFlag > 1)
      return;

    // Set pluggable memory manager
    if (!fgMemoryManager)
    {
        if (memoryManager)
        {
            fgMemoryManager = memoryManager;
            fgMemMgrAdopted = false;
        }
        else
        {
            fgMemoryManager = new MemoryManagerImpl();
        }
    }

    /***
     * Panic Handler:
     *
     ***/
    if (!panicHandler)
    {
        fgDefaultPanicHandler = new DefaultPanicHandler();
    }
    else
    {
        fgUserPanicHandler = panicHandler;
    }


    // Determine our endianness (with regard to a XMLCh 16-bit word)
    union {
    	XMLCh ch;
    	unsigned char ar[sizeof(XMLCh)];
    } endianTest;
    endianTest.ch = 1;
    fgXMLChBigEndian = (endianTest.ar[sizeof(XMLCh)-1] == 1);

    // Determine if we can use SSE2 functions
#if defined(XERCES_HAVE_CPUID_INTRINSIC)
    int CPUInfo[4]={0};
    __cpuid(CPUInfo, 1);
    if(CPUInfo[3] & (1UL << 26))
        fgSSE2ok = true;
    else
        fgSSE2ok = false;
#elif defined(XERCES_HAVE_GETCPUID)
    unsigned int eax, ebx, ecx, edx;
    if(!__get_cpuid (1, &eax, &ebx, &ecx, &edx) || (edx & (1UL << 26))==0)
        fgSSE2ok = false;
    else
        fgSSE2ok = true;
#elif defined(XERCES_HAVE_SSE2_INTRINSIC)
    // if we cannot find out at runtime, assume the define has it right
    fgSSE2ok = true;
#else
    fgSSE2ok = false;
#endif

    // Initialize the platform-specific mutex and file mgrs
    fgMutexMgr		= makeMutexMgr(fgMemoryManager);
    fgFileMgr		= makeFileMgr(fgMemoryManager);


    // Create the local sync mutex
    gSyncMutex = new XMLMutex(fgMemoryManager);

    // Create the global "atomic operations" mutex.
    fgAtomicMutex = new XMLMutex(fgMemoryManager);

    //
    //  Ask the per-platform code to make the desired transcoding service for
    //  us to use. This call cannot throw any exceptions or do anything that
    //  cause any transcoding to happen. It should create the service and
    //  return it or zero if it cannot.
    //
    //  This one also cannot use any utility services. It can only create a
    //  transcoding service object and return it.
    //
    //  If we cannot make one, then we call panic to end the process.
    //
    XMLInitializer::initializeTransService(); // TransService static data.

    fgTransService = makeTransService();

    if (!fgTransService)
        panic(PanicHandler::Panic_NoTransService);

    // Initialize the transcoder service
    fgTransService->initTransService();

    //
    //  Try to create a default local code page transcoder. This is the one
    //  that will be used internally by the XMLString class. If we cannot
    //  create one, then call the panic method.
    //
    XMLLCPTranscoder* defXCode = XMLPlatformUtils::fgTransService->makeNewLCPTranscoder(fgMemoryManager);
    if (!defXCode)
        panic(PanicHandler::Panic_NoDefTranscoder);
    XMLString::initString(defXCode, fgMemoryManager);

    //
    //  Now lets ask the per-platform code to give us an instance of the type
    //  of network access implementation he wants to use. This can return
    //  a zero pointer if this platform doesn't want to support this.
    //
    fgNetAccessor = makeNetAccessor();

    /***
     * Message Loader:
     *
     *     Locale setting
     *     nlsHome setting
     ***/
    XMLMsgLoader::setLocale(locale);
    XMLMsgLoader::setNLSHome(nlsHome);

    // Initialize static data.
    //
    XMLInitializer::initializeStaticData();
}

void XMLPlatformUtils::Initialize(XMLSize_t initialDOMHeapAllocSize
                                , XMLSize_t maxDOMHeapAllocSize
                                , XMLSize_t maxDOMSubAllocationSize
                                , const char*          const locale
                                , const char*          const nlsHome
                                ,       PanicHandler*  const panicHandler
                                ,       MemoryManager* const memoryManager)
{
  Initialize (locale, nlsHome, panicHandler, memoryManager);

  // Don't change the parameters unless it is the first time.
  //
  if (gInitFlag == 1)
    XMLInitializer::initializeDOMHeap(initialDOMHeapAllocSize,
                                      maxDOMHeapAllocSize,
                                      maxDOMSubAllocationSize);
}

void XMLPlatformUtils::Terminate()
{
    //
    // To prevent it from running underflow.
    // otherwise we come to delete non-existing resources.
    //
    //  no error or exception
    //
    if (gInitFlag == 0)
        return;

    gInitFlag--;

    if (gInitFlag > 0)
	return;

    // Terminate static data.
    //
    XMLInitializer::terminateStaticData();

    // Delete any net accessor that got installed
    delete fgNetAccessor;
    fgNetAccessor = 0;

    //
    //  Call some other internal modules to give them a chance to clean up.
    //  Do the string class last in case something tries to use it during
    //  cleanup.
    //
    XMLString::termString();

    // Clean up the the transcoding service
    delete fgTransService;
    fgTransService = 0;

    XMLInitializer::terminateTransService(); // TransService static data.

    // Clean up mutexes
    delete gSyncMutex;		gSyncMutex = 0;
    delete fgAtomicMutex;	fgAtomicMutex = 0;

    // Clean up our mgrs
    delete fgFileMgr;		fgFileMgr = 0;
    delete fgMutexMgr;		fgMutexMgr = 0;

    /***
     *  de-allocate resource
     *
     *  refer to discussion in the Initialize()
     ***/
    XMLMsgLoader::setLocale(0);
    XMLMsgLoader::setNLSHome(0);

    delete fgDefaultPanicHandler;
    fgDefaultPanicHandler = 0;
    fgUserPanicHandler = 0;

    // de-allocate default memory manager
    if (fgMemMgrAdopted)
        delete fgMemoryManager;
    else
        fgMemMgrAdopted = true;

    // set memory manager to 0
    fgMemoryManager = 0;

    // And say we are no longer initialized
    gInitFlag = 0;
}




// ---------------------------------------------------------------------------
//  XMLPlatformUtils: The panic method
// ---------------------------------------------------------------------------
void XMLPlatformUtils::panic(const PanicHandler::PanicReasons reason)
{
    fgUserPanicHandler? fgUserPanicHandler->panic(reason) : fgDefaultPanicHandler->panic(reason);
}



// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Private Static Methods
// ---------------------------------------------------------------------------

XMLNetAccessor* XMLPlatformUtils::makeNetAccessor()
{
	XMLNetAccessor* na = 0;

#if defined (XERCES_USE_NETACCESSOR_CURL)
		na = new CurlNetAccessor();
#elif defined (XERCES_USE_NETACCESSOR_SOCKET)
		na = new SocketNetAccessor();
#elif defined (XERCES_USE_NETACCESSOR_CFURL)
		na = new MacOSURLAccessCF();
#elif defined (XERCES_USE_NETACCESSOR_WINSOCK)
		na = new WinSockNetAccessor();
#endif

	return na;
}


//
//  This method is called by the platform independent part of this class
//  when client code asks to have one of the supported message sets loaded.
//

XMLMsgLoader* XMLPlatformUtils::loadAMsgSet(const XMLCh* const msgDomain)
{
    XMLMsgLoader* ms=0;

    try
    {
	#if defined (XERCES_USE_MSGLOADER_ICU)
		ms = new ICUMsgLoader(msgDomain);
	#elif defined (XERCES_USE_MSGLOADER_ICONV)
		ms = new MsgCatalogLoader(msgDomain);
    #elif defined (XERCES_USE_WIN32_MSGLOADER)
		ms = new Win32MsgLoader(msgDomain);
	#elif defined (XERCES_USE_MSGLOADER_INMEMORY)
		ms = new InMemMsgLoader(msgDomain);
	#else
		#error No MsgLoader configured for platform! You must configure it.
	#endif
    }
    catch(const OutOfMemoryException&)
    {
        throw;
    }
    catch(...)
    {
        panic(PanicHandler::Panic_CantLoadMsgDomain);
    }

    return ms;
}


//
//  This method is called very early in the bootstrapping process. This guy
//  must create a transcoding service and return it. It cannot use any string
//  methods, any transcoding services, throw any exceptions, etc... It just
//  makes a transcoding service and returns it, or returns zero on failure.
//

XMLTransService* XMLPlatformUtils::makeTransService()
{
	XMLTransService* tc = 0;

	#if defined   (XERCES_USE_TRANSCODER_ICU)
		tc = new ICUTransService(fgMemoryManager);
	#elif defined (XERCES_USE_TRANSCODER_GNUICONV)
		tc = new IconvGNUTransService(fgMemoryManager);
	#elif defined (XERCES_USE_TRANSCODER_ICONV)
		tc = new IconvTransService(fgMemoryManager);
	#elif defined (XERCES_USE_TRANSCODER_MACOSUNICODECONVERTER)
		tc = new MacOSUnicodeConverter(fgMemoryManager);
	#elif defined (XERCES_USE_TRANSCODER_WINDOWS)
		tc = new Win32TransService(fgMemoryManager);
	#else
		#error No Transcoder configured for platform! You must configure it.
	#endif

	return tc;
}


// ---------------------------------------------------------------------------
//  XMLPlatformUtils: File Methods
// ---------------------------------------------------------------------------
XMLFileMgr*
XMLPlatformUtils::makeFileMgr(MemoryManager* const memmgr)
{
	XMLFileMgr* mgr = NULL;

	#if XERCES_USE_FILEMGR_POSIX
		mgr = new (memmgr) PosixFileMgr;
	#elif XERCES_USE_FILEMGR_WINDOWS
		mgr = new (memmgr) WindowsFileMgr;
	#else
		#error No File Manager configured for platform! You must configure it.
	#endif

	return mgr;
}


FileHandle
XMLPlatformUtils::openFile(const char* const fileName
                           , MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

	return fgFileMgr->fileOpen(fileName, false, memmgr);
}


FileHandle
XMLPlatformUtils::openFile(const XMLCh* const fileName, MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

	return fgFileMgr->fileOpen(fileName, false, memmgr);
}


FileHandle
XMLPlatformUtils::openFileToWrite(const char* const fileName
                                  , MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

	return fgFileMgr->fileOpen(fileName, true, memmgr);
}


FileHandle
XMLPlatformUtils::openFileToWrite(const XMLCh* const fileName
                                  , MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

 	return fgFileMgr->fileOpen(fileName, true, memmgr);
}


FileHandle
XMLPlatformUtils::openStdInHandle(MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

	return fgFileMgr->openStdIn(memmgr);
}


void
XMLPlatformUtils::closeFile(FileHandle theFile
                            , MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

	fgFileMgr->fileClose(theFile, memmgr);
}

void
XMLPlatformUtils::resetFile(FileHandle theFile
                            , MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

	fgFileMgr->fileReset(theFile, memmgr);
}


XMLFilePos
XMLPlatformUtils::curFilePos(FileHandle theFile
                             , MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

	return fgFileMgr->curPos(theFile, memmgr);
}

XMLFilePos
XMLPlatformUtils::fileSize(FileHandle theFile
                           , MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

	return fgFileMgr->fileSize(theFile, memmgr);
}


XMLSize_t
XMLPlatformUtils::readFileBuffer(   FileHandle      theFile
                                 ,  const XMLSize_t		  toRead
                                 ,        XMLByte* const  toFill
                                 ,  MemoryManager* const  memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

    return fgFileMgr->fileRead(theFile, toRead, toFill, memmgr);
}


void
XMLPlatformUtils::writeBufferToFile(   const   FileHandle   theFile
                                    ,  XMLSize_t	    toWrite
                                    ,  const XMLByte* const toFlush
                                    ,  MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

	fgFileMgr->fileWrite(theFile, toWrite, toFlush, memmgr);
}


// ---------------------------------------------------------------------------
//  XMLPlatformUtils: File system methods
// ---------------------------------------------------------------------------
XMLCh* XMLPlatformUtils::getFullPath(const XMLCh* const srcPath,
                                     MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

	return fgFileMgr->getFullPath(srcPath, memmgr);
}


XMLCh* XMLPlatformUtils::getCurrentDirectory(MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

    return fgFileMgr->getCurrentDirectory(memmgr);
}


bool XMLPlatformUtils::isRelative(const XMLCh* const toCheck
                                  , MemoryManager* const memmgr)
{
    if (!fgFileMgr)
		ThrowXMLwithMemMgr(XMLPlatformUtilsException, XMLExcepts::CPtr_PointerIsZero, memmgr);

    return fgFileMgr->isRelative(toCheck, memmgr);
}


inline bool
XMLPlatformUtils::isAnySlash(XMLCh c)
{
	// As far as we know, all supported Xerces
	// platforms use at least a forward slash
	// as a path delimiter. So we always check for
	// that.
	//
	// If XERCES_PATH_DELIMITER_BACKSLASH evaluates to true,
	// we also consider that as a slash.
	//
	// XERCES_PATH_DELIMITER_BACKSLASH may be set in config.h
	// by configure, or elsewhere by platform-specific
	// code.
    return	(
			false
		 || chForwardSlash == c
	#if XERCES_PATH_DELIMITER_BACKSLASH
		 || chBackSlash == c
	#endif
     		);
}


// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Timing Methods
// ---------------------------------------------------------------------------
unsigned long XMLPlatformUtils::getCurrentMillis()
{
	unsigned long ms = 0;

	// *** TODO: additional platform support?
	#if HAVE_GETTIMEOFDAY
		struct timeval aTime;
		gettimeofday(&aTime, NULL);
		ms = (unsigned long) (aTime.tv_sec * 1000 + aTime.tv_usec / 1000);
	#elif HAVE_FTIME
		timeb aTime;
		ftime(&aTime);
		ms = (unsigned long)(aTime.time*1000 + aTime.millitm);
	#else
		// Make this a warning instead?
		#error No timing support is configured for this platform. You must configure it.
	#endif

	return ms;
}


// -----------------------------------------------------------------------
//  Mutex methods
// -----------------------------------------------------------------------
XMLMutexMgr* XMLPlatformUtils::makeMutexMgr(MemoryManager* const memmgr)
{
	XMLMutexMgr* mgr = NULL;

	#if XERCES_USE_MUTEXMGR_NOTHREAD
		mgr = new (memmgr) NoThreadMutexMgr;
	#elif XERCES_USE_MUTEXMGR_POSIX
		mgr = new (memmgr) PosixMutexMgr;
	#elif XERCES_USE_MUTEXMGR_WINDOWS
		mgr = new (memmgr) WindowsMutexMgr;
	#else
		#error No Mutex Manager configured for platform! You must configure it.
	#endif

	return mgr;
}


XMLMutexHandle XMLPlatformUtils::makeMutex(MemoryManager* const memmgr)
{
    if (!fgMutexMgr)
		XMLPlatformUtils::panic(PanicHandler::Panic_MutexErr);

	return fgMutexMgr->create(memmgr);
}


void XMLPlatformUtils::closeMutex(XMLMutexHandle const mtx, MemoryManager* const memmgr)
{
    if (!fgMutexMgr)
		XMLPlatformUtils::panic(PanicHandler::Panic_MutexErr);

	fgMutexMgr->destroy(mtx, memmgr);
}


void XMLPlatformUtils::lockMutex(XMLMutexHandle const mtx)
{
    if (!fgMutexMgr)
		XMLPlatformUtils::panic(PanicHandler::Panic_MutexErr);

	fgMutexMgr->lock(mtx);
}


void XMLPlatformUtils::unlockMutex(XMLMutexHandle const mtx)
{
    if (!fgMutexMgr)
		XMLPlatformUtils::panic(PanicHandler::Panic_MutexErr);

	fgMutexMgr->unlock(mtx);
}

// ---------------------------------------------------------------------------
//  XMLPlatformUtils: Msg support methods
// ---------------------------------------------------------------------------
XMLMsgLoader* XMLPlatformUtils::loadMsgSet(const XMLCh* const msgDomain)
{
    //
    //  Ask the platform support to load up the correct type of message
    //  loader for the indicated message set. We don't check here whether it
    //  works or not. That's their decision.
    //
    return loadAMsgSet(msgDomain);
}

// ---------------------------------------------------------------------------
//  XMLPlatformUtils: NEL Character Handling
// ---------------------------------------------------------------------------
void XMLPlatformUtils::recognizeNEL(bool state, MemoryManager* const manager) {

    //Make sure initialize has been called
    if (gInitFlag == 0) {
        return;
    }

    if (state) {

        if (!XMLChar1_0::isNELRecognized()) {
            XMLChar1_0::enableNELWS();
        }
    }
    else {

        if (XMLChar1_0::isNELRecognized()) {
            ThrowXMLwithMemMgr(RuntimeException, XMLExcepts::NEL_RepeatedCalls, manager);
        }
    }
}


bool XMLPlatformUtils::isNELRecognized() {

    return XMLChar1_0::isNELRecognized();
}

// ---------------------------------------------------------------------------
//  XMLPlatformUtils: IANA Encoding checking setting
// ---------------------------------------------------------------------------
void XMLPlatformUtils::strictIANAEncoding(const bool state) {

    //Make sure initialize has been called
    if (gInitFlag == 0) {
        return;
    }

    fgTransService->strictIANAEncoding(state);
}


bool XMLPlatformUtils::isStrictIANAEncoding() {

    if (gInitFlag)
        return fgTransService->isStrictIANAEncoding();

    return false;
}

/***
 *
 *  Previously, each <OS>PlatformUtils.cpp has its onw copy of the
 *  method weavePaths(), and almost of them implemented the same logic,
 *  with few platform specific difference, and unfortunately that
 *  implementation was wrong.
 *
 *  The only platform specific issue is slash character.
 *  On all platforms other than Windows, chForwardSlash and chBackSlash
 *  are considered slash, while on Windows, two additional characters,
 *  chYenSign and chWonSign are slash as well.
 *
 *  The idea is to maintain a SINGLE copy of this method rather than
 *  each <OS>PlatformUtils.cpp has its own copy, we introduce a new
 *  method, XMLPlatformUtils::isAnySlash(), to replace the direct checking
 *  code ( if ( c == chForwardSlash || c == chBackSlash).
 *
 *  With this approach, we might have a performance hit since isAnySlash()
 *  is so frequently used in this implementation, so we intend to make it
 *  inline. Then we face a complier issue.
 *
 *  There are two compilation units involved, one is PlatformUtils.cpp and
 *  the other <OS>PlatformUtils.cpp. When PlatformUtils.cp get compiled,
 *  the weavePath(), remove**Slash() have dependency upon isAnySlash() which
 *  is in <OS>PlatformUtils.cpp (and what is worse, it is inlined), so we have
 *  undefined/unresolved symbol: isAnySlash() on AIX/xlc_r, Solaris/cc and
 *  Linux/gcc, while MSVC and HP/aCC are fine with this.
 *
 *  That means we can not place these new methods in PlatformUtils.cpp with
 *  inlined XMLPlatformUtils::isAnySlash() in <OS>PlatformUtils.cpp.
 *
 *  The solution to this is <os>PlatformUtils.cpp will include this file so that
 *  we have only one copy of these methods while get compiled in <os>PlatformUtils
 *  inlined isAnySlash().
 *
 ***/
XMLCh* XMLPlatformUtils::weavePaths(const XMLCh* const    basePath
                                  , const XMLCh* const    relativePath
                                  , MemoryManager* const  manager)

{
    // Create a buffer as large as both parts and empty it
    XMLCh* tmpBuf = (XMLCh*) manager->allocate
    (
        (XMLString::stringLen(basePath)
         + XMLString::stringLen(relativePath) + 2) * sizeof(XMLCh)
    );//new XMLCh[XMLString::stringLen(basePath) + XMLString::stringLen(relativePath) + 2];
    *tmpBuf = 0;

    //
    //  If we have no base path, then just take the relative path as is.
    //
    if ((!basePath) || (!*basePath))
    {
        XMLString::copyString(tmpBuf, relativePath);
        return tmpBuf;
    }

    //
    // Remove anything after the last slash
    //
    const XMLCh* basePtr = basePath + (XMLString::stringLen(basePath) - 1);
    while ((basePtr >= basePath)  &&  ((isAnySlash(*basePtr) == false)))
    {
        basePtr--;
    }

    // There is no relevant base path, so just take the relative part
    if (basePtr < basePath)
    {
        XMLString::copyString(tmpBuf, relativePath);
        return tmpBuf;
    }

    //
    // 1. concatenate the base and relative
    // 2. remove all occurences of "/./"
    // 3. remove all occurences of segment/../ where segment is not ../
	//

    XMLString::subString(tmpBuf, basePath, 0, (basePtr - basePath + 1), manager);
    tmpBuf[basePtr - basePath + 1] = 0;
    XMLString::catString(tmpBuf, relativePath);

    removeDotSlash(tmpBuf, manager);

    removeDotDotSlash(tmpBuf, manager);

    return tmpBuf;

}

//
// Remove all occurences of './' when it is part of '/./'
//
// Since it could be '.\' or other combination on windows ( eg, '.'+chYanSign)
// we can't make use of patterMatch().
//
//
void XMLPlatformUtils::removeDotSlash(XMLCh* const path
                                      , MemoryManager* const manager)
{
    if ((!path) || (!*path))
        return;

    XMLCh* srcPtr = XMLString::replicate(path, manager);
    XMLSize_t srcLen = XMLString::stringLen(srcPtr);
    ArrayJanitor<XMLCh>   janName(srcPtr, manager);
    XMLCh* tarPtr = path;

    while (*srcPtr)
    {
        if ( 3 <= srcLen )
        {
            if ( (isAnySlash(*srcPtr))     &&
                (chPeriod == *(srcPtr+1)) &&
                (isAnySlash(*(srcPtr+2)))  )
            {
                // "\.\x" seen
                // skip the first two, and start from the 3rd,
                // since "\x" could be another "\."
                srcPtr+=2;
                srcLen-=2;
            }
            else
            {
                *tarPtr++ = *srcPtr++;  // eat the current char
                srcLen--;
            }
        }
        else if ( 1 == srcLen )
        {
            *tarPtr++ = *srcPtr++;
        }
        else if ( 2 == srcLen)
        {
            *tarPtr++ = *srcPtr++;
            *tarPtr++ = *srcPtr++;
        }

    }

    *tarPtr = 0;

    return;
}

//
// Remove all occurences of '/segment/../' when segment is not '..'
//
// Cases with extra /../ is left to the underlying file system.
//
void XMLPlatformUtils::removeDotDotSlash(XMLCh* const path
                                         , MemoryManager* const manager)
{
    XMLSize_t pathLen = XMLString::stringLen(path);
    XMLCh* tmp1 = (XMLCh*) manager->allocate
    (
        (pathLen+1) * sizeof(XMLCh)
    );//new XMLCh [pathLen+1];
    ArrayJanitor<XMLCh>   tmp1Name(tmp1, manager);

    XMLCh* tmp2 = (XMLCh*) manager->allocate
    (
        (pathLen+1) * sizeof(XMLCh)
    );//new XMLCh [pathLen+1];
    ArrayJanitor<XMLCh>   tmp2Name(tmp2, manager);

    // remove all "<segment>/../" where "<segment>" is a complete
    // path segment not equal to ".."
    int index = -1;
    int segIndex = -1;
    int offset = 1;

    while ((index = searchSlashDotDotSlash(&(path[offset]))) != -1)
    {
        // Undo offset
        index += offset;

        // Find start of <segment> within substring ending at found point.
        XMLString::subString(tmp1, path, 0, index-1, manager);
        segIndex = index - 1;
        while ((segIndex >= 0) && (!isAnySlash(tmp1[segIndex])))
        {
            segIndex--;
        }

        // Ensure <segment> exists and != ".."
        if (segIndex >= 0                 &&
            (path[segIndex+1] != chPeriod ||
             path[segIndex+2] != chPeriod ||
             segIndex + 3 != index))
        {

            XMLString::subString(tmp1, path, 0, segIndex, manager);
            XMLString::subString(tmp2, path, index+3, XMLString::stringLen(path), manager);

            path[0] = 0;
            XMLString::catString(path, tmp1);
            XMLString::catString(path, tmp2);

            offset = (segIndex == 0 ? 1 : segIndex);
        }
        else
        {
            offset += 4;
        }

    }// while

}

int XMLPlatformUtils::searchSlashDotDotSlash(XMLCh* const srcPath)
{
    if ((!srcPath) || (!*srcPath))
        return -1;

    XMLCh* srcPtr = srcPath;
    XMLSize_t srcLen = XMLString::stringLen(srcPath);
    int    retVal = -1;

    while (*srcPtr)
    {
        if ( 4 <= srcLen )
        {
            if ( (isAnySlash(*srcPtr))     &&
                 (chPeriod == *(srcPtr+1)) &&
                 (chPeriod == *(srcPtr+2)) &&
                 (isAnySlash(*(srcPtr+3)))  )
            {
                retVal = (int)(srcPtr - srcPath);
                break;
            }
            else
            {
                srcPtr++;
                srcLen--;
            }
        }
        else
        {
            break;
        }

    } // while

    return retVal;

}


XERCES_CPP_NAMESPACE_END
