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
#include "ZipCrc32Cryptograph.h"

bool CZipCrc32Cryptograph::InitDecode(CZipAutoBuffer& password, CZipFileHeader& currentFile, CZipStorage& storage)
{
	CryptInitKeys(password);
	CZipAutoBuffer buf(ZIPARCHIVE_ENCR_HEADER_LEN);
	storage.Read(buf, ZIPARCHIVE_ENCR_HEADER_LEN, false);
	BYTE b = 0;
	for (int i = 0; i < ZIPARCHIVE_ENCR_HEADER_LEN; i++)
	{
		b = buf[i]; // only temporary
		CryptDecode((char&)b);
	}
	// check the last byte
	return currentFile.IsDataDescriptor() ?
		(BYTE(currentFile.m_uModTime >> 8) == b) : (BYTE(currentFile.m_uCrc32 >> 24) == b);
}

void CZipCrc32Cryptograph::InitEncode(CZipAutoBuffer& password, CZipFileHeader& currentFile, CZipStorage& storage)
{
	CZipAutoBuffer buf(ZIPARCHIVE_ENCR_HEADER_LEN);
	// use pseudo-crc since we don't know it yet	
	CryptInitKeys(password);
	srand(UINT(time(NULL)));
	// generate pseudo-random sequence
	char c;
	char* buffer = (char*)buf;
	for (int i = 0; i < ZIPARCHIVE_ENCR_HEADER_LEN - 2; i++)
	{
		int t1 = rand();
		c = (char)((t1 >> 6) & 0xFF);
		if (!c)
			c = (char)(t1 & 0xFF);
		CryptEncode(c);
		buffer[i] = c;

	}
	int iCrc = (int)currentFile.m_uModTime << 16;	
	c = (char)((iCrc >> 16) & 0xFF);
	CryptEncode(c);
	buffer[ZIPARCHIVE_ENCR_HEADER_LEN - 2] = c;
	c = (char)((iCrc >> 24) & 0xFF);
	CryptEncode(c);
	buffer[ZIPARCHIVE_ENCR_HEADER_LEN - 1] = c;
	storage.Write(buf, ZIPARCHIVE_ENCR_HEADER_LEN, false);
	currentFile.m_uComprSize += ZIPARCHIVE_ENCR_HEADER_LEN;
}

void CZipCrc32Cryptograph::CryptInitKeys(CZipAutoBuffer& password)
{
	m_keys[0] = 305419896L;
	m_keys[1] = 591751049L;
	m_keys[2] = 878082192L;
	for (DWORD i = 0; i < password.GetSize(); i++)
		CryptUpdateKeys(password[i]);
}

void CZipCrc32Cryptograph::CryptUpdateKeys(char c)
{	
	m_keys[0] = CryptCRC32(m_keys[0], c);
	m_keys[1] += m_keys[0] & 0xff;
	m_keys[1] = m_keys[1] * 134775813L + 1;
	c = char(m_keys[1] >> 24);
	m_keys[2] = CryptCRC32(m_keys[2], c);
}

