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
#include "ZipException.h"
#include "zlib/zlib.h"
#include <errno.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#if defined _MFC_VER && defined _ZIP_IMPL_MFC
	IMPLEMENT_DYNAMIC( CZipException, CException)
#endif

CZipException::CZipException(int iCause, LPCTSTR lpszZipName)
#ifdef _MFC_VER
	:CException(TRUE)
#endif
{
	m_iCause = iCause;

	if (lpszZipName)
		m_szFileName = lpszZipName;

#ifdef _ZIP_SYSTEM_WIN
	m_iSystemError = ::GetLastError();
#else
	m_iSystemError = errno;
#endif
}

CZipException::~CZipException() throw()
{

}

// inline void CZipException::Throw(int iZipError, LPCTSTR lpszZipName)
// {
// #ifdef _MFC_VER
// 	throw new CZipException(iZipError, lpszZipName);
// #else
// 	CZipException e(iZipError, lpszZipName);
// 	throw e;
// #endif
// MSVC++: ignore "Unreachable code" warning here, it's due to 
// optimizations
// }


#ifdef _ZIP_ENABLE_ERROR_DESCRIPTION

ZBOOL CZipException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
	UINT* )

{
	if (!lpszError || !nMaxError)
		return FALSE;
	CZipString sz = GetErrorDescription();
	if (sz.IsEmpty())
		return FALSE;
	UINT iLen = sz.GetLength();
	if (nMaxError - 1 < iLen)
		iLen = nMaxError - 1;
	LPTSTR lpsz = sz.GetBuffer(iLen);
#if _MSC_VER >= 1400
	#ifdef _UNICODE	
		wcsncpy_s(lpszError, nMaxError, lpsz, iLen);
	#else
		strncpy_s(lpszError, nMaxError, lpsz, iLen);
	#endif
#else
	#ifdef _UNICODE	
		wcsncpy(lpszError, lpsz, iLen);
	#else
		strncpy(lpszError, lpsz, iLen);
	#endif
#endif

	lpszError[iLen] = _T('\0');
	return TRUE;
}


CZipString CZipException::GetErrorDescription()
{
	return GetInternalErrorDescription(m_iCause);
}


CZipString CZipException::GetSystemErrorDescription()
{
#ifdef WIN32
	DWORD x = GetLastError();
	if (x)
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,    
			          NULL, x, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
				      (LPTSTR) &lpMsgBuf, 0, NULL);
		CZipString sz = (LPCTSTR)lpMsgBuf;
		LocalFree(lpMsgBuf);
		return sz;
	}
#endif
	return GetInternalErrorDescription(errno == 0 ? genericError : errno, true);
}

CZipString CZipException::GetInternalErrorDescription(int iCause, bool bNoLoop)
{
	CZipString sz;
	switch (iCause)
	{
		case EROFS:
			sz = _T("Read-only file system.");
			break;
		case ESPIPE:
			sz = _T("Illegal seek.");
			break;
		case ENOSPC:
			sz = _T("No space left on device.");
			break;
		case EFBIG:
			sz = _T("File too large.");
			break;
		case EMFILE:
			sz = _T("Too many open files.");
			break;
		case ENFILE:
			sz = _T("File table overflow.");
			break;
		case EINVAL:
			sz = _T("Invalid argument.");
			break;
		case EISDIR:
			sz = _T("Is a directory.");
			break;
		case ENOTDIR:
			sz = _T("Not a directory.");
			break;
		case ENODEV:
			sz = _T("No such device.");
			break;
		case EXDEV:
			sz = _T("Cross-device link.");
			break;
		case EEXIST:
			sz = _T("File exists.");
			break;
		case EFAULT:
			sz = _T("Bad address.");
			break;
		case EACCES:
			sz = _T("Permission denied.");
			break;
		case ENOMEM:
			sz = _T("Not enough space.");
			break;
		case EBADF:
			sz = _T("Bad file number.");
			break;
		case ENXIO:
			sz = _T("No such device or address.");
			break;
		case EIO:
			sz = _T("I/O error.");
			break;
		case EINTR:
			sz = _T("Interrupted system call.");
			break;
		case ENOENT:
			sz = _T("No such file or directory.");
			break;
		case EPERM:
			sz = _T("Not super-user.");
			break;
		case badZipFile:
			sz = _T("Damaged or not a zip file.");
			break;
		case badCrc:
			sz = _T("Crc is mismatched.");
			break;
		case noCallback:
			sz = _T("There is no spanned archive callback object set.");
			break;
		case noVolumeSize:
			sz = _T("The volume size was not defined for a split archive.");
			break;
		case aborted:
			sz = _T("Volume change aborted in a segmented archive.");
			break;
		case abortedAction:
			sz = _T("Action aborted.");
			break;
		case abortedSafely:
			sz = _T("Action aborted safely.");
			break;
		case nonRemovable:
			sz = _T("The device selected for the spanned archive is not removable.");
			break;
		case tooManyVolumes:
			sz = _T("The limit of the maximum number of volumes has been reached.");
			break;
		case tooManyFiles:
			sz = _T("The limit of the maximum number of files in an archive has been reached.");
			break;
		case tooLongData:
			sz = _T("The filename, the comment or local or central extra field of the file added to the archive is too long.");
			break;
		case tooBigSize:
			sz = _T("The file size is too large to be supported.");
			break;
		case badPassword:
			sz = _T("An incorrect password set for the file being decrypted.");
			break;
		case dirWithSize:
			sz = _T("The directory with a non-zero size found while testing.");
			break;
		case internalError:
			sz = _T("An internal error.");
			break;
		case fileError:
			sz.Format(_T("%s (%s)."), _T("A file error occurred"), (LPCTSTR)GetSystemErrorDescription());
			break;
		case notRemoved:
			sz.Format(_T("%s (%s)."), _T("Error while removing a file"), (LPCTSTR)GetSystemErrorDescription());
			break;
		case notRenamed:
			sz.Format(_T("%s (%s)."), _T("Error while renaming a file"), (LPCTSTR)GetSystemErrorDescription());
			break;
		case platfNotSupp:
			sz = _T("Cannot create a file for the specified platform.");
			break;
		case cdirNotFound:
			sz = _T("The central directory was not found in the archive (or you were trying to open not the last volume of a segmented archive).");
			break;
		case noZip64:
			sz = _T("The Zip64 format has not been enabled for the library, but is required to use the archive.");
			break;
		case noAES:
			sz = _T("WinZip AES encryption has not been enabled for the library, but is required to decompress the archive.");
			break;
#ifdef _ZIP_IMPL_STL
			case outOfBounds:
			sz = _T("The collection is empty and the bounds do not exist.");
			break;
#endif
#ifdef _ZIP_USE_LOCKING
		case mutexError:
			sz = _T("Locking or unlocking resources access was unsuccessful.");
			break;
#endif
		case streamEnd:
			sz = _T("Zlib library error (end of stream).");
			break;
		case errNo:
			sz = GetInternalErrorDescription(errno != errNo ? errno : genericError);
			break;
		case streamError:
			sz = _T("Zlib library error (stream error).");
			break;
		case dataError:
			sz = _T("Zlib library error (data error).");
			break;
		case memError:
			sz = _T("Not enough memory.");
			break;
		case bufError:
			sz = _T("Zlib library error (buffer error).");
			break;
		case versionError:
			sz = _T("Zlib library error (version error).");
			break;
		default:
			sz = bNoLoop ? _T("Unspecified error") :(LPCTSTR) GetSystemErrorDescription();
	}
	return sz;
}

#endif //_ZIP_ENABLE_ERROR_DESCRIPTION
