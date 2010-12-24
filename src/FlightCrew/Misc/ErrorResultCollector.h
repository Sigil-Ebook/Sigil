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
#ifndef ERRORRESULTCOLLECTOR_H
#define ERRORRESULTCOLLECTOR_H

#include <vector>
#include <xercesc/sax/ErrorHandler.hpp>
namespace XERCES_CPP_NAMESPACE 
{ class SAXParseException; class SAXException; class DOMException; }
namespace xc = XERCES_CPP_NAMESPACE;

#include "Result.h"

namespace FlightCrew
{

class ErrorResultCollector : public xc::ErrorHandler
{
public:

    void warning(    const xc::SAXParseException &exception );

    void error(      const xc::SAXParseException &exception );

    void fatalError( const xc::SAXParseException &exception );

    void resetErrors();

    std::vector<Result> GetResults();

    void AddNewExceptionAsResult( const xc::SAXParseException &exception,
                                  bool xml_error = false );

    void AddNewExceptionAsResult( const xc::SAXException &exception );

    void AddNewExceptionAsResult( const xc::XMLException &exception );

    void AddNewExceptionAsResult( const xc::DOMException &exception );

private:

    std::vector<Result> m_Results;
};

} // namespace FlightCrew

#endif // ERRORRESULTCOLLECTOR_H
