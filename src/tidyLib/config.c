/*
  config.c -- read config file and manage config properties
  
  (c) 1998-2008 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2008/06/18 20:18:54 $ 
    $Revision: 1.111 $ 

*/

/*
  config files associate a property name with a value.

  // comments can start at the beginning of a line
  # comments can start at the beginning of a line
  name: short values fit onto one line
  name: a really long value that
   continues on the next line

  property names are case insensitive and should be less than
  60 characters in length and must start at the begining of
  the line, as whitespace at the start of a line signifies a
  line continuation.
*/

#include "config.h"
#include "tidy-int.h"
#include "message.h"
#include "tmbstr.h"
#include "tags.h"

#ifdef WINDOWS_OS
#include <io.h>
#else
#ifdef DMALLOC
/*
   macro for valloc() in dmalloc.h may conflict with declaration for valloc() in unistd.h -
   we don't need (debugging for) valloc() here. dmalloc.h should come last but it doesn't.
*/
#ifdef valloc
#undef valloc
#endif
#endif
#include <unistd.h>
#endif

#ifdef TIDY_WIN32_MLANG_SUPPORT
#include "win32tc.h"
#endif

void TY_(InitConfig)( TidyDocImpl* doc )
{
    TidyClearMemory( &doc->config, sizeof(TidyConfigImpl) );
    TY_(ResetConfigToDefault)( doc );
}

void TY_(FreeConfig)( TidyDocImpl* doc )
{
    TY_(ResetConfigToDefault)( doc );
    TY_(TakeConfigSnapshot)( doc );
}


/* Arrange so index can be cast to enum
*/
static const ctmbstr boolPicks[] = 
{
  "no",
  "yes",
  NULL
};

static const ctmbstr autoBoolPicks[] = 
{
  "no",
  "yes",
  "auto",
  NULL
};

static const ctmbstr repeatAttrPicks[] = 
{
  "keep-first",
  "keep-last",
  NULL
};

static const ctmbstr accessPicks[] = 
{
  "0 (Tidy Classic)",
  "1 (Priority 1 Checks)",
  "2 (Priority 2 Checks)",
  "3 (Priority 3 Checks)",
  NULL
};

static const ctmbstr charEncPicks[] = 
{
  "raw",
  "ascii",
  "latin0",
  "latin1",
  "utf8",
#ifndef NO_NATIVE_ISO2022_SUPPORT
  "iso2022",
#endif
  "mac",
  "win1252",
  "ibm858",

#if SUPPORT_UTF16_ENCODINGS
  "utf16le",
  "utf16be",
  "utf16",
#endif

#if SUPPORT_ASIAN_ENCODINGS
  "big5",
  "shiftjis",
#endif

  NULL
};

static const ctmbstr newlinePicks[] = 
{
  "LF",
  "CRLF",
  "CR",
  NULL
};

static const ctmbstr doctypePicks[] = 
{
  "omit",
  "auto",
  "strict",
  "transitional",
  "user",
  NULL 
};

static const ctmbstr sorterPicks[] = 
{
  "none",
  "alpha",
  NULL
};

#define MU TidyMarkup
#define DG TidyDiagnostics
#define PP TidyPrettyPrint
#define CE TidyEncoding
#define MS TidyMiscellaneous

#define IN TidyInteger
#define BL TidyBoolean
#define ST TidyString

#define XX (TidyConfigCategory)-1
#define XY (TidyOptionType)-1

#define DLF DEFAULT_NL_CONFIG

/* If Accessibility checks not supported, make config setting read-only */
#if SUPPORT_ACCESSIBILITY_CHECKS
#define ParseAcc ParseInt
#else
#define ParseAcc NULL 
#endif

static void AdjustConfig( TidyDocImpl* doc );

/* parser for integer values */
static ParseProperty ParseInt;

/* parser for 't'/'f', 'true'/'false', 'y'/'n', 'yes'/'no' or '1'/'0' */
static ParseProperty ParseBool;

/* parser for 't'/'f', 'true'/'false', 'y'/'n', 'yes'/'no', '1'/'0'
   or 'auto' */
static ParseProperty ParseAutoBool;

/* a string excluding whitespace */
static ParseProperty ParseName;

/* a CSS1 selector - CSS class naming for -clean option */
static ParseProperty ParseCSS1Selector;

/* a string including whitespace */
static ParseProperty ParseString;

/* a space or comma separated list of tag names */
static ParseProperty ParseTagNames;

/* alpha */
static ParseProperty ParseSorter;

/* RAW, ASCII, LATIN0, LATIN1, UTF8, ISO2022, MACROMAN, 
   WIN1252, IBM858, UTF16LE, UTF16BE, UTF16, BIG5, SHIFTJIS
*/
static ParseProperty ParseCharEnc;
static ParseProperty ParseNewline;

/* omit | auto | strict | loose | <fpi> */
static ParseProperty ParseDocType;

/* keep-first or keep-last? */
static ParseProperty ParseRepeatAttr;


static const TidyOptionImpl option_defs[] =
{
  { TidyUnknownOption,           MS, "unknown!",                    IN, 0,               NULL,              NULL            },
  { TidyIndentSpaces,            PP, "indent-spaces",               IN, 2,               ParseInt,          NULL            },
  { TidyWrapLen,                 PP, "wrap",                        IN, 68,              ParseInt,          NULL            },
  { TidyTabSize,                 PP, "tab-size",                    IN, 8,               ParseInt,          NULL            },
  { TidyCharEncoding,            CE, "char-encoding",               IN, ASCII,           ParseCharEnc,      charEncPicks    },
  { TidyInCharEncoding,          CE, "input-encoding",              IN, LATIN1,          ParseCharEnc,      charEncPicks    },
  { TidyOutCharEncoding,         CE, "output-encoding",             IN, ASCII,           ParseCharEnc,      charEncPicks    },
  { TidyNewline,                 CE, "newline",                     IN, DLF,             ParseNewline,      newlinePicks    },
  { TidyDoctypeMode,             MU, "doctype-mode",                IN, TidyDoctypeAuto, NULL,              doctypePicks    },
  { TidyDoctype,                 MU, "doctype",                     ST, 0,               ParseDocType,      doctypePicks    },
  { TidyDuplicateAttrs,          MU, "repeated-attributes",         IN, TidyKeepLast,    ParseRepeatAttr,   repeatAttrPicks },
  { TidyAltText,                 MU, "alt-text",                    ST, 0,               ParseString,       NULL            },

  /* obsolete */
  { TidySlideStyle,              MS, "slide-style",                 ST, 0,               ParseName,         NULL            },

  { TidyErrFile,                 MS, "error-file",                  ST, 0,               ParseString,       NULL            },
  { TidyOutFile,                 MS, "output-file",                 ST, 0,               ParseString,       NULL            },
  { TidyWriteBack,               MS, "write-back",                  BL, no,              ParseBool,         boolPicks       },
  { TidyShowMarkup,              PP, "markup",                      BL, yes,             ParseBool,         boolPicks       },
  { TidyShowWarnings,            DG, "show-warnings",               BL, yes,             ParseBool,         boolPicks       },
  { TidyQuiet,                   MS, "quiet",                       BL, no,              ParseBool,         boolPicks       },
  { TidyIndentContent,           PP, "indent",                      IN, TidyNoState,     ParseAutoBool,     autoBoolPicks   },
  { TidyHideEndTags,             MU, "hide-endtags",                BL, no,              ParseBool,         boolPicks       },
  { TidyXmlTags,                 MU, "input-xml",                   BL, no,              ParseBool,         boolPicks       },
  { TidyXmlOut,                  MU, "output-xml",                  BL, no,              ParseBool,         boolPicks       },
  { TidyXhtmlOut,                MU, "output-xhtml",                BL, no,              ParseBool,         boolPicks       },
  { TidyHtmlOut,                 MU, "output-html",                 BL, no,              ParseBool,         boolPicks       },
  { TidyXmlDecl,                 MU, "add-xml-decl",                BL, no,              ParseBool,         boolPicks       },
  { TidyUpperCaseTags,           MU, "uppercase-tags",              BL, no,              ParseBool,         boolPicks       },
  { TidyUpperCaseAttrs,          MU, "uppercase-attributes",        BL, no,              ParseBool,         boolPicks       },
  { TidyMakeBare,                MU, "bare",                        BL, no,              ParseBool,         boolPicks       },
  { TidyMakeClean,               MU, "clean",                       BL, no,              ParseBool,         boolPicks       },
  { TidyLogicalEmphasis,         MU, "logical-emphasis",            BL, no,              ParseBool,         boolPicks       },
  { TidyDropPropAttrs,           MU, "drop-proprietary-attributes", BL, no,              ParseBool,         boolPicks       },
  { TidyDropFontTags,            MU, "drop-font-tags",              BL, no,              ParseBool,         boolPicks       },
  { TidyDropEmptyParas,          MU, "drop-empty-paras",            BL, yes,             ParseBool,         boolPicks       },
  { TidyFixComments,             MU, "fix-bad-comments",            BL, yes,             ParseBool,         boolPicks       },
  { TidyBreakBeforeBR,           PP, "break-before-br",             BL, no,              ParseBool,         boolPicks       },

  /* obsolete */
  { TidyBurstSlides,             PP, "split",                       BL, no,              ParseBool,         boolPicks       },

  { TidyNumEntities,             MU, "numeric-entities",            BL, no,              ParseBool,         boolPicks       },
  { TidyQuoteMarks,              MU, "quote-marks",                 BL, no,              ParseBool,         boolPicks       },
  { TidyQuoteNbsp,               MU, "quote-nbsp",                  BL, yes,             ParseBool,         boolPicks       },
  { TidyQuoteAmpersand,          MU, "quote-ampersand",             BL, yes,             ParseBool,         boolPicks       },
  { TidyWrapAttVals,             PP, "wrap-attributes",             BL, no,              ParseBool,         boolPicks       },
  { TidyWrapScriptlets,          PP, "wrap-script-literals",        BL, no,              ParseBool,         boolPicks       },
  { TidyWrapSection,             PP, "wrap-sections",               BL, yes,             ParseBool,         boolPicks       },
  { TidyWrapAsp,                 PP, "wrap-asp",                    BL, yes,             ParseBool,         boolPicks       },
  { TidyWrapJste,                PP, "wrap-jste",                   BL, yes,             ParseBool,         boolPicks       },
  { TidyWrapPhp,                 PP, "wrap-php",                    BL, yes,             ParseBool,         boolPicks       },
  { TidyFixBackslash,            MU, "fix-backslash",               BL, yes,             ParseBool,         boolPicks       },
  { TidyIndentAttributes,        PP, "indent-attributes",           BL, no,              ParseBool,         boolPicks       },
  { TidyXmlPIs,                  MU, "assume-xml-procins",          BL, no,              ParseBool,         boolPicks       },
  { TidyXmlSpace,                MU, "add-xml-space",               BL, no,              ParseBool,         boolPicks       },
  { TidyEncloseBodyText,         MU, "enclose-text",                BL, no,              ParseBool,         boolPicks       },
  { TidyEncloseBlockText,        MU, "enclose-block-text",          BL, no,              ParseBool,         boolPicks       },
  { TidyKeepFileTimes,           MS, "keep-time",                   BL, no,              ParseBool,         boolPicks       },
  { TidyWord2000,                MU, "word-2000",                   BL, no,              ParseBool,         boolPicks       },
  { TidyMark,                    MS, "tidy-mark",                   BL, yes,             ParseBool,         boolPicks       },
  { TidyEmacs,                   MS, "gnu-emacs",                   BL, no,              ParseBool,         boolPicks       },
  { TidyEmacsFile,               MS, "gnu-emacs-file",              ST, 0,               ParseString,       NULL            },
  { TidyLiteralAttribs,          MU, "literal-attributes",          BL, no,              ParseBool,         boolPicks       },
  { TidyBodyOnly,                MU, "show-body-only",              IN, no,              ParseAutoBool,     autoBoolPicks   },
  { TidyFixUri,                  MU, "fix-uri",                     BL, yes,             ParseBool,         boolPicks       },
  { TidyLowerLiterals,           MU, "lower-literals",              BL, yes,             ParseBool,         boolPicks       },
  { TidyHideComments,            MU, "hide-comments",               BL, no,              ParseBool,         boolPicks       },
  { TidyIndentCdata,             MU, "indent-cdata",                BL, no,              ParseBool,         boolPicks       },
  { TidyForceOutput,             MS, "force-output",                BL, no,              ParseBool,         boolPicks       },
  { TidyShowErrors,              DG, "show-errors",                 IN, 6,               ParseInt,          NULL            },
  { TidyAsciiChars,              CE, "ascii-chars",                 BL, no,              ParseBool,         boolPicks       },
  { TidyJoinClasses,             MU, "join-classes",                BL, no,              ParseBool,         boolPicks       },
  { TidyJoinStyles,              MU, "join-styles",                 BL, yes,             ParseBool,         boolPicks       },
  { TidyEscapeCdata,             MU, "escape-cdata",                BL, no,              ParseBool,         boolPicks       },
#if SUPPORT_ASIAN_ENCODINGS
  { TidyLanguage,                CE, "language",                    ST, 0,               ParseName,         NULL            },
  { TidyNCR,                     MU, "ncr",                         BL, yes,             ParseBool,         boolPicks       },
#endif
#if SUPPORT_UTF16_ENCODINGS
  { TidyOutputBOM,               CE, "output-bom",                  IN, TidyAutoState,   ParseAutoBool,     autoBoolPicks   },
#endif
  { TidyReplaceColor,            MU, "replace-color",               BL, no,              ParseBool,         boolPicks       },
  { TidyCSSPrefix,               MU, "css-prefix",                  ST, 0,               ParseCSS1Selector, NULL            },
  { TidyInlineTags,              MU, "new-inline-tags",             ST, 0,               ParseTagNames,     NULL            },
  { TidyBlockTags,               MU, "new-blocklevel-tags",         ST, 0,               ParseTagNames,     NULL            },
  { TidyEmptyTags,               MU, "new-empty-tags",              ST, 0,               ParseTagNames,     NULL            },
  { TidyPreTags,                 MU, "new-pre-tags",                ST, 0,               ParseTagNames,     NULL            },
  { TidyAccessibilityCheckLevel, DG, "accessibility-check",         IN, 0,               ParseAcc,          accessPicks     },
  { TidyVertSpace,               PP, "vertical-space",              BL, no,              ParseBool,         boolPicks       },
#if SUPPORT_ASIAN_ENCODINGS
  { TidyPunctWrap,               PP, "punctuation-wrap",            BL, no,              ParseBool,         boolPicks       },
#endif
  { TidyMergeDivs,               MU, "merge-divs",                  IN, TidyAutoState,   ParseAutoBool,     autoBoolPicks   },
  { TidyDecorateInferredUL,      MU, "decorate-inferred-ul",        BL, no,              ParseBool,         boolPicks       },
  { TidyPreserveEntities,        MU, "preserve-entities",           BL, no,              ParseBool,         boolPicks       },
  { TidySortAttributes,          PP, "sort-attributes",             IN, TidySortAttrNone,ParseSorter,       sorterPicks     },
  { TidyMergeSpans,              MU, "merge-spans",                 IN, TidyAutoState,   ParseAutoBool,     autoBoolPicks   },
  { TidyAnchorAsName,            MU, "anchor-as-name",              BL, yes,             ParseBool,         boolPicks       },

  // Added by Strahinja Markovic
  { TidyClassStartID,            MS, "class-start-id",              IN, 0,               ParseInt,          NULL            },

  { N_TIDY_OPTIONS,              XX, NULL,                          XY, 0,               NULL,              NULL            }
};

/* Should only be called by options set by name
** thus, it is cheaper to do a few scans than set
** up every option in a hash table.
*/
const TidyOptionImpl* TY_(lookupOption)( ctmbstr s )
{
    const TidyOptionImpl* np = option_defs;
    for ( /**/; np < option_defs + N_TIDY_OPTIONS; ++np )
    {
        if ( TY_(tmbstrcasecmp)(s, np->name) == 0 )
            return np;
    }
    return NULL;
}

const TidyOptionImpl* TY_(getOption)( TidyOptionId optId )
{
  if ( optId < N_TIDY_OPTIONS )
      return option_defs + optId;
  return NULL;
}


static void FreeOptionValue( TidyDocImpl* doc, const TidyOptionImpl* option, TidyOptionValue* value )
{
    if ( option->type == TidyString && value->p && value->p != option->pdflt )
        TidyDocFree( doc, value->p );
}

static void CopyOptionValue( TidyDocImpl* doc, const TidyOptionImpl* option,
                             TidyOptionValue* oldval, const TidyOptionValue* newval )
{
    assert( oldval != NULL );
    FreeOptionValue( doc, option, oldval );

    if ( option->type == TidyString )
    {
        if ( newval->p && newval->p != option->pdflt )
            oldval->p = TY_(tmbstrdup)( doc->allocator, newval->p );
        else
            oldval->p = newval->p;
    }
    else
        oldval->v = newval->v;
}


static Bool SetOptionValue( TidyDocImpl* doc, TidyOptionId optId, ctmbstr val )
{
   const TidyOptionImpl* option = &option_defs[ optId ];
   Bool status = ( optId < N_TIDY_OPTIONS );
   if ( status )
   {
      assert( option->id == optId && option->type == TidyString );
      FreeOptionValue( doc, option, &doc->config.value[ optId ] );
      doc->config.value[ optId ].p = TY_(tmbstrdup)( doc->allocator, val );
   }
   return status;
}

Bool TY_(SetOptionInt)( TidyDocImpl* doc, TidyOptionId optId, ulong val )
{
   Bool status = ( optId < N_TIDY_OPTIONS );
   if ( status )
   {
       assert( option_defs[ optId ].type == TidyInteger );
       doc->config.value[ optId ].v = val;
   }
   return status;
}

Bool TY_(SetOptionBool)( TidyDocImpl* doc, TidyOptionId optId, Bool val )
{
   Bool status = ( optId < N_TIDY_OPTIONS );
   if ( status )
   {
       assert( option_defs[ optId ].type == TidyBoolean );
       doc->config.value[ optId ].v = val;
   }
   return status;
}

static void GetOptionDefault( const TidyOptionImpl* option,
                              TidyOptionValue* dflt )
{
    if ( option->type == TidyString )
        dflt->p = (char*)option->pdflt;
    else
        dflt->v = option->dflt;
}

static Bool OptionValueEqDefault( const TidyOptionImpl* option,
                                  const TidyOptionValue* val )
{
    return ( option->type == TidyString ) ?
        val->p == option->pdflt :
        val->v == option->dflt;
}

Bool TY_(ResetOptionToDefault)( TidyDocImpl* doc, TidyOptionId optId )
{
    Bool status = ( optId > 0 && optId < N_TIDY_OPTIONS );
    if ( status )
    {
        TidyOptionValue dflt;
        const TidyOptionImpl* option = option_defs + optId;
        TidyOptionValue* value = &doc->config.value[ optId ];
        assert( optId == option->id );
        GetOptionDefault( option, &dflt );
        CopyOptionValue( doc, option, value, &dflt );
    }
    return status;
}

static void ReparseTagType( TidyDocImpl* doc, TidyOptionId optId )
{
    ctmbstr tagdecl = cfgStr( doc, optId );
    tmbstr dupdecl = TY_(tmbstrdup)( doc->allocator, tagdecl );
    TY_(ParseConfigValue)( doc, optId, dupdecl );
    TidyDocFree( doc, dupdecl );
}

static Bool OptionValueIdentical( const TidyOptionImpl* option,
                                  const TidyOptionValue* val1,
                                  const TidyOptionValue* val2 )
{
    if ( option->type == TidyString )
    {
        if ( val1->p == val2->p )
            return yes;
        if ( !val1->p || !val2->p )
            return no;
        return TY_(tmbstrcmp)( val1->p, val2->p ) == 0;
    }
    else
        return val1->v == val2->v;
}

static Bool NeedReparseTagDecls( const TidyOptionValue* current,
                                 const TidyOptionValue* new,
                                 uint *changedUserTags )
{
    Bool ret = no;
    uint ixVal;
    const TidyOptionImpl* option = option_defs;
    *changedUserTags = tagtype_null;

    for ( ixVal=0; ixVal < N_TIDY_OPTIONS; ++option, ++ixVal )
    {
        assert( ixVal == (uint) option->id );
        switch (option->id)
        {
#define TEST_USERTAGS(USERTAGOPTION,USERTAGTYPE) \
        case USERTAGOPTION: \
            if (!OptionValueIdentical(option,&current[ixVal],&new[ixVal])) \
            { \
                *changedUserTags |= USERTAGTYPE; \
                ret = yes; \
            } \
            break
            TEST_USERTAGS(TidyInlineTags,tagtype_inline);
            TEST_USERTAGS(TidyBlockTags,tagtype_block);
            TEST_USERTAGS(TidyEmptyTags,tagtype_empty);
            TEST_USERTAGS(TidyPreTags,tagtype_pre);
        default:
            break;
        }
    }
    return ret;
}

static void ReparseTagDecls( TidyDocImpl* doc, uint changedUserTags  )
{
#define REPARSE_USERTAGS(USERTAGOPTION,USERTAGTYPE) \
    if ( changedUserTags & USERTAGTYPE ) \
    { \
        TY_(FreeDeclaredTags)( doc, USERTAGTYPE ); \
        ReparseTagType( doc, USERTAGOPTION ); \
    }
    REPARSE_USERTAGS(TidyInlineTags,tagtype_inline);
    REPARSE_USERTAGS(TidyBlockTags,tagtype_block);
    REPARSE_USERTAGS(TidyEmptyTags,tagtype_empty);
    REPARSE_USERTAGS(TidyPreTags,tagtype_pre);
}

void TY_(ResetConfigToDefault)( TidyDocImpl* doc )
{
    uint ixVal;
    const TidyOptionImpl* option = option_defs;
    TidyOptionValue* value = &doc->config.value[ 0 ];
    for ( ixVal=0; ixVal < N_TIDY_OPTIONS; ++option, ++ixVal )
    {
        TidyOptionValue dflt;
        assert( ixVal == (uint) option->id );
        GetOptionDefault( option, &dflt );
        CopyOptionValue( doc, option, &value[ixVal], &dflt );
    }
    TY_(FreeDeclaredTags)( doc, tagtype_null );
}

void TY_(TakeConfigSnapshot)( TidyDocImpl* doc )
{
    uint ixVal;
    const TidyOptionImpl* option = option_defs;
    const TidyOptionValue* value = &doc->config.value[ 0 ];
    TidyOptionValue* snap  = &doc->config.snapshot[ 0 ];

    AdjustConfig( doc );  /* Make sure it's consistent */
    for ( ixVal=0; ixVal < N_TIDY_OPTIONS; ++option, ++ixVal )
    {
        assert( ixVal == (uint) option->id );
        CopyOptionValue( doc, option, &snap[ixVal], &value[ixVal] );
    }
}

void TY_(ResetConfigToSnapshot)( TidyDocImpl* doc )
{
    uint ixVal;
    const TidyOptionImpl* option = option_defs;
    TidyOptionValue* value = &doc->config.value[ 0 ];
    const TidyOptionValue* snap  = &doc->config.snapshot[ 0 ];
    uint changedUserTags;
    Bool needReparseTagsDecls = NeedReparseTagDecls( value, snap,
                                                     &changedUserTags );
    
    for ( ixVal=0; ixVal < N_TIDY_OPTIONS; ++option, ++ixVal )
    {
        assert( ixVal == (uint) option->id );
        CopyOptionValue( doc, option, &value[ixVal], &snap[ixVal] );
    }
    if ( needReparseTagsDecls )
        ReparseTagDecls( doc, changedUserTags );
}

void TY_(CopyConfig)( TidyDocImpl* docTo, TidyDocImpl* docFrom )
{
    if ( docTo != docFrom )
    {
        uint ixVal;
        const TidyOptionImpl* option = option_defs;
        const TidyOptionValue* from = &docFrom->config.value[ 0 ];
        TidyOptionValue* to   = &docTo->config.value[ 0 ];
        uint changedUserTags;
        Bool needReparseTagsDecls = NeedReparseTagDecls( to, from,
                                                         &changedUserTags );

        TY_(TakeConfigSnapshot)( docTo );
        for ( ixVal=0; ixVal < N_TIDY_OPTIONS; ++option, ++ixVal )
        {
            assert( ixVal == (uint) option->id );
            CopyOptionValue( docTo, option, &to[ixVal], &from[ixVal] );
        }
        if ( needReparseTagsDecls )
            ReparseTagDecls( docTo, changedUserTags  );
        AdjustConfig( docTo );  /* Make sure it's consistent */
    }
}


#ifdef _DEBUG

/* Debug accessor functions will be type-safe and assert option type match */
ulong   TY_(_cfgGet)( TidyDocImpl* doc, TidyOptionId optId )
{
  assert( optId < N_TIDY_OPTIONS );
  return doc->config.value[ optId ].v;
}

Bool    TY_(_cfgGetBool)( TidyDocImpl* doc, TidyOptionId optId )
{
  ulong val = TY_(_cfgGet)( doc, optId );
  const TidyOptionImpl* opt = &option_defs[ optId ];
  assert( opt && opt->type == TidyBoolean );
  return (Bool) val;
}

TidyTriState    TY_(_cfgGetAutoBool)( TidyDocImpl* doc, TidyOptionId optId )
{
  ulong val = TY_(_cfgGet)( doc, optId );
  const TidyOptionImpl* opt = &option_defs[ optId ];
  assert( opt && opt->type == TidyInteger
          && opt->parser == ParseAutoBool );
  return (TidyTriState) val;
}

ctmbstr TY_(_cfgGetString)( TidyDocImpl* doc, TidyOptionId optId )
{
  const TidyOptionImpl* opt;

  assert( optId < N_TIDY_OPTIONS );
  opt = &option_defs[ optId ];
  assert( opt && opt->type == TidyString );
  return doc->config.value[ optId ].p;
}
#endif


#if 0
/* for use with Gnu Emacs */
void SetEmacsFilename( TidyDocImpl* doc, ctmbstr filename )
{
    SetOptionValue( doc, TidyEmacsFile, filename );
}
#endif

static tchar GetC( TidyConfigImpl* config )
{
    if ( config->cfgIn )
        return TY_(ReadChar)( config->cfgIn );
    return EndOfStream;
}

static tchar FirstChar( TidyConfigImpl* config )
{
    config->c = GetC( config );
    return config->c;
}

static tchar AdvanceChar( TidyConfigImpl* config )
{
    if ( config->c != EndOfStream )
        config->c = GetC( config );
    return config->c;
}

static tchar SkipWhite( TidyConfigImpl* config )
{
    while ( TY_(IsWhite)(config->c) && !TY_(IsNewline)(config->c) )
        config->c = GetC( config );
    return config->c;
}

/* skip until end of line
static tchar SkipToEndofLine( TidyConfigImpl* config )
{
    while ( config->c != EndOfStream )
    {
        config->c = GetC( config );
        if ( config->c == '\n' || config->c == '\r' )
            break;
    }
    return config->c;
}
*/

/*
 skip over line continuations
 to start of next property
*/
static uint NextProperty( TidyConfigImpl* config )
{
    do
    {
        /* skip to end of line */
        while ( config->c != '\n' &&  config->c != '\r' &&  config->c != EndOfStream )
             config->c = GetC( config );

        /* treat  \r\n   \r  or  \n as line ends */
        if ( config->c == '\r' )
             config->c = GetC( config );

        if ( config->c == '\n' )
            config->c = GetC( config );
    }
    while ( TY_(IsWhite)(config->c) );  /* line continuation? */

    return config->c;
}

/*
 Todd Lewis contributed this code for expanding
 ~/foo or ~your/foo according to $HOME and your
 user name. This will work partially on any system 
 which defines $HOME.  Support for ~user/foo will
 work on systems that support getpwnam(userid), 
 namely Unix/Linux.
*/
static ctmbstr ExpandTilde( TidyDocImpl* doc, ctmbstr filename )
{
    char *home_dir = NULL;

    if ( !filename )
        return NULL;

    if ( filename[0] != '~' )
        return filename;

    if (filename[1] == '/')
    {
        home_dir = getenv("HOME");
        if ( home_dir )
            ++filename;
    }
#ifdef SUPPORT_GETPWNAM
    else
    {
        struct passwd *passwd = NULL;
        ctmbstr s = filename + 1;
        tmbstr t;

        while ( *s && *s != '/' )
            s++;

        if ( t = TidyDocAlloc(doc, s - filename) )
        {
            memcpy(t, filename+1, s-filename-1);
            t[s-filename-1] = 0;

            passwd = getpwnam(t);

            TidyDocFree(doc, t);
        }

        if ( passwd )
        {
            filename = s;
            home_dir = passwd->pw_dir;
        }
    }
#endif /* SUPPORT_GETPWNAM */

    if ( home_dir )
    {
        uint len = TY_(tmbstrlen)(filename) + TY_(tmbstrlen)(home_dir) + 1;
        tmbstr p = (tmbstr)TidyDocAlloc( doc, len );
        TY_(tmbstrcpy)( p, home_dir );
        TY_(tmbstrcat)( p, filename );
        return (ctmbstr) p;
    }
    return (ctmbstr) filename;
}

Bool TIDY_CALL tidyFileExists( TidyDoc tdoc, ctmbstr filename )
{
  TidyDocImpl* doc = tidyDocToImpl( tdoc );
  ctmbstr fname = (tmbstr) ExpandTilde( doc, filename );
#ifndef NO_ACCESS_SUPPORT
  Bool exists = ( access(fname, 0) == 0 );
#else
  Bool exists;
  /* at present */
  FILE* fin = fopen(fname, "r");
  if (fin != NULL)
      fclose(fin);
  exists = ( fin != NULL );
#endif
  if ( fname != filename )
      TidyDocFree( doc, (tmbstr) fname );
  return exists;
}


#ifndef TIDY_MAX_NAME
#define TIDY_MAX_NAME 64
#endif

int TY_(ParseConfigFile)( TidyDocImpl* doc, ctmbstr file )
{
    return TY_(ParseConfigFileEnc)( doc, file, "ascii" );
}

/* open the file and parse its contents
*/
int TY_(ParseConfigFileEnc)( TidyDocImpl* doc, ctmbstr file, ctmbstr charenc )
{
    uint opterrs = doc->optionErrors;
    tmbstr fname = (tmbstr) ExpandTilde( doc, file );
    TidyConfigImpl* cfg = &doc->config;
    FILE* fin = fopen( fname, "r" );
    int enc = TY_(CharEncodingId)( doc, charenc );

    if ( fin == NULL || enc < 0 )
    {
        TY_(FileError)( doc, fname, TidyConfig );
        return -1;
    }
    else
    {
        tchar c;
        cfg->cfgIn = TY_(FileInput)( doc, fin, enc );
        c = FirstChar( cfg );
       
        for ( c = SkipWhite(cfg); c != EndOfStream; c = NextProperty(cfg) )
        {
            uint ix = 0;
            tmbchar name[ TIDY_MAX_NAME ] = {0};

            /* // or # start a comment */
            if ( c == '/' || c == '#' )
                continue;

            while ( ix < sizeof(name)-1 && c != '\n' && c != EndOfStream && c != ':' )
            {
                name[ ix++ ] = (tmbchar) c;  /* Option names all ASCII */
                c = AdvanceChar( cfg );
            }

            if ( c == ':' )
            {
                const TidyOptionImpl* option = TY_(lookupOption)( name );
                c = AdvanceChar( cfg );
                if ( option )
                    option->parser( doc, option );
                else
                {
                    if (NULL != doc->pOptCallback)
                    {
                        TidyConfigImpl* cfg = &doc->config;
                        tmbchar buf[8192];
                        uint i = 0;
                        tchar delim = 0;
                        Bool waswhite = yes;

                        tchar c = SkipWhite( cfg );

                        if ( c == '"' || c == '\'' )
                        {
                            delim = c;
                            c = AdvanceChar( cfg );
                        }

                        while ( i < sizeof(buf)-2 && c != EndOfStream && c != '\r' && c != '\n' )
                        {
                            if ( delim && c == delim )
                                break;

                            if ( TY_(IsWhite)(c) )
                            {
                                if ( waswhite )
                                {
                                    c = AdvanceChar( cfg );
                                    continue;
                                }
                                c = ' ';
                            }
                            else
                                waswhite = no;

                            buf[i++] = (tmbchar) c;
                            c = AdvanceChar( cfg );
                        }
                        buf[i] = '\0';
                        if (no == (*doc->pOptCallback)( name, buf ))
                            TY_(ReportUnknownOption)( doc, name );
                    }
                    else
                        TY_(ReportUnknownOption)( doc, name );
                }
            }
        }

        TY_(freeFileSource)(&cfg->cfgIn->source, yes);
        TY_(freeStreamIn)( cfg->cfgIn );
        cfg->cfgIn = NULL;
    }

    if ( fname != (tmbstr) file )
        TidyDocFree( doc, fname );

    AdjustConfig( doc );

    /* any new config errors? If so, return warning status. */
    return (doc->optionErrors > opterrs ? 1 : 0); 
}

/* returns false if unknown option, missing parameter,
** or option doesn't use parameter
*/
Bool TY_(ParseConfigOption)( TidyDocImpl* doc, ctmbstr optnam, ctmbstr optval )
{
    const TidyOptionImpl* option = TY_(lookupOption)( optnam );
    Bool status = ( option != NULL );
    if ( !status )
    {
        /* Not a standard tidy option.  Check to see if the user application 
           recognizes it  */
        if (NULL != doc->pOptCallback)
            status = (*doc->pOptCallback)( optnam, optval );
        if (!status)
            TY_(ReportUnknownOption)( doc, optnam );
    }
    else 
        status = TY_(ParseConfigValue)( doc, option->id, optval );
    return status;
}

/* returns false if unknown option, missing parameter,
** or option doesn't use parameter
*/
Bool TY_(ParseConfigValue)( TidyDocImpl* doc, TidyOptionId optId, ctmbstr optval )
{
    const TidyOptionImpl* option = option_defs + optId;
    Bool status = ( optId < N_TIDY_OPTIONS && optval != NULL );

    if ( !status )
        TY_(ReportBadArgument)( doc, option->name );
    else
    {
        TidyBuffer inbuf;            /* Set up input source */
        tidyBufInitWithAllocator( &inbuf, doc->allocator );
        tidyBufAttach( &inbuf, (byte*)optval, TY_(tmbstrlen)(optval)+1 );
        doc->config.cfgIn = TY_(BufferInput)( doc, &inbuf, ASCII );
        doc->config.c = GetC( &doc->config );

        status = option->parser( doc, option );

        TY_(freeStreamIn)(doc->config.cfgIn);  /* Release input source */
        doc->config.cfgIn  = NULL;
        tidyBufDetach( &inbuf );
    }
    return status;
}


/* ensure that char encodings are self consistent */
Bool  TY_(AdjustCharEncoding)( TidyDocImpl* doc, int encoding )
{
    int outenc = -1;
    int inenc = -1;
    
    switch( encoding )
    {
    case MACROMAN:
        inenc = MACROMAN;
        outenc = ASCII;
        break;

    case WIN1252:
        inenc = WIN1252;
        outenc = ASCII;
        break;

    case IBM858:
        inenc = IBM858;
        outenc = ASCII;
        break;

    case ASCII:
        inenc = LATIN1;
        outenc = ASCII;
        break;

    case LATIN0:
        inenc = LATIN0;
        outenc = ASCII;
        break;

    case RAW:
    case LATIN1:
    case UTF8:
#ifndef NO_NATIVE_ISO2022_SUPPORT
    case ISO2022:
#endif

#if SUPPORT_UTF16_ENCODINGS
    case UTF16LE:
    case UTF16BE:
    case UTF16:
#endif
#if SUPPORT_ASIAN_ENCODINGS
    case SHIFTJIS:
    case BIG5:
#endif
        inenc = outenc = encoding;
        break;
    }

    if ( inenc >= 0 )
    {
        TY_(SetOptionInt)( doc, TidyCharEncoding, encoding );
        TY_(SetOptionInt)( doc, TidyInCharEncoding, inenc );
        TY_(SetOptionInt)( doc, TidyOutCharEncoding, outenc );
        return yes;
    }
    return no;
}

/* ensure that config is self consistent */
void AdjustConfig( TidyDocImpl* doc )
{
    if ( cfgBool(doc, TidyEncloseBlockText) )
        TY_(SetOptionBool)( doc, TidyEncloseBodyText, yes );

    if ( cfgAutoBool(doc, TidyIndentContent) == TidyNoState )
        TY_(SetOptionInt)( doc, TidyIndentSpaces, 0 );

    /* disable wrapping */
    if ( cfg(doc, TidyWrapLen) == 0 )
        TY_(SetOptionInt)( doc, TidyWrapLen, 0x7FFFFFFF );

    /* Word 2000 needs o:p to be declared as inline */
    if ( cfgBool(doc, TidyWord2000) )
    {
        doc->config.defined_tags |= tagtype_inline;
        TY_(DefineTag)( doc, tagtype_inline, "o:p" );
    }

    /* #480701 disable XHTML output flag if both output-xhtml and xml input are set */
    if ( cfgBool(doc, TidyXmlTags) )
        TY_(SetOptionBool)( doc, TidyXhtmlOut, no );

    /* XHTML is written in lower case */
    if ( cfgBool(doc, TidyXhtmlOut) )
    {
        TY_(SetOptionBool)( doc, TidyXmlOut, yes );
        TY_(SetOptionBool)( doc, TidyUpperCaseTags, no );
        TY_(SetOptionBool)( doc, TidyUpperCaseAttrs, no );
        /* TY_(SetOptionBool)( doc, TidyXmlPIs, yes ); */
    }

    /* if XML in, then XML out */
    if ( cfgBool(doc, TidyXmlTags) )
    {
        TY_(SetOptionBool)( doc, TidyXmlOut, yes );
        TY_(SetOptionBool)( doc, TidyXmlPIs, yes );
    }

    /* #427837 - fix by Dave Raggett 02 Jun 01
    ** generate <?xml version="1.0" encoding="iso-8859-1"?>
    ** if the output character encoding is Latin-1 etc.
    */
    if ( cfg(doc, TidyOutCharEncoding) != ASCII &&
         cfg(doc, TidyOutCharEncoding) != UTF8 &&
#if SUPPORT_UTF16_ENCODINGS
         cfg(doc, TidyOutCharEncoding) != UTF16 &&
         cfg(doc, TidyOutCharEncoding) != UTF16BE &&
         cfg(doc, TidyOutCharEncoding) != UTF16LE &&
#endif
         cfg(doc, TidyOutCharEncoding) != RAW &&
         cfgBool(doc, TidyXmlOut) )
    {
        TY_(SetOptionBool)( doc, TidyXmlDecl, yes );
    }

    /* XML requires end tags */
    if ( cfgBool(doc, TidyXmlOut) )
    {
#if SUPPORT_UTF16_ENCODINGS
        /* XML requires a BOM on output if using UTF-16 encoding */
        ulong enc = cfg( doc, TidyOutCharEncoding );
        if ( enc == UTF16LE || enc == UTF16BE || enc == UTF16 )
            TY_(SetOptionInt)( doc, TidyOutputBOM, yes );
#endif
        TY_(SetOptionBool)( doc, TidyQuoteAmpersand, yes );
        TY_(SetOptionBool)( doc, TidyHideEndTags, no );
    }
}

/* unsigned integers */
Bool ParseInt( TidyDocImpl* doc, const TidyOptionImpl* entry )
{
    ulong number = 0;
    Bool digits = no;
    TidyConfigImpl* cfg = &doc->config;
    tchar c = SkipWhite( cfg );

    while ( TY_(IsDigit)(c) )
    {
        number = c - '0' + (10 * number);
        digits = yes;
        c = AdvanceChar( cfg );
    }

    if ( !digits )
        TY_(ReportBadArgument)( doc, entry->name );
    else
        TY_(SetOptionInt)( doc, entry->id, number );
    return digits;
}

/* true/false or yes/no or 0/1 or "auto" only looks at 1st char */
static Bool ParseTriState( TidyTriState theState, TidyDocImpl* doc,
                           const TidyOptionImpl* entry, ulong* flag )
{
    TidyConfigImpl* cfg = &doc->config;
    tchar c = SkipWhite( cfg );

    if (c == 't' || c == 'T' || c == 'y' || c == 'Y' || c == '1')
        *flag = yes;
    else if (c == 'f' || c == 'F' || c == 'n' || c == 'N' || c == '0')
        *flag = no;
    else if (theState == TidyAutoState && (c == 'a' || c =='A'))
        *flag = TidyAutoState;
    else
    {
        TY_(ReportBadArgument)( doc, entry->name );
        return no;
    }

    return yes;
}

/* cr, lf or crlf */
Bool ParseNewline( TidyDocImpl* doc, const TidyOptionImpl* entry )
{
    int nl = -1;
    tmbchar work[ 16 ] = {0};
    tmbstr cp = work, end = work + sizeof(work);
    TidyConfigImpl* cfg = &doc->config;
    tchar c = SkipWhite( cfg );

    while ( c!=EndOfStream && cp < end && !TY_(IsWhite)(c) && c != '\r' && c != '\n' )
    {
        *cp++ = (tmbchar) c;
        c = AdvanceChar( cfg );
    }
    *cp = 0;

    if ( TY_(tmbstrcasecmp)(work, "lf") == 0 )
        nl = TidyLF;
    else if ( TY_(tmbstrcasecmp)(work, "crlf") == 0 )
        nl = TidyCRLF;
    else if ( TY_(tmbstrcasecmp)(work, "cr") == 0 )
        nl = TidyCR;

    if ( nl < TidyLF || nl > TidyCR )
        TY_(ReportBadArgument)( doc, entry->name );
    else
        TY_(SetOptionInt)( doc, entry->id, nl );
    return ( nl >= TidyLF && nl <= TidyCR );
}

Bool ParseBool( TidyDocImpl* doc, const TidyOptionImpl* entry )
{
    ulong flag = 0;
    Bool status = ParseTriState( TidyNoState, doc, entry, &flag );
    if ( status )
        TY_(SetOptionBool)( doc, entry->id, flag != 0 );
    return status;
}

Bool ParseAutoBool( TidyDocImpl* doc, const TidyOptionImpl* entry )
{
    ulong flag = 0;
    Bool status = ParseTriState( TidyAutoState, doc, entry, &flag );
    if ( status )
        TY_(SetOptionInt)( doc, entry->id, flag );
    return status;
}

/* a string excluding whitespace */
Bool ParseName( TidyDocImpl* doc, const TidyOptionImpl* option )
{
    tmbchar buf[ 1024 ] = {0};
    uint i = 0;
    uint c = SkipWhite( &doc->config );

    while ( i < sizeof(buf)-2 && c != EndOfStream && !TY_(IsWhite)(c) )
    {
        buf[i++] = (tmbchar) c;
        c = AdvanceChar( &doc->config );
    }
    buf[i] = 0;

    if ( i == 0 )
        TY_(ReportBadArgument)( doc, option->name );
    else
        SetOptionValue( doc, option->id, buf );
    return ( i > 0 );
}

/* #508936 - CSS class naming for -clean option */
Bool ParseCSS1Selector( TidyDocImpl* doc, const TidyOptionImpl* option )
{
    char buf[256] = {0};
    uint i = 0;
    uint c = SkipWhite( &doc->config );

    while ( i < sizeof(buf)-2 && c != EndOfStream && !TY_(IsWhite)(c) )
    {
        buf[i++] = (tmbchar) c;
        c = AdvanceChar( &doc->config );
    }
    buf[i] = '\0';

    if ( i == 0 || !TY_(IsCSS1Selector)(buf) ) {
        TY_(ReportBadArgument)( doc, option->name );
        return no;
    }

    buf[i++] = '-';  /* Make sure any escaped Unicode is terminated */
    buf[i] = 0;      /* so valid class names are generated after */
                     /* Tidy appends last digits. */

    SetOptionValue( doc, option->id, buf );
    return yes;
}

/* Coordinates Config update and Tags data */
static void DeclareUserTag( TidyDocImpl* doc, TidyOptionId optId,
                            UserTagType tagType, ctmbstr name )
{
  ctmbstr prvval = cfgStr( doc, optId );
  tmbstr catval = NULL;
  ctmbstr theval = name;
  if ( prvval )
  {
    uint len = TY_(tmbstrlen)(name) + TY_(tmbstrlen)(prvval) + 3;
    catval = TY_(tmbstrndup)( doc->allocator, prvval, len );
    TY_(tmbstrcat)( catval, ", " );
    TY_(tmbstrcat)( catval, name );
    theval = catval;
  }
  TY_(DefineTag)( doc, tagType, name );
  SetOptionValue( doc, optId, theval );
  if ( catval )
    TidyDocFree( doc, catval );
}

/* a space or comma separated list of tag names */
Bool ParseTagNames( TidyDocImpl* doc, const TidyOptionImpl* option )
{
    TidyConfigImpl* cfg = &doc->config;
    tmbchar buf[1024];
    uint i = 0, nTags = 0;
    uint c = SkipWhite( cfg );
    UserTagType ttyp = tagtype_null;

    switch ( option->id )
    {
    case TidyInlineTags:  ttyp = tagtype_inline;    break;
    case TidyBlockTags:   ttyp = tagtype_block;     break;
    case TidyEmptyTags:   ttyp = tagtype_empty;     break;
    case TidyPreTags:     ttyp = tagtype_pre;       break;
    default:
       TY_(ReportUnknownOption)( doc, option->name );
       return no;
    }

    SetOptionValue( doc, option->id, NULL );
    TY_(FreeDeclaredTags)( doc, ttyp );
    cfg->defined_tags |= ttyp;

    do
    {
        if (c == ' ' || c == '\t' || c == ',')
        {
            c = AdvanceChar( cfg );
            continue;
        }

        if ( c == '\r' || c == '\n' )
        {
            uint c2 = AdvanceChar( cfg );
            if ( c == '\r' && c2 == '\n' )
                c = AdvanceChar( cfg );
            else
                c = c2;

            if ( !TY_(IsWhite)(c) )
            {
                buf[i] = 0;
                TY_(UngetChar)( c, cfg->cfgIn );
                TY_(UngetChar)( '\n', cfg->cfgIn );
                break;
            }
        }

        /*
        if ( c == '\n' )
        {
            c = AdvanceChar( cfg );
            if ( !TY_(IsWhite)(c) )
            {
                buf[i] = 0;
                TY_(UngetChar)( c, cfg->cfgIn );
                TY_(UngetChar)( '\n', cfg->cfgIn );
                break;
            }
        }
        */

        while ( i < sizeof(buf)-2 && c != EndOfStream && !TY_(IsWhite)(c) && c != ',' )
        {
            buf[i++] = (tmbchar) c;
            c = AdvanceChar( cfg );
        }

        buf[i] = '\0';
        if (i == 0)          /* Skip empty tag definition.  Possible when */
            continue;        /* there is a trailing space on the line. */
            
        /* add tag to dictionary */
        DeclareUserTag( doc, option->id, ttyp, buf );
        i = 0;
        ++nTags;
    }
    while ( c != EndOfStream );

    if ( i > 0 )
      DeclareUserTag( doc, option->id, ttyp, buf );
    return ( nTags > 0 );
}

/* a string including whitespace */
/* munges whitespace sequences */

Bool ParseString( TidyDocImpl* doc, const TidyOptionImpl* option )
{
    TidyConfigImpl* cfg = &doc->config;
    tmbchar buf[8192];
    uint i = 0;
    tchar delim = 0;
    Bool waswhite = yes;

    tchar c = SkipWhite( cfg );

    if ( c == '"' || c == '\'' )
    {
        delim = c;
        c = AdvanceChar( cfg );
    }

    while ( i < sizeof(buf)-2 && c != EndOfStream && c != '\r' && c != '\n' )
    {
        if ( delim && c == delim )
            break;

        if ( TY_(IsWhite)(c) )
        {
            if ( waswhite )
            {
                c = AdvanceChar( cfg );
                continue;
            }
            c = ' ';
        }
        else
            waswhite = no;

        buf[i++] = (tmbchar) c;
        c = AdvanceChar( cfg );
    }
    buf[i] = '\0';

    SetOptionValue( doc, option->id, buf );
    return yes;
}

Bool ParseCharEnc( TidyDocImpl* doc, const TidyOptionImpl* option )
{
    tmbchar buf[64] = {0};
    uint i = 0;
    int enc = ASCII;
    Bool validEncoding = yes;
    tchar c = SkipWhite( &doc->config );

    while ( i < sizeof(buf)-2 && c != EndOfStream && !TY_(IsWhite)(c) )
    {
        buf[i++] = (tmbchar) TY_(ToLower)( c );
        c = AdvanceChar( &doc->config );
    }
    buf[i] = 0;

    enc = TY_(CharEncodingId)( doc, buf );

#ifdef TIDY_WIN32_MLANG_SUPPORT
    /* limit support to --input-encoding */
    if (option->id != TidyInCharEncoding && enc > WIN32MLANG)
        enc = -1;
#endif

    if ( enc < 0 )
    {
        validEncoding = no;
        TY_(ReportBadArgument)( doc, option->name );
    }
    else
        TY_(SetOptionInt)( doc, option->id, enc );

    if ( validEncoding && option->id == TidyCharEncoding )
        TY_(AdjustCharEncoding)( doc, enc );
    return validEncoding;
}


int TY_(CharEncodingId)( TidyDocImpl* ARG_UNUSED(doc), ctmbstr charenc )
{
    int enc = TY_(GetCharEncodingFromOptName)( charenc );

#ifdef TIDY_WIN32_MLANG_SUPPORT
    if (enc == -1)
    {
        uint wincp = TY_(Win32MLangGetCPFromName)(doc->allocator, charenc);
        if (wincp)
            enc = wincp;
    }
#endif

    return enc;
}

ctmbstr TY_(CharEncodingName)( int encoding )
{
    ctmbstr encodingName = TY_(GetEncodingNameFromTidyId)(encoding);

    if (!encodingName)
        encodingName = "unknown";

    return encodingName;
}

ctmbstr TY_(CharEncodingOptName)( int encoding )
{
    ctmbstr encodingName = TY_(GetEncodingOptNameFromTidyId)(encoding);

    if (!encodingName)
        encodingName = "unknown";

    return encodingName;
}

/*
   doctype: omit | auto | strict | loose | <fpi>

   where the fpi is a string similar to

      "-//ACME//DTD HTML 3.14159//EN"
*/
Bool ParseDocType( TidyDocImpl* doc, const TidyOptionImpl* option )
{
    tmbchar buf[ 32 ] = {0};
    uint i = 0;
    Bool status = yes;
    TidyDoctypeModes dtmode = TidyDoctypeAuto;

    TidyConfigImpl* cfg = &doc->config;
    tchar c = SkipWhite( cfg );

    /* "-//ACME//DTD HTML 3.14159//EN" or similar */

    if ( c == '"' || c == '\'' )
    {
        status = ParseString(doc, option);
        if (status)
            TY_(SetOptionInt)( doc, TidyDoctypeMode, TidyDoctypeUser );

        return status;
    }

    /* read first word */
    while ( i < sizeof(buf)-1 && c != EndOfStream && !TY_(IsWhite)(c) )
    {
        buf[i++] = (tmbchar) c;
        c = AdvanceChar( cfg );
    }
    buf[i] = '\0';

    if ( TY_(tmbstrcasecmp)(buf, "auto") == 0 )
        dtmode = TidyDoctypeAuto;
    else if ( TY_(tmbstrcasecmp)(buf, "omit") == 0 )
        dtmode = TidyDoctypeOmit;
    else if ( TY_(tmbstrcasecmp)(buf, "strict") == 0 )
        dtmode = TidyDoctypeStrict;
    else if ( TY_(tmbstrcasecmp)(buf, "loose") == 0 ||
              TY_(tmbstrcasecmp)(buf, "transitional") == 0 )
        dtmode = TidyDoctypeLoose;
    else
    {
        TY_(ReportBadArgument)( doc, option->name );
        status = no;
    }
     
    if ( status )
        TY_(SetOptionInt)( doc, TidyDoctypeMode, dtmode );
    return status;
}

Bool ParseRepeatAttr( TidyDocImpl* doc, const TidyOptionImpl* option )
{
    Bool status = yes;
    tmbchar buf[64] = {0};
    uint i = 0;

    TidyConfigImpl* cfg = &doc->config;
    tchar c = SkipWhite( cfg );

    while (i < sizeof(buf)-1 && c != EndOfStream && !TY_(IsWhite)(c))
    {
        buf[i++] = (tmbchar) c;
        c = AdvanceChar( cfg );
    }
    buf[i] = '\0';

    if ( TY_(tmbstrcasecmp)(buf, "keep-first") == 0 )
        cfg->value[ TidyDuplicateAttrs ].v = TidyKeepFirst;
    else if ( TY_(tmbstrcasecmp)(buf, "keep-last") == 0 )
        cfg->value[ TidyDuplicateAttrs ].v = TidyKeepLast;
    else
    {
        TY_(ReportBadArgument)( doc, option->name );
        status = no;
    }
    return status;
}

Bool ParseSorter( TidyDocImpl* doc, const TidyOptionImpl* option )
{
    Bool status = yes;
    tmbchar buf[64] = {0};
    uint i = 0;

    TidyConfigImpl* cfg = &doc->config;
    tchar c = SkipWhite( cfg );

    while (i < sizeof(buf)-1 && c != EndOfStream && !TY_(IsWhite)(c))
    {
        buf[i++] = (tmbchar) c;
        c = AdvanceChar( cfg );
    }
    buf[i] = '\0';

    if ( TY_(tmbstrcasecmp)(buf, "alpha") == 0 )
        cfg->value[ TidySortAttributes ].v = TidySortAttrAlpha;
    else if ( TY_(tmbstrcasecmp)(buf, "none") == 0)
        cfg->value[ TidySortAttributes ].v = TidySortAttrNone;
    else
    {
        TY_(ReportBadArgument)( doc, option->name );
        status = no;
    }
    return status;
}

/* Use TidyOptionId as iterator.
** Send index of 1st option after TidyOptionUnknown as start of list.
*/
TidyIterator TY_(getOptionList)( TidyDocImpl* ARG_UNUSED(doc) )
{
    return (TidyIterator) (size_t)1;
}

/* Check if this item is last valid option.
** If so, zero out iterator.
*/
const TidyOptionImpl*  TY_(getNextOption)( TidyDocImpl* ARG_UNUSED(doc),
                                           TidyIterator* iter )
{
  const TidyOptionImpl* option = NULL;
  size_t optId;
  assert( iter != NULL );
  optId = (size_t) *iter;
  if ( optId > TidyUnknownOption && optId < N_TIDY_OPTIONS )
  {
    option = &option_defs[ optId ];
    optId++;
  }
  *iter = (TidyIterator) ( optId < N_TIDY_OPTIONS ? optId : (size_t)0 );
  return option;
}

/* Use a 1-based array index as iterator: 0 == end-of-list
*/
TidyIterator TY_(getOptionPickList)( const TidyOptionImpl* option )
{
    size_t ix = 0;
    if ( option && option->pickList )
        ix = 1;
    return (TidyIterator) ix;
}

ctmbstr      TY_(getNextOptionPick)( const TidyOptionImpl* option,
                                     TidyIterator* iter )
{
    size_t ix;
    ctmbstr val = NULL;
    assert( option!=NULL && iter != NULL );

    ix = (size_t) *iter;
    if ( ix > 0 && ix < 16 && option->pickList )
        val = option->pickList[ ix-1 ];
    *iter = (TidyIterator) ( val && option->pickList[ix] ? ix + 1 : (size_t)0 );
    return val;
}

static int  WriteOptionString( const TidyOptionImpl* option,
                               ctmbstr sval, StreamOut* out )
{
  ctmbstr cp = option->name;
  while ( *cp )
      TY_(WriteChar)( *cp++, out );
  TY_(WriteChar)( ':', out );
  TY_(WriteChar)( ' ', out );
  cp = sval;
  while ( *cp )
      TY_(WriteChar)( *cp++, out );
  TY_(WriteChar)( '\n', out );
  return 0;
}

static int  WriteOptionInt( const TidyOptionImpl* option, uint ival, StreamOut* out )
{
  tmbchar sval[ 32 ] = {0};
  TY_(tmbsnprintf)(sval, sizeof(sval), "%u", ival );
  return WriteOptionString( option, sval, out );
}

static int  WriteOptionBool( const TidyOptionImpl* option, Bool bval, StreamOut* out )
{
  ctmbstr sval = bval ? "yes" : "no";
  return WriteOptionString( option, sval, out );
}

static int  WriteOptionPick( const TidyOptionImpl* option, uint ival, StreamOut* out )
{
    uint ix;
    const ctmbstr* val = option->pickList;
    for ( ix=0; val[ix] && ix<ival; ++ix )
        /**/;
    if ( ix==ival && val[ix] )
        return WriteOptionString( option, val[ix], out );
    return -1;
}

Bool  TY_(ConfigDiffThanSnapshot)( TidyDocImpl* doc )
{
  int diff = memcmp( &doc->config.value, &doc->config.snapshot,
                     N_TIDY_OPTIONS * sizeof(uint) );
  return ( diff != 0 );
}

Bool  TY_(ConfigDiffThanDefault)( TidyDocImpl* doc )
{
  Bool diff = no;
  const TidyOptionImpl* option = option_defs + 1;
  const TidyOptionValue* val = doc->config.value;
  for ( /**/; !diff && option && option->name; ++option, ++val )
  {
      diff = !OptionValueEqDefault( option, val );
  }
  return diff;
}


static int  SaveConfigToStream( TidyDocImpl* doc, StreamOut* out )
{
    int rc = 0;
    const TidyOptionImpl* option;
    for ( option=option_defs+1; 0==rc && option && option->name; ++option )
    {
        const TidyOptionValue* val = &doc->config.value[ option->id ];
        if ( option->parser == NULL )
            continue;
        if ( OptionValueEqDefault( option, val ) && option->id != TidyDoctype)
            continue;

        if ( option->id == TidyDoctype )  /* Special case */
        {
          ulong dtmode = cfg( doc, TidyDoctypeMode );
          if ( dtmode == TidyDoctypeUser )
          {
            tmbstr t;
            
            /* add 2 double quotes */
            if (( t = (tmbstr)TidyDocAlloc( doc, TY_(tmbstrlen)( val->p ) + 2 ) ))
            {
              t[0] = '\"'; t[1] = 0;
            
              TY_(tmbstrcat)( t, val->p );
              TY_(tmbstrcat)( t, "\"" );
              rc = WriteOptionString( option, t, out );
            
              TidyDocFree( doc, t );
            }
          }
          else if ( dtmode == option_defs[TidyDoctypeMode].dflt )
            continue;
          else
            rc = WriteOptionPick( option, dtmode, out );
        }
        else if ( option->pickList )
          rc = WriteOptionPick( option, val->v, out );
        else
        {
          switch ( option->type )
          {
          case TidyString:
            rc = WriteOptionString( option, val->p, out );
            break;
          case TidyInteger:
            rc = WriteOptionInt( option, val->v, out );
            break;
          case TidyBoolean:
            rc = WriteOptionBool( option, val->v ? yes : no, out );
            break;
          }
        }
    }
    return rc;
}

int  TY_(SaveConfigFile)( TidyDocImpl* doc, ctmbstr cfgfil )
{
    int status = -1;
    StreamOut* out = NULL;
    uint outenc = cfg( doc, TidyOutCharEncoding );
    uint nl = cfg( doc, TidyNewline );
    FILE* fout = fopen( cfgfil, "wb" );
    if ( fout )
    {
        out = TY_(FileOutput)( doc, fout, outenc, nl );
        status = SaveConfigToStream( doc, out );
        fclose( fout );
        TidyDocFree( doc, out );
    }
    return status;
}

int  TY_(SaveConfigSink)( TidyDocImpl* doc, TidyOutputSink* sink )
{
    uint outenc = cfg( doc, TidyOutCharEncoding );
    uint nl = cfg( doc, TidyNewline );
    StreamOut* out = TY_(UserOutput)( doc, sink, outenc, nl );
    int status = SaveConfigToStream( doc, out );
    TidyDocFree( doc, out );
    return status;
}

/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
