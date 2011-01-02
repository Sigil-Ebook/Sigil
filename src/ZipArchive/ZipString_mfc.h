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

#ifndef ZIPARCHIVE_ZIPSTRING_DOT_H
	#error Do not include this file directly. Include ZipString.h instead
#endif


#include "stdafx.h"
#include "ZipExport.h"

typedef CString CZipString;

/**
	A pointer type to point to one of: Collate, CollateNoCase, Compare, CompareNoCase.
*/
typedef int (CZipString::*ZIPSTRINGCOMPARE)( LPCTSTR ) const;

/**
	Returns a pointer to a method in the CZipString structure, 
	used to compare elements depending on the arguments.
*/
ZIP_API ZIPSTRINGCOMPARE GetCZipStrCompFunc(bool bCaseSensitive, bool bCollate = true);
