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
* \file ZipCompatibility.h
* ZipCompatibility namespace declaration.
*
*/

#if !defined(ZIPARCHIVE_ZIPCOMPATIBILITY_DOT_H)
#define ZIPARCHIVE_ZIPCOMPATIBILITY_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

class CZipAutoBuffer;
class CZipFileHeader;

#include "ZipString.h"
#include "ZipPlatform.h"
/**
	Includes functions that provide support for the proper conversion of attributes 
	and filenames between different system platforms.
*/
namespace ZipCompatibility  
{
	/**
		The codes of the compatibility of the file attribute information.

		\see
			CZipArchive::GetSystemCompatibility
		\see
			CZipFileHeader::GetSystemCompatibility
		\see
			ZipPlatform::GetSystemID
	*/
	enum ZipPlatforms
	{		   
	   zcDosFat,		///< MS-DOS and OS/2 (FAT / VFAT / FAT32 file systems)
       zcAmiga,			///< Amiga 
       zcVaxVms,		///< VAX/VMS
       zcUnix,			///< Unix / Linux
       zcVmCms,			///< VM/CMS
       zcAtari,			///< Atari ST
       zcOs2Hpfs,		///< OS/2 H.P.F.S.
       zcMacintosh,		///< Macintosh 
       zcZsystem,		///< Z-System
       zcCpm,			///< CP/M 
	   zcTops20,		///< TOPS-20
       zcNtfs,			///< Windows NTFS
	   zcQDos,			///< SMS/QDOS
	   zcAcorn,			///< Acorn RISC OS
	   ZcMvs,			///< MVS
	   zcVfat,			///< Win32 VFAT
	   zcAtheOS,		///< AtheOS
	   zcBeOS,			///< BeOS
	   zcTandem,		///< Tandem NSK
	   zcTheos,			///< Theos
	   zcMacDarwin,		///< Mac OS/X (Darwin)
	   zcLast			///< For the internal use
	};

	/**
		Platform independent attributes.
	*/
	enum InternalFileAttributes
	{
		attROnly	= 0x01,	///< Read-only attribute.
		attHidd		= 0x02,	///< Hidden attribute.
		attSys		= 0x04,	///< System attribute.
		attDir		= 0x10,	///< Directory attribute.
		attArch		= 0x20	///< Archived attribute.
	};

	ZIP_API DWORD GetAsInternalAttributes(DWORD uAttr, int iFromSystem);

	/**
		Checks whether the system with the given code is supported by the ZipArchive Library.

		\param iCode
			One of the #ZipPlatforms values to check.

		\return
			\c true, if supported; \c false otherwise.
	*/
	ZIP_API bool IsPlatformSupported(int iCode);

	/**
		Converts the system attributes between different system platforms.

		\param uAttr
			The attributes to convert.

		\param iFromSystem
			The system code to convert \a uAttr from.

		\param iToSystem
			The system code to convert \a uAttr to.

		\return
			The converted attributes.

		
		\see
			ZipPlatforms
	*/
	ZIP_API DWORD ConvertToSystem(DWORD uAttr, int iFromSystem, int iToSystem);

	/**
		Converts the string stored in \a buffer using the given code page.

		\param buffer
			The buffer to convert the string from.

		\param szString
			The string to receive the result.

		\param uCodePage
			The code page used in conversion.
	*/
	ZIP_API void ConvertBufferToString(CZipString& szString, const CZipAutoBuffer& buffer, UINT uCodePage);
	
	/**
		Converts the \a lpszString using the given code page.

		\param lpszString
			The string to convert from.

		\param buffer
			The buffer to receive the result.

		\param uCodePage
			The code page used in conversion.
	*/
	ZIP_API void ConvertStringToBuffer(LPCTSTR lpszString, CZipAutoBuffer& buffer, UINT uCodePage);

	/**
		Changes the path separators from slash to backslash or vice-versa in \a szFileName.

		\param szFileName
			The filename to have the path separators changed.

		\param	bReplaceSlash
			If \c true, changes slash to backslash. If \c false, changes backslash to slash.
	*/
	ZIP_API void SlashBackslashChg(CZipString& szFileName, bool bReplaceSlash);

	/**
		Normalizes path separators to the default character used by the current platform.

		\param szFileName
			The filename to have the path separators normalized.
	*/
	ZIP_API void NormalizePathSeparators(CZipString& szFileName);

	/**
		Returns the default filename code page for the given platform.

		\param iPlatform
			One of the ZipCompatibility::ZipPlatforms values.	

		\return 
			The default filename code page.
	*/
	ZIP_API UINT GetDefaultNameCodePage(int iPlatform);

	
	/**
		Returns the default filename code page for the current platform.

		\return 
			The default filename code page.
	*/
	ZIP_API UINT GetDefaultNameCodePage();

	/**
		Returns the default comment code page.

		\param iPlatform
			One of the ZipCompatibility::ZipPlatforms values.

		\return 
			The default comment code page.
	*/
	ZIP_API UINT GetDefaultCommentCodePage(int iPlatform);

	/**
		Returns the default password code page.

		\param iPlatform
			One of the ZipCompatibility::ZipPlatforms values.

		\return 
			The default password code page.
	*/
	ZIP_API UINT GetDefaultPasswordCodePage(int iPlatform);

	/**
		Returns the default comment code page for the current platform.

		\return 
			The default comment code page.
	*/
	ZIP_API UINT GetDefaultCommentCodePage();
};

#endif // !defined(ZIPARCHIVE_ZIPCOMPATIBILITY_DOT_H)
