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
#include "ErrorResultCollector.h"
#include <FromXercesStringConverter.h>
#include <xercesc/sax/SAXParseException.hpp>

namespace FlightCrew
{

void ErrorResultCollector::warning( const xc::SAXParseException &exception )
{
    AddNewExceptionAsResult( exception );
}


void ErrorResultCollector::error( const xc::SAXParseException &exception )
{
    AddNewExceptionAsResult( exception );
}


void ErrorResultCollector::fatalError( const xc::SAXParseException &exception )
{
    AddNewExceptionAsResult( exception, true );
}


void ErrorResultCollector::resetErrors()
{
    m_Results.clear();
}


std::vector< Result > ErrorResultCollector::GetResults()
{
    return m_Results;
}


void ErrorResultCollector::AddNewExceptionAsResult( const xc::SAXParseException &exception,
                                                    bool xml_error )
{
    ResultId id = xml_error ? ERROR_XML_NOT_WELL_FORMED : ERROR_SCHEMA_NOT_SATISFIED;

    m_Results.push_back( Result().SetCustomMessage( fromX( exception.getMessage() ) )
                                 .SetErrorColumn( (int) exception.getColumnNumber() )
                                 .SetErrorLine( (int) exception.getLineNumber() )
                                 .SetResultId( id )
                       );
}


void ErrorResultCollector::AddNewExceptionAsResult( const xc::SAXException &exception )
{
    m_Results.push_back( Result().SetCustomMessage( fromX( exception.getMessage() ) )
                                 .SetResultId( ERROR_GENERIC )
                       );
}


void ErrorResultCollector::AddNewExceptionAsResult( const xc::XMLException &exception )
{
    m_Results.push_back( Result().SetCustomMessage( fromX( exception.getMessage() ) )
                                 .SetErrorLine( (int) exception.getSrcLine() )
                                 .SetResultId( ERROR_GENERIC )
                       );
}


void ErrorResultCollector::AddNewExceptionAsResult( const xc::DOMException &exception )
{
    m_Results.push_back( Result().SetCustomMessage( fromX( exception.getMessage() ) )
                                 .SetResultId( ERROR_GENERIC )
                       );
}

} //namespace FlightCrew