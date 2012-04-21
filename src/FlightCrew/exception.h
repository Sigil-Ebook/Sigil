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

#pragma once
#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <boost/exception/all.hpp>

namespace FlightCrew
{

#define boost_throw(x) BOOST_THROW_EXCEPTION(x)

// FlightCrew uses the "Exception types as semantic tags" idiom.
// For more information, see this link:
//   http://www.boost.org/doc/libs/1_44_0/libs/exception/doc/exception_types_as_simple_semantic_tags.html

/**
 * The common base for all exceptions.
 */
struct ExceptionBase: virtual std::exception, virtual boost::exception {};

/**
 * Thrown when a file does not exist.
 */
struct FileDoesNotExistEx : virtual ExceptionBase {};
typedef boost::error_info< struct file_path, std::string > ei_FilePath;

/**
 * Thrown when a file is not in utf-8/16.
 */
struct FileNotInUnicodeEx : virtual ExceptionBase {};

/**
 * Thrown when a path is not in utf-8.
 */
struct PathNotInUtf8 : virtual ExceptionBase {};

/**
 * Thrown when the Xerces parser gives us null for a document.
 */
struct XercesParsingError : virtual ExceptionBase {};


} // namespace FlightCrew

#endif // EXCEPTION_H