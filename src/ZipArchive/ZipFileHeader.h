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
* \file ZipFileHeader.h
* Includes the CZipFileHeader class.
*
*/
#if !defined(ZIPARCHIVE_ZIPFILEHEADER_DOT_H)
#define ZIPARCHIVE_ZIPFILEHEADER_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "ZipExport.h"
#include "ZipStorage.h"
#include "ZipAutoBuffer.h"
#include "sys/types.h"
#include "ZipCompatibility.h"
#include "ZipCollections.h"
#include "ZipExtraField.h"
#include "ZipStringStoreSettings.h"
#include "ZipCryptograph.h"
#include "BitFlag.h"


class CZipCentralDir;

/**
	Represents a single file stored in a zip archive.
*/
class ZIP_API CZipFileHeader  
{
	friend class CZipCentralDir;
	friend class CZipArchive;
protected:
	CZipFileHeader(CZipCentralDir* pCentralDir);
public:	
	/**
		File header state flags.

		\see
			CZipArchive::UnicodeMode
	*/
	enum StateFlags
	{
		sfNone				= 0x00,		///< No special flags set.
#ifdef _ZIP_UNICODE_CUSTOM	
		sfCustomUnicode		= 0x10,		///< The header uses custom Unicode functionality.
#endif
		sfModified			= 0x20		///< The file has been modified.
	};

	CZipFileHeader();
	CZipFileHeader(const CZipFileHeader& header)
	{
		*this = header;
	}
	CZipFileHeader& operator=(const CZipFileHeader& header);
	virtual ~CZipFileHeader();

	/**
		Predicts the filename size after conversion using the current filename code page.

		\return 
			The number of characters not including a terminating \c NULL character.
	*/
	int PredictFileNameSize() const 
	{
		if (m_fileName.HasBuffer())
		{
			return m_fileName.GetBufferSize();
		}
		CZipAutoBuffer buffer;
		ConvertFileName(buffer);
		return buffer.GetSize();
	}
	
	/**
		Predicts a file comment size.

		\return
			The number of characters in the comment not including a terminating \c NULL character.
	*/
	int	PredictCommentSize() const 
	{
		if (m_comment.HasBuffer())
		{
			return m_comment.GetBufferSize();
		}
		CZipAutoBuffer buffer;
		ConvertComment(buffer);
		return buffer.GetSize();
	}


	/**
		Returns the filename. If necessary, performs the conversion using the current filename code page.
		Caches the result of conversion for faster access the next time.

		\param bClearBuffer
			If \c true, releases the internal buffer after performing the filename conversion.
			If \c false, the internal buffer is not released and both representations of the 
			filename are kept in memory (converted and not converted). This takes more memory, but the 
			conversion does not take place again when the central directory is written back to the archive.

		\return
			The converted filename.

		\see
			<a href="kb">0610051525</a>
		\see
			SetFileName
		\see 
			GetStringStoreSettings
		\see
			CZipStringStoreSettings::m_uNameCodePage
	*/
	const CZipString& GetFileName(bool bClearBuffer = true);

	/**
		Sets the filename.

		The actual renaming of the file inside of the archive depends on the current commit mode.

		\param	lpszFileName
			The filename to set.

		\return 
			\c true, if the method succeeded; \c false, if the current state of the archive is invalid for this method to be called.

		\note
			Leading path separators are removed from the filename unless the header is a directory and the filename contains of only one path separator (indicating a root path).

		\see
			<a href="kb">0610231944|rename</a>
		\see
			GetFileName
		\see 
			CZipArchive::CommitChanges
	*/
	bool SetFileName(LPCTSTR lpszFileName);

	/**
		Returns the file comment.
		
		\param bClearBuffer
			If \c true, releases the internal buffer after performing the comment conversion.
			If \c false, the internal buffer is not released and both representations of the 
			comment are kept in memory (converted and not converted). This takes more memory, but the 
			conversion does not take place again when the central directory is written back to the archive.

		\return
			The file comment.

		\see
			<a href="kb">0610231944|comment</a>
		\see
			SetComment
	*/
	const CZipString& GetComment(bool bClearBuffer = false);

	/**
		Sets the file comment.

		\param lpszComment
			The file comment.

		\return 
			\c true, if the method succeeded; \c false, if the current state of the archive is invalid for this method to be called.

		\see
			<a href="kb">0610231944|comment</a>
		\see
			GetComment
	*/
	bool SetComment(LPCTSTR lpszComment);

	/**
		Returns the value indicating whether the data descriptor is present.

		\return
			\c true, if the data descriptor is present; \c false otherwise.
	*/
	bool IsDataDescriptor()const {	return (m_uFlag & (WORD) 8) != 0;}

	/**
		Returns the data descriptor size as it is required for the current file.
		Takes into account various factors, such as the archive segmentation type,
		encryption and the need for the Zip64 format.

		\param pStorage
			The storage to test for the segmentation type.

		\return 
			The required data descriptor size in bytes.
	*/
	WORD GetDataDescriptorSize(const CZipStorage* pStorage) const
	{
		return GetDataDescriptorSize(NeedsSignatureInDataDescriptor(pStorage));
	}

	/**
		Returns the data descriptor size as it is required for the current file.
		Takes into account various factors, such as the need for the data descriptor signature
		or for the Zip64 format.

		\param bConsiderSignature
			\c true, if the data descriptor signature is needed; \c false otherwise.

		\return 
			The required data descriptor size in bytes.
	*/
	WORD GetDataDescriptorSize(bool bConsiderSignature = false) const;	

	/**
		Returns the size of the compressed data.
	
		\param bReal
			Set to \c true when calling for a file already in an archive.
			Set to \c false when the header is not a part of the archive.
	
		\return
			The compressed data size in bytes.

		\see
			GetEncryptedInfoSize
	 */
	ZIP_SIZE_TYPE GetDataSize(bool bReal) const
	{
		DWORD uEncrSize = GetEncryptedInfoSize();
		return bReal ? (m_uComprSize - uEncrSize) : (m_uComprSize + uEncrSize);
	}

	/**
		Returns the encrypted information size. The returned value depends on the used encryption method.

		\return
			The  encrypted information size in bytes.
	*/
	DWORD GetEncryptedInfoSize() const
	{
		return CZipCryptograph::GetEncryptedInfoSize(m_uEncryptionMethod);
	}

	/**
		Returns the total size of this structure in the central directory.

		\return
			The total size in bytes.
	*/
	DWORD GetSize() const;

	/**
		Returns the local header size. Before calling this method, the local information must be up-to-date
		(see <a href="kb">0610242128|local</a> for more information).

		\param bReal
			If \c true, uses the real local filename size.
			If \c false, predicts the filename size. 			

		\return
			The local header size in bytes.
	*/
	DWORD GetLocalSize(bool bReal) const;	

	/**
		Returns the value indicating whether the compression is efficient.

		\return 
			\c true if the compression is efficient; \c false if the file should be 
			stored without the compression instead.
	*/
	bool CompressionEfficient()
	{
		ZIP_SIZE_TYPE uBefore = m_uUncomprSize;
		// ignore the length of encryption info 
		ZIP_SIZE_TYPE uAfter = GetDataSize(true);
		return  uAfter <= uBefore;
	}

	/**
		Returns the compression ratio.

		\return
			The compression ratio of the file.
	*/
	float GetCompressionRatio()
	{
#if _MSC_VER >= 1300 || !defined(_ZIP_ZIP64)
		return m_uUncomprSize ? ((float)m_uComprSize * 100 ) / m_uUncomprSize: 0;
#else
		return m_uUncomprSize ? ((float)(__int64)(m_uComprSize) / (float)(__int64)m_uUncomprSize) * 100: 0;
#endif
	}

	/**
		Sets the file modification date.

		\param ttime
			The date to set. If this value is incorrect, the date defaults to January 1, 1980.

		\see
			GetTime
	*/
	void SetTime(const time_t& ttime);

	/**
		Returns the file modification time.

		\return
			The modification time.

		\see
			SetTime
	*/
	time_t GetTime()const;

	/**
		Returns the file system compatibility.
		External software can use this information e.g. to determine end-of-line 
		format for text files etc.
		The ZipArchive Library uses it to perform a proper file attributes conversion.

		\return
			The file system compatibility. It can be one of the ZipCompatibility::ZipPlatforms values.
		\see
			CZipArchive::GetSystemComatibility
		\see
			ZipPlatform::GetSystemID
	*/
	int GetSystemCompatibility()const
	{
		return (int)m_iSystemCompatibility;
	}

	/**
		Returns the file attributes.

		\return
			The file attributes, converted if necessary to be compatible with the current system.

		\note 
			Throws an exception, if the archive system or the current system 
			is not supported by the ZipArchive Library.			

		\see
			GetOriginalAttributes
	*/
	DWORD GetSystemAttr();

	
	/**
		Sets the file attributes.

		\param	uAttr
			The attributes to set.

		\note
			Throws an exception, if the archive system or the current system is not supported by the ZipArchive Library.

		\see	
			GetSystemAttr
	*/
	bool SetSystemAttr(DWORD uAttr);

	/**
		Returns the file attributes exactly as they are stored in the archive.

		\return 
			The file attributes as they are stored in the archive.
			No conversion is performed.

		\note
			The attributes for Linux are stored shifted left by 16 bits in this field.
		\see 
			GetSystemAttr
	*/
	DWORD GetOriginalAttributes() const {return m_uExternalAttr;}

	/**
		Returns the value indicating whether the file represents a directory.
		This method checks the file attributes. If the attributes value is zero, 
		the method checks for the presence of a path 
		separator at the end of the filename. If the path separator is present, 
		the file is assumed to be a directory.

		\return
			\c true, if the file represents a directory; \c false otherwise.		
	*/
	bool IsDirectory();

#ifdef _ZIP_UNICODE_CUSTOM	
	/**
		Returns the current string store settings.
	
		\return
			The string store settings.

		\see
			<a href="kb">0610051525|custom</a>
		\see
			CZipArchive::GetStringStoreSettings
	 */
	CZipStringStoreSettings GetStringStoreSettings()
	{
		return m_stringSettings;
	}
#endif

	/**
		Returns the value indicating whether the file is encrypted.
		If the file is encrypted, you need to set the password with the 
		CZipArchive::SetPassword method before decompressing the file.

		\return
			\c true if the file is encrypted; \c false otherwise.

		\see
			CZipArchive::SetPassword
	*/
	bool IsEncrypted()const {	return m_uEncryptionMethod != CZipCryptograph::encNone;}

	/**
		Returns the encryption method of the file.

		\return
			The file encryption method. It can be one of the CZipCryptograph::EncryptionMethod values.
	*/
	int GetEncryptionMethod() const {return m_uEncryptionMethod;}

	/**
		Returns the value indicating whether the file is encrypted using WinZip AES encryption method.

		\return
			\c true, if the file is encrypted using WinZip AES encryption method; \c false otherwise.
	*/
	bool IsWinZipAesEncryption() const
	{
		return CZipCryptograph::IsWinZipAesEncryption(m_uEncryptionMethod);
	}

	/**
		Returns an approximate file compression level.

		\return 
			The compression level. May not be the real value used when compressing the file.
	*/
	int GetCompressionLevel() const;

	/**
		Returns the value indicating whether the current CZipFileHeader object has the time set.

		\return
			\c true, if the time is set; \c false otherwise.
	*/
	bool HasTime() const
	{
		return m_uModTime != 0 || m_uModDate != 0;
	}

	/**
		Returns the value indicating whether the file was modified.

		\return
			\c true, if the file was modified; \c false otherwise.

		\see
			CZipArchive::CommitChanges
	*/
	bool IsModified() const
	{
		return m_state.IsSetAny(sfModified);
	}


	/**
		Returns the state flags.

		\return
			State flags. It can be one or more of #StateFlags values.		
	*/
	const ZipArchiveLib::CBitFlag& GetState() const
	{
		return m_state;
	}

	static char m_gszSignature[];		///< The central file header signature.
	static char m_gszLocalSignature[];	///< The local file header signature.
	unsigned char m_uVersionMadeBy;		///< The version of the software that created the archive.
	WORD m_uVersionNeeded;				///< The version needed to extract the file.
	WORD m_uFlag;						///< A general purpose bit flag.
	WORD m_uMethod;						///< The compression method. It can be one of the CZipCompressor::CompressionMethod values.
	WORD m_uModTime;					///< The file last modification time.
	WORD m_uModDate;					///< The file last modification date.
	DWORD m_uCrc32;						///< The crc-32 value.
	ZIP_SIZE_TYPE m_uComprSize;			///< The compressed size.
	ZIP_SIZE_TYPE m_uUncomprSize;		///< The uncompressed size.
	ZIP_VOLUME_TYPE m_uVolumeStart;		///< The volume number at which the compressed file starts.
	WORD m_uInternalAttr;				///< Internal file attributes.
	ZIP_SIZE_TYPE m_uLocalComprSize;	///< The compressed size written in the local header.
	ZIP_SIZE_TYPE m_uLocalUncomprSize;	///< The uncompressed size written in the local header.
	ZIP_SIZE_TYPE m_uOffset;			///< Relative offset of the local header with respect to CZipFileHeader::m_uVolumeStart.
	CZipExtraField m_aLocalExtraData;	///< The local extra field. Do not modify it after you started compressing the file.
	CZipExtraField m_aCentralExtraData; ///< The central extra field.
protected:
	DWORD m_uExternalAttr;				///< External file attributes.
 	WORD m_uLocalFileNameSize;			///< The local filename length.
	BYTE m_uEncryptionMethod;			///< The file encryption method. It can be one of the CZipCryptograph::EncryptionMethod values.
	bool m_bIgnoreCrc32;				///< The value indicating whether to ignore Crc32 checking.
	DWORD m_uLocalHeaderSize;
	
	CZipCentralDir* m_pCentralDir;		///< The parent central directory. It can be \c NULL when the header is not a part of central directory.

	

	/**
		Sets the file system compatibility.

		\param	iSystemID
			The file system compatibility. It can be one of the ZipCompatibility::ZipPlatforms values.

		\param bUpdateAttr
			If \c true, the attributes will be converted to the new system; \c false otherwise.

		\see
			GetSystemCompatibility
	*/
	void SetSystemCompatibility(int iSystemID, bool bUpdateAttr = false)
	{
		if (bUpdateAttr)
		{
			if ((int)m_iSystemCompatibility != iSystemID)
			{
				DWORD uAttr = GetSystemAttr();
				m_iSystemCompatibility = (char)(iSystemID & 0xFF);
				SetSystemAttr(uAttr & 0xFFFF);				
			}
			return;
		}
		m_iSystemCompatibility = (char)(iSystemID & 0xFF);
	}
	
	/**
		Prepares the filename for writing to the archive.
	*/
	void PrepareStringBuffers()
	{
		if (!m_fileName.HasBuffer())
		{
			ConvertFileName(m_fileName.m_buffer);
		}
		if (!m_comment.HasBuffer())
		{
			ConvertComment(m_comment.m_buffer);
		}
	}

	/**
		Validates an existing data descriptor after file decompression.

		\param pStorage
			The storage to read the data descriptor from.

		\return
			\c true, if the data descriptor is valid; \c false otherwise.
	*/
	bool CheckDataDescriptor(CZipStorage* pStorage) const;


	/**
		Prepares the data for writing when adding a new file. When Zip64 extensions are required for this file, 
		this method adds Zip64 extra data to #m_aLocalExtraData.

		\param	iLevel
			The compression level.

		\param bSegm
			Set to \c true, if the archive is segmented; \c false otherwise.
	*/
	void PrepareData(int iLevel, bool bSegm);

	/**	
		Writes the local file header to the \a pStorage. 
		The filename and extra field are the same as those that will be stored in the central directory.

		\param	pStorage
			The storage to write the local file header to.

	*/
	void WriteLocal(CZipStorage *pStorage);

	/**
		Reads the local file header from the archive and validates the read data.

		\param pCentralDir
			Used when the archive was opened with CZipArchive::OpenFrom method. Points to the original central directory.

		\return
			\c true, if the data read is consistent; \c false otherwise.

		
		\see 
			CZipArchive::SetIgnoredConsistencyChecks
	*/
	bool ReadLocal(CZipCentralDir* pCentralDir);

	/**
		Writes the central file header to \a pStorage.

		\param	pStorage
			The storage to write the central file header to.

		\return
			The size of the file header.

	*/
	DWORD Write(CZipStorage *pStorage);

	/**
		Reads the central file header from \a pStorage and validates the read data.

		\param bReadSignature
			\c true, if the the central header signature should be read; \c false otherwise.


		\return
			\c true, if the read data is consistent; \c false otherwise.

	*/
	bool Read(bool bReadSignature);


	/**
		Validates the member fields lengths. 
		The tested fields are: filename, extra fields and comment.

		\return
			\c false, if any of the lengths exceeds the allowed value.
	*/
	bool CheckLengths(bool local) const
	{
		if (m_comment.GetBufferSize() > (int)USHRT_MAX || m_fileName.GetBufferSize() > (int)USHRT_MAX)
			return false;
		else if (local)
			return m_aLocalExtraData.Validate();
		else
			return m_aCentralExtraData.Validate();
	}

	/**
		Writes the Crc32 to \a pBuf.

		\param pBuf
			The buffer to write the Crc32 to. Must have be of at least 4 bytes size.
	*/
	void WriteCrc32(char* pBuf) const;
	
	
	/**
		Returns the value indicating whether the file needs the data descriptor.
		The data descriptor is needed when a file is encrypted or the Zip64 format needs to be used.

		\return 
			\c true, if the data descriptor is needed; \c false otherwise.
	*/
	bool NeedsDataDescriptor() const;
	

	/**
		Writes the data descriptor.

		\param pDest
			The buffer to receive the data.

		\param bLocal
			Set to \c true, if the local sizes are used; \c false otherwise.
	*/
	void WriteSmallDataDescriptor(char* pDest, bool bLocal = true);

	/**
		Writes the data descriptor taking into account the Zip64 format.

		\param pStorage
			The storage to write the data descriptor to.
	*/
	void WriteDataDescriptor(CZipStorage* pStorage);

	/**
		Returns the value indicating whether the signature in the data descriptor is needed.

		\param pStorage
			The current storage.

		\return
			\c true, if the signature is needed; \c false otherwise.
	*/
	bool NeedsSignatureInDataDescriptor(const CZipStorage* pStorage) const
	{
		return pStorage->IsSegmented() || IsEncrypted();
	}

	/**
		Updates the local header in the archive after is has already been written.

		\param pStorage
			The storage to update the data descriptor in.
	*/
	void UpdateLocalHeader(CZipStorage* pStorage);

	/**
		Adjusts the local compressed size.
	*/
	void AdjustLocalComprSize()
	{
		AdjustLocalComprSize(m_uLocalComprSize);
	}

	/**
		Adjusts the local compressed size.

		\param uLocalComprSize
			The value to adjust.
	*/
	void AdjustLocalComprSize(ZIP_SIZE_TYPE& uLocalComprSize)
	{
		uLocalComprSize += GetEncryptedInfoSize();
	}

	/**
		Verifies the central header signature.

		\param buf
			The buffer that contains the signature to verify.

		\return 
			\c true, if the signature is valid; \c false otherwise.
	*/
	static bool VerifySignature(CZipAutoBuffer& buf)
	{
		return memcmp(buf, m_gszSignature, 4) == 0;
	}

	/**
		Updates the general purpose bit flag. 

		\param bSegm
			\c true, if the current archive is a segmented archive; \c false otherwise.
	*/
	void UpdateFlag(bool bSegm)
	{
		if (bSegm || m_uEncryptionMethod == CZipCryptograph::encStandard)
			m_uFlag  |= 8; // data descriptor present

		if (IsEncrypted())
			m_uFlag  |= 1;		// encrypted file		
	}

	
private:	

	struct StringWithBuffer
	{
		StringWithBuffer()
		{
			m_pString = NULL;
		}
		CZipAutoBuffer m_buffer;
		StringWithBuffer& operator = (const StringWithBuffer& original)
		{			
			if (original.HasString())
			{
				SetString(original.GetString());
			}
			else
			{
				ClearString();
			}
			m_buffer = original.m_buffer;
			return *this;
		}
		void AllocateString()
		{
			ClearString();
			m_pString = new CZipString(_T(""));
		}
		bool HasString() const
		{
			return m_pString != NULL;
		}
		bool HasBuffer() const 
		{
			return m_buffer.IsAllocated() && m_buffer.GetSize() > 0;
		}
		void ClearString()
		{
			if (HasString())
			{
				delete m_pString;
				m_pString = NULL;
			}
		}
		void ClearBuffer()
		{
			m_buffer.Release();
		}

		const CZipString& GetString() const
		{
			ASSERT(HasString());
			return *m_pString;
		}

		CZipString& GetString()
		{
			ASSERT(HasString());
			return *m_pString;
		}

		void SetString(LPCTSTR value)
		{
			if (!HasString())
				AllocateString();
			*m_pString = value;
		}
		
		int GetBufferSize() const
		{
			return m_buffer.GetSize();
		}
		~StringWithBuffer()
		{
			ClearString();
		}
	protected:
		CZipString* m_pString;
	};

	ZipArchiveLib::CBitFlag m_state;
		
	void Initialize(CZipCentralDir* pCentralDir);

	void SetModified(bool bModified = true)
	{
		m_state.Change(sfModified, bModified);
	}

	void ConvertFileName(CZipAutoBuffer& buffer) const;
	void ConvertFileName(CZipString& szFileName) const;
	void ConvertComment(CZipAutoBuffer& buffer) const;
	void ConvertComment(CZipString& szComment) const;

	bool UpdateFileNameFlags(const CZipString* szNewFileName, bool bAllowRemoveCDir);
	bool UpdateCommentFlags(const CZipString* szNewComment);
	bool UpdateStringsFlags(bool bAllowRemoveCDir)
	{
		return UpdateFileNameFlags(NULL, bAllowRemoveCDir) | UpdateCommentFlags(NULL);
	}

	UINT GetDefaultFileNameCodePage() const
	{
		return ZipCompatibility::GetDefaultNameCodePage(GetSystemCompatibility());
	}

	UINT GetDefaultCommentCodePage() const
	{
		return ZipCompatibility::GetDefaultCommentCodePage(GetSystemCompatibility());
	}

	void ClearFileName();

	void GetCrcAndSizes(char* pBuffer)const;

	bool NeedsZip64() const
	{
		return m_uComprSize >= UINT_MAX || m_uUncomprSize >= UINT_MAX || m_uVolumeStart >= USHRT_MAX || m_uOffset >= UINT_MAX;
	}


	void OnNewFileClose(CZipStorage* pStorage)
	{
		UpdateLocalHeader(pStorage);
		WriteDataDescriptor(pStorage);
		pStorage->Flush();
	}

#ifdef _ZIP_UNICODE_CUSTOM
	CZipStringStoreSettings m_stringSettings;
#endif
	StringWithBuffer m_fileName;
	StringWithBuffer m_comment;
	char m_iSystemCompatibility;
};

#endif // !defined(ZIPARCHIVE_ZIPFILEHEADER_DOT_H)
