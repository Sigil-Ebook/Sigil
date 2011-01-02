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
* \file ZipExport.h
*	Contains the Windows DLL exports definitions.
*
*/


#if !defined (ZIPARCHIVE_ZIPEXPORT_DOT_H)
#define ZIPARCHIVE_ZIPEXPORT_DOT_H

#if defined (ZIP_HAS_DLL)
#    if defined (ZIP_BUILD_DLL)
#      define ZIP_API __declspec (dllexport)
#    else
#      define ZIP_API  __declspec (dllimport)
#	endif
#else
#  define ZIP_API
#endif     /* ZIP_HAS_DLL */

#endif     /* ZIPARCHIVE_ZIPEXPORT_DOT_H */
