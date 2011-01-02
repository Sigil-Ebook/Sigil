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

#if (defined _ZIP_IMPL_STL && (!defined _ZIP_FILE_IMPLEMENTATION || _ZIP_FILE_IMPLEMENTATION == ZIP_ZFI_DEFAULT)) || _ZIP_FILE_IMPLEMENTATION == ZIP_ZFI_STL

#if defined __APPLE__ || defined __CYGWIN__
	#define FILE_FUNCTIONS_64B_BY_DEFAULT
#else
	#undef FILE_FUNCTIONS_64B_BY_DEFAULT	
#endif	

#include "ZipFile.h"
#include "ZipException.h"
#include "ZipPlatform.h"
#include "BitFlag.h"

#include <fcntl.h>

CZipFile::CZipFile(LPCTSTR lpszFileName, UINT openFlags)
{
	m_hFile = -1;
	Open(lpszFileName, openFlags, true);
}

CZipFile::CZipFile()
{
	m_hFile = -1;
}

void CZipFile::ThrowError() const
{
	CZipException::Throw(errno, m_szFileName);
}


ZIP_FILE_USIZE CZipFile::GetLength() const
{
	// cannot use Seek here, Seek is not const
	ZIP_SIZE_TYPE lLen, lCur;
#ifdef FILE_FUNCTIONS_64B_BY_DEFAULT
	lCur = (ZIP_SIZE_TYPE)_lseek(m_hFile, 0, current);
#else
	lCur = (ZIP_SIZE_TYPE)_lseeki64(m_hFile, 0, current);	
#endif
	if (lCur == (ZIP_SIZE_TYPE)-1)
		ThrowError();
#ifdef FILE_FUNCTIONS_64B_BY_DEFAULT
	lLen = (ZIP_SIZE_TYPE)_lseek(m_hFile, 0, end);
#else
	lLen = (ZIP_SIZE_TYPE)_lseeki64(m_hFile, 0, end);	
#endif

	// first go back
#ifdef FILE_FUNCTIONS_64B_BY_DEFAULT
	bool err = _lseek(m_hFile, lCur, begin) == -1;
#else
	bool err = _lseeki64(m_hFile, lCur, begin) == -1;	
#endif

	if (err || lLen == (ZIP_SIZE_TYPE)-1)
		ThrowError();
	return lLen;

}


bool CZipFile::Open(LPCTSTR lpszFileName, UINT openFlags, bool bThrow)
{
	if (!IsClosed())
		Close();

#ifdef O_BINARY
	UINT iNewFlags = O_BINARY;
#else
	UINT iNewFlags = 0;
#endif

	bool bReadOnly = false;
	DWORD temp = openFlags & 3;
	if (temp == modeWrite)
	{
		iNewFlags |= O_WRONLY;
	}
	else if (temp == modeReadWrite)
	{
		iNewFlags |= O_RDWR;
	}
	else
	{
		// O_RDONLY is defined as 0
		bReadOnly = true;
		iNewFlags |= O_RDONLY;
	}
	
	if (openFlags & modeCreate)
		iNewFlags |= O_CREAT;

	if (!(openFlags & modeNoTruncate) && !bReadOnly)
		iNewFlags |= O_TRUNC;

	m_hFile = ZipPlatform::OpenFile(lpszFileName, iNewFlags, openFlags & 0x70);
	if (m_hFile == -1)
		if (bThrow)
			CZipException::Throw(errno, lpszFileName);
		else
			return false;
	m_szFileName = lpszFileName;
	return true;
}


void CZipFile::SetLength(ULONGLONG uNewLen)
{
	if (!ZipPlatform::TruncateFile(m_hFile, uNewLen))
	{
		ThrowError();
	}
}

ZIP_FILE_USIZE CZipFile::GetPosition() const
{
#ifdef FILE_FUNCTIONS_64B_BY_DEFAULT
	#ifndef __GNUC__
		ZIP_FILE_USIZE ret = _tell(m_hFile);
	#else
		ZIP_FILE_USIZE ret = lseek(m_hFile, 0, SEEK_CUR);
	#endif
#else
	#ifndef __GNUC__
		ZIP_FILE_USIZE ret = (ZIP_FILE_USIZE)_telli64(m_hFile);
	#else
		ZIP_FILE_USIZE ret = (ZIP_FILE_USIZE)lseek64(m_hFile, 0, SEEK_CUR);
	#endif			
#endif
	if (ret == (ZIP_FILE_USIZE)-1)
		ThrowError();
	return ret;
}

ZIP_FILE_USIZE CZipFile::Seek(ZIP_FILE_SIZE dOff, int nFrom)
{
	// restricted to signed
#ifdef FILE_FUNCTIONS_64B_BY_DEFAULT
	ZIP_FILE_USIZE ret = (ZIP_FILE_USIZE)_lseek(m_hFile, dOff, nFrom);
#else
	ZIP_FILE_USIZE ret = (ZIP_FILE_USIZE)_lseeki64(m_hFile, dOff, nFrom);
#endif
	if (ret == (ZIP_FILE_USIZE)-1)
		ThrowError();
	return (ZIP_FILE_USIZE)ret;
}

void CZipFile::Close()
{
	if (IsClosed())
		return;
	if (_close(m_hFile) != 0)
		ThrowError();
	else
	{
		m_szFileName.Empty();
		m_hFile = -1;
	}
}

void CZipFile::Flush()
{
	if (!ZipPlatform::FlushFile(m_hFile)) 
		ThrowError();
}

CZipFile::operator HANDLE()
{
	intptr_t fh = ZipPlatform::GetFileSystemHandle(m_hFile);
	if (fh == -1)
		ThrowError();
	return (HANDLE)fh;
}

void CZipFile::Write(const void* lpBuf, UINT nCount)
{
	if (nCount == 0)
	{
		return;
	}
	if (_write(m_hFile, lpBuf, nCount) != (int) nCount)
		ThrowError();
}

UINT CZipFile::Read(void *lpBuf, UINT nCount)
{
	if (nCount == 0)
	{
		return 0;
	}
	errno = 0;
	int ret = _read(m_hFile, lpBuf, nCount);
	if (ret < (int) nCount && errno != 0)
		ThrowError();
	return ret;

}

#endif
