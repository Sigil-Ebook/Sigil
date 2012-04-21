#ifndef __MESSAGE_H__
#define __MESSAGE_H__

/* message.h -- general message writing routines

  (c) 1998-2007 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.
  
  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2007/05/30 16:47:31 $ 
    $Revision: 1.29 $ 

*/

#include "forward.h"
#include "tidy.h"  /* For TidyReportLevel */

/* General message writing routines.
** Each message is a single warning, error, etc.
**
** This routine will keep track of counts and,
** if the caller has set a filter, it will be
** called.  The new preferred way of handling
** Tidy diagnostics output is either a) define
** a new output sink or b) install a message
** filter routine.
**
** Keeps track of ShowWarnings, ShowErrors, etc.
*/

ctmbstr TY_(ReleaseDate)(void);

/* void TY_(ShowVersion)( TidyDocImpl* doc ); */
void TY_(ReportUnknownOption)( TidyDocImpl* doc, ctmbstr option );
void TY_(ReportBadArgument)( TidyDocImpl* doc, ctmbstr option );
void TY_(NeedsAuthorIntervention)( TidyDocImpl* doc );

/* void TY_(HelloMessage)( TidyDocImpl* doc, ctmbstr date, ctmbstr filename ); */
void TY_(ReportMarkupVersion)( TidyDocImpl* doc );
void TY_(ReportNumWarnings)( TidyDocImpl* doc );

void TY_(GeneralInfo)( TidyDocImpl* doc );
/* void TY_(UnknownOption)( TidyDocImpl* doc, char c ); */
/* void TY_(UnknownFile)( TidyDocImpl* doc, ctmbstr program, ctmbstr file ); */
void TY_(FileError)( TidyDocImpl* doc, ctmbstr file, TidyReportLevel level );

void TY_(ErrorSummary)( TidyDocImpl* doc );

void TY_(ReportEncodingWarning)(TidyDocImpl* doc, uint code, uint encoding);
void TY_(ReportEncodingError)(TidyDocImpl* doc, uint code, uint c, Bool discarded);
void TY_(ReportEntityError)( TidyDocImpl* doc, uint code, ctmbstr entity, int c );
void TY_(ReportAttrError)( TidyDocImpl* doc, Node* node, AttVal* av, uint code );
void TY_(ReportMissingAttr)( TidyDocImpl* doc, Node* node, ctmbstr name );

#if SUPPORT_ACCESSIBILITY_CHECKS

void TY_(ReportAccessWarning)( TidyDocImpl* doc, Node* node, uint code );
void TY_(ReportAccessError)( TidyDocImpl* doc, Node* node, uint code );

#endif

void TY_(ReportNotice)(TidyDocImpl* doc, Node *element, Node *node, uint code);
void TY_(ReportWarning)(TidyDocImpl* doc, Node *element, Node *node, uint code);
void TY_(ReportError)(TidyDocImpl* doc, Node* element, Node* node, uint code);
void TY_(ReportFatal)(TidyDocImpl* doc, Node* element, Node* node, uint code);

/* error codes for entities/numeric character references */

#define MISSING_SEMICOLON            1
#define MISSING_SEMICOLON_NCR        2
#define UNKNOWN_ENTITY               3
#define UNESCAPED_AMPERSAND          4
#define APOS_UNDEFINED               5

/* error codes for element messages */

#define MISSING_ENDTAG_FOR           6
#define MISSING_ENDTAG_BEFORE        7
#define DISCARDING_UNEXPECTED        8
#define NESTED_EMPHASIS              9
#define NON_MATCHING_ENDTAG          10
#define TAG_NOT_ALLOWED_IN           11
#define MISSING_STARTTAG             12
#define UNEXPECTED_ENDTAG            13
#define USING_BR_INPLACE_OF          14
#define INSERTING_TAG                15
#define SUSPECTED_MISSING_QUOTE      16
#define MISSING_TITLE_ELEMENT        17
#define DUPLICATE_FRAMESET           18
#define CANT_BE_NESTED               19
#define OBSOLETE_ELEMENT             20
#define PROPRIETARY_ELEMENT          21
#define UNKNOWN_ELEMENT              22
#define TRIM_EMPTY_ELEMENT           23
#define COERCE_TO_ENDTAG             24
#define ILLEGAL_NESTING              25
#define NOFRAMES_CONTENT             26
#define CONTENT_AFTER_BODY           27
#define INCONSISTENT_VERSION         28
#define MALFORMED_COMMENT            29
#define BAD_COMMENT_CHARS            30
#define BAD_XML_COMMENT              31
#define BAD_CDATA_CONTENT            32
#define INCONSISTENT_NAMESPACE       33
#define DOCTYPE_AFTER_TAGS           34
#define MALFORMED_DOCTYPE            35
#define UNEXPECTED_END_OF_FILE       36
#define DTYPE_NOT_UPPER_CASE         37
#define TOO_MANY_ELEMENTS            38
#define UNESCAPED_ELEMENT            39
#define NESTED_QUOTATION             40
#define ELEMENT_NOT_EMPTY            41
#define ENCODING_IO_CONFLICT         42
#define MIXED_CONTENT_IN_BLOCK       43
#define MISSING_DOCTYPE              44
#define SPACE_PRECEDING_XMLDECL      45
#define TOO_MANY_ELEMENTS_IN         46
#define UNEXPECTED_ENDTAG_IN         47
#define REPLACING_ELEMENT            83
#define REPLACING_UNEX_ELEMENT       84
#define COERCE_TO_ENDTAG_WARN        85

/* error codes used for attribute messages */

#define UNKNOWN_ATTRIBUTE            48
#define INSERTING_ATTRIBUTE          49
#define MISSING_ATTR_VALUE           50
#define BAD_ATTRIBUTE_VALUE          51
#define UNEXPECTED_GT                52
#define PROPRIETARY_ATTRIBUTE        53
#define PROPRIETARY_ATTR_VALUE       54
#define REPEATED_ATTRIBUTE           55
#define MISSING_IMAGEMAP             56
#define XML_ATTRIBUTE_VALUE          57
#define UNEXPECTED_QUOTEMARK         58
#define MISSING_QUOTEMARK            59
#define ID_NAME_MISMATCH             60

#define BACKSLASH_IN_URI             61
#define FIXED_BACKSLASH              62
#define ILLEGAL_URI_REFERENCE        63
#define ESCAPED_ILLEGAL_URI          64

#define NEWLINE_IN_URI               65
#define ANCHOR_NOT_UNIQUE            66

#define JOINING_ATTRIBUTE            68
#define UNEXPECTED_EQUALSIGN         69
#define ATTR_VALUE_NOT_LCASE         70
#define XML_ID_SYNTAX                71

#define INVALID_ATTRIBUTE            72

#define BAD_ATTRIBUTE_VALUE_REPLACED 73

#define INVALID_XML_ID               74
#define UNEXPECTED_END_OF_FILE_ATTR  75
#define MISSING_ATTRIBUTE            86
#define WHITE_IN_URI                 87

#define PREVIOUS_LOCATION            88 /* last */

/* character encoding errors */

#define VENDOR_SPECIFIC_CHARS        76
#define INVALID_SGML_CHARS           77
#define INVALID_UTF8                 78
#define INVALID_UTF16                79
#define ENCODING_MISMATCH            80
#define INVALID_URI                  81
#define INVALID_NCR                  82

/* accessibility flaws */

#define BA_MISSING_IMAGE_ALT       1
#define BA_MISSING_LINK_ALT        2
#define BA_MISSING_SUMMARY         4
#define BA_MISSING_IMAGE_MAP       8
#define BA_USING_FRAMES            16
#define BA_USING_NOFRAMES          32
#define BA_INVALID_LINK_NOFRAMES   64  /* WAI [6.5.1.4] */  
#define BA_WAI                     (1 << 31)

/* presentation flaws */

#define USING_SPACER            1
#define USING_LAYER             2
#define USING_NOBR              4
#define USING_FONT              8
#define USING_BODY              16

#define REPLACED_CHAR           0
#define DISCARDED_CHAR          1

/* badchar bit field */

#define BC_VENDOR_SPECIFIC_CHARS   1
#define BC_INVALID_SGML_CHARS      2
#define BC_INVALID_UTF8            4
#define BC_INVALID_UTF16           8
#define BC_ENCODING_MISMATCH       16 /* fatal error */
#define BC_INVALID_URI             32
#define BC_INVALID_NCR             64

#endif /* __MESSAGE_H__ */
