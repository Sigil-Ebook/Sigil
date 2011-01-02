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
#include "ZipStorage.h"
#include "ZipArchive.h"
#include "ZipPlatform.h"

char CZipStorage::m_gszExtHeaderSignat[] = {0x50, 0x4b, 0x07, 0x08};
const ZIP_FILE_USIZE CZipStorage::SignatureNotFound = ZIP_FILE_USIZE(-1);
#define SIGNATURE_SIZE 4

using namespace ZipArchiveLib;


CZipStorage::CZipStorage()
{
	Initialize();
}

void CZipStorage::Initialize()
{	
	m_pSplitChangeVolumeFunc = m_pSpanChangeVolumeFunc = m_pChangeVolumeFunc = NULL;
	m_iWriteBufferSize = 65536;	
	m_pFile = NULL;	
	m_iLocateBufferSize = 32768;
	m_uBytesBeforeZip = 0;
	m_uCurrentVolume = ZIP_VOLUME_NUMBER_UNSPECIFIED;
	m_szArchiveName.Empty();
	m_pSplitNames = NULL;
	m_pCachedSizes = NULL;
	m_bAutoDeleteSplitNames = false;
	m_state = 0;
}

CZipStorage::~CZipStorage()
{
	ClearSplitNames();
	ClearCachedSizes();
}

DWORD CZipStorage::Read(void *pBuf, DWORD iSize, bool bAtOnce)
{
	if (iSize == 0)
		return 0;
	DWORD iRead;
	for(;;)
	{
		iRead = m_pFile->Read(pBuf, iSize);
		if (!iRead)
		{
			if (IsSegmented())
				ChangeVolume();
			else
				ThrowError(CZipException::badZipFile);
		}
		else
			break;
	}

	if (iRead == iSize)
		return iRead;
	else if ((bAtOnce && !IsBinarySplit()) || !IsSegmented())
		ThrowError(CZipException::badZipFile);

	while (iRead < iSize)
	{
		ChangeVolume();
		UINT iNewRead = m_pFile->Read((char*)pBuf + iRead, iSize - iRead);
		if (!iNewRead && iRead < iSize)
			ThrowError(CZipException::badZipFile);
		iRead += iNewRead;
	}

	return iRead;
}

void CZipStorage::Open(LPCTSTR lpszPathName, int iMode, ZIP_SIZE_TYPE uVolumeSize)
{
	m_uCurrentVolume = ZIP_VOLUME_NUMBER_UNSPECIFIED;
	m_pWriteBuffer.Allocate(m_iWriteBufferSize); 
	m_uBytesInWriteBuffer = 0;
	m_state.Set(stateOpened | stateAutoClose);
	m_pFile = &m_internalfile;
	m_szArchiveName = lpszPathName;
	m_pChangeVolumeFunc = NULL;

	CBitFlag mode(iMode);

	if (mode.IsSetAny(CZipArchive::zipCreate)) // create new archive
	{		
		m_uCurrentVolume = 0;
		if (!mode.IsSetAny(CZipArchive::zipModeSegmented))
		{
			OpenFile(lpszPathName, (mode.IsSetAll(CZipArchive::zipCreateAppend) ? CZipFile::modeNoTruncate : CZipFile::modeCreate) | CZipFile::modeReadWrite);
		}
		else // create a segmented archive
		{			
			m_uBytesWritten = 0;
			if (mode.IsSetAny(CZipArchive::zipModeSpan))
			{
				if (!m_pSpanChangeVolumeFunc)
					ThrowError(CZipException::noCallback);
				if (!ZipPlatform::IsDriveRemovable(lpszPathName))
					ThrowError(CZipException::nonRemovable);
				m_state.Set(stateSpan);
				m_pChangeVolumeFunc = m_pSpanChangeVolumeFunc;
			}
			else if (uVolumeSize <= 0)
				ThrowError(CZipException::noVolumeSize);
			else
			{
				m_uSplitData = uVolumeSize;
				if (mode.IsSetAll(CZipArchive::zipModeBinSplit))
				{
					m_state.Set(stateBinarySplit);
					ClearCachedSizes(); // just in case
					m_pCachedSizes = new CZipArray<ZIP_FILE_USIZE>();
				}
				else
					m_state.Set(stateSplit);
				EnsureSplitNames();
				m_pChangeVolumeFunc = m_pSplitChangeVolumeFunc;
			}

			NextVolume(4);
			Write(m_gszExtHeaderSignat, 4, true);
		}
	}
	else // open existing
	{
		m_state.Set(stateExisting);
		bool readOnly = mode.IsSetAll(CZipArchive::zipOpenReadOnly);
		if (readOnly)
			m_state.Set(stateReadOnly);
		OpenFile(lpszPathName, CZipFile::modeNoTruncate |
			(readOnly ? CZipFile::modeRead : CZipFile::modeReadWrite));
		// this will be revised in UpdateSegmMode
		if (mode.IsSetAny(CZipArchive::zipModeSpan))
			m_state.Set(stateSpan);
		else if (mode.IsSetAll(CZipArchive::zipModeBinSplit))
		{
			m_state.Set(stateBinarySplit);
			EnsureSplitNames();
			m_uCurrentVolume = m_pSplitNames->GetVolumeNumber(m_szArchiveName);
			if (m_uCurrentVolume == 0)
				ThrowError(CZipException::badZipFile);
			m_uCurrentVolume -= 1;
			if (m_uCurrentVolume > 0)
			{				
				m_uSplitData = m_uCurrentVolume;
				CacheSizes();
			}
			else
			{
				ClearSplitNames();
				m_state.Clear(stateBinarySplit);
			}
		}
		else if (mode.IsSetAny(CZipArchive::zipModeSplit))
			m_state.Set(stateSplit);
	}	
}

void CZipStorage::CacheSizes()
{
	ClearCachedSizes(); // just in case
	m_pCachedSizes = new CZipArray<ZIP_FILE_USIZE>();
	m_pCachedSizes->SetSize((ZIP_ARRAY_SIZE_TYPE)(m_uCurrentVolume + 1));
	ZIP_VOLUME_TYPE lastVolume = m_uCurrentVolume;
	for(;;)
	{
		m_pCachedSizes->SetAt((ZIP_ARRAY_SIZE_TYPE)m_uCurrentVolume, m_pFile->GetLength());
		if (m_uCurrentVolume == 0)
			break;
		ChangeVolume((ZIP_VOLUME_TYPE)(m_uCurrentVolume - 1));
	}
	ChangeVolume(lastVolume);
}

void CZipStorage::Open(CZipAbstractFile& af, int iMode, bool bAutoClose)
{
	m_pWriteBuffer.Allocate(m_iWriteBufferSize); 
	m_uBytesInWriteBuffer = 0;
	m_state.Set(stateOpened);
	if (bAutoClose)
		m_state.Set(stateAutoClose);
	m_pFile = &af;

	CBitFlag mode(iMode);

	if (mode.IsSetAny(CZipArchive::zipCreate))
	{		
		m_uCurrentVolume = 0;
		if (mode.IsSetAll(CZipArchive::zipCreateAppend))
			af.SeekToEnd();
		else
			af.SetLength(0);
	}
	else // open existing
	{
		m_state.Set(stateExisting);
		if (mode.IsSetAll(CZipArchive::zipOpenReadOnly))
			m_state.Set(stateReadOnly);
		af.SeekToBegin();
	}
}


void CZipStorage::ChangeVolume(ZIP_VOLUME_TYPE uNumber)
{
	if (uNumber == m_uCurrentVolume || !IsSegmented()) // the second condition may happen in some bad archives
		return;

	m_uCurrentVolume = uNumber;	
	OpenFile(IsSpanned() ? ChangeSpannedRead() : ChangeSplitRead(),
		CZipFile::modeNoTruncate | CZipFile::modeRead);
}

void CZipStorage::ThrowError(int err) const
{
	CZipException::Throw(err, m_pFile->GetFilePath());
}

bool CZipStorage::OpenFile(LPCTSTR lpszName, UINT uFlags, bool bThrow)
{
	return m_pFile->Open(lpszName, uFlags | CZipFile::shareDenyWrite, bThrow);
}


CZipString CZipStorage::ChangeSpannedRead()
{
	CZipString szTemp = m_pFile->GetFilePath();
	m_pFile->Close();
	CallCallback(0, CZipSegmCallback::scVolumeNeededForRead, szTemp);
	return m_pChangeVolumeFunc->m_szExternalFile;
}

CZipString CZipStorage::ChangeSplitRead()
{
	bool lastPart = (ZIP_SIZE_TYPE)m_uCurrentVolume == m_uSplitData;
	CZipString szVolumeName = GetSplitVolumeName(lastPart);
	if (m_pChangeVolumeFunc)
	{
		int iCode = CZipSegmCallback::scVolumeNeededForRead;
		for(;;)
		{
			CallCallback(lastPart ? ZIP_SPLIT_LAST_VOLUME : 0, iCode, szVolumeName);
			if (ZipPlatform::FileExists(m_pChangeVolumeFunc->m_szExternalFile))
			{
				szVolumeName = m_pChangeVolumeFunc->m_szExternalFile;
				break;
			}
			else
				iCode = CZipSegmCallback::scFileNotFound;
		}
	}
	m_pFile->Close();
	return szVolumeName;
}

CZipString CZipStorage::RenameLastFileInSplitArchive()
{
	ASSERT(IsSplit());

	// give to the last volume the zip extension
	CZipString szFileName = m_pFile->GetFilePath();
	CZipString szNewFileName = GetSplitVolumeName(true);
	if (m_pChangeVolumeFunc)
	{
		int code = CZipSegmCallback::scVolumeNeededForWrite;
		for(;;)
		{
			CallCallback(ZIP_SPLIT_LAST_VOLUME, code, szNewFileName);
			szNewFileName = m_pChangeVolumeFunc->m_szExternalFile;
			if (ZipPlatform::FileExists(szNewFileName))
				code = CZipSegmCallback::scFileNameDuplicated;
			else
				break;
		}
	}

	m_pFile->Flush();
	m_pFile->Close();

	ZIPSTRINGCOMPARE pCompare = GetCZipStrCompFunc(ZipPlatform::GetSystemCaseSensitivity());
	if ((szFileName.*(pCompare))(szNewFileName) == 0)
	{
		return szNewFileName;
	}

	if (!m_pChangeVolumeFunc && ZipPlatform::FileExists(szNewFileName))
	{
		ZipPlatform::RemoveFile(szNewFileName);
	}
	ZipPlatform::RenameFile(szFileName, szNewFileName);
	return szNewFileName;
}

CZipString CZipStorage::Close(bool bWrite, bool bGetLastVolumeName)
{
	bool bClose = true;
	CZipString sz;
	if (bWrite)
	{
		Flush();
		if (IsSplit() && !IsExisting())
		{
			sz = RenameLastFileInSplitArchive();
			bClose = false;// already closed in RenameLastFileInSplitArchive
		}
	}

	if (bGetLastVolumeName && sz.IsEmpty())
	{
		if (IsSplit() && IsExisting())
			sz = m_pSplitNames->GetVolumeName(m_szArchiveName, (ZIP_VOLUME_TYPE)(m_uSplitData + 1), CZipSplitNamesHandler::flLast | CZipSplitNamesHandler::flExisting);
		else
			sz = m_pFile->GetFilePath();
	}
	
	if (bClose)
	{
		if (bWrite)
			FlushFile();
		if (m_state.IsSetAny(stateAutoClose))
			m_pFile->Close();
	}

	m_pWriteBuffer.Release();
	m_uCurrentVolume = ZIP_VOLUME_NUMBER_UNSPECIFIED;
	m_state = 0;
	m_pFile = NULL;
	m_uBytesBeforeZip = 0;
	ClearSplitNames();
	ClearCachedSizes();
	return sz;
}

void CZipStorage::NextVolume(ZIP_SIZE_TYPE uNeeded)
{
	Flush();
	ASSERT(IsSegmented());
	bool bSpan = IsSpanned();
	if (m_uBytesWritten)
	{
		m_uBytesWritten = 0;
		m_uCurrentVolume++;
		ZIP_VOLUME_TYPE uMaxVolumes = (ZIP_VOLUME_TYPE)(bSpan ? 999 : 0xFFFF);
		if (m_uCurrentVolume >= uMaxVolumes) // m_uCurrentVolume is a zero-based index
			ThrowError(CZipException::tooManyVolumes);
	} 

	CZipString szFileName;
	
	if (bSpan)
		szFileName  = m_szArchiveName;
	else
		szFileName =  GetSplitVolumeName(false);

	if (!m_pFile->IsClosed())
	{
		m_pFile->Flush();
		if (IsBinarySplit())
			m_pCachedSizes->Add(m_pFile->GetLength());
		m_pFile->Close();
	}	

	if (m_pChangeVolumeFunc)
	{
		int iCode = CZipSegmCallback::scVolumeNeededForWrite;
		for(;;)
		{
			CallCallback(uNeeded, iCode, szFileName);
			// allow changing of the filename
			szFileName = m_pChangeVolumeFunc->m_szExternalFile;

			if (ZipPlatform::FileExists(szFileName))
				iCode = CZipSegmCallback::scFileNameDuplicated;
			else
			{
				if (bSpan)
				{
					CZipString label;
					label.Format(_T("pkback# %.3d"), m_uCurrentVolume + 1);
					if (!ZipPlatform::SetVolLabel(szFileName, label))
					{
						iCode = CZipSegmCallback::scCannotSetVolLabel;
						continue;
					}
				}					
				
				if (OpenFile(szFileName, CZipFile::modeCreate | CZipFile::modeReadWrite, false))
					break;
				else
					iCode = CZipSegmCallback::scFileCreationFailure;
			}

		}
		m_uCurrentVolSize = bSpan ? GetFreeVolumeSpace() : m_uSplitData;
	}
	else
	{
		if (bSpan)
			ThrowError(CZipException::internalError);
		m_uCurrentVolSize = m_uSplitData;
		OpenFile(szFileName, CZipFile::modeCreate | CZipFile::modeReadWrite);
	}
}

void CZipStorage::CallCallback(ZIP_SIZE_TYPE uNeeded, int iCode, CZipString szTemp)
{
	if (!m_pChangeVolumeFunc)
		ThrowError(CZipException::internalError);
	m_pChangeVolumeFunc->m_szExternalFile = szTemp;
	m_pChangeVolumeFunc->m_uVolumeNeeded = (ZIP_VOLUME_TYPE)(m_uCurrentVolume + 1);
	m_pChangeVolumeFunc->m_iCode = iCode; 
	if (!m_pChangeVolumeFunc->Callback(uNeeded))
		CZipException::Throw(CZipException::aborted, szTemp);
}

ZIP_SIZE_TYPE CZipStorage::GetFreeVolumeSpace() const
{
	ASSERT (IsSpanned());
	CZipString szTemp = m_pFile->GetFilePath();
	if (szTemp.IsEmpty()) // called once when creating a segmented archive
		return 0;
	else
	{
		CZipPathComponent zpc(szTemp);
        ULONGLONG ret = ZipPlatform::GetDeviceFreeSpace(zpc.GetFilePath());
        if (ret > (ZIP_SIZE_TYPE)(-1))
			return (ZIP_SIZE_TYPE)(-1);
        else
			return (ZIP_SIZE_TYPE)ret;
	}
}


void CZipStorage::UpdateSegmMode(ZIP_VOLUME_TYPE uLastDisk)
{
	bool binarySplit = IsBinarySplit();
	if (!binarySplit)
		m_uCurrentVolume = uLastDisk;

	if (uLastDisk > 0 || binarySplit)
	{
		// segmentation detected
		CZipString szFilePath = m_pFile->GetFilePath();
		if (!m_state.IsSetAny(stateSegmented))
			m_state.Set(ZipPlatform::IsDriveRemovable(szFilePath) ? stateSpan : stateSplit);
		
		if (IsSpanned())
		{
			if (!m_pSpanChangeVolumeFunc)
				ThrowError(CZipException::noCallback);
			m_pChangeVolumeFunc = m_pSpanChangeVolumeFunc;
		}
		else /*if (IsSplit())*/
		{
			EnsureSplitNames();
			if (!binarySplit)
				m_uSplitData = uLastDisk; // the last volume
			m_pChangeVolumeFunc = m_pSplitChangeVolumeFunc;
		}
		m_pWriteBuffer.Release(); // no need for this in this case
	}
	else
		// it will clear all segmented flags
		m_state.Clear(stateSpan | stateBinarySplit);
}

ZIP_SIZE_TYPE CZipStorage::AssureFree(ZIP_SIZE_TYPE uNeeded)
{
	ZIP_SIZE_TYPE uFree;
	while ((uFree = VolumeLeft()) < uNeeded)
	{
		if (IsSplit() && !m_uBytesWritten && !m_uBytesInWriteBuffer)
			// in the splitArchive mode, if the size of the archive is less 
			// than the size of the packet to be written at once,
			// increase once the size of the volume
			m_uCurrentVolSize = uNeeded;
		else
			NextVolume(uNeeded);
	}
	return uFree;
}

void CZipStorage::Write(const void *pBuf, DWORD iSize, bool bAtOnce)
{
	if (!IsSegmented())
		WriteInternalBuffer((char*)pBuf, iSize);
	else
	{
		bool atOnce = bAtOnce && !IsBinarySplit();
		// if not at once, one byte is enough of free space
		DWORD iNeeded = atOnce ? iSize : 1; 
		DWORD uTotal = 0;

		while (uTotal < iSize)
		{			
			ZIP_SIZE_TYPE uFree = AssureFree(iNeeded);
			DWORD uLeftToWrite = iSize - uTotal;
			DWORD uToWrite = uFree < uLeftToWrite ? (DWORD)uFree : uLeftToWrite;
			WriteInternalBuffer((char*)pBuf + uTotal, uToWrite);
			if (atOnce)
				return;
			else
				uTotal += uToWrite;
		}

	}
}


void CZipStorage::WriteInternalBuffer(const char *pBuf, DWORD uSize)
{
	DWORD uWritten = 0;
	while (uWritten < uSize)
	{
		DWORD uFreeInBuffer = GetFreeInBuffer();
		if (uFreeInBuffer == 0)
		{
			Flush();
			uFreeInBuffer = m_pWriteBuffer.GetSize();
		}
		DWORD uLeftToWrite = uSize - uWritten;
		DWORD uToCopy = uLeftToWrite < uFreeInBuffer ? uLeftToWrite : uFreeInBuffer;
		memcpy((char*)m_pWriteBuffer + m_uBytesInWriteBuffer, pBuf + uWritten, uToCopy);
		uWritten += uToCopy;
		m_uBytesInWriteBuffer += uToCopy;
	}
}

ZIP_SIZE_TYPE CZipStorage::VolumeLeft() const
{
	// for spanned archives m_uCurrentVolSize is updated after each flush()
	ZIP_SIZE_TYPE uBytes = m_uBytesInWriteBuffer + (IsSpanned() ? 0 : m_uBytesWritten);	
	return uBytes > m_uCurrentVolSize ? 0 : m_uCurrentVolSize - uBytes;
}

void CZipStorage::Flush()
{	
	if (m_uBytesInWriteBuffer)
	{
		m_pFile->Write(m_pWriteBuffer, m_uBytesInWriteBuffer);
		if (IsSegmented())
			m_uBytesWritten += m_uBytesInWriteBuffer;
		m_uBytesInWriteBuffer = 0;
	}
	if (IsSpanned()) 
		// after writing it is difficult to predict the free space due to 
		// not completely written clusters, write operation may start from a new cluster
		m_uCurrentVolSize = GetFreeVolumeSpace();	
}

ZIP_FILE_USIZE CZipStorage::LocateSignature(char* szSignature, ZIP_SIZE_TYPE uMaxDepth)
{
	m_pFile->SeekToEnd();
	int leftToFind = SIGNATURE_SIZE - 1;
	bool found = false; // for fast checking if leftToFind needs resetting	
	if (!IsBinarySplit())
	{
		return LocateSignature(szSignature, uMaxDepth, leftToFind, found, m_pFile->GetLength());
	}
	else
	{		
		for(;;)
		{
			ZIP_FILE_USIZE uFileLength = GetCachedSize(m_uCurrentVolume);
			ZIP_FILE_USIZE position = LocateSignature(szSignature, uMaxDepth, leftToFind, found, uFileLength);
			if (position != SignatureNotFound || uMaxDepth <= uFileLength)
				return position;
			uMaxDepth -= (ZIP_SIZE_TYPE)uFileLength;
			if (m_uCurrentVolume == 0)
				return SignatureNotFound;
			ChangeVolumeDec();
			m_pFile->SeekToEnd();
		}
	}
}

ZIP_FILE_USIZE CZipStorage::LocateSignature(char* szSignature, ZIP_SIZE_TYPE uMaxDepth, int& leftToFind, bool& found, ZIP_FILE_USIZE uFileLength)
{
	CZipAutoBuffer buffer(m_iLocateBufferSize);
	
	ZIP_SIZE_TYPE max = (ZIP_SIZE_TYPE)(uFileLength < uMaxDepth ? uFileLength : uMaxDepth);
	ZIP_SIZE_TYPE position = (ZIP_SIZE_TYPE)(uFileLength - m_pFile->GetPosition());
	int offset = 0;
	
	int toRead = m_iLocateBufferSize;
	
	while ( position < max )
	{
		position += toRead;
		if ( position > max )
		{
			int diff = (int) ( position - max );
			toRead -= diff;
			offset = diff;
			position = max;
		}
		Seek(position, seekFromEnd);	
		int actuallyRead = m_pFile->Read((char*)buffer + offset, toRead);
		if (actuallyRead != toRead)
			ThrowError(CZipException::badZipFile);
		int pos = m_iLocateBufferSize - 1;
		while ( pos >= offset )
		{
			if ( buffer[pos] == szSignature[leftToFind] )
			{
				if ( leftToFind == 0 )
					return (ZIP_FILE_USIZE)(uFileLength - ( position - ( pos - offset ) ));
				if ( !found )
					found = true;
				leftToFind--;
				pos--;
			}
			else if ( found )
			{
				leftToFind = SIGNATURE_SIZE - 1;
				found = false;
				// do not decrease position, the current pos may be the first to find
			}
			else
				pos--;
		}
	}
	return SignatureNotFound;	
}

void CZipStorage::SeekInBinary(ZIP_FILE_SIZE lOff, bool bSeekToBegin)
{
	ASSERT(IsBinarySplit());
	if (bSeekToBegin)
		m_pFile->SeekToBegin();

	if (lOff == 0)
		return;

	if (lOff > 0)
	{
		ZIP_SIZE_TYPE uPosition = (ZIP_SIZE_TYPE)m_pFile->GetPosition();
		ZIP_FILE_USIZE uLength = GetCachedSize(m_uCurrentVolume);
		if ((ZIP_FILE_USIZE)(uPosition + lOff) < uLength)
		{
			m_pFile->Seek(lOff, CZipAbstractFile::current);
			return;
		}
		ZIP_VOLUME_TYPE uVolume = (ZIP_VOLUME_TYPE)(m_uCurrentVolume + 1);
		lOff -= uLength - uPosition;
		for(;;)
		{
			uLength = GetCachedSize(uVolume);
			if ((ZIP_FILE_USIZE)lOff < uLength)
			{
				ChangeVolume(uVolume);
				if (lOff > 0)
				{					
					m_pFile->Seek(lOff, CZipAbstractFile::current);
				}
				return;
			}
			else
				lOff -= uLength;
			uVolume++;
		}
	}
	else
	{
		ZIP_SIZE_TYPE uPosition = (ZIP_SIZE_TYPE)m_pFile->GetPosition();
		if (uPosition >= (ZIP_SIZE_TYPE)(-lOff))
		{
			m_pFile->Seek(lOff, CZipAbstractFile::current);
			return;
		}
		lOff += uPosition;
		ZIP_VOLUME_TYPE uVolume = (ZIP_VOLUME_TYPE)(m_uCurrentVolume - 1);
		for(;;)
		{
			ZIP_FILE_USIZE uLength = GetCachedSize(uVolume);
			if (uLength >= (ZIP_SIZE_TYPE)(-lOff))
			{
				ChangeVolume(uVolume);
				if (lOff < 0)
				{					
					m_pFile->Seek(lOff, CZipAbstractFile::end);
				}
				return;
			}
			else
				lOff += uLength;
			if (uVolume == 0)
				ThrowError(CZipException::genericError);
			uVolume--;
		}
	}
}

ULONGLONG CZipStorage::Seek(ULONGLONG lOff, SeekType iSeekType)
{	
	if (iSeekType == seekCurrent)
	{		
		if (IsExistingSegmented())
		{
			ZIP_SIZE_TYPE uPosition = (ZIP_SIZE_TYPE)m_pFile->GetPosition();
			ZIP_FILE_USIZE uLength = m_pFile->GetLength();
			while (uPosition + lOff >= uLength)
			{
				ZIP_SIZE_TYPE uCanSeek = (ZIP_SIZE_TYPE)(uLength - uPosition);
				lOff -= uCanSeek;
				ChangeVolume();
				uPosition = 0;
				uLength = m_pFile->GetLength();
			}
			return lOff > 0 ? m_pFile->SafeSeek((ZIP_FILE_USIZE)lOff) : 0;
		}
		else
			return m_pFile->Seek((ZIP_FILE_SIZE)lOff, CZipAbstractFile::current);			
	}
	else
	{
		if (m_uCurrentVolume == 0 && m_uBytesBeforeZip > 0)
			lOff += m_uBytesBeforeZip;
		return m_pFile->SafeSeek((ZIP_FILE_USIZE)lOff, iSeekType == seekFromBeginning);
	}
}

void CZipStorage::FinalizeSegm()
{
	ASSERT(IsNewSegmented()); // spanned archive in creation

	CZipString szFileName;
	if (IsSplit())
	{
		// the file is already closed
		szFileName = RenameLastFileInSplitArchive();
		if (IsBinarySplit() && m_uCurrentVolume != 0)
		{
			ZIP_SIZE_TYPE size;
			ZipPlatform::GetFileSize(szFileName, size);
			m_pCachedSizes->Add(ZIP_FILE_USIZE(size));
		}
	}
	else
	{
		szFileName = m_pFile->GetFilePath();
		m_pFile->Close();
	}
	m_state.Set(stateExisting);
	if (m_uCurrentVolume == 0) // one-volume segmented archive was converted to a normal archive
	{
		if (IsSplit())
		{
			ClearSplitNames();
			if (IsBinarySplit())
				ClearCachedSizes();
		}
		m_state.Clear(stateSplit | stateBinarySplit | stateSpan);
	}
	else
	{
		m_uSplitData = m_uCurrentVolume;
		if (IsSplit())
		{
			m_szArchiveName = szFileName;
		}
	}

	OpenFile(szFileName, CZipFile::modeNoTruncate | (IsSegmented() ? CZipFile::modeReadWrite : CZipFile::modeRead));	
}

