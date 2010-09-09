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


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include <xercesc/util/XMLUniDefs.hpp>
#include "PSVIUni.hpp"

XERCES_CPP_NAMESPACE_USE


const XMLCh PSVIUni::fgPsvColon[] = {
	chLatin_p, chLatin_s, chLatin_v, chColon, chNull
};

const XMLCh PSVIUni::fgAllDeclarationsProcessed[] =
{
	chLatin_a, chLatin_l, chLatin_l, chLatin_D, chLatin_e, chLatin_c, chLatin_l, chLatin_a, chLatin_r, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_s, chLatin_P, chLatin_r, chLatin_o, chLatin_c, chLatin_e, chLatin_s, chLatin_s, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgAttribute[] =
{
	chLatin_a, chLatin_t, chLatin_t, chLatin_r, chLatin_i, chLatin_b, chLatin_u, chLatin_t, chLatin_e, chNull
};

const XMLCh PSVIUni::fgAttributes[] =
{
	chLatin_a, chLatin_t, chLatin_t, chLatin_r, chLatin_i, chLatin_b, chLatin_u, chLatin_t, chLatin_e, chLatin_s, chNull
};

const XMLCh PSVIUni::fgAttributeType[] =
{
	chLatin_a, chLatin_t, chLatin_t, chLatin_r, chLatin_i, chLatin_b, chLatin_u, chLatin_t, chLatin_e, chLatin_T, chLatin_y, chLatin_p, chLatin_e, chNull
};

const XMLCh PSVIUni::fgBaseURI[] =
{
	chLatin_b, chLatin_a, chLatin_s, chLatin_e, chLatin_U, chLatin_R, chLatin_I, chNull
};

const XMLCh PSVIUni::fgCanonicalRepresentation[] =
{
    chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_c, chLatin_a, chLatin_n, chLatin_o, chLatin_n, chLatin_i, chLatin_c, chLatin_a,
    chLatin_l, chLatin_R, chLatin_e, chLatin_p, chNull

};

const XMLCh PSVIUni::fgCharacter[] =
{
	chLatin_c, chLatin_h, chLatin_a, chLatin_r, chLatin_a, chLatin_c, chLatin_t, chLatin_e, chLatin_r, chNull
};

const XMLCh PSVIUni::fgCharacterEncodingScheme[] =
{
	chLatin_c, chLatin_h, chLatin_a, chLatin_r, chLatin_a, chLatin_c, chLatin_t, chLatin_e, chLatin_r, chLatin_E, chLatin_n, chLatin_c, chLatin_o, chLatin_d, chLatin_i, chLatin_n, chLatin_g, chLatin_S, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_e, chNull
};

const XMLCh PSVIUni::fgChildren[] =
{
	chLatin_c, chLatin_h, chLatin_i, chLatin_l, chLatin_d, chLatin_r, chLatin_e, chLatin_n, chNull
};

const XMLCh PSVIUni::fgComment[] =
{
	chLatin_c, chLatin_o, chLatin_m, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh PSVIUni::fgContent[] =
{
	chLatin_c, chLatin_o, chLatin_n, chLatin_t, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh PSVIUni::fgDocument[] =
{
	chLatin_d, chLatin_o, chLatin_c, chLatin_u, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh PSVIUni::fgDocTypeDeclaration[] =
{
	chLatin_d, chLatin_o, chLatin_c, chLatin_T, chLatin_y, chLatin_p, chLatin_e, chLatin_D, chLatin_e, chLatin_c, chLatin_l, chLatin_a, chLatin_r, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgDocumentElement[] =
{
	chLatin_d, chLatin_o, chLatin_c, chLatin_u, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chLatin_E, chLatin_l, chLatin_e, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh PSVIUni::fgElement[] =
{
	chLatin_e, chLatin_l, chLatin_e, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh PSVIUni::fgInScopeNamespaces[] =
{
	chLatin_i, chLatin_n, chLatin_S, chLatin_c, chLatin_o, chLatin_p, chLatin_e, chLatin_N, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chLatin_s, chNull
};

const XMLCh PSVIUni::fgLocalName[] =
{
	chLatin_l, chLatin_o, chLatin_c, chLatin_a, chLatin_l, chLatin_N, chLatin_a, chLatin_m, chLatin_e, chNull
};

const XMLCh PSVIUni::fgNamespace[] =
{
	chLatin_n, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chNull
};

const XMLCh PSVIUni::fgNamespaceAttributes[] =
{
	chLatin_n, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chLatin_A, chLatin_t, chLatin_t, chLatin_r, chLatin_i, chLatin_b, chLatin_u, chLatin_t, chLatin_e, chLatin_s, chNull
};

const XMLCh PSVIUni::fgNamespaceName[] =
{
	chLatin_n, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chLatin_N, chLatin_a, chLatin_m, chLatin_e, chNull
};

const XMLCh PSVIUni::fgNormalizedValue[] =
{
	chLatin_n, chLatin_o, chLatin_r, chLatin_m, chLatin_a, chLatin_l, chLatin_i, chLatin_z, chLatin_e, chLatin_d, chLatin_V, chLatin_a, chLatin_l, chLatin_u, chLatin_e, chNull
};

const XMLCh PSVIUni::fgNotations[] =
{
	chLatin_n, chLatin_o, chLatin_t, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_s, chNull
};

const XMLCh PSVIUni::fgPrefix[] =
{
	chLatin_p, chLatin_r, chLatin_e, chLatin_f, chLatin_i, chLatin_x, chNull
};

const XMLCh PSVIUni::fgProcessingInstruction[] =
{
	chLatin_p, chLatin_r, chLatin_o, chLatin_c, chLatin_e, chLatin_s, chLatin_s, chLatin_i, chLatin_n, chLatin_g, chLatin_I, chLatin_n, chLatin_s, chLatin_t, chLatin_r, chLatin_u, chLatin_c, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgReferences[] =
{
	chLatin_r, chLatin_e, chLatin_f, chLatin_e, chLatin_r, chLatin_e, chLatin_n, chLatin_c, chLatin_e, chLatin_s, chNull
};

const XMLCh PSVIUni::fgSpecified[] =
{
	chLatin_s, chLatin_p, chLatin_e, chLatin_c, chLatin_i, chLatin_f, chLatin_i, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgStandalone[] =
{
	chLatin_s, chLatin_t, chLatin_a, chLatin_n, chLatin_d, chLatin_a, chLatin_l, chLatin_o, chLatin_n, chLatin_e, chNull
};

const XMLCh PSVIUni::fgTarget[] =
{
	chLatin_t, chLatin_a, chLatin_r, chLatin_g, chLatin_e, chLatin_t, chNull
};

const XMLCh PSVIUni::fgText[] =
{
	chLatin_t, chLatin_e, chLatin_x, chLatin_t, chNull
};

const XMLCh PSVIUni::fgTextContent[] =
{
	chLatin_t, chLatin_e, chLatin_x, chLatin_t, chLatin_C, chLatin_o, chLatin_n, chLatin_t, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh PSVIUni::fgUnparsedEntities[] =
{
	chLatin_u, chLatin_n, chLatin_p, chLatin_a, chLatin_r, chLatin_s, chLatin_e, chLatin_d, chLatin_E, chLatin_n, chLatin_t, chLatin_i, chLatin_t, chLatin_i, chLatin_e, chLatin_s, chNull
};

const XMLCh PSVIUni::fgVersion[] =
{
	chLatin_v, chLatin_e, chLatin_r, chLatin_s, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgAbstract[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_a, chLatin_b, chLatin_s, chLatin_t, chLatin_r, chLatin_a, chLatin_c, chLatin_t, chNull
};

const XMLCh PSVIUni::fgAnnotation[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_a, chLatin_n, chLatin_n, chLatin_o, chLatin_t, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgAnnotations[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_a, chLatin_n, chLatin_n, chLatin_o, chLatin_t, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_s, chNull
};

const XMLCh PSVIUni::fgApplicationInformation[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_a, chLatin_p, chLatin_p, chLatin_l, chLatin_i, chLatin_c, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_I, chLatin_n, chLatin_f, chLatin_o, chLatin_r, chLatin_m, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgAttributeDeclaration[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_a, chLatin_t, chLatin_t, chLatin_r, chLatin_i, chLatin_b, chLatin_u, chLatin_t, chLatin_e, chLatin_D, chLatin_e, chLatin_c, chLatin_l, chLatin_a, chLatin_r, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgAttributeGroupDefinition[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_a, chLatin_t, chLatin_t, chLatin_r, chLatin_i, chLatin_b, chLatin_u, chLatin_t, chLatin_e, chLatin_G, chLatin_r, chLatin_o, chLatin_u, chLatin_p, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgAttributeUse[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_a, chLatin_t, chLatin_t, chLatin_r, chLatin_i, chLatin_b, chLatin_u, chLatin_t, chLatin_e, chLatin_U, chLatin_s, chLatin_e, chNull
};

const XMLCh PSVIUni::fgAttributeUses[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_a, chLatin_t, chLatin_t, chLatin_r, chLatin_i, chLatin_b, chLatin_u, chLatin_t, chLatin_e, chLatin_U, chLatin_s, chLatin_e, chLatin_s, chNull
};

const XMLCh PSVIUni::fgAttributeWildcard[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_a, chLatin_t, chLatin_t, chLatin_r, chLatin_i, chLatin_b, chLatin_u, chLatin_t, chLatin_e, chLatin_W, chLatin_i, chLatin_l, chLatin_d, chLatin_c, chLatin_a, chLatin_r, chLatin_d, chNull
};

const XMLCh PSVIUni::fgBaseTypeDefinition[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_b, chLatin_a, chLatin_s, chLatin_e, chLatin_T, chLatin_y, chLatin_p, chLatin_e, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgComplexTypeDefinition[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_c, chLatin_o, chLatin_m, chLatin_p, chLatin_l, chLatin_e, chLatin_x, chLatin_T, chLatin_y, chLatin_p, chLatin_e, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgCompositor[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_c, chLatin_o, chLatin_m, chLatin_p, chLatin_o, chLatin_s, chLatin_i, chLatin_t, chLatin_o, chLatin_r, chNull
};

const XMLCh PSVIUni::fgContentType[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_c, chLatin_o, chLatin_n, chLatin_t, chLatin_e, chLatin_n, chLatin_t, chLatin_T, chLatin_y, chLatin_p, chLatin_e, chNull
};

const XMLCh PSVIUni::fgDeclaration[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_d, chLatin_e, chLatin_c, chLatin_l, chLatin_a, chLatin_r, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgDerivationMethod[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_d, chLatin_e, chLatin_r, chLatin_i, chLatin_v, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_M, chLatin_e, chLatin_t, chLatin_h, chLatin_o, chLatin_d, chNull
};

const XMLCh PSVIUni::fgDisallowedSubstitutions[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_d, chLatin_i, chLatin_s, chLatin_a, chLatin_l, chLatin_l, chLatin_o, chLatin_w, chLatin_e, chLatin_d, chLatin_S, chLatin_u, chLatin_b, chLatin_s, chLatin_t, chLatin_i, chLatin_t, chLatin_u, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_s, chNull
};

const XMLCh PSVIUni::fgDocumentLocation[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_d, chLatin_o, chLatin_c, chLatin_u, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chLatin_L, chLatin_o, chLatin_c, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgPsvDocument[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_d, chLatin_o, chLatin_c, chLatin_u, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chNull
};
    
const XMLCh PSVIUni::fgElementDeclaration[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_e, chLatin_l, chLatin_e, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chLatin_D, chLatin_e, chLatin_c, chLatin_l, chLatin_a, chLatin_r, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgFacetFixed[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_f, chLatin_i, chLatin_x, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgFacets[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_f, chLatin_a, chLatin_c, chLatin_e, chLatin_t, chLatin_s, chNull
};

const XMLCh PSVIUni::fgFields[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_f, chLatin_i, chLatin_e, chLatin_l, chLatin_d, chLatin_s, chNull
};

const XMLCh PSVIUni::fgFinal[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_f, chLatin_i, chLatin_n, chLatin_a, chLatin_l, chNull
};

const XMLCh PSVIUni::fgFundamentalFacets[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_f, chLatin_u, chLatin_n, chLatin_d, chLatin_a, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chLatin_a, chLatin_l, chLatin_F, chLatin_a, chLatin_c, chLatin_e, chLatin_t, chLatin_s, chNull
};

const XMLCh PSVIUni::fgIdentityConstraintCategory[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_i, chLatin_d, chLatin_e, chLatin_n, chLatin_t, chLatin_i, chLatin_t, chLatin_y, chLatin_C, chLatin_o, chLatin_n, chLatin_s, chLatin_t, chLatin_r, chLatin_a, chLatin_i, chLatin_n, chLatin_t, chLatin_C, chLatin_a, chLatin_t, chLatin_e, chLatin_g, chLatin_o, chLatin_r, chLatin_y, chNull
};

const XMLCh PSVIUni::fgIdentityConstraintDefinition[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_i, chLatin_d, chLatin_e, chLatin_n, chLatin_t, chLatin_i, chLatin_t, chLatin_y, chLatin_C, chLatin_o, chLatin_n, chLatin_s, chLatin_t, chLatin_r, chLatin_a, chLatin_i, chLatin_n, chLatin_t, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgIdentityConstraintDefinitions[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_i, chLatin_d, chLatin_e, chLatin_n, chLatin_t, chLatin_i, chLatin_t, chLatin_y, chLatin_C, chLatin_o, chLatin_n, chLatin_s, chLatin_t, chLatin_r, chLatin_a, chLatin_i, chLatin_n, chLatin_t, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_s, chNull
};

const XMLCh PSVIUni::fgIdentityConstraintTable[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_i, chLatin_d, chLatin_e, chLatin_n, chLatin_t, chLatin_i, chLatin_t, chLatin_y, chLatin_C, chLatin_o, chLatin_n, chLatin_s, chLatin_t, chLatin_r, chLatin_a, chLatin_i, chLatin_n, chLatin_t, chLatin_T, chLatin_a, chLatin_b, chLatin_l, chLatin_e, chNull
};

const XMLCh PSVIUni::fgIdIdrefTable[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_i, chLatin_d, chLatin_I, chLatin_d, chLatin_r, chLatin_e, chLatin_f, chLatin_T, chLatin_a, chLatin_b, chLatin_l, chLatin_e, chNull
};

const XMLCh PSVIUni::fgItemTypeDefinition[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_i, chLatin_t, chLatin_e, chLatin_m, chLatin_T, chLatin_y, chLatin_p, chLatin_e, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgMaxOccurs[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_a, chLatin_x, chLatin_O, chLatin_c, chLatin_c, chLatin_u, chLatin_r, chLatin_s, chNull
};

const XMLCh PSVIUni::fgMemberTypeDefinition[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_e, chLatin_m, chLatin_b, chLatin_e, chLatin_r, chLatin_T, chLatin_y, chLatin_p, chLatin_e, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgMemberTypeDefinitions[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_e, chLatin_m, chLatin_b, chLatin_e, chLatin_r, chLatin_T, chLatin_y, chLatin_p, chLatin_e, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_s, chNull
};

const XMLCh PSVIUni::fgMinOccurs[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_i, chLatin_n, chLatin_O, chLatin_c, chLatin_c, chLatin_u, chLatin_r, chLatin_s, chNull
};

const XMLCh PSVIUni::fgModelGroup[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_o, chLatin_d, chLatin_e, chLatin_l, chLatin_G, chLatin_r, chLatin_o, chLatin_u, chLatin_p, chNull
};

const XMLCh PSVIUni::fgModelGroupDefinition[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_o, chLatin_d, chLatin_e, chLatin_l, chLatin_G, chLatin_r, chLatin_o, chLatin_u, chLatin_p, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgName[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_n, chLatin_a, chLatin_m, chLatin_e, chNull
};

const XMLCh PSVIUni::fgNamespaceConstraint[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_n, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chLatin_C, chLatin_o, chLatin_n, chLatin_s, chLatin_t, chLatin_r, chLatin_a, chLatin_i, chLatin_n, chLatin_t, chNull
};

const XMLCh PSVIUni::fgNamespaces[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_n, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chLatin_s, chNull
};

const XMLCh PSVIUni::fgNamespaceSchemaInformation[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_n, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chLatin_S, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_I, chLatin_n, chLatin_f, chLatin_o, chLatin_r, chLatin_m, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgNil[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_n, chLatin_i, chLatin_l, chNull
};

const XMLCh PSVIUni::fgNillable[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_n, chLatin_i, chLatin_l, chLatin_l, chLatin_a, chLatin_b, chLatin_l, chLatin_e, chNull
};

const XMLCh PSVIUni::fgNotation[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_n, chLatin_o, chLatin_t, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgNotationDeclaration[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_n, chLatin_o, chLatin_t, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_D, chLatin_e, chLatin_c, chLatin_l, chLatin_a, chLatin_r, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgParticle[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_p, chLatin_a, chLatin_r, chLatin_t, chLatin_i, chLatin_c, chLatin_l, chLatin_e, chNull
};

const XMLCh PSVIUni::fgParticles[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_p, chLatin_a, chLatin_r, chLatin_t, chLatin_i, chLatin_c, chLatin_l, chLatin_e, chLatin_s, chNull
};

const XMLCh PSVIUni::fgPrimitiveTypeDefinition[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_p, chLatin_r, chLatin_i, chLatin_m, chLatin_i, chLatin_t, chLatin_i, chLatin_v, chLatin_e, chLatin_T, chLatin_y, chLatin_p, chLatin_e, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgProcessContents[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_p, chLatin_r, chLatin_o, chLatin_c, chLatin_e, chLatin_s, chLatin_s, chLatin_C, chLatin_o, chLatin_n, chLatin_t, chLatin_e, chLatin_n, chLatin_t, chLatin_s, chNull
};

const XMLCh PSVIUni::fgProhibitedSubstitutions[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_p, chLatin_r, chLatin_o, chLatin_h, chLatin_i, chLatin_b, chLatin_i, chLatin_t, chLatin_e, chLatin_d, chLatin_S, chLatin_u, chLatin_b, chLatin_s, chLatin_t, chLatin_i, chLatin_t, chLatin_u, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_s, chNull
};

const XMLCh PSVIUni::fgPublicIdentifier[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_p, chLatin_u, chLatin_b, chLatin_l, chLatin_i, chLatin_c, chLatin_I, chLatin_d, chLatin_e, chLatin_n, chLatin_t, chLatin_i, chLatin_f, chLatin_i, chLatin_e, chLatin_r, chNull
};

const XMLCh PSVIUni::fgReferencedKey[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_r, chLatin_e, chLatin_f, chLatin_e, chLatin_r, chLatin_e, chLatin_n, chLatin_c, chLatin_e, chLatin_d, chLatin_K, chLatin_e, chLatin_y, chNull
};

const XMLCh PSVIUni::fgRequired[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_r, chLatin_e, chLatin_q, chLatin_u, chLatin_i, chLatin_r, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgSchemaAnnotations[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_A, chLatin_n, chLatin_n, chLatin_o, chLatin_t, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_s, chNull
};

const XMLCh PSVIUni::fgSchemaComponents[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_C, chLatin_o, chLatin_m, chLatin_p, chLatin_o, chLatin_n, chLatin_e, chLatin_n, chLatin_t, chLatin_s, chNull
};

const XMLCh PSVIUni::fgSchemaDefault[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_D, chLatin_e, chLatin_f, chLatin_a, chLatin_u, chLatin_l, chLatin_t, chNull
};

const XMLCh PSVIUni::fgSchemaDocument[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_D, chLatin_o, chLatin_c, chLatin_u, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chNull
};

const XMLCh PSVIUni::fgSchemaDocuments[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_D, chLatin_o, chLatin_c, chLatin_u, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chLatin_s, chNull
};

const XMLCh PSVIUni::fgSchemaErrorCode[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_E, chLatin_r, chLatin_r, chLatin_o, chLatin_r, chLatin_C, chLatin_o, chLatin_d, chLatin_e, chNull
};

const XMLCh PSVIUni::fgSchemaInformation[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_I, chLatin_n, chLatin_f, chLatin_o, chLatin_r, chLatin_m, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgSchemaNamespace[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_N, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chNull
};

const XMLCh PSVIUni::fgSchemaNormalizedValue[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_N, chLatin_o, chLatin_r, chLatin_m, chLatin_a, chLatin_l, chLatin_i, chLatin_z, chLatin_e, chLatin_d, chLatin_V, chLatin_a, chLatin_l, chLatin_u, chLatin_e, chNull
};

const XMLCh PSVIUni::fgSchemaSpecified[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chLatin_S, chLatin_p, chLatin_e, chLatin_c, chLatin_i, chLatin_f, chLatin_i, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgScope[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_c, chLatin_o, chLatin_p, chLatin_e, chNull
};

const XMLCh PSVIUni::fgSelector[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_e, chLatin_l, chLatin_e, chLatin_c, chLatin_t, chLatin_o, chLatin_r, chNull
};

const XMLCh PSVIUni::fgSimpleTypeDefinition[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_i, chLatin_m, chLatin_p, chLatin_l, chLatin_e, chLatin_T, chLatin_y, chLatin_p, chLatin_e, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgSubstitutionGroupAffiliation[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_u, chLatin_b, chLatin_s, chLatin_t, chLatin_i, chLatin_t, chLatin_u, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_G, chLatin_r, chLatin_o, chLatin_u, chLatin_p, chLatin_A, chLatin_f, chLatin_f, chLatin_i, chLatin_l, chLatin_i, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgSubstitutionGroupExclusions[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_u, chLatin_b, chLatin_s, chLatin_t, chLatin_i, chLatin_t, chLatin_u, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_G, chLatin_r, chLatin_o, chLatin_u, chLatin_p, chLatin_E, chLatin_x, chLatin_c, chLatin_l, chLatin_u, chLatin_s, chLatin_i, chLatin_o, chLatin_n, chLatin_s, chNull
};

const XMLCh PSVIUni::fgSystemIdentifier[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_s, chLatin_y, chLatin_s, chLatin_t, chLatin_e, chLatin_m, chLatin_I, chLatin_d, chLatin_e, chLatin_n, chLatin_t, chLatin_i, chLatin_f, chLatin_i, chLatin_e, chLatin_r, chNull
};

const XMLCh PSVIUni::fgTargetNamespace[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_t, chLatin_a, chLatin_r, chLatin_g, chLatin_e, chLatin_t, chLatin_N, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chNull
};

const XMLCh PSVIUni::fgTerm[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_t, chLatin_e, chLatin_r, chLatin_m, chNull
};

const XMLCh PSVIUni::fgTypeDefinition[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_t, chLatin_y, chLatin_p, chLatin_e, chLatin_D, chLatin_e, chLatin_f, chLatin_i, chLatin_n, chLatin_i, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgUserInformation[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_u, chLatin_s, chLatin_e, chLatin_r, chLatin_I, chLatin_n, chLatin_f, chLatin_o, chLatin_r, chLatin_m, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgValidationAttempted[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_v, chLatin_a, chLatin_l, chLatin_i, chLatin_d, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_A, chLatin_t, chLatin_t, chLatin_e, chLatin_m, chLatin_p, chLatin_t, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgValidationContext[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_v, chLatin_a, chLatin_l, chLatin_i, chLatin_d, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_C, chLatin_o, chLatin_n, chLatin_t, chLatin_e, chLatin_x, chLatin_t, chNull
};

const XMLCh PSVIUni::fgValidity[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_v, chLatin_a, chLatin_l, chLatin_i, chLatin_d, chLatin_i, chLatin_t, chLatin_y, chNull
};

const XMLCh PSVIUni::fgValue[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_v, chLatin_a, chLatin_l, chLatin_u, chLatin_e, chNull
};

const XMLCh PSVIUni::fgValueConstraint[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_v, chLatin_a, chLatin_l, chLatin_u, chLatin_e, chLatin_C, chLatin_o, chLatin_n, chLatin_s, chLatin_t, chLatin_r, chLatin_a, chLatin_i, chLatin_n, chLatin_t, chNull
};

const XMLCh PSVIUni::fgVariety[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_v, chLatin_a, chLatin_r, chLatin_i, chLatin_e, chLatin_t, chLatin_y, chNull
};

const XMLCh PSVIUni::fgWildcard[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_w, chLatin_i, chLatin_l, chLatin_d, chLatin_c, chLatin_a, chLatin_r, chLatin_d, chNull
};

const XMLCh PSVIUni::fgXpath[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_x, chLatin_p, chLatin_a, chLatin_t, chLatin_h, chNull
};

const XMLCh PSVIUni::fgAll[] =
{
	chLatin_a, chLatin_l, chLatin_l, chNull
};

const XMLCh PSVIUni::fgAny[] =
{
	chLatin_a, chLatin_n, chLatin_y, chNull
};

const XMLCh PSVIUni::fgAppinfo[] =
{
	chLatin_a, chLatin_p, chLatin_p, chLatin_i, chLatin_n, chLatin_f, chLatin_o, chNull
};

const XMLCh PSVIUni::fgAtomic[] =
{
	chLatin_a, chLatin_t, chLatin_o, chLatin_m, chLatin_i, chLatin_c, chNull
};

const XMLCh PSVIUni::fgChoice[] =
{
	chLatin_c, chLatin_h, chLatin_o, chLatin_i, chLatin_c, chLatin_e, chNull
};

const XMLCh PSVIUni::fgDefault[] =
{
	chLatin_d, chLatin_e, chLatin_f, chLatin_a, chLatin_u, chLatin_l, chLatin_t, chNull
};

const XMLCh PSVIUni::fgDocumentation[] =
{
	chLatin_d, chLatin_o, chLatin_c, chLatin_u, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgElementOnly[] =
{
	chLatin_e, chLatin_l, chLatin_e, chLatin_m, chLatin_e, chLatin_n, chLatin_t, chLatin_O, chLatin_n, chLatin_l, chLatin_y, chNull
};

const XMLCh PSVIUni::fgEmpty[] =
{
	chLatin_e, chLatin_m, chLatin_p, chLatin_t, chLatin_y, chNull
};

const XMLCh PSVIUni::fgExtension[] =
{
	chLatin_e, chLatin_x, chLatin_t, chLatin_e, chLatin_n, chLatin_s, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgFalse[] =
{
	chLatin_f, chLatin_a, chLatin_l, chLatin_s, chLatin_e, chNull
};

const XMLCh PSVIUni::fgFull[] =
{
	chLatin_f, chLatin_u, chLatin_l, chLatin_l, chNull
};

const XMLCh PSVIUni::fgGlobal[] =
{
	chLatin_g, chLatin_l, chLatin_o, chLatin_b, chLatin_a, chLatin_l, chNull
};

const XMLCh PSVIUni::fgInfoset[] =
{
	chLatin_i, chLatin_n, chLatin_f, chLatin_o, chLatin_s, chLatin_e, chLatin_t, chNull
};

const XMLCh PSVIUni::fgInvalid[] =
{
	chLatin_i, chLatin_n, chLatin_v, chLatin_a, chLatin_l, chLatin_i, chLatin_d, chNull
};

const XMLCh PSVIUni::fgKey[] =
{
	chLatin_k, chLatin_e, chLatin_y, chNull
};

const XMLCh PSVIUni::fgKeyref[] =
{
	chLatin_k, chLatin_e, chLatin_y, chLatin_r, chLatin_e, chLatin_f, chNull
};

const XMLCh PSVIUni::fgLax[] =
{
	chLatin_l, chLatin_a, chLatin_x, chNull
};

const XMLCh PSVIUni::fgList[] =
{
	chLatin_l, chLatin_i, chLatin_s, chLatin_t, chNull
};

const XMLCh PSVIUni::fgLocal[] =
{
	chLatin_l, chLatin_o, chLatin_c, chLatin_a, chLatin_l, chNull
};

const XMLCh PSVIUni::fgMixed[] =
{
	chLatin_m, chLatin_i, chLatin_x, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgNone[] =
{
	chLatin_n, chLatin_o, chLatin_n, chLatin_e, chNull
};

const XMLCh PSVIUni::fgNotKnown[] =
{
	chLatin_n, chLatin_o, chLatin_t, chLatin_K, chLatin_n, chLatin_o, chLatin_w, chLatin_n, chNull
};

const XMLCh PSVIUni::fgNsNamespace[] =
{
	chLatin_n, chLatin_s, chLatin_N, chLatin_a, chLatin_m, chLatin_e, chLatin_s, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chNull
};

const XMLCh PSVIUni::fgOnePointZero[] =
{
	chDigit_1, chPeriod, chDigit_0, chNull	
};

const XMLCh PSVIUni::fgPartial[] =
{
	chLatin_p, chLatin_a, chLatin_r, chLatin_t, chLatin_i, chLatin_a, chLatin_l, chNull
};

const XMLCh PSVIUni::fgRestrict[] =
{
	chLatin_r, chLatin_e, chLatin_s, chLatin_t, chLatin_r, chLatin_i, chLatin_c, chLatin_t, chNull
};

const XMLCh PSVIUni::fgRestriction[] =
{
	chLatin_r, chLatin_e, chLatin_s, chLatin_t, chLatin_r, chLatin_i, chLatin_c, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgSchema[] =
{
	chLatin_s, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chNull
};

const XMLCh PSVIUni::fgSequence[] =
{
	chLatin_s, chLatin_e, chLatin_q, chLatin_u, chLatin_e, chLatin_n, chLatin_c, chLatin_e, chNull
};

const XMLCh PSVIUni::fgSimple[] =
{
	chLatin_s, chLatin_i, chLatin_m, chLatin_p, chLatin_l, chLatin_e, chNull
};

const XMLCh PSVIUni::fgSkip[] =
{
	chLatin_s, chLatin_k, chLatin_i, chLatin_p, chNull
};

const XMLCh PSVIUni::fgStrict[] =
{
	chLatin_s, chLatin_t, chLatin_r, chLatin_i, chLatin_c, chLatin_t, chNull
};

const XMLCh PSVIUni::fgSubstitution[] =
{
	chLatin_s, chLatin_u, chLatin_b, chLatin_s, chLatin_t, chLatin_i, chLatin_t, chLatin_u, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgTotal[] =
{
	chLatin_t, chLatin_o, chLatin_t, chLatin_a, chLatin_l, chNull
};

const XMLCh PSVIUni::fgTrue[] =
{
	chLatin_t, chLatin_r, chLatin_u, chLatin_e, chNull
};

const XMLCh PSVIUni::fgUnbounded[] =
{
	chLatin_u, chLatin_n, chLatin_b, chLatin_o, chLatin_u, chLatin_n, chLatin_d, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgUnion[] =
{
	chLatin_u, chLatin_n, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgUnique[] =
{
	chLatin_u, chLatin_n, chLatin_i, chLatin_q, chLatin_u, chLatin_e, chNull
};

const XMLCh PSVIUni::fgUnknown[] =
{
	chLatin_u, chLatin_n, chLatin_k, chLatin_n, chLatin_o, chLatin_w, chLatin_n, chNull
};

const XMLCh PSVIUni::fgValid[] =
{
	chLatin_v, chLatin_a, chLatin_l, chLatin_i, chLatin_d, chNull
};

const XMLCh PSVIUni::fgVCFixed[] =
{
	chLatin_f, chLatin_i, chLatin_x, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgXMLChNull[] =
{
	chNull
};

const XMLCh PSVIUni::fgAg[] =
{
	chLatin_a, chLatin_g, chNull
};

const XMLCh PSVIUni::fgAnnot[] =
{
	chLatin_a, chLatin_n, chLatin_n, chLatin_o, chLatin_t, chNull
};

const XMLCh PSVIUni::fgAttr[] =
{
	chLatin_a, chLatin_t, chLatin_t, chLatin_r, chNull
};

const XMLCh PSVIUni::fgAu[] =
{
	chLatin_a, chLatin_u, chNull
};

const XMLCh PSVIUni::fgElt[] =
{
	chLatin_e, chLatin_l, chLatin_t, chNull
};

const XMLCh PSVIUni::fgIdc[] =
{
	chLatin_i, chLatin_d, chLatin_c, chNull
};

const XMLCh PSVIUni::fgMg[] =
{
	chLatin_m, chLatin_g, chNull
};

const XMLCh PSVIUni::fgNot[] =
{
	chLatin_n, chLatin_o, chLatin_t, chNull
};

const XMLCh PSVIUni::fgType[] =
{
	chLatin_t, chLatin_y, chLatin_p, chLatin_e, chNull
};

const XMLCh PSVIUni::fgBounded[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_b, chLatin_o, chLatin_u, chLatin_n, chLatin_d, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgCardinality[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_c, chLatin_a, chLatin_r, chLatin_d, chLatin_i, chLatin_n, chLatin_a, chLatin_l, chLatin_i, chLatin_t, chLatin_y, chNull
};

const XMLCh PSVIUni::fgEnumeration[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_e, chLatin_n, chLatin_u, chLatin_m, chLatin_e, chLatin_r, chLatin_a, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chNull
};

const XMLCh PSVIUni::fgFractionDigits[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_f, chLatin_r, chLatin_a, chLatin_c, chLatin_t, chLatin_i, chLatin_o, chLatin_n, chLatin_D, chLatin_i, chLatin_g, chLatin_i, chLatin_t, chLatin_s, chNull
};

const XMLCh PSVIUni::fgLength[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_l, chLatin_e, chLatin_n, chLatin_g, chLatin_t, chLatin_h, chNull
};

const XMLCh PSVIUni::fgMaxExclusive[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_a, chLatin_x, chLatin_E, chLatin_x, chLatin_c, chLatin_l, chLatin_u, chLatin_s, chLatin_i, chLatin_v, chLatin_e, chNull
};

const XMLCh PSVIUni::fgMaxInclusive[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_a, chLatin_x, chLatin_I, chLatin_n, chLatin_c, chLatin_l, chLatin_u, chLatin_s, chLatin_i, chLatin_v, chLatin_e, chNull
};

const XMLCh PSVIUni::fgMaxLength[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_a, chLatin_x, chLatin_L, chLatin_e, chLatin_n, chLatin_g, chLatin_t, chLatin_h, chNull
};

const XMLCh PSVIUni::fgMinExclusive[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_i, chLatin_n, chLatin_E, chLatin_x, chLatin_c, chLatin_l, chLatin_u, chLatin_s, chLatin_i, chLatin_v, chLatin_e, chNull
};

const XMLCh PSVIUni::fgMinInclusive[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_i, chLatin_n, chLatin_I, chLatin_n, chLatin_c, chLatin_l, chLatin_u, chLatin_s, chLatin_i, chLatin_v, chLatin_e, chNull
};

const XMLCh PSVIUni::fgMinLength[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_m, chLatin_i, chLatin_n, chLatin_L, chLatin_e, chLatin_n, chLatin_g, chLatin_t, chLatin_h, chNull
};

const XMLCh PSVIUni::fgNumeric[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_n, chLatin_u, chLatin_m, chLatin_e, chLatin_r, chLatin_i, chLatin_c, chNull
};

const XMLCh PSVIUni::fgOrdered[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_o, chLatin_r, chLatin_d, chLatin_e, chLatin_r, chLatin_e, chLatin_d, chNull
};

const XMLCh PSVIUni::fgPattern[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_p, chLatin_a, chLatin_t, chLatin_t, chLatin_e, chLatin_r, chLatin_n, chNull
};

const XMLCh PSVIUni::fgTotalDigits[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_t, chLatin_o, chLatin_t, chLatin_a, chLatin_l, chLatin_D, chLatin_i, chLatin_g, chLatin_i, chLatin_t, chLatin_s, chNull
};

const XMLCh PSVIUni::fgWhiteSpace[] =
{
	chLatin_p, chLatin_s, chLatin_v, chColon, chLatin_w, chLatin_h, chLatin_i, chLatin_t, chLatin_e, chLatin_S, chLatin_p, chLatin_a, chLatin_c, chLatin_e, chNull
};

const XMLCh PSVIUni::fgNamespaceInfoset[] = { //http://www.w3.org/2001/05/XMLInfoset
	chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash, 
	chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g, 
	chForwardSlash, chDigit_2, chDigit_0, chDigit_0, chDigit_1, chForwardSlash, chDigit_0, chDigit_5, 
	chForwardSlash, chLatin_X, chLatin_M, chLatin_L, chLatin_I, chLatin_n, chLatin_f, chLatin_o, chLatin_s, 
	chLatin_e, chLatin_t, chNull
};
const XMLCh PSVIUni::fgXsi[] = { //xsi
	chLatin_x, chLatin_s, chLatin_i, chNull
};
const XMLCh PSVIUni::fgNamespaceInstance[] = { //http://www.w3.org/2001/XMLSchema-instance
	chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash, 
	chLatin_w, chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, 
	chLatin_g, chForwardSlash, chDigit_2, chDigit_0, chDigit_0, chDigit_1, chForwardSlash, chLatin_X, 
	chLatin_M, chLatin_L, chLatin_S, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chDash, chLatin_i, 
	chLatin_n, chLatin_s, chLatin_t, chLatin_a, chLatin_n, chLatin_c, chLatin_e,  chNull
};
const XMLCh PSVIUni::fgPsv[] = { //psv
	chLatin_p, chLatin_s, chLatin_v, chNull
};
const XMLCh PSVIUni::fgNamespacePsvi[] = { //http://apache.org/xml/2001/PSVInfosetExtension
	chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash, 
	chLatin_a, chLatin_p, chLatin_a, chLatin_c, chLatin_h, chLatin_e, chPeriod, chLatin_o, chLatin_r, chLatin_g, 
	chForwardSlash, chLatin_x, chLatin_m, chLatin_l, chForwardSlash, chDigit_2, chDigit_0, chDigit_0, 
	chDigit_1, chForwardSlash, chLatin_P, chLatin_S, chLatin_V, chLatin_I, chLatin_n, chLatin_f, chLatin_o, 
	chLatin_s, chLatin_e, chLatin_t, chLatin_E, chLatin_x, chLatin_t, chLatin_e, chLatin_n, chLatin_s, 
	chLatin_i, chLatin_o, chLatin_n, chNull
};
const XMLCh PSVIUni::fgXml[] =
{
	chLatin_x, chLatin_m, chLatin_l, chNull
};
const XMLCh PSVIUni::fgNamespaceXmlSchema[] = { //http://www.w3.org/2001/XMLSchema
	chLatin_h, chLatin_t, chLatin_t, chLatin_p, chColon, chForwardSlash, chForwardSlash, chLatin_w, 
	chLatin_w, chLatin_w, chPeriod, chLatin_w, chDigit_3, chPeriod, chLatin_o, chLatin_r, chLatin_g,
	chForwardSlash, chDigit_2, chDigit_0, chDigit_0, chDigit_1, chForwardSlash, chLatin_X, chLatin_M, 
	chLatin_L, chLatin_S, chLatin_c, chLatin_h, chLatin_e, chLatin_m, chLatin_a, chNull	
};
