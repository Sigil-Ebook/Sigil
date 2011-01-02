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
#include "ZipExtraField.h"
CZipArray<WORD> CZipExtraField::m_aNoSizeExtraHeadersID;

bool CZipExtraField::Read(CZipStorage *pStorage, WORD uSize)
{
	if (uSize == 0)
		return true;
	Clear();
	CZipAutoBuffer buffer;
	buffer.Allocate(uSize);
	pStorage->Read(buffer, uSize, true);
	char* position = (char*) buffer;
	do
	{		
		CZipExtraData* pExtra = new CZipExtraData();
		if (!pExtra->Read(position, uSize))
		{
			delete pExtra;
			return false;
		}
		int totalSize = pExtra->GetTotalSize();
		if (totalSize > uSize || totalSize < 0)
			return false;
		position += totalSize;
		uSize = (WORD)(uSize - totalSize);
		Add(pExtra);
	}
	while (uSize > 0);
	return true;
}


void CZipExtraField::Write(char* buffer)const
{
	int offset = 0;
	for (int i = 0; i < GetCount(); i++)
	{	
		offset += GetAt(i)->Write(buffer + offset);
	}
}

int CZipExtraField::GetTotalSize()const
{
	int total = 0;
	for (int i = 0; i < GetCount(); i++)
		total += GetAt(i)->GetTotalSize();
	return total;
}

void CZipExtraField::RemoveInternalHeaders()
{
	for (int i = GetCount() - 1; i >= 0; i--)
	{
		WORD headerID = GetAt(i)->GetHeaderID();
		if (headerID == ZIP_EXTRA_PKZIP 
			|| headerID == ZIP_EXTRA_WINZIP_AES
			|| headerID == ZIP_EXTRA_UNICODE_PATH
			|| headerID == ZIP_EXTRA_UNICODE_COMMENT
			|| headerID == ZIP_EXTRA_ZARCH_NAME)
				RemoveAt(i);
	}
}

void CZipExtraField::RemoveInternalLocalHeaders()
{
	for (int i = GetCount() - 1; i >= 0; i--)
	{
		WORD headerID = GetAt(i)->GetHeaderID();
		if (headerID == ZIP_EXTRA_WINZIP_AES
			|| headerID == ZIP_EXTRA_UNICODE_PATH
			|| headerID == ZIP_EXTRA_UNICODE_COMMENT)
				RemoveAt(i);
	}
}

void CZipExtraField::Remove(WORD headerID)
{
	for (int i = GetCount() - 1; i >= 0; i--)
	{
		if (headerID == GetAt(i)->GetHeaderID())
			RemoveAt(i);
	}
}

CZipExtraData* CZipExtraField::Lookup(WORD headerID, int& index) const
{
	// we can do a non-efficient search here
	// usually the number of extra fields is low, if any
	for (int i = 0; i < GetCount(); i++)
	{
		CZipExtraData* pExtra = GetAt(i);
		if (pExtra->m_uHeaderID == headerID)
		{
			index = i;
			return pExtra;
		}
	}
	return NULL;
}

