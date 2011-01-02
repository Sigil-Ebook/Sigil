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
* \file ZipCallback.h
* Includes the CZipCallback and the derived classes.
*
*/

#if !defined(ZIPARCHIVE_ZIPCALLBACK_DOT_H)
#define ZIPARCHIVE_ZIPCALLBACK_DOT_H

#if _MSC_VER > 1000
	#pragma once
	#if defined ZIP_HAS_DLL
		#pragma warning (push)
		#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
	#endif
#endif

#include "ZipString.h"
#include "ZipExport.h"

/*
	When processing split archives, this value is passes as the argument to the CZipCallback::Callback method
	to notify that the current volume is the last one.
*/
#define ZIP_SPLIT_LAST_VOLUME (ZIP_SIZE_TYPE)(-1)

/**
	A base class for callback objects that are notified when various actions take place.
	You need to derive your own class and overload the #Callback method to use it.
	Do not derive from CZipCallback directly but derive from:
	\li CZipSegmCallback, when you create a segmented archive - you will be notified 
		when a next volume is processed in a segmented archive.
	\li CZipActionCallback, for other notifications.

	\see
		<a href="kb">0610051553|span</a>
	\see
		<a href="kb">0610231200</a>
	\see
		CZipSegmCallback
	\see
		CZipActionCallback
*/
struct ZIP_API CZipCallback
{
	/**
		The method called as a callback.

		Return \c false from inside of this method to abort the current operation. If it is a segmented archive callback object,
		a CZipException with CZipException::aborted code will be thrown, otherwise the code will be CZipException::abortedAction or CZipException::abortedSafely.
		The following actions can be safely aborted (without corrupting the archive):
		- extracting
		- testing
		- saving the central directory in a not segmented archive 
		- counting bytes before deleting files
		(saved data is removed in case of break	and you can save it again).

		If the archive is segmented and if saving is aborted, the archive
		will not be damaged, but saved part of the central directory will be not removed
		and the new central directory will have to be saved after it.

		\param uProgress
			The value depends on the type of the operation.

		\return 
			\c false to abort the current operation; \c true to continue it.

		\note
			Override this method in a derived class. If you define this method inside the class declaration, consider inlining
			it to make an action progress faster.

		\see
			CZipSegmCallback
	*/
	virtual bool Callback(ZIP_SIZE_TYPE uProgress) = 0;


	/**
		The filename of an external file, if an action (adding, extracting or segmentation) uses such a file.
		You can set its value, to change the currently used path.
	*/
	CZipString m_szExternalFile;
	virtual ~CZipCallback(){}
};

/**
	When you derive from CZipSegmCallback, you can use the derived class as a callback object for:
		- the disk change notification in a spanned archive,
		- notification about the next part being processed in a split archive.

	You need to override the CZipActionCallback::Callback method.
	The value of the \a uProgress parameter in the callback method has the following meaning:
		- the minimum number of free bytes required on the disk when writing a spanned archive
		- it is set to \c 0 when reading a spanned archive
		- when writing or reading a split archive it is set to \c 0 apart for the last volume when it is set to \c ZIP_SPLIT_LAST_VOLUME

	Return \c false from the callback function to abort the operation: the proper exception will be thrown.

	\see
		<a href="kb">0610051553|span</a>
	\see
		CZipActionCallback::Callback
	\see
		CZipArchive::SetSegmCallback
*/
struct ZIP_API CZipSegmCallback : public CZipCallback
{
	/**
		Values indicating the reason for calling the callback.
	*/
	enum SegmCodes
	{
		scVolumeNeededForRead,	///< The next volume is needed when reading a segmented archive. The number of the volume is stored in #m_uVolumeNeeded.
		scVolumeNeededForWrite,	///< The next volume is needed when writing a segmented archive. The number of the volume is stored in #m_uVolumeNeeded.
		scFileNameDuplicated,	///< The file that is used for writing a new volume already exists.
		scCannotSetVolLabel,	///< The disk label could not be set. The disk may be write-protected. It is called only for spanned archives.
		scFileCreationFailure,	///< The archive file could not be created. The disk may be write-protected.
		scFileNotFound,			///< The given volume file was not found when reading a split archive. The number of the volume is stored in #m_uVolumeNeeded. It is called only for split archives.
	};

	ZIP_VOLUME_TYPE m_uVolumeNeeded;		///< The number of the volume needed when reading or writing a segmented archive. Volumes are numbered starting from 1.
	
	int m_iCode; ///< The reason for calling the callback. It can be one of the #SegmCodes values.
};

/**
	When you derive from CZipActionCallback, you can use the new class as a callback object when adding, extracting, deleting, testing files
	or saving the central directory.
	You need to override the CZipActionCallback::Callback method.
	The value of the \a uProgress parameter in the callback method is the amount of data just processed.

	\see
		<a href="kb">0610231200</a>
	\see
		CZipActionCallback::Callback
	\see
		CZipArchive::SetCallback
*/
struct ZIP_API CZipActionCallback : public CZipCallback
{		
	friend class CZipArchive;
	friend class CZipCentralDir;

	/**
		Values used for specifying a callback type in the CZipArchive::SetCallback method.
		You can assign several values to the same callback object (use the logical \c OR).
		\see
			<a href="kb">0610231200</a>
		\see
			CZipArchive::SetCallback
		\see
			CZipActionCallback
	*/
	enum CallbackType
	{
		/**
			Not used.
			You can use it for your own purposes.			
		*/
		cbNothing		= 0x0000,

		/**
			Compressing a file.
			The callback called when adding a file with one of the CZipArchive::AddNewFile methods.
		*/
		cbAdd			= 0x0001,

		/**
			Moving a file from a temporary archive.
			The callback called while adding a file (only on a segmented archive) when the smartness level contains
			CZipArchive::zipsmCheckForEff or CZipArchive::zipsmCheckForEffInMem
			and if the just compressed file is being moved from a temporary place (file or memory) to the archive.
		*/
		cbAddTmp		= 0x0002,

		/**
			Storing a file.
			The callback called while adding a file and if its compressing has proven to be inefficient
			and it is now being stored (instead of compressed) in the archive. The smartness level must 
			contain CZipArchive::zipsmCheckForEff or CZipArchive::zipsmCheckForEffInMem. The archive can be
			segmented or not.
		*/
		cbAddStore		= 0x0004,	

		/**
			Extracting a file.
			The callback called when extracting a file with one of the CZipArchive::ExtractFile methods.
		*/
		cbExtract		= 0x0008,

		/**
			Counting data to process before deleting.
			The callback called before the actual deletion takes place and the map of holes and continuous
			areas is being created. It is safe to abort the operation (by returning \c false from
			the callback method).
		*/
		cbDeleteCnt		= 0x0010,

		/**
			Deleting files.
			The callback called when moving data while deleting file(s) with the CZipArchive::RemoveFile
			method or one of the CZipArchive::RemoveFiles methods.
		*/
		cbDelete		= 0x0020,

		/**
			Testing a file.
			The callback called when testing a file with the CZipArchive::TestFile method.
		*/
		cbTest			= 0x0040,

		/**
			Saving the central directory.
			The callback called when saving the central directory with the CZipCentralDir::Write method
			(usually on close or flush). It is safe to abort the operation on a segmented archive - 
			the saved part of the central directory will be removed from disk.
		*/
		cbSave			= 0x0080,

		/**
			Getting a file from another archive.
			The callback called when using one of the CZipArchive::GetFromArchive methods.
		*/
		cbGet			= 0x0100,

		/**
			Renaming a file.
			The callback called when committing changes with the CZipArchive::CommitChanges method, if there is a need to adjust space 
			for the new filenames.
		*/
		cbModify		= 0x0200,

		/**
			Moving data.
			The callback called when moving data inside the archive. It happens while replacing files to adjust space 
			for the new file or while shifting data with the CZipArchive::ShiftData() method.
		*/
		cbMoveData		= 0x0400,

		/**
			The callback called when counting files and bytes to process when performing multiple actions.
			When this callback is called, CZipActionCallback::m_uTotalToProcess is
			not set (because it is unknown and that's why the counting is performed), 
			but it allows to abort the counting process.
		*/
		cbCalculateForMulti= 0x0800,
		
		/**
			The callback called when adding multiple files with one of the CZipArchive::AddNewFiles
			methods. This callback also registers for #cbAdd.

			\see
				<a href="kb">0610231200|multi</a>
		*/
		cbMultiAdd		= 0x1000 | cbAdd,

		
		/**
			The callback called when preparing existing files for encryption.

			\see
				<a href="kb">0610201627|existing</a>
		*/
		cbEncryptPrepare= 0x2000,

		/**
			The callback called in order to report the progress of adjusting space inside the archive before the actual encryption takes place.

			\see
				<a href="kb">0610201627|existing</a>
		*/
		cbEncryptMoveData= 0x4000,

		/**
			The callback called for every file being encrypted.

			\see
				<a href="kb">0610201627|existing</a>
		*/
		cbEncrypt		= 0x8000,


		/**
			The callback called when encrypting existing files in the archive. This callback also registers for #cbEncryptMoveData and #cbEncrypt.

			\see
				<a href="kb">0610201627|existing</a>
			\see
				<a href="kb">0610231200|multi</a>
		*/
		cbMultiEncrypt	= 0x10000 | cbEncryptMoveData | cbEncrypt,

		/**
			Reserved.
			You can declare your own callback types above this value. Do not use the numeric value, 
			but the symbol name - the value may change in the future releases of the library.
		*/
		cbNextValue		= 0x20000,
		
		/**
			All sub-actions.
			Represents the sub-actions callbacks - they are called as a part of bigger actions
			(#cbAddTmp | #cbAddStore | #cbDeleteCnt | #cbMoveData | #cbCalculateForMulti | #cbEncryptPrepare | #cbEncryptMoveData).
		*/
		cbSubActions	= cbAddTmp | cbAddStore | cbDeleteCnt | cbMoveData | cbCalculateForMulti | cbEncryptPrepare | cbEncryptMoveData,

		/**
			Main callbacks.
			Represents the main action callbacks (#cbAdd | #cbExtract | #cbDelete | #cbTest | #cbSave | #cbGet | #cbModify | #cbEncrypt).
		*/
		cbActions		= cbAdd | cbExtract | cbDelete | cbTest | cbSave | cbGet | cbModify | cbEncrypt,

		/**
			Multiple action callbacks.
			Represents the multiple action callbacks.
		*/
		
		cbMultiActions	= cbMultiAdd | cbMultiEncrypt,

		/**
			All values.
			Use this value to use one callback object for all callback types.
		*/
		cbAll			= cbActions | cbSubActions | cbMultiActions
	};


	/**
		A structure that provides global information in case of using multiple actions callbacks.

		\see
			<a href="kb">0610231200|multi</a>
		\see
			cbMultiActions		
	*/
	struct ZIP_API CMultiActionsInfo
	{
		friend struct CZipActionCallback;

		ZIP_SIZE_TYPE m_uTotalBytesToProcess;	///< The total number of bytes to process.
		ZIP_SIZE_TYPE m_uTotalFilesToProcess;	///< The total number of files to process.	
		ZIP_SIZE_TYPE m_uBytesProcessed;		///< The total number of bytes processed so far.
		ZIP_SIZE_TYPE m_uFilesProcessed;		///< The total number of files processed so far.

		/**
			Returns the number of files left to process.
			\return 
				The number of files left to process.
		*/
		ZIP_SIZE_TYPE LeftFilesToProcess() const {return m_uTotalFilesToProcess - m_uFilesProcessed;}

		/**
			Returns the number of bytes left to process.
			\return 
				The number of bytes left to process.
		*/
		ZIP_SIZE_TYPE LeftBytesToProcess() const {return m_uTotalBytesToProcess - m_uBytesProcessed;}		
	private:
		void Init(ZIP_SIZE_TYPE uTotalItemsToProcess, ZIP_SIZE_TYPE uTotalBytesToProcess, int iReactType)
		{
			m_uTotalFilesToProcess = uTotalItemsToProcess;
			m_uTotalBytesToProcess = uTotalBytesToProcess;
			m_uBytesProcessed = m_uFilesProcessed = 0;
			m_iReactType = iReactType;
			m_bActive = false;
		}
		void OnCallbackInit(int iType)
		{
			// this assumes, that the callback type will stay unchanged 
			// between Init() and CallbackEnd()
			m_bActive = iType == m_iReactType;
		}

		void OnCallCallback(ZIP_SIZE_TYPE uProgress)
		{
			if (m_bActive)
				m_uBytesProcessed += uProgress;
		}
		bool OnNextFile()
		{
			if (m_bActive)
			{
				m_uFilesProcessed++;
				return true;
			}
			else
				return false;
		}
		bool m_bActive;		
		int m_iReactType;
	};

	CZipActionCallback()
	{		
		m_uTotalToProcess = 0;
		m_uProcessed = 0;
		m_pMultiActionsInfo = NULL;
	}

	/**
		Called when a multiple actions operation is about to begin. Initializes CMultiActionsInfo object.

		\param uTotalFilesToProcess
			The number of files to process.

		\param uTotalBytesToProcess
			The number of bytes to process.

		\param iReactType
			The type of the callback that will cause increasing of values in CMultiActionsInfo.
			It can be one of #CallbackType values.

		\see
			<a href="kb">0610231200|multi</a>
		\see
			GetMultiActionsInfo

		\note
			When overriding this method, call the base method from your code.
	*/
	virtual void MultiActionsInit(ZIP_SIZE_TYPE uTotalFilesToProcess, ZIP_SIZE_TYPE uTotalBytesToProcess, int iReactType)
	{
		InitMultiActionsInfo();
		m_pMultiActionsInfo->Init(uTotalFilesToProcess, uTotalBytesToProcess, iReactType);
	}
	
	/**
		Called to initialize the callback object.
		Sets the filenames and resets #m_uTotalToProcess and #m_uProcessed variables to \c 0.
		#m_iType variable is already set to the proper value. Called at the beginning of the operation.

		\param lpszFileInZip
			The file that is being processed in the archive.

		\param lpszExternalFile
			The external file being processed. This is set to \c NULL, if the operation takes place in memory.
	*/
	virtual void Init(LPCTSTR lpszFileInZip = NULL, LPCTSTR lpszExternalFile = NULL) 
	{
		m_szFileInZip = lpszFileInZip;
		m_szExternalFile = lpszExternalFile;
		m_uTotalToProcess = 0;  // not yet known
		m_uProcessed = 0; // nothing yet done
		CacheStepSize();		
		ResetProgressStage();
		if (m_pMultiActionsInfo)
			// the type is known now
			m_pMultiActionsInfo->OnCallbackInit(m_iType);
	}

	/**
		Called after calculating the total amount of data to process.

		\param uTotalToDo
			Total amount of data to process. The method initializes #m_uTotalToProcess with this value.
	*/
	virtual void SetTotal(ZIP_SIZE_TYPE uTotalToDo)
	{
		m_uTotalToProcess = uTotalToDo;
// 		m_uProcessed = 0; // already done in CZipCallbackProvider::Get
	}	

	/**
		Called when a next action begins in multiple actions processing.
		This method calls #Callback method with \a uProgress parameter set to \c 0.

		\return
			The value returned from the #Callback method.

		\see
			<a href="kb">0610231200|multi</a>

		\note
			When overriding this method, call the base method from your code.
	*/
	virtual bool MultiActionsNext()
	{
		if (m_pMultiActionsInfo && m_pMultiActionsInfo->OnNextFile())
			return Callback(0);
		else
			return true;
	}

	/**
		Called after the operation finishes.
		It is not called in case of an exception, but
		it is called before throwing CZipException::abortedAction or CZipException::abortedSafely.
	*/
	virtual void CallbackEnd()
	{
		//ASSERT(m_uProcessed == m_uTotalToProcess);		
	};
	
	/**
		Called at the end of the multiple actions operation.
		Releases multiple actions information (CMultiActionsInfo).
		Override this method, if you want this information to be persisted after
		operation finishes. In that case, CMultiActionsInfo will be released
		on destruction.

		\see
			<a href="kb">0610231200|multi</a>
	*/
	virtual void MultiActionsEnd()
	{
		ReleaseMultiActionsInfo();
	}

	/**
		Returns the amount of data left to process.

		\return
			The amount of data left to process.
	*/
	ZIP_SIZE_TYPE LeftToProcess() const {return m_uTotalToProcess - m_uProcessed;}

	/**
		The total amount of data to process. This value is set when the #SetTotal method is called.

		Depending on the action it is set then to:
		- Adding a file : the size of the external file being added (or if the type of the callback is CZipActionCallback::cbAddTmp, 
			the size of the compressed data: CZipFileHeader::m_uComprSize).
		- Extracting a file : the size of the uncompressed data (CZipFileHeader::m_uUncomprSize).
		- Testing a file : the same as above.
		- Deleting a file : the number of bytes to move - the size of all files to remain above the first 
			file to delete (calculated from offsets in CZipFileHeader::m_uOffset).
		- Saving the central directory : the number of files in the archive.
	*/
	ZIP_SIZE_TYPE m_uTotalToProcess;				
	ZIP_SIZE_TYPE m_uProcessed;	///< The total amount of data processed so far.
	CZipString m_szFileInZip;		///< The name of the file being processed in the archive.

	/**
		The type of the callback. It is set to one of the #CallbackType values when an action begins.
		It's useful, if you have more than one callback assigned to the same callback object.
	*/
	int m_iType;

	/**
		Returns the current CMultiActionsInfo object in multiple actions operation or \c NULL, if 
		the current operation is not one of CZipActionCallback::cbMultiActions actions.

		\return
			The current CMultiActionsInfo object.

		\see
			<a href="kb">0610231200|multi</a>
	*/
	CMultiActionsInfo* GetMultiActionsInfo()
	{
		return m_pMultiActionsInfo;
	}

	/**
		Sets the type of the callback, that will cause the current CMultiActionsInfo object to update its progress.

		\param iType
			The type of the callback. It can be one of the #CallbackType values.
	*/
	void SetReactType(int iType)
	{
		m_pMultiActionsInfo->m_iReactType = iType;
	}

	/**
		Specifies how often the Callback() method is called. The Callback() method is called every \b n-th callback request,
		where \b n is the value returned from the GetStepSize() method.
		Override this method to adjust the frequency of calling the callback. 

		\return 
			The value that determines the frequency of calling the Callback() method.
			By default, it returns \c 256 for #cbSave, #cbDeleteCnt, #cbCalculateForMulti and #cbEncryptPrepare and \c 1
			for other callback types.

		\note
			This method is called in the Init() method and the returned value is cached for further processing.
			This means that when you override this method, even with some time-consuming operations, it will not affect
			the in-action performance.

		\note
			Do not use a too low value, because it may increase significantly the time needed to process a large number of files.
	*/
	virtual int GetStepSize()
	{
		return m_iType == cbSave || m_iType == cbDeleteCnt || m_iType == cbCalculateForMulti || m_iType == cbEncryptPrepare ? 256 : 1;
	}

	~CZipActionCallback()
	{
		ReleaseMultiActionsInfo();
	}

	/**
		Called by processing classes when data processing progressed.

		\param uProgress
			The amount of data processed.

		\return
			The value returned from #Callback, if the method was called; \c true otherwise.
			
	*/
	bool RequestCallback(ZIP_SIZE_TYPE uProgress = 1)
	{
		if (!uProgress)
			return true;
		if (m_iCachedStepSize == 1)
			return CallCallback(uProgress);
		else
		{
			m_uAccumulatedProgress += uProgress;		
			if (m_iCurrentStep >= m_iCachedStepSize)
			{
				bool ret = CallCallback(m_uAccumulatedProgress);
				ResetProgressStage();
				return ret;
			}
			else
			{
				m_iCurrentStep++;
				return true;
			}
		}
	}

	/**
		Called by processing classes when data processing is finished to 
		allow calling the Callback() method for the remaining amount of processed data (for 
		which the Callback() has not been called). This usually happens, 
		when GetStepSize() does not return 1.

		\param uProgress
			The amount of data processed.

		\return
			The value returned from the #Callback method, if the method was called; \c true otherwise.
			
	*/
	bool RequestLastCallback(ZIP_SIZE_TYPE uProgress = 0)
	{
		bool ret;
		if (m_uAccumulatedProgress == 0 && uProgress == 0)
			ret = true;
		else
			ret = CallCallback(m_uAccumulatedProgress + uProgress);
		ResetProgressStage();
		return ret;
	}

protected:	
	/**
		
		Calls the Callback() method internally, increases CZipActionCallback::m_uProcessed and lets the CZipActionCallback
		increase its values, if needed.

		\param uProgress
			The amount of data processed.

		\return
			The return value from the Callback() method.
	*/
	virtual bool CallCallback(ZIP_SIZE_TYPE uProgress)
	{
		m_uProcessed += uProgress;
		if (m_pMultiActionsInfo)
			m_pMultiActionsInfo->OnCallCallback(uProgress);
		return Callback(uProgress);
	}

	/**
		Caches the value returned from the GetStepSize() method. Called in the Init()method.
	*/
	void CacheStepSize()
	{
		m_iCachedStepSize = GetStepSize();
		if (m_iCachedStepSize == 0)
			m_iCachedStepSize = 1;
	}

	/**
		Initializes the values that depend on the frequency of calling the Callback() method.
		Called with every call to the Callback() method.

		\see
			GetStepSize
	*/
	void ResetProgressStage()
	{
		m_iCurrentStep = 1;
		m_uAccumulatedProgress = 0;
	}
private:
	CMultiActionsInfo* m_pMultiActionsInfo;
	void InitMultiActionsInfo()
	{
		ReleaseMultiActionsInfo();
		m_pMultiActionsInfo = new CMultiActionsInfo();
	}
	void ReleaseMultiActionsInfo()
	{
		if (m_pMultiActionsInfo != NULL)
		{
			delete m_pMultiActionsInfo;
			m_pMultiActionsInfo = NULL;
		}
	}

	int m_iCachedStepSize; ///< The cached step value for the time of processing.
	int m_iCurrentStep;
	ZIP_SIZE_TYPE m_uAccumulatedProgress;
};

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning (pop)	
#endif


#endif
