////////////////////////////////////////////////////////////////////////////////
// This source file is part of the ZipArchive library source distribution and
// is Copyrighted 2000 - 2009 by Artpol Software - Tadeusz Dracz
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
#include "DeflateCompressor.h"
#include "zlib/deflate.h"

namespace ZipArchiveLib
{

#ifndef DEF_MEM_LEVEL
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
#endif


CDeflateCompressor::CDeflateCompressor(CZipStorage* pStorage)
	:CBaseLibCompressor(pStorage)
{
	m_stream.zalloc = (zarch_alloc_func)_zipalloc;
	m_stream.zfree = (zarch_free_func)_zipfree;	
}


void CDeflateCompressor::InitCompression(int iLevel, CZipFileHeader* pFile, CZipCryptograph* pCryptograph)
{
	CZipCompressor::InitCompression(iLevel, pFile, pCryptograph);

	m_stream.avail_in = (zarch_uInt)0;
	m_stream.avail_out = (zarch_uInt)m_pBuffer.GetSize();
	m_stream.next_out = (zarch_Bytef*)(char*)m_pBuffer;
	m_stream.total_in = 0;
	m_stream.total_out = 0;
	
	if (pFile->m_uMethod == methodDeflate)
	{
		SetOpaque(&m_stream.opaque, &m_options);
		
		int err = zarch_deflateInit2_(&m_stream, iLevel,
			Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof(zarch_z_stream));
		
		CheckForError(err);
	}	
}

void CDeflateCompressor::Compress(const void *pBuffer, DWORD uSize)
{ 
	m_stream.next_in = (zarch_Bytef*)pBuffer;
	m_stream.avail_in = uSize;
	UpdateFileCrc(pBuffer, uSize);
	
	
	while (m_stream.avail_in > 0)
	{
		if (m_stream.avail_out == 0)
		{
			FlushWriteBuffer();
			m_stream.avail_out = m_pBuffer.GetSize();
			m_stream.next_out = (zarch_Bytef*)(char*)m_pBuffer;
		}
		
		if (m_pFile->m_uMethod == methodDeflate)
		{
			ZIP_ZLIB_TYPE uTotal = m_stream.total_out;
			CheckForError(zarch_deflate(&m_stream,  Z_NO_FLUSH));
			m_uComprLeft += m_stream.total_out - uTotal;
		}
		else
		{
			DWORD uToCopy = (m_stream.avail_in < m_stream.avail_out) 
				? m_stream.avail_in : m_stream.avail_out;
			
			memcpy(m_stream.next_out, m_stream.next_in, uToCopy);
			
			m_stream.avail_in -= uToCopy;
			m_stream.avail_out -= uToCopy;
			m_stream.next_in += uToCopy;
			m_stream.next_out += uToCopy;
			m_stream.total_in += uToCopy;
			m_stream.total_out += uToCopy;
			m_uComprLeft += uToCopy;
		}
	}
}

void CDeflateCompressor::FinishCompression(bool bAfterException)
{
	m_stream.avail_in = 0;
	if (!bAfterException)
	{
		if (m_pFile->m_uMethod == methodDeflate)
		{
			int err;
			do
			{
				if (m_stream.avail_out == 0)
				{
					FlushWriteBuffer();
					m_stream.avail_out = m_pBuffer.GetSize();
					m_stream.next_out = (zarch_Bytef*)(char*)m_pBuffer;
				}
				ZIP_SIZE_TYPE uTotal = m_stream.total_out;
				err = zarch_deflate(&m_stream,  Z_FINISH);
				m_uComprLeft += m_stream.total_out - uTotal;
			}
			while (err == Z_OK);
			
			if (err == Z_STREAM_END)
				err = Z_OK;
			CheckForError(err);
		}
		
		if (m_uComprLeft > 0)
			FlushWriteBuffer();
		
		if (m_pFile->m_uMethod == methodDeflate)
			CheckForError(zarch_deflateEnd(&m_stream));
		
		
		// it may be increased by the encrypted header size in CZipFileHeader::PrepareData
		m_pFile->m_uComprSize += m_stream.total_out;
		m_pFile->m_uUncomprSize = m_stream.total_in;
	}
	EmptyPtrList();
	ReleaseBuffer();
}

void CDeflateCompressor::InitDecompression(CZipFileHeader* pFile, CZipCryptograph* pCryptograph)
{
	CBaseLibCompressor::InitDecompression(pFile, pCryptograph);
	if (m_pFile->m_uMethod == methodDeflate)
	{
		SetOpaque(&m_stream.opaque, &m_options);
		CheckForError(zarch_inflateInit2_(&m_stream, -MAX_WBITS, ZLIB_VERSION, sizeof(zarch_z_stream)));
	}
	m_stream.total_out = 0;
	m_stream.avail_in = 0;
}

DWORD CDeflateCompressor::Decompress(void *pBuffer, DWORD uSize)
{
	if (m_bDecompressionDone)
		return 0;

	m_stream.next_out = (zarch_Bytef*)pBuffer;
	m_stream.avail_out = uSize > m_uUncomprLeft 
		? (DWORD)m_uUncomprLeft : uSize;
			
	DWORD uRead = 0;

	// may happen when the file is 0 sized
	bool bForce = m_stream.avail_out == 0 && m_uComprLeft > 0;
	while (m_stream.avail_out > 0 || (bForce && m_uComprLeft > 0))
	{
		if ((m_stream.avail_in == 0) &&
			(m_uComprLeft >= 0)) // Also when there are zero bytes left
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
			
			m_stream.next_in = (zarch_Bytef*)(char*)m_pBuffer;
			m_stream.avail_in = uToRead;
		}
		
		if (m_pFile->m_uMethod == methodStore)
		{
			DWORD uToCopy = m_stream.avail_out < m_stream.avail_in 
				? m_stream.avail_out : m_stream.avail_in;
			
			memcpy(m_stream.next_out, m_stream.next_in, uToCopy);
			
			UpdateCrc(m_stream.next_out, uToCopy);
			
			m_uUncomprLeft -= uToCopy;
			m_stream.avail_in -= uToCopy;
			m_stream.avail_out -= uToCopy;
			m_stream.next_out += uToCopy;
			m_stream.next_in += uToCopy;
			m_stream.total_out += uToCopy;
			uRead += uToCopy;
		}
		else
		{
			ZIP_SIZE_TYPE uTotal = m_stream.total_out;
			zarch_Bytef* pOldBuf =  m_stream.next_out;
			int ret = zarch_inflate(&m_stream, Z_SYNC_FLUSH);
			// will not exceed DWORD
			DWORD uToCopy = (DWORD)(m_stream.total_out - uTotal);
			
			UpdateCrc(pOldBuf, uToCopy);
			
			m_uUncomprLeft -= uToCopy;
			uRead += uToCopy;
            
			if (ret == Z_STREAM_END)
			{
				m_bDecompressionDone = true;
				return uRead;
			}
			else			
				CheckForError(ret);
		}
	}

	if (!uRead && m_options.m_bCheckLastBlock && uSize != 0 && m_pFile->m_uMethod == methodDeflate)
	{
		if (zarch_inflate(&m_stream, Z_SYNC_FLUSH) != Z_STREAM_END)
			// there were no more bytes to read and there was no ending block, 
			// otherwise the method would return earlier
			ThrowError(CZipException::badZipFile);
	}
	
	return uRead;
}

void CDeflateCompressor::FinishDecompression(bool bAfterException)
{	
	if (!bAfterException && m_pFile->m_uMethod == methodDeflate)
		zarch_inflateEnd(&m_stream);
	EmptyPtrList();
	ReleaseBuffer();
}


} // namespace
