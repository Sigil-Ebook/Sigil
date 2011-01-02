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
* \file BytesWriter.h
*	Includes the ZipArchiveLib::CBytesWriter class.
*
*/

#if !defined(ZIPARCHIVE_BYTESWRITER_DOT_H)
#define ZIPARCHIVE_BYTESWRITER_DOT_H

#if _MSC_VER > 1000
	#pragma once
#endif

#include "ZipCompatibility.h"

namespace ZipArchiveLib
{
	/**
		Provides implementation for various 
		buffer operations depending on the current platform and configuration.
	 */
	class ZIP_API CBytesWriter
	{
	public:
		
	#ifndef _ZIP_BIG_ENDIAN
		/**
			Reads \a iCount bytes from \a pSource into \a pDestination.

			\param[out] uDestination
				The buffer to retrieve data with byte-ordering depending on the machine.

			\param[in] pSource
				The buffer with little-endian ordered data.

			\param iCount 
				The number of bytes to read.
		*/
		static void ReadBytes(WORD& uDestination, const char* pSource, int iCount = 2)
		{
			uDestination = 0;
			memcpy(&uDestination, pSource, iCount);
		}

		static void ReadBytes(DWORD& uDestination, const char* pSource, int iCount = 4)
		{
			uDestination = 0;
			memcpy(&uDestination, pSource, iCount);
		}


		#ifndef _ZIP_STRICT_U16
		static void ReadBytes(int& iDestination, const char* pSource, int iCount)
		{
			iDestination = 0;
			memcpy(&iDestination, pSource, iCount);
		}
		#endif

		
		static void WriteBytes(char* pDestination, WORD uSource)
		{
			memcpy(pDestination, &uSource, 2);
		}

		/**
			Writes \a iCount bytes from \a pSource into \a pDestination.

			\param[out] pDestination
				The buffer to retrieve little-endian ordered data.

			\param[in] uSource
				The buffer with byte-ordering depending on the machine.

			\param iCount 
				The number of bytes to write.
		*/
		static void WriteBytes(char* pDestination, DWORD uSource, int iCount = 4)
		{
			memcpy(pDestination, &uSource, iCount);
		}

		#ifndef _ZIP_STRICT_U16
		static void WriteBytes(char* pDestination, int uSource, int iCount)
		{
			memcpy(pDestination, &uSource, iCount);
		}
		#endif

	#else

		static void ReadBytes(char* pDestination, const char* pSource, int iDestSize, int iCount)
		{
			int i = iCount - iDestSize;
			while (i < 0)
			{
				*pDestination++ = 0;
				i++;
			}
			for (; i < iCount; i++)
				(pDestination)[i] = pSource[iCount - i - 1];
		}
		
		static void ReadBytes(WORD& uDestination, const char* pSource, int iCount = 2)
		{
			ReadBytes((char*)&uDestination, pSource, 2, iCount);
		}

		static void ReadBytes(DWORD& uDestination, const char* pSource, int iCount = 4)
		{
			ReadBytes((char*)&uDestination, pSource, 4, iCount);
		}


		#ifndef _ZIP_STRICT_U16
		static void ReadBytes(int& iDestination, const char* pSource, int iCount)
		{
			ReadBytes((char*)&iDestination, pSource, sizeof(int), iCount);
		}
		#endif


		static void WriteBytes(char* pDestination, WORD uSource)
		{
			for (int i = 0; i < 2; i++)
				pDestination[i] = ((char*)&uSource)[2 - i - 1];
		}

		static void WriteBytes(char* pDestination, DWORD uSource, int iCount = 4)
		{
			for (int i = 0; i < iCount; i++)
				pDestination[i] = ((char*)&uSource)[4 - i - 1];
		}

		#ifndef _ZIP_STRICT_U16
		static void WriteBytes(char* pDestination, int iSource, int iCount)
		{
			for (int i = 0; i < iCount; i++)
				pDestination[i] = ((char*)&iSource)[sizeof(int) - i - 1];
		}
		#endif

	#endif
		
		static DWORD WriteSafeU32(DWORD uValue)
		{
			return uValue;
		}

	#ifdef _ZIP_STRICT_U16
		static WORD WriteSafeU16(WORD uValue)
		{
			return uValue;
		}
	#else
		static WORD WriteSafeU16(int uValue)
		{
			return (WORD)uValue;
		}
	#endif


	};
}

#endif
