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
* \file BaseLibCompressor.h
*	Includes the ZipArchiveLib::CBaseLibCompressor class.
*
*/

#if !defined(ZIPARCHIVE_BASELIBCOMPRESSOR_DOT_H)
#define ZIPARCHIVE_BASELIBCOMPRESSOR_DOT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "ZipExport.h"
#include "ZipCompressor.h"
#include "ZipCollections.h"
#include "ZipException.h"

namespace ZipArchiveLib
{

/**
	A base class for compressors that use external libraries, such as zlib or bzip2.
*/
class ZIP_API CBaseLibCompressor : public CZipCompressor
{	
public:
	/**
		Represents options of compressors that use external libraries.

		\see
			<a href="kb">0610231446|options</a>
		\see
			CZipArchive::SetCompressionOptions
	*/
	struct ZIP_API COptions : CZipCompressor::COptions
	{
		COptions()
		{
			m_bDetectLibMemoryLeaks = true;
		}

		/**		
			\c true, if the ZipArchive Library should detect memory leaks in an external library; \c false otherwise. 
			Recommended to be set to \c true.
		*/
		bool m_bDetectLibMemoryLeaks;
	};

	/**
		Initializes a new instance of the CBaseLibCompressor class.

		\param pStorage
			The current storage object.
	 */
	CBaseLibCompressor(CZipStorage* pStorage)
		:CZipCompressor(pStorage)
	{
	}
	
	void InitDecompression(CZipFileHeader* pFile, CZipCryptograph* pCryptograph)
	{
		CZipCompressor::InitDecompression(pFile, pCryptograph);
		m_bDecompressionDone = false;
	}	

	~CBaseLibCompressor()
	{
		EmptyPtrList();
	}
protected:	

	/**
		A memory allocation method called by an external library.

		\param opaque
			Internal data.

		\param items
			The number of blocks to allocate.

		\param size
			The size of each block to allocate.

		\return 
			The address of a newly allocated memory.
	*/
	static void* _zipalloc(void* opaque, UINT items, UINT size)
	{
		void* p = new char[size * items];
		if (opaque)
		{
			CZipPtrList<void*>* list  = (CZipPtrList<void*>*) opaque;
			list->AddTail(p);
		}
		return p;
	}

	/**
		A memory deallocation method called by an external library.

		\param opaque
			Internal data.

		\param address
			Memory address to free.
	*/
	static void _zipfree(void* opaque, void* address)
	{
		if (opaque)
		{
			CZipPtrList<void*>* list  = (CZipPtrList<void*>*) opaque;
			CZipPtrListIter iter = list->Find(address);
			if (list->IteratorValid(iter))
				list->RemoveAt(iter);
		}
		delete[] (char*) address;
	}

	/**
		Frees the memory allocated by an external library that hasn't been freed
		due to an error in the library (it should never happen).
	*/
	void EmptyPtrList();

	/**
		Checks whether \a iErr value is an error code.

		\param iErr
			The code to check.

		\return
			\c true, if \a iErr is an error code; \c false otherwise.
	*/
	virtual bool IsCodeErrorOK(int iErr) const = 0;

	/**
		Checks whether \a iErr value is an error code and throws an exception if it is.

		\param iErr
			The error code.

	*/
	void CheckForError(int iErr)
	{
		if (!IsCodeErrorOK(iErr))
			ThrowError(iErr, true);
	}

	/**
		Sets an address of internal data used in ZipArchive Library memory allocation and deallocation methods.

		\param opaque
			Receives an address of the internal data.
		\param pOptions
			The current decompressor options.
	*/
	void SetOpaque(void** opaque, const	COptions* pOptions);

	/**
		Signalizes that the decompression process reached the end of the compressed data. It is internally set by derived classes.
	*/
	bool m_bDecompressionDone;
private:
	typedef CZipPtrList<void*>::iterator CZipPtrListIter;

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning (push)
	#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
#endif

	CZipPtrList<void*> m_list; ///< A list holding pointers to the memory areas allocated by an external library.

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning( pop)
#endif

};

} // namespace

#endif
