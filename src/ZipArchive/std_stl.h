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

#ifndef ZIPARCHIVE_STDAFX_DOT_H
	#error Do not include this file directly. Include stdafx.h instead
#endif

#include "_features.h"

#if defined DEBUG && !defined _DEBUG
	#define _DEBUG
#endif

#if _MSC_VER > 1000
	// STL warnings
	#pragma warning (disable : 4710) // 'function' : function not inlined
	#pragma warning (disable : 4514) // unreferenced inline/local function has been removed
	#pragma warning (disable : 4786) // 'identifier' : identifier was truncated to 'number' characters in the debug information
	#pragma warning (disable : 4702) // unreachable code
#endif

#if defined (_UNICODE) && !defined (UNICODE)
	#define UNICODE
#endif
#if defined (UNICODE) && !defined (_UNICODE)
	#define _UNICODE
#endif

#ifndef _WIN32
	#ifndef NULL
		#define NULL    0
	#endif

	#include <cctype>
	#include <climits>
	typedef int HFILE;
	typedef void*				HANDLE;
	typedef unsigned int        DWORD;
	typedef long				LONG;
	typedef int                 ZBOOL; /* to avoid conflicts when using Objective-C under Mac */
	typedef unsigned char       BYTE;
	typedef unsigned short      WORD;
	typedef unsigned int        UINT;

	#ifndef FALSE
		#define FALSE               (int)0
	#endif

	#ifndef TRUE
		#define TRUE                (int)1
	#endif


	typedef unsigned short WCHAR;    // wc,   16-bit UNICODE character
	typedef const WCHAR *LPCWSTR;
	typedef const char *LPCSTR;
	typedef WCHAR *LPWSTR;
	typedef char *LPSTR;

	#ifdef  _UNICODE
		typedef wchar_t TCHAR;
		typedef LPCWSTR LPCTSTR;
		typedef LPWSTR LPTSTR;
		#define _T(x)      L ## x
	#else   /* _UNICODE */               // r_winnt
		typedef char TCHAR;
		typedef LPCSTR LPCTSTR;
		typedef LPSTR LPTSTR;
		#ifdef _T
			#undef _T
		#endif
		#define _T(x)      x
	#endif /* _UNICODE */                // r_winnt

	typedef unsigned long long ULONGLONG;
	typedef long long LONGLONG;
	#define CP_ACP 0
	#define CP_OEMCP 1

	#ifndef CP_UTF8
		#define CP_UTF8	65001
	#endif

	
	#ifndef _I64_MAX
		#ifdef LLONG_MAX	
			#define _I64_MAX LLONG_MAX
		#elif defined LONG_LONG_MAX
			#define _I64_MAX LONG_LONG_MAX
		#else
			#define _I64_MAX LONGLONG_MAX
		#endif
	#endif
	#ifndef _UI64_MAX
		#ifdef ULLONG_MAX	
			#define _UI64_MAX ULLONG_MAX
		#elif defined ULONG_LONG_MAX
			#define _UI64_MAX ULONG_LONG_MAX
		#else 
			#define _UI64_MAX ULONGLONG_MAX
		#endif
	#endif
	#define _lseeki64 lseek64
#else
	#include <TCHAR.H>
   	#include <windows.h>	
	#include <stddef.h>
#ifndef _I64_MAX
	#include <limits.h>
#endif
  	#ifndef STRICT
		#define STRICT
	#endif	
	typedef BOOL ZBOOL;
	
#endif	// #ifndef _WIN32


#ifndef ASSERT
	#ifdef _DEBUG
		#include <assert.h>
		#define ASSERT(f) assert((f))
	#else
		#define ASSERT(f)
	#endif
#endif

#if !defined(_INTPTR_T_DEFINED) && !defined(__GNUC__) && !defined __BORLANDC__
	typedef long intptr_t;
#endif

#define ZIP_FILE_USIZE ULONGLONG
#define ZIP_FILE_SIZE LONGLONG
#define ZIP_FILE_SIZEMAX _I64_MAX
