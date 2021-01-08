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

#ifndef CSELECTION_H_
#define CSELECTION_H_

#include "Query/CObject.h"
#include <vector>
#include <string>
#include "gumbo.h"
#include "gumbo_edit.h"

class CNode;

class CSelection: public CObject
{

	public:

        CSelection(GumboNode* apNode, bool error = false);

		CSelection(std::vector<GumboNode*> aNodes, bool error = false);

		virtual ~CSelection();

	public:

		CSelection find(std::string aSelector);

		CNode nodeAt(size_t i);

		size_t nodeNum();

        bool parseError() { return mparseError; }

	private:

		std::vector<GumboNode*> mNodes;
        bool mparseError;
};

#endif /* CSELECTION_H_ */

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
