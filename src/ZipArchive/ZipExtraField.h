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
* \file ZipExtraField.h
* Includes the CZipExtraField class.
*
*/

#if !defined(ZIPARCHIVE_ZIPEXTRAFIELD_DOT_H)
#define ZIPARCHIVE_ZIPEXTRAFIELD_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "ZipExport.h"
#include "ZipExtraData.h"
#include "ZipCollections.h"
#include "ZipStorage.h"

#define ZIP_EXTRA_PKZIP 0x0001

#define ZIP_EXTRA_ZARCH_NAME 0x5A4C // ZL - ZipArchive Library

#define ZIP_EXTRA_ZARCH_SEEK 0x5A4D 

#define ZIP_EXTRA_WINZIP_AES 0x9901

#define ZIP_EXTRA_UNICODE_PATH 0x7075
#define ZIP_EXTRA_UNICODE_COMMENT 0x6375


#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
#endif

/**
	Represents a local or central extra field in a zip archive.
	This is a collection of extra data records (CZipExtraData).

	\see
		<a href="kb">0610242300</a>
	\see
		CZipExtraData
*/
class ZIP_API CZipExtraField
{
	friend class CZipFileHeader;
public:
	CZipExtraField()
	{
	}
	CZipExtraField(const CZipExtraField& arr)
	{		
		*this = arr;
	}
	CZipExtraField& operator=(const CZipExtraField& field)
	{
		Clear();
		for (int i = 0; i < field.GetCount(); i++)
			Add(new CZipExtraData(*field.GetAt(i)));
		return *this;
	}

	/**
		Returns the total size the extra data will occupy in the archive.

		\return
			The size in bytes.
	*/
	int GetTotalSize() const;

	/**
		Validates the current size of the extra field.

		\return
			\c false, if the size is larger than allowed; \c false otherwise.
	*/
	bool Validate() const
	{
		return GetTotalSize() <= (int)USHRT_MAX;
	}
	
	/**
		Returns the number of extra data records included in the extra field.

		\return
			The number of extra fields included.
	*/
	int GetCount() const
	{
		return (int)m_aData.GetSize();
	}

	/**
		Returns the extra data record at the given index.

		\param index
			The index of extra data record to retrieve.

		\return
			The extra data record.
	*/
	CZipExtraData* GetAt(int index) const
	{		
		return m_aData.GetAt(index);
	}

	/**
		Removes the extra data record at the given index.

		\param index
			The index of the extra data record to remove.
	*/
	void RemoveAt(int index)
	{
		delete (GetAt(index));
		m_aData.RemoveAt(index);
	}

	/**
		Adds a new extra data record to the extra field.

		\param pExtra
			The extra data record to add.

		\return
			The index of \a pExtra in the internal collection.
	*/
	int Add(CZipExtraData* pExtra)
	{
		return (int)m_aData.Add(pExtra);
	}

	/**
		Creates a new extra data record with the given ID
		and adds it to the extra field.

		\param headerID
			The extra data ID.

		\return
			The created extra data record.

		\see
			CZipExtraData::GetHeaderID
	*/
	CZipExtraData* CreateNew(WORD headerID)
	{
		int temp;
		return CreateNew(headerID, temp);
	}

	/**
		Creates a new extra data record with the given ID
		and adds it to the extra field.

		\param headerID
			The extra data ID.

		\param idx
			Receives the value of the index of the new 
			extra data in the internal collection.

		\return
			The created extra data record.

		\see
			CZipExtraData::GetHeaderID
	*/
	CZipExtraData* CreateNew(WORD headerID, int& idx)
	{
		CZipExtraData* pData = new CZipExtraData(headerID);
		pData->m_bHasSize = HasSize(headerID);
		idx = (int)m_aData.Add(pData);
		return pData;
	}

	/**
		Removes all extra data records from the central extra field
		that are internally used by the ZipArchive Library.
	*/
	void RemoveInternalHeaders();

	/**
		Removes all extra data records from the local extra field
		that are internally used by the ZipArchive Library.
	*/
	void RemoveInternalLocalHeaders();

	/**
		Removes the extra data with the given ID.

		\param headerID
			The ID of the extra data to remove.
	*/
	void Remove(WORD headerID);

	/**
		Searches the extra field for the extra data record with the given ID.

		\param headerID
			The ID of the extra data to search.

		\return
			The found extra data record or \c NULL, if the extra data could not be found.
	*/
	CZipExtraData* Lookup(WORD headerID) const
	{
		int temp;
		return Lookup(headerID, temp);
	}

	/**
		Returns the value indicating whether the extra data record with the given ID is present in the extra field.

		\param headerID
			The ID of the extra data to check.

		\return 
			\c true, if the extra data record with the given ID is present in the extra field; \c false otherwise.
	*/
	bool HasHeader(WORD headerID)
	{
		return Lookup(headerID) != NULL;
	}

	/**
		Searches the extra field for the extra data record with the given ID.

		\param headerID
			The ID of the extra data to search.

		\param index
			Receives the value of the index of the found 
			extra data in the internal collection.

		\return
			The found extra data record or \c NULL, if the extra data could not be found.
	*/
	CZipExtraData* Lookup(WORD headerID, int& index) const;
	~CZipExtraField()
	{
		Clear();
	}

	/**
		An array of headers that do not write extra data size.

		\see
			<a href="kb">0610242300|read</a>
	*/
	static CZipArray<WORD> m_aNoSizeExtraHeadersID;

	/**
		Returns the value indicating whether the extra data record with the given ID writes its size.

		\param headerID
			The ID of extra data to examine.

		\return
			\c true, if the extra data record writes its size; \c false otherwise.

		\see
			m_aNoSizeExtraHeadersID			
	*/
	static bool HasSize(WORD headerID)
	{
		ZIP_ARRAY_SIZE_TYPE size = m_aNoSizeExtraHeadersID.GetSize();
		for (ZIP_ARRAY_SIZE_TYPE i = 0; i < size; i++)
		{
			if (m_aNoSizeExtraHeadersID.GetAt(i) == headerID)
				return false;
		}
		return true;
	}

protected:

	/**
		Removes all extra data records from the extra field.
	*/
	void Clear()
	{
		for (int i = 0; i < GetCount(); i++)
			delete (GetAt(i));
		m_aData.RemoveAll();
	}	

	/**
		Reads the extra field from \a buffer.

		\param pStorage
			The storage to read the data from.

		\param uSize
			The size of the data to read.

		\return
			\c false, if \a uSize was smaller than the declared extra field size; \c true otherwise.
	*/
	bool Read(CZipStorage* pStorage, WORD uSize);

	/**
		Writes the extra field to \a buffer.

		\param buffer
			The buffer to write to.
	*/
	void Write(char* buffer) const;	
	
private:
	
	CZipArray<CZipExtraData*> m_aData;
};


#endif // !defined(ZIPARCHIVE_ZIPEXTRAFIELD_DOT_H)
