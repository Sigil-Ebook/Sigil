/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id: PSVIUni.hpp 527149 2007-04-10 14:56:39Z amassari $
 */

#if !defined(XERCESC_INCLUDE_GUARD_PSVIUNI_HPP)
#define XERCESC_INCLUDE_GUARD_PSVIUNI_HPP

#include <xercesc/util/XercesDefs.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLUTIL_EXPORT PSVIUni
{
public :

    static const XMLCh fgPsvColon[];
    
	//Infoset Element Names
    static const XMLCh fgAllDeclarationsProcessed[];
    static const XMLCh fgAttribute[];
    static const XMLCh fgAttributes[];
    static const XMLCh fgAttributeType[];
    static const XMLCh fgBaseURI[];
    static const XMLCh fgCharacter[];
    static const XMLCh fgCharacterEncodingScheme[];
    static const XMLCh fgChildren[];
    static const XMLCh fgComment[];
    static const XMLCh fgContent[];
    static const XMLCh fgDocument[];
    static const XMLCh fgDocTypeDeclaration[];
    static const XMLCh fgDocumentElement[];
    static const XMLCh fgElement[];
    static const XMLCh fgInScopeNamespaces[];
    static const XMLCh fgLocalName[];
    static const XMLCh fgNamespace[];
    static const XMLCh fgNamespaceAttributes[];
    static const XMLCh fgNamespaceName[];
    static const XMLCh fgNormalizedValue[];
    static const XMLCh fgNotations[];
    static const XMLCh fgPrefix[];
    static const XMLCh fgProcessingInstruction[];
    static const XMLCh fgReferences[];
    static const XMLCh fgSpecified[];
    static const XMLCh fgStandalone[];
    static const XMLCh fgTarget[];
    static const XMLCh fgText[];
    static const XMLCh fgTextContent[];
    static const XMLCh fgUnparsedEntities[];
    static const XMLCh fgVersion[];

	//PSVI Element Names
    static const XMLCh fgAbstract[];
    static const XMLCh fgAnnotation[];
    static const XMLCh fgAnnotations[];
    static const XMLCh fgApplicationInformation[];
    static const XMLCh fgAttributeDeclaration[];
    static const XMLCh fgAttributeGroupDefinition[];
    static const XMLCh fgAttributeUse[];
    static const XMLCh fgAttributeUses[];
    static const XMLCh fgAttributeWildcard[];
    static const XMLCh fgBaseTypeDefinition[];
    static const XMLCh fgCanonicalRepresentation[];
    static const XMLCh fgComplexTypeDefinition[];
    static const XMLCh fgCompositor[];
    static const XMLCh fgContentType[];
    static const XMLCh fgDeclaration[];
    static const XMLCh fgDerivationMethod[];
    static const XMLCh fgDisallowedSubstitutions[];
    static const XMLCh fgPsvDocument[];
    static const XMLCh fgDocumentLocation[];
    static const XMLCh fgElementDeclaration[];
    static const XMLCh fgFacets[];
    static const XMLCh fgFacetFixed[];
    static const XMLCh fgFields[];
    static const XMLCh fgFinal[];    
    static const XMLCh fgFundamentalFacets[];
    static const XMLCh fgIdentityConstraintCategory[];
    static const XMLCh fgIdentityConstraintDefinition[];
    static const XMLCh fgIdentityConstraintDefinitions[];
    static const XMLCh fgIdentityConstraintTable[];
    static const XMLCh fgIdIdrefTable[];
    static const XMLCh fgItemTypeDefinition[];
    static const XMLCh fgMaxOccurs[];
    static const XMLCh fgMemberTypeDefinition[];
    static const XMLCh fgMemberTypeDefinitions[];
    static const XMLCh fgMinOccurs[];
    static const XMLCh fgModelGroup[];
    static const XMLCh fgModelGroupDefinition[];
    static const XMLCh fgName[];
    static const XMLCh fgNamespaceConstraint[];
    static const XMLCh fgNamespaces[];
    static const XMLCh fgNamespaceSchemaInformation[];
    static const XMLCh fgNil[];
    static const XMLCh fgNillable[];
    static const XMLCh fgNotation[];
    static const XMLCh fgNotationDeclaration[];
    static const XMLCh fgParticle[];
    static const XMLCh fgParticles[];
    static const XMLCh fgPrimitiveTypeDefinition[];
    static const XMLCh fgProcessContents[];
    static const XMLCh fgProhibitedSubstitutions[];
    static const XMLCh fgPublicIdentifier[];
    static const XMLCh fgReferencedKey[];
    static const XMLCh fgRequired[];
    static const XMLCh fgSchemaAnnotations[];
    static const XMLCh fgSchemaComponents[];
    static const XMLCh fgSchemaDefault[];
    static const XMLCh fgSchemaDocument[];
    static const XMLCh fgSchemaDocuments[];
    static const XMLCh fgSchemaErrorCode[];
    static const XMLCh fgSchemaInformation[];
    static const XMLCh fgSchemaNamespace[];
    static const XMLCh fgSchemaNormalizedValue[];
    static const XMLCh fgSchemaSpecified[];
    static const XMLCh fgScope[];
    static const XMLCh fgSelector[];
    static const XMLCh fgSimpleTypeDefinition[];
    static const XMLCh fgSubstitutionGroupAffiliation[];
    static const XMLCh fgSubstitutionGroupExclusions[];
    static const XMLCh fgSystemIdentifier[];
    static const XMLCh fgTargetNamespace[];
    static const XMLCh fgTerm[];
    static const XMLCh fgTypeDefinition[];
    static const XMLCh fgUserInformation[];
    static const XMLCh fgValidationAttempted[];
    static const XMLCh fgValidationContext[];
    static const XMLCh fgValidity[];
    static const XMLCh fgValue[];
    static const XMLCh fgValueConstraint[];
    static const XMLCh fgVariety[];
    static const XMLCh fgWildcard[];
    static const XMLCh fgXpath[];

	//PSVI Element Values
    static const XMLCh fgAll[];
    static const XMLCh fgAny[];
    static const XMLCh fgAppinfo[];
    static const XMLCh fgAtomic[];
    static const XMLCh fgChoice[];
    static const XMLCh fgDefault[];
    static const XMLCh fgDocumentation[];
    static const XMLCh fgElementOnly[];
    static const XMLCh fgEmpty[];
    static const XMLCh fgExtension[];
    static const XMLCh fgFalse[];
    static const XMLCh fgFull[];
    static const XMLCh fgGlobal[];
    static const XMLCh fgInfoset[];
    static const XMLCh fgInvalid[];
    static const XMLCh fgKey[];
    static const XMLCh fgKeyref[];
    static const XMLCh fgLax[];
    static const XMLCh fgList[];
    static const XMLCh fgLocal[];
    static const XMLCh fgMixed[];
    static const XMLCh fgNone[];
    static const XMLCh fgNotKnown[];
    static const XMLCh fgNsNamespace[];
    static const XMLCh fgOnePointZero[];
    static const XMLCh fgPartial[];
    static const XMLCh fgRestrict[];
    static const XMLCh fgRestriction[];
    static const XMLCh fgSchema[];
    static const XMLCh fgSequence[];
    static const XMLCh fgSimple[];
    static const XMLCh fgSkip[];
    static const XMLCh fgStrict[];
    static const XMLCh fgSubstitution[];
    static const XMLCh fgTotal[];
    static const XMLCh fgTrue[];
    static const XMLCh fgUnbounded[];
    static const XMLCh fgUnion[];
    static const XMLCh fgUnique[];
    static const XMLCh fgUnknown[];
    static const XMLCh fgValid[];
    static const XMLCh fgVCFixed[];
    static const XMLCh fgXMLChNull[];

	//PSVI Element Types (Shortened)
    static const XMLCh fgAg[];
    static const XMLCh fgAnnot[];
    static const XMLCh fgAttr[];
    static const XMLCh fgAu[];
    static const XMLCh fgElt[];
    static const XMLCh fgIdc[];
    static const XMLCh fgMg[];
    static const XMLCh fgNot[];
    static const XMLCh fgType[];

	//Facets    
    static const XMLCh fgBounded[];
    static const XMLCh fgCardinality[];
    static const XMLCh fgEnumeration[];
    static const XMLCh fgFractionDigits[];
    static const XMLCh fgLength[];
    static const XMLCh fgMaxExclusive[];
    static const XMLCh fgMaxInclusive[];
    static const XMLCh fgMaxLength[];
    static const XMLCh fgMinExclusive[];
    static const XMLCh fgMinInclusive[];
    static const XMLCh fgMinLength[];
    static const XMLCh fgNumeric[];
    static const XMLCh fgOrdered[];
    static const XMLCh fgPattern[];
    static const XMLCh fgTotalDigits[];
    static const XMLCh fgWhiteSpace[];
    
   	//Namespaces and prefixes
    	
	static const XMLCh fgNamespaceInfoset[];
	static const XMLCh fgXsi[];
	static const XMLCh fgNamespaceInstance[];
	static const XMLCh fgPsv[];
	static const XMLCh fgNamespacePsvi[];
	static const XMLCh fgXml[];
	static const XMLCh fgNamespaceXmlSchema[];

};

XERCES_CPP_NAMESPACE_END

#endif
