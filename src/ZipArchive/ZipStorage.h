////////////////////////////////////////////////////////////////////////////////
// This source file is part of the ZipArchive library source distribution and
// is Copyrighted 2000 - 2010 by Artpol Software - Tadeusz Dracz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// For the licensing details refer to the License.txt file.
//
// Web Site: http://www.artpol-software.com
////////////////////////////////////////////////////////////////////////////////

/**
* \file ZipStorage.h
* Includes the CZipStorage class.	
*
*/

#if !defined(ZIPARCHIVE_ZIPSTORAGE_DOT_H)
#define ZIPARCHIVE_ZIPSTORAGE_DOT_H

#if _MSC_VER > 1000
	#pragma once
	#if defined ZIP_HAS_DLL
		#pragma warning (push)
		#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
	#endif
#endif

#include "ZipFile.h"	
#include "ZipAutoBuffer.h"
#include "ZipString.h"
#include "ZipMemFile.h"
#include "ZipExport.h"
#include "ZipCallback.h"
#include "BitFlag.h"
#include "ZipSplitNamesHandler.h"
#include "ZipException.h"
#include "ZipCollections.h"

/**
	Represents the storage layer for an archive.
*/
class ZIP_API CZipStorage
{
	friend class CZipArchive;
	friend class CZipCentralDir;
public:
	/**
		Storage state.
	*/
	enum State
	{
		stateOpened			= 0x0001,					///< The storage file is opened.
		stateReadOnly		= 0x0002,					///< The storage file is opened as read-only.
		stateAutoClose		= 0x0004,					///< The storage file will be closed when the storage is closed.
		stateExisting		= 0x0008,					///< The storage file existed before opening.
		stateSegmented		= 0x0010,					///< The current archive is segmented.
		stateSplit			= stateSegmented | 0x0020,	///< The current archive is split.
		stateBinarySplit	= stateSplit	 | 0x0040,	///< The current archive is binary split.
		stateSpan			= stateSegmented | 0x0080	///< The current archive is spanned.
	};	
	
	/**
		The direction of the seeking operation.

		\see
			CZipStorage::Seek
	*/
	enum SeekType
	{
		seekFromBeginning, ///< Start seeking from the beginning of a file.
		seekFromEnd, ///< Start seeking from the end of a file.
		/**
			Start seeking from the current position in the archive.
			This value can cause a volume change when a segmented archive is opened for reading.
		*/
		seekCurrent
	};
	CZipStorage();
	virtual ~CZipStorage();

	void Initialize();
	/**
		Opens a new or existing archive in memory.
		The meaning for the parameters is the same as in the CZipArchive::Open(CZipAbstractFile& , int, bool) method.
	*/
	void Open(CZipAbstractFile& af, int iMode, bool bAutoClose);

	/**
		Opens or creates an archive.

		The meaning for the parameters is the same as in the CZipArchive::Open(LPCTSTR, int, ZIP_SIZE_TYPE) method.
	*/
	void Open(LPCTSTR lpszPathName, int iMode, ZIP_SIZE_TYPE uVolumeSize);


	/**
		Closes a segmented archive in creation and reopens it as an existing segmented archive.
		No modifications are allowed afterwards.
		The archive may also turn out to be a not segmented archive.
	*/
	void FinalizeSegm();

	
	/**
		Called only by CZipCentralDir::Read when opening an existing archive.

		\param	uLastVolume
			The number of the volume the central directory is on.
	*/
	void UpdateSegmMode(ZIP_VOLUME_TYPE uLastVolume);

	/**
		Ensures than in a segmented archive, there is enough free space on the current volume.

		\param uNeeded
			The size of the required free space in bytes.

		\return
			The number of free bytes on the current volume.

	*/
	ZIP_SIZE_TYPE AssureFree(ZIP_SIZE_TYPE uNeeded);

	/**
		Writes a chunk of data to the archive.

		\param	pBuf
			The buffer with data.

		\param	iSize
			The number of bytes to write.

		\param	bAtOnce
			If \c true, the whole chunk must fit in the current volume.
			If there is not enough free space, a volume change is performed.

	*/
	void Write(const void *pBuf, DWORD iSize, bool bAtOnce);

	/** 
		Returns the total size currently occupied by the archive.

		\return
			The length of the current archive file increased by the number of bytes in the write buffer.	
	*/
	ZIP_SIZE_TYPE GetOccupiedSpace() const
	{
		return ZIP_SIZE_TYPE(m_pFile->GetLength() + m_uBytesInWriteBuffer);
	}

	/**
		The same as the CZipArchive::IsClosed method.
	*/
	bool IsClosed(bool bArchive) const 
	{
		if (bArchive)
			return !m_state.IsSetAny(stateOpened);
		else
			// assume not auto-close files always opened
			return !m_pFile || m_state.IsSetAny(stateAutoClose) && m_pFile->IsClosed();
	}

	/**
		Reads a chunk of data from the archive.

		\param	pBuf
			The buffer to receive the data.

		\param	iSize
			The number of bytes to read.

		\param	bAtOnce
			If \c true, no volume change is allowed during reading. 
			If the requested number of bytes cannot be read from a single volume, an exception is thrown.

	*/
	DWORD Read(void* pBuf, DWORD iSize, bool bAtOnce);

	/**
		Returns the position in the file, taking into account the number of bytes in the write buffer 
		and the number of bytes before the archive. 		

		\return 
			The position in the file.

		\note
			For binary split archives, it returns the position from the beginning of the first part.
	*/
	ZIP_SIZE_TYPE GetPosition() const
	{
		ZIP_SIZE_TYPE uPos = (ZIP_SIZE_TYPE)(m_pFile->GetPosition()) + m_uBytesInWriteBuffer;
		if (m_uCurrentVolume == 0)
			uPos -= m_uBytesBeforeZip;
		else if (IsBinarySplit()) // not for the first volume
		{
			ZIP_VOLUME_TYPE uVolume = m_uCurrentVolume;
			ASSERT(m_pCachedSizes->GetSize() > (ZIP_ARRAY_SIZE_TYPE)(uVolume - 1));
			do
			{
				uVolume--;
				uPos += (ZIP_SIZE_TYPE)m_pCachedSizes->GetAt((ZIP_ARRAY_SIZE_TYPE)uVolume);
			}
			while (uVolume > 0);
		}
		return uPos;
	}


	/**
		Flushes the data from the read buffer to the disk.

	*/
	void Flush();


	/**
		Forces any data remaining in the file buffer to be written to the disk.
	*/
	void FlushFile()
	{
		if (!IsReadOnly())
			m_pFile->Flush();
	}

	void FlushBuffers()
	{
		Flush();
		FlushFile();
	}

	/**
		Changes volumes during writing to a segmented archive.

		\param	uNeeded
			The number of bytes needed in the volume.

	*/
	void NextVolume(ZIP_SIZE_TYPE uNeeded);


	/**
		Returns a zero-based number of the current volume.
	*/
	ZIP_VOLUME_TYPE GetCurrentVolume() const {return m_uCurrentVolume;}

 
	/**
		Changes the volume during extract operations.

		\param	uNumber
			A zero-based number of the requested volume.
	*/
	void ChangeVolume(ZIP_VOLUME_TYPE uNumber);

	/**
		Changes the current volume to the next volume during extract operations.
	*/
	void ChangeVolume()
	{
		ChangeVolume((ZIP_VOLUME_TYPE)(m_uCurrentVolume + 1));
	}

	/**
		Changes the current volume to the previous volume during extract operations.
	*/
	void ChangeVolumeDec()
	{
		if (m_uCurrentVolume == 0)
			ThrowError(CZipException::badZipFile);
		ChangeVolume((ZIP_VOLUME_TYPE)(m_uCurrentVolume - 1));
	}
	
	/**
		Returns the value indicating whether the archive is a split archive (binary or regular).

		\return
			\c true, if the archive is a split archive; \c false otherwise.
	*/
	bool IsSplit() const
	{
		return m_state.IsSetAll(stateSplit);
	}

	/**
		Returns the value indicating whether the archive is a binary split archive.

		\return
			\c true, if the archive is a binary split archive; \c false otherwise.
	*/
	bool IsBinarySplit() const
	{
		return m_state.IsSetAll(stateBinarySplit);
	}

	/**
		Returns the value indicating whether the archive is a regular split archive (not binary).

		\return
			\c true, if the archive is a regular split archive; \c false otherwise.
	*/
	bool IsRegularSplit() const
	{
		return m_state.IsSetAll(stateSplit) && !m_state.IsSetAll(stateBinarySplit);
	}

	/**
		Returns the value indicating whether the archive is a spanned archive.

		\return
			\c true, if the archive is a spanned archive; \c false otherwise.
	*/
	bool IsSpanned() const
	{
		return m_state.IsSetAll(stateSpan);
	}

	/**
		The same as the CZipArchive::IsReadOnly method.
	*/
	bool IsReadOnly() const
	{
		return m_state.IsSetAny(stateReadOnly) || IsExistingSegmented();
	}
	
	/**
		Returns the value indicating whether the archive is an existing segmented archive.

		\return
			\c true, if the archive is an existing segmented archive; \c false otherwise.
	*/
	bool IsExistingSegmented() const
	{
		return m_state.IsSetAll(stateSegmented | stateExisting);
	}

	/**
		Returns the value indicating whether the archive is a new segmented archive.

		\return
			\c true, if the archive is a new segmented archive; \c false otherwise.
	*/
	bool IsNewSegmented() const
	{
		return m_state.IsSetAny(stateSegmented) && !IsExisting();
	}

	/**
		Returns the value indicating whether the archive is a segmented archive.

		\return
			\c true, if the archive is a segmented archive; \c false otherwise.
	*/
	bool IsSegmented() const
	{
		return m_state.IsSetAny(stateSegmented);
	}

	/**
		Returns the value indicating whether the archive is an existing archive.

		\return
			\c true, if the archive is an existing archive; \c false, if the archive is a new archive.
	*/
	bool IsExisting() const
	{
		return m_state.IsSetAny(stateExisting);
	}

	/**
		Sets the split names handler.

		\see
			CZipArchive::SetSplitNamesHandler(CZipSplitNamesHandler*, bool)
		\see
			CZipSplitNamesHandler
	*/
	bool SetSplitNamesHandler(CZipSplitNamesHandler* pNames, bool bAutoDelete)
	{
		if (m_state != 0)
		{
			ZIPTRACE("%s(%i) : The archive is already opened.\n");
			return false;
		}
		ClearSplitNames();
		m_pSplitNames = pNames;
		m_bAutoDeleteSplitNames = bAutoDelete;
		return true;
	}

	/**
		Returns the current split names handler.

		\return
			The current split names handler.
		\see
			CZipSplitNamesHandler
	*/
	CZipSplitNamesHandler* GetSplitNamesHandler()
	{
		return m_pSplitNames;
	}

	/**
		Returns the current split names handler (const).

		\return
			The current split names handler.
		\see
			CZipSplitNamesHandler
	*/
	const CZipSplitNamesHandler* GetSplitNamesHandler() const
	{
		return m_pSplitNames;
	}
	
	/**
		Performs the seeking operation on the #m_pFile.

		\param lOff
			The new position in the file.

		\param iSeekType
			The direction of the seek operation.
			It can be one of the #SeekType values.
	*/
	ULONGLONG Seek(ULONGLONG lOff, SeekType iSeekType = seekFromBeginning);	

	/**
		Performs the seeking operation in a binary split archive.

		\param lOff
			The offset to move the file pointer.

		\param bSeekToBegin
			If \c true, the file pointer is moved to the beginning before seeking.
			If \c false, the file pointer is moved relatively to the current position.
	*/
	void SeekInBinary(ZIP_FILE_SIZE lOff, bool bSeekToBegin = false);	

	/**
		Returns the number of free bytes on the current volume.	

		\return 
			The number of free bytes on the current volume.
	*/
	ZIP_SIZE_TYPE VolumeLeft() const;
	
	/**	
		Closes the storage.

		\param	bWrite
			Set to \c false, if the storage should not perform any write operations.
		\param bGetLastVolumeName
			Set to \c true, if the storage should return the path.

		\return
			The file path of the archive or of the last volume in the archive.
			Only if \a bGetLastVolumeName is set to \c true.

	*/
	CZipString Close(bool bWrite, bool bGetLastVolumeName = false);

	/**
		Represents the physical storage for the archive (or the current archive segment in segmented archives).
	*/
	CZipAbstractFile* m_pFile;

	/**
		The signature of the extended header.
	*/
	static char m_gszExtHeaderSignat[];

	ZipArchiveLib::CBitFlag& GetState()
	{
		return m_state;
	}

protected:

	/**
		Returns the file offset after the last data byte in the archive.

		\return 
			The file offset after the last data byte in the archive.
	*/
	ZIP_SIZE_TYPE GetLastDataOffset()
	{
		return (ZIP_SIZE_TYPE)m_pFile->GetLength() - m_uBytesBeforeZip;
	}
	
	/**
		Reverse-finds the location of the given signature starting from the current position in file.

		\param szSignature
			The signature to locate.

		\param uMaxDepth
			The maximum number of bytes to search for \a szSignature.

		\return
			The location of the signature.

	*/
	ZIP_FILE_USIZE LocateSignature(char* szSignature, ZIP_SIZE_TYPE uMaxDepth);
		

	/**
		Flushes without writing. It can be used only on not segmented archives.
	*/
	void EmptyWriteBuffer()
	{
		m_uBytesInWriteBuffer = 0;
	}

	/**
		Opens a physical file.

		\param	lpszName
			The name of the file to open.

		\param	uFlags
			The file open flags.

		\param	bThrow
			If \c true, throw an exception in case of failure.

		\return
			\c true if successful; \c false otherwise.
	*/
	bool OpenFile(LPCTSTR lpszName, UINT uFlags, bool bThrow = true);

	/**
		Renames the last segment file in a split archive when finalizing the archive.

		\return
			The name of the last segment.
	*/
	CZipString RenameLastFileInSplitArchive();

	/**
		Writes data to the internal buffer.

		\param	*pBuf
			The buffer to copy the data from.

		\param	uSize
			The number of bytes to write.

	*/
	void WriteInternalBuffer(const char *pBuf, DWORD uSize);

	/**
		Returns the free space size on the current removable disk.

		\return
			The free space in bytes.
	*/
	ZIP_SIZE_TYPE GetFreeVolumeSpace() const;

	/**
		Calls the segmented callback object.
		Throws an exception if the callback method returns \c false.

		\param uNeeded
			The minimum number of free bytes required on the disk.

		\param	iCode
			The code to be passed to the callback method.

		\param	szTemp
			The string to be used as a filename (as an argument
			in the CZipException::Throw method) when an exception must be thrown.

				\see
			CZipArchive::SetSegmCallback
	*/
	void CallCallback(ZIP_SIZE_TYPE uNeeded, int iCode, CZipString szTemp);

	/**
		Changes a file when processing a split archive.
	*/
	CZipString ChangeSplitRead();

	/**
		Changes a disk when processing a spanned archive.
	*/
	CZipString ChangeSpannedRead();

	/**
		Returns the free space left in the write buffer.

		\return
			The free space in bytes.
	*/
	DWORD GetFreeInBuffer() const {return m_pWriteBuffer.GetSize() - m_uBytesInWriteBuffer;}	

	/**
		The value it holds, depends on the current mode:		
		- An opened existing split archive: the number of the last volume ( usually the one with the "zip" extension).
		- A split archive in creation: the size of the volume.

		This method is used only when processing split archives.
	*/
	ZIP_SIZE_TYPE m_uSplitData;
	
	/**
		The number of bytes available in the write buffer.		
	*/
	DWORD m_uBytesInWriteBuffer;

	/**
		The value it holds depends on the segmentation mode:
		- A split archive: the total size of the current volume.
		- A spanned archive: the free space on the current volume.
	*/
	ZIP_SIZE_TYPE m_uCurrentVolSize;

	/**
		The write buffer caching data.
	*/
	CZipAutoBuffer m_pWriteBuffer;

	/**
		Stores the number of bytes that have been written physically to the current segment.
		Used only when processing a segmented archive in creation.
	*/
	ZIP_SIZE_TYPE m_uBytesWritten;	

	/**
		The current volume number in a segmented archive.
		The value is zero-based.
	*/
	ZIP_VOLUME_TYPE m_uCurrentVolume;
	
	/**
		The number of bytes before the actual zip archive in a file.
		\see
			CZipArchive::GetBytesBeforeZip
	*/
	ZIP_SIZE_TYPE m_uBytesBeforeZip;


	/**
		The size of the write buffer. 		

		\see
			CZipArchive::SetAdvanced
	*/
	int m_iWriteBufferSize;

	/**
		The size of the buffer used in searching for the central directory.

		\see
			CZipArchive::SetAdvanced
	*/
	int m_iLocateBufferSize;		

	/**
		A callback object called when there is a need for a volume change
		in a spanned archive.

		\see
			CZipArchive::SetSegmCallback
	*/
	CZipSegmCallback* m_pSpanChangeVolumeFunc;

	/**
		A callback object called when there is a need for a volume change
		in a split archive.

		\see
			CZipArchive::SetSegmCallback
	*/
	CZipSegmCallback* m_pSplitChangeVolumeFunc;
	
private:
	ZIP_FILE_USIZE LocateSignature(char* szSignature, ZIP_SIZE_TYPE uMaxDepth, int& leftToFind, bool& found, ZIP_FILE_USIZE uFileLength);
	CZipString GetSplitVolumeName(bool bLast)
	{
		if (m_pSplitNames == NULL)
		{
			ThrowError(CZipException::genericError);
		}
		int flags = bLast ? CZipSplitNamesHandler::flLast : CZipSplitNamesHandler::flNone;
		if (IsExisting())
			flags |= CZipSplitNamesHandler::flExisting;
		return m_pSplitNames->GetVolumeName(m_szArchiveName, (ZIP_VOLUME_TYPE)(m_uCurrentVolume + 1), flags);
	}

	void ClearSplitNames()
	{
		if (m_pSplitNames)
		{
			if (m_bAutoDeleteSplitNames)
				delete m_pSplitNames;
			m_pSplitNames = NULL;
			m_bAutoDeleteSplitNames = false;
		}
	}

	void ClearCachedSizes()
	{
		if (m_pCachedSizes)
		{
			delete m_pCachedSizes;
			m_pCachedSizes = NULL;
		}
	}

	void EnsureSplitNames()
	{
		if (IsSplit())
		{
			if (m_pSplitNames == NULL)
			{
				m_bAutoDeleteSplitNames = true;
				if (m_state.IsSetAll(stateBinarySplit))
					m_pSplitNames = new CZipBinSplitNamesHandler();
				else
					m_pSplitNames = new CZipRegularSplitNamesHandler();
			}
			m_pSplitNames->Initialize(m_szArchiveName);
		}
	}

	ZIP_FILE_USIZE GetCachedSize(ZIP_VOLUME_TYPE uVolume)
	{
		ASSERT(m_pCachedSizes);
		if (m_pCachedSizes->GetSize() > (ZIP_ARRAY_SIZE_TYPE)uVolume)
			return m_pCachedSizes->GetAt((ZIP_ARRAY_SIZE_TYPE)uVolume);
		ThrowError(CZipException::genericError);
		// for a compiler
		return 0;
	}

	void CacheSizes();

	ZipArchiveLib::CBitFlag m_state;
	CZipSegmCallback* m_pChangeVolumeFunc;
	CZipString m_szArchiveName;
	CZipFile m_internalfile;
	CZipSplitNamesHandler* m_pSplitNames;
	CZipArray<ZIP_FILE_USIZE>* m_pCachedSizes;
	bool m_bAutoDeleteSplitNames;
	static const ZIP_FILE_USIZE SignatureNotFound;
	void ThrowError(int err) const;
};

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning (pop)	
#endif


#endif // !defined(ZIPARCHIVE_ZIPSTORAGE_DOT_H)
