/************************************************************************
**
**  Copyright (C) 2010  Strahinja Markovic
**
**  This file is part of FlightCrew.
**
**  FlightCrew is free software: you can redistribute it and/or modify
**  it under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  FlightCrew is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU Lesser General Public License for more details.
**
**  You should have received a copy of the GNU Lesser General Public License
**  along with FlightCrew.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

// We set the MSVC warning level down to 3
// for code that we have no control over
#if defined(_MSC_VER)
#   pragma warning( push, 3 )
#endif

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/bind/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp> 
#include <boost/foreach.hpp> 
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>
#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>
#include "Misc/BoostFilesystemUse.h"


// ... and then we reset the warning level
// back to normal (warning level 4)
#if defined(_MSC_VER)
#   pragma warning( pop )
#endif

namespace XercesExt { struct QName; };
namespace xe = XercesExt;

using xe::QName;

#include "constants.h"
#include "exception.h"
#include "Misc/CustomAssert.h"

// We're most definitely not going to use
// it as BOOST_FOREACH.
#define foreach BOOST_FOREACH

// We will be using these everywhere,
// so let's make life a bit easier.
using boost::tuple;
using boost::make_tuple;
using boost::tie;

namespace xc = XERCES_CPP_NAMESPACE;

typedef unsigned int uint;
