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
#include "SaxSchemaValidator.h"
#include "Misc/ErrorResultCollector.h"
#include <ToXercesStringConverter.h>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include "Misc/Utilities.h"

namespace FlightCrew
{
    
std::vector< Result > SaxSchemaValidator::ValidateAgainstSchema(
    const fs::path &filepath,
    const std::string &external_schema_location,
    const std::vector< const xc::MemBufInputSource* > &schemas )
{
    boost::scoped_ptr< xc::SAX2XMLReader > parser( xc::XMLReaderFactory::createXMLReader() );

    parser->setFeature( xc::XMLUni::fgSAX2CoreValidation,            true  );
    parser->setFeature( xc::XMLUni::fgXercesLoadSchema,              false );
    parser->setFeature( xc::XMLUni::fgXercesUseCachedGrammarInParse, true  );
    parser->setFeature( xc::XMLUni::fgXercesSkipDTDValidation,       true  );

    // We don't need DTD validation
    parser->setProperty( xc::XMLUni::fgXercesScannerName, 
                         (void*) xc::XMLUni::fgSGXMLScanner );    

    LoadSchemas( *parser, external_schema_location, schemas );

    ErrorResultCollector collector;
    parser->setErrorHandler( &collector );

    try
    {
        parser->parse( filepath.string().c_str() );
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


void SaxSchemaValidator::LoadSchemas( 
    xc::SAX2XMLReader &parser,
    const std::string &external_schema_location,
    const std::vector< const xc::MemBufInputSource* > &schemas )
{
   
    foreach( const xc::MemBufInputSource *input, schemas )
    {
        parser.loadGrammar( *input, xc::Grammar::SchemaGrammarType, true );  
    }

    parser.setProperty( xc::XMLUni::fgXercesSchemaExternalSchemaLocation, 
                        (void*) toX( external_schema_location ) );
}

} //namespace FlightCrew
