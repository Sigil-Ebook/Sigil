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
#include "ZipCompatibility.h"
#include "ZipPlatform.h"
#include "ZipException.h"
#include "ZipAutoBuffer.h"
#include "ZipFileHeader.h"
#include "ZipArchive.h"


// *********************** WINDOWS **************************
#ifndef _WIN32
	#define FILE_ATTRIBUTE_READONLY             0x00000001  
	#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
	#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
	#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
	#define FILE_ATTRIBUTE_ARCHIVE              0x00000020 
#endif
// *********************** UINX **************************

#define USER_PERMISSIONS_MASK  0x01C0
#define EXTRACT_USER_PERMISSIONS(x) ((x & USER_PERMISSIONS_MASK) >> 6)
#define CREATE_USER_PERMISSIONS(x) ((x & 0x0007) << 6)

#define GROUP_PERMISSIONS_MASK 0x0038
#define EXTRACT_GROUP_PERMISSIONS(x) ((x & GROUP_PERMISSIONS_MASK) >> 3)
#define CREATE_GROUP_PERMISSIONS(x) ((x & 0x0007) << 3)


#define OTHER_PERMISSIONS_MASK  0x0007
#define EXTRACT_OTHER_PERMISSIONS(x) ((x & OTHER_PERMISSIONS_MASK))
#define CREATE_OTHER_PERMISSIONS(x) (x & 0x0007)

#define UNIX_DIRECTORY_ATTRIBUTE 0x4000
#define UNIX_FILE_ATTRIBUTE 0x8000

#define UNIX_EXEC 1
#define UNIX_WRITE 2
#define UNIX_READ 4

using namespace ZipCompatibility;

typedef DWORD(*conv_func)(DWORD , bool );

DWORD AttrDos(DWORD , bool );
DWORD AttrUnix(DWORD, bool);
DWORD AttrMac(DWORD , bool );

conv_func conv_funcs[21] = {AttrDos,
							NULL,
							NULL,
							AttrUnix,
							NULL,
							NULL,
							AttrDos,
							AttrMac,
							NULL,
							NULL,
							NULL,
							AttrDos,
							NULL,
							NULL,
							NULL,
							AttrDos,
							NULL,
							NULL,
							NULL,
							NULL,
							AttrMac,
};



DWORD ZipCompatibility::ConvertToSystem(DWORD uAttr, int iFromSystem, int iToSystem)
{
	if (iToSystem != iFromSystem && iFromSystem < zcLast && iToSystem < zcLast)
	{
		conv_func p = conv_funcs[iFromSystem], q = conv_funcs[iToSystem];
		if (p && q)
			uAttr = q( p(uAttr, true), false);
	 	else
	 		CZipException::Throw(CZipException::platfNotSupp);
	}
	return uAttr; 
}

DWORD ZipCompatibility::GetAsInternalAttributes(DWORD uAttr, int iFromSystem)
{
	if (iFromSystem < zcLast)
	{
		conv_func f = conv_funcs[iFromSystem];
		if (!f)
			CZipException::Throw(CZipException::platfNotSupp);
		return f(uAttr, true);
	}
	return uAttr;
}


DWORD AttrDos(DWORD uAttr, bool )
{
	return uAttr;	
}

DWORD AttrUnix(DWORD uAttr, bool bFrom)
{
	DWORD uNewAttr = 0;
	if (bFrom)
	{
		bool isDir = (uAttr & UNIX_DIRECTORY_ATTRIBUTE) != 0;
		if (isDir)
			uNewAttr = attDir;

		DWORD uGroupAttr = EXTRACT_GROUP_PERMISSIONS(uAttr);
		DWORD uOtherAttr = EXTRACT_OTHER_PERMISSIONS(uAttr);
		uAttr = EXTRACT_USER_PERMISSIONS (uAttr);
		

		// we may set archive attribute if the file hasn't got the execute permissions
		// and is not a directory
		if (!isDir && !(uAttr & UNIX_EXEC))
			uNewAttr |= attArch	;

		if (!(uAttr & UNIX_WRITE)) 
		    uNewAttr |= attROnly;

	    if (!(uGroupAttr & UNIX_READ) && !(uOtherAttr & UNIX_READ)) 
		    uNewAttr |= attHidd;
	}
	else
	{

		uNewAttr = CREATE_USER_PERMISSIONS (UNIX_READ);

		// we cannot assume that if the file hasn't the archive attribute set		
		// then it is executable and set execute permissions

		if (!(uAttr & attHidd)) 
			uNewAttr |= (CREATE_OTHER_PERMISSIONS (UNIX_READ) | CREATE_GROUP_PERMISSIONS (UNIX_READ));
							

		if (!(uAttr & attROnly))
			uNewAttr |= (CREATE_GROUP_PERMISSIONS (UNIX_WRITE) | CREATE_USER_PERMISSIONS (UNIX_WRITE));

		if (uAttr & attDir) 
		{
			uNewAttr |= UNIX_DIRECTORY_ATTRIBUTE;
			uNewAttr |= (CREATE_OTHER_PERMISSIONS (UNIX_EXEC) | CREATE_GROUP_PERMISSIONS (UNIX_EXEC)) |
				CREATE_USER_PERMISSIONS (UNIX_EXEC);
		}
		else
			uNewAttr |= UNIX_FILE_ATTRIBUTE;

	}

	return uNewAttr;	
}

DWORD AttrMac(DWORD uAttr, bool )
{
	return uAttr & (attDir | attROnly);
}

// ************************************************************************
ZIPINLINE bool ZipCompatibility::IsPlatformSupported(int iCode)
{
	return iCode == zcDosFat || iCode == zcUnix || iCode == zcMacintosh
		|| iCode == zcNtfs || iCode == zcOs2Hpfs || iCode == zcVfat || iCode == zcMacDarwin;
}

void ZipCompatibility::ConvertBufferToString(CZipString& szString, const CZipAutoBuffer& buffer, UINT uCodePage)
{
#ifdef _UNICODE	
	ZipPlatform::MultiByteToWide(buffer, szString, uCodePage);
#else
	// 	iLen does not include the NULL character
	int iLen;
	if (uCodePage == CP_OEMCP)
	{
		CZipAutoBuffer buf;
		buf = buffer;
		ZipPlatform::AnsiOem(buf, false);		
		iLen = buf.GetSize();
		memcpy(szString.GetBuffer(iLen), buf.GetBuffer(), iLen);
	}
	else
	{
		iLen = buffer.GetSize();		
		memcpy(szString.GetBuffer(iLen), buffer.GetBuffer(), iLen);
	}
	szString.ReleaseBuffer(iLen);
#endif
}

void ZipCompatibility::ConvertStringToBuffer(LPCTSTR lpszString, CZipAutoBuffer& buffer, UINT uCodePage)
{
#ifdef _UNICODE
	ZipPlatform::WideToMultiByte(lpszString, buffer, uCodePage);
#else
	int iLen = (int)strlen(lpszString);
	// 	iLen does not include the NULL character
	buffer.Allocate(iLen);
	memcpy(buffer, lpszString, (size_t)iLen);
	if (uCodePage == CP_OEMCP)
		ZipPlatform::AnsiOem(buffer, true);
#endif
}

void ZipCompatibility::SlashBackslashChg(CZipString& szFileName, bool bReplaceSlash)
{
	TCHAR t1 = _T('\\') /*backslash*/, t2 = _T('/'), c1, c2;
	if (bReplaceSlash)
	{
		c1 = t1;
		c2 = t2;
	}
	else
	{
		c1 = t2;
		c2 = t1;
	}
	szFileName.Replace(c2, c1);
}

void ZipCompatibility::NormalizePathSeparators(CZipString& szFileName)
{
	int iPlatform = ZipPlatform::GetSystemID();
	ZipCompatibility::SlashBackslashChg(szFileName, iPlatform == ZipCompatibility::zcDosFat || iPlatform == ZipCompatibility::zcNtfs);
}

UINT ZipCompatibility::GetDefaultNameCodePage(int iPlatform)
{
	if (iPlatform == ZipCompatibility::zcDosFat || iPlatform == ZipCompatibility::zcNtfs)
		return CP_OEMCP;
	else if (iPlatform == ZipCompatibility::zcUnix || iPlatform == ZipCompatibility::zcMacintosh || iPlatform == zcMacDarwin)
		return CP_UTF8;
	else
		return CP_ACP;
}

UINT ZipCompatibility::GetDefaultNameCodePage()
{
	return GetDefaultNameCodePage(ZipPlatform::GetSystemID());
}

UINT ZipCompatibility::GetDefaultCommentCodePage(int iPlatform)
{
	if (iPlatform == ZipCompatibility::zcUnix || iPlatform == ZipCompatibility::zcMacintosh || iPlatform == zcMacDarwin)
		return CP_UTF8;
	else
		return CP_ACP;
}

UINT ZipCompatibility::GetDefaultPasswordCodePage(int iPlatform)
{
	if (iPlatform == ZipCompatibility::zcUnix || iPlatform == ZipCompatibility::zcMacintosh || iPlatform == zcMacDarwin)
		return CP_UTF8;
	else
		return CP_ACP;
}

UINT ZipCompatibility::GetDefaultCommentCodePage()
{
	return GetDefaultCommentCodePage(ZipPlatform::GetSystemID());
}
