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
* \file DirEnumerator.h
*	Includes the ZipArchiveLib::CDirEnumerator class.
*
*/

#if !defined(ZIPARCHIVE_DIRENUMERATOR_DOT_H)
#define ZIPARCHIVE_DIRENUMERATOR_DOT_H

#if _MSC_VER > 1000
	#pragma once
	#pragma warning( push )
	#pragma warning (disable : 4100) // unreferenced formal parameter
	#if defined ZIP_HAS_DLL
		#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
	#endif
#endif

#include "ZipString.h"
#include "ZipPathComponent.h"
#include "FileFilter.h"

/**
	Includes helper classes. Some of them can be reused in other applications.
*/
namespace ZipArchiveLib
{
	/**
		A base class for processing multiple files in a directory.
		It provides a directory enumeration functionality.
	*/
	class ZIP_API CDirEnumerator
	{
		LPCTSTR m_lpszDirectory;
		LPCTSTR m_lpszFileNameMask;
		bool m_bRecursive;
		CZipString m_szCurrentDirectory;	
	protected:
		/**
			Initializes a new CDirEnumerator object.

			\param lpszDirectory
				A directory to process.

			\param bRecursive 
				The value indicating whether the subfolders of \a lpszDirectory
				should be processed recursively.

			\see
				GetDirectory
			\see 
				IsRecursive
		*/
		CDirEnumerator(LPCTSTR lpszDirectory, bool bRecursive = true)
		{
			CZipString dir(lpszDirectory);
			if (dir.IsEmpty())
				m_lpszDirectory = _T(".");
			else
				m_lpszDirectory = lpszDirectory;	
			m_bRecursive = bRecursive;
		}

		/**
			Override this method to perform file processing while enumerating directories.
			This method is not called for directories, but for files only.

			\param lpszPath
				The full path to the current file.

			\param info
				A structure containing an information about the current file.

			\return
				Return \c true to continue the enumeration.
				When you return \c false, the enumeration is aborted.

			\see
				CFileFilter::Evaluate
				
		*/
		virtual bool Process(LPCTSTR lpszPath, const CFileInfo& info) = 0;

		/** 
			This method is called at the beginning of the enumeration process.

			\see 
				OnEnumerationEnd
		*/
		virtual void OnEnumerationBegin(){}

		/** 
			This method is called at the end of the enumeration process.

			\param bResult
				It is set to \c false, if the #Process method returned \c false (the enumeration
				was aborted). Otherwise, it is set to \c true.

			\see
				OnEnumerationBegin
		*/
		virtual void OnEnumerationEnd(bool bResult){}

		/**
			This method is called when an enumeration process enters a new directory.

			\see 
				GetCurrentDirectory
			\see
				ExitDirectory
		*/
		virtual void EnterDirectory(){}

		/**
			This method is called when an enumeration process exits a directory.

			\see 
				GetCurrentDirectory
			\see
				EnterDirectory
		*/
		virtual void ExitDirectory(){}
		
	public:		

		/**
			Returns the directory being enumerated.

			\return
				The directory being enumerated (root).	

			\see
				CDirEnumerator::CDirEnumerator
		*/
		LPCTSTR GetDirectory() const {return m_lpszDirectory;} 	

		/**
			Returns the value indicating whether the subfolders of the root directory 
			are processed recursively.

			\return
				\c true, if the enumeration process is recursive; \c false otherwise.

			\see
				CDirEnumerator::CDirEnumerator
		*/
		bool IsRecursive() const {return m_bRecursive;}

		/**
			Returns the directory being currently processed.

			\return 
				The directory being currently processed.
		*/
		LPCTSTR GetCurrentDirectory() const {return m_szCurrentDirectory;}

		/**
			Starts the enumeration process. Calls CFileFilter::Evaluate method for every file or directory found.
			If CFileFilter::Evaluate returns \c true, the file is processed by the #Process method.

			\param filter
				A filter that decides which directories and/or files should be processed and which should not.

			\return 
				\c false, if the process was aborted (the #Process method returned \c false); \c true otherwise.

			\see
				CFileFilter::Evaluate
		*/
		bool Start(CFileFilter& filter);

		virtual ~CDirEnumerator(){}
	private:
		static bool IsDots(LPCTSTR lpszName);
	};

}

#if _MSC_VER > 1000
	#pragma warning( pop )
#endif

#endif

