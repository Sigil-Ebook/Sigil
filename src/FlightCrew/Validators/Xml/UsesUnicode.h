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
#ifndef USESUNICODE_H
#define USESUNICODE_H

#include "../IValidator.h"

namespace FlightCrew
{

class UsesUnicode : public IValidator
{

public:

    std::vector< Result > ValidateFile( const fs::path &filepath );

private:

    /**
     * Checks if the file contains a valid utf8 bytestream.
     *
     * @param filepath The path to the file to check.
     * @return \c true when valid utf8
     */
    bool FileIsValidUtf8( const fs::path &filepath );

    /**
     * Gets the first num_chars characters from the file.
     *
     * @param filepath The path to the file to check.
     * @return The starting characters.
     */
    std::string GetFirstNumCharsFromFile( const fs::path &filepath,
                                          unsigned int num_chars );

    /**
     * Checks if the line contains an utf8 encoding declaration.
     *
     * @param line A line of text, preferably with an xml declaration.
     * @return \c true when line specifies utf8.
     */
    bool FileDeclaresUtf8( const std::string &line );

    /**
     * Checks if the line contains an utf16 encoding declaration.
     *
     * @param line A line of text, preferably with an xml declaration.
     * @return \c true when line specifies utf16.
     */
    bool FileDeclaresUtf16( const std::string &line );

    /**
     * Checks if the line contains an xml declaration.
     *
     * @param line A line of text.
     * @return \c true when line contains an xml declaration.
     */
    bool HasXmlDeclaration( const std::string &line );

    /**
     * Returns the specified encoding in a line containing an xml declaration.
     *
     * @param line A line of text, preferably with an xml declaration.
     * @return The encoding used.
     */
    std::string GetDeclaredEncoding( const std::string &line );

};

} // namespace FlightCrew

#endif // USESUNICODE_H
