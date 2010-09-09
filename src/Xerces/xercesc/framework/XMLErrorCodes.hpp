// This file is generated, don't edit it!!

#if !defined(XERCESC_INCLUDE_GUARD_ERRHEADER_XMLErrs)
#define XERCESC_INCLUDE_GUARD_ERRHEADER_XMLErrs

#include <xercesc/framework/XMLErrorReporter.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/dom/DOMError.hpp>

XERCES_CPP_NAMESPACE_BEGIN

class XMLErrs
{
public :
    enum Codes
    {
        NoError                            = 0
      , W_LowBounds                        = 1
      , NotationAlreadyExists              = 2
      , AttListAlreadyExists               = 3
      , ContradictoryEncoding              = 4
      , UndeclaredElemInCM                 = 5
      , UndeclaredElemInAttList            = 6
      , XMLException_Warning               = 7
      , XIncludeResourceErrorWarning       = 8
      , XIncludeCannotOpenFile             = 9
      , XIncludeIncludeFailedResourceError   = 10
      , W_HighBounds                       = 11
      , E_LowBounds                        = 12
      , FeatureUnsupported                 = 13
      , TopLevelNoNameComplexType          = 14
      , TopLevelNoNameAttribute            = 15
      , NoNameRefAttribute                 = 16
      , NoNameRefElement                   = 17
      , NoNameRefGroup                     = 18
      , NoNameRefAttGroup                  = 19
      , AnonComplexTypeWithName            = 20
      , AnonSimpleTypeWithName             = 21
      , InvalidElementContent              = 22
      , SimpleTypeContentError             = 23
      , ExpectedSimpleTypeInList           = 24
      , ListUnionRestrictionError          = 25
      , SimpleTypeDerivationByListError    = 26
      , ExpectedSimpleTypeInRestriction    = 27
      , DuplicateFacet                     = 28
      , ExpectedSimpleTypeInUnion          = 29
      , EmptySimpleTypeContent             = 30
      , InvalidSimpleContent               = 31
      , UnspecifiedBase                    = 32
      , InvalidComplexContent              = 33
      , SchemaElementContentError          = 34
      , ContentError                       = 35
      , UnknownSimpleType                  = 36
      , UnknownComplexType                 = 37
      , UnresolvedPrefix                   = 38
      , RefElementNotFound                 = 39
      , TypeNotFound                       = 40
      , TopLevelAttributeNotFound          = 41
      , InvalidChildInComplexType          = 42
      , BaseTypeNotFound                   = 43
      , DatatypeValidatorCreationError     = 44
      , InvalidChildFollowingSimpleContent   = 45
      , InvalidChildFollowingConplexContent   = 46
      , AttributeDefaultFixedValue         = 47
      , NotOptionalDefaultAttValue         = 48
      , DuplicateAttribute                 = 49
      , AttributeWithTypeAndSimpleType     = 50
      , AttributeSimpleTypeNotFound        = 51
      , ElementWithFixedAndDefault         = 52
      , InvalidDeclarationName             = 53
      , ElementWithTypeAndAnonType         = 54
      , NotSimpleOrMixedElement            = 55
      , DisallowedSimpleTypeExtension      = 56
      , InvalidSimpleContentBase           = 57
      , InvalidComplexTypeBase             = 58
      , InvalidChildInSimpleContent        = 59
      , InvalidChildInComplexContent       = 60
      , AnnotationError                    = 61
      , DisallowedBaseDerivation           = 62
      , InvalidBlockValue                  = 63
      , InvalidFinalValue                  = 64
      , InvalidSubstitutionGroupElement    = 65
      , SubstitutionGroupTypeMismatch      = 66
      , DuplicateElementDeclaration        = 67
      , InvalidAttValue                    = 68
      , AttributeRefContentError           = 69
      , DuplicateRefAttribute              = 70
      , ForbiddenDerivationByRestriction   = 71
      , ForbiddenDerivationByExtension     = 72
      , BaseNotComplexType                 = 73
      , ImportNamespaceDifference          = 74
      , DeclarationNoSchemaLocation        = 75
      , IncludeNamespaceDifference         = 76
      , OnlyAnnotationExpected             = 77
      , InvalidAttributeContent            = 78
      , AttributeRequiredGlobal            = 79
      , AttributeRequiredLocal             = 80
      , AttributeDisallowedGlobal          = 81
      , AttributeDisallowedLocal           = 82
      , InvalidMin2MaxOccurs               = 83
      , AnyAttributeContentError           = 84
      , NoNameGlobalElement                = 85
      , NoCircularDefinition               = 86
      , DuplicateGlobalType                = 87
      , DuplicateGlobalDeclaration         = 88
      , WS_CollapseExpected                = 89
      , Import_1_1                         = 90
      , Import_1_2                         = 91
      , ElemIDValueConstraint              = 92
      , NoNotationType                     = 93
      , EmptiableMixedContent              = 94
      , EmptyComplexRestrictionDerivation   = 95
      , MixedOrElementOnly                 = 96
      , InvalidContentRestriction          = 97
      , ForbiddenDerivation                = 98
      , AtomicItemType                     = 99
      , GroupContentError                  = 100
      , AttGroupContentError               = 101
      , MinMaxOnGroupChild                 = 102
      , DeclarationNotFound                = 103
      , AllContentLimited                  = 104
      , BadMinMaxAllCT                     = 105
      , BadMinMaxAllElem                   = 106
      , DuplicateAttInDerivation           = 107
      , NotExpressibleWildCardIntersection   = 108
      , BadAttDerivation_1                 = 109
      , BadAttDerivation_2                 = 110
      , BadAttDerivation_3                 = 111
      , BadAttDerivation_4                 = 112
      , BadAttDerivation_5                 = 113
      , BadAttDerivation_6                 = 114
      , BadAttDerivation_7                 = 115
      , BadAttDerivation_8                 = 116
      , BadAttDerivation_9                 = 117
      , AllContentError                    = 118
      , RedefineNamespaceDifference        = 119
      , Redefine_InvalidSimpleType         = 120
      , Redefine_InvalidSimpleTypeBase     = 121
      , Redefine_InvalidComplexType        = 122
      , Redefine_InvalidComplexTypeBase    = 123
      , Redefine_InvalidGroupMinMax        = 124
      , Redefine_DeclarationNotFound       = 125
      , Redefine_GroupRefCount             = 126
      , Redefine_AttGroupRefCount          = 127
      , Redefine_InvalidChild              = 128
      , Notation_DeclNotFound              = 129
      , IC_DuplicateDecl                   = 130
      , IC_BadContent                      = 131
      , IC_KeyRefReferNotFound             = 132
      , IC_KeyRefCardinality               = 133
      , IC_XPathExprMissing                = 134
      , AttUseCorrect                      = 135
      , AttDeclPropCorrect3                = 136
      , AttDeclPropCorrect5                = 137
      , AttGrpPropCorrect3                 = 138
      , InvalidTargetNSValue               = 139
      , XMLException_Error                 = 140
      , InvalidRedefine                    = 141
      , InvalidNSReference                 = 142
      , NotAllContent                      = 143
      , InvalidAnnotationContent           = 144
      , InvalidFacetName                   = 145
      , InvalidXMLSchemaRoot               = 146
      , CircularSubsGroup                  = 147
      , ELTSchemaNS                        = 148
      , InvalidAttTNS                      = 149
      , NSDeclInvalid                      = 150
      , DOMLevel1Node                      = 151
      , DuplicateAnyAttribute              = 152
      , AnyAttributeBeforeAttribute        = 153
      , E_HighBounds                       = 154
      , F_LowBounds                        = 155
      , EntityExpansionLimitExceeded       = 156
      , ExpectedCommentOrCDATA             = 157
      , ExpectedAttrName                   = 158
      , ExpectedNotationName               = 159
      , NoRepInMixed                       = 160
      , ExpectedDefAttrDecl                = 161
      , ExpectedEqSign                     = 162
      , ExpectedElementName                = 163
      , CommentsMustStartWith              = 164
      , InvalidDocumentStructure           = 165
      , ExpectedDeclString                 = 166
      , BadXMLVersion                      = 167
      , UnsupportedXMLVersion              = 168
      , UnterminatedXMLDecl                = 169
      , BadXMLEncoding                     = 170
      , BadStandalone                      = 171
      , UnterminatedComment                = 172
      , PINameExpected                     = 173
      , UnterminatedPI                     = 174
      , InvalidCharacter                   = 175
      , UnterminatedStartTag               = 176
      , ExpectedAttrValue                  = 177
      , UnterminatedEndTag                 = 178
      , ExpectedAttributeType              = 179
      , ExpectedEndOfTagX                  = 180
      , ExpectedMarkup                     = 181
      , NotValidAfterContent               = 182
      , ExpectedComment                    = 183
      , ExpectedCommentOrPI                = 184
      , ExpectedWhitespace                 = 185
      , NoRootElemInDOCTYPE                = 186
      , ExpectedQuotedString               = 187
      , ExpectedPublicId                   = 188
      , InvalidPublicIdChar                = 189
      , UnterminatedDOCTYPE                = 190
      , InvalidCharacterInIntSubset        = 191
      , UnexpectedWhitespace               = 192
      , InvalidCharacterInAttrValue        = 193
      , ExpectedMarkupDecl                 = 194
      , TextDeclNotLegalHere               = 195
      , ConditionalSectInIntSubset         = 196
      , ExpectedPEName                     = 197
      , UnterminatedEntityDecl             = 198
      , InvalidCharacterRef                = 199
      , UnterminatedCharRef                = 200
      , ExpectedEntityRefName              = 201
      , EntityNotFound                     = 202
      , NoUnparsedEntityRefs               = 203
      , UnterminatedEntityRef              = 204
      , RecursiveEntity                    = 205
      , PartialMarkupInEntity              = 206
      , UnterminatedElementDecl            = 207
      , ExpectedContentSpecExpr            = 208
      , ExpectedAsterisk                   = 209
      , UnterminatedContentModel           = 210
      , ExpectedSystemOrPublicId           = 211
      , UnterminatedNotationDecl           = 212
      , ExpectedSeqChoiceLeaf              = 213
      , ExpectedChoiceOrCloseParen         = 214
      , ExpectedSeqOrCloseParen            = 215
      , ExpectedEnumValue                  = 216
      , ExpectedEnumSepOrParen             = 217
      , UnterminatedEntityLiteral          = 218
      , MoreEndThanStartTags               = 219
      , ExpectedOpenParen                  = 220
      , AttrAlreadyUsedInSTag              = 221
      , BracketInAttrValue                 = 222
      , Expected2ndSurrogateChar           = 223
      , ExpectedEndOfConditional           = 224
      , ExpectedIncOrIgn                   = 225
      , ExpectedINCLUDEBracket             = 226
      , UnexpectedEOE                      = 227
      , PEPropogated                       = 228
      , ExtraCloseSquare                   = 229
      , PERefInMarkupInIntSubset           = 230
      , EntityPropogated                   = 231
      , ExpectedNumericalCharRef           = 232
      , ExpectedOpenSquareBracket          = 233
      , BadSequenceInCharData              = 234
      , IllegalSequenceInComment           = 235
      , UnterminatedCDATASection           = 236
      , ExpectedNDATA                      = 237
      , NDATANotValidForPE                 = 238
      , HexRadixMustBeLowerCase            = 239
      , DeclStringRep                      = 240
      , DeclStringsInWrongOrder            = 241
      , NoExtRefsInAttValue                = 242
      , XMLDeclMustBeLowerCase             = 243
      , ExpectedEntityValue                = 244
      , BadDigitForRadix                   = 245
      , EndedWithTagsOnStack               = 246
      , NestedCDATA                        = 247
      , UnknownPrefix                      = 248
      , PartialTagMarkupError              = 249
      , EmptyMainEntity                    = 250
      , CDATAOutsideOfContent              = 251
      , Unexpected2ndSurrogateChar         = 252
      , NoPIStartsWithXML                  = 253
      , XMLDeclMustBeFirst                 = 254
      , XMLVersionRequired                 = 255
      , StandaloneNotLegal                 = 256
      , EncodingRequired                   = 257
      , ColonNotLegalWithNS                = 258
      , XMLException_Fatal                 = 259
      , BadSchemaLocation                  = 260
      , SchemaScanFatalError               = 261
      , IllegalRefInStandalone             = 262
      , PEBetweenDecl                      = 263
      , NoEmptyStrNamespace                = 264
      , NoUseOfxmlnsAsPrefix               = 265
      , NoUseOfxmlnsURI                    = 266
      , PrefixXMLNotMatchXMLURI            = 267
      , XMLURINotMatchXMLPrefix            = 268
      , NoXMLNSAsElementPrefix             = 269
      , CT_SimpleTypeChildRequired         = 270
      , InvalidRootElemInDOCTYPE           = 271
      , InvalidElementName                 = 272
      , InvalidAttrName                    = 273
      , InvalidEntityRefName               = 274
      , DuplicateDocTypeDecl               = 275
      , XIncludeOrphanFallback             = 276
      , XIncludeNoHref                     = 277
      , XIncludeXPointerNotSupported       = 278
      , XIncludeInvalidParseVal            = 279
      , XIncludeMultipleFallbackElems      = 280
      , XIncludeIncludeFailedNoFallback    = 281
      , XIncludeCircularInclusionLoop      = 282
      , XIncludeCircularInclusionDocIncludesSelf   = 283
      , XIncludeDisallowedChild            = 284
      , XIncludeConflictingNotation        = 285
      , XIncludeConflictingEntity          = 286
      , F_HighBounds                       = 287
    };

    static bool isFatal(const XMLErrs::Codes toCheck)
    {
        return ((toCheck >= F_LowBounds) && (toCheck <= F_HighBounds));
    }

    static bool isWarning(const XMLErrs::Codes toCheck)
    {
        return ((toCheck >= W_LowBounds) && (toCheck <= W_HighBounds));
    }

    static bool isError(const XMLErrs::Codes toCheck)
    {
        return ((toCheck >= E_LowBounds) && (toCheck <= E_HighBounds));
    }

    static XMLErrorReporter::ErrTypes errorType(const XMLErrs::Codes toCheck)
    {
       if ((toCheck >= W_LowBounds) && (toCheck <= W_HighBounds))
           return XMLErrorReporter::ErrType_Warning;
       else if ((toCheck >= F_LowBounds) && (toCheck <= F_HighBounds))
            return XMLErrorReporter::ErrType_Fatal;
       else if ((toCheck >= E_LowBounds) && (toCheck <= E_HighBounds))
            return XMLErrorReporter::ErrType_Error;
       return XMLErrorReporter::ErrTypes_Unknown;
    }
    static DOMError::ErrorSeverity  DOMErrorType(const XMLErrs::Codes toCheck)
    {
       if ((toCheck >= W_LowBounds) && (toCheck <= W_HighBounds))
           return DOMError::DOM_SEVERITY_WARNING;
       else if ((toCheck >= F_LowBounds) && (toCheck <= F_HighBounds))
            return DOMError::DOM_SEVERITY_FATAL_ERROR;
       else return DOMError::DOM_SEVERITY_ERROR;
    }

private:
    // -----------------------------------------------------------------------
    //  Unimplemented constructors and operators
    // -----------------------------------------------------------------------
    XMLErrs();
};

XERCES_CPP_NAMESPACE_END

#endif

