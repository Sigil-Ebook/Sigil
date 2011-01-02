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
* \file ZipAbstractFile.h
*	Includes the CZipAbstractFile class.
*
*/

#if !defined(ZIPARCHIVE_ZIPABSTRACTFILE_DOT_H)
#define ZIPARCHIVE_ZIPABSTRACTFILE_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif
#include "ZipExport.h"
#include "ZipString.h"

class ZIP_API CZipAbstractFile
{
public:

	enum 
	{	begin	= SEEK_SET, // 0
		current = SEEK_CUR, // 1
		end		= SEEK_END  // 2
	};
	CZipAbstractFile(){}
	virtual bool Open(LPCTSTR , UINT , bool ){return false;}
	virtual void Close() = 0;
	virtual void Flush() = 0;
	virtual ZIP_FILE_USIZE GetPosition() const = 0;	
	virtual ZIP_FILE_USIZE Seek(ZIP_FILE_SIZE lOff, int nFrom) = 0;
	ZIP_FILE_USIZE SafeSeek(ZIP_FILE_USIZE lOff, bool fromBeginning = true)
	{
		ZIP_FILE_SIZE offset;
		if (lOff > ZIP_FILE_SIZEMAX)
		{
			offset = GetLength() - lOff;
			fromBeginning = !fromBeginning;
		}
		else
			offset = (ZIP_FILE_USIZE)lOff;
		
		if (fromBeginning)
			return Seek(offset, CZipAbstractFile::begin);
		else
			return Seek(-offset, CZipAbstractFile::end);	
	}
	virtual ZIP_FILE_USIZE GetLength() const = 0;
	virtual void SetLength(ZIP_FILE_USIZE nNewLen) = 0;	
	virtual ZIP_FILE_USIZE SeekToBegin(){return Seek(0, begin);}
	virtual ZIP_FILE_USIZE SeekToEnd(){return Seek(0, end);}
	virtual CZipString GetFilePath() const = 0;	
	virtual bool HasFilePath() const = 0;	
	virtual UINT Read(void *lpBuf, UINT nCount) = 0;
	virtual void Write(const void* lpBuf, UINT nCount) = 0;	
	virtual bool IsClosed() const = 0;	
	virtual ~CZipAbstractFile(){};

};



#endif // !defined(ZIPARCHIVE_ZIPABSTRACTFILE_DOT_H)
