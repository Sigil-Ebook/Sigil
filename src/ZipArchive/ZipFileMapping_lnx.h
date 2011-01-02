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

#ifndef ZIPARCHIVE_ZIPFILEMAPPING_DOT_H
	#error Do not include this file directly. Include ZipFileMapping.h instead
#endif


#include <sys/mman.h>

namespace ZipArchiveLib
{
	struct CZipFileMapping
	{
		CZipFileMapping()
		{
			m_iSize = 0;
			m_pFileMap = NULL;
		}
		bool CreateMapping(CZipFile* pFile)
		{
			if (!pFile)
				return false;
			m_iSize = pFile->GetLength();
			m_pFileMap = mmap(0, m_iSize, PROT_READ|PROT_WRITE, MAP_SHARED, pFile->m_hFile, 0);
			return (m_pFileMap != NULL);
		}
		void RemoveMapping()
		{
                
			if (m_pFileMap)
			{
				munmap(m_pFileMap, m_iSize);
				m_pFileMap = NULL;
			}
		}
		~CZipFileMapping()
		{
			RemoveMapping();
		}
		char* GetMappedMemory()
		{
			return reinterpret_cast<char*> (m_pFileMap);
		}
	protected:
		void* m_pFileMap;
		size_t m_iSize;

	};
}

