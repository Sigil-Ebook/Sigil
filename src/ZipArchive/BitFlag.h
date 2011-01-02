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
* \file BitFlag.h
*	Includes the ZipArchiveLib::CBitFlag class.
*/

#if !defined(ZIPARCHIVE_BITFLAG_DOT_H)
#define ZIPARCHIVE_BITFLAG_DOT_H

#if _MSC_VER > 1000
	#pragma once
#endif

namespace ZipArchiveLib
{
	/**
		Provides functionality for bit operations on an integer value.
	*/
	struct ZIP_API CBitFlag
	{
	public:

		/**
			The current value.
		*/
		int m_value;

		/**
			Initializes a new instance of the CBitFlag class.
		 */
		CBitFlag()
			:m_value(0)
		{
		}

		/**
			Initializes a new instance of the CBitFlag class.

			\param value
				The initial value.
		 */
		CBitFlag(int value)
			:m_value(value)
		{
		}		

		/**
			Sets the given flags.

			\param flags
				The flags to set.
		 */
		void Set(int flags)
		{
			m_value |= flags;
		}
		
		/**
			Clears the given flags.

			\param flags
				The flags to clear.
		 */
		void Clear(int flags)
		{
			m_value &= ~flags;
		}

		/**
			Returns the value indicating whether any of the given flags is set.

			\param flags
				The flags to examine.

			\return
				\c true, if any of the given flags is set; \c false otherwise.
		*/
		bool IsSetAny(int flags) const
		{
			return (m_value & flags) != 0;
		}


		/**
			Returns the value indicating whether all of the given flags are set.

			\param flags
				The flags to examine.

			\return
				\c true, if all of the given flags are set; \c false otherwise.
		*/
		bool IsSetAll(int flags) const
		{
			return (m_value & flags) == flags;
		}


		/**
			Sets the given flags and examines if this caused a modification to the current object.

			\param flags
				The flags to set.

			\return
				\c true, if the operation caused modification; \c false otherwise.
		*/
		bool SetWithCheck(int flags)
		{
			if (!IsSetAll(flags))
			{
				Set(flags);
				return true;
			}
			else
				return false;
		}

		/**
			Clears the given flags and examines if this caused a modification to the current object.

			\param flags
				The flags to clear.

			\return
				\c true, if the operation caused modification; \c false otherwise.
		*/
		bool ClearWithCheck(int flags)
		{
			if (IsSetAny(flags))
			{
				Clear(flags);
				return true;
			}
			else
				return false;
		}

		/**
			Changes the given flags and examines if this caused a modification to the current object.

			\param flags
				The flags to change.
			\param set
				If \c true, the flags will be set; otherwise the flags will be cleared. 

			\return
				\c true, if the operation caused a modification; \c false otherwise.
		*/
		bool ChangeWithCheck(int flags, bool set)
		{
			return set ? SetWithCheck(flags) : ClearWithCheck(flags);
		}

		/**
			Changes the given flags.

			\param flags
				The flags to change.

			\param set
				If \c true, the flags will be set; otherwise the flags will be cleared. 
		*/
		void Change(int flags, bool set)
		{
			set ? Set(flags) : Clear(flags);
		}

		/**
			Returns the current value.
		*/
		operator int() const
		{
			return m_value;
		}

		CBitFlag& operator = (const CBitFlag& flag)
		{
			m_value = flag.m_value;
			return *this;
		}

		bool operator == (int value)
		{
			return m_value == value;
		}

		bool operator == (const CBitFlag& flag)
		{
			return m_value == flag.m_value;
		}
	};
}

#endif


