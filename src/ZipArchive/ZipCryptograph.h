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
* \file ZipCryptograph.h
* Includes the CZipCryptograph class.
*
*/

#if !defined(ZIPARCHIVE_ZIPCRYPTOGRAPH_DOT_H)
#define ZIPARCHIVE_ZIPCRYPTOGRAPH_DOT_H

#if _MSC_VER > 1000
	#pragma once
	#pragma warning( push )
	#pragma warning (disable : 4100) // unreferenced formal parameter
#endif // _MSC_VER > 1000

#include "ZipAutoBuffer.h"
#include "ZipStorage.h"

class CZipFileHeader;

/**
	The base class for cryptographs used in encryption and decryption of file data.

	\see
		<a href="kb">0610201627</a>
*/
class ZIP_API CZipCryptograph
{
public:
	
	/**
		The encryption method.

		\see
			<a href="kb">0610201627</a>
	*/
	enum EncryptionMethod
	{
		encStandard,		///< The traditional zip encryption.
		encWinZipAes128,	///< WinZip AES 128-bit encryption.
		encWinZipAes192,	///< WinZip AES 192-bit encryption.
		encWinZipAes256,	///< WinZip AES 256-bit encryption.
		encNone = 0xFF		///< Indicates no encryption.
	};

	/**
		A factory method that creates an appropriate cryptograph for the given method.

		\param iEncryptionMethod
			The encryption method to create a cryptograph for. It can be one of the #EncryptionMethod values.

		\return
			The new cryptograph. The caller is responsible for destroying the object.
			If the method is not supported, creates CZipCrc32Cryptograph.
	*/
	static CZipCryptograph* CreateCryptograph(int iEncryptionMethod);

	/**
		Returns the value indicating whether the given method is one of the WinZip AES encryption methods.

		\param iEncryptionMethod
			The encryption method to test. It can be one of the #EncryptionMethod values.

		\return
			\c true, if the method is one the WinZip AES encryption methods; \c false otherwise.
	*/
	static bool IsWinZipAesEncryption(int iEncryptionMethod)
	{
		return iEncryptionMethod == encWinZipAes128 || iEncryptionMethod == encWinZipAes192 || iEncryptionMethod == encWinZipAes256;
	}

	/**
		Returns the total size of the extra data that is added to the compression stream during encryption with the given method.

		\param iEncryptionMethod
			The encryption method. It can be one of the #EncryptionMethod values.

		\return
			The total size of extra data for the given encryption method.

	*/
	static DWORD GetEncryptedInfoSize(int iEncryptionMethod);

	/**
		Returns the size of the extra data that is added before the compression stream during encryption with the given method.

		\param iEncryptionMethod
			The encryption method. It can be one of the #EncryptionMethod values.

		\return
			The size of extra data at the beginning of the compression stream for the given encryption method.

	*/
	static DWORD GetEncryptedInfoSizeBeforeData(int iEncryptionMethod);

	/**
		Returns the size of the extra data that is added after the compression stream during encryption with the given method.

		\param iEncryptionMethod
			The encryption method. It can be one of the #EncryptionMethod values.

		\return
			The size of extra data at the end of the compression stream for the given encryption method.

	*/
	static DWORD GetEncryptedInfoSizeAfterData(int iEncryptionMethod);

	/**
		Returns the value indicating whether the given encryption method is supported by the current compilation of the ZipArchive Library.

		\param iEncryptionMethod
			The encryption method to test. It can be one of the #EncryptionMethod values.

		\return
			\c true, if the method is supported; \c false otherwise.
	*/
	static bool IsEncryptionSupported(int iEncryptionMethod)
	{		
		return iEncryptionMethod == encStandard;
	}

	/**
		The method called when an existing file is opened for extraction.
	
		\param password
			The supplied password with the CZipArchive::SetPassword method.
	
		\param currentFile
			The file being decoded and extracted.
	
		\param storage
			The current CZipStorage.
	
		\return
			\c true, if the password is initially considered correct; \c false otherwise.	
	 */
	virtual bool InitDecode(CZipAutoBuffer& password, CZipFileHeader& currentFile, CZipStorage& storage) = 0;

	/**
		The method called when a new file is opened for compression.
		
		\param password
			The supplied password with the CZipArchive::SetPassword method.
		
		\param currentFile
			The file being compressed and encoded.
		
		\param storage
			The current CZipStorage.		
	 */
	virtual void InitEncode(CZipAutoBuffer& password, CZipFileHeader& currentFile, CZipStorage& storage) = 0;

	/**
		Decodes the given data.
	
		\param pBuffer
			The buffer that holds the data to decode and that receives the results.
	
		\param uSize
			The size of \a pBuffer.	
	 */
	virtual void Decode(char* pBuffer, DWORD uSize) = 0;

	/**
		Encodes the given data.
	
		\param pBuffer
			The buffer that holds the data to encode and that receives the results.
	
		\param uSize
			The size of \a pBuffer.	
	 */
	virtual void Encode(char* pBuffer, DWORD uSize) = 0;

	/**
		The method called at the end of the decoding process.
	
		\param currentFile
			The file being decoded and extracted.
	
		\param storage
			The current CZipStorage.
	 */
	virtual void FinishDecode(CZipFileHeader& currentFile, CZipStorage& storage){};

	/**
		The method called at the end of the decoding process.
	
		\param currentFile
			The file being compressed and encoded.
	
		\param storage
			The current CZipStorage.
	 */
	virtual void FinishEncode(CZipFileHeader& currentFile, CZipStorage& storage){};

	/**
		Returns the value indicating whether the current compressor can handle the given encryption method.

		\param iEncryptionMethod
			The encryption method to test. It can be one of the #EncryptionMethod values.

		\return 
			\c true, if the current compressor can handle the given encryption method; \c false otherwise.
	*/
	virtual bool CanHandle(int iEncryptionMethod)
	{
		return false;
	}
	virtual ~CZipCryptograph(){}
};

#if _MSC_VER > 1000
	#pragma warning( pop )
#endif // _MSC_VER > 1000

#endif
