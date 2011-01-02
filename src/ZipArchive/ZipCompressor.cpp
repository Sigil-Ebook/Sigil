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
#include "ZipCompressor.h"
#include "BytesWriter.h"
#include "DeflateCompressor.h"

using namespace ZipArchiveLib;

CZipCompressor* CZipCompressor::CreateCompressor(WORD uMethod, CZipStorage* pStorage)
{
	if (uMethod == methodStore || uMethod == methodDeflate)
		return new CDeflateCompressor(pStorage);

	return NULL;
}

void CZipCompressor::UpdateFileCrc(const void *pBuffer, DWORD uSize)
{
	m_pFile->m_uCrc32 = zarch_crc32(m_pFile->m_uCrc32, (zarch_Bytef*)pBuffer, uSize);
}

void CZipCompressor::UpdateCrc(const void *pBuffer, DWORD uSize)
{
	m_uCrc32 = zarch_crc32(m_uCrc32, (zarch_Bytef*)pBuffer, uSize);
}

void CZipCompressor::UpdateOptions(const COptionsMap& optionsMap)
{
	const COptions* pOptions = GetOptions();
	if (pOptions == NULL)
		return;
	const COptions* pNewOptions = optionsMap.Get(pOptions->GetType());
	if (pNewOptions != NULL)
		UpdateOptions(pNewOptions);
}

void CZipCompressor::InitBuffer()
{
	// This should be greater that 64k for deflate when creating offsets pairs is enabled
	// otherwise deflate will not be able to write one block in one go and will never report
	// a flushed block for low-compressable data
	const COptions* pOptions = GetOptions();
	DWORD bufferSize = 0;
	if (pOptions != NULL)
        bufferSize = pOptions->m_iBufferSize;
	if (bufferSize == 0)
		bufferSize = COptions::cDefaultBufferSize;
	m_pBuffer.Allocate(bufferSize);
}


void CZipCompressor::COptionsMap::Set(const CZipCompressor::COptions* pOptions)
{
	if (pOptions == NULL)
		return;
	int iType = pOptions->GetType();
	Remove(iType);
	SetAt(iType, pOptions->Clone());
}

CZipCompressor::COptions* CZipCompressor::COptionsMap::Get(int iType) const
{
	COptions* pTemp = NULL;
	if (Lookup(iType, pTemp))
		return pTemp;
	else
		return NULL;
}

void CZipCompressor::COptionsMap::Remove(int iType)
{
	COptions* pTemp = Get(iType);
	if (pTemp != NULL)
	{
		delete pTemp;
		RemoveKey(iType);
	}	
}

CZipCompressor::COptionsMap::~COptionsMap()
{
	COptionsMap::iterator iter = GetStartPosition();
	while (IteratorValid(iter))
	{
		COptions* pOptions = NULL;
		int iDummyType;
		GetNextAssoc(iter, iDummyType, pOptions);
		delete pOptions;
	}
	RemoveAll();
}

