/************************************************************************
**
**  Copyright (C) 2019  Kevin B. Hendricks, Stratford, Ontario Canada
**  Copyright (C) 2015  John Schember <john@nachtimwald.com>
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#pragma once
#ifndef SG_EXCEPTION_H
#define SG_EXCEPTION_H

#include <exception>
#include <string>

/**
* Thrown when a file does not exist.
*/
class FileDoesNotExist : public std::runtime_error
{
public:
    FileDoesNotExist(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when a resource object does not exist.
 */
class ResourceDoesNotExist : public std::runtime_error {
public:
    ResourceDoesNotExist(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when the book has no HTML files.
 */
class NoHTMLFiles : public std::runtime_error {
public:
    NoHTMLFiles(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown for XML parsing errors.
 */
class ErrorParsingXml : public std::runtime_error {
public:
    ErrorParsingXml(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when a file cannot be read.
 */
class CannotReadFile : public std::runtime_error {
public:
    CannotReadFile(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when a file cannot be opened.
 */
class CannotOpenFile : public std::runtime_error {
public:
    CannotOpenFile(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when a file cannot be put into an archive.
 */
class CannotStoreFile : public std::runtime_error {
public:
    CannotStoreFile(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when a file cannot be copied.
 */
class CannotCopyFile : public std::runtime_error {
public:
    CannotCopyFile(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when a file cannot be written.
 */
class CannotWriteFile : public std::runtime_error {
public:
    CannotWriteFile (const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when the user tries to open a file encrypted with DRM.
 */
class FileEncryptedWithDrm : public std::runtime_error {
public:
    FileEncryptedWithDrm(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown for XML parsing errors.
 */
class FontObfuscationError : public std::runtime_error {
public:
    FontObfuscationError(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown when the document has no root document element.
 */
class ErrorBuildingDOM : public std::runtime_error {
public:
    ErrorBuildingDOM(const std::string &msg) : std::runtime_error(msg) { };
};

/**
 * Thrown for Invalid EPUB errors while loading and parsing content files.
 */
class EPUBLoadParseError : public std::runtime_error {
public:
    EPUBLoadParseError(const std::string &msg) : std::runtime_error(msg) { };
};


/**
 * Thrown for Invalid EPUB errors while loading and parsing content files.
 */
class UNZIPLoadParseError : public std::runtime_error {
public:
    UNZIPLoadParseError(const std::string &msg) : std::runtime_error(msg) { };
};

#endif // SG_EXCEPTION_H
