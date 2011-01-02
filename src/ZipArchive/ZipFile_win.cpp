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

#if _ZIP_FILE_IMPLEMENTATION == ZIP_ZFI_WIN

#include "ZipFile.h"
#include "ZipException.h"
#include "ZipPathComponent.h"
#include "BitFlag.h"

#if _MSC_VER < 1300 && !defined INVALID_SET_FILE_POINTER
	#define INVALID_SET_FILE_POINTER (DWORD)(-1)
#endif

CZipFile::CZipFile()
{
	m_hFile = INVALID_HANDLE_VALUE;
}

CZipFile::CZipFile(LPCTSTR lpszFileName, UINT openFlags)
{
	m_hFile = INVALID_HANDLE_VALUE;
	Open(lpszFileName, openFlags, true);
}

void CZipFile::ThrowError() const
{
	CZipException::Throw(CZipException::fileError, m_szFileName);
}

bool CZipFile::Open(LPCTSTR lpszFileName, UINT openFlags, bool bThrow)
{
	if (!IsClosed())
		Close();

	CZipString fileName = lpszFileName;
	if (fileName.IsEmpty())
	{
		return false;
	}
	DWORD access;
	DWORD temp = openFlags & 3;
	if (temp == modeWrite)
	{
		access = GENERIC_WRITE;
	}
	else if (temp == modeReadWrite)
	{
		access = GENERIC_READ | GENERIC_WRITE;
	}
	else
	{
		access = GENERIC_READ;
	}

	DWORD share;
	temp = openFlags & 0x70;
	if (temp == shareDenyWrite)
	{
		share = FILE_SHARE_READ;
	}
	else if (temp == shareDenyRead)
	{
		share = FILE_SHARE_WRITE;
	}
	else if (temp == shareDenyNone)
	{
		share = FILE_SHARE_READ | FILE_SHARE_WRITE;
	}
	else
	{
		share = 0;
	}
	CZipPathComponent::AddPrefix(fileName, false);

	DWORD create;
	if (openFlags & modeCreate)
	{
		if (openFlags & modeNoTruncate)
		{
			create = OPEN_ALWAYS;
		}
		else
		{
			create = CREATE_ALWAYS;
		}
	}
	else
	{
		create = OPEN_EXISTING;
	}

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	m_hFile = ::CreateFile(fileName, access, share, &sa, create, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		if (bThrow)
			ThrowError();
		else
			return false;
	}
		
	m_szFileName = lpszFileName;
	return true;
}


ULONGLONG CZipFile::GetLength() const
{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);
	ULARGE_INTEGER size;
	size.LowPart = GetFileSize(m_hFile, &size.HighPart);
	if (size.LowPart == INVALID_FILE_SIZE && ::GetLastError() != NO_ERROR)
	{
		ThrowError();
	}

	return size.QuadPart;
}


void CZipFile::SetLength(ULONGLONG uNewLen)
{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);
	if (uNewLen > _I64_MAX)
	{
		CZipException::Throw(CZipException::tooBigSize);
	}
	Seek(uNewLen, FILE_BEGIN);
	if (::SetEndOfFile(m_hFile) == FALSE)
	{
		ThrowError();
	}
}

ULONGLONG CZipFile::GetPosition() const
{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);

	// do not call Seek, to keep GetPosition const
#if (defined(NTDDI_VERSION) && NTDDI_VERSION >=NTDDI_WIN2K) || (_WIN32_WINNT >= 0x0500 && WINVER >= 0x0500)	
	LARGE_INTEGER li, oli = {0};
	li.QuadPart = (LONGLONG)0;
	if (::SetFilePointerEx(m_hFile, li, &oli, FILE_CURRENT) == FALSE)	
	{
		ThrowError();
	}
	return oli.QuadPart;
#else
	LARGE_INTEGER li;
	li.QuadPart = (LONGLONG)0;
	li.LowPart = ::SetFilePointer(m_hFile, li.LowPart, &li.HighPart, FILE_CURRENT);
	if (li.LowPart == INVALID_SET_FILE_POINTER  && GetLastError() != NO_ERROR)
	{
		ThrowError();
	}
	return li.QuadPart;
#endif
}

ULONGLONG CZipFile::Seek(LONGLONG dOff, int nFrom)
{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);
	ASSERT(nFrom == FILE_BEGIN || nFrom == FILE_END || nFrom == FILE_CURRENT);

#if (defined(NTDDI_VERSION) && NTDDI_VERSION >=NTDDI_WIN2K) || (_WIN32_WINNT >= 0x0500 && WINVER >= 0x0500)
	
	LARGE_INTEGER li, oli = {0};
	li.QuadPart = (LONGLONG)dOff;
	if (::SetFilePointerEx(m_hFile, li, &oli, (DWORD)nFrom) == FALSE)	
	{
		ThrowError();
	}
	return oli.QuadPart;
#else
	LARGE_INTEGER li;
	li.QuadPart = (LONGLONG)dOff;
	li.LowPart = ::SetFilePointer(m_hFile, li.LowPart, &li.HighPart, (DWORD)nFrom);
	if (li.LowPart == INVALID_SET_FILE_POINTER  && GetLastError() != NO_ERROR)
	{
		ThrowError();
	}
	return li.QuadPart;
#endif
}

void CZipFile::Close()
{
	if (IsClosed())
		return;
	bool ok = ::CloseHandle(m_hFile) == TRUE;
	m_hFile = INVALID_HANDLE_VALUE;
	m_szFileName.Empty();
	if (!ok)
	{
		ThrowError();
	}
}

void CZipFile::Flush()
{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);
	if (::FlushFileBuffers(m_hFile) == FALSE)
	{
		ThrowError();
	}
}

CZipFile::operator HANDLE()
{
	return m_hFile;
}


void CZipFile::Write(const void* lpBuf, UINT nCount)
{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);
	if (nCount == 0)
	{
		return;
	}
	DWORD written = 0;
	if (::WriteFile(m_hFile, lpBuf, nCount, &written, NULL) == FALSE || written != nCount)
	{
		ThrowError();
	}
}

UINT CZipFile::Read(void *lpBuf, UINT nCount)
{
	ASSERT(m_hFile != INVALID_HANDLE_VALUE);
	if (nCount == 0)
	{
		return 0;
	}
	DWORD read = 0;
	if (::ReadFile(m_hFile, lpBuf, nCount, &read, NULL) == FALSE)
	{
		ThrowError();
	}
	return (UINT)read;

}

#endif