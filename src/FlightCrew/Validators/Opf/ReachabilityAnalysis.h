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
#ifndef REACHABILITYANALYSIS_H
#define REACHABILITYANALYSIS_H

#include "../XmlValidator.h"
#include <boost/unordered/unordered_map_fwd.hpp>
#include <boost/unordered/unordered_set_fwd.hpp>

namespace FlightCrew
{


class ReachabilityAnalysis : public XmlValidator
{
public:

    virtual std::vector<Result> ValidateXml( 
        const xc::DOMDocument &document,
        const fs::path &filepath = fs::path() );

private:

    std::vector<Result> ResultsForOpsDocsNotInSpine( 
        const xc::DOMDocument &document,
        const boost::unordered_map< std::string, fs::path > &manifest_items,
        const boost::unordered_set< fs::path > &reachable_resources );

    std::vector<Result> ResultsForResourcesNotInManifest( 
        const boost::unordered_map< std::string, fs::path > &manifest_items,
        const boost::unordered_set< fs::path > &reachable_resources );

    std::vector<Result> ResultsForUnusedResources(
        const boost::unordered_map< std::string, fs::path > &manifest_items,
        const boost::unordered_set< fs::path > &reachable_resources );

    bool AllowedToBeNotReachable( const fs::path &filepath );

    boost::unordered_map< std::string, fs::path > GetManifestItems( 
        const xc::DOMDocument &document,
        const fs::path &opf_folder_path );

    boost::unordered_set< fs::path > StartingSetOpsPaths( 
        const xc::DOMDocument &document,
        const boost::unordered_map< std::string, fs::path > &manifest_items,
        const fs::path &opf_folder_path );

    boost::unordered_set< fs::path > SpinePaths( 
        const xc::DOMDocument &document,
        const boost::unordered_map< std::string, fs::path > &manifest_items );

    boost::unordered_set< fs::path > GuidePaths( 
        const xc::DOMDocument &document,
        const fs::path &opf_folder_path );

    boost::unordered_set< fs::path > ToursPaths( 
        const xc::DOMDocument &document,
        const fs::path &opf_folder_path );

    fs::path GetPathToNcx( 
        const xc::DOMDocument &document, 
        const boost::unordered_map< std::string, fs::path > &manifest_items );

    boost::unordered_set< fs::path > NcxPaths( 
        const xc::DOMDocument &document,
        const boost::unordered_map< std::string, fs::path > &manifest_items );

    boost::unordered_set< fs::path > DetermineReachableResources( 
        const boost::unordered_set< fs::path > &starting_ops_paths );

    boost::unordered_set< fs::path > GetDirectlyReachable( 
        const boost::unordered_set< fs::path > &resources );

    boost::unordered_set< fs::path > GetOnlyOpsDocs( 
        const boost::unordered_set< fs::path > &resources );

    boost::unordered_set< fs::path > GetOnlyCssDocs( 
        const boost::unordered_set< fs::path > &resources );

    boost::unordered_set< fs::path > GetLinkedResourcesFromAllOps( 
        const boost::unordered_set< fs::path > &ops_docs );

    boost::unordered_set< fs::path > GetLinkedResourcesFromAllCss( 
        const boost::unordered_set< fs::path > &css_docs );

    boost::unordered_set< fs::path > GetLinkedResourcesFromOps(
        const fs::path &ops_document );   

    boost::unordered_set< fs::path > GetLinkedResourcesFromCss(
        const fs::path &css_document );

    boost::unordered_set< fs::path > GetPathsFromItems( 
        const boost::unordered_map< std::string, fs::path > &manifest_items );

    bool IsFilesystemPath( const fs::path &path );
};

} // namespace FlightCrew

#endif // REACHABILITYANALYSIS_H

