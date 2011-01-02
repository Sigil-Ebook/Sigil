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
* \file ZipSplitNamesHandler.h
*	Includes the CZipSplitNamesHandler, CZipRegularSplitNamesHandler and CZipBinSplitNamesHandler class.
*
*/

#if !defined(ZIPARCHIVE_ZIPSPLITNAMESHANDLER_DOT_H)
#define ZIPARCHIVE_ZIPSPLITNAMESHANDLER_DOT_H

#if _MSC_VER > 1000
	#pragma once
	#pragma warning( push )
	#pragma warning (disable : 4100) // unreferenced formal parameter
#endif

#include "_features.h"
#include "ZipString.h"
#include "ZipPathComponent.h"
#include "BitFlag.h"


/**
	Generates and parses names for split archive volumes.
	Base class for split names handlers.
	\see
		<a href="kb">0610051553|splitNames</a>
*/
class ZIP_API CZipSplitNamesHandler
{
	
public:
	/**
		Flags for the #GetVolumeName method.
	*/
	enum Flags
	{
		flNone		= 0x00,	///< No special flags.
		flLast		= 0x01, ///< The volume is the last volume in the archive.
		flExisting	= 0x02	///< The archive is an existing archive.
	};

	/**
		Initializes a new instance of the CZipSplitNamesHandler class.
	*/
	CZipSplitNamesHandler()
	{		

	}

	/**
		Called when opening an archive. 

		\param szArchiveName
			The archive path provided when opening an archive.
	*/
	virtual void Initialize(const CZipString& szArchiveName)
	{

	}

	/**
		Returns the path for the given volume number.

		\param szArchiveName
			The archive path provided when opening an archive.

		\param uCurrentVolume
			The current volume number. The first volume number is \c 1.

		\param flags
			Additional flags. It can be one or more of the #Flags values.
	*/
	virtual CZipString GetVolumeName(const CZipString& szArchiveName, ZIP_VOLUME_TYPE uCurrentVolume, ZipArchiveLib::CBitFlag flags) const = 0;

	/**
		Returns the volume number for the given volume path.

		\param szVolumePath
			The volume path.

		\return
			The volume number parsed from the \a szVolumePath. The first volume number is \c 1. 
			Return \c 0 to indicate an error during parsing.

		\note
			Implementing of this method is only required for a handler for binary split archives.
	*/
	virtual ZIP_VOLUME_TYPE GetVolumeNumber(const CZipString& szVolumePath) const
	{
		// unspecified
		return 0;
	}

	virtual ~CZipSplitNamesHandler()
	{
	}
};


/**
	Generates names for regular split archives.

	\see
		<a href="kb">0610051553|splitNames</a>
*/
class ZIP_API CZipRegularSplitNamesHandler : public CZipSplitNamesHandler
{
protected:
	CZipString m_szExt;
public:
	CZipRegularSplitNamesHandler()
		:m_szExt(_T("zip"))
	{
	}

	void Initialize(const CZipString& szArchiveName)
	{
		CZipPathComponent zpc(szArchiveName);
		m_szExt = zpc.GetFileExt();
	}

	CZipString GetVolumeName(const CZipString& szArchiveName, ZIP_VOLUME_TYPE uCurrentVolume, ZipArchiveLib::CBitFlag flags) const
	{		 
		CZipString szExt;
		if (flags.IsSetAny(CZipSplitNamesHandler::flLast))
			szExt = m_szExt;
		else
		{
			if (uCurrentVolume < 100)
				szExt.Format(_T("z%.2u"), uCurrentVolume);
			else
				szExt.Format(_T("z%u"), uCurrentVolume);
		}
		CZipPathComponent zpc(szArchiveName);
		zpc.SetExtension(szExt);
		return zpc.GetFullPath();
	}	
};

/**
	Generates names for binary split archives.

	\see
		<a href="kb">0610051553|splitNames</a>
*/
class ZIP_API CZipBinSplitNamesHandler : public CZipSplitNamesHandler
{
public:
	CZipString GetVolumeName(const CZipString& szArchiveName, ZIP_VOLUME_TYPE uCurrentVolume, ZipArchiveLib::CBitFlag flags) const
	{
		CZipString szExt;
		if (uCurrentVolume < 1000)
			szExt.Format(_T("%.3u"), uCurrentVolume);
		else
			szExt.Format(_T("%u"), uCurrentVolume);
		if (flags.IsSetAny(CZipSplitNamesHandler::flExisting))
		{
			CZipPathComponent zpc(szArchiveName);
			zpc.SetExtension(szExt);
			return zpc.GetFullPath();
		}
		else
		{
			return szArchiveName + _T(".") + szExt;
		}
	}

	ZIP_VOLUME_TYPE GetVolumeNumber(const CZipString& szArchiveName) const
	{
		CZipPathComponent zpc(szArchiveName);
		CZipString szExt = zpc.GetFileExt();
		szExt.MakeLower();
		if (szExt.GetLength() < 3)
			return 0;
		__int64 ret;
#if !defined __GNUC__ || defined __MINGW32__
		ret = _ttoi64((LPCTSTR)szExt);		
#else
		errno = 0;
		ret = (__int64)strtoll((LPCTSTR)szExt, NULL, 10);
		if (errno != 0)
			return 0;		
#endif
		return (ZIP_VOLUME_TYPE)((ret <= 0 || ret > UINT_MAX) ? 0 : ret);
	}
};

#if _MSC_VER > 1000
	#pragma warning( pop )
#endif

#endif
