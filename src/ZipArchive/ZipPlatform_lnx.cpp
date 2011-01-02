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

#ifdef _ZIP_SYSTEM_LINUX

#if defined __APPLE__ || defined __CYGWIN__
	#define FILE_FUNCTIONS_64B_BY_DEFAULT
#else
	#undef FILE_FUNCTIONS_64B_BY_DEFAULT	
#endif	

#include "ZipPlatform.h"
#include "ZipFileHeader.h"
#include "ZipException.h"
#include "ZipAutoBuffer.h"

#include <utime.h>

#include "ZipPathComponent.h"
#include "ZipCompatibility.h"

#include <sys/types.h>

#if defined (__FreeBSD__) || defined (__APPLE__)
	#include <sys/param.h>
	#include <sys/mount.h>
#else
	#include <sys/vfs.h>
#endif
#include <sys/stat.h>

#include <unistd.h>
#include <stdio.h>

#include <fcntl.h>

const TCHAR CZipPathComponent::m_cSeparator = _T('/');

#ifndef _UTIMBUF_DEFINED
#define _utimbuf utimbuf
#endif

#define ZIP_DEFAULT_DIR_ATTRIBUTES (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
ULONGLONG ZipPlatform::GetDeviceFreeSpace(LPCTSTR lpszPath)
{
	struct statfs sStats;

	#if defined (__SVR4) && defined (__sun)
		if (statvfs(lpszPath, &sStats) == -1) // Solaris
	#else
		if (statfs(lpszPath, &sStats) == -1)
	#endif
		return 0;

        return sStats.f_bsize * sStats.f_bavail;
}


CZipString ZipPlatform::GetTmpFileName(LPCTSTR lpszPath, ZIP_SIZE_TYPE uSizeNeeded)
{
	TCHAR empty[] = _T(""), prefix [] = _T("zar");
	CZipString tempPath = lpszPath;
	if (tempPath.IsEmpty())
		tempPath = "/tmp";
	if (uSizeNeeded > 0 && ZipPlatform::GetDeviceFreeSpace(tempPath) < uSizeNeeded)
		return empty;
	CZipPathComponent::AppendSeparator(tempPath);
	tempPath += prefix;
	tempPath += _T("XXXXXX");
	int handle = mkstemp(tempPath.GetBuffer(tempPath.GetLength()));
	tempPath.ReleaseBuffer();
	if (handle != -1)
	{
		close(handle); // we just create the file and open it later
		return tempPath;
	}
	else
		return empty;		
}

bool ZipPlatform::GetCurrentDirectory(CZipString& sz)
{
	char* pBuf = getcwd(NULL, 0);
	if (!pBuf)
		return false;
	sz = pBuf;
	free(pBuf);
	return true;
}

bool ZipPlatform::SetFileAttr(LPCTSTR lpFileName, DWORD uAttr)
{
	return chmod(lpFileName, uAttr) == 0;
}

bool ZipPlatform::GetFileAttr(LPCTSTR lpFileName, DWORD& uAttr)
{
	struct stat sStats;
	if (stat(lpFileName, &sStats) == -1)
		return false;
  	uAttr = (sStats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO | S_IFMT));
  	return true;
}

bool ZipPlatform::SetExeAttr(LPCTSTR lpFileName)
{
	DWORD uAttr;
	if (!GetFileAttr(lpFileName, uAttr))
		return false;
	uAttr |= S_IXUSR;
	return ZipPlatform::SetFileAttr(lpFileName, uAttr);
}


bool ZipPlatform::GetFileModTime(LPCTSTR lpFileName, time_t & ttime)
{
    struct stat st;
	if (stat(lpFileName, &st) != 0)
		return false;

 	ttime = st.st_mtime;
	if (ttime == (time_t)-1)
	{
		ttime = time(NULL);
		return false;
	}
	else
		return true;
}

bool ZipPlatform::SetFileModTime(LPCTSTR lpFileName, time_t ttime)
{
	struct utimbuf ub;
	ub.actime = time(NULL);
	ub.modtime = ttime == -1 ? time(NULL) : ttime; // if wrong file time, set it to the current
	return utime(lpFileName, &ub) == 0;
}


bool ZipPlatform::ChangeDirectory(LPCTSTR lpDirectory)
{
	return chdir(lpDirectory) == 0; 
}
int ZipPlatform::FileExists(LPCTSTR lpszName)
{
    	struct stat st;
	if (stat(lpszName, &st) != 0)
		return 0;
	else
	{
		if (S_ISDIR(st.st_mode))
			return -1;
		else
			return 1;
	}
}

ZIPINLINE  bool ZipPlatform::IsDriveRemovable(LPCTSTR lpszFilePath)
{
	// not implemented
	return true;
}

ZIPINLINE  bool ZipPlatform::SetVolLabel(LPCTSTR lpszPath, LPCTSTR lpszLabel)
{
	// not implemented
        return true;
}

ZIPINLINE void ZipPlatform::AnsiOem(CZipAutoBuffer& buffer, bool bAnsiToOem)
{
	// not implemented
}

ZIPINLINE  bool ZipPlatform::RemoveFile(LPCTSTR lpszFileName, bool bThrow, int iMode)
{
	if ((iMode & ZipPlatform::dfmRemoveReadOnly) != 0)
	{
		DWORD attr;
		if (ZipPlatform::GetFileAttr(lpszFileName, attr)
			&& (ZipCompatibility::GetAsInternalAttributes(attr, ZipPlatform::GetSystemID()) & ZipCompatibility::attROnly) != 0)
		{
			ZipPlatform::SetFileAttr(lpszFileName, ZipPlatform::GetDefaultAttributes());
		}
	}
	if (unlink(lpszFileName) != 0)
		if (bThrow)
			CZipException::Throw(CZipException::notRemoved, lpszFileName);
		else 
			return false;
	return true;
}

ZIPINLINE  bool ZipPlatform::RenameFile( LPCTSTR lpszOldName, LPCTSTR lpszNewName, bool bThrow)
{	
	if (rename(lpszOldName, lpszNewName) != 0)
	{
		if (bThrow)
			CZipException::Throw(CZipException::notRenamed, lpszOldName);
		return false;
	}
	return true;
}

ZIPINLINE  bool ZipPlatform::IsDirectory(DWORD uAttr)
{
	return S_ISDIR(uAttr) != 0;
}

ZIPINLINE  bool ZipPlatform::CreateNewDirectory(LPCTSTR lpDirectory)
{	
	return mkdir(lpDirectory, ZIP_DEFAULT_DIR_ATTRIBUTES) == 0;
}

ZIPINLINE  DWORD ZipPlatform::GetDefaultAttributes()
{
	return S_IFREG | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
}

ZIPINLINE  DWORD ZipPlatform::GetDefaultDirAttributes()
{
	return S_IFDIR | ZIP_DEFAULT_DIR_ATTRIBUTES;
}


ZIPINLINE  int ZipPlatform::GetSystemID()
{
	return ZipCompatibility::zcUnix;
}

ZIPINLINE bool ZipPlatform::GetSystemCaseSensitivity()
{
	return true;
}


bool ZipPlatform::TruncateFile(int iDes, ULONGLONG uSize)
{
#ifdef FILE_FUNCTIONS_64B_BY_DEFAULT
	return ftruncate(iDes, uSize) == 0;
#else
	return ftruncate64(iDes, uSize) == 0;
	
#endif

}

int ZipPlatform::OpenFile(LPCTSTR lpszFileName, UINT iMode, int iShareMode)
{
#ifdef FILE_FUNCTIONS_64B_BY_DEFAULT
	return open(lpszFileName, iMode, S_IRUSR | S_IWUSR | S_IRGRP |S_IROTH );	
#else
	return open64(lpszFileName, iMode, S_IRUSR | S_IWUSR | S_IRGRP |S_IROTH );	
#endif
}

bool ZipPlatform::FlushFile(int iDes)
{
	return fsync(iDes) == 0;
}

intptr_t ZipPlatform::GetFileSystemHandle(int iDes)
{
        return iDes;
}


#endif // _ZIP_SYSTEM_LINUX
