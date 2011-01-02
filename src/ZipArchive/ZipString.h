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
* \file ZipString.h
*	Includes the CZipString class.
*
*/


#ifndef ZIPARCHIVE_ZIPSTRING_DOT_H
#define ZIPARCHIVE_ZIPSTRING_DOT_H

#if _MSC_VER > 1000
	#pragma once
#endif

#include "_platform.h"

#ifdef _ZIP_IMPL_STL
	#include "ZipString_stl.h"
#else
	#include "ZipString_mfc.h"
#endif

namespace ZipArchiveLib
{
	ZIP_API bool IsStringAscii(const CZipString& value);	
};

#endif  /* ZIPARCHIVE_ZIPSTRING_DOT_H */
