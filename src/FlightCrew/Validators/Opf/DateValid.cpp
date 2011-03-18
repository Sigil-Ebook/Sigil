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
#include "DateValid.h"
#include <ToXercesStringConverter.h>
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>

// We could make it so that some of these use the previous ones,
// but this is more readable.
static const std::string DATE_Y        
    = "\\d{4}";
static const std::string DATE_YM      
    = "\\d{4}-\\d{2}";
static const std::string DATE_YMD   
    = "\\d{4}-\\d{2}-\\d{2}";
static const std::string DATE_YMD_HM
    = "\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}(((\\+|-)\\d{2}:\\d{2})|Z)";
static const std::string DATE_YMD_HMS
    = "\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}(((\\+|-)\\d{2}:\\d{2})|Z)";
static const std::string DATE_YMD_HMSF
    = "\\d{4}-\\d{2}-\\d{2}T\\d{2}:\\d{2}:\\d{2}\\.\\d+(((\\+|-)\\d{2}:\\d{2})|Z)";

static const boost::regex DATE_Y_REGEX( DATE_Y );
static const boost::regex DATE_YM_REGEX( DATE_YM );
static const boost::regex DATE_YMD_REGEX( DATE_YMD );
static const boost::regex DATE_YMD_HM_REGEX( DATE_YMD_HM );
static const boost::regex DATE_YMD_HMS_REGEX( DATE_YMD_HMS );
static const boost::regex DATE_YMD_HMSF_REGEX( DATE_YMD_HMSF );


namespace FlightCrew
{

std::vector< Result > DateValid::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    std::vector< xc::DOMElement* > dates = xe::GetElementsByQName( 
        document, QName( "date", DC_XML_NAMESPACE ) );

    std::vector< Result > results;

    foreach( xc::DOMElement* date, dates )
    {
        std::string date_string = fromX( date->getTextContent() );

        if ( date_string.empty() || !ValidDateString( date_string ) )
        {
            results.push_back(  
                ResultWithNodeLocation( ERROR_OPF_BAD_DATE_VALUE, *date )
                .AddMessageArgument( date_string )
                );
        }
    }

    return results;
}


// For the date format specification, see here:
// http://www.w3.org/TR/NOTE-datetime
bool DateValid::ValidDateString( const std::string &date_string )
{
    return boost::regex_match( date_string, DATE_Y_REGEX )       ||
           boost::regex_match( date_string, DATE_YM_REGEX )      ||
           boost::regex_match( date_string, DATE_YMD_REGEX )     ||
           boost::regex_match( date_string, DATE_YMD_HM_REGEX )  ||
           boost::regex_match( date_string, DATE_YMD_HMS_REGEX ) ||
           boost::regex_match( date_string, DATE_YMD_HMSF_REGEX );
}

} // namespace FlightCrew

