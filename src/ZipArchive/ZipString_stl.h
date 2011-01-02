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

#ifndef ZIPARCHIVE_ZIPSTRING_DOT_H
	#error Do not include this file directly. Include ZipString.h instead
#endif

#include "stdafx.h"

#if _MSC_VER > 1000
	#pragma warning( push, 3 ) // STL requirements
	#pragma warning( disable : 4275 ) // non dll-interface class used as base for dll-interface
	#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
#endif


#include <cstring>
#include <algorithm>
#include <stdarg.h>
#include <stdio.h>
#include <cctype>
#include <locale>

#include "ZipExport.h"

#ifndef __GNUC__
    #ifndef _vsntprintf 
	#ifdef  _UNICODE
		#define _vsntprintf _vsnwprintf
	#else
		#define _vsntprintf _vsnprintf
	#endif
    #endif
#elif !defined(_vsntprintf)
       #define _vsntprintf vsnprintf
#endif

typedef std::basic_string<TCHAR> stdbs;
/**	
	It contains mostly the methods required by ZipArchive Library.
*/
class ZIP_API CZipString : public stdbs
{
	void TrimInternalL(size_type iPos)
	{
		if (iPos == npos)
			erase ();
		if (iPos)
			erase(0, iPos);
	}
	void TrimInternalR(size_type iPos)
	{
		if (iPos == npos)
			erase ();
		erase(++iPos);
	}
	
#ifndef __GNUC__
	static int zslen(const TCHAR* lpsz)
	{
		if (!lpsz) return 0;

		// we want to take into account the locale stuff (by using standard templates)

		#ifdef _UNICODE
			return (int)std::wstring(lpsz).length();
		#else
			return (int)std::string(lpsz).length();
		#endif
	}	
#else
   	static int zslen(const TCHAR* lpsz)
   	{
  		#if (__GNUC__ < 3) // I'm not sure which precisely version should be put here
      		return lpsz ? std::string_char_traits<TCHAR>::length(lpsz) : 0;
		#else
      		return lpsz ? std::char_traits<TCHAR>::length(lpsz) : 0;
		#endif

	}	
#endif

static TCHAR tl(TCHAR c)
{
	// use_facet doesn't work here well (doesn't convert all the local characters properly)
	return std::tolower(c, std::locale());
}
static TCHAR tu(TCHAR c)
{
	// use_facet doesn't work here well (doesn't convert all the local characters properly)
	return std::toupper(c, std::locale());
}

public:
	CZipString(){}
	explicit CZipString (TCHAR ch, int nRepeat = 1):stdbs(nRepeat, ch){}
	CZipString( const CZipString& stringSrc ) {assign(stringSrc);}
	CZipString( const stdbs& stringSrc ) {assign(stringSrc);}
	CZipString( LPCTSTR lpsz ){if (!lpsz) Empty(); else assign(lpsz);}
	operator LPCTSTR() const{return c_str();}
	
	int GetLength() const {return (int) size();}
	bool IsEmpty() const {return empty();}
	void Empty() {erase(begin(), end());}
	TCHAR GetAt (int iIndex) const{return at(iIndex);}
	TCHAR operator[] (int iIndex) const{return at(iIndex);}
	void SetAt( int nIndex, TCHAR ch ) {at(nIndex) = ch;}
	LPTSTR GetBuffer(int nMinBufLength)
	{
		if ((int)size() < nMinBufLength)
			resize(nMinBufLength);
      		return empty() ? const_cast<TCHAR*>(data()) : &(at(0));
	}
	void ReleaseBuffer( int nNewLength = -1 ) { resize(nNewLength > -1 ? nNewLength : zslen(c_str()));}
	void TrimLeft( TCHAR chTarget )
	{
		TrimInternalL(find_first_not_of(chTarget));
	}
	void TrimLeft( LPCTSTR lpszTargets )
	{
		TrimInternalL(find_first_not_of(lpszTargets));
	}
	void TrimRight( TCHAR chTarget )
	{
		TrimInternalR(find_last_not_of(chTarget));
	}
	void TrimRight( LPCTSTR lpszTargets )
	{
		TrimInternalR(find_last_not_of(lpszTargets));
	}

#if _MSC_VER >= 1300
	#pragma warning( push )
	#pragma warning (disable : 4793) // 'vararg' : causes native code generation for function 'void CZipString::Format(LPCTSTR,...)'
#endif

	void Format(LPCTSTR lpszFormat, ...)
	{
		va_list arguments;
		va_start (arguments, lpszFormat);
		TCHAR* pBuf = NULL;
		int iCounter = 1, uTotal = 0;
		do 
		{
			int nChars = iCounter * 1024;
			int nLen = sizeof(TCHAR) * nChars;
			
			TCHAR* pTempBuf = (TCHAR*)realloc((void*)pBuf, nLen);			
			if (!pTempBuf)
			{
				if (pBuf != NULL)
					free(pBuf);
				va_end (arguments);
				return;
			}
			pBuf = pTempBuf;

#if _MSC_VER >= 1400	
			uTotal = _vsntprintf_s(pBuf, nChars, nChars - 1, lpszFormat, arguments);
#else
			uTotal = _vsntprintf(pBuf, nChars - 1, lpszFormat, arguments);
#endif
			
			if (uTotal == -1 || (uTotal == nChars - 1) ) // for some implementations
			{
				pBuf[nChars - 1] = _T('\0');
				if (iCounter == 7)
					break;
			}
			else
			{
				pBuf[uTotal] = _T('\0');
				break;
			}
			iCounter++;

		} while (true);
		
		va_end (arguments);
	    *this = pBuf;
		free(pBuf);
	}

#if _MSC_VER >= 1300
	#pragma warning( pop )
#endif

	void Insert( int nIndex, LPCTSTR pstr ){insert(nIndex, pstr, zslen(pstr));}
	void Insert( int nIndex, TCHAR ch ) {insert(nIndex, 1, ch);}
	int Delete( int nIndex, int nCount = 1 )
	{
		int iSize = (int) size();
		int iToDelete = iSize < nIndex + nCount ? iSize - nIndex : nCount;
		if (iToDelete > 0)
		{
			erase(nIndex, iToDelete);
			iSize -= iToDelete;
		}
		return iSize;
	}
#ifndef __MINGW32__	
	void MakeLower() 
	{
			std::transform(begin(),end(),begin(),tl);
	}
	void MakeUpper() 
	{
			std::transform(begin(),end(),begin(),tu);
	}
#else
	void MakeLower() 
	{
			std::transform(begin(),end(),begin(),tolower);
	}
	void MakeUpper() 
	{
			std::transform(begin(),end(),begin(),toupper);
	}
#endif	
	void MakeReverse()
	{
		std::reverse(begin(), end());

	}
	CZipString Left( int nCount ) const { return substr(0, nCount);}
	CZipString Right( int nCount) const 
	{
		int s = (int)size();
		nCount = s < nCount ? s : nCount;
		return substr(s - nCount);
	}
	CZipString Mid( int nFirst ) const {return substr(nFirst);}
	CZipString Mid( int nFirst, int nCount ) const {return substr(nFirst, nCount);}
	int Collate( LPCTSTR lpsz ) const
	{
#if !defined __GNUC__ || defined __MINGW32__
		return _tcscoll(c_str(), lpsz);
#else
 		//return compare(lpsz);
		return strcoll(c_str(), lpsz);
#endif
	}

	int CollateNoCase( LPCTSTR lpsz ) const
	{
#if !defined __GNUC__ || defined __MINGW32__
		return _tcsicoll(c_str(), lpsz);
#else
		if (std::locale() == std::locale::classic())
			return strcasecmp(c_str(), lpsz);
		else
			// this may be not case-insensitive !!!
			return strcoll(c_str(), lpsz);
			//return stricoll(c_str(), lpsz);
#endif
	}

	int Compare( LPCTSTR lpsz ) const
	{
		return compare(lpsz);
	}

	int CompareNoCase( LPCTSTR lpsz ) const
	{
#if !defined __GNUC__ || defined __MINGW32__
		return _tcsicmp(c_str(), lpsz);
#else
		return strcasecmp(c_str(), lpsz);
		//return stricmp(c_str(), lpsz);
#endif
	}
	
	bool operator != (LPCTSTR lpsz)
	{
		return Compare(lpsz) != 0;
	}
	bool operator == (LPCTSTR lpsz)
	{
		return Compare(lpsz) == 0;
	}
	int Find( TCHAR ch, int nStart = 0) const
	{
		return (int) find(ch, nStart);
	}

	int Find( LPCTSTR pstr, int nStart = 0) const
	{
		return (int) find(pstr, nStart);
	}

	int Replace( TCHAR chOld, TCHAR chNew )
	{
		int iCount = 0;
		for (iterator it = begin(); it != end(); ++it)
			if (*it == chOld)
			{
				*it = chNew;
				iCount++;
			}
		return iCount;
	}
	
};

/**
	A pointer type to point to CZipString to Collate or CollateNoCase
	or Compare or CompareNoCase
*/
typedef int (CZipString::*ZIPSTRINGCOMPARE)( LPCTSTR ) const;

/**
	return a pointer to the function in CZipString structure, 
	used to compare elements depending on the arguments
*/
	ZIP_API ZIPSTRINGCOMPARE GetCZipStrCompFunc(bool bCaseSensitive, bool bCollate = true);


#if _MSC_VER > 1000
	#pragma warning( pop)
#endif
