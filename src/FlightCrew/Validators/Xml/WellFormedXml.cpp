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
#include "WellFormedXml.h"
#include "Misc/ErrorResultCollector.h"
#include <xercesc/sax/SAXException.hpp>
#include <LocationAwareDOMParser.h>
#include <ToXercesStringConverter.h>
#include "Misc/Utilities.h"

namespace FlightCrew
{
    

WellFormedXml::WellFormedXml()
{
    // This scanner ignores schemas and DTDs
    parser.useScanner( xc::XMLUni::fgWFXMLScanner );
    parser.setValidationScheme( xc::AbstractDOMParser::Val_Never );
    parser.setDoNamespaces( true );
}


std::vector< Result > WellFormedXml::ValidateFile( const fs::path &filepath )
{
    parser.resetDocumentPool();

    ErrorResultCollector collector;
    parser.setErrorHandler( &collector );

    try
    {
        parser.parse( toX( Util::BoostPathToUtf8Path( filepath ) ) );
    }

    catch ( xc::SAXException& exception )
    {
    	collector.AddNewExceptionAsResult( exception );
    }

    catch ( xc::XMLException& exception )
    {
        collector.AddNewExceptionAsResult( exception );
    }    

    catch ( xc::DOMException& exception )
    {
    	collector.AddNewExceptionAsResult( exception );
    }

    return Util::AddPathToResults( collector.GetResults(), filepath );
}


xc::DOMDocument& WellFormedXml::GetDocument()
{
    return *parser.getDocument();
}

} //namespace FlightCrew
