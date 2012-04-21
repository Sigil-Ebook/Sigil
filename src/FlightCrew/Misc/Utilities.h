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
#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <string>
#include <algorithm>
#include <boost/unordered/unordered_set_fwd.hpp>
#include "Misc/BoostFilesystemUse.h"
#include "XercesHUse.h"
#include "Result.h"

namespace FlightCrew
{

namespace Util
{
    std::string ReadUnicodFile( const fs::path &filepath );

    std::string GetFirstNumChars( const std::string &string, unsigned int num_chars );

    std::string GetFirstNumCharsFromFile( const fs::path &filepath, unsigned int num_chars );

    int LineOfCharIndex( const std::string &string, unsigned int char_index );

    boost::shared_ptr< xc::DOMDocument > LoadXmlDocument( const fs::path &filepath );

    boost::shared_ptr< xc::DOMDocument > LoadXhtmlDocument( const fs::path &filepath );

    std::string UrlDecode( const std::string &encoded_url );

    std::string GetUrlFragment( const std::string &decoded_url );

    std::string UrlWithoutFragment( const std::string &decoded_url );

    std::string UrlWithoutFileScheme( const std::string &decoded_url );

    fs::path NormalizePath( const fs::path &filepath );

    fs::path Utf8PathToBoostPath( const std::string &utf8_path );

    std::string BoostPathToUtf8Path( const fs::path &filepath );

    std::vector< Result > AddPathToResults( const std::vector< Result > &results, const fs::path &filepath );

    template< typename T >
    void RemoveDuplicates( std::vector<T> &vector )
    {
        std::sort( vector.begin(), vector.end() );
        vector.erase( std::unique( vector.begin(), vector.end() ), vector.end() );
    }

    template< typename T >
    bool Contains( const std::vector<T> &vector, const T &value )
    {
        return std::find( vector.begin(), vector.end(), value ) != vector.end(); 
    }

    template< typename T >
    std::vector<T>& Extend( std::vector<T> &base_vector, const std::vector <T> &extension_vector ) 
    {
            base_vector.insert( base_vector.end(), extension_vector.begin(), extension_vector.end() );
            return base_vector;
    }

    template< typename T >
    std::vector<T>& SortedInPlace( std::vector<T> &vector ) 
    {
        std::sort( vector.begin(), vector.end() );
        return vector;
    }

    // The STL algos for set union and intersection
    // only work for sorted ranges, which boost::unordered_sets aren't.
    template< typename T >
    boost::unordered_set< T > SetIntersection( 
        const boost::unordered_set< T > &first,
        const boost::unordered_set< T > &second )
    {
        boost::unordered_set< T > intersection;

        // We will iterate over the smaller set
        // and check presence in the larger one
        // for the sake of performance.
        if ( second.size() < first.size() )
        
            return SetIntersection( second, first );

        for ( typename boost::unordered_set< T > ::const_iterator it = first.begin();
            it != first.end();
            ++it )
        {
            if ( second.find( *it ) != second.end() )
                    
                intersection.insert( *it );
        }

        return intersection;
    }

    template< typename T >
    boost::unordered_set< T > SetUnion( 
        const boost::unordered_set< T > &first,
        const boost::unordered_set< T > &second )
    {
        boost::unordered_set< T > union_set;
        union_set.insert( first .begin(), first .end() );
        union_set.insert( second.begin(), second.end() );

        return union_set;
    }

    template< typename T >
    boost::unordered_set< T > SetSubtraction(
        const boost::unordered_set< T > &first,
        const boost::unordered_set< T > &second )
    {
        boost::unordered_set< T > subtracted;

        for ( typename boost::unordered_set< T >::const_iterator it = first.begin();
            it != first.end();
            ++it )
        {
            if ( second.find( *it ) == second.end() )
                    
                subtracted.insert( *it );
        }

        return subtracted;
    }

} // namespace Util

} // namespace FlightCrew

#endif // UTILITIES_H
