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

#ifndef ZIPARCHIVE_ZIPCOLLECTIONS_DOT_H
	#error Do not include this file directly. Include ZipCollections.h instead
#endif

#if _MSC_VER > 1000
	#pragma warning( push )
	#pragma warning (disable:4786) // 'identifier' : identifier was truncated to 'number' characters in the debug information
#endif

#include <afxtempl.h>

#define ZIP_ARRAY_SIZE_TYPE INT_PTR

typedef CStringArray CZipStringArray;

template <class TYPE>
class CZipArray : public CArray<TYPE, TYPE>
{
public:
	typedef int (*CompareFunction)(const void* pArg1, const void* pArg2);
private:
	static int CompareAsc(const void *pArg1, const void *pArg2)
	{
		TYPE w1 = *(TYPE*)pArg1;
		TYPE w2 = *(TYPE*)pArg2;
		return w1 == w2 ? 0 :(w2 > w1 ? - 1 : 1);
	}
	static int CompareDesc(const void *pArg1, const void *pArg2)
	{
		TYPE w1 = *(TYPE*)pArg1;
		TYPE w2 = *(TYPE*)pArg2;
		return w1 == w2 ? 0 :(w1 > w2 ? - 1 : 1);		
	}
public:	
	void Sort(bool bAscending)
	{
		Sort(bAscending ? CompareAsc : CompareDesc);
	}
	void Sort(CompareFunction pFunction)
	{
		INT_PTR uSize = GetSize();
		if (!uSize) // if omitted operator [] will fail if empty
			return;
		qsort((void*)&((*this)[0]), (size_t)uSize , sizeof(TYPE), pFunction);
	}
};

template<class TYPE>
class CZipPtrList : public CTypedPtrList<CPtrList, TYPE>
{
public:
	typedef POSITION iterator;
	typedef POSITION const_iterator;

	bool IteratorValid(const iterator &iter) const
	{
		return iter != NULL;
	}

};

template<class KEY, class VALUE>
class CZipMap : public CMap<KEY, KEY, VALUE, VALUE>
{
public:
	typedef POSITION iterator;
	typedef POSITION const_iterator;

	bool IteratorValid(const iterator &iter) const
	{
		return iter != NULL;
	}
};

#if _MSC_VER > 1000
	#pragma warning( pop )
#endif