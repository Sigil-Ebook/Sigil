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
* \file ZipBaseException.h
*	Contains a type definition of the base exception class.
*
*/

#ifndef ZIPARCHIVE_ZIPBASEEXCEPTION_DOT_H
#define ZIPARCHIVE_ZIPBASEEXCEPTION_DOT_H

	#ifdef _ZIP_IMPL_STL
		typedef std::exception CZipBaseException;
	#else
		typedef CException CZipBaseException;
#endif

#endif //ZIPARCHIVE_ZIPBASEEXCEPTION_DOT_H
