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

#include "ZipAbstractFile.h"
#include "ZipString.h"
#include "ZipExport.h"

#ifndef __GNUC__
	#include <io.h>
#else
	#include <unistd.h>
	#include <errno.h>	
#endif

#if !defined (_MSC_VER) || _MSC_VER < 1400	

// there seems to be a problem under Windows sometimes when using one of the functions below 
// without the underscore at the beginning	
#ifndef _lseek
	#define _lseek lseek
#endif

#ifndef _read
	#define _read read
#endif

#ifndef _close
	#define _close close
#endif

#ifndef _tell
	#define _tell tell
#endif

#ifndef _write
	#define _write write
#endif

#endif 
class ZIP_API CZipFile : public CZipAbstractFile
{
	void ThrowError() const;
public:
	int m_hFile;
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
	ZIP_FILE_USIZE GetLength() const;
	CZipString GetFilePath() const {return m_szFileName;}
	bool HasFilePath() const
	{
		return true;
	}
	bool IsClosed()const { return m_hFile == -1;}
	bool Open(LPCTSTR lpszFileName, UINT openFlags, bool bThrow);
	void Close(); 

	void Write(const void* lpBuf, UINT nCount);
	ZIP_FILE_USIZE GetPosition() const;	
	void SetLength(ZIP_FILE_USIZE uNewLen);
	UINT Read(void *lpBuf, UINT nCount);
	ZIP_FILE_USIZE Seek(ZIP_FILE_SIZE dOff, int nFrom);
	
	virtual ~CZipFile (){Close();};
protected:
	CZipString m_szFileName;

};
