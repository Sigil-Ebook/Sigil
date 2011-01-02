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

#if defined _MSC_VER && _MSC_VER < 1300	
	// STL warnings 	
	#pragma warning (push, 3) 
#endif

#include "FileFilter.h"

namespace ZipArchiveLib
{

bool CGroupFileFilter::Accept(LPCTSTR lpszParentDir, LPCTSTR lpszName, const CFileInfo& info)
{
	bool conditionToBreak;
	bool valueToReturn;

	// handle the evaluation as quickly as possible
	if (m_iType == CGroupFileFilter::And)
	{
		conditionToBreak = false;
		valueToReturn = m_bInverted;
	}
	else
	{
		conditionToBreak = true;
		valueToReturn = !m_bInverted;
	}

	for (ZIP_ARRAY_SIZE_TYPE i = 0; i < m_filters.GetSize(); i++)
	{
		CFileFilter* pFilter = m_filters[i];
		if (pFilter->HandlesFile(info) && pFilter->Evaluate(lpszParentDir, lpszName, info) == conditionToBreak)
			return valueToReturn;
	}

	return !valueToReturn;
	
}

} // namespace


#if defined _MSC_VER && _MSC_VER < 1300
	// STL warnings 
	#pragma warning (pop) 
#endif
