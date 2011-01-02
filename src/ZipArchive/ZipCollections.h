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
* \file ZipCollections.h
*	Includes the collections classes used by the ZipArchive Library.
*
*/


#ifndef ZIPARCHIVE_ZIPCOLLECTIONS_DOT_H
#define ZIPARCHIVE_ZIPCOLLECTIONS_DOT_H

#if _MSC_VER > 1000
	#pragma once
#endif

#include "_platform.h"
#include "ZipExport.h"

#ifdef _ZIP_IMPL_STL
	#include "ZipCollections_stl.h"
#else
	#include "ZipCollections_mfc.h"	
#endif

typedef CZipArray<ZIP_INDEX_TYPE> CZipIndexesArray;

#endif  /* ZIPARCHIVE_ZIPCOLLECTIONS_DOT_H */

