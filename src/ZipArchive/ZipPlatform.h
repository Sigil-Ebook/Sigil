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
* \file ZipPlatform.h
* ZipPlatform namespace declaration.
*
*/
#if !defined(ZIPARCHIVE_ZIPPLATFORM_DOT_H)
#define ZIPARCHIVE_ZIPPLATFORM_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

class CZipFileHeader;
class CZipAutoBuffer;

#include "ZipString.h"
#include "ZipPathComponent.h"
#include <sys/types.h>
#include "ZipExport.h"

/**
	Includes functions that require system-specific implementation.
*/
namespace ZipPlatform
{
	/**
		The mode for deleting files.
	*/
	enum DeleteFileMode
	{
		dfmRegular = 0x00,				///< No special action is taken when overwriting a file.
		dfmRemoveReadOnly = 0x01		///< The read-only attribute is cleared before overwriting a file.
#if defined _ZIP_SYSTEM_WIN && defined SHFileOperation
		,dfmRecycleBin = 0x02			///< The overwritten file is moved to the Recycle Bin (Windows only).
#endif
	};
	/**
		Returns the default case-sensitivity for the current file system.

		\return
			\c true, if the system is case-sensitive; \c false otherwise.
	*/
	ZIP_API bool GetSystemCaseSensitivity();

	/**
		Returns the current system identifier.

		\return
			One of the ZipCompatibility::ZipPlatforms values.

		\see
			CZipArchive::SetSystemCompatibility
	*/
	ZIP_API int GetSystemID();

	/**
		Returns the default file attributes for the current system.

		\return
			The default file attributes.
	*/
	ZIP_API DWORD GetDefaultAttributes(); 

	/**
		Returns the default directory attributes for the current system.

		\return
			The default directory attributes.
	*/
	ZIP_API DWORD GetDefaultDirAttributes();

	/**
		Returns the free space on the given device.

		\param lpszPath
			Points to the device to test.

		\return
			The free space in bytes.
	*/
	ZIP_API ULONGLONG GetDeviceFreeSpace(LPCTSTR lpszPath);

	/**
		Returns the name of a temporary file ensuring there is enough free space in the destination directory.

		\param lpszPath
			The path to the directory to initially create the file in.

		\param uSizeNeeded
			The requested free space size in bytes. If set to <code>0</code>, the 
			space availability is not checked.
	*/
	ZIP_API CZipString GetTmpFileName(LPCTSTR lpszPath = NULL, ZIP_SIZE_TYPE uSizeNeeded = 0);

	/**
		\name Various operations on files and directories.
		If the functions returns a \c bool value, then \c true indicates that the operation was successful.
	*/
	//@{
	ZIP_API bool GetCurrentDirectory(CZipString& sz);	///< Returns the current directory and stores it in \a sz.
	ZIP_API bool ChangeDirectory(LPCTSTR lpDirectory);	///< Changes the current directory.
	ZIP_API bool SetFileAttr(LPCTSTR lpFileName, DWORD uAttr);	///< Sets the file attributes.
	ZIP_API bool GetFileAttr(LPCTSTR lpFileName, DWORD& uAttr); ///< Returns the file attributes.
	ZIP_API bool GetFileModTime(LPCTSTR lpFileName, time_t & ttime); ///< Returns the file modification time.
	ZIP_API bool SetFileModTime(LPCTSTR lpFileName, time_t ttime);	 ///< Set the file modification time.
	ZIP_API bool GetFileSize(LPCTSTR lpszFileName, ZIP_SIZE_TYPE& dSize); ///< Returns the file size.
	ZIP_API bool CreateNewDirectory(LPCTSTR lpDirectory);	///< Creates a new directory.
	ZIP_API bool SetVolLabel(LPCTSTR lpszPath, LPCTSTR lpszLabel); ///< Sets a label on a removable device. \c lpszPath may point to a file on the device.
	ZIP_API bool ForceDirectory(LPCTSTR lpDirectory);	///< Creates nested directories at once.
	ZIP_API bool RemoveFile(LPCTSTR lpszFileName, bool bThrow = true, int iMode = dfmRegular); ///< Removes a file.
	ZIP_API bool RenameFile( LPCTSTR lpszOldName, LPCTSTR lpszNewName, bool bThrow = true); ///< Renames a file.

#ifdef _ZIP_SYSTEM_LINUX
	ZIP_API bool SetExeAttr( LPCTSTR lpFileName ); ///< Sets executable permissions for a file.
#endif

#if defined _ZIP_IMPL_STL || _ZIP_FILE_IMPLEMENTATION == ZIP_ZFI_STL
	/**
		Truncates the file.

		\note
			Defined only in the STL version.
	*/
	ZIP_API bool TruncateFile(int iDes, ULONGLONG uSize);

	/**
		Opens the file.

		\note
			Defined only in the STL version.
	*/
	ZIP_API int OpenFile(LPCTSTR lpszFileName, UINT iMode, int iShareMode);

	/**
		Flushes the file to the disk.

		\note
			Defined only in the STL version.
	*/
	ZIP_API bool FlushFile(int iDes);

	/**
		Returns the underlying system handle.

		\note
			Defined only in the STL version.
	*/
	ZIP_API intptr_t GetFileSystemHandle(int iDes);
#endif
	//@}


	/**
		Checks if the given drive is removable.

		\param	lpszFilePath
			The path to the drive. May point to a file path or a directory on the drive.

		\return
			\c true. if the drive is removable; \c false otherwise.

		\note
			Implemented only on Windows system, on all others always returns \c true.
	*/
	ZIP_API bool IsDriveRemovable(LPCTSTR lpszFilePath);

	/**
		Checks if the given attributes represent a directory.

		\param	uAttr
			The attributes to test.

		\return
			\c true if the attributes represent a directory; \c false otherwise.
	*/
	ZIP_API bool IsDirectory(DWORD uAttr);

	/**
		Performs the translation between ANSI and OEM character sets.

		\param	buffer
			The buffer containing characters to be translated.

		\param	bAnsiToOem
			If \c true, convert ANSI to OEM; if \c false, OEM to ANSI.
	*/
	ZIP_API void AnsiOem(CZipAutoBuffer& buffer, bool bAnsiToOem);

	/**
		Checks if the given file or directory exists.

		\param	lpszName
			The path to the file or directory to test.

		\return	
			One of the following values:
			- \c -1 : the given file exists and is a directory
			- \c 1 : the given file exists and is a regular file
			- \c 0 : there is no such a file
	*/
	ZIP_API int FileExists(LPCTSTR lpszName);

#ifdef _UNICODE	
	/**
		Converts a wide character string to a multi-byte character string.

		\param	lpszIn
			The wide character string to convert.

		\param	szOut
			The buffer to receive the converted string.
			Does not contain the terminating \c NULL character.

		\param uCodePage
			The code page used in conversion.

		\return	
			The \a szOut buffer length, or \c -1 when not succeeded.

		\note 
			Defined only in the UNICODE version.
	*/
	ZIP_API int WideToMultiByte(LPCWSTR lpszIn, CZipAutoBuffer &szOut, UINT uCodePage);

	/**
		Converts a multi-byte character string to a wide character string.

		\param	szIn
			The multi-byte character string to convert.
			Should not contain the terminating \c NULL character.

		\param	szOut
			Receives the converted string.

		\param uCodePage
			The code page used in conversion.

		\return
			The length of the string after the conversion (without the terminating \c NULL character)
			or \c -1 when the function did not succeed.

		\note 
			Defined only in the UNICODE version.
	*/	
	ZIP_API int MultiByteToWide(const CZipAutoBuffer &szIn, CZipString& szOut, UINT uCodePage);
#endif
};


#endif // !defined(ZIPARCHIVE_ZIPPLATFORM_DOT_H)

