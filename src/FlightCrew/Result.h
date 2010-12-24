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
#ifndef RESULT_H
#define RESULT_H

#include <string>
#include <vector>
#include <NodeLocationInfo.h>
#include "ResultId.h"
#include "DllExporting.h"

#if defined(_MSC_VER)
// This warning complains that the private members 
// of this class that use the STL are not being exported
// for use by clients.
// Since the clients can't even access the private members 
// of this class, it's not a problem.
#   pragma warning( disable : 4251 )
#endif

namespace FlightCrew
{

/**
 * Defines a validation result, usually a warning or error.
 * Users that just want to consume a prepared Result should 
 * look at the GetErrorLine, GetErrorColumn and GetMessage 
 * member functions.
 */
class FC_WIN_DLL_API Result
{

public:

    /**
     * Constructor.
     *
     * @param result_id The ID of the Result.
     * @param node_location The DOM node location where the result was found.
     */
    Result( ResultId result_id = ALL_OK,
            XercesExt::NodeLocationInfo node_location = XercesExt::NodeLocationInfo() );

    /**
     * Returns the type of the Result, either a warning or an error.
     *
     * @return The type of the Result.
     */
    ResultType GetResultType() const;

    /**
     * Returns the ID of the Result.
     *
     * @return The ID.
     */
    ResultId GetResultId() const;

    /**
     * Sets the Result's ID.
     *
     * @param result_id The new ID.
     * @return A reference to this result, for easy function chaining.
     */
    Result& SetResultId( ResultId result_id );

    /**
     * Returns the error line number.
     *
     * @return The line number.
     */
    int GetErrorLine() const;

    /**
     * Sets the Result's error line number.
     *
     * @param result_id The new line number.
     * @return A reference to this result, for easy function chaining.
     */
    Result& SetErrorLine( int error_line );

    /**
     * Returns the error column number.
     *
     * @return The column number.
     */
    int GetErrorColumn() const;

    /**
     * Sets the Result's error column number.
     * @note This is usually unreliable information because of the way
     * Xerces works with XSD's. It's going to be in the ballpark, but
     * it won't (usually) have the precision you want.
     *
     * @param result_id The new column number.
     * @return A reference to this result, for easy function chaining.
     */
    Result& SetErrorColumn( int error_column );

    /**
     * Returns the path to the file in which this Result occurs.
     * The path is relative to the root of the epub document.
     * 
     * @return The path in UTF-8.
     */
    std::string GetFilepath() const;

    /**
     * Sets the path to the file in which this Result occurs.
     * The path should be relative to the root of the epub document.
     *
     * @param filepath The new path in UTF-8.
     * @return A reference to this result, for easy function chaining.
     */
    Result& SetFilepath( const std::string &filepath );

    /**
     * Adds a message argument that fills in a placeholder in the  
     * message that applies to this Result's ID. The order in which
     * the arguments are added is the order in which they will replace
     * the placeholders.
     *
     * @param message_argument The argument in UTF-8.
     * @return A reference to this result, for easy function chaining.
     */
    Result& AddMessageArgument( const std::string &message_argument );

    /**
     * Sets all the message arguments that will fill in the placeholders
     * in the message that applies to this Result's ID. The order in which
     * the arguments are added is the order in which they will replace
     * the placeholders.
     *
     * @param message_arguments The arguments in UTF-8.
     * @return A reference to this result, for easy function chaining.
     */
    Result& SetMessageArguments( const std::vector< std::string > &message_arguments );

    /**
     * Returns all the stored message arguments.
     *
     * @return The current message arguments.
     */
    const std::vector< std::string > &GetMessageArguments() const;

    /**
     * Returns the error message for this Result with all the
     * message arguments already applied.
     */
    std::string GetMessage() const;

    /**
     * Sets a custom message for this Result. This overrides the template
     * message for Results with this ID and ignores all stored message arguments.
     */
    Result& SetCustomMessage( const std::string &custom_message );
    
    bool operator< ( const Result& other ) const;

    bool operator== ( const Result& other ) const;

private:

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The Result's ID.
     */
    ResultId m_ResultId;

    /**
     * The line where this Result was found in the content document.
     */
    int m_ErrorLine;

    /**
     * The column in the line where this Result was found in the content document.
     */
    int m_ErrorColumn;

    /**
     * The message arguments for the placeholders in the message template.
     */
    std::vector< std::string > m_MessageArguments;

    /**
     * A custom message that overrides the template one.
     */
    std::string m_CustomMessage;

    /**
     * The relative path to the file where the Result was found.
     */
    std::string m_Filepath;
};

} // namespace FlightCrew

#endif // RESULT_H
