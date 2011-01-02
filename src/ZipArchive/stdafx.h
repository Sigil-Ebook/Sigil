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
* \file stdafx.h
*	Contains global definitions.
*
*/

#if !defined(ZIPARCHIVE_STDAFX_DOT_H)
#define ZIPARCHIVE_STDAFX_DOT_H

#include "_features.h"

#if _MSC_VER > 1000
	#pragma once
#endif

// uncomment to disable compiling standard error messages into the library
#define _ZIP_ENABLE_ERROR_DESCRIPTION

#if _MSC_VER < 1300 && !defined __BORLANDC__ && !defined (__GNUC__)
	#define ZIPINLINE inline
#else
	#define ZIPINLINE
#endif

#ifdef _ZIP_IMPL_STL
	#include "std_stl.h"
#else
	#include "std_mfc.h"
#endif

#ifdef TRACE
	#if _MSC_VER >= 1300
		#define ZIPTRACE(f) TRACE(f,__FILE__,__LINE__)
	#else
		#define ZIPTRACE(f) TRACE(_T(f))
	#endif
#else
	#define ZIPTRACE(f)
	#define NOZIPTRACE
#endif

	#ifdef _ZIP_STRICT_U16
		#define ZIP_INDEX_TYPE WORD
		#define ZIP_VOLUME_TYPE WORD		
	#else
		#define ZIP_INDEX_TYPE int
		#define ZIP_VOLUME_TYPE int
	#endif	

	#define ZIP_SIZE_TYPE DWORD
	#define ZIP_ZLIB_TYPE int

#if !defined(_INTPTR_T_DEFINED) && !defined(__GNUC__)	
	#if __BORLANDC__ > 0x551
		#include <stdint.h>
	#elif _MSC_VER <= 1200 || !defined _MSC_VER
		typedef long intptr_t;
	#endif
#endif

#ifndef UNICODE
	#ifdef _ZIP_UNICODE_NORMALIZE
		#undef _ZIP_UNICODE_NORMALIZE
	#endif
	#ifdef _ZIP_UNICODE_CUSTOM
		#undef _ZIP_UNICODE_CUSTOM
	#endif
	#if defined _ZIP_UNICODE && defined _ZIP_SYSTEM_WIN
		#undef _ZIP_UNICODE
	#endif
#endif

#define ZIP_FILE_INDEX_NOT_FOUND ZIP_INDEX_TYPE(-1)
#define ZIP_FILE_INDEX_UNSPECIFIED ZIP_FILE_INDEX_NOT_FOUND
#define ZIP_VOLUME_NUMBER_UNSPECIFIED ZIP_VOLUME_TYPE(-1)

#ifdef __MINGW32__
	#define SUFFIX_I64(v) v##LL
#else
	#define SUFFIX_I64(v) v##i64
#endif

#ifdef _ZIP_SYSTEM_WIN
	#define ZIP_SYSTEM_ERROR_TYPE DWORD
#else
	#define ZIP_SYSTEM_ERROR_TYPE int
#endif

#endif // !defined(ZIPARCHIVE_STDAFX_DOT_H)
 
