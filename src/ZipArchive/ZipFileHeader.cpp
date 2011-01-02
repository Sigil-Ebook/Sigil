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

#include "stdafx.h"
#include "ZipFileHeader.h"
#include "ZipAutoBuffer.h"
#include "ZipArchive.h"
#include "ZipPlatform.h"
#include "ZipCompatibility.h"
#include <time.h>

#include "ZipCrc32Cryptograph.h"
#include "BytesWriter.h"
#include "zlib/zlib.h"

#define FILEHEADERSIZE	46
#define LOCALFILEHEADERSIZE	30
#ifdef _ZIP_UNICODE_CUSTOM
	#define ZIP_EXTRA_ZARCH_NAME_VER 1
#endif

using namespace ZipArchiveLib;

char CZipFileHeader::m_gszSignature[] = {0x50, 0x4b, 0x01, 0x02};
char CZipFileHeader::m_gszLocalSignature[] = {0x50, 0x4b, 0x03, 0x04};
CZipFileHeader::CZipFileHeader()
{
	Initialize(NULL);
	SetSystemCompatibility(ZipPlatform::GetSystemID());
}

CZipFileHeader::CZipFileHeader(CZipCentralDir* pCentralDir)
{
	Initialize(pCentralDir);
}

CZipFileHeader::~CZipFileHeader()
{
	
}

void CZipFileHeader::Initialize(CZipCentralDir* pCentralDir)
{
	m_uExternalAttr = 0;//ZipPlatform::GetDefaultAttributes();
	m_uModDate = m_uModTime = 0;
	m_uMethod = CZipCompressor::methodDeflate;
	m_uVersionMadeBy = 0;
	m_uCrc32 = 0;
	// initialize to 0, because on 64 bit platform unsigned long is 8 byte and we are copying only 4 bytes in Read()
	m_uComprSize = m_uUncomprSize = m_uOffset = 0;
	m_uLocalFileNameSize = 0;
	m_uLocalComprSize = m_uLocalUncomprSize = 0;
	m_uLocalHeaderSize = 0;
	m_uVolumeStart = 0;
	m_uEncryptionMethod = CZipCryptograph::encNone;
	m_bIgnoreCrc32 = false;
	m_uFlag = 0;
	m_pCentralDir = pCentralDir;
	m_state = sfNone;
}

bool CZipFileHeader::Read(bool bReadSignature)
{
	m_state = sfNone;

	CZipStorage *pStorage = m_pCentralDir->GetStorage();
	WORD uFileNameSize, uCommentSize, uExtraFieldSize;
	CZipAutoBuffer buf(FILEHEADERSIZE);
	if (bReadSignature)
	{
		pStorage->Read(buf, FILEHEADERSIZE, true);	
		if (!VerifySignature(buf))
			return false;
	}
	else
		pStorage->Read((char*)buf + 4, FILEHEADERSIZE - 4, true);	

	WORD uVersionMadeBy;
	CBytesWriter::ReadBytes(uVersionMadeBy,	buf + 4);
	CBytesWriter::ReadBytes(m_uVersionNeeded,	buf + 6);
	CBytesWriter::ReadBytes(m_uFlag,			buf + 8);
	CBytesWriter::ReadBytes(m_uMethod,			buf + 10);
	CBytesWriter::ReadBytes(m_uModTime,			buf + 12);
	CBytesWriter::ReadBytes(m_uModDate,			buf + 14);
	CBytesWriter::ReadBytes(m_uCrc32,			buf + 16);
	CBytesWriter::ReadBytes(m_uComprSize,		buf + 20, 4);
	CBytesWriter::ReadBytes(m_uUncomprSize,		buf + 24, 4);
	CBytesWriter::ReadBytes(uFileNameSize,		buf + 28);
	CBytesWriter::ReadBytes(uExtraFieldSize,	buf + 30);
	CBytesWriter::ReadBytes(uCommentSize,		buf + 32);
	CBytesWriter::ReadBytes(m_uVolumeStart,		buf + 34, 2);
	CBytesWriter::ReadBytes(m_uInternalAttr,	buf + 36);
	CBytesWriter::ReadBytes(m_uExternalAttr,	buf + 38);
	CBytesWriter::ReadBytes(m_uOffset,			buf + 42, 4);
	buf.Release();

	m_uVersionMadeBy = uVersionMadeBy & 0xFF;
	SetSystemCompatibility((uVersionMadeBy & 0xFF00) >> 8);

	// we may need to modify this later
	m_uEncryptionMethod = (BYTE)((m_uFlag & (WORD) 1) != 0 ? CZipCryptograph::encStandard : CZipCryptograph::encNone);
	
	ZIP_VOLUME_TYPE uCurDsk = pStorage->GetCurrentVolume();	
	m_fileName.m_buffer.Allocate(uFileNameSize); // don't add NULL at the end
	pStorage->Read(m_fileName.m_buffer, uFileNameSize, true);
	
#ifdef _ZIP_UNICODE_CUSTOM
	const CZipStringStoreSettings& stringStoreSettings = m_pCentralDir->GetStringStoreSettings();

	if (stringStoreSettings.IsStandardNameCodePage())
		m_stringSettings.SetDefaultNameCodePage(GetSystemCompatibility());
	else
		m_stringSettings.m_uNameCodePage = stringStoreSettings.m_uNameCodePage;

	if (!stringStoreSettings.IsStandardCommentCodePage())
		m_stringSettings.m_uCommentCodePage = stringStoreSettings.m_uCommentCodePage;
#endif

	if (!m_aCentralExtraData.Read(pStorage, uExtraFieldSize))
		return false;
#if defined _ZIP_UNICODE || defined _ZIP_UNICODE_CUSTOM || defined _ZIP_AES || defined _ZIP_ZIP64
	CZipExtraData* pExtra;
#endif


#ifdef _ZIP_UNICODE_CUSTOM
	if (m_state == sfNone)
	{
		pExtra = m_aCentralExtraData.Lookup(ZIP_EXTRA_ZARCH_NAME);
		if (pExtra != NULL)
		{	
			m_state = sfCustomUnicode;

			WORD uExtraDataSize = (WORD)pExtra->m_data.GetSize();
			int offset = 1;
			if (offset > uExtraDataSize)
				return false;
			if (pExtra->m_data[0] <= ZIP_EXTRA_ZARCH_NAME_VER) // else don't parse it
			{
				offset++;
				if (offset > uExtraDataSize)
					return false;
				BYTE flag = pExtra->m_data[1];
				bool bReadCommentCp = (flag & 4) != 0;
				if ((flag & 1) != 0)
				{
					// code page present
					if (offset + 4 > uExtraDataSize)
						return false;
					// avoid warnings
					DWORD temp;
					CBytesWriter::ReadBytes(temp, pExtra->m_data + offset);
					m_stringSettings.m_uNameCodePage = temp;
					offset += 4;
				}

				if ((flag & 3) == 3)
				{
					m_stringSettings.m_bStoreNameInExtraData = true;
					int iFileNameSize = pExtra->m_data.GetSize() - 2 - 4;
					if (bReadCommentCp)
						iFileNameSize -= 4;
					// code page present
					if (offset + iFileNameSize > uExtraDataSize || iFileNameSize <= 0)
						return false;
					// just in case
					m_fileName.ClearString();
					m_fileName.m_buffer.Allocate(iFileNameSize);
					memcpy(m_fileName.m_buffer, pExtra->m_data + offset, iFileNameSize);
					offset += iFileNameSize;
				}
				else
					m_stringSettings.m_bStoreNameInExtraData = false;

				if (bReadCommentCp)
				{
					// code page present
					if (offset + 4 > uExtraDataSize)
						return false;
					DWORD temp;
					CBytesWriter::ReadBytes(temp, pExtra->m_data + offset);
					m_stringSettings.m_uCommentCodePage = temp;
					// offset += 4;
				}
			}			
		}
		else if (m_pCentralDir->GetUnicodeMode() == CZipArchive::umCustom)
		{
			m_state = sfCustomUnicode;
		}
	}
#endif



	if (uCommentSize)
	{
		m_comment.m_buffer.Allocate(uCommentSize);
		pStorage->Read(m_comment.m_buffer, uCommentSize, true);
	}

	m_aCentralExtraData.RemoveInternalHeaders();

	return pStorage->GetCurrentVolume() == uCurDsk || pStorage->IsBinarySplit(); // check that the whole header is in one volume
}


time_t CZipFileHeader::GetTime()const
{
	struct tm atm;
	atm.tm_sec = (m_uModTime & ~0xFFE0) << 1;
	atm.tm_min = (m_uModTime & ~0xF800) >> 5;
	atm.tm_hour = m_uModTime >> 11;

	atm.tm_mday = m_uModDate & ~0xFFE0;
	atm.tm_mon = ((m_uModDate & ~0xFE00) >> 5) - 1;
	atm.tm_year = (m_uModDate >> 9) + 80;
	atm.tm_isdst = -1;
	return mktime(&atm);
}

// write the header to the central dir
DWORD CZipFileHeader::Write(CZipStorage *pStorage)
{
	m_aCentralExtraData.RemoveInternalHeaders();
	
	WORD uMethod = m_uMethod;

	PrepareStringBuffers();

	if (!CheckLengths(false))
		CZipException::Throw(CZipException::tooLongData);

#ifdef _ZIP_UNICODE_CUSTOM
	if (m_state == sfCustomUnicode)
	{
		if (m_stringSettings.m_bStoreNameInExtraData)
		{
			if (!m_fileName.HasString() && m_fileName.HasBuffer())
				GetFileName(false); // don't clear the buffer, it will be needed in a moment
			ASSERT(m_fileName.HasString());
			ASSERT(m_fileName.HasBuffer());
			if (m_fileName.GetString().GetLength() == 0)
				m_stringSettings.m_bStoreNameInExtraData = false;
		}

		if (!m_stringSettings.IsStandard(GetSystemCompatibility()))
		{
			CZipExtraData* pExtra = m_aCentralExtraData.CreateNew(ZIP_EXTRA_ZARCH_NAME);
			bool bWriteCommentCp = !m_stringSettings.IsStandardCommentCodePage(GetSystemCompatibility());

			BYTE flag = 0;				
			int offset = 2;
			char* data = NULL;
			if (m_stringSettings.m_bStoreNameInExtraData)
			{
				CZipAutoBuffer buffer;
				// m_fileName.m_buffer contains CP_ACP page, we don't check if the current page is CP_ACP - too large dependency on PrepareStringBuffers
				ZipCompatibility::ConvertStringToBuffer(m_fileName.GetString(), buffer, m_stringSettings.m_uNameCodePage);
				int size = 2 + 4 + buffer.GetSize();
				if (bWriteCommentCp)
					size += 4;
				pExtra->m_data.Allocate(size);
				data = (char*)pExtra->m_data;
				CBytesWriter::WriteBytes(data + offset, (DWORD)m_stringSettings.m_uNameCodePage);
				offset += 4;
				memcpy(data + offset, buffer, buffer.GetSize());
				offset += buffer.GetSize();
				flag = 3;
			}
			else if (!m_stringSettings.IsStandardNameCodePage(GetSystemCompatibility()))
			{
				int size = 2 + 4;
				if (bWriteCommentCp)
					size += 4;
				pExtra->m_data.Allocate(size);
				data = (char*)pExtra->m_data;
				CBytesWriter::WriteBytes(data + offset, (DWORD)m_stringSettings.m_uNameCodePage);
				offset += 4;
				flag = 1;
			}

			if (bWriteCommentCp)
			{
				if (!pExtra->m_data.IsAllocated())
				{
					pExtra->m_data.Allocate(2 + 4);
					data = (char*)pExtra->m_data;
				}
				ASSERT(data);
				CBytesWriter::WriteBytes(data + offset, (DWORD)m_stringSettings.m_uCommentCodePage);
				flag |= 4;
			}

			data[0] = ZIP_EXTRA_ZARCH_NAME_VER;
			data[1] = flag;
		}
	}
#endif

	WORD uFileNameSize = (WORD)m_fileName.GetBufferSize(), uCommentSize = m_comment.GetBufferSize(),
		uExtraFieldSize = (WORD)m_aCentralExtraData.GetTotalSize();
	DWORD uSize = FILEHEADERSIZE + uFileNameSize + uCommentSize + uExtraFieldSize;
	CZipAutoBuffer buf(uSize);
	char* dest = (char*)buf;
	memcpy(dest, m_gszSignature, 4);
	WORD uVersionMadeBy = (int)m_uVersionMadeBy & 0xFF;
	uVersionMadeBy |= (WORD)(m_iSystemCompatibility << 8);
	CBytesWriter::WriteBytes(dest + 4,  uVersionMadeBy);
	CBytesWriter::WriteBytes(dest + 6,  m_uVersionNeeded);
	CBytesWriter::WriteBytes(dest + 8,  m_uFlag);
	CBytesWriter::WriteBytes(dest + 10, uMethod);
	CBytesWriter::WriteBytes(dest + 12, m_uModTime);
	CBytesWriter::WriteBytes(dest + 14, m_uModDate);
	WriteCrc32(dest + 16);
	CBytesWriter::WriteBytes(dest + 20, CBytesWriter::WriteSafeU32(m_uComprSize));
	CBytesWriter::WriteBytes(dest + 24, CBytesWriter::WriteSafeU32(m_uUncomprSize));
	CBytesWriter::WriteBytes(dest + 28, uFileNameSize);
	CBytesWriter::WriteBytes(dest + 30, uExtraFieldSize);
	CBytesWriter::WriteBytes(dest + 32, uCommentSize);
	CBytesWriter::WriteBytes(dest + 34, CBytesWriter::WriteSafeU16(m_uVolumeStart));
	CBytesWriter::WriteBytes(dest + 36, m_uInternalAttr);
	CBytesWriter::WriteBytes(dest + 38, m_uExternalAttr);
	CBytesWriter::WriteBytes(dest + 42, CBytesWriter::WriteSafeU32(m_uOffset));

	memcpy(dest + 46, m_fileName.m_buffer, uFileNameSize);

	if (uExtraFieldSize)
		m_aCentralExtraData.Write(dest + 46 + uFileNameSize);

	if (uCommentSize)
		memcpy(dest + 46 + uFileNameSize + uExtraFieldSize, m_comment.m_buffer, uCommentSize);
	
	pStorage->Write(dest, uSize, true);

	// remove to avoid miscalculations in GetSize()
	m_aCentralExtraData.RemoveInternalHeaders();	
	ClearFileName();
	return uSize;
}

bool CZipFileHeader::ReadLocal(CZipCentralDir* pCentralDir)
{	
	CZipStorage* pStorage = pCentralDir->GetStorage();
	pStorage->ChangeVolume(m_uVolumeStart);
	bool isBinary = pStorage->IsBinarySplit();
	if (isBinary)
		pStorage->SeekInBinary(m_uOffset, true);
	else
		pStorage->Seek(m_uOffset);

	char buf[LOCALFILEHEADERSIZE];
	
	pStorage->Read(buf, LOCALFILEHEADERSIZE, true);
	if (memcmp(buf, m_gszLocalSignature, 4) != 0)
		return false;
	
	bool bIsDataDescr = (((WORD)*(buf + 6)) & 8) != 0;
	
	WORD uTemp; 
	CBytesWriter::ReadBytes(uTemp, buf + 6);
	// do not compare the whole flag - the bits reserved by PKWARE may differ 
	// in local and central headers
	if (pCentralDir->IsConsistencyCheckOn( CZipArchive::checkLocalFlag)
		&& (uTemp & 0xf) != (m_uFlag & 0xf))
		return false;
	
	// method
	WORD uMethod;
	CBytesWriter::ReadBytes(uMethod, buf + 8);

	// this may be different in the local header (it may contain disk name for example)
	CBytesWriter::ReadBytes(m_uLocalFileNameSize, buf + 26); 
	WORD uExtraFieldSize;
	CBytesWriter::ReadBytes(uExtraFieldSize, buf + 28);
	ZIP_VOLUME_TYPE uCurDsk = pStorage->GetCurrentVolume();
	// skip reading the local file name
	
	if (isBinary)
		pStorage->SeekInBinary(m_uLocalFileNameSize);
	else
		pStorage->m_pFile->Seek(m_uLocalFileNameSize, CZipAbstractFile::current);

	m_uLocalHeaderSize = LOCALFILEHEADERSIZE + m_uLocalFileNameSize + uExtraFieldSize;
	if (!m_aLocalExtraData.Read(pStorage, uExtraFieldSize))
		return false;

		CBytesWriter::ReadBytes(m_uLocalComprSize, buf + 18, 4);
		CBytesWriter::ReadBytes(m_uLocalUncomprSize, buf + 22, 4);


	if (uMethod == CZipCompressor::methodWinZipAes && IsEncrypted())
		CZipException::Throw(CZipException::noAES);

	if (pCentralDir->IsConsistencyCheckOn( CZipArchive::checkLocalMethod)
		&& uMethod != m_uMethod )
		return false;

	if (!bIsDataDescr && pCentralDir->IsConsistencyCheckOn( CZipArchive::checkLocalCRC | CZipArchive::checkLocalSizes))
	{
		// read all at once - probably faster than checking and reading separately
		DWORD uCrc32;
		CBytesWriter::ReadBytes(uCrc32, buf + 14);
		if (pCentralDir->IsConsistencyCheckOn( CZipArchive::checkLocalCRC)
			&& uCrc32 != m_uCrc32)
			return false;

		if (pCentralDir->IsConsistencyCheckOn( CZipArchive::checkLocalSizes)
			// do not check, if local compressed size is 0 - this usually means, that some archiver 
			// could not update the compressed size after compression
			&& ( m_uLocalComprSize != 0 && m_uLocalComprSize != m_uComprSize || m_uLocalUncomprSize != m_uUncomprSize))
			return false;
	}	
	return pStorage->GetCurrentVolume() == uCurDsk || isBinary; // check that the whole header is in one volume
}

void CZipFileHeader::SetTime(const time_t & ttime)
{
#if _MSC_VER >= 1400
	tm gts;
	tm* gt = &gts;
	localtime_s(gt, &ttime);
#else
	tm* gt = localtime(&ttime);
#endif
	WORD year, month, day, hour, min, sec;
	if (gt == NULL)
	{
		year = 0;
		month = day = 1;
		hour = min = sec = 0;
	}
	else
	{
		year = (WORD)(gt->tm_year + 1900);		
		if (year <= 1980)
			year = 0;
		else
			year -= 1980;
		month = (WORD)gt->tm_mon + 1;
		day = (WORD)gt->tm_mday;
		hour = (WORD)gt->tm_hour;
		min = (WORD)gt->tm_min;
		sec = (WORD)gt->tm_sec;
	}
	    
    m_uModDate = (WORD) (day + ( month << 5) + (year << 9));
    m_uModTime = (WORD) ((sec >> 1) + (min << 5) + (hour << 11));
}

void CZipFileHeader::ConvertFileName(CZipAutoBuffer& buffer) const
{	
	if (!m_fileName.HasString())
		return;
	CZipString temp = m_fileName.GetString();
	ZipCompatibility::SlashBackslashChg(temp, false);
	UINT codePage;
#ifdef _ZIP_UNICODE_CUSTOM
	if (m_state.IsSetAny(sfCustomUnicode))
	{
		if (m_stringSettings.m_bStoreNameInExtraData)
		{
			codePage = GetDefaultFileNameCodePage();
			
		}
		else
		{
			codePage = m_stringSettings.m_uNameCodePage;
		}
	}
	else
#endif
	{
		codePage = GetDefaultFileNameCodePage();
	}

	ZipCompatibility::ConvertStringToBuffer(temp, buffer, codePage);
}

void CZipFileHeader::ConvertFileName(CZipString& szFileName) const
{	
	if (!m_fileName.HasBuffer())
		return;
	UINT codePage;
#ifdef _ZIP_UNICODE_CUSTOM
	if (m_state.IsSetAny(sfCustomUnicode))
	{
		codePage = m_stringSettings.m_uNameCodePage;
	}
	else
#endif
	{
		codePage = GetDefaultFileNameCodePage();
	}
	ZipCompatibility::ConvertBufferToString(szFileName, m_fileName.m_buffer, codePage);
	// some archives may have an invalid path separator stored
	ZipCompatibility::NormalizePathSeparators(szFileName);
}

void CZipFileHeader::ConvertComment(CZipAutoBuffer& buffer) const
{	
	if (!m_comment.HasString())
		return;
	UINT codePage;
#ifdef _ZIP_UNICODE_CUSTOM
	if (m_state.IsSetAny(sfCustomUnicode))
	{
		codePage = m_stringSettings.m_uCommentCodePage;
	}
	else
#endif
	{
		codePage = GetDefaultCommentCodePage();
	}

	ZipCompatibility::ConvertStringToBuffer(m_comment.GetString(), buffer, codePage);
}

void CZipFileHeader::ConvertComment(CZipString& szComment) const
{	
	if (!m_comment.HasBuffer())
		return;
	UINT codePage;
#ifdef _ZIP_UNICODE_CUSTOM
	if (m_state.IsSetAny(sfCustomUnicode))
	{
		codePage = m_stringSettings.m_uCommentCodePage;
	}
	else
#endif
	{
		codePage = GetDefaultCommentCodePage();
	}
	ZipCompatibility::ConvertBufferToString(szComment, m_comment.m_buffer, codePage);
}

// write the local header
void CZipFileHeader::WriteLocal(CZipStorage *pStorage)
{	
	m_aLocalExtraData.RemoveInternalLocalHeaders();
	if (IsDataDescriptor())
	{
		m_uLocalComprSize = 0;
		// write, if we know it - WinZip 9.0 in segmented mode with AES encryption will 
		// complain otherwise (this seems like a bug, because the data descriptor is present and
		// local descriptor should be discarded)
		if (!IsWinZipAesEncryption())
			m_uLocalUncomprSize = 0;
	}
	else
	{
	}

	WORD uMethod = m_uMethod;

	PrepareStringBuffers();
	// this check was already performed, if a file was replaced
	if (!CheckLengths(true))
		m_pCentralDir->ThrowError(CZipException::tooLongData);
	m_uLocalFileNameSize = (WORD)m_fileName.GetBufferSize();
	DWORD uExtraFieldSize = m_aLocalExtraData.GetTotalSize();
	m_uLocalHeaderSize = LOCALFILEHEADERSIZE + uExtraFieldSize + m_uLocalFileNameSize;
	CZipAutoBuffer buf(m_uLocalHeaderSize);
	char* dest = (char*) buf;
	memcpy(dest, m_gszLocalSignature, 4);

	CBytesWriter::WriteBytes(dest + 4,  m_uVersionNeeded);
	CBytesWriter::WriteBytes(dest + 6,  m_uFlag);
	CBytesWriter::WriteBytes(dest + 8,  uMethod);
	CBytesWriter::WriteBytes(dest + 10, m_uModTime);
	CBytesWriter::WriteBytes(dest + 12, m_uModDate);
	WriteSmallDataDescriptor(dest + 14);
	CBytesWriter::WriteBytes(dest + 26, m_uLocalFileNameSize);
	CBytesWriter::WriteBytes(dest + 28, (WORD)uExtraFieldSize);
	memcpy(dest + 30, m_fileName.m_buffer, m_uLocalFileNameSize);

	if (uExtraFieldSize)
		m_aLocalExtraData.Write(dest + 30 + m_uLocalFileNameSize);
	
	// possible volume change before writing to the file in the segmented archive
	// so write the local header first 
	pStorage->Write(dest, m_uLocalHeaderSize, true);

	m_uVolumeStart = pStorage->IsBinarySplit() ? 0 : pStorage->GetCurrentVolume();	
	m_uOffset = pStorage->GetPosition() - m_uLocalHeaderSize;
	
	m_aLocalExtraData.RemoveInternalLocalHeaders();
	ClearFileName();
}


WORD CZipFileHeader::GetDataDescriptorSize(bool bConsiderSignature) const
{
	if (IsDataDescriptor())
	{
		
		WORD size = 12;
		return (WORD)(bConsiderSignature ? size + 4 : size);
	}
	else
		return 0;
}	

bool CZipFileHeader::NeedsDataDescriptor() const
{
	return m_uEncryptionMethod == CZipCryptograph::encStandard;
}

void CZipFileHeader::PrepareData(int iLevel, bool bSegm)
{
	// could be == 1, but the way below it works for PredictMaximumFileSizeInArchive when used on an existing segmented archive
	m_uInternalAttr = 0;

	// version made by

	m_uVersionMadeBy = (unsigned char)0x14;


	m_uCrc32 = 0;
	m_uComprSize = 0;
	m_uUncomprSize = 0;

	m_uFlag  = 0;
	if (m_uMethod == CZipCompressor::methodDeflate)
	{
		switch (iLevel)
		{
		case 1:
			m_uFlag  |= 6;
			break;
		case 2:
			m_uFlag  |= 4;
			break;
		case 8:
		case 9:
			m_uFlag  |= 2;
			break;
		}
	}

	UpdateFlag(bSegm);

	AdjustLocalComprSize();

	m_uVersionNeeded = 0;
	if (m_uVersionNeeded == 0)
		m_uVersionNeeded = IsDirectory() ? 0xa : 0x14; // 1.0 or 2.0		
}


void CZipFileHeader::GetCrcAndSizes(char * pBuffer)const
{
	WriteCrc32(pBuffer);
	CBytesWriter::WriteBytes(pBuffer + 4, m_uComprSize, 4);
	CBytesWriter::WriteBytes(pBuffer + 8, m_uUncomprSize, 4);
}

bool CZipFileHeader::CheckDataDescriptor(CZipStorage* pStorage) const
{
	if (!IsDataDescriptor())
		return true;

	const int sizeOfSize = 4;

	const int size = 4 + 2 * sizeOfSize; // crc and two sizes

	CZipAutoBuffer buf(size + 4);
	pStorage->Read(buf, size, false);
	char* pBuf;

	// when an archive is segmented, files that are divided between volume have bit 3 of flag set
	// which tell about the presence of the data descriptor after the compressed data
	// This signature may be in a segmented archive that is one volume only
	// (it is detected as a not segmented archive)
	if (memcmp(buf, CZipStorage::m_gszExtHeaderSignat, 4) == 0) // there is a signature
	{
		pStorage->Read((char*)buf + size, 4, false);
		pBuf = (char*)buf + 4;
	}
	else 
		pBuf = buf;

	DWORD uCrc32 = 0;
	ZIP_SIZE_TYPE uCompressed = 0, uUncompressed = 0;

	CBytesWriter::ReadBytes(uCrc32,			pBuf);
	CBytesWriter::ReadBytes(uCompressed,	pBuf + 4, sizeOfSize);
	CBytesWriter::ReadBytes(uUncompressed,	pBuf + 4 + sizeOfSize, sizeOfSize);
	return uCrc32 == m_uCrc32 && uCompressed == m_uComprSize && uUncompressed == m_uUncomprSize;
}

DWORD CZipFileHeader::GetSize()const
{	
	DWORD uSize = FILEHEADERSIZE + PredictFileNameSize() + PredictCommentSize();
	uSize += m_aCentralExtraData.GetTotalSize();

#ifdef _ZIP_UNICODE_CUSTOM
	if (m_state.IsSetAny(sfCustomUnicode))
	{
		if (m_stringSettings.m_bStoreNameInExtraData)
		{
			CZipString temp;
			if (m_fileName.HasString())
				temp = m_fileName.GetString();
			else
				ConvertFileName(temp);			
			if (temp.GetLength() > 0)
			{
				uSize += 4 + 2 + 4; // headerID, size + version, flag + filename code page
				CZipAutoBuffer buffer;				
				ZipCompatibility::ConvertStringToBuffer(temp, buffer, m_stringSettings.m_uNameCodePage);
				uSize += buffer.GetSize();
				if (!m_stringSettings.IsStandardCommentCodePage(GetSystemCompatibility()))
					uSize += 4;
			}
		}
	}
#endif
	return uSize;
}

DWORD CZipFileHeader::GetLocalSize(bool bReal)const
{
	if (bReal)
		return m_uLocalHeaderSize;

	DWORD uSize = LOCALFILEHEADERSIZE + m_aLocalExtraData.GetTotalSize() + PredictFileNameSize();
	return uSize;
}

bool CZipFileHeader::SetComment(LPCTSTR lpszComment)
{
	if (m_pCentralDir)
	{
		// update the lpszFileName to make sure the renaming is necessary
		GetComment();
		CZipString newComment(lpszComment);
		if (!UpdateCommentFlags(&newComment))
		{			
			if (m_comment.GetString().Collate(newComment) == 0)
				return true;
		}
		// just in case
		m_comment.ClearBuffer();

		bool ret;
		CZipString oldComment= m_comment.GetString();
		m_comment.SetString(lpszComment);
		ret = m_pCentralDir->OnFileCentralChange();
		if (!ret)
			m_comment.SetString(oldComment);

		return ret;
	}
	else
	{
		m_comment.ClearBuffer();
		m_comment.SetString(lpszComment);
		return true;
	}
}

const CZipString& CZipFileHeader::GetComment(bool bClearBuffer)
{
	if (m_comment.HasString())
	{
		return m_comment.GetString();
	}
	m_comment.AllocateString();
	ConvertComment(m_comment.GetString());
	if (bClearBuffer)
		m_comment.ClearBuffer();
	return m_comment.GetString();
}

int CZipFileHeader::GetCompressionLevel() const
{
	if (m_uMethod == CZipCompressor::methodStore)
		return CZipCompressor::levelStore;
	else if ((m_uFlag & (WORD) 6) != 0)
		return 1;
	else if ((m_uFlag & (WORD) 4) != 0)
		return 2;
	else if ((m_uFlag & (WORD) 2) != 0)
		return CZipCompressor::levelBest;
	else
		return CZipCompressor::levelDefault;
}

bool CZipFileHeader::SetFileName(LPCTSTR lpszFileName)
{
	CZipString newFileName(lpszFileName);
	if (!IsDirectory() || newFileName.GetLength() != 1 || !CZipPathComponent::IsSeparator(newFileName[0]))
		// do not remove from directories where only path separator is present
		CZipPathComponent::RemoveSeparatorsLeft(newFileName);
	if (m_pCentralDir)
	{
		// update the lpszFileName to make sure the renaming is necessary
		GetFileName();

		if (!UpdateFileNameFlags(&newFileName, true))
		{						
			if (IsDirectory())
				CZipPathComponent::AppendSeparator(newFileName);
			else
				CZipPathComponent::RemoveSeparators(newFileName);

			if (m_fileName.GetString().Collate(newFileName) == 0)
				return true;
		}
		// just in case
		m_fileName.ClearBuffer();

		bool ret;
		CZipString oldFileName = m_fileName.GetString();
		m_fileName.SetString(lpszFileName);
		ret = m_pCentralDir->OnFileNameChange(this);
		if (ret)
		{
			SetModified();
		}
		else
			m_fileName.SetString(oldFileName);

		return ret;
	}
	else
	{
		m_fileName.ClearBuffer();		
		m_fileName.SetString(newFileName);
		return true;
	}
}

const CZipString& CZipFileHeader::GetFileName(bool bClearBuffer)
{
	if (m_fileName.HasString())
	{
		return m_fileName.GetString();
	}
	m_fileName.AllocateString();
	ConvertFileName(m_fileName.GetString());
	// don't keep it in memory
	if (bClearBuffer)
	{
		m_fileName.ClearBuffer();
	}
	return m_fileName.GetString();
}

bool CZipFileHeader::IsDirectory()
{
	return ZipPlatform::IsDirectory(GetSystemAttr());
}

DWORD CZipFileHeader::GetSystemAttr()
{
	if (ZipCompatibility::IsPlatformSupported(GetSystemCompatibility()))
	{		
		DWORD uAttr = GetSystemCompatibility() == ZipCompatibility::zcUnix ? (m_uExternalAttr >> 16) : (m_uExternalAttr & 0xFFFF);
		DWORD uConvertedAttr = ZipCompatibility::ConvertToSystem(uAttr, GetSystemCompatibility(), ZipPlatform::GetSystemID());
		if (m_uComprSize == 0 && !ZipPlatform::IsDirectory(uConvertedAttr) && CZipPathComponent::HasEndingSeparator(GetFileName()))			
			// can happen, a folder can have attributes set and no dir attribute (Python modules)
			// TODO: [postponed] fix and cache after reading from central dir, but avoid calling GetFileName() there to keep lazy name conversion
			return ZipPlatform::GetDefaultDirAttributes() | uConvertedAttr; 
		else
		{						
#ifdef _ZIP_SYSTEM_LINUX
			// converting from Windows attributes may create a not readable linux directory
			if (GetSystemCompatibility() != ZipCompatibility::zcUnix && ZipPlatform::IsDirectory(uConvertedAttr))
				return ZipPlatform::GetDefaultDirAttributes();
#endif
			return uConvertedAttr;
		}
	}
	else
		return CZipPathComponent::HasEndingSeparator(GetFileName()) ? ZipPlatform::GetDefaultDirAttributes() : ZipPlatform::GetDefaultAttributes();
}

bool CZipFileHeader::SetSystemAttr(DWORD uAttr)
{
	// The high-word should no be set in attributes, 
	// It will be overwritten by Unix attributes, which are stored in high-word.
	ASSERT((uAttr & 0xFFFF0000) == 0);
	// make it readable under Unix as well, since it stores its attributes in HIWORD(uAttr)
	DWORD uNewAtrr = ZipCompatibility::ConvertToSystem(uAttr, ZipPlatform::GetSystemID(), GetSystemCompatibility());
	if (GetSystemCompatibility() == ZipCompatibility::zcUnix)
	{		
		uNewAtrr <<= 16;
		if (ZipPlatform::IsDirectory(uAttr))
			uNewAtrr |= 0x10; // make it recognizable under other systems (all use 0x10 for directory)
	}
	else
		// make it readable under linux
		uNewAtrr |= (ZipCompatibility::ConvertToSystem(uAttr, ZipPlatform::GetSystemID(), ZipCompatibility::zcUnix) << 16);		

	if (uNewAtrr != m_uExternalAttr)
	{
		if (m_pCentralDir)
		{
			if (!m_pCentralDir->OnFileCentralChange())
			{
				return false;
			}
		}
		m_uExternalAttr = uNewAtrr;
	}
	return true;
}

CZipFileHeader& CZipFileHeader::operator=(const CZipFileHeader& header)
{
	m_uVersionMadeBy = header.m_uVersionMadeBy;	
	m_uVersionNeeded = header.m_uVersionNeeded;
	m_iSystemCompatibility = header.m_iSystemCompatibility;
	m_uFlag = header.m_uFlag;
	m_uMethod = header.m_uMethod;
	m_uModTime = header.m_uModTime;
	m_uModDate = header.m_uModDate;
	m_uCrc32 = header.m_uCrc32;
	m_uComprSize = header.m_uComprSize;
	m_uUncomprSize = header.m_uUncomprSize;
	m_uVolumeStart = header.m_uVolumeStart;
	m_uInternalAttr = header.m_uInternalAttr;
	m_uLocalComprSize = header.m_uLocalComprSize;
	m_uLocalHeaderSize = header.m_uLocalHeaderSize;
	m_uLocalUncomprSize = header.m_uLocalUncomprSize;
	m_uExternalAttr = header.m_uExternalAttr;	
	m_uLocalFileNameSize = header.m_uLocalFileNameSize;;
	m_uOffset = header.m_uOffset;
	m_aLocalExtraData = header.m_aLocalExtraData;
	m_aCentralExtraData = header.m_aCentralExtraData;
	m_uEncryptionMethod = header.m_uEncryptionMethod;
	m_fileName = header.m_fileName;
	m_comment = header.m_comment;
	m_state = header.m_state;

#ifdef _ZIP_UNICODE_CUSTOM
	m_stringSettings = header.m_stringSettings;
#endif
	m_pCentralDir = header.m_pCentralDir;
	
	return *this;
}

void CZipFileHeader::WriteSmallDataDescriptor(char* pDest, bool bLocal)
{
	WriteCrc32(pDest);
	if (bLocal)
	{
		CBytesWriter::WriteBytes(pDest + 4, m_uLocalComprSize, 4);
		CBytesWriter::WriteBytes(pDest + 8, m_uLocalUncomprSize, 4);
	}
	else
	{
		CBytesWriter::WriteBytes(pDest + 4, m_uComprSize, 4);
		CBytesWriter::WriteBytes(pDest + 8, m_uUncomprSize, 4);
	}
}


void CZipFileHeader::WriteDataDescriptor(CZipStorage* pStorage)
{
	if (!IsDataDescriptor())
		return;
	bool signature = NeedsSignatureInDataDescriptor(pStorage);
	CZipAutoBuffer buf;
	buf.Allocate(GetDataDescriptorSize(signature));
	char* pBuf;
	if (signature)
	{
		memcpy(buf, CZipStorage::m_gszExtHeaderSignat, 4);
		pBuf = (char*)buf + 4;
	}
	else
		pBuf = buf;
	WriteCrc32(pBuf);
		CBytesWriter::WriteBytes(pBuf + 4, m_uComprSize, 4);
		CBytesWriter::WriteBytes(pBuf + 8, m_uUncomprSize, 4);
	pStorage->Write(buf, buf.GetSize(), true);
}

void CZipFileHeader::UpdateLocalHeader(CZipStorage* pStorage)
{
	if (pStorage->IsSegmented() || IsDataDescriptor())
		// there is nothing to fix 
		return;
	pStorage->Flush();
	ZIP_FILE_USIZE uPos = pStorage->m_pFile->GetPosition();	
		// update crc and sizes, the sizes may already be all right,
		// but 8 more bytes won't make a difference, we need to update crc32 anyway
		CZipAutoBuffer buf(12);		
		m_uLocalComprSize = CBytesWriter::WriteSafeU32(m_uComprSize);
		m_uLocalUncomprSize = CBytesWriter::WriteSafeU32(m_uUncomprSize);
		WriteSmallDataDescriptor(buf);
		pStorage->Seek(m_uOffset + 14);
		pStorage->m_pFile->Write(buf, 12);

	pStorage->m_pFile->SafeSeek(uPos);
}

void CZipFileHeader::WriteCrc32(char* pBuf) const
{
	DWORD uCrc = m_bIgnoreCrc32 ? 0 : m_uCrc32;
	CBytesWriter::WriteBytes(pBuf, uCrc);
}


void CZipFileHeader::ClearFileName()
{	
#ifdef _ZIP_UNICODE_CUSTOM	
		if (m_state.IsSetAny(sfCustomUnicode) && m_stringSettings.m_bStoreNameInExtraData)
			// we are keeping m_pszFileName, clear the buffer, we need the original, when writing extra header in central directory (can happen many times - better performance)
			// and when accessing the filename
			m_fileName.ClearBuffer();
		else
#endif
			m_fileName.ClearString();
}

bool CZipFileHeader::UpdateFileNameFlags(const CZipString* szNewFileName, bool bAllowRemoveCDir)
{
#if defined _ZIP_UNICODE || defined _ZIP_UNICODE_CUSTOM
	CBitFlag iMode = m_pCentralDir->GetUnicodeMode();
#endif	
	// move the buffer to the name, to ensure it is converted properly now
	GetComment();
	bool centralDirChanged = false;
	bool changed = false;
#ifdef _ZIP_UNICODE_CUSTOM	
	bool isCustom = iMode == CZipArchive::umCustom;
	const CZipStringStoreSettings& stringStoreSettings = m_pCentralDir->GetStringStoreSettings();
	if (m_state.ChangeWithCheck(sfCustomUnicode, isCustom))
	{		
		// the mode changed
		if (isCustom)
		{
			// changed to custom
			m_stringSettings.m_bStoreNameInExtraData = stringStoreSettings.m_bStoreNameInExtraData;
			m_stringSettings.m_uNameCodePage = stringStoreSettings.m_uNameCodePage;
			if (!m_stringSettings.m_bStoreNameInExtraData && m_stringSettings.m_uNameCodePage != GetDefaultFileNameCodePage())
				// the local filename needs to be rewritten, the name code page has changed
				changed = true;
		}
		else
		{
			// changed from custom
			if (!m_stringSettings.m_bStoreNameInExtraData && m_stringSettings.m_uNameCodePage != GetDefaultFileNameCodePage())
				changed = true;

			m_stringSettings.m_bStoreNameInExtraData = stringStoreSettings.m_bStoreNameInExtraData;
			m_stringSettings.m_uNameCodePage = stringStoreSettings.m_uNameCodePage;
		}
		centralDirChanged = true;
	}
	else if (isCustom)
	{		
		if (stringStoreSettings.m_bStoreNameInExtraData != m_stringSettings.m_bStoreNameInExtraData)
		{
			if (m_stringSettings.m_bStoreNameInExtraData)
			{
				changed |= (stringStoreSettings.m_uNameCodePage == GetDefaultFileNameCodePage());
			}
			else
			{
				changed |= (m_stringSettings.m_uNameCodePage == GetDefaultFileNameCodePage());
			}
		}
		else if (!m_stringSettings.m_bStoreNameInExtraData)
			changed |= (stringStoreSettings.m_uNameCodePage != m_stringSettings.m_uNameCodePage);

		m_stringSettings.m_bStoreNameInExtraData = stringStoreSettings.m_bStoreNameInExtraData;
		m_stringSettings.m_uNameCodePage = stringStoreSettings.m_uNameCodePage;
	}
#endif
	if (changed || centralDirChanged)
	{
		m_comment.ClearBuffer();
	}
	// remove the central directory in case the filename did not changed, but the comment flags changed
	if (!changed && centralDirChanged && bAllowRemoveCDir && m_pCentralDir && m_comment.HasString())
		m_pCentralDir->OnFileCentralChange();
	return changed;
}

bool CZipFileHeader::UpdateCommentFlags(const CZipString* szNewComment)
{
#if defined _ZIP_UNICODE || defined _ZIP_UNICODE_CUSTOM
	CBitFlag iMode = m_pCentralDir->GetUnicodeMode();
#endif
	bool changed = false;
#ifdef _ZIP_UNICODE_CUSTOM		
		bool isCustom = iMode == CZipArchive::umCustom;
		changed |= m_state.ChangeWithCheck(sfCustomUnicode, isCustom);
		if (isCustom)
		{
			const CZipStringStoreSettings& stringStoreSettings = m_pCentralDir->GetStringStoreSettings();
			changed |= (m_stringSettings.m_uCommentCodePage != stringStoreSettings.m_uCommentCodePage);
			m_stringSettings.m_uCommentCodePage = stringStoreSettings.m_uCommentCodePage;
		}
#endif
	return changed;
}
