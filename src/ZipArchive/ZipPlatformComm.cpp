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
#include "ZipFile.h"
#include "ZipPlatform.h"
#include "ZipException.h"

using namespace ZipPlatform;

bool ZipPlatform::ForceDirectory(LPCTSTR lpDirectory)
{
	ASSERT(lpDirectory);
	CZipString szDirectory = lpDirectory;
	szDirectory.TrimRight(CZipPathComponent::m_cSeparator);
	CZipPathComponent zpc(szDirectory);
	if ((zpc.GetFilePath().Compare((LPCTSTR)szDirectory)) == 0 ||
		(FileExists(szDirectory) == -1))
		return true;
	if (!ForceDirectory(zpc.GetFilePath()))
		return false;
	if (!CreateNewDirectory(szDirectory))
		return false;
	return true;
}

bool ZipPlatform::GetFileSize(LPCTSTR lpszFileName, ZIP_SIZE_TYPE& dSize)
{
	CZipFile f;
	if (!f.Open(lpszFileName, CZipFile::modeRead | CZipFile::shareDenyWrite, false))
		return false;
	bool ret;
	try
	{
		ZIP_FILE_USIZE size = f.GetLength();
		// the file may be too large if zip64 is not enabled
		ret = size <= ZIP_SIZE_TYPE(-1);
		if (ret)
			dSize = (ZIP_SIZE_TYPE)size;
	}
#ifdef _ZIP_IMPL_MFC
	catch(CZipBaseException* e)
	{
		e->Delete();
		ret = false;
	}
#else
	catch(CZipBaseException e)
	{
		ret = false;
	}
#endif

	try
	{
		f.Close();
	}
#ifdef _ZIP_IMPL_MFC
	catch(CZipBaseException* e)
	{
		e->Delete();
	}
#else
	catch(CZipBaseException e)
	{
	}
#endif

	return ret;	
}


