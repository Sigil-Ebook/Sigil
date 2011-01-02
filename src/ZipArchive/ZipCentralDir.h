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
* \file ZipCentralDir.h
*	Includes the CZipCentralDir class.
*
*/

#if !defined(ZIPARCHIVE_ZIPCENTRALDIR_DOT_H)
#define ZIPARCHIVE_ZIPCENTRALDIR_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning (push)
	#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
#endif

#include "ZipException.h"
#include "ZipFileHeader.h"
#include "ZipAutoBuffer.h"
#include "ZipCollections.h"
#include "ZipCompatibility.h"
#include "ZipExport.h"
#include "ZipCallbackProvider.h"
#include "ZipMutex.h"


class CZipArchive;

/**
	Represents the central directory record in an archive.
*/
class ZIP_API CZipCentralDir  
{
public:

	/**
		Used in fast finding files by the filename.
		A structure for the internal use only.

		\see
			CZipCentralDir::m_pFindArray
		\see
			CZipArchive::FindFile
		\see
			CZipArchive::EnableFindFast
	*/
	struct ZIP_API CZipFindFast
	{
		CZipFindFast()
		{
			m_uIndex = 0;
			m_pHeader= NULL;
		}
		CZipFindFast(CZipFileHeader* pHeader, ZIP_INDEX_TYPE uIndex):m_pHeader(pHeader), m_uIndex(uIndex){}

		/**
			A pointer to the structure in CZipCentralDir. We extract a name from it.
		*/
		CZipFileHeader* m_pHeader;

		/**
			The index in the central directory of the #m_pHeader.
		*/
		ZIP_INDEX_TYPE m_uIndex;
	};
	

	/**
		Stores general information about the central directory.
		Request this information with CZipArchive::GetCentralDirInfo.
	*/
	struct ZIP_API CInfo
	{
		
		/**
			The position of the End of Central Directory Record.
			In the Zip64 it points to the Zip64 counterpart.
		*/
		ZIP_SIZE_TYPE   m_uEndOffset;

		/**
			The zero-based number of the volume with the End of Central Directory Record. To determine the total number of segments in the archive,
			add one to this value.
		*/
		ZIP_VOLUME_TYPE m_uLastVolume;
		ZIP_VOLUME_TYPE m_uVolumeWithCD;		///< The number of the volume with the start of the central directory.
		ZIP_INDEX_TYPE m_uVolumeEntriesNo;	///< The total number of entries in the central directory on the last volume.
		ZIP_INDEX_TYPE m_uEntriesNumber;	///< The total number of entries in the central directory.

		/**
			The size of the central directory. 
			This value is valid only if #m_bInArchive is \c true; in other cases use the #GetSize method instead.
		*/
		ZIP_SIZE_TYPE m_uSize;

		/**
			The offset of the start of the central directory with respect to the starting volume number.
			It is the value written in the central directory record.
			This value is valid only if #m_bInArchive is \c true.
		*/
		ZIP_SIZE_TYPE m_uOffset;


		/**
			This value is \c true if the central directory is physically present in the archive; \c false otherwise.
		*/
		bool m_bInArchive;

	private:
		friend class CZipCentralDir;		
		void Init()
		{
			m_iReference = 1;
#ifdef _ZIP_USE_LOCKING
			m_mutex.Open();
#endif
			m_pCompare = GetCZipStrCompFunc(ZipPlatform::GetSystemCaseSensitivity());
			m_bCaseSensitive = false;
			m_bFindFastEnabled = false;
			m_pszComment.Release();
			// initialize ( necessary when using 64 bits - we are copying only 4 bytes in Read())	
			m_bInArchive = false;
			m_uEndOffset = 0;
			m_uLastVolume = 0;
			m_uVolumeWithCD = 0;
			m_uVolumeEntriesNo = 0;
			m_uEntriesNumber = 0;
			m_uSize = 0;  
			m_uOffset = 0;
			m_iLastIndexAdded = ZIP_FILE_INDEX_UNSPECIFIED;
		}
		bool CheckIfOK_1()
		{
			return (m_uEndOffset >= m_uOffset + m_uSize);
		}
		ZIP_SIZE_TYPE CalculateBytesBeforeZip()
		{
			return m_uEndOffset - m_uSize - m_uOffset;
		}
		bool CheckIfOK_2()
		{
			return (m_uSize || !m_uEntriesNumber) && (m_uEntriesNumber || !m_uSize);
		}
		
		/**
			Returns a value indicating if the current archive properties requires the Zip64 format.
			
			\return
				\c true, if the Zip64 is needed; \c false otherwise.
		
			\see
				<a href="kb">0610051629</a>
		 */
		bool NeedsZip64() const
		{
			return m_uLastVolume >= USHRT_MAX || m_uVolumeWithCD >= USHRT_MAX || m_uVolumeEntriesNo >= USHRT_MAX || m_uEntriesNumber >= USHRT_MAX || m_uSize >= UINT_MAX || m_uOffset >= UINT_MAX;
		}

		CZipAutoBuffer m_pszComment;	///< The global archive comment.		

		/**
			The case-sensitivity of CZipCentralDir::m_pFindArray sorting.
		*/
		bool m_bCaseSensitive;

		/**
			The value set with the CZipCentralDir::EnableFindFast method.
		*/
		bool m_bFindFastEnabled;	

		/**
			The index of the recently added file.
		*/
		ZIP_INDEX_TYPE m_iLastIndexAdded;

	private:
		/**
			The method used in string comparisons. It is set depending on the current case-sensitivity.
		*/
		ZIPSTRINGCOMPARE m_pCompare;
		int m_iReference;
#ifdef _ZIP_USE_LOCKING
		ZipArchiveLib::CZipMutex m_mutex;
#endif
	};

	CZipCentralDir();
	virtual ~CZipCentralDir();

	static char m_gszSignature[]; ///< The End of Central Directory Record signature.
	static char m_gszSignature64Locator[]; ///< The Zip64 End of Central Directory Locator signature.
	CZipFileHeader* m_pOpenedFile;	///< It points to the currently opened file or it is \c NULL, if no file is opened.

	/**
		Initializes the central directory during construction.

		\param pArchive
			The archive that creates the object.

	*/
	void InitOnCreate(CZipArchive* pArchive);

	/**
		Initializes the object.

		\param pSource
			If not \c NULL, it specifies the central directory for sharing.

					
	*/
	void Init(CZipCentralDir* pSource = NULL);

	/**
		Reads the central directory from the archive.
	*/
	void Read();

	/**
		Opens the file with the given index.

		\param uIndex
			A zero-based index of the file to open.

	*/
	void OpenFile(ZIP_INDEX_TYPE uIndex);

	/**	
		Tests if the given file index is valid.

		\param	uIndex
			A zero-based index to test.

		\return
			\c true, if the file with the given index exists inside the archive; \c false otherwise.
	*/
	bool IsValidIndex(ZIP_INDEX_TYPE uIndex)const;

	/**
		Removes a file header from the central directory.

		\param	pHeader
			The header to remove.

		\param uIndex
			The index of the header to remove. Use \c ZIP_FILE_INDEX_UNSPECIFIED, if the index is unknown.

		\param	bShift 
			If \c true, the data inside the archive is moved over the hole created after removing the file.
			If \c false, the unused area inside the archive remains.

	*/
	void RemoveFile(CZipFileHeader* pHeader, ZIP_INDEX_TYPE uIndex = ZIP_FILE_INDEX_UNSPECIFIED, bool bShift = true);


    /**
       Removes last file from the central directory.       

	   \param	pHeader
			The header to remove.

		\param uIndex
			The index of the header to remove. Use \c ZIP_FILE_INDEX_UNSPECIFIED, if the index is unknown.
	   
     */
	void RemoveLastFile(CZipFileHeader* pHeader = NULL, ZIP_INDEX_TYPE uIndex = ZIP_FILE_INDEX_UNSPECIFIED);

	/**
		Removes all files.

	*/
	void RemoveAll();

	/**
		Closes the central directory.
	*/
	void Close();

	/**
		Adds a new file to the central directory.

		\param	header
			Used as a template for the data stored inside the archive.

		\param uReplaceIndex
			The index of the file to be replaced or \c ZIP_FILE_INDEX_UNSPECIFIED, if the index is unknown.

		\param iLevel
			The compression level.
	
		\param bRichHeaderTemplateCopy
			\c true, if copy crc and sizes values from \a header;  \c false otherwise.

		\return
			The new header.

	*/	
	CZipFileHeader* AddNewFile(const CZipFileHeader & header, ZIP_INDEX_TYPE uReplaceIndex, int iLevel, bool bRichHeaderTemplateCopy = false);

	/** 
		Returns the index of the recently added file (if any).

		\return
			The index of the recently added file or \c ZIP_FILE_INDEX_UNSPECIFIED if the index is unknown.
	*/
	ZIP_INDEX_TYPE GetLastIndexAdded() const
	{
		return m_pInfo ? m_pInfo->m_iLastIndexAdded : ZIP_FILE_INDEX_UNSPECIFIED;
	}

	/**
		Removes physically the central directory from the archive.

	*/
	void RemoveFromDisk();

	/**
		Returns the central directory size.
		\param	bWhole
			If \c true, include the size of the file headers.
			If \c false, the size of the file headers is not included.

		\return
			The size of the central directory.

		\see
			CZipArchive::GetCentralDirSize
	*/
	ZIP_SIZE_TYPE GetSize(bool bWhole = false) const;

	/**
		Closes a file opened for reading inside the archive.
		\param skipCheckingDataDescriptor
			If \c true, the data descriptor that is located after the compressed data in the archive is checked for validity.
			Set this value to \c false after closing the file after an exception was thrown.

	*/
	void CloseFile(bool skipCheckingDataDescriptor = false);

	/**
		Closes a file opened for reading inside the archive.

	*/
	void CloseNewFile();

	/**
		Writes the central directory to the archive.
		
	*/
	void Write();
	
	/**
		Enables Find Fast.

		\see
			CZipArchive::EnableFindFast
	*/
	void EnableFindFast(bool bEnable, bool bCaseSensitive);

	/**
		Searches for the file.

		\see
			CZipArchive::FindFile
	*/
	ZIP_INDEX_TYPE FindFile(LPCTSTR lpszFileName, bool bCaseSensitive, bool bSporadically, bool bFileNameOnly);


	/**
		Returns the Find Fast index.

		\see
			CZipArchive::GetFindFastIndex
	*/
	ZIP_INDEX_TYPE GetFindFastIndex(ZIP_INDEX_TYPE uFindFastIndex)const
	{
		if (!IsValidIndex(uFindFastIndex) || !m_pInfo->m_bFindFastEnabled)
			return ZIP_FILE_INDEX_NOT_FOUND;
		
		return (*m_pFindArray)[(ZIP_ARRAY_SIZE_TYPE)uFindFastIndex]->m_uIndex;
	}


	/**
		Returns the information about the file with the given index.

		\see
			CZipArchive::operator[](ZIP_INDEX_TYPE)
	*/
	CZipFileHeader* operator[](ZIP_INDEX_TYPE uIndex)
	{
		return (*m_pHeaders)[(ZIP_ARRAY_SIZE_TYPE)uIndex];
	}

	/**
		Returns the information about the file with the given index.

		\see
			CZipArchive::operator[](ZIP_INDEX_TYPE) const
	*/
	const CZipFileHeader* operator[](ZIP_INDEX_TYPE uIndex) const
	{
		return (*m_pHeaders)[(ZIP_ARRAY_SIZE_TYPE)uIndex];
	}

	/**
		Returns the number of files in the archive.

		\return
			The number of files in the archive.
			
	*/
	ZIP_ARRAY_SIZE_TYPE GetCount() const
	{
		return m_pHeaders == NULL ? 0 : m_pHeaders->GetSize();
	}
	
	/**
		Sets the global comment.

		\see
			CZipArchive::SetGlobalComment
	*/
	void SetComment(LPCTSTR lpszComment, UINT codePage);

	/**
		Returns the global comment.

		\see
			CZipArchive::GetGlobalComment
	*/
	void GetComment(CZipString& szComment) const;

	

	/**
		Finds the index of the file with the given name.

		\param	lpszFileName
			The name of the file to find.

		\return
			The index in CZipCentralDir::m_pFindArray with the corresponding CZipFindFast structure
			or \c ZIP_FILE_INDEX_NOT_FOUND, if there is no such file with the given name.

		\see
			CZipArchive::FindFile
	*/
	ZIP_INDEX_TYPE FindFileNameIndex(LPCTSTR lpszFileName) const;

	/**
		Returns the information about the central directory.
			
		\see
			CZipArchive::GetCentralDirInfo
	*/
	void GetInfo(CInfo& info) const {info = *m_pInfo;}

	/**
		Returns the value indicating whether the Find Fast feature is enabled.

		\return
			The value of CInfo::m_bFindFastEnabled.
	*/
	bool IsFindFastEnabled(){return m_pInfo->m_bFindFastEnabled;}
	
	/**
		Rebuilds the CZipCentralDir::m_pFindArray array.
	*/
	void RebuildFindFastArray()
	{
		if (m_pInfo->m_bFindFastEnabled)
			BuildFindFastArray(m_pInfo->m_bCaseSensitive);
	}

	/**
		Consistency checks to ignore. It can be one or more of the CZipArchive::ConsistencyCheck values.

		\see 
			CZipArchive::SetIgnoredConsistencyChecks
	*/
	int m_iIgnoredChecks;

	/**
		The currently set special flags.

		\see
			CZipArchive::SetSpecialFlags
	*/
	ZipArchiveLib::CBitFlag m_specialFlags;

	/**
		Returns the value indicating whether the specified consistency check should be performed.

		\param iLevel
			The level to check. It can be one or more of the CZipArchive::ConsistencyCheck values.

		\return
			\c true, if the specified check should be performed; \c false otherwise.

		\see
			m_iIgnoredChecks
	*/
	bool IsConsistencyCheckOn(int iLevel)
	{
		// check, if not ignored
		return (m_iIgnoredChecks & iLevel) == 0;
	}

	/**
		Returns the current storage.

		\return 
			The current storage.
	*/
	CZipStorage* GetStorage() {return m_pStorage;}
#ifdef _ZIP_UNICODE_CUSTOM
	/**
		Returns the current string store settings.

		\see
			CZipArchive::GetStringStoreSettings
	*/
	const CZipStringStoreSettings& GetStringStoreSettings() const;
#endif
	/**
		Called when a filename of a file in the archive changes.

		\return
			\c true, if the change is permitted; \c false otherwise.
			
	*/
	bool OnFileNameChange(CZipFileHeader* pHeader);

	/**
		Called when data of a file changes in a central directory file header.

		\return
			\c true, if the change is permitted; \c false otherwise.
			
	*/
	bool OnFileCentralChange();

	/**
		Sets the current Unicode mode.

		\see
			CZipArchive::SetUnicodeMode
	*/
	void SetUnicodeMode(int iMode) {m_iUnicodeMode = iMode;}

	/**
		Returns the current Unicode mode.

		\see
			CZipArchive::GetUnicodeMode
	*/
	int GetUnicodeMode() const {return m_iUnicodeMode;}		
	
	/**
		Examines whether any file is modified.

		\return
			\c true, if there are modified files; \c false otherwise.

		\see
			CZipArchive::IsModified
	*/
	bool IsAnyFileModified() const;

	/**
		Throws an exception with the given code.
	*/
	void ThrowError(int err) const;

	CZipArchive* GetArchive() 
	{
		return m_pArchive;
	}

	ZIP_FILE_USIZE LocateSignature() ;
protected:

	/**
		The reference to the main CZipArchive object.
	*/
	CZipArchive* m_pArchive;

	/**
		Points to CZipArchive::m_storage.
	*/
	CZipStorage* m_pStorage;
	

#if _MSC_VER > 1000
	#pragma warning( push )
	#pragma warning (disable : 4702) // unreachable code
#endif
	

	/**
		The current Unicode mode.
	*/
	int m_iUnicodeMode;
	
	/**
		The method used in comparison when sorting headers.
	*/
	static int CompareHeaders(const void *pArg1, const void *pArg2)
	{
		CZipFileHeader* pw1 = *(CZipFileHeader**)pArg1;
		CZipFileHeader* pw2 = *(CZipFileHeader**)pArg2;
		if (pw1 == pw2)
			return 0;

		if (pw1->m_uVolumeStart == pw2->m_uVolumeStart)
		{
			if (pw1->m_uOffset < pw2->m_uOffset)
				return -1;
			else if (pw1->m_uOffset > pw2->m_uOffset)
				return 1;
				ASSERT(FALSE);


			// two files with the same offsets in the same volume???
			CZipException::Throw(CZipException::badZipFile);
			return 0; // just for the compiler comfort (and discomfort of another)
		}
		else if (pw1->m_uVolumeStart < pw2->m_uVolumeStart)
			return -1;
		else // if (pw1->m_uVolumeStart > pw2->m_uVolumeStart)
			return 1;		
	}

#if _MSC_VER > 1000
	#pragma warning( pop )
#endif

	static int CompareFindFastCollate(const void* pArg1, const void* pArg2)
	{
		CZipFindFast* pHeader1 = *(CZipFindFast**)pArg1;
		CZipFindFast* pHeader2 = *(CZipFindFast**)pArg2;		
		return pHeader1->m_pHeader->GetFileName().Collate(pHeader2->m_pHeader->GetFileName());
	}

	static int CompareFindFastCollateNoCase(const void* pArg1, const void* pArg2)
	{
		CZipFindFast* pHeader1 = *(CZipFindFast**)pArg1;
		CZipFindFast* pHeader2 = *(CZipFindFast**)pArg2;		
		return pHeader1->m_pHeader->GetFileName().CollateNoCase(pHeader2->m_pHeader->GetFileName());
	}

	/**
		Holds the information about all files inside the archive.

		\see
			CZipFileHeader
	*/
	CZipArray<CZipFileHeader*>* m_pHeaders;


	/**
		Builds the CZipCentralDir::m_pFindArray array.
	*/
	void BuildFindFastArray( bool bCaseSensitive );

	void ClearFindFastArray()
	{
		ZIP_ARRAY_SIZE_TYPE uCount = m_pFindArray->GetSize();
		for (ZIP_ARRAY_SIZE_TYPE i = 0; i < uCount; i++)
			delete (*m_pFindArray)[i];
		m_pFindArray->RemoveAll();
	}

	/**
		The Find Fast array.

		\see
			CZipFindFast
		\see
			CInfo::m_bFindFastEnabled
		\see
			CZipArchive::FindFile
	*/
	CZipArray<CZipFindFast*>* m_pFindArray;


	/**
		The method used in comparison involving the Find Fast feature.

		\param	lpszFileName
			The name of the file.

		\param	uIndex
			The index from the CZipCentralDir::m_pFindArray array.

		\return 
			The return value has the following meaning:
			- 0 if the filenames are the same
			- < 0 if the filename stored in the array is less than \a lpszFileName
			- > 0 if the filename stored in the array is greater than \a lpszFileName
	*/
	int CompareElement(LPCTSTR lpszFileName, ZIP_INDEX_TYPE uIndex) const
	{
		return ((*m_pFindArray)[(ZIP_ARRAY_SIZE_TYPE)uIndex]->m_pHeader->GetFileName().*(m_pInfo->m_pCompare))(lpszFileName);
	}

	/**
		Inserts a new CZipFindFast element to the CZipCentralDir::m_pFindArray array.
		Initializes the CZipFindFast object with \a pHeader and \a uIndex values.

		\param pHeader
			The element to insert.

		\param uIndex
			The original index of \a pHeader in the central directory. 
			If set to \c ZIP_FILE_INDEX_UNSPECIFIED, it is assumed to be the last element.

		\return
			The index in the CZipCentralDir::m_pFindArray array.
	*/
	ZIP_INDEX_TYPE InsertFindFastElement(CZipFileHeader* pHeader, ZIP_INDEX_TYPE uIndex);

	/**
		Removes a CZipFindFast element from the CZipCentralDir::m_pFindArray array.

		\param pHeader
			The element associated with this object will be removed.

		\param bShift
			If \c true, the affected indexes will be shifted; \c false otherwise.
	*/
	ZIP_INDEX_TYPE RemoveFindFastElement(CZipFileHeader* pHeader, bool bShift);

	/**
		The central directory information.

		\see
			CInfo
	*/
	CInfo* m_pInfo;

	/**
		Reads file headers from the archive.
	*/
	void ReadHeaders();

	/**
		Frees the memory allocated for the CZipFileHeader structures.
	*/
	void RemoveHeaders();

	/**
		Removes data descriptors from a segmented archive that turned out to be single-segment only.
		It is not called for encrypted files.

		\param	bFromBuffer
			If \c true, removes the descriptors from the write buffer in memory otherwise from the file on the disk.

		\return
			\c false, if the file mapping to memory was not successful
			(can happen only when \a bFromBuffer is \c false); \c true otherwise.
		
	*/
	bool RemoveDataDescr(bool bFromBuffer);

	/**
		Writes the file headers to the archive.
	*/
	void WriteHeaders(bool bOneDisk);

	/**
		Writes the End of Central Directory Record.

		\return
			The size of the record.

	*/
	void WriteCentralEnd();


	/**
		Creates data that can be shared between different archive objects.

		\see 
			DestroySharedData
	*/
	void CreateSharedData();

	/**
		Destroys data shared between different archive objects, if the usage reference count
		of the data is zero.

		
		\see 
			CreateSharedData
	*/
	void DestroySharedData();

#ifdef _ZIP_USE_LOCKING
	/**
		Locks the access to the shared data.

		
		\see
			UnlockAccess
		\see 
			CreateSharedData
	*/
	void LockAccess()
	{

		ASSERT(m_pInfo);
		m_pInfo->m_mutex.Lock();
	}

	/**
		Unlocks the access to the shared data.

		
		\see
			LockAccess
		\see 
			CreateSharedData
	*/
	void UnlockAccess()
	{
		if (m_pInfo)
			m_pInfo->m_mutex.Unlock();
	}
#endif
private:
	void InitUnicode();
};

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning (pop)	
#endif


#endif // !defined(ZIPARCHIVE_ZIPCENTRALDIR_DOT_H)
