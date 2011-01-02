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
* \file FileInfo.h
*	Includes the ZipArchiveLib::CFileInfo class.
*
*/

#if !defined(ZIPARCHIVE_FILEINFO_DOT_H)
#define ZIPARCHIVE_FILEINFO_DOT_H

#if _MSC_VER > 1000
	#pragma once
#endif

#include "stdafx.h"
#include "ZipExport.h"
#include "ZipPlatform.h"

namespace ZipArchiveLib
{
	/**
		A structure holding a file or a directory information.
	*/
	struct ZIP_API CFileInfo
	{
	public:
		/**
			Initializes a new instance of the CFileInfo class.
		*/
		CFileInfo()
		{
			m_uSize = 0;
			m_uAttributes = 0;
			m_uCreateTime = m_uModTime = m_uAccessTime = 0;
		}
		ZIP_FILE_USIZE m_uSize;		///< The file size.
		DWORD m_uAttributes;		///< The file system attributes.
		time_t m_uCreateTime;		///< The Creation time.
		time_t m_uModTime;			///< The last modification time.
		time_t m_uAccessTime;		///< The last access time.

		/**
			Returns the value indicating whether the current CFileInfo
			object represents a directory or a regular file.

			\return 
				\c true, if the current CFileInfo object represents 
				a directory; \c false, if it represents a regular file.
		*/
		bool IsDirectory() const
		{
			return ZipPlatform::IsDirectory(m_uAttributes);
		}
	};
}
#endif
