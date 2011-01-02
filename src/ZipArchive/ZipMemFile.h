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
* \file ZipMemFile.h
* Includes the CZipMemFile class.
*
*/
#if !defined(ZIPARCHIVE_ZIPMEMFILE_DOT_H)
#define ZIPARCHIVE_ZIPMEMFILE_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "ZipAbstractFile.h"
#include "ZipString.h"
#include "ZipExport.h"

/**
	Represents a file in memory.
	Automatically grows when necessary.
*/
class ZIP_API CZipMemFile : public CZipAbstractFile
// when ZIP_ZFI_WIN is defined under VS 6.0, do not derive from CFile, as sizes are limited to DWORD
#if defined _ZIP_IMPL_MFC && (_MSC_VER >= 1300 || _ZIP_FILE_IMPLEMENTATION != ZIP_ZFI_WIN) 
	, public CFile
#endif
{
protected:
	size_t m_nGrowBy, m_nPos;
	size_t m_nBufSize, m_nDataSize;
	BYTE* m_lpBuf;
	bool m_bAutoDelete;
	void Free()
	{
		if (m_lpBuf)
		{
			free(m_lpBuf);
			m_lpBuf = NULL;
		}
	}
	void Init()
	{
		m_nGrowBy = m_nPos = 0;
		m_nBufSize = m_nDataSize = 0;
		m_lpBuf = NULL;

	}
	void Grow(size_t nBytes);
public:
#if defined _ZIP_IMPL_MFC && (_MSC_VER >= 1300 || _ZIP_FILE_IMPLEMENTATION != ZIP_ZFI_WIN) 
	DECLARE_DYNAMIC(CZipMemFile)
#endif
	bool IsClosed() const { return m_lpBuf == NULL;}
	void Flush(){}

	ZIP_FILE_USIZE Seek(ZIP_FILE_SIZE lOff, int nFrom);
	ZIP_FILE_USIZE GetLength() const {return m_nDataSize;}
	void Write(const void* lpBuf, UINT nCount);
	UINT Read(void* lpBuf, UINT nCount);
	void SetLength(ZIP_FILE_USIZE nNewLen);
	CZipString GetFilePath() const  {return _T("");} 	
	bool HasFilePath() const
	{
		return false;
	}

	CZipMemFile(long nGrowBy = 1024)
	{
		Init();
		m_nGrowBy = nGrowBy;
		m_bAutoDelete = true;
	}

	CZipMemFile(BYTE* lpBuf, UINT nBufSize, long nGrowBy = 0)
	{
		Init();
		Attach(lpBuf, nBufSize, nGrowBy);
	}

	CZipMemFile(CZipMemFile& from)
	{
		Copy(from);
	}

	void Copy(CZipMemFile& from)
	{
		SetLength(from.m_nDataSize);
		from.Read(m_lpBuf, (UINT)from.m_nDataSize);
	}

	ZIP_FILE_USIZE GetPosition() const {	return m_nPos;}
	void Attach(BYTE* lpBuf, UINT nBufSize, long nGrowBy = 0)
	{
		Close();
		m_lpBuf = lpBuf;
		m_nGrowBy = nGrowBy;
		m_nBufSize = nBufSize;
		m_nDataSize = nGrowBy == 0 ? nBufSize : 0;
		m_bAutoDelete = false;
		m_nPos = 0;
	}

	void ReInit(long nGrowBy = 1024)
	{
		Close();
		Init();
		m_nGrowBy = nGrowBy;
		m_bAutoDelete = true;
	}

	BYTE* Detach()
	{
		BYTE* b = m_lpBuf;
		Init();
		return b;
	}
	void Close()
	{
		if (m_bAutoDelete)
			Free();
		Init();
	}
	virtual ~CZipMemFile(){Close();}

};

#endif // !defined(ZIPARCHIVE_ZIPMEMFILE_DOT_H)
