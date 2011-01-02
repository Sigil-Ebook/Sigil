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
* \file ZipException.h
* Includes the CZipException class.
*
*/

#if !defined(ZIPARCHIVE_ZIPEXCEPTION_DOT_H)
#define ZIPARCHIVE_ZIPEXCEPTION_DOT_H

#if _MSC_VER > 1000
#pragma once
#pragma warning( push )
#pragma warning (disable:4702) // disable "Unreachable code" warning in Throw function in the Release mode
	#if defined ZIP_HAS_DLL
		#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
		#pragma warning( disable : 4275 ) // non dll-interface used as base for dll-interface class
	#endif
#endif


#include "ZipString.h"
#include "ZipBaseException.h"
#include "ZipExport.h"
 
/**
	Represents exceptions specific to the ZipArchive Library.

	\see
		<a href="kb">0610222049</a>
*/
class ZIP_API CZipException : public CZipBaseException
{
public:

	/**
		Throws an exception.
		Whether it throws an object or a pointer to it, depends 
		on the current version (STL or MFC correspondingly).

		\param	iCause
			The error cause. It takes one of the #ZipErrors values.

		\param	lpszZipName
			The name of the file where the error occurred (if applicable).
			May be \c NULL.

		\see
			<a href="kb">0610222049</a>
	*/
	static void Throw(int iCause = CZipException::genericError, LPCTSTR lpszZipName = NULL)
	{
		#ifdef _ZIP_IMPL_MFC
			throw new CZipException(iCause, lpszZipName);
		#else
			CZipException e(iCause, lpszZipName);
			throw e;
		#endif
	}

	/**
		Initializes a new instance of the CZipException class.

		\param	iCause
			The error cause. Takes one of the #ZipErrors values.

		\param	lpszZipName
			The name of the file where the error occurred (if applicable).
			May be \c NULL.
	*/
	CZipException(int iCause = genericError, LPCTSTR lpszZipName = NULL);

	CZipException(CZipException& e)
	{
		m_szFileName = e.m_szFileName;
		m_iCause = e.m_iCause;
		m_iSystemError = e.m_iSystemError;
	}

#ifdef _ZIP_ENABLE_ERROR_DESCRIPTION

    /**
		Returns the error description.

		\return 
			The error description.
     */
	CZipString GetErrorDescription();

	
    /**
		Returns the error description. This method is provided for compatibility with the MFC version (\c CException::GetErrorMessage).

		\param lpszError 
			The buffer to receive the error message.

		\param nMaxError
			The maximum number of characters \a lpszError can hold, 
			including the ending \c NULL character.

		\return 
			\c TRUE if the error string was successfully copied to \a lpszError; \c FALSE otherwise.

		\note 
			The method will not copy more than \c nMaxError - 1 characters 
			to the buffer, and it always appends a \c NULL character.
			If \a lpszError is too small, the error message will be truncated.
     */
	ZBOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError, UINT* = NULL);

#endif //_ZIP_ENABLE_ERROR_DESCRIPTION

	/**
		The name of the archive for which the error occurred.
	*/
	CZipString m_szFileName;

	/**
		The codes of errors thrown by the ZipArchive Library.
	*/
	enum ZipErrors
	{
		noError,			///< No error.
// 			 1 - 199 reserved for errno (from STL) values - used only in non-MFC versions
		genericError		= 200,	///< An unspecified error.
		badZipFile,			///< Damaged or not a zip file.
		badCrc,				///< Crc is mismatched.
		noCallback,			///< There is no spanned archive callback object set.
		noVolumeSize,		///< The volume size was not defined for a split archive.
		aborted,			///< The volume change callback in a segmented archive method returned \c false.
		abortedAction,		///< The action callback method returned \c false.
		abortedSafely,		///< The action callback method returned \c false, but the data is not corrupted.
		nonRemovable,		///< The device selected for the spanned archive is not removable.
		tooManyVolumes,		///< The limit of the maximum number of volumes has been reached.
		tooManyFiles,		///< The limit of the maximum number of files in an archive has been reached.
		tooLongData,		///< The filename, the comment or local or central extra field of the file added to the archive is too long.
		tooBigSize,			///< The file size is too large to be supported.
		badPassword,		///< An incorrect password set for the file being decrypted.
		dirWithSize,		///< The directory with a non-zero size found while testing.
		internalError,		///< An internal error.
		fileError,			///< A file error occurred. Examine #m_iSystemError for more information.
		notRemoved,			///< Error while removing a file. Examine #m_iSystemError for more information.
		notRenamed,			///< Error while renaming a file. Examine #m_iSystemError for more information.
		platfNotSupp,		///< Cannot create a file for the specified platform.
		cdirNotFound,		///< The central directory was not found in the archive (or you were trying to open not the last disk of a segmented archive).				
		noZip64,			///< The Zip64 format has not been enabled for the library, but is required to use the archive.
		noAES,				///< WinZip AES encryption has not been enabled for the library, but is required to decompress the archive.
#ifdef _ZIP_IMPL_STL
		outOfBounds,		///< The collection is empty and the bounds do not exist.
#endif
#ifdef _ZIP_USE_LOCKING
		mutexError,			///< Locking or unlocking resources access was unsuccessful.
#endif
		streamEnd	= 500,	///< Zlib library error.
		needDict,			///< Zlib library error.
		errNo,				///< Zlib library error.
		streamError,		///< Zlib library error.
		dataError,			///< Zlib library error.
		memError,			///< Zlib library or \c CZipMemFile error.
		bufError,			///< Zlib library error.
		versionError,		///< Zlib library error.
	};

	/**
		The error code. It takes one of the CZipException::ZipErrors values.
	*/
	int m_iCause;
	/**
		An error code reported by the system during the recent operation.
		It is set to \c <code>GetLastError()</code> value on Windows and to \c errno on other platforms.		
	*/
	ZIP_SYSTEM_ERROR_TYPE m_iSystemError;
	virtual ~CZipException() throw();

protected:

#ifdef _ZIP_ENABLE_ERROR_DESCRIPTION

	/**
		Returns the error description.

		\param	iCause
			The error cause. Takes one of the #ZipErrors values.

		\param bNoLoop
			If \c true, does not search for en error description, it the error code is #genericError.

		\return 
			The error description.
	 */
	CZipString GetInternalErrorDescription(int iCause, bool bNoLoop = false);


	/**
	   Returns the error description based on system variables.
	   
	  \return 
			The error description.
	 */
	CZipString GetSystemErrorDescription();


#endif //_ZIP_ENABLE_ERROR_DESCRIPTION

#if defined _MFC_VER && defined _ZIP_IMPL_MFC
	DECLARE_DYNAMIC(CZipException)
#endif
};

#if _MSC_VER > 1000
	#pragma warning( pop )
#endif

#endif // !defined(ZIPARCHIVE_ZIPEXCEPTION_DOT_H)


