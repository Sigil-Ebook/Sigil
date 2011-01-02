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

#ifndef ZIPARCHIVE_ZIPFILE_DOT_H
	#error Do not include this file directly. Include ZipFile.h instead
#endif

#if !defined _ZIP_SYSTEM_WIN || defined _ZIP_SYSTEM_LINUX
	#error This implementation can be used under the Windows platform only.
#endif

#include "ZipAbstractFile.h"
#include "ZipString.h"
#include "ZipExport.h"

class ZIP_API CZipFile : public CZipAbstractFile
{
	void ThrowError() const;
public:
	HANDLE m_hFile;
	operator HANDLE();

	enum OpenModes
	{
		modeRead =         0x00000,
		modeWrite =        0x00001,
		modeReadWrite =    0x00002,
		shareExclusive =   0x00010,
		shareDenyWrite =   0x00020,
		shareDenyRead =    0x00030,
		shareDenyNone =    0x00040,
		modeCreate =       0x01000,
		modeNoTruncate =   0x02000
	};
	
	CZipFile();
	CZipFile(LPCTSTR lpszFileName, UINT openFlags);
	void Flush();
	ULONGLONG GetLength() const;
	CZipString GetFilePath() const {return m_szFileName;}
	bool HasFilePath() const
	{
		return true;
	}
	bool IsClosed()const { return m_hFile == INVALID_HANDLE_VALUE;}
	bool Open(LPCTSTR lpszFileName, UINT openFlags, bool bThrow);
	void Close(); 

	void Write(const void* lpBuf, UINT nCount);
	ULONGLONG GetPosition() const;	
	void SetLength(ULONGLONG uNewLen);
	UINT Read(void *lpBuf, UINT nCount);

	ULONGLONG Seek(LONGLONG dOff, int nFrom);
	
	virtual ~CZipFile (){Close();};
protected:
	CZipString m_szFileName;
};