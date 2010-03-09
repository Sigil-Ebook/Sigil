/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
*************************************************************************/

#include <boost/exception/all.hpp>

#define boost_throw(x) BOOST_THROW_EXCEPTION(x)

// Sigil uses the "Exception types as semantic tags" idiom.
// For more information, see this link:
//   http://www.boost.org/doc/libs/1_41_0/libs/exception/doc/exception_types_as_simple_semantic_tags.html

/**
 * The common base for all exceptions
 */
struct ExceptionBase: virtual std::exception, virtual boost::exception {};

/**
* Thrown when a file does not exist.
*/
struct FileDoesNotExist : virtual ExceptionBase {};
typedef boost::error_info< struct file_name, std::string > errinfo_file_name;

/**
 * Thrown when a resource object does not exist.
 */
struct ResourceDoesNotExist : virtual ExceptionBase {};
typedef boost::error_info< struct resource_name, std::string > errinfo_resource_name;

/**
 * Thrown when the book has no HTML files.
 */
struct NoHTMLFiles : virtual ExceptionBase {};

/**
 * Thrown for XML parsing errors.
 */
struct ErrorParsingXML : virtual ExceptionBase {};
typedef boost::error_info< struct error_string, std::string > errinfo_XML_parsing_error_string;
typedef boost::error_info< struct line_number, qint64 > errinfo_XML_parsing_line_number;
typedef boost::error_info< struct column_number, qint64 > errinfo_XML_parsing_column_number;

/**
 * Thrown for content.xml parsing errors.
 */
struct ErrorParsingContentXML : virtual ErrorParsingXML {};
struct ErrorParsingOPF : virtual ErrorParsingXML {};

/**
 * Thrown when an OPF file cannot be found.
 */
struct NoAppropriateOPFFileFound : virtual ExceptionBase {};

/**
 * Wrapper for CZipExceptions.
 */
struct CZipExceptionWrapper : virtual ExceptionBase {};
typedef boost::error_info< struct error_string, std::string > errinfo_zip_info;

/**
 * Thrown when a file cannot be read.
 */
struct CannotReadFile : virtual ExceptionBase {};
typedef boost::error_info< struct error_string, std::string > errinfo_file_fullpath;

/**
 * Thrown when a file cannot be opened.
 */
struct CannotOpenFile : virtual ExceptionBase {};
typedef boost::error_info< struct error_string, std::string > errinfo_file_errorstring;

