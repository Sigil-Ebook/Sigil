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

/**
 * Reports errors with files that are unmanifested or unused and OPS
 * docs not in the OPF <spine>.
 */
class ReachabilityAnalysis : public XmlValidator
{
public:
    
    // inherited
    virtual std::vector< Result > ValidateXml( 
        const xc::DOMDocument &document,
        const fs::path &filepath = fs::path() );

private:

    /**
     * Returns validation results for problems with OPS docs not present in the OPF spine.
     *
     * @param document The OPF document.
     * @param manifest_items A map of manifest items. The keys are the IDs, the
     *                       values are full paths to those files.
     * @param reachable_resources A set of full paths to all the reachable files.
     * @return The validation results.
     */
    std::vector< Result > ResultsForOpsDocsNotInSpine( 
        const xc::DOMDocument &document,
        const boost::unordered_map< std::string, fs::path > &manifest_items,
        const boost::unordered_set< fs::path > &reachable_resources );

    /**
     * Returns validation results for problems with resources not present in the OPF manifest.
     *
     * @param document The OPF document.
     * @param manifest_items A map of manifest items. The keys are the IDs, the
     *                       values are full paths to those files.
     * @param reachable_resources A set of full paths to all the reachable files.
     * @return The validation results.
     */
    std::vector< Result > ResultsForResourcesNotInManifest( 
        const boost::unordered_map< std::string, fs::path > &manifest_items,
        const boost::unordered_set< fs::path > &reachable_resources );

    /**
     * Returns validation results for problems with resources that are unused but present
     * in the OPF manifest.
     *
     * @param document The OPF document.
     * @param manifest_items A map of manifest items. The keys are the IDs, the
     *                       values are full paths to those files.
     * @param reachable_resources A set of full paths to all the reachable files.
     * @return The validation results.
     */
    std::vector< Result > ResultsForUnusedResources(
        const boost::unordered_map< std::string, fs::path > &manifest_items,
        const boost::unordered_set< fs::path > &reachable_resources );

    /**
     * Determines if the specified file is allowed to be unreachable.
     * 
     * @param filepath The file to analyze.
     * @return \c true if the file is allowed to be unreachable.
     */
    bool AllowedToBeNotReachable( const fs::path &filepath );

    /**
     * Returns an ID->fullpath mapping of the manifest items in the OPF document. 
     *
     * @param document The OPF document.
     * @param opf_folder_path The path to the folder in which the OPF file resides.
     * @return The manifest items.
     */
    boost::unordered_map< std::string, fs::path > GetManifestItems( 
        const xc::DOMDocument &document,
        const fs::path &opf_folder_path );

    /**
     * Returns a set of paths to the starting OPS documents. The starting docs are
     * the ones that are explicitly listed in the OPF or NCX in some way.
     *
     * @param document The OPF document.
     * @param manifest_items A map of manifest items. The keys are the IDs, the
     *                       values are full paths to those files.
     * @param opf_folder_path The path to the folder in which the OPF file resides.
     * @return The starting OPS paths.
     */
    boost::unordered_set< fs::path > StartingSetOpsPaths( 
        const xc::DOMDocument &document,
        const boost::unordered_map< std::string, fs::path > &manifest_items,
        const fs::path &opf_folder_path );

    /**
     * Returns a set of paths listed in the OPF <spine>.
     *
     * @param document The OPF document.
     * @param manifest_items A map of manifest items. The keys are the IDs, the
     *                       values are full paths to those files.
     * @return The spine paths.
     */
    boost::unordered_set< fs::path > SpinePaths( 
        const xc::DOMDocument &document,
        const boost::unordered_map< std::string, fs::path > &manifest_items );

    /**
     * Returns a set of paths listed in the OPF <guide>.
     *
     * @param document The OPF document.
     * @param opf_folder_path The path to the folder in which the OPF file resides.
     * @return The guide paths.
     */
    boost::unordered_set< fs::path > GuidePaths( 
        const xc::DOMDocument &document,
        const fs::path &opf_folder_path );

    /**
     * Returns a set of paths listed in the OPF <tours>.
     *
     * @param document The OPF document.
     * @param opf_folder_path The path to the folder in which the OPF file resides.
     * @return The tours paths.
     */
    boost::unordered_set< fs::path > ToursPaths( 
        const xc::DOMDocument &document,
        const fs::path &opf_folder_path );

    /**
     * Returns a path to the NCX file.
     *
     * @param document The OPF document.
     * @param manifest_items A map of manifest items. The keys are the IDs, the
     *                       values are full paths to those files.
     * @return The path to the NCX.
     */
    fs::path GetPathToNcx( 
        const xc::DOMDocument &document, 
        const boost::unordered_map< std::string, fs::path > &manifest_items );

    /**
     * Returns a set of paths listed in the NCX.
     *
     * @param document The OPF document.
     * @param manifest_items A map of manifest items. The keys are the IDs, the
     *                       values are full paths to those files.
     * @return The NCX paths.
     */
    boost::unordered_set< fs::path > NcxPaths( 
        const xc::DOMDocument &document,
        const boost::unordered_map< std::string, fs::path > &manifest_items );

    /**
     * Returns a set of full paths to all the reachable resources.
     * 
     * @param starting_ops_paths The paths to the starting OPS documents.
     * @return All the reachable resources.
     */
    boost::unordered_set< fs::path > DetermineReachableResources( 
        const boost::unordered_set< fs::path > &starting_ops_paths );

    /**
     * Returns a list of all the resources that are directly reachable
     * (i.e. "one step away") from the provided resources.
     * 
     * @param resources The resources from which reachability of new resources 
     *                  will be determined.
     * @return The directly reachable resources.
     */
    boost::unordered_set< fs::path > GetDirectlyReachableResources( 
        const boost::unordered_set< fs::path > &resources );

    /**
     * From the set of provided resources, returns only the OPS ones.
     * 
     * @param resources The resource set.
     * @return The OPS resources.
     */
    boost::unordered_set< fs::path > GetOnlyOpsDocs( 
        const boost::unordered_set< fs::path > &resources );

    /**
     * From the set of provided resources, returns only the CSS ones.
     * 
     * @param resources The resource set.
     * @return The CSS resources.
     */
    boost::unordered_set< fs::path > GetOnlyCssDocs( 
        const boost::unordered_set< fs::path > &resources );

    /**
     * For all the provided OPS docs, returns a set of all reachable resources.
     * 
     * @param resources The OPS path set.
     * @return The reachable resources.
     */
    boost::unordered_set< fs::path > GetLinkedResourcesFromAllOps( 
        const boost::unordered_set< fs::path > &ops_docs );

    /**
     * For all the provided CSS docs, returns a set of all reachable resources.
     * 
     * @param resources The OPS path set.
     * @return The reachable resources.
     */
    boost::unordered_set< fs::path > GetLinkedResourcesFromAllCss( 
        const boost::unordered_set< fs::path > &css_docs );

    /**
     * For the provided OPS doc, returns a set of all reachable resources.
     * 
     * @param resources The OPS path.
     * @return The reachable resources.
     */
    boost::unordered_set< fs::path > GetLinkedResourcesFromOps(
        const fs::path &ops_document );   

    /**
     * For the provided CSS doc, returns a set of all reachable resources.
     * 
     * @param resources The CSS path.
     * @return The reachable resources.
     */
    boost::unordered_set< fs::path > GetLinkedResourcesFromCss(
        const fs::path &css_document );

    /**
     * Extracts the paths from map into a set.
     * 
     * @param manifest_items A map of manifest items. The keys are the IDs, the
     *                       values are full paths to those files.
     * @return A set of all the paths.
     */
    boost::unordered_set< fs::path > GetPathsFromItems( 
        const boost::unordered_map< std::string, fs::path > &manifest_items );

    /**
     * Examines if the provided path points to a file on the filesystem.
     * (as opposed to a file on the Internet).
     *
     * @param path The path to inspect.
     * @return \c true if the path is a filesystem path.
     */
    bool IsFilesystemPath( const fs::path &path );
};

} // namespace FlightCrew

#endif // REACHABILITYANALYSIS_H

