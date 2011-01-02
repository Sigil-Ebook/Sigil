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
* \file ZipCompressor.h
* Includes the CZipCompressor class.
*
*/

#if !defined(ZIPARCHIVE_ZIPCOMPRESSOR_DOT_H)
#define ZIPARCHIVE_ZIPCOMPRESSOR_DOT_H

#if _MSC_VER > 1000
	#pragma once
	#pragma warning( push )
	#pragma warning (disable: 4100) // unreferenced formal parameter
	#pragma warning (disable: 4275) // non dll-interface class 'CObject' used as base for dll-interface class 'CMap<KEY,ARG_KEY,VALUE,ARG_VALUE>'
#endif

#include "ZipExport.h"
#include "ZipAutoBuffer.h"
#include "ZipFileHeader.h"
#include "ZipStorage.h"
#include "ZipCryptograph.h"
#include "ZipException.h"

/**
	A base class for compressors used in compression and decompression of data.
*/
class ZIP_API CZipCompressor
{
protected:
	CZipStorage* m_pStorage;			///< The current storage object.
	CZipAutoBuffer m_pBuffer;			///< A buffer that receives compressed data or provides data for decompression.
	CZipCryptograph* m_pCryptograph;	///< The current cryptograph.
	CZipFileHeader* m_pFile;			///< The file header being compressed or decompressed.

	
	/**
		Initializes a new instance of the CZipCompressor class.

		\param pStorage
			The current storage object.
	 */
	CZipCompressor(CZipStorage* pStorage)
		:m_pStorage(pStorage)
	{
		m_pCryptograph = NULL;
		m_uUncomprLeft = 0;
		m_uComprLeft  = 0;
		m_uCrc32 = 0;
	}
	
public:		
	/**
		The type of a compressor.
	*/
	enum CompressorType
	{
		typeDeflate = 1,	///< Deflate compression (default in zip archives).
		typeBzip2,			///< Bzip2 compression.
		typePPMd			///< PPMd compression
	};

	/**
		The compression level.
	*/
	enum CompressionLevel
	{		
		levelDefault = -1,	///< The default compression level (equals \c 6 for deflate).
		levelStore = 0,		///< No compression used. Data is stored.
		levelFastest = 1,	///< The fastest compression. The compression ratio is the lowest (apart from #levelStore).
		levelBest = 9		///< The highest compression ratio. It's usually the slowest one.
	};

	/**
		The compression method.
	*/
	enum CompressionMethod
	{
		methodStore = 0, ///< A file is stored, not compressed.
		methodDeflate = 8, ///< The deflate compression method.
		/**
			The bzip2 compression method.

			\see
				<a href="kb">0610231446|bzip2</a>				
		*/
		methodBzip2 = 12,

		/**
			This value means that WinZip AES encryption is used.
			The original compression method is stored in a WinZip extra field.
			It is only an informational value - you cannot set it as a compression method. The ZipArchive 
			Library handles this value internally.

			\see
				<a href="kb">0610201627|aes</a>
		*/
		methodWinZipAes = 99
	};

	/**
		The base class for compressors options.

		\see
			<a href="kb">0610231446|options</a>
		\see
			CZipArchive::SetCompressionOptions
	*/
	struct ZIP_API COptions
	{

		/**	  
			  Helper constants.
		*/
		enum Constants
		{
			/**
				The default size of the buffer used in compression and decompression operations.
			*/
			cDefaultBufferSize = 2 * 65536
		};

		COptions()
		{
			m_iBufferSize = cDefaultBufferSize;
		}

		/**
			Returns the type of the compressor to which the current options apply.

			\return
				The type of the compressor. It can be one of the #CompressorType values.
		*/
		virtual int GetType() const = 0;

		/**
			Clones the current options object.

			\return 
				The cloned object of the same type as the current object.
		*/
		virtual COptions* Clone() const = 0;

		/**
			The size of the buffer used in compression and decompression operations. 
			By default it is set to #cDefaultBufferSize. For the optimal performance of the 
			deflate algorithm it should be set at least to 128kB.

			\see
				CZipArchive::SetAdvanced
		*/
		int m_iBufferSize;
		virtual ~COptions()
		{
		}
	};


	/**
		A dictionary used for keeping options for different types of compressors.

		\see
			CZipArchive::SetCompressionOptions
	*/
	class ZIP_API COptionsMap : public CZipMap<int, COptions*>
	{
		public:
			void Set(const COptions* pOptions);
			void Remove(int iType);
			COptions* Get(int iType) const; 
			~COptionsMap();
	};

	/**
		Returns the value indicating whether the given compression method is supported by the ZipArchive Library.
		
		\param uCompressionMethod
			The compression method. It can be one of the #CompressionMethod values.

		\return 
			\c true, if the compression method is supported, \c false otherwise.
	*/
	static bool IsCompressionSupported(WORD uCompressionMethod)
	{		
		return uCompressionMethod == methodStore || uCompressionMethod == methodDeflate
			;
	}

	ZIP_SIZE_TYPE m_uUncomprLeft;	///< The number of bytes left to decompress.
	ZIP_SIZE_TYPE m_uComprLeft;		///< The number of bytes left to compress.
	DWORD m_uCrc32;	///< The CRC32 file checksum.	

	/**
		Returns the value indicating whether the current #CZipCompressor object supports the given compression method.
		
		\param uMethod
			The compression method. It can be one of the #CompressionMethod values.

		\return 
			\c true, if the compression method is supported; \c false otherwise.
			
	*/
	virtual bool CanProcess(WORD uMethod) = 0;


	/**
		The method called when a new file is opened for compression.
		
		\param iLevel
			The compression level.
		
		\param pFile
			The file being compressed.
		
		\param pCryptograph
			The current CZipCryptograph. It can be \c NULL, if no encryption is used.

		\see
			Compress
		\see 
			FinishCompression
	 */
	virtual void InitCompression(int iLevel, CZipFileHeader* pFile, CZipCryptograph* pCryptograph)	
	{
		InitBuffer();
		m_uComprLeft = 0;
		m_pFile = pFile;
		m_pCryptograph = pCryptograph;
	}

	/**
		The method called when a new file is opened for extraction.
		
		\param pFile
			The file being extracted.
		
		\param pCryptograph
			The current CZipCryptograph. It can be \c NULL, if no decryption is used.

		\see
			Decompress
		\see 
			FinishDecompression
	 */
	virtual void InitDecompression(CZipFileHeader* pFile, CZipCryptograph* pCryptograph)
	{
		InitBuffer();
		m_pFile = pFile;
		m_pCryptograph = pCryptograph;

		m_uComprLeft = m_pFile->GetDataSize(true);
		m_uUncomprLeft = m_pFile->m_uUncomprSize;
		m_uCrc32 = 0;
	}

	/**
		Compresses the given data.
	
		\param pBuffer
			The buffer that holds the data to compress.
	
		\param uSize
			The size of \a pBuffer.	

		\see
			InitCompression
		\see 
			FinishCompression
	 */
	virtual void Compress(const void *pBuffer, DWORD uSize) = 0;

	/**
		Decompresses the given data.
	
		\param pBuffer
			The buffer that receives the decompressed data.
	
		\param uSize
			The size of \a pBuffer.	

		\return 
			The number of bytes decompressed and written to \a pBuffer.

		\note
			This method should be called repeatedly until it returns \c 0.

		\see
			InitDecompression
		\see 
			FinishDecompression
	 */
	virtual DWORD Decompress(void *pBuffer, DWORD uSize) = 0;

	/**
		The method called at the end of the compression process.
	
		\param bAfterException
			Set to \c true, if an exception occurred before or to \c false otherwise.

		\see
			InitCompression
		\see 
			Compress
	 */
	virtual void FinishCompression(bool bAfterException){}

	/**
		The method called at the end of the decompression process.
	
		\param bAfterException
			Set to \c true, if an exception occurred before or to \c false otherwise.

		\see
			InitDecompression
		\see 
			Decompress
	 */
	virtual void FinishDecompression(bool bAfterException){}

	/**
		Returns the current options of the compressor.

		\return
			The current options for the compressor.

		\see
			<a href="kb">0610231446|options</a>
		\see
			CZipArchive::SetCompressionOptions
		\see 
			UpdateOptions
	*/
	virtual const COptions* GetOptions() const
	{
		return NULL;
	}

	/**
		Updates the current options with the options stored in \a optionsMap,
		if the appropriate options are present in the map.

		\param optionsMap
			The map to get the new options from.

		\see
			<a href="kb">0610231446|options</a>
		\see
			GetOptions		
	*/
	void UpdateOptions(const COptionsMap& optionsMap);


	virtual ~CZipCompressor()
	{
	}

	/**
		A factory method that creates an appropriate compressor for the given compression method.

		\param uMethod
			The compression method to create a compressor for. It can be one of the #CompressionMethod values.

		\param pStorage
			The current storage object.
	*/
	static CZipCompressor* CreateCompressor(WORD uMethod, CZipStorage* pStorage);

	
protected:
	/**
		Updates the current options with the new options.

		\param pOptions
			The new options to apply.
	*/
	virtual void UpdateOptions(const COptions* pOptions)
	{		
	}
	/**
		Updates CRC value while compression. 

		\param pBuffer
			A buffer with data for which the CRC value should be updated.

		\param uSize
			The size of the buffer.
	*/
	void UpdateFileCrc(const void *pBuffer, DWORD uSize);

	/**
		Updates CRC value while decompression. 

		\param pBuffer
			A buffer with data for which the CRC value should be updated.

		\param uSize
			The size of the buffer.
	*/
	void UpdateCrc(const void *pBuffer, DWORD uSize);

	/**
		Flushes data in the buffer into the storage, encrypting the data if needed.

	*/
	void FlushWriteBuffer()
	{
		WriteBuffer(m_pBuffer, (DWORD)m_uComprLeft);
		m_uComprLeft = 0;
	}

	/**
		Writes the buffer into the storage, encrypting the data if needed.

		\param pBuffer
			The buffer with data to write.

		\param uSize
			The size of the buffer.
	*/
	void WriteBuffer(char* pBuffer, DWORD uSize)
	{
		if (uSize == 0)
			return;
		if (m_pCryptograph)
			m_pCryptograph->Encode(pBuffer, uSize);
		m_pStorage->Write(pBuffer, uSize, false);		
	}

	/**
		Fills the read buffer.

		\return
			The number of bytes read.
	*/
	DWORD FillBuffer()
	{
		DWORD uToRead = m_pBuffer.GetSize();
		if (m_uComprLeft < uToRead)
			uToRead = (DWORD)m_uComprLeft;
		
		if (uToRead > 0)
		{
			m_pStorage->Read(m_pBuffer, uToRead, false);
			if (m_pCryptograph)
				m_pCryptograph->Decode(m_pBuffer, uToRead);
			m_uComprLeft -= uToRead;
		}
		return uToRead;
	}

	/**
		Initializes the internal buffer.

		\see
			ReleaseBuffer
	*/
	void InitBuffer();

	/**
		Releases the internal buffer.

		\see
			InitBuffer
	*/
	void ReleaseBuffer()
	{
		m_pBuffer.Release();
	}

	/**
		Converts an internal error code of the compressor to the ZipArchive Library error code.

		\param iErr
			An internal error code.

		\return
			A ZipArchive Library error code.
	*/
	virtual int ConvertInternalError(int iErr) const
	{
		return iErr;
	}

	/**
		Throws an exception with a given error code.

		\param iErr
			An error code.

		\param bInternal
			\c true, if \a iErr is an internal error code and needs a conversion to the ZipArchive Library error code; \c false otherwise.

		
		\see
			ConvertInternalError
	*/
	void ThrowError(int iErr, bool bInternal = false)
	{
		if (bInternal)
			iErr = ConvertInternalError(iErr);
		CZipException::Throw(iErr, m_pStorage->IsClosed(true) ? _T("") : (LPCTSTR)m_pStorage->m_pFile->GetFilePath());
	}
};

#if _MSC_VER > 1000
	#pragma warning( pop )
#endif

#endif
