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

#ifndef ZIPARCHIVE_ZIPMUTEX_DOT_H
	#error Do not include this file directly. Include ZipMutex.h instead
#endif

#ifdef _ZIP_USE_LOCKING

#include "ZipException.h"

namespace ZipArchiveLib
{
class ZIP_API CZipMutex
{
	HANDLE m_handle;
public:
	CZipMutex(bool bOpen = false)
	{		
		if (bOpen)
			Open();
		else
			m_handle = NULL;
	}

	void Open()
	{
		Close();
		m_handle = ::CreateMutex(NULL, FALSE, NULL);
		if (m_handle == NULL)
			CZipException::Throw(CZipException::mutexError);
	}

	void Lock()
	{
		DWORD dwRet = ::WaitForSingleObject(m_handle, INFINITE);
		if (dwRet != WAIT_OBJECT_0 && dwRet != WAIT_ABANDONED)
			CZipException::Throw(CZipException::mutexError);
	}

	void Unlock()
	{
		if (!::ReleaseMutex(m_handle))
			CZipException::Throw(CZipException::mutexError);
	}

	void Close()
	{
		if (m_handle != NULL)
		{
			::CloseHandle(m_handle);
			m_handle = NULL;
		}
	}

	CZipMutex& operator=(const CZipMutex&)
	{
		m_handle = NULL;
		return *this;
	}

	~CZipMutex()
	{
		Close();
	}
};

}

#endif
