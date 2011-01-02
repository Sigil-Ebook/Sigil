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

#include "stdafx.h"
#include "BaseLibCompressor.h"

namespace ZipArchiveLib
{

void CBaseLibCompressor::SetOpaque(void** opaque, const COptions* pOptions)
{
	*opaque = pOptions->m_bDetectLibMemoryLeaks ? &m_list : 0;
}

void CBaseLibCompressor::EmptyPtrList()
{
	if (m_list.GetCount())
	{
		// if some memory hasn't been freed due to an error in zlib, so free it now
		CZipPtrListIter iter = m_list.GetHeadPosition();
		while (m_list.IteratorValid(iter))
			delete[] (char*) m_list.GetNext(iter);
	}
	m_list.RemoveAll();
}

} // namespace






