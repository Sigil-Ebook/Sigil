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
* \file ZipCallbackProvider.h
* Includes the ZipArchiveLib::CZipCallbackProvider class.
*
*/

#if !defined(ZIPARCHIVE_ZIPCALLBACKPROVIDER_DOT_H)
#define ZIPARCHIVE_ZIPCALLBACKPROVIDER_DOT_H

#if _MSC_VER > 1000
	#pragma once
	#if defined ZIP_HAS_DLL
		#pragma warning (push)
		#pragma warning( disable : 4275 ) // non dll-interface used as base for dll-interface class
	#endif
#endif

#include "ZipCallback.h"
#include "ZipExport.h"
#include "ZipCollections.h"

namespace ZipArchiveLib
{

class ZIP_API CZipCallbackProvider : public CZipMap<CZipActionCallback::CallbackType, CZipActionCallback*>		
{
public:
	void Set(CZipActionCallback* pCallback, int iWhich)
	{
		CZipActionCallback::CallbackType cbs[] = {CZipActionCallback::cbAdd, CZipActionCallback::cbAddTmp, CZipActionCallback::cbAddStore, CZipActionCallback::cbExtract, CZipActionCallback::cbDeleteCnt, CZipActionCallback::cbDelete, CZipActionCallback::cbTest, CZipActionCallback::cbSave, CZipActionCallback::cbGet, CZipActionCallback::cbModify, CZipActionCallback::cbMoveData, CZipActionCallback::cbCalculateForMulti, CZipActionCallback::cbMultiAdd, CZipActionCallback::cbEncryptPrepare, CZipActionCallback::cbEncryptMoveData, CZipActionCallback::cbEncrypt, CZipActionCallback::cbMultiEncrypt};
		int iCount = sizeof(cbs)/sizeof(CZipActionCallback::CallbackType);
		for (int i = 0; i < iCount; i++)
		{
			CZipActionCallback::CallbackType iCallback = cbs[i];
			if (iWhich & iCallback)
				SetInternal(pCallback, iCallback);
		}
	}

	CZipActionCallback* Get(CZipActionCallback::CallbackType iType)
	{
		CZipActionCallback* pCallback = NULL;
		if (Lookup(iType, pCallback))
		{
			pCallback->m_iType = iType;
			return pCallback;
		}
		else
			return NULL;
	}
protected:
	void SetInternal(CZipActionCallback* pCallback, CZipActionCallback::CallbackType iType)
	{
		if (pCallback)
		{
			SetAt(iType, pCallback);
		}
		else
			RemoveKey(iType);
	}	
};

} // namespace

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning (pop)	
#endif


#endif // !defined(ZIPARCHIVE_ZIPCALLBACKPROVIDER_DOT_H)
