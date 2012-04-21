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
#include "OpfValidatorsList.h"
#include "Validators/Opf/ContributorAttributesPresent.h"
#include "Validators/Opf/CoverageAttributesPresent.h"
#include "Validators/Opf/CreatorAttributesPresent.h"
#include "Validators/Opf/CreatorOrContributorRoleValid.h"
#include "Validators/Opf/DateAttributesPresent.h"
#include "Validators/Opf/DateValid.h"
#include "Validators/Opf/DCMetadataAllowedChildren.h"
#include "Validators/Opf/DCMetadataAttributesPresent.h"
#include "Validators/Opf/DescriptionAttributesPresent.h"
#include "Validators/Opf/FormatAttributesPresent.h"
#include "Validators/Opf/GuideAllowedChildren.h"
#include "Validators/Opf/GuideAttributesPresent.h"
#include "Validators/Opf/IdentifierAttributesPresent.h"
#include "Validators/Opf/IdentifierPresent.h"
#include "Validators/Opf/IdsUnique.h"
#include "Validators/Opf/IdsValid.h"
#include "Validators/Opf/ItemAttributesPresent.h"
#include "Validators/Opf/ItemHrefUnique.h"
#include "Validators/Opf/ItemHrefValid.h"
#include "Validators/Opf/ItemLinearValid.h"
#include "Validators/Opf/ItemPresent.h"
#include "Validators/Opf/ItemrefAttributesPresent.h"
#include "Validators/Opf/ItemrefIdrefValid.h"
#include "Validators/Opf/ItemrefPresent.h"
#include "Validators/Opf/ItemReqModsOnlyWithReqNS.h"
#include "Validators/Opf/LanguagePresent.h"
#include "Validators/Opf/ManifestAllowedChildren.h"
#include "Validators/Opf/ManifestAttributesPresent.h"
#include "Validators/Opf/MetaAttributesPresent.h"
#include "Validators/Opf/MetadataAllowedChildren.h"
#include "Validators/Opf/MetadataAttributesPresent.h"
#include "Validators/Opf/OneManifest.h"
#include "Validators/Opf/OneMetadata.h"
#include "Validators/Opf/OneSpine.h"
#include "Validators/Opf/PackageAllowedChildren.h"
#include "Validators/Opf/PackageAttributesPresent.h"
#include "Validators/Opf/PackageIsRoot.h"
#include "Validators/Opf/PackageUniqueIdentifierValid.h"
#include "Validators/Opf/PackageVersionCorrect.h"
#include "Validators/Opf/PublisherAttributesPresent.h"
#include "Validators/Opf/ReferenceAttributesPresent.h"
#include "Validators/Opf/ReferenceTypeValid.h"
#include "Validators/Opf/RelationAttributesPresent.h"
#include "Validators/Opf/RightsAttributesPresent.h"
#include "Validators/Opf/SiteAttributesPresent.h"
#include "Validators/Opf/SourceAttributesPresent.h"
#include "Validators/Opf/SpineAllowedChildren.h"
#include "Validators/Opf/SpineAttributesPresent.h"
#include "Validators/Opf/SubjectAttributesPresent.h"
#include "Validators/Opf/TitleAttributesPresent.h"
#include "Validators/Opf/TitlePresent.h"
#include "Validators/Opf/TourAllowedChildren.h"
#include "Validators/Opf/TourAttributesPresent.h"
#include "Validators/Opf/ToursAllowedChildren.h"
#include "Validators/Opf/ToursAttributesPresent.h"
#include "Validators/Opf/TypeAttributesPresent.h"
#include "Validators/Opf/XMetadataAllowedChildren.h"
#include "Validators/Opf/XMetadataAttributesPresent.h"
#include "Validators/Opf/NcxPresent.h"
#include "Validators/Opf/SpineTocValid.h"
#include "Validators/Opf/ItemFilesPresent.h"
#include "Validators/Opf/ReachabilityAnalysis.h"
#include "Validators/Opf/ItemMediaTypeValid.h"
#include "Validators/Opf/ItemrefIdrefUnique.h"


namespace FlightCrew
{

std::vector< boost::shared_ptr< XmlValidator > > GetOpfXmlValidators()
{
    std::vector< boost::shared_ptr< XmlValidator > > validators;
    validators.push_back( boost::shared_ptr< XmlValidator >( new ContributorAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new CoverageAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new CreatorAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new CreatorOrContributorRoleValid() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new DateAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new DateValid() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new DCMetadataAllowedChildren() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new DCMetadataAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new DescriptionAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new FormatAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new GuideAllowedChildren() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new GuideAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new IdentifierAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new IdentifierPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new IdsUnique() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new IdsValid() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemHrefUnique() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemHrefValid() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemLinearValid() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemrefAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemrefIdrefValid() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemrefPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemReqModsOnlyWithReqNS() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new LanguagePresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ManifestAllowedChildren() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ManifestAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new MetaAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new MetadataAllowedChildren() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new MetadataAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new OneManifest() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new OneMetadata() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new OneSpine() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new PackageAllowedChildren() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new PackageAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new PackageIsRoot() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new PackageUniqueIdentifierValid() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new PackageVersionCorrect() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new PublisherAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ReferenceAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ReferenceTypeValid() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new RelationAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new RightsAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new SiteAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new SourceAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new SpineAllowedChildren() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new SpineAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new SubjectAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new TitleAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new TitlePresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new TourAllowedChildren() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new TourAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ToursAllowedChildren() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ToursAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new TypeAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new XMetadataAllowedChildren() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new XMetadataAttributesPresent() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new NcxPresent() ) );    
    validators.push_back( boost::shared_ptr< XmlValidator >( new SpineTocValid() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemFilesPresent() ) );    
    validators.push_back( boost::shared_ptr< XmlValidator >( new ReachabilityAnalysis() ) );
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemMediaTypeValid() ) );   
    validators.push_back( boost::shared_ptr< XmlValidator >( new ItemrefIdrefUnique() ) );   
    
    return validators;
}

} // namespace FlightCrew