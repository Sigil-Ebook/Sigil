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
#include "MetadataAllowedChildren.h"
#include <ToXercesStringConverter.h>
#include <FromXercesStringConverter.h>
#include <XmlUtils.h>

namespace FlightCrew
{

// These have to go here; otherwise, we get the static initialization order fiasco
const std::string MAIN_XML_NAMESPACE = "http://www.w3.org/XML/1998/namespace"; 
const std::string OPF_XML_NAMESPACE  = "http://www.idpf.org/2007/opf"; 
const std::string DC_XML_NAMESPACE   = "http://purl.org/dc/elements/1.1/";
const std::string NCX_XML_NAMESPACE  = "http://www.daisy.org/z3986/2005/ncx/";
    
const QName DC_METADATA_QNAME = QName( "dc-metadata", OPF_XML_NAMESPACE );
const QName X_METADATA_QNAME  = QName( "x-metadata",  OPF_XML_NAMESPACE );

const QName TITLE_QNAME       = QName( "title",       DC_XML_NAMESPACE  );
const QName LANGUAGE_QNAME    = QName( "language",    DC_XML_NAMESPACE  );
const QName IDENTIFIER_QNAME  = QName( "identifier",  DC_XML_NAMESPACE  );
const QName CREATOR_QNAME     = QName( "creator",     DC_XML_NAMESPACE  );
const QName SUBJECT_QNAME     = QName( "subject",     DC_XML_NAMESPACE  );
const QName DESCRIPTION_QNAME = QName( "description", DC_XML_NAMESPACE  );
const QName PUBLISHER_QNAME   = QName( "publisher",   DC_XML_NAMESPACE  );
const QName CONTRIBUTOR_QNAME = QName( "contributor", DC_XML_NAMESPACE  );
const QName DATE_QNAME        = QName( "date",        DC_XML_NAMESPACE  );
const QName TYPE_QNAME        = QName( "type",        DC_XML_NAMESPACE  );
const QName FORMAT_QNAME      = QName( "format",      DC_XML_NAMESPACE  );
const QName SOURCE_QNAME      = QName( "source",      DC_XML_NAMESPACE  );
const QName RELATION_QNAME    = QName( "relation",    DC_XML_NAMESPACE  );
const QName COVERAGE_QNAME    = QName( "coverage",    DC_XML_NAMESPACE  );
const QName RIGHTS_QNAME      = QName( "rights",      DC_XML_NAMESPACE  );
const QName META_QNAME        = QName( "meta",        OPF_XML_NAMESPACE );


std::vector<Result> MetadataAllowedChildren::ValidateXml(
    const xc::DOMDocument &document,
    const fs::path& )
{
    xc::DOMNodeList *metadatas = document.getElementsByTagNameNS(
                                    toX( OPF_XML_NAMESPACE ),  toX( "metadata" ) );

    std::vector<Result> results;

    if ( metadatas->getLength() < 1 )
    
        return results;    

    xc::DOMElement* metadata = static_cast< xc::DOMElement* >( metadatas->item( 0 ) );
    std::vector< xc::DOMElement* > children = xe::GetElementChildren( *metadata );

    // A <metadata> element can have either a dc-metadata element and an optional
    // x-metadata element, OR the standard children (title, creator, language etc.)
    // plus any other child not in the reserved namespaces.
    // See the OPF schema: 
    // http://www.idpf.org/doc_library/epub/OPF_2.0.1_draft.htm#AppendixA

    if ( xe::ElementListContains( children, DC_METADATA_QNAME ) ||
         xe::ElementListContains( children, X_METADATA_QNAME  ) )
    {
        std::vector<Result> subresults = ValidateDCXChildrenSubset( children );
        results.insert( results.end(), subresults.begin(), subresults.end() );
    }

    else
    {
        std::vector<Result> subresults = ValidateStandardChildren( children );
        results.insert( results.end(), subresults.begin(), subresults.end() );
    }

    return results;
}


std::vector<Result> MetadataAllowedChildren::ValidateDCXChildrenSubset(
    std::vector< xc::DOMElement* > children )
{
    std::vector<Result> results;

    for ( uint i = 0; i < children.size(); ++i )
    {
        xc::DOMElement *child = children[ i ];
        QName current_qname( fromX( child->getLocalName() ), 
                             fromX( child->getNamespaceURI() ) );

        if ( current_qname != DC_METADATA_QNAME &&
             current_qname != X_METADATA_QNAME 
            )
        {
            results.push_back( NotAllowedChildResult( *children[ i ] ) );
        }
    } 

    return results;
}


std::vector<Result> MetadataAllowedChildren::ValidateStandardChildren( 
    std::vector< xc::DOMElement* > children )
{
    std::vector<Result> results;

    for ( uint i = 0; i < children.size(); ++i )
    {
        std::string local_name     = fromX( children[ i ]->getLocalName() );
        std::string namespace_name = fromX( children[ i ]->getNamespaceURI() );
        QName child_qname( local_name, namespace_name );

        if ( child_qname != TITLE_QNAME       &&
             child_qname != LANGUAGE_QNAME    &&
             child_qname != IDENTIFIER_QNAME  &&
             child_qname != CREATOR_QNAME     &&
             child_qname != SUBJECT_QNAME     &&
             child_qname != DESCRIPTION_QNAME &&
             child_qname != PUBLISHER_QNAME   &&
             child_qname != CONTRIBUTOR_QNAME &&
             child_qname != DATE_QNAME        &&
             child_qname != TYPE_QNAME        &&
             child_qname != FORMAT_QNAME      &&
             child_qname != SOURCE_QNAME      &&
             child_qname != RELATION_QNAME    &&
             child_qname != COVERAGE_QNAME    &&
             child_qname != RIGHTS_QNAME      &&
             child_qname != META_QNAME 
            )
        {            
            if ( namespace_name == OPF_XML_NAMESPACE ||
                 namespace_name == DC_XML_NAMESPACE
                )
            {
                results.push_back( NotAllowedChildResult( *children[ i ] ) );
            }
        }
    } 

    return results;
}

} //namespace FlightCrew