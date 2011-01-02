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
* \file _features.h
*	Contains the definitions that enable or disable certain features in the ZipArchive Library.
*
*/
#if !defined(ZIPARCHIVE_FEATURES_DOT_H)
/// @cond
#define ZIPARCHIVE_FEATURES_DOT_H 
/// @endcond

#if _MSC_VER > 1000
#pragma once
#endif

// #define _ZIP_TRIAL
#include "_platform.h"

#ifdef __GNUC__

#ifndef __int64
	#define __int64 long long
#endif

#endif

/************ Feel free to adjust the definitions in the following block ************/
/************************************ BLOCK START ***********************************/

/**
	Define it, if you use ZIP64.

	\see
		<a href="kb">0610051629</a>
*/
// #define _ZIP_ZIP64
/**
	Define it, if you use AES.

	\see
		<a href="kb">0610201627|aes</a>
*/
// #define _ZIP_AES
/**
	Define it, if you use the BZIP2 algorithm for compression.

	\see
		<a href="kb">0610231446|bzip2</a>
*/
// #define _ZIP_BZIP2
/**
	Define it, if you want to create seekable data.

	\see
		<a href="kb">0711101739</a>

*/
// #define _ZIP_SEEK
/**
	Define it, if you want to use Unicode support for filenames and comments (WinZip compatible). This functionality is available in the Full Version only.
	Under Windows, this functionality requires Unicode compilation.

	\see
		<a href="kb">0610051525|unicode</a>

*/
// #define _ZIP_UNICODE
/**
	Define it, if you want to use custom Unicode support for filenames and comments provided by the ZipArchive Library. This functionality is available only under Windows and is deprecated.
	Requires Unicode compilation.

	\see
		<a href="kb">0610051525|custom</a>

*/
// #define _ZIP_UNICODE_CUSTOM
/**
	Define it, if you use Unicode and under Windows you decompress archives from systems that use different Unicode Normalization form for filenames (like Mac OS X).
	This functionality is available only under Windows and requires Unicode compilation.

	\see
		<a href="kb">0610051525|considerNormal</a>

*/
// #define _ZIP_UNICODE_NORMALIZE
/**
	Define it, if you use the AES encryption in a multithreaded environment or archive sharing (CZipArchive::OpenFrom).

	\see
		<a href="kb">0610201627|aes</a>
	\see
		<a href="kb">0610241003|thread</a>
*/
// #define _ZIP_USE_LOCKING
#ifndef _ZIP_ZIP64
// Uncomment this to have the index and volume numbers types defined as WORD. Otherwise they are defined as int.
#define _ZIP_STRICT_U16
#endif

/************************************* BLOCK END ***********************************/
/***** The contents below this line are usually not intended for modification ******/


/**
	Default implementation of CZipFile class. 

	\see
			<a href="kb">0610050933|fileImpl</a>
*/
#define ZIP_ZFI_DEFAULT 0

/**
	STL implementation of CZipFile class. 

	\see
			<a href="kb">0610050933|fileImpl</a>
*/
#define ZIP_ZFI_STL 1

/**
	Windows API implementation of CZipFile class. 

	\see
			<a href="kb">0610050933|fileImpl</a>
*/
#define ZIP_ZFI_WIN 2

/**
	Active implementation of CZipFile class. 

	\see
			<a href="kb">0610050933|fileImpl</a>
*/
#define _ZIP_FILE_IMPLEMENTATION ZIP_ZFI_DEFAULT

#if defined _ZIP_ZIP64 && (defined __BORLANDC__ || (defined _MSC_VER && _MSC_VER < 1300 && defined _ZIP_IMPL_MFC))
	/**
		Define this, if you want to use Zip64 functionality in Visual Studio 6.0 MFC or C++Builder. These platforms do not support large files, 
		but this definition will cause to use the STL implementation of CZipFile (which has the support for large files) instead of MFC.
		\see
			<a href="kb">0610050933|fileImpl</a>
	*/
	#if _ZIP_FILE_IMPLEMENTATION != ZIP_ZFI_WIN
		#undef _ZIP_FILE_IMPLEMENTATION
		#define _ZIP_FILE_IMPLEMENTATION ZIP_ZFI_WIN		
	#endif
#endif

#ifdef _ZIP_UNICODE_NORMALIZE
	#if !defined _MSC_VER && !defined __BORLANDC__
		#undef _ZIP_UNICODE_NORMALIZE
	#else
		// uncomment the below, if you are having compilation problems with Unicode Normalization functions in ZipPlatform::MultiByteToWide
		//#if WINVER < 0x600
		//	#define WINVER 0x600
		//#endif
	#endif
#endif

#endif // !defined(ZIPARCHIVE_FEATURES_DOT_H)
