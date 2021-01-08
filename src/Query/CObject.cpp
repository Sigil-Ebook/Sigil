/************************************************************************
 **
 **  Copyright (C) 2021 Kevin B. Hendricks, Stratford, ON, Canada
 **
 **  This file is part of Sigil.
 **
 **  Sigil is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Sigil is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
 **
 **
 ** Taken from:
 ** 
 ** gumbo-query
 ** https://github.com/lazytiger/gumbo-query
 **
 ** A C++ library that provides jQuery-like selectors for Google's Gumbo-Parser.
 ** Selector engine is an implementation based on cascadia.
 **
 ** Available under the MIT License  
 ** See ORIGINAL_LICENSE file in the source code 
 ** hoping@baimashi.com, Copyright (C) 2016
 **
 *************************************************************************/

#include <QDebug>
#include "Query/CObject.h"

CObject::CObject()
{
	mReferences = 1;
}

CObject::~CObject()
{
	if (mReferences != 1)
	{
               qDebug() << "something wrong, reference count not zero";
               // throw "something wrong, reference count not zero";
	}
}

void CObject::retain()
{
	mReferences++;
}

void CObject::release()
{
	if (mReferences < 0)
	{
                qDebug() << "something wrong, reference count is negative";
		// throw "something wrong, reference count is negative";
	}

	if (mReferences == 1)
	{
		delete this;
	}
	else
	{
		mReferences--;
	}
}

unsigned int CObject::references()
{
	return mReferences;
}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */

