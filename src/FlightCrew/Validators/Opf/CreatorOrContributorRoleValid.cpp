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
#include "CreatorOrContributorRoleValid.h"
#include <FromXercesStringConverter.h>
#include <ToXercesStringConverter.h>
#include <XmlUtils.h>
#include "Misc/Utilities.h"

namespace FlightCrew
{

std::vector<Result> CreatorOrContributorRoleValid::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    std::vector< xc::DOMElement* > elements = xe::GetElementsByQName( 
        document, QName( "creator", DC_XML_NAMESPACE ) );

    Util::Extend< xc::DOMElement* >( elements, xe::GetElementsByQName( 
        document, QName( "contributor", DC_XML_NAMESPACE ) ) );

    std::vector<Result> results;
    boost::unordered_set< std::string > relators = GetRelatorSet();

    foreach( xc::DOMElement* element, elements )
    {
        std::string role = fromX( element->getAttributeNS( toX( OPF_XML_NAMESPACE ), toX( "role" ) ) );
        
        // The "role" is not a required attribute and
        // can thus be empty.
        if ( !role.empty() &&
             relators.count( role ) == 0 &&
             !boost::starts_with( role, "oth." ) )
        {
            results.push_back( 
                ResultWithNodeLocation( ERROR_OPF_BAD_CREATOR_OR_CONTRIBUTOR_ROLE_VALUE, *element )
                .AddMessageArgument( role )
                );
        }
    }

    return results;
}

boost::unordered_set< std::string > CreatorOrContributorRoleValid::GetRelatorSet()
{
    boost::unordered_set< std::string > relators;
    relators.insert( "act" );
    relators.insert( "adp" );
    relators.insert( "anl" );
    relators.insert( "anm" );
    relators.insert( "ann" );
    relators.insert( "app" );
    relators.insert( "arc" );
    relators.insert( "arr" );
    relators.insert( "acp" );
    relators.insert( "art" );
    relators.insert( "ard" );
    relators.insert( "asg" );
    relators.insert( "asn" );
    relators.insert( "att" );
    relators.insert( "auc" );
    relators.insert( "aut" );
    relators.insert( "aqt" );
    relators.insert( "aft" );
    relators.insert( "aud" );
    relators.insert( "aui" );
    relators.insert( "aus" );
    relators.insert( "ant" );
    relators.insert( "bnd" );
    relators.insert( "bdd" );
    relators.insert( "bkd" );
    relators.insert( "bkp" );
    relators.insert( "bjd" );
    relators.insert( "bpd" );
    relators.insert( "bsl" );
    relators.insert( "cll" );
    relators.insert( "ctg" );
    relators.insert( "cns" );
    relators.insert( "chr" );
    relators.insert( "cng" );
    relators.insert( "cli" );
    relators.insert( "clb" );
    relators.insert( "col" );
    relators.insert( "clt" );
    relators.insert( "cmm" );
    relators.insert( "cwt" );
    relators.insert( "com" );
    relators.insert( "cpl" );
    relators.insert( "cpt" );
    relators.insert( "cpe" );
    relators.insert( "cmp" );
    relators.insert( "cmt" );
    relators.insert( "ccp" );
    relators.insert( "cnd" );
    relators.insert( "csl" );
    relators.insert( "csp" );
    relators.insert( "cos" );
    relators.insert( "cot" );
    relators.insert( "coe" );
    relators.insert( "cts" );
    relators.insert( "ctt" );
    relators.insert( "cte" );
    relators.insert( "ctr" );
    relators.insert( "ctb" );
    relators.insert( "cpc" );
    relators.insert( "cph" );
    relators.insert( "crr" );
    relators.insert( "crp" );
    relators.insert( "cst" );
    relators.insert( "cov" );
    relators.insert( "cre" );
    relators.insert( "cur" );
    relators.insert( "dnc" );
    relators.insert( "dtc" );
    relators.insert( "dtm" );
    relators.insert( "dte" );
    relators.insert( "dto" );
    relators.insert( "dfd" );
    relators.insert( "dft" );
    relators.insert( "dfe" );
    relators.insert( "dgg" );
    relators.insert( "dln" );
    relators.insert( "dpc" );
    relators.insert( "dpt" );
    relators.insert( "dsr" );
    relators.insert( "drt" );
    relators.insert( "dis" );
    relators.insert( "dst" );
    relators.insert( "dnr" );
    relators.insert( "drm" );
    relators.insert( "dub" );
    relators.insert( "edt" );
    relators.insert( "elg" );
    relators.insert( "elt" );
    relators.insert( "eng" );
    relators.insert( "egr" );
    relators.insert( "etr" );
    relators.insert( "exp" );
    relators.insert( "fac" );
    relators.insert( "fld" );
    relators.insert( "flm" );
    relators.insert( "fpy" );
    relators.insert( "frg" );
    relators.insert( "fmo" );
    relators.insert( "fnd" );
    relators.insert( "gis" );
    relators.insert( "hnr" );
    relators.insert( "hst" );
    relators.insert( "ilu" );
    relators.insert( "ill" );
    relators.insert( "ins" );
    relators.insert( "itr" );
    relators.insert( "ive" );
    relators.insert( "ivr" );
    relators.insert( "inv" );
    relators.insert( "lbr" );
    relators.insert( "ldr" );
    relators.insert( "lsa" );
    relators.insert( "led" );
    relators.insert( "len" );
    relators.insert( "lil" );
    relators.insert( "lit" );
    relators.insert( "lie" );
    relators.insert( "lel" );
    relators.insert( "let" );
    relators.insert( "lee" );
    relators.insert( "lbt" );
    relators.insert( "lse" );
    relators.insert( "lso" );
    relators.insert( "lgd" );
    relators.insert( "ltg" );
    relators.insert( "lyr" );
    relators.insert( "mfr" );
    relators.insert( "mrk" );
    relators.insert( "mdc" );
    relators.insert( "mte" );
    relators.insert( "mod" );
    relators.insert( "mon" );
    relators.insert( "mcp" );
    relators.insert( "msd" );
    relators.insert( "mus" );
    relators.insert( "nrt" );
    relators.insert( "opn" );
    relators.insert( "orm" );
    relators.insert( "org" );
    relators.insert( "oth" );
    relators.insert( "own" );
    relators.insert( "ppm" );
    relators.insert( "pta" );
    relators.insert( "pth" );
    relators.insert( "pat" );
    relators.insert( "prf" );
    relators.insert( "pma" );
    relators.insert( "pht" );
    relators.insert( "ptf" );
    relators.insert( "ptt" );
    relators.insert( "pte" );
    relators.insert( "plt" );
    relators.insert( "prt" );
    relators.insert( "pop" );
    relators.insert( "prm" );
    relators.insert( "prc" );
    relators.insert( "pro" );
    relators.insert( "pmn" );
    relators.insert( "prd" );
    relators.insert( "prg" );
    relators.insert( "pdr" );
    relators.insert( "pfr" );
    relators.insert( "pbl" );
    relators.insert( "pbd" );
    relators.insert( "ppt" );
    relators.insert( "rcp" );
    relators.insert( "rce" );
    relators.insert( "red" );
    relators.insert( "ren" );
    relators.insert( "rpt" );
    relators.insert( "rps" );
    relators.insert( "rth" );
    relators.insert( "rtm" );
    relators.insert( "res" );
    relators.insert( "rsp" );
    relators.insert( "rst" );
    relators.insert( "rse" );
    relators.insert( "rpy" );
    relators.insert( "rsg" );
    relators.insert( "rev" );
    relators.insert( "rbr" );
    relators.insert( "sce" );
    relators.insert( "sad" );
    relators.insert( "scr" );
    relators.insert( "scl" );
    relators.insert( "spy" );
    relators.insert( "sec" );
    relators.insert( "std" );
    relators.insert( "sgn" );
    relators.insert( "sng" );
    relators.insert( "sds" );
    relators.insert( "spk" );
    relators.insert( "spn" );
    relators.insert( "stm" );
    relators.insert( "stn" );
    relators.insert( "str" );
    relators.insert( "stl" );
    relators.insert( "sht" );
    relators.insert( "srv" );
    relators.insert( "tch" );
    relators.insert( "tcd" );
    relators.insert( "ths" );
    relators.insert( "trc" );
    relators.insert( "trl" );
    relators.insert( "tyd" );
    relators.insert( "tyg" );
    relators.insert( "vdg" );
    relators.insert( "voc" );
    relators.insert( "wit" );
    relators.insert( "wde" );
    relators.insert( "wdc" );
    relators.insert( "wam" );

    return relators;
}

} // namespace FlightCrew

