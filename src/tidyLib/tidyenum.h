#ifndef __TIDYENUM_H__
#define __TIDYENUM_H__

/* @file tidyenum.h -- Split public enums into separate header

  Simplifies enum re-use in various wrappers.  e.g. SWIG
  generated wrappers and COM IDL files.

  Copyright (c) 1998-2008 World Wide Web Consortium
  (Massachusetts Institute of Technology, European Research 
  Consortium for Informatics and Mathematics, Keio University).
  All Rights Reserved.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2008/06/18 20:18:54 $ 
    $Revision: 1.18 $ 

  Contributing Author(s):

     Dave Raggett <dsr@w3.org>

  The contributing author(s) would like to thank all those who
  helped with testing, bug fixes and suggestions for improvements. 
  This wouldn't have been possible without your help.

  COPYRIGHT NOTICE:
 
  This software and documentation is provided "as is," and
  the copyright holders and contributing author(s) make no
  representations or warranties, express or implied, including
  but not limited to, warranties of merchantability or fitness
  for any particular purpose or that the use of the software or
  documentation will not infringe any third party patents,
  copyrights, trademarks or other rights. 

  The copyright holders and contributing author(s) will not be held
  liable for any direct, indirect, special or consequential damages
  arising out of any use of the software or documentation, even if
  advised of the possibility of such damage.

  Permission is hereby granted to use, copy, modify, and distribute
  this source code, or portions hereof, documentation and executables,
  for any purpose, without fee, subject to the following restrictions:

  1. The origin of this source code must not be misrepresented.
  2. Altered versions must be plainly marked as such and must
     not be misrepresented as being the original source.
  3. This Copyright notice may not be removed or altered from any
     source or altered source distribution.
 
  The copyright holders and contributing author(s) specifically
  permit, without fee, and encourage the use of this source code
  as a component for supporting the Hypertext Markup Language in
  commercial products. If you use this source code in a product,
  acknowledgment is not required but would be appreciated.


  Created 2001-05-20 by Charles Reitzel
  Updated 2002-07-01 by Charles Reitzel - 1st Implementation

*/

#ifdef __cplusplus
extern "C" {
#endif

/* Enumerate configuration options
*/

/** Categories of Tidy configuration options
*/
typedef enum
{
  TidyMarkup,          /**< Markup options: (X)HTML version, etc */
  TidyDiagnostics,     /**< Diagnostics */
  TidyPrettyPrint,     /**< Output layout */
  TidyEncoding,        /**< Character encodings */
  TidyMiscellaneous    /**< File handling, message format, etc. */
} TidyConfigCategory;


/** Option IDs Used to get/set option values.
*/
typedef enum
{
  TidyUnknownOption,   /**< Unknown option! */
  TidyIndentSpaces,    /**< Indentation n spaces */
  TidyWrapLen,         /**< Wrap margin */
  TidyTabSize,         /**< Expand tabs to n spaces */

  TidyCharEncoding,    /**< In/out character encoding */
  TidyInCharEncoding,  /**< Input character encoding (if different) */
  TidyOutCharEncoding, /**< Output character encoding (if different) */    
  TidyNewline,         /**< Output line ending (default to platform) */

  TidyDoctypeMode,     /**< See doctype property */
  TidyDoctype,         /**< User specified doctype */

  TidyDuplicateAttrs,  /**< Keep first or last duplicate attribute */
  TidyAltText,         /**< Default text for alt attribute */
  
  /* obsolete */
  TidySlideStyle,      /**< Style sheet for slides: not used for anything yet */

  TidyErrFile,         /**< File name to write errors to */
  TidyOutFile,         /**< File name to write markup to */
  TidyWriteBack,       /**< If true then output tidied markup */
  TidyShowMarkup,      /**< If false, normal output is suppressed */
  TidyShowWarnings,    /**< However errors are always shown */
  TidyQuiet,           /**< No 'Parsing X', guessed DTD or summary */
  TidyIndentContent,   /**< Indent content of appropriate tags */
                       /**< "auto" does text/block level content indentation */
  TidyHideEndTags,     /**< Suppress optional end tags */
  TidyXmlTags,         /**< Treat input as XML */
  TidyXmlOut,          /**< Create output as XML */
  TidyXhtmlOut,        /**< Output extensible HTML */
  TidyHtmlOut,         /**< Output plain HTML, even for XHTML input.
                           Yes means set explicitly. */
  TidyXmlDecl,         /**< Add <?xml?> for XML docs */
  TidyUpperCaseTags,   /**< Output tags in upper not lower case */
  TidyUpperCaseAttrs,  /**< Output attributes in upper not lower case */
  TidyMakeBare,        /**< Make bare HTML: remove Microsoft cruft */
  TidyMakeClean,       /**< Replace presentational clutter by style rules */
  TidyLogicalEmphasis, /**< Replace i by em and b by strong */
  TidyDropPropAttrs,   /**< Discard proprietary attributes */
  TidyDropFontTags,    /**< Discard presentation tags */
  TidyDropEmptyParas,  /**< Discard empty p elements */
  TidyFixComments,     /**< Fix comments with adjacent hyphens */
  TidyBreakBeforeBR,   /**< Output newline before <br> or not? */

  /* obsolete */
  TidyBurstSlides,     /**< Create slides on each h2 element */

  TidyNumEntities,     /**< Use numeric entities */
  TidyQuoteMarks,      /**< Output " marks as &quot; */
  TidyQuoteNbsp,       /**< Output non-breaking space as entity */
  TidyQuoteAmpersand,  /**< Output naked ampersand as &amp; */
  TidyWrapAttVals,     /**< Wrap within attribute values */
  TidyWrapScriptlets,  /**< Wrap within JavaScript string literals */
  TidyWrapSection,     /**< Wrap within <![ ... ]> section tags */
  TidyWrapAsp,         /**< Wrap within ASP pseudo elements */
  TidyWrapJste,        /**< Wrap within JSTE pseudo elements */
  TidyWrapPhp,         /**< Wrap within PHP pseudo elements */
  TidyFixBackslash,    /**< Fix URLs by replacing \ with / */
  TidyIndentAttributes,/**< Newline+indent before each attribute */
  TidyXmlPIs,          /**< If set to yes PIs must end with ?> */
  TidyXmlSpace,        /**< If set to yes adds xml:space attr as needed */
  TidyEncloseBodyText, /**< If yes text at body is wrapped in P's */
  TidyEncloseBlockText,/**< If yes text in blocks is wrapped in P's */
  TidyKeepFileTimes,   /**< If yes last modied time is preserved */
  TidyWord2000,        /**< Draconian cleaning for Word2000 */
  TidyMark,            /**< Add meta element indicating tidied doc */
  TidyEmacs,           /**< If true format error output for GNU Emacs */
  TidyEmacsFile,       /**< Name of current Emacs file */
  TidyLiteralAttribs,  /**< If true attributes may use newlines */
  TidyBodyOnly,        /**< Output BODY content only */
  TidyFixUri,          /**< Applies URI encoding if necessary */
  TidyLowerLiterals,   /**< Folds known attribute values to lower case */
  TidyHideComments,    /**< Hides all (real) comments in output */
  TidyIndentCdata,     /**< Indent <!CDATA[ ... ]]> section */
  TidyForceOutput,     /**< Output document even if errors were found */
  TidyShowErrors,      /**< Number of errors to put out */
  TidyAsciiChars,      /**< Convert quotes and dashes to nearest ASCII char */
  TidyJoinClasses,     /**< Join multiple class attributes */
  TidyJoinStyles,      /**< Join multiple style attributes */
  TidyEscapeCdata,     /**< Replace <![CDATA[]]> sections with escaped text */

#if SUPPORT_ASIAN_ENCODINGS
  TidyLanguage,        /**< Language property: not used for anything yet */
  TidyNCR,             /**< Allow numeric character references */
#else
  TidyLanguageNotUsed,
  TidyNCRNotUsed,
#endif
#if SUPPORT_UTF16_ENCODINGS
  TidyOutputBOM,      /**< Output a Byte Order Mark (BOM) for UTF-16 encodings */
                      /**< auto: if input stream has BOM, we output a BOM */
#else
  TidyOutputBOMNotUsed,
#endif

  TidyReplaceColor,    /**< Replace hex color attribute values with names */
  TidyCSSPrefix,       /**< CSS class naming for -clean option */

  TidyInlineTags,      /**< Declared inline tags */
  TidyBlockTags,       /**< Declared block tags */
  TidyEmptyTags,       /**< Declared empty tags */
  TidyPreTags,         /**< Declared pre tags */

  TidyAccessibilityCheckLevel, /**< Accessibility check level 
                                   0 (old style), or 1, 2, 3 */

  TidyVertSpace,       /**< degree to which markup is spread out vertically */
#if SUPPORT_ASIAN_ENCODINGS
  TidyPunctWrap,       /**< consider punctuation and breaking spaces for wrapping */
#else
  TidyPunctWrapNotUsed,
#endif
  TidyMergeDivs,       /**< Merge multiple DIVs */
  TidyDecorateInferredUL,  /**< Mark inferred UL elements with no indent CSS */
  TidyPreserveEntities,    /**< Preserve entities */
  TidySortAttributes,      /**< Sort attributes */
  TidyMergeSpans,       /**< Merge multiple SPANs */
  TidyAnchorAsName,    /**< Define anchors as name attributes */
  TidyClassStartID,    /**< Added by Strahinja Markovic: initial value for Tidy CSS class suffix */
  N_TIDY_OPTIONS       /**< Must be last */
} TidyOptionId;

/** Option data types
*/
typedef enum
{
  TidyString,          /**< String */
  TidyInteger,         /**< Integer or enumeration */
  TidyBoolean          /**< Boolean flag */
} TidyOptionType;


/** AutoBool values used by ParseBool, ParseTriState, ParseIndent, ParseBOM
*/
typedef enum
{
   TidyNoState,     /**< maps to 'no' */
   TidyYesState,    /**< maps to 'yes' */
   TidyAutoState    /**< Automatic */
} TidyTriState;

/** TidyNewline option values to control output line endings.
*/
typedef enum
{
    TidyLF,         /**< Use Unix style: LF */
    TidyCRLF,       /**< Use DOS/Windows style: CR+LF */
    TidyCR          /**< Use Macintosh style: CR */
} TidyLineEnding;


/** Mode controlling treatment of doctype
*/
typedef enum
{
    TidyDoctypeOmit,    /**< Omit DOCTYPE altogether */
    TidyDoctypeAuto,    /**< Keep DOCTYPE in input.  Set version to content */
    TidyDoctypeStrict,  /**< Convert document to HTML 4 strict content model */
    TidyDoctypeLoose,   /**< Convert document to HTML 4 transitional
                             content model */
    TidyDoctypeUser     /**< Set DOCTYPE FPI explicitly */
} TidyDoctypeModes;

/** Mode controlling treatment of duplicate Attributes
*/
typedef enum
{
    TidyKeepFirst,
    TidyKeepLast
} TidyDupAttrModes;

/** Mode controlling treatment of sorting attributes
*/
typedef enum
{
    TidySortAttrNone,
    TidySortAttrAlpha
} TidyAttrSortStrategy;

/* I/O and Message handling interface
**
** By default, Tidy will define, create and use 
** instances of input and output handlers for 
** standard C buffered I/O (i.e. FILE* stdin,
** FILE* stdout and FILE* stderr for content
** input, content output and diagnostic output,
** respectively.  A FILE* cfgFile input handler
** will be used for config files.  Command line
** options will just be set directly.
*/

/** Message severity level
*/
typedef enum 
{
  TidyInfo,             /**< Information about markup usage */
  TidyWarning,          /**< Warning message */
  TidyConfig,           /**< Configuration error */
  TidyAccess,           /**< Accessibility message */
  TidyError,            /**< Error message - output suppressed */
  TidyBadDocument,      /**< I/O or file system error */
  TidyFatal             /**< Crash! */
} TidyReportLevel;


/* Document tree traversal functions
*/

/** Node types
*/
typedef enum 
{
  TidyNode_Root,        /**< Root */
  TidyNode_DocType,     /**< DOCTYPE */
  TidyNode_Comment,     /**< Comment */
  TidyNode_ProcIns,     /**< Processing Instruction */
  TidyNode_Text,        /**< Text */
  TidyNode_Start,       /**< Start Tag */
  TidyNode_End,         /**< End Tag */
  TidyNode_StartEnd,    /**< Start/End (empty) Tag */
  TidyNode_CDATA,       /**< Unparsed Text */
  TidyNode_Section,     /**< XML Section */
  TidyNode_Asp,         /**< ASP Source */
  TidyNode_Jste,        /**< JSTE Source */
  TidyNode_Php,         /**< PHP Source */
  TidyNode_XmlDecl      /**< XML Declaration */
} TidyNodeType;


/** Known HTML element types
*/
typedef enum
{
  TidyTag_UNKNOWN,  /**< Unknown tag! */
  TidyTag_A,        /**< A */
  TidyTag_ABBR,     /**< ABBR */
  TidyTag_ACRONYM,  /**< ACRONYM */
  TidyTag_ADDRESS,  /**< ADDRESS */
  TidyTag_ALIGN,    /**< ALIGN */
  TidyTag_APPLET,   /**< APPLET */
  TidyTag_AREA,     /**< AREA */
  TidyTag_B,        /**< B */
  TidyTag_BASE,     /**< BASE */
  TidyTag_BASEFONT, /**< BASEFONT */
  TidyTag_BDO,      /**< BDO */
  TidyTag_BGSOUND,  /**< BGSOUND */
  TidyTag_BIG,      /**< BIG */
  TidyTag_BLINK,    /**< BLINK */
  TidyTag_BLOCKQUOTE,   /**< BLOCKQUOTE */
  TidyTag_BODY,     /**< BODY */
  TidyTag_BR,       /**< BR */
  TidyTag_BUTTON,   /**< BUTTON */
  TidyTag_CAPTION,  /**< CAPTION */
  TidyTag_CENTER,   /**< CENTER */
  TidyTag_CITE,     /**< CITE */
  TidyTag_CODE,     /**< CODE */
  TidyTag_COL,      /**< COL */
  TidyTag_COLGROUP, /**< COLGROUP */
  TidyTag_COMMENT,  /**< COMMENT */
  TidyTag_DD,       /**< DD */
  TidyTag_DEL,      /**< DEL */
  TidyTag_DFN,      /**< DFN */
  TidyTag_DIR,      /**< DIR */
  TidyTag_DIV,      /**< DIF */
  TidyTag_DL,       /**< DL */
  TidyTag_DT,       /**< DT */
  TidyTag_EM,       /**< EM */
  TidyTag_EMBED,    /**< EMBED */
  TidyTag_FIELDSET, /**< FIELDSET */
  TidyTag_FONT,     /**< FONT */
  TidyTag_FORM,     /**< FORM */
  TidyTag_FRAME,    /**< FRAME */
  TidyTag_FRAMESET, /**< FRAMESET */
  TidyTag_H1,       /**< H1 */
  TidyTag_H2,       /**< H2 */
  TidyTag_H3,       /**< H3 */
  TidyTag_H4,       /**< H4 */
  TidyTag_H5,       /**< H5 */
  TidyTag_H6,       /**< H6 */
  TidyTag_HEAD,     /**< HEAD */
  TidyTag_HR,       /**< HR */
  TidyTag_HTML,     /**< HTML */
  TidyTag_I,        /**< I */
  TidyTag_IFRAME,   /**< IFRAME */
  TidyTag_ILAYER,   /**< ILAYER */
  TidyTag_IMG,      /**< IMG */
  TidyTag_INPUT,    /**< INPUT */
  TidyTag_INS,      /**< INS */
  TidyTag_ISINDEX,  /**< ISINDEX */
  TidyTag_KBD,      /**< KBD */
  TidyTag_KEYGEN,   /**< KEYGEN */
  TidyTag_LABEL,    /**< LABEL */
  TidyTag_LAYER,    /**< LAYER */
  TidyTag_LEGEND,   /**< LEGEND */
  TidyTag_LI,       /**< LI */
  TidyTag_LINK,     /**< LINK */
  TidyTag_LISTING,  /**< LISTING */
  TidyTag_MAP,      /**< MAP */
  TidyTag_MARQUEE,  /**< MARQUEE */
  TidyTag_MENU,     /**< MENU */
  TidyTag_META,     /**< META */
  TidyTag_MULTICOL, /**< MULTICOL */
  TidyTag_NOBR,     /**< NOBR */
  TidyTag_NOEMBED,  /**< NOEMBED */
  TidyTag_NOFRAMES, /**< NOFRAMES */
  TidyTag_NOLAYER,  /**< NOLAYER */
  TidyTag_NOSAVE,   /**< NOSAVE */
  TidyTag_NOSCRIPT, /**< NOSCRIPT */
  TidyTag_OBJECT,   /**< OBJECT */
  TidyTag_OL,       /**< OL */
  TidyTag_OPTGROUP, /**< OPTGROUP */
  TidyTag_OPTION,   /**< OPTION */
  TidyTag_P,        /**< P */
  TidyTag_PARAM,    /**< PARAM */
  TidyTag_PLAINTEXT,/**< PLAINTEXT */
  TidyTag_PRE,      /**< PRE */
  TidyTag_Q,        /**< Q */
  TidyTag_RB,       /**< RB */
  TidyTag_RBC,      /**< RBC */
  TidyTag_RP,       /**< RP */
  TidyTag_RT,       /**< RT */
  TidyTag_RTC,      /**< RTC */
  TidyTag_RUBY,     /**< RUBY */
  TidyTag_S,        /**< S */
  TidyTag_SAMP,     /**< SAMP */
  TidyTag_SCRIPT,   /**< SCRIPT */
  TidyTag_SELECT,   /**< SELECT */
  TidyTag_SERVER,   /**< SERVER */
  TidyTag_SERVLET,  /**< SERVLET */
  TidyTag_SMALL,    /**< SMALL */
  TidyTag_SPACER,   /**< SPACER */
  TidyTag_SPAN,     /**< SPAN */
  TidyTag_STRIKE,   /**< STRIKE */
  TidyTag_STRONG,   /**< STRONG */
  TidyTag_STYLE,    /**< STYLE */
  TidyTag_SUB,      /**< SUB */
  TidyTag_SUP,      /**< SUP */
  TidyTag_TABLE,    /**< TABLE */
  TidyTag_TBODY,    /**< TBODY */
  TidyTag_TD,       /**< TD */
  TidyTag_TEXTAREA, /**< TEXTAREA */
  TidyTag_TFOOT,    /**< TFOOT */
  TidyTag_TH,       /**< TH */
  TidyTag_THEAD,    /**< THEAD */
  TidyTag_TITLE,    /**< TITLE */
  TidyTag_TR,       /**< TR */
  TidyTag_TT,       /**< TT */
  TidyTag_U,        /**< U */
  TidyTag_UL,       /**< UL */
  TidyTag_VAR,      /**< VAR */
  TidyTag_WBR,      /**< WBR */
  TidyTag_XMP,      /**< XMP */
  TidyTag_NEXTID,   /**< NEXTID */

  N_TIDY_TAGS       /**< Must be last */
} TidyTagId;

/* Attribute interrogation
*/

/** Known HTML attributes
*/
typedef enum
{
  TidyAttr_UNKNOWN,           /**< UNKNOWN= */
  TidyAttr_ABBR,              /**< ABBR= */
  TidyAttr_ACCEPT,            /**< ACCEPT= */
  TidyAttr_ACCEPT_CHARSET,    /**< ACCEPT_CHARSET= */
  TidyAttr_ACCESSKEY,         /**< ACCESSKEY= */
  TidyAttr_ACTION,            /**< ACTION= */
  TidyAttr_ADD_DATE,          /**< ADD_DATE= */
  TidyAttr_ALIGN,             /**< ALIGN= */
  TidyAttr_ALINK,             /**< ALINK= */
  TidyAttr_ALT,               /**< ALT= */
  TidyAttr_ARCHIVE,           /**< ARCHIVE= */
  TidyAttr_AXIS,              /**< AXIS= */
  TidyAttr_BACKGROUND,        /**< BACKGROUND= */
  TidyAttr_BGCOLOR,           /**< BGCOLOR= */
  TidyAttr_BGPROPERTIES,      /**< BGPROPERTIES= */
  TidyAttr_BORDER,            /**< BORDER= */
  TidyAttr_BORDERCOLOR,       /**< BORDERCOLOR= */
  TidyAttr_BOTTOMMARGIN,      /**< BOTTOMMARGIN= */
  TidyAttr_CELLPADDING,       /**< CELLPADDING= */
  TidyAttr_CELLSPACING,       /**< CELLSPACING= */
  TidyAttr_CHAR,              /**< CHAR= */
  TidyAttr_CHAROFF,           /**< CHAROFF= */
  TidyAttr_CHARSET,           /**< CHARSET= */
  TidyAttr_CHECKED,           /**< CHECKED= */
  TidyAttr_CITE,              /**< CITE= */
  TidyAttr_CLASS,             /**< CLASS= */
  TidyAttr_CLASSID,           /**< CLASSID= */
  TidyAttr_CLEAR,             /**< CLEAR= */
  TidyAttr_CODE,              /**< CODE= */
  TidyAttr_CODEBASE,          /**< CODEBASE= */
  TidyAttr_CODETYPE,          /**< CODETYPE= */
  TidyAttr_COLOR,             /**< COLOR= */
  TidyAttr_COLS,              /**< COLS= */
  TidyAttr_COLSPAN,           /**< COLSPAN= */
  TidyAttr_COMPACT,           /**< COMPACT= */
  TidyAttr_CONTENT,           /**< CONTENT= */
  TidyAttr_COORDS,            /**< COORDS= */
  TidyAttr_DATA,              /**< DATA= */
  TidyAttr_DATAFLD,           /**< DATAFLD= */
  TidyAttr_DATAFORMATAS,      /**< DATAFORMATAS= */
  TidyAttr_DATAPAGESIZE,      /**< DATAPAGESIZE= */
  TidyAttr_DATASRC,           /**< DATASRC= */
  TidyAttr_DATETIME,          /**< DATETIME= */
  TidyAttr_DECLARE,           /**< DECLARE= */
  TidyAttr_DEFER,             /**< DEFER= */
  TidyAttr_DIR,               /**< DIR= */
  TidyAttr_DISABLED,          /**< DISABLED= */
  TidyAttr_ENCODING,          /**< ENCODING= */
  TidyAttr_ENCTYPE,           /**< ENCTYPE= */
  TidyAttr_FACE,              /**< FACE= */
  TidyAttr_FOR,               /**< FOR= */
  TidyAttr_FRAME,             /**< FRAME= */
  TidyAttr_FRAMEBORDER,       /**< FRAMEBORDER= */
  TidyAttr_FRAMESPACING,      /**< FRAMESPACING= */
  TidyAttr_GRIDX,             /**< GRIDX= */
  TidyAttr_GRIDY,             /**< GRIDY= */
  TidyAttr_HEADERS,           /**< HEADERS= */
  TidyAttr_HEIGHT,            /**< HEIGHT= */
  TidyAttr_HREF,              /**< HREF= */
  TidyAttr_HREFLANG,          /**< HREFLANG= */
  TidyAttr_HSPACE,            /**< HSPACE= */
  TidyAttr_HTTP_EQUIV,        /**< HTTP_EQUIV= */
  TidyAttr_ID,                /**< ID= */
  TidyAttr_ISMAP,             /**< ISMAP= */
  TidyAttr_LABEL,             /**< LABEL= */
  TidyAttr_LANG,              /**< LANG= */
  TidyAttr_LANGUAGE,          /**< LANGUAGE= */
  TidyAttr_LAST_MODIFIED,     /**< LAST_MODIFIED= */
  TidyAttr_LAST_VISIT,        /**< LAST_VISIT= */
  TidyAttr_LEFTMARGIN,        /**< LEFTMARGIN= */
  TidyAttr_LINK,              /**< LINK= */
  TidyAttr_LONGDESC,          /**< LONGDESC= */
  TidyAttr_LOWSRC,            /**< LOWSRC= */
  TidyAttr_MARGINHEIGHT,      /**< MARGINHEIGHT= */
  TidyAttr_MARGINWIDTH,       /**< MARGINWIDTH= */
  TidyAttr_MAXLENGTH,         /**< MAXLENGTH= */
  TidyAttr_MEDIA,             /**< MEDIA= */
  TidyAttr_METHOD,            /**< METHOD= */
  TidyAttr_MULTIPLE,          /**< MULTIPLE= */
  TidyAttr_NAME,              /**< NAME= */
  TidyAttr_NOHREF,            /**< NOHREF= */
  TidyAttr_NORESIZE,          /**< NORESIZE= */
  TidyAttr_NOSHADE,           /**< NOSHADE= */
  TidyAttr_NOWRAP,            /**< NOWRAP= */
  TidyAttr_OBJECT,            /**< OBJECT= */
  TidyAttr_OnAFTERUPDATE,     /**< OnAFTERUPDATE= */
  TidyAttr_OnBEFOREUNLOAD,    /**< OnBEFOREUNLOAD= */
  TidyAttr_OnBEFOREUPDATE,    /**< OnBEFOREUPDATE= */
  TidyAttr_OnBLUR,            /**< OnBLUR= */
  TidyAttr_OnCHANGE,          /**< OnCHANGE= */
  TidyAttr_OnCLICK,           /**< OnCLICK= */
  TidyAttr_OnDATAAVAILABLE,   /**< OnDATAAVAILABLE= */
  TidyAttr_OnDATASETCHANGED,  /**< OnDATASETCHANGED= */
  TidyAttr_OnDATASETCOMPLETE, /**< OnDATASETCOMPLETE= */
  TidyAttr_OnDBLCLICK,        /**< OnDBLCLICK= */
  TidyAttr_OnERRORUPDATE,     /**< OnERRORUPDATE= */
  TidyAttr_OnFOCUS,           /**< OnFOCUS= */
  TidyAttr_OnKEYDOWN,         /**< OnKEYDOWN= */
  TidyAttr_OnKEYPRESS,        /**< OnKEYPRESS= */
  TidyAttr_OnKEYUP,           /**< OnKEYUP= */
  TidyAttr_OnLOAD,            /**< OnLOAD= */
  TidyAttr_OnMOUSEDOWN,       /**< OnMOUSEDOWN= */
  TidyAttr_OnMOUSEMOVE,       /**< OnMOUSEMOVE= */
  TidyAttr_OnMOUSEOUT,        /**< OnMOUSEOUT= */
  TidyAttr_OnMOUSEOVER,       /**< OnMOUSEOVER= */
  TidyAttr_OnMOUSEUP,         /**< OnMOUSEUP= */
  TidyAttr_OnRESET,           /**< OnRESET= */
  TidyAttr_OnROWENTER,        /**< OnROWENTER= */
  TidyAttr_OnROWEXIT,         /**< OnROWEXIT= */
  TidyAttr_OnSELECT,          /**< OnSELECT= */
  TidyAttr_OnSUBMIT,          /**< OnSUBMIT= */
  TidyAttr_OnUNLOAD,          /**< OnUNLOAD= */
  TidyAttr_PROFILE,           /**< PROFILE= */
  TidyAttr_PROMPT,            /**< PROMPT= */
  TidyAttr_RBSPAN,            /**< RBSPAN= */
  TidyAttr_READONLY,          /**< READONLY= */
  TidyAttr_REL,               /**< REL= */
  TidyAttr_REV,               /**< REV= */
  TidyAttr_RIGHTMARGIN,       /**< RIGHTMARGIN= */
  TidyAttr_ROWS,              /**< ROWS= */
  TidyAttr_ROWSPAN,           /**< ROWSPAN= */
  TidyAttr_RULES,             /**< RULES= */
  TidyAttr_SCHEME,            /**< SCHEME= */
  TidyAttr_SCOPE,             /**< SCOPE= */
  TidyAttr_SCROLLING,         /**< SCROLLING= */
  TidyAttr_SELECTED,          /**< SELECTED= */
  TidyAttr_SHAPE,             /**< SHAPE= */
  TidyAttr_SHOWGRID,          /**< SHOWGRID= */
  TidyAttr_SHOWGRIDX,         /**< SHOWGRIDX= */
  TidyAttr_SHOWGRIDY,         /**< SHOWGRIDY= */
  TidyAttr_SIZE,              /**< SIZE= */
  TidyAttr_SPAN,              /**< SPAN= */
  TidyAttr_SRC,               /**< SRC= */
  TidyAttr_STANDBY,           /**< STANDBY= */
  TidyAttr_START,             /**< START= */
  TidyAttr_STYLE,             /**< STYLE= */
  TidyAttr_SUMMARY,           /**< SUMMARY= */
  TidyAttr_TABINDEX,          /**< TABINDEX= */
  TidyAttr_TARGET,            /**< TARGET= */
  TidyAttr_TEXT,              /**< TEXT= */
  TidyAttr_TITLE,             /**< TITLE= */
  TidyAttr_TOPMARGIN,         /**< TOPMARGIN= */
  TidyAttr_TYPE,              /**< TYPE= */
  TidyAttr_USEMAP,            /**< USEMAP= */
  TidyAttr_VALIGN,            /**< VALIGN= */
  TidyAttr_VALUE,             /**< VALUE= */
  TidyAttr_VALUETYPE,         /**< VALUETYPE= */
  TidyAttr_VERSION,           /**< VERSION= */
  TidyAttr_VLINK,             /**< VLINK= */
  TidyAttr_VSPACE,            /**< VSPACE= */
  TidyAttr_WIDTH,             /**< WIDTH= */
  TidyAttr_WRAP,              /**< WRAP= */
  TidyAttr_XML_LANG,          /**< XML_LANG= */
  TidyAttr_XML_SPACE,         /**< XML_SPACE= */
  TidyAttr_XMLNS,             /**< XMLNS= */

  TidyAttr_EVENT,             /**< EVENT= */
  TidyAttr_METHODS,           /**< METHODS= */
  TidyAttr_N,                 /**< N= */
  TidyAttr_SDAFORM,           /**< SDAFORM= */
  TidyAttr_SDAPREF,           /**< SDAPREF= */
  TidyAttr_SDASUFF,           /**< SDASUFF= */
  TidyAttr_URN,               /**< URN= */

  N_TIDY_ATTRIBS              /**< Must be last */
} TidyAttrId;

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif /* __TIDYENUM_H__ */
