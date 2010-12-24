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
#include "DomSchemaValidator.h"
#include "Misc/ErrorResultCollector.h"
#include <ToXercesStringConverter.h>
#include <LocationAwareDOMParser.h>
#include <xercesc/sax/SAXException.hpp>
#include "Misc/Utilities.h"

namespace FlightCrew
{
    
std::vector<Result> DomSchemaValidator::ValidateAgainstSchema(
    const fs::path &filepath,
    const std::string &external_schema_location,
    const std::vector< const xc::MemBufInputSource* > &schemas,
    const std::vector< const xc::MemBufInputSource* > &dtds )
{
    xe::LocationAwareDOMParser parser;

    parser.setDoSchema(             true  );
    parser.setLoadSchema(           false );
    parser.setSkipDTDValidation(    true  );
    parser.setDoNamespaces(         true  );
    parser.useCachedGrammarInParse( true  );  

    parser.setValidationScheme( xc::AbstractDOMParser::Val_Always ); 

    LoadSchemas( parser, external_schema_location, schemas, dtds );

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

    return Util::AddPathToResults( collector.GetResults(), filepath );
}


void DomSchemaValidator::LoadSchemas( 
    xe::LocationAwareDOMParser &parser,
    const std::string &external_schema_location,
    const std::vector< const xc::MemBufInputSource* > &schemas,
    const std::vector< const xc::MemBufInputSource* > &dtds )
{
    foreach( const xc::MemBufInputSource *input, dtds )
    {
        parser.loadGrammar( *input, xc::Grammar::DTDGrammarType,    true );   
    }

    foreach( const xc::MemBufInputSource *input, schemas )
    {
        parser.loadGrammar( *input, xc::Grammar::SchemaGrammarType, true );  
    }

    parser.setExternalSchemaLocation( external_schema_location.c_str() );
}

} //namespace FlightCrew
