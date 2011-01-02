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
* \file FileFilter.h
*	Includes the ZipArchiveLib::CFileFilter and the derived classes.
*
*/


#if !defined(ZIPARCHIVE_FILEFILTER_DOT_H)
#define ZIPARCHIVE_FILEFILTER_DOT_H

#if _MSC_VER > 1000
	#pragma once
	#pragma warning( push )
	#pragma warning (disable : 4100) // unreferenced formal parameter
#endif

#include "stdafx.h"
#include "ZipExport.h"
#include "FileInfo.h"
#include "Wildcard.h"
#include "ZipPlatform.h"
#include "ZipCollections.h"

namespace ZipArchiveLib
{
	/**
		A base class for filters used in the directory enumeration process.

		\see
			<a href="kb">0610231446|filters</a>
		\see 
			CDirEnumerator::Start
	*/
	class ZIP_API CFileFilter
	{	
	public:	
		
		/**
			Initializes a new instance of the CFileFilter class.

			\param bInverted
				Set to \c true to invert the behavior of the filter or to \c false for the normal behavior.				

			\see
				SetInverted					
		*/
		CFileFilter(bool bInverted = false)
			:m_bInverted(bInverted)
		{		
		}

		/**
			This method is directly called by the CDirEnumerator::Start for each file or directory that was
			previously accepted with the #HandlesFile method.

			It internally calls the #Accept method and inverts its result, if the filter is in the inverted 
			mode and does not handle the inversion internally.
			
			- If this method returns \c true for a file, then the file is processed (the CDirEnumerator::Process method
			is called for the file). Otherwise the file is not processed.
			- If this method returns \c true for a directory, then the directory can be traversed for files and subfolders
			(depending on the CDirEnumerator::IsRecursive() method). Otherwise the directory is not traversed.

			
			\param lpszParentDir
				The parent directory containing the file to accept.

			\param lpszName
				The name of the file to accept (without a path).

			\param info
				The structure containing the information about the current file.

			\return
				\c true, if the file is accepted (taking inversion into account); \c false otherwise.

			\see				
				Accept
			\see 
				HandlesFile
			\see
				HandlesInversion
			\see
				SetInverted
			\see
				IsInverted
			\see
				CDirEnumerator::Start
			\see
				CDirEnumerator::Process
		*/
		bool Evaluate(LPCTSTR lpszParentDir, LPCTSTR lpszName, const CFileInfo& info)
		{
			bool ret = Accept(lpszParentDir, lpszName, info);
			if (!HandlesInversion())
				return m_bInverted ? !ret : ret;
			return ret;
		}

		/**
			Sets the filter to operate in the inverted or in the normal mode.
			If the filter operates in an inverted mode, the file that this filer accepts
			is \b not processed and vice versa. Normal mode means that a file is processed
			(the CDirEnumerator::Process method is called for that file), 
			if the filter accepts the file.

			\param bInverted
				\c true to make the filter operate in an inverted mode or \c false to make 
				the filter operate in a normal mode.

			\see
				HandlesInversion
			\see
				IsInverted
		*/
		void SetInverted(bool bInverted = true) { m_bInverted = bInverted;}

		/**
			Returns the value indicating whether the filter operates in an inverted mode or in a normal mode.

			\return
				\c true, if the filter operates in an inverted mode; \c false otherwise.

			\see
				SetInverted
			\see
				HandlesInversion
		*/
		bool IsInverted() const {return m_bInverted;}

		/**
			Returns the value indicating whether the filter can decide about processing of the \a info file.
			If it can, then the #Evaluate method will be called for the \a info file.
			By default this method returns \c true for files and \c false for directories. Override this method
			to change its behavior.

			- If \a info is a file and this method returns \c false, then the file is not processed.
			- If \a info is a directory and this method returns \c false, then the directory can still be traversed for files
			and subfolders (depending on the CDirEnumerator::IsRecursive() method).

			The #Evaluate method is not called in both cases.

			\param info
				The structure containing the information about the file.

			\return
				\c true, if the \a info file should be evaluated by the #Evaluate method; \c false otherwise.

			\note
				This method is particularly useful when the filter operates in the inverted mode (see #SetInverted).
				For example, if a filter should accept directories in both inverted and non-inverted mode, it would need
				to specially handle inversion for directories without this method. With this method it can just not accept
				directories for evaluation and they still will be traversed.
		*/
		virtual bool HandlesFile(const CFileInfo& info)
		{
			return !info.IsDirectory();
		}

		virtual ~CFileFilter()
		{
		}
protected:
		/**
			This method is directly called by the #Evaluate method during an enumeration process.

			If this method returns \c true, the file will later be processed 
			by the CDirEnumerator::Process method. If this method returns \c false for a directory, 
			the directory is not enumerated at all.

			The meaning of the return value can be reversed by the #SetInverted method.
			If this filter handles the inversion internally, the return value from this method 
			is not reversed by the #Evaluate method.

			\param lpszParentDir
				The parent directory containing the file to accept.

			\param lpszName
				The name of the file to accept (without a path).

			\param info
				The structure containing the information about the current file.

			\return
				\c true, if the file is accepted; \c false otherwise.

			\see
				Evaluate
			\see
				HandlesInversion
			\see
				CDirEnumerator::Start
			\see
				CDirEnumerator::Process
		*/
		virtual bool Accept(LPCTSTR lpszParentDir, LPCTSTR lpszName, const CFileInfo& info)
		{
			return true;
		}	

		/**
			Returns the value indicating whether the current filter handles the inversion mode internally
			or not.
			- If the filter is in the inverted mode and it handles the inversion, then the return value \c true
			from the #Accept method means that the file should be processed.
			- If the filter is in the inverted mode and it does not handle the inversion, then the return value \c true
			from the #Accept method means that the file should not be processed.

			It may be more efficient to handle the inversion internally in some cases.

			\return 
				\c true, if the filter handles the inversion internally; \c false otherwise.

			\note
				This method returns \c false by default. Override this method to change this behavior.

			\see
				SetInverted
			\see
				IsInverted
		*/
		virtual bool HandlesInversion() const
		{
			return false;
		}
		bool m_bInverted;

	};


	/**
		A filter that allows filtering files by a filename mask while an enumeration process.

		\see
			<a href="kb">0610231446|filters</a>
		\see
			<a href="kb">0610242025|wildcards</a>
		\see 
			CDirEnumerator::Start
	*/
	class ZIP_API CNameFileFilter : public CFileFilter
	{	
		CWildcard m_matcher;
		int m_iAppliesToTypes;
	public:

		/**
			The file type to which the CNameFileFilter filter can be applied.
			You can use the logical \c OR to combine them.

			\see
				SetAppliesToTypes
			\see
				GetAppliesToTypes
		*/
		enum AppliesToTypes
		{
			toFile			= 0x1,					///< Regular files only.
			toDirectory		= 0x2,					///< Directories only.
			toAll			= toFile | toDirectory	///< Both regular files and directories.
		};

		/**
			Initializes a new instance of the CNameFileFilter class.

			\param lpszPattern
				A mask to match against a filename. This filter uses the CWildcard functionality for this purpose.

			\param iAppliesToTypes
				The file type to which this filter applies. It an be one or more of the #AppliesToTypes values.

			\param bInverted
				Set to \c true to invert the behavior of the filter or to \c false for the normal behavior.				

			\param bCaseSensitive
				\c true, if the matching process is case-sensitive; \c false otherwise. 
				By default, a system case-sensitivity setting is used.

			\see
				<a href="kb">0610231446|filters</a>
			\see
				<a href="kb">0610242025|wildcards</a>
			\see
				SetInverted
			\see
				SetAppliesToTypes
			\see
				CWildcard
			\see
				ZipPlatform::GetSystemCaseSensitivity			
		*/
		CNameFileFilter(LPCTSTR lpszPattern = _T("*"), bool bInverted = false, int iAppliesToTypes = toFile, bool bCaseSensitive = ZipPlatform::GetSystemCaseSensitivity())
			:CFileFilter(bInverted), m_matcher(lpszPattern, bCaseSensitive)
		{
			m_iAppliesToTypes = iAppliesToTypes;
		}

		/**
			Returns the value indicating whether the filter can be applied to the given \a iType type.

			\param iType
				It can be one or more of the #AppliesToTypes values.

			\return 
				\c true, if the filter can be applied to \a iType type; \c false otherwise.

			\see
				SetAppliesToTypes
			\see
				GetAppliesToTypes
		*/
		bool AppliesToType(int iType)
		{
			return (m_iAppliesToTypes & iType) == iType;
		}

		/**
			Sets the file type to which this filter applies.

			\param iType
				The file type to which this filter applies. It can be one or more of the #AppliesToTypes values.

			\see
				GetAppliesToTypes

		*/
		void SetAppliesToTypes(int iType) { m_iAppliesToTypes = iType; }

		/**
			Returns the file type to which this filter applies.

			\return 
				The file type to which this filter applies. It can be one or more of the #AppliesToTypes values.

			\see
				SetAppliesToTypes
		*/
		int GetAppliesToTypes() {return m_iAppliesToTypes;}

		/**
			Returns the value indicating whether the filter can decide about processing of the \a info file.
			The CNameFileFilter returns the value depending on the #GetAppliesToTypes value.
			
			\param info
				The structure containing the information about the file.

			\return
				\c true, if the \a info file will be evaluated by the #Evaluate method; \c false otherwise.

			\see
				GetAppliesToTypes
			\see
				SetAppliesToTypes

		*/
		bool HandlesFile(const CFileInfo& info)
		{
			return info.IsDirectory() ? AppliesToType(toDirectory) : AppliesToType(toFile);
		}
	protected:
		virtual bool Accept(LPCTSTR, LPCTSTR lpszName, const CFileInfo& info)
		{
			return m_matcher.IsMatch(lpszName);
		}			
	};

	/**
		A filter that allows grouping of other filters.

		\see
			<a href="kb">0610231446|filters</a>
		\see 
			CDirEnumerator::Start
	*/
	class ZIP_API CGroupFileFilter : public CFileFilter
	{			

	public:	

		/**
			The grouping type.
		*/
		enum GroupType
		{
			And,	///< Logical AND. All the grouped filters must accept a file for the file to be processed.
			Or		///< Logical OR. At least one of the grouped filters must accept a file for the file to be processed.
		};

		/**
			Initializes a new instance of the CGroupFileFilter class.

			\param groupType
				The grouping type. Should be one of the #GroupType values.

			\param bAutoDelete
				\c true, if the grouped filters should be automatically destroyed by this filter; \c false otherwise.

			\param bInverted
				Set to \c true to invert the behavior of the filter or to \c false for the normal behavior.
				This filter handles the inversion mode internally.

			\see
				<a href="kb">0610231446|filters</a>
			\see
				SetType	
			\see
				SetAutoDelete
			\see
				SetInverted
			\see
				HandlesInversion
		*/
		CGroupFileFilter(GroupType groupType = CGroupFileFilter::And, bool bAutoDelete = true, bool bInverted = false)
			:CFileFilter(bInverted), m_iType(groupType), m_bAutoDelete(bAutoDelete)
		{
		}

		/**
			Adds \a pFilter to the filter's group.

			\param pFilter
				The filter to add.
		*/
		void Add(CFileFilter* pFilter)
		{
			m_filters.Add(pFilter);
		}

		/**
			Returns the filter at the given position.

			\param uIndex
				The index of the filter to return.
		*/
		CFileFilter* GetAt(ZIP_ARRAY_SIZE_TYPE uIndex)
		{
			return m_filters[uIndex];
		}

		/**
			Returns the filter at the given position.

			\param uIndex
				The index of the filter to return.
		*/
		const CFileFilter* GetAt(ZIP_ARRAY_SIZE_TYPE uIndex) const
		{
			return m_filters[uIndex];
		}

		/**
			Returns the filter at the given position.

			\param uIndex
				The index of the filter to return.
		*/
		const CFileFilter* operator[] (ZIP_ARRAY_SIZE_TYPE uIndex) const
		{
			return GetAt(uIndex);
		}

		/**
			Returns the filter at the given position.

			\param uIndex
				The index of the filter to return.
		*/
		CFileFilter* operator[] (ZIP_ARRAY_SIZE_TYPE uIndex)
		{
			return GetAt(uIndex);
		}

		/**
			Remove the filter at the given position.
			The removed filter is deleted from memory, 
			if the CGroupFileFilter object is in the auto-delete mode.

			\param uIndex
				The index of the filter to remove.

			\see
				SetAutoDelete
		*/
		void RemoveAt(ZIP_ARRAY_SIZE_TYPE uIndex)
		{
			CFileFilter* filter = m_filters[uIndex];
			// first remove, then delete
			m_filters.RemoveAt(uIndex);
			if (m_bAutoDelete)
				delete filter;
		}


		/**
			Removes all contained filters from the collection.

			The removed filters are deleted from memory, 
			if the CGroupFileFilter object is in the auto-delete mode.

			\see
				SetAutoDelete

		*/
		void Clear()
		{
			if (m_filters.GetSize() == 0)
				return;

			ZIP_ARRAY_SIZE_TYPE i = m_filters.GetSize() - 1;
			for (; ;)
			{
				RemoveAt(i);
				if (i == 0)
					break;
				i--;
			}
		}

		/**
			Returns the number of grouped filters.

			\return
				The number of grouped filters.

		*/
		ZIP_ARRAY_SIZE_TYPE GetSize()
		{
			return m_filters.GetSize();
		}

		/**
			Sets the type of grouping. 

			\param iType 
				The type of grouping. Should be one of the #GroupType values.

			\see 
				GetType
		*/
		void SetType(GroupType iType) {m_iType = iType;}

		/**
			Returns the type of grouping. 

			\return 
				The type of grouping. It can be one of the #GroupType values.

			\see 
				SetType
		*/
		GroupType GetType() const {return m_iType;}

		/**
			Enables or disables auto-deletion of grouped filters.
			If auto-deletion is enabled, the grouped filters are released from memory
			when they are removed from the group or when the CGroupFileFilter
			object is destroyed.

			\param bAutoDelete
				\c true, to enable auto-deletion; \c false to disable.

			\see	
				IsAutoDelete
		*/
		void SetAutoDelete(bool bAutoDelete) {m_bAutoDelete = bAutoDelete;}

		/**
			Returns the value indicating whether the auto-deletion is enabled.

			\return
				\c true, if the auto-deletion is enabled; \c false otherwise.

			\see
				SetAutoDelete
		*/
		bool IsAutoDelete() const {return m_bAutoDelete;}

		/**
			Returns the value indicating whether the filter can decide about processing of the \a info file.
			The CGroupFileFilter returns the value depending on the value returned by the grouped filters.
			
			\param info
				The structure containing the information about the file.

			\return
				\c true, if any of the grouped filters accepts \a info; \c false otherwise.

		*/
		bool HandlesFile(const CFileInfo& info)
		{
			for (ZIP_ARRAY_SIZE_TYPE i = 0; i < m_filters.GetSize(); i++)
				// it is enough that one filter handles it
				if (m_filters[i]->HandlesFile(info))
					return true;
			return false;
		}
		

		~CGroupFileFilter()
		{
			Clear();
		}

	protected:

		virtual bool Accept(LPCTSTR lpszParentDir, LPCTSTR lpszName, const CFileInfo& info);
		/**
			This filter handles the inversion mode internally.

			\return
				This method returns \c true for this class.

			\see
				CFileFilter::HandlesInversion
		*/
		bool HandlesInversion() const
		{
			return true;
		}		
		GroupType m_iType; ///< Set with the #SetType method or in constructor.
		bool m_bAutoDelete; ///< Set with the #SetAutoDelete or in constructor.

	private:

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning (push)
	#pragma warning( disable : 4251 ) // needs to have dll-interface to be used by clients of class
#endif

		CZipArray<CFileFilter*> m_filters;

#if (_MSC_VER > 1000) && (defined ZIP_HAS_DLL)
	#pragma warning( pop)
#endif

	};
}

#if _MSC_VER > 1000
	#pragma warning( pop )
#endif

#endif
