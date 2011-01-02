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
* \file DeflateCompressor.h
*	Includes the ZipArchiveLib::CDeflateCompressor class.
*
*/

#if !defined(ZIPARCHIVE_DEFLATECOMPRESSOR_DOT_H)
#define ZIPARCHIVE_DEFLATECOMPRESSOR_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "ZipExport.h"
#include "BaseLibCompressor.h"
#include "ZipException.h"
#include "zlib/zlib.h"

namespace ZipArchiveLib
{

/**
	Compresses and decompresses data using the Zlib library.
*/
class ZIP_API CDeflateCompressor : public CBaseLibCompressor
{			
public:	
	/**
		Represents options of the CDeflateCompressor.

		\see
			<a href="kb">0610231446|options</a>
		\see
			CZipArchive::SetCompressionOptions
	*/
	struct ZIP_API COptions : CBaseLibCompressor::COptions
	{
		COptions()
		{
			m_bCheckLastBlock = true;
		}

		int GetType() const
		{
			return typeDeflate;
		}

		CZipCompressor::COptions* Clone() const
		{
			return new COptions(*this);
		}

		/**
			Enables or disables checking, if the compressed data ends with an end-of-stream block.
			This should be enabled to protect against malformed data.
			\c true, if the checking of the last block should be enabled; \c false otherwise.
		*/
		bool m_bCheckLastBlock;	

	};	

	/**
		Initializes a new instance of the CDeflateCompressor class.

		\param pStorage
			The current storage object.
	*/
	CDeflateCompressor(CZipStorage* pStorage);

	bool CanProcess(WORD uMethod) {return uMethod == methodStore || uMethod == methodDeflate;}

	void InitCompression(int iLevel, CZipFileHeader* pFile, CZipCryptograph* pCryptograph);
	void InitDecompression(CZipFileHeader* pFile, CZipCryptograph* pCryptograph);

	DWORD Decompress(void *pBuffer, DWORD uSize);
	void Compress(const void *pBuffer, DWORD uSize);
	
	void FinishCompression(bool bAfterException);
	void FinishDecompression(bool bAfterException);


	const CZipCompressor::COptions* GetOptions() const
	{
		return &m_options;
	}

	~CDeflateCompressor()
	{
	}
protected:
	void UpdateOptions(const CZipCompressor::COptions* pOptions)
	{
		m_options = *(COptions*)pOptions;
	}
	
	int ConvertInternalError(int iErr) const
	{
		switch (iErr)
		{
		case Z_NEED_DICT:
			return CZipException::needDict;
		case Z_STREAM_END:
			return CZipException::streamEnd;
		case Z_ERRNO:
			return CZipException::errNo;
		case Z_STREAM_ERROR:
			return CZipException::streamError;
		case Z_DATA_ERROR:
			return CZipException::dataError;
		case Z_MEM_ERROR:
			return CZipException::memError;
		case Z_BUF_ERROR:
			return CZipException::bufError;
		case Z_VERSION_ERROR:
			return CZipException::versionError;
		default:
			return CZipException::genericError;
		}
	}

	bool IsCodeErrorOK(int iErr) const
	{
		return iErr == Z_OK || iErr == Z_NEED_DICT;
	}

private:			
	COptions m_options;
	zarch_z_stream m_stream;
};

} // namespace

#endif

