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
* \file _platform.h
*	Contains definitions that determine the target compilation platform.
*
*/

#if !defined(ZIPARCHIVE_PLATFORM_DOT_H)
#define ZIPARCHIVE_PLATFORM_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

/************ Feel free to adjust the definitions in the following block ************/
/************************************ BLOCK START ***********************************/

//#define _ZIP_IMPL_MFC
//#define _ZIP_SYSTEM_LINUX

// simplified endianess detection
#ifdef __APPLE__
	#if  __BIG_ENDIAN__ == 1
		#define _ZIP_BIG_ENDIAN
	#endif
#endif

/************************************* BLOCK END ***********************************/
/********* The contents below this line are not intended for modification **********/

#ifndef _ZIP_IMPL_MFC
	#define _ZIP_IMPL_STL
#else
	#ifdef _ZIP_IMPL_STL
		#undef _ZIP_IMPL_STL
	#endif
#endif

#ifndef _ZIP_SYSTEM_LINUX
	#define _ZIP_SYSTEM_WIN
#else
	#ifdef _ZIP_SYSTEM_WIN
		#undef _ZIP_SYSTEM_WIN
	#endif
#endif

#if defined (_ZIP_SYSTEM_LINUX) && defined (_ZIP_IMPL_MFC)
	#undef _ZIP_IMPL_MFC
	#define _ZIP_IMPL_STL
	#error Using MFC under a non-Windows platform is not supported
#endif

#endif // !defined(ZIPARCHIVE_PLATFORM_DOT_H)
