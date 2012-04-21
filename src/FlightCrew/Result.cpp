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

#include <stdafx.h>
#include "Result.h"
#include "ErrorMessages.h"

namespace FlightCrew
{

Result::Result( ResultId result_id, xe::NodeLocationInfo node_location )
    :
    m_ResultId( result_id ),
    m_ErrorLine( node_location.LineNumber ),
    m_ErrorColumn( node_location.ColumnNumber )
{

}

ResultType Result::GetResultType() const
{
    if ( static_cast< int >( m_ResultId ) < static_cast< int >( ResultType_WARNING ) )

        return ResultType_ERROR;

    return ResultType_WARNING;
}


ResultId Result::GetResultId() const
{
    return m_ResultId;
}


Result& Result::SetResultId( ResultId result_id )
{
    m_ResultId = result_id;
    return *this;
}


int Result::GetErrorLine() const
{
    return m_ErrorLine;
}


Result& Result::SetErrorLine( int error_line )
{
    m_ErrorLine = error_line;
    return *this;
}


int Result::GetErrorColumn() const
{
    return m_ErrorColumn;
}


Result& Result::SetErrorColumn( int error_line )
{
    m_ErrorColumn = error_line;
    return *this;
}


std::string Result::GetFilepath() const
{
    return m_Filepath;
}


Result& Result::SetFilepath( const std::string &filepath )
{
    m_Filepath = filepath;
    return *this;
}


Result& Result::AddMessageArgument( const std::string &message_argument )
{
    m_MessageArguments.push_back( message_argument );
    return *this;
}


Result& Result::SetMessageArguments( const std::vector< std::string > &message_arguments )
{
    m_MessageArguments = message_arguments;
    return *this;
}


const std::vector< std::string > & Result::GetMessageArguments() const
{
    return m_MessageArguments;
}


std::string Result::GetMessage() const
{
    if ( !m_CustomMessage.empty() )

        return m_CustomMessage;

    boost::format formatter( ErrorMessages::Instance().MessageForId( m_ResultId ) );

    foreach( std::string argument, m_MessageArguments )
    {
        formatter % argument;
    }

    return formatter.str();
}


Result& Result::SetCustomMessage( const std::string &custom_message )
{
    m_CustomMessage = custom_message;
    return *this;
}


bool Result::operator< ( const Result& other ) const
{
    // Yes, this is ugly but it also needs to be fast.
    // We need to make sure that all private vars are
    // included because some STL algos uses two "<" 
    // operations to check for equality. Since this is
    // called freaking everywhere, we have to make sure
    // only the required comparisons are made, and no more.

    return
        m_Filepath != other.m_Filepath ?
        m_Filepath <  other.m_Filepath :
            m_ErrorLine != other.m_ErrorLine ? 
            m_ErrorLine <  other.m_ErrorLine :
                m_ErrorColumn != other.m_ErrorColumn ? 
                m_ErrorColumn <  other.m_ErrorColumn :
                    m_ResultId != other.m_ResultId ? 
                    m_ResultId <  other.m_ResultId :
                        m_CustomMessage != other.m_CustomMessage ? 
                        m_CustomMessage <  other.m_CustomMessage :
                            m_MessageArguments < other.m_MessageArguments;
}


bool Result::operator==( const Result& other ) const
{
    return
        m_ResultId         == other.m_ResultId      &&
        m_ErrorLine        == other.m_ErrorLine     &&
        m_ErrorColumn      == other.m_ErrorColumn   &&        
        m_Filepath         == other.m_Filepath      &&
        m_CustomMessage    == other.m_CustomMessage &&
        m_MessageArguments == other.m_MessageArguments;
}


} // namespace FlightCrew


