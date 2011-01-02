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
* \file ZipPathComponent.h
* Includes the CZipPathComponent class.
*
*/
#if !defined(ZIPARCHIVE_ZIPPATHCOMPONENT_DOT_H)
#define ZIPARCHIVE_ZIPPATHCOMPONENT_DOT_H

#if _MSC_VER > 1000
	#pragma once
	#if defined ZIP_HAS_DLL
		#pragma warning (push)
		#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
	#endif
#endif

#include "ZipString.h"
#include "ZipExport.h"

/**
	Splits a file path into components.
*/
class ZIP_API CZipPathComponent  
{
	
public:	
#ifdef _ZIP_SYSTEM_WIN
	static const CZipString PathPrefix;
	/**
		The type of the prefix in path.
	*/
	enum PrefixType
	{
		ptNone = 0,			///< There is no prefix present.
		ptUnc = 2,			///< UNC path.
		ptUnicode = 4,		///< Unicode path.
		ptUncWin = 8		///< Windows UNC path.
	};	
	/**
		Returns the length of the path prefix detected.

		\param path
			The path to examine.

		\return
			The length of the path prefix or \c 0, if no prefix was detected.
	*/
	static int IsPrefixed(const CZipString& path);

	/**
		Adds an UNC prefix to the paths.

		\param path
			The path to add a prefix to.

		\param isFolder
			\c true if the \a path points to a folder; \c false otherwise.

		\return
			The prefixed path.
	*/
	static CZipString AddPrefix(LPCTSTR path, bool isFolder = true);

	/**
		Adds an UNC prefix to the paths.

		\param path
			The path to add a prefix to.

		\param isFolder
			\c true if the \a path points to a folder; \c false otherwise.
	*/
	static void AddPrefix(CZipString& path, bool isFolder = true);

#endif
	CZipPathComponent(){}
	/**
		Initializes a new instance of the CZipPathComponent class.

		\param	lpszFullPath
			The full path to the file.

		\see
			SetFullPath
	*/
	CZipPathComponent(LPCTSTR lpszFullPath)
	{
		SetFullPath(lpszFullPath);
	}

	virtual ~CZipPathComponent();

	static const TCHAR m_cSeparator; ///< A system specific default path separator.

	/**
		Appends a path separator to \a szPath, if it is not already appended.

		\param szPath
			The path to have a separator appended.
	*/
	static void AppendSeparator(CZipString& szPath)
	{
		RemoveSeparators(szPath);
		szPath += m_cSeparator;
	}

	/**
		Combines a path information with a file name information.

		\param szPath
			Provides the path information and retrieves the result.

		\param lpszName
			The filename to be appended to the path.
	*/
	static void Combine(CZipString& szPath, LPCTSTR lpszName)
	{
		AppendSeparator(szPath);
		if (lpszName != NULL)
			szPath += lpszName;
	}

	/**
		Removes path separators from the end of \a szPath

		\param szPath
			The path to have path separators removed.
	*/
	static void RemoveSeparators(CZipString& szPath)
	{
		szPath.TrimRight(_T("\\/"));
	}

	/**
		Removes path separators from the beginning of \a szPath.

		\param szPath
			The path to have path separators removed.
	*/
	static void RemoveSeparatorsLeft(CZipString& szPath)
	{
		szPath.TrimLeft(_T("\\/"));
	}


    /**
		Returns the value indicating whether the given character is a path separator.

		\param c 
			The character to test.

		\return
			\c true, if \a c is a path separator; \c false otherwise.
     */
	static bool IsSeparator(TCHAR c)
	{
		return c == _T('\\') || c == _T('/');
	}
	
	/**
		Checks if \a szPath has a path separator appended.

		\param szPath
			The path to be tested.

		\return
			\c true, if \a szPath has a path separator at the end; \c false otherwise.
	*/
	static bool HasEndingSeparator(const CZipString& szPath)
	{
		int iLen = szPath.GetLength();
		if (iLen)
			return IsSeparator(szPath[iLen - 1]);
		else
			return false;
	}
	
	/**
		Sets the full path to the file.

		\param	lpszFullPath
			The full path to the file including a filename.
			The last element in the path is assumed to be the filename.
	*/
	void SetFullPath(LPCTSTR lpszFullPath);

	/**
		Returns the name of the file without the extension (and without the path).

		\return
			The title of the file.
	*/
	CZipString GetFileTitle() const { return m_szFileTitle;}

	/**
		Sets the file title (the name without the extension and without the path).

		\param	lpszFileTitle
			The title to set.
	*/
	void SetFileTitle(LPCTSTR lpszFileTitle) { m_szFileTitle = lpszFileTitle;}

	
	/**
		Sets the extension.

		\param	lpszExt
			The extension to set. May contain the dot character at the beginning, but doesn't have to.
	*/
	void SetExtension(LPCTSTR lpszExt) 
	{
		m_szFileExt = lpszExt;
		m_szFileExt.TrimLeft(_T('.'));
	}

	/**
		Returns the extension of the file.

		\return
			The extension without the dot character.
	*/
	CZipString GetFileExt() const { return m_szFileExt;}

	/**
		Returns the drive of the file.

		\return
			The drive without a path separator at the end.
	*/
	CZipString GetFileDrive() const { return m_szDrive;}

	/**
		Returns the full path to the file without the drive.

		\return
			The path without the drive and without a path separator at the beginning.
	*/
	CZipString GetNoDrive() const ;

	/**
		Returns the filename.

		\return
			The filename including the extension and without the path.
	*/
	CZipString GetFileName() const
	{
		CZipString szFullFileName = m_szFileTitle;
		if (!m_szFileExt.IsEmpty())
		{
			szFullFileName += _T(".");
			szFullFileName += m_szFileExt;
		}
		return szFullFileName;
	}

	/**
		Returns the full path to the file.

		\return
			The full path information including the filename.
	*/
	CZipString GetFullPath() const
	{
		CZipString szFullPath = GetFilePath();
		CZipString szFileName = GetFileName();
		if (!szFileName.IsEmpty())
		{
			if (szFullPath.IsEmpty())
				szFullPath += _T('.');
			szFullPath  += m_cSeparator;
			szFullPath  += szFileName;
		}
		return szFullPath;
	}

	/**
		Returns the path part only.

		\return
			The file path without the filename and without a path separator at the end.
	*/
	CZipString GetFilePath() const
	{
		CZipString szDrive = m_szDrive;
		CZipString szDir = m_szDirectory;
		if (!szDrive.IsEmpty() && !szDir.IsEmpty())
			szDrive += m_cSeparator;

		return m_szPrefix + szDrive + szDir;	
	}
protected:
	/**
		\name Path parts.
	*/
	//@{
	CZipString m_szDirectory,	///< The path without the filename and without path separators at the end and the beginning.
		m_szFileTitle,			///< The filename without the extension.
		m_szFileExt,			///< The file extension without the dot character.
		m_szDrive,				///< The drive (if the system path standard uses it). It does not include a path separator at the end.
		m_szPrefix;				///< The prefix (e.g. for the UNC path or Unicode path under Windows).
	//@}
	
};

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning (pop)	
#endif


#endif // !defined(ZIPARCHIVE_ZIPPATHCOMPONENT_DOT_H)
