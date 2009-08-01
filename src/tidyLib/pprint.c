/*
  pprint.c -- pretty print parse tree  
  
  (c) 1998-2007 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.
  
  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2008/03/22 20:23:37 $ 
    $Revision: 1.119 $ 

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pprint.h"
#include "tidy-int.h"
#include "parser.h"
#include "entities.h"
#include "tmbstr.h"
#include "utf8.h"

/*
  Block-level and unknown elements are printed on
  new lines and their contents indented 2 spaces

  Inline elements are printed inline.

  Inline content is wrapped on spaces (except in
  attribute values or preformatted text, after
  start tags and before end tags
*/

static void PPrintAsp( TidyDocImpl* doc, uint indent, Node* node );
static void PPrintJste( TidyDocImpl* doc, uint indent, Node* node );
static void PPrintPhp( TidyDocImpl* doc, uint indent, Node* node );
static int  TextEndsWithNewline( Lexer *lexer, Node *node, uint mode );
static int  TextStartsWithWhitespace( Lexer *lexer, Node *node, uint start, uint mode );
static Bool InsideHead( TidyDocImpl* doc, Node *node );
static Bool ShouldIndent( TidyDocImpl* doc, Node *node );

#if SUPPORT_ASIAN_ENCODINGS
/* #431953 - start RJ Wraplen adjusted for smooth international ride */

#if 0
uint CWrapLen( TidyDocImpl* doc, uint ind )
{
    ctmbstr lang = cfgStr( doc, TidyLanguage );
    uint wraplen = cfg( doc, TidyWrapLen );

    if ( !TY_(tmbstrcasecmp)(lang, "zh") )
        /* Chinese characters take two positions on a fixed-width screen */ 
        /* It would be more accurate to keep a parallel linelen and wraphere
           incremented by 2 for Chinese characters and 1 otherwise, but this
           is way simpler.
        */
        return (ind + (( wraplen - ind ) / 2)) ; 
    
    if ( !TY_(tmbstrcasecmp)(lang, "ja") )
        /* average Japanese text is 30% kanji */
        return (ind + ((( wraplen - ind ) * 7) / 10)) ; 
    
    return wraplen;
}
#endif

typedef enum
{
  UC00, /* None                       */
  UCPC, /* Punctuation, Connector     */
  UCPD, /* Punctuation, Dash          */
  UCPE, /* Punctuation, Close         */
  UCPS, /* Punctuation, Open          */
  UCPI, /* Punctuation, Initial quote */
  UCPF, /* Punctuation, Final quote   */
  UCPO, /* Punctuation, Other         */
  UCZS, /* Separator, Space           */
  UCZL, /* Separator, Line            */
  UCZP  /* Separator, Paragraph       */
} UnicodeCategory;

/*
  From the original code, the following characters are removed:

    U+2011 (non-breaking hyphen)
    U+202F (narrow non-break space)
    U+2044 (fraction slash) 
    U+200B (zero width space)
    ...... (bidi formatting control characters)

  U+2011 and U+202F are non-breaking, U+2044 is a Sm character,
  U+200B is a non-visible space, wrapping after it would make
  this space visible, bidi should be done using HTML features
  and the characters are neither Px or Zx.

  The following Unicode 3.0 punctuation characters are added:

    U+2048 (question exclamation mark)
    U+2049 (exclamation question mark)
    U+204A (tironian sign et)
    U+204B (reversed pilcrow sign)
    U+204C (black leftwards bullet)
    U+204D (black rightwards bullet)
    U+3030 (wavy dash)
    U+30FB (katakana middle dot)
    U+FE63 (small hyphen-minus)
    U+FE68 (small reverse solidus)
    U+FF3F (fullwidth low line)
    U+FF5B (fullwidth left curly bracket)
    U+FF5D (fullwidth right curly bracket)

  Other additional characters were not included in Unicode 3.0.
  The table is based on Unicode 4.0. It must include only those
  characters marking a wrapping point, "before" if the general
  category is UCPS or UCPI, otherwise "after".
*/
static struct _unicode4cat
{
  unsigned long code;
  UnicodeCategory category;
} const unicode4cat[] =
{
#if 0
  { 0x037E, UCPO }, { 0x0387, UCPO }, { 0x055A, UCPO }, { 0x055B, UCPO },
  { 0x055C, UCPO }, { 0x055D, UCPO }, { 0x055E, UCPO }, { 0x055F, UCPO },
  { 0x0589, UCPO }, { 0x058A, UCPD }, { 0x05BE, UCPO }, { 0x05C0, UCPO },
  { 0x05C3, UCPO }, { 0x05F3, UCPO }, { 0x05F4, UCPO }, { 0x060C, UCPO },
  { 0x060D, UCPO }, { 0x061B, UCPO }, { 0x061F, UCPO }, { 0x066A, UCPO },
  { 0x066B, UCPO }, { 0x066C, UCPO }, { 0x066D, UCPO }, { 0x06D4, UCPO },
  { 0x0700, UCPO }, { 0x0701, UCPO }, { 0x0702, UCPO }, { 0x0703, UCPO },
  { 0x0704, UCPO }, { 0x0705, UCPO }, { 0x0706, UCPO }, { 0x0707, UCPO },
  { 0x0708, UCPO }, { 0x0709, UCPO }, { 0x070A, UCPO }, { 0x070B, UCPO },
  { 0x070C, UCPO }, { 0x070D, UCPO }, { 0x0964, UCPO }, { 0x0965, UCPO },
  { 0x0970, UCPO }, { 0x0DF4, UCPO }, { 0x0E4F, UCPO }, { 0x0E5A, UCPO },
  { 0x0E5B, UCPO }, { 0x0F04, UCPO }, { 0x0F05, UCPO }, { 0x0F06, UCPO },
  { 0x0F07, UCPO }, { 0x0F08, UCPO }, { 0x0F09, UCPO }, { 0x0F0A, UCPO },
  { 0x0F0B, UCPO }, { 0x0F0D, UCPO }, { 0x0F0E, UCPO }, { 0x0F0F, UCPO },
  { 0x0F10, UCPO }, { 0x0F11, UCPO }, { 0x0F12, UCPO }, { 0x0F3A, UCPS },
  { 0x0F3B, UCPE }, { 0x0F3C, UCPS }, { 0x0F3D, UCPE }, { 0x0F85, UCPO },
  { 0x104A, UCPO }, { 0x104B, UCPO }, { 0x104C, UCPO }, { 0x104D, UCPO },
  { 0x104E, UCPO }, { 0x104F, UCPO }, { 0x10FB, UCPO }, { 0x1361, UCPO },
  { 0x1362, UCPO }, { 0x1363, UCPO }, { 0x1364, UCPO }, { 0x1365, UCPO },
  { 0x1366, UCPO }, { 0x1367, UCPO }, { 0x1368, UCPO }, { 0x166D, UCPO },
  { 0x166E, UCPO }, { 0x1680, UCZS }, { 0x169B, UCPS }, { 0x169C, UCPE },
  { 0x16EB, UCPO }, { 0x16EC, UCPO }, { 0x16ED, UCPO }, { 0x1735, UCPO },
  { 0x1736, UCPO }, { 0x17D4, UCPO }, { 0x17D5, UCPO }, { 0x17D6, UCPO },
  { 0x17D8, UCPO }, { 0x17D9, UCPO }, { 0x17DA, UCPO }, { 0x1800, UCPO },
  { 0x1801, UCPO }, { 0x1802, UCPO }, { 0x1803, UCPO }, { 0x1804, UCPO },
  { 0x1805, UCPO }, { 0x1806, UCPD }, { 0x1807, UCPO }, { 0x1808, UCPO },
  { 0x1809, UCPO }, { 0x180A, UCPO }, { 0x180E, UCZS }, { 0x1944, UCPO },
  { 0x1945, UCPO }, 
#endif
  { 0x2000, UCZS }, { 0x2001, UCZS }, { 0x2002, UCZS }, { 0x2003, UCZS },
  { 0x2004, UCZS }, { 0x2005, UCZS }, { 0x2006, UCZS }, { 0x2008, UCZS },
  { 0x2009, UCZS }, { 0x200A, UCZS }, { 0x2010, UCPD }, { 0x2012, UCPD },
  { 0x2013, UCPD }, { 0x2014, UCPD }, { 0x2015, UCPD }, { 0x2016, UCPO },
  { 0x2017, UCPO }, { 0x2018, UCPI }, { 0x2019, UCPF }, { 0x201A, UCPS },
  { 0x201B, UCPI }, { 0x201C, UCPI }, { 0x201D, UCPF }, { 0x201E, UCPS },
  { 0x201F, UCPI }, { 0x2020, UCPO }, { 0x2021, UCPO }, { 0x2022, UCPO },
  { 0x2023, UCPO }, { 0x2024, UCPO }, { 0x2025, UCPO }, { 0x2026, UCPO },
  { 0x2027, UCPO }, { 0x2028, UCZL }, { 0x2029, UCZP }, { 0x2030, UCPO },
  { 0x2031, UCPO }, { 0x2032, UCPO }, { 0x2033, UCPO }, { 0x2034, UCPO },
  { 0x2035, UCPO }, { 0x2036, UCPO }, { 0x2037, UCPO }, { 0x2038, UCPO },
  { 0x2039, UCPI }, { 0x203A, UCPF }, { 0x203B, UCPO }, { 0x203C, UCPO },
  { 0x203D, UCPO }, { 0x203E, UCPO }, { 0x203F, UCPC }, { 0x2040, UCPC },
  { 0x2041, UCPO }, { 0x2042, UCPO }, { 0x2043, UCPO }, { 0x2045, UCPS },
  { 0x2046, UCPE }, { 0x2047, UCPO }, { 0x2048, UCPO }, { 0x2049, UCPO },
  { 0x204A, UCPO }, { 0x204B, UCPO }, { 0x204C, UCPO }, { 0x204D, UCPO },
  { 0x204E, UCPO }, { 0x204F, UCPO }, { 0x2050, UCPO }, { 0x2051, UCPO },
  { 0x2053, UCPO }, { 0x2054, UCPC }, { 0x2057, UCPO }, { 0x205F, UCZS },
  { 0x207D, UCPS }, { 0x207E, UCPE }, { 0x208D, UCPS }, { 0x208E, UCPE },
  { 0x2329, UCPS }, { 0x232A, UCPE }, { 0x23B4, UCPS }, { 0x23B5, UCPE },
  { 0x23B6, UCPO }, { 0x2768, UCPS }, { 0x2769, UCPE }, { 0x276A, UCPS },
  { 0x276B, UCPE }, { 0x276C, UCPS }, { 0x276D, UCPE }, { 0x276E, UCPS },
  { 0x276F, UCPE }, { 0x2770, UCPS }, { 0x2771, UCPE }, { 0x2772, UCPS },
  { 0x2773, UCPE }, { 0x2774, UCPS }, { 0x2775, UCPE }, { 0x27E6, UCPS },
  { 0x27E7, UCPE }, { 0x27E8, UCPS }, { 0x27E9, UCPE }, { 0x27EA, UCPS },
  { 0x27EB, UCPE }, { 0x2983, UCPS }, { 0x2984, UCPE }, { 0x2985, UCPS },
  { 0x2986, UCPE }, { 0x2987, UCPS }, { 0x2988, UCPE }, { 0x2989, UCPS },
  { 0x298A, UCPE }, { 0x298B, UCPS }, { 0x298C, UCPE }, { 0x298D, UCPS },
  { 0x298E, UCPE }, { 0x298F, UCPS }, { 0x2990, UCPE }, { 0x2991, UCPS },
  { 0x2992, UCPE }, { 0x2993, UCPS }, { 0x2994, UCPE }, { 0x2995, UCPS },
  { 0x2996, UCPE }, { 0x2997, UCPS }, { 0x2998, UCPE }, { 0x29D8, UCPS },
  { 0x29D9, UCPE }, { 0x29DA, UCPS }, { 0x29DB, UCPE }, { 0x29FC, UCPS },
  { 0x29FD, UCPE }, { 0x3001, UCPO }, { 0x3002, UCPO }, { 0x3003, UCPO },
  { 0x3008, UCPS }, { 0x3009, UCPE }, { 0x300A, UCPS }, { 0x300B, UCPE },
  { 0x300C, UCPS }, { 0x300D, UCPE }, { 0x300E, UCPS }, { 0x300F, UCPE },
  { 0x3010, UCPS }, { 0x3011, UCPE }, { 0x3014, UCPS }, { 0x3015, UCPE },
  { 0x3016, UCPS }, { 0x3017, UCPE }, { 0x3018, UCPS }, { 0x3019, UCPE },
  { 0x301A, UCPS }, { 0x301B, UCPE }, { 0x301C, UCPD }, { 0x301D, UCPS },
  { 0x301E, UCPE }, { 0x301F, UCPE }, { 0x3030, UCPD }, { 0x303D, UCPO },
  { 0x30A0, UCPD }, { 0x30FB, UCPC }, { 0xFD3E, UCPS }, { 0xFD3F, UCPE },
  { 0xFE30, UCPO }, { 0xFE31, UCPD }, { 0xFE32, UCPD }, { 0xFE33, UCPC },
  { 0xFE34, UCPC }, { 0xFE35, UCPS }, { 0xFE36, UCPE }, { 0xFE37, UCPS },
  { 0xFE38, UCPE }, { 0xFE39, UCPS }, { 0xFE3A, UCPE }, { 0xFE3B, UCPS },
  { 0xFE3C, UCPE }, { 0xFE3D, UCPS }, { 0xFE3E, UCPE }, { 0xFE3F, UCPS },
  { 0xFE40, UCPE }, { 0xFE41, UCPS }, { 0xFE42, UCPE }, { 0xFE43, UCPS },
  { 0xFE44, UCPE }, { 0xFE45, UCPO }, { 0xFE46, UCPO }, { 0xFE47, UCPS },
  { 0xFE48, UCPE }, { 0xFE49, UCPO }, { 0xFE4A, UCPO }, { 0xFE4B, UCPO },
  { 0xFE4C, UCPO }, { 0xFE4D, UCPC }, { 0xFE4E, UCPC }, { 0xFE4F, UCPC },
  { 0xFE50, UCPO }, { 0xFE51, UCPO }, { 0xFE52, UCPO }, { 0xFE54, UCPO },
  { 0xFE55, UCPO }, { 0xFE56, UCPO }, { 0xFE57, UCPO }, { 0xFE58, UCPD },
  { 0xFE59, UCPS }, { 0xFE5A, UCPE }, { 0xFE5B, UCPS }, { 0xFE5C, UCPE },
  { 0xFE5D, UCPS }, { 0xFE5E, UCPE }, { 0xFE5F, UCPO }, { 0xFE60, UCPO },
  { 0xFE61, UCPO }, { 0xFE63, UCPD }, { 0xFE68, UCPO }, { 0xFE6A, UCPO },
  { 0xFE6B, UCPO }, { 0xFF01, UCPO }, { 0xFF02, UCPO }, { 0xFF03, UCPO },
  { 0xFF05, UCPO }, { 0xFF06, UCPO }, { 0xFF07, UCPO }, { 0xFF08, UCPS },
  { 0xFF09, UCPE }, { 0xFF0A, UCPO }, { 0xFF0C, UCPO }, { 0xFF0D, UCPD },
  { 0xFF0E, UCPO }, { 0xFF0F, UCPO }, { 0xFF1A, UCPO }, { 0xFF1B, UCPO },
  { 0xFF1F, UCPO }, { 0xFF20, UCPO }, { 0xFF3B, UCPS }, { 0xFF3C, UCPO },
  { 0xFF3D, UCPE }, { 0xFF3F, UCPC }, { 0xFF5B, UCPS }, { 0xFF5D, UCPE },
  { 0xFF5F, UCPS }, { 0xFF60, UCPE }, { 0xFF61, UCPO }, { 0xFF62, UCPS },
  { 0xFF63, UCPE }, { 0xFF64, UCPO }, { 0xFF65, UCPC }, { 0x10100,UCPO },
  { 0x10101,UCPO }, { 0x1039F,UCPO },

  /* final entry */
  { 0x0000, UC00 }
};

typedef enum
{
    NoWrapPoint,
    WrapBefore,
    WrapAfter
} WrapPoint;

/*
  If long lines of text have no white space as defined in HTML 4
  (U+0009, U+000A, U+000D, U+000C, U+0020) other characters could
  be used to determine a wrap point. Since user agents would
  normalize the inserted newline character to a space character,
  this wrapping behaviour would insert visual whitespace into the
  document.

  Characters of the General Category Pi and Ps in the Unicode
  character database (opening punctuation and intial quote
  characters) mark a wrapping point before the character, other
  punctuation characters (Pc, Pd, Pe, Pf, and Po), breakable
  space characters (Zs), and paragraph and line separators
  (Zl, Zp) mark a wrap point after the character. Using this
  function Tidy can for example pretty print

    <p>....................&ldquo;...quote...&rdquo;...</p>
  as
    <p>....................\n&ldquo;...quote...&rdquo;...</p>
  or
    <p>....................&ldquo;...quote...&rdquo;\n...</p>

  if the next normal wrapping point would exceed the user
  chosen wrapping column.
*/
static WrapPoint CharacterWrapPoint(tchar c)
{
    int i;
    for (i = 0; unicode4cat[i].code && unicode4cat[i].code <= c; ++i)
        if (unicode4cat[i].code == c)
        {
            /* wrapping before opening punctuation and initial quotes */
            if (unicode4cat[i].category == UCPS ||
                unicode4cat[i].category == UCPI)
                return WrapBefore;
            /* else wrapping after this character */
            else
                return WrapAfter;
        }
    /* character has no effect on line wrapping */
    return NoWrapPoint;
}

static WrapPoint Big5WrapPoint(tchar c)
{
    if ((c & 0xFF00) == 0xA100)
    { 
        /* opening brackets have odd codes: break before them */ 
        if ( c > 0xA15C && c < 0xA1AD && (c & 1) == 1 ) 
            return WrapBefore;
        return WrapAfter;
    }
    return NoWrapPoint;
}

#endif /* SUPPORT_ASIAN_ENCODINGS */

static void InitIndent( TidyIndent* ind )
{
    ind->spaces = -1;
    ind->attrValStart = -1;
    ind->attrStringStart = -1;
}

void TY_(InitPrintBuf)( TidyDocImpl* doc )
{
    TidyClearMemory( &doc->pprint, sizeof(TidyPrintImpl) );
    InitIndent( &doc->pprint.indent[0] );
    InitIndent( &doc->pprint.indent[1] );
    doc->pprint.allocator = doc->allocator;
}

void TY_(FreePrintBuf)( TidyDocImpl* doc )
{
    TidyDocFree( doc, doc->pprint.linebuf );
    TY_(InitPrintBuf)( doc );
}

static void expand( TidyPrintImpl* pprint, uint len )
{
    uint* ip;
    uint buflen = pprint->lbufsize;

    if ( buflen == 0 )
        buflen = 256;
    while ( len >= buflen )
        buflen *= 2;

    ip = (uint*) TidyRealloc( pprint->allocator, pprint->linebuf, buflen*sizeof(uint) );
    if ( ip )
    {
      TidyClearMemory( ip+pprint->lbufsize, 
                       (buflen-pprint->lbufsize)*sizeof(uint) );
      pprint->lbufsize = buflen;
      pprint->linebuf = ip;
    }
}

static uint GetSpaces( TidyPrintImpl* pprint )
{
    int spaces = pprint->indent[ 0 ].spaces;
    return ( spaces < 0 ? 0U : (uint) spaces );
}
static int ClearInString( TidyPrintImpl* pprint )
{
    TidyIndent *ind = pprint->indent + pprint->ixInd;
    return ind->attrStringStart = -1;
}
static int ToggleInString( TidyPrintImpl* pprint )
{
    TidyIndent *ind = pprint->indent + pprint->ixInd;
    Bool inString = ( ind->attrStringStart >= 0 );
    return ind->attrStringStart = ( inString ? -1 : (int) pprint->linelen );
}
static Bool IsInString( TidyPrintImpl* pprint )
{
    TidyIndent *ind = pprint->indent + 0; /* Always 1st */
    return ( ind->attrStringStart >= 0 && 
             ind->attrStringStart < (int) pprint->linelen );
}
static Bool IsWrapInString( TidyPrintImpl* pprint )
{
    TidyIndent *ind = pprint->indent + 0; /* Always 1st */
    int wrap = (int) pprint->wraphere;
    return ( ind->attrStringStart == 0 ||
             (ind->attrStringStart > 0 && ind->attrStringStart < wrap) );
}

static Bool HasMixedContent (Node *element)
{
    Node * node;

    if (!element)
        return no;

    for (node = element->content; node; node = node->next)
        if ( TY_(nodeIsText)(node) )
             return yes;

    return no;
}

static void ClearInAttrVal( TidyPrintImpl* pprint )
{
    TidyIndent *ind = pprint->indent + pprint->ixInd;
    ind->attrValStart = -1;
}
static int SetInAttrVal( TidyPrintImpl* pprint )
{
    TidyIndent *ind = pprint->indent + pprint->ixInd;
    return ind->attrValStart = (int) pprint->linelen;
}
static Bool IsWrapInAttrVal( TidyPrintImpl* pprint )
{
    TidyIndent *ind = pprint->indent + 0; /* Always 1st */
    int wrap = (int) pprint->wraphere;
    return ( ind->attrValStart == 0 ||
             (ind->attrValStart > 0 && ind->attrValStart < wrap) );
}

static Bool WantIndent( TidyDocImpl* doc )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Bool wantIt = GetSpaces(pprint) > 0;
    if ( wantIt )
    {
        Bool indentAttrs = cfgBool( doc, TidyIndentAttributes );
        wantIt = ( ( !IsWrapInAttrVal(pprint) || indentAttrs ) &&
                   !IsWrapInString(pprint) );
    }
    return wantIt;
}


static uint  WrapOff( TidyDocImpl* doc )
{
    uint saveWrap = cfg( doc, TidyWrapLen );
    TY_(SetOptionInt)( doc, TidyWrapLen, 0xFFFFFFFF );  /* very large number */
    return saveWrap;
}

static void  WrapOn( TidyDocImpl* doc, uint saveWrap )
{
    TY_(SetOptionInt)( doc, TidyWrapLen, saveWrap );
}

static uint  WrapOffCond( TidyDocImpl* doc, Bool onoff )
{
    if ( onoff )
        return WrapOff( doc );
    return cfg( doc, TidyWrapLen );
}


static void AddC( TidyPrintImpl* pprint, uint c, uint string_index)
{
    if ( string_index + 1 >= pprint->lbufsize )
        expand( pprint, string_index + 1 );
    pprint->linebuf[string_index] = c;
}

static uint AddChar( TidyPrintImpl* pprint, uint c )
{
    AddC( pprint, c, pprint->linelen );
    return ++pprint->linelen;
}

static uint AddAsciiString( TidyPrintImpl* pprint, ctmbstr str, uint string_index )
{
    uint ix, len = TY_(tmbstrlen)( str );
    if ( string_index + len >= pprint->lbufsize )
        expand( pprint, string_index + len );

    for ( ix=0; ix<len; ++ix )
        pprint->linebuf[string_index + ix] = str[ ix ];
    return string_index + len;
}

static uint AddString( TidyPrintImpl* pprint, ctmbstr str )
{
   return pprint->linelen = AddAsciiString( pprint, str, pprint->linelen );
}

/* Saves current output point as the wrap point,
** but only if indentation would NOT overflow 
** the current line.  Otherwise keep previous wrap point.
*/
static Bool SetWrap( TidyDocImpl* doc, uint indent )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Bool wrap = ( indent + pprint->linelen < cfg(doc, TidyWrapLen) );
    if ( wrap )
    {
        if ( pprint->indent[0].spaces < 0 )
            pprint->indent[0].spaces = indent;
        pprint->wraphere = pprint->linelen;
    }
    else if ( pprint->ixInd == 0 )
    {
        /* Save indent 1st time we pass the the wrap line */
        pprint->indent[ 1 ].spaces = indent;
        pprint->ixInd = 1;
    }
    return wrap;
}

static void CarryOver( int* valTo, int* valFrom, uint wrapPoint )
{
  if ( *valFrom > (int) wrapPoint )
  {
    *valTo = *valFrom - wrapPoint;
    *valFrom = -1;
  }
}


static Bool SetWrapAttr( TidyDocImpl* doc,
                         uint indent, int attrStart, int strStart )
{
    TidyPrintImpl* pprint = &doc->pprint;
    TidyIndent *ind = pprint->indent + 0;

    Bool wrap = ( indent + pprint->linelen < cfg(doc, TidyWrapLen) );
    if ( wrap )
    {
        if ( ind[0].spaces < 0 )
            ind[0].spaces = indent;
        pprint->wraphere = pprint->linelen;
    }
    else if ( pprint->ixInd == 0 )
    {
        /* Save indent 1st time we pass the the wrap line */
        pprint->indent[ 1 ].spaces = indent;
        pprint->ixInd = 1;

        /* Carry over string state */
        CarryOver( &ind[1].attrStringStart, &ind[0].attrStringStart, pprint->wraphere );
        CarryOver( &ind[1].attrValStart, &ind[0].attrValStart, pprint->wraphere );
    }
    ind += doc->pprint.ixInd;
    ind->attrValStart = attrStart;
    ind->attrStringStart = strStart;
    return wrap;
}


/* Reset indent state after flushing a new line
*/
static void ResetLine( TidyPrintImpl* pprint )
{
    TidyIndent* ind = pprint->indent + 0;
    if ( pprint->ixInd > 0 )
    {
        ind[0] = ind[1];
        InitIndent( &ind[1] );
    }

    if ( pprint->wraphere > 0 )
    {
        int wrap = (int) pprint->wraphere;
        if ( ind[0].attrStringStart > wrap )
            ind[0].attrStringStart -= wrap;
        if ( ind[0].attrValStart > wrap )
            ind[0].attrValStart -= wrap;
    }
    else
    {
        if ( ind[0].attrStringStart > 0 )
            ind[0].attrStringStart = 0;
        if ( ind[0].attrValStart > 0 )
            ind[0].attrValStart = 0;
    }
    pprint->wraphere = pprint->ixInd = 0;
}

/* Shift text after wrap point to
** beginning of next line.
*/
static void ResetLineAfterWrap( TidyPrintImpl* pprint )
{
    if ( pprint->linelen > pprint->wraphere )
    {
        uint *p = pprint->linebuf;
        uint *q = p + pprint->wraphere;
        uint *end = p + pprint->linelen;

        if ( ! IsWrapInAttrVal(pprint) )
        {
            while ( q < end && *q == ' ' )
                ++q, ++pprint->wraphere;
        }

        while ( q < end )
            *p++ = *q++;

        pprint->linelen -= pprint->wraphere;
    }
    else
    {
        pprint->linelen = 0;
    }

    ResetLine( pprint );
}

/* Goes ahead with writing current line up to
** previously saved wrap point.  Shifts unwritten
** text in output buffer to beginning of next line.
*/
static void WrapLine( TidyDocImpl* doc )
{
    TidyPrintImpl* pprint = &doc->pprint;
    uint i;

    if ( pprint->wraphere == 0 )
        return;

    if ( WantIndent(doc) )
    {
        uint spaces = GetSpaces( pprint );
        for ( i = 0; i < spaces; ++i )
            TY_(WriteChar)( ' ', doc->docOut );
    }

    for ( i = 0; i < pprint->wraphere; ++i )
        TY_(WriteChar)( pprint->linebuf[i], doc->docOut );

    if ( IsWrapInString(pprint) )
        TY_(WriteChar)( '\\', doc->docOut );

    TY_(WriteChar)( '\n', doc->docOut );
    ResetLineAfterWrap( pprint );
}

/* Checks current output line length along with current indent.
** If combined they overflow output line length, go ahead
** and flush output up to the current wrap point.
*/
static Bool CheckWrapLine( TidyDocImpl* doc )
{
    TidyPrintImpl* pprint = &doc->pprint;
    if ( GetSpaces(pprint) + pprint->linelen >= cfg(doc, TidyWrapLen) )
    {
        WrapLine( doc );
        return yes;
    }
    return no;
}

static Bool CheckWrapIndent( TidyDocImpl* doc, uint indent )
{
    TidyPrintImpl* pprint = &doc->pprint;
    if ( GetSpaces(pprint) + pprint->linelen >= cfg(doc, TidyWrapLen) )
    {
        WrapLine( doc );
        if ( pprint->indent[ 0 ].spaces < 0 )
            pprint->indent[ 0 ].spaces = indent;
        return yes;
    }
    return no;
}

static void WrapAttrVal( TidyDocImpl* doc )
{
    TidyPrintImpl* pprint = &doc->pprint;
    uint i;

    /* assert( IsWrapInAttrVal(pprint) ); */
    if ( WantIndent(doc) )
    {
        uint spaces = GetSpaces( pprint );
        for ( i = 0; i < spaces; ++i )
            TY_(WriteChar)( ' ', doc->docOut );
    }

    for ( i = 0; i < pprint->wraphere; ++i )
        TY_(WriteChar)( pprint->linebuf[i], doc->docOut );

    if ( IsWrapInString(pprint) )
        TY_(WriteChar)( '\\', doc->docOut );
    else
        TY_(WriteChar)( ' ', doc->docOut );

    TY_(WriteChar)( '\n', doc->docOut );
    ResetLineAfterWrap( pprint );
}

static void PFlushLineImpl( TidyDocImpl* doc )
{
    TidyPrintImpl* pprint = &doc->pprint;

    uint i;

    CheckWrapLine( doc );

    if ( WantIndent(doc) )
    {
        uint spaces = GetSpaces( pprint );
        for ( i = 0; i < spaces; ++i )
            TY_(WriteChar)( ' ', doc->docOut );
    }

    for ( i = 0; i < pprint->linelen; ++i )
        TY_(WriteChar)( pprint->linebuf[i], doc->docOut );
    
    if ( IsInString(pprint) )
        TY_(WriteChar)( '\\', doc->docOut );
    ResetLine( pprint );
    pprint->linelen = 0;
}

void TY_(PFlushLine)( TidyDocImpl* doc, uint indent )
{
    TidyPrintImpl* pprint = &doc->pprint;

    if ( pprint->linelen > 0 )
        PFlushLineImpl( doc );

    TY_(WriteChar)( '\n', doc->docOut );
    pprint->indent[ 0 ].spaces = indent;
}

static void PCondFlushLine( TidyDocImpl* doc, uint indent )
{
    TidyPrintImpl* pprint = &doc->pprint;

    if ( pprint->linelen > 0 )
    {
         PFlushLineImpl( doc );

         TY_(WriteChar)( '\n', doc->docOut );
         pprint->indent[ 0 ].spaces = indent;
    }
}

static void PPrintChar( TidyDocImpl* doc, uint c, uint mode )
{
    tmbchar entity[128];
    ctmbstr p;
    TidyPrintImpl* pprint  = &doc->pprint;
    uint outenc = cfg( doc, TidyOutCharEncoding );
    Bool qmark = cfgBool( doc, TidyQuoteMarks );

    if ( c == ' ' && !(mode & (PREFORMATTED | COMMENT | ATTRIBVALUE | CDATA)))
    {
        /* coerce a space character to a non-breaking space */
        if (mode & NOWRAP)
        {
            ctmbstr ent = "&nbsp;";
            /* by default XML doesn't define &nbsp; */
            if ( cfgBool(doc, TidyNumEntities) || cfgBool(doc, TidyXmlTags) )
                ent = "&#160;";
            AddString( pprint, ent );
            return;
        }
        else
            pprint->wraphere = pprint->linelen;
    }

    /* comment characters are passed raw */
    if ( mode & (COMMENT | CDATA) )
    {
        AddChar( pprint, c );
        return;
    }

    /* except in CDATA map < to &lt; etc. */
    if ( !(mode & CDATA) )
    {
        if ( c == '<')
        {
            AddString( pprint, "&lt;" );
            return;
        }
            
        if ( c == '>')
        {
            AddString( pprint, "&gt;" );
            return;
        }

        /*
          naked '&' chars can be left alone or
          quoted as &amp; The latter is required
          for XML where naked '&' are illegal.
        */
        if ( c == '&' && cfgBool(doc, TidyQuoteAmpersand)
             && !cfgBool(doc, TidyPreserveEntities) )
        {
            AddString( pprint, "&amp;" );
            return;
        }

        if ( c == '"' && qmark )
        {
            AddString( pprint, "&quot;" );
            return;
        }

        if ( c == '\'' && qmark )
        {
            AddString( pprint, "&#39;" );
            return;
        }

        if ( c == 160 && outenc != RAW )
        {
            if ( cfgBool(doc, TidyQuoteNbsp) )
            {
                if ( cfgBool(doc, TidyNumEntities) ||
                     cfgBool(doc, TidyXmlTags) )
                    AddString( pprint, "&#160;" );
                else
                    AddString( pprint, "&nbsp;" );
            }
            else
                AddChar( pprint, c );
            return;
        }
    }

#if SUPPORT_ASIAN_ENCODINGS

    /* #431953 - start RJ */
    /* Handle encoding-specific issues */
    switch ( outenc )
    {
    case UTF8:
#if SUPPORT_UTF16_ENCODINGS
    case UTF16:
    case UTF16LE:
    case UTF16BE:
#endif
        if (!(mode & PREFORMATTED) && cfg(doc, TidyPunctWrap))
        {
            WrapPoint wp = CharacterWrapPoint(c);
            if (wp == WrapBefore)
                pprint->wraphere = pprint->linelen;
            else if (wp == WrapAfter)
                pprint->wraphere = pprint->linelen + 1;
        }
        break;

    case BIG5:
        /* Allow linebreak at Chinese punctuation characters */
        /* There are not many spaces in Chinese */
        AddChar( pprint, c );
        if (!(mode & PREFORMATTED)  && cfg(doc, TidyPunctWrap))
        {
            WrapPoint wp = Big5WrapPoint(c);
            if (wp == WrapBefore)
                pprint->wraphere = pprint->linelen;
            else if (wp == WrapAfter)
                pprint->wraphere = pprint->linelen + 1;
        }
        return;

    case SHIFTJIS:
#ifndef NO_NATIVE_ISO2022_SUPPORT
    case ISO2022: /* ISO 2022 characters are passed raw */
#endif
    case RAW:
        AddChar( pprint, c );
        return;
    }
    /* #431953 - end RJ */

#else /* SUPPORT_ASIAN_ENCODINGS */

    /* otherwise ISO 2022 characters are passed raw */
    if (
#ifndef NO_NATIVE_ISO2022_SUPPORT
        outenc == ISO2022 ||
#endif
        outenc == RAW )
    {
        AddChar( pprint, c );
        return;
    }

#endif /* SUPPORT_ASIAN_ENCODINGS */

    /* don't map latin-1 chars to entities */
    if ( outenc == LATIN1 )
    {
        if (c > 255)  /* multi byte chars */
        {
            uint vers = TY_(HTMLVersion)( doc );
            if ( !cfgBool(doc, TidyNumEntities) && (p = TY_(EntityName)(c, vers)) )
                TY_(tmbsnprintf)(entity, sizeof(entity), "&%s;", p);
            else
                TY_(tmbsnprintf)(entity, sizeof(entity), "&#%u;", c);

            AddString( pprint, entity );
            return;
        }

        if (c > 126 && c < 160)
        {
            TY_(tmbsnprintf)(entity, sizeof(entity), "&#%u;", c);
            AddString( pprint, entity );
            return;
        }

        AddChar( pprint, c );
        return;
    }

    /* don't map UTF-8 chars to entities */
    if ( outenc == UTF8 )
    {
        AddChar( pprint, c );
        return;
    }

#if SUPPORT_UTF16_ENCODINGS
    /* don't map UTF-16 chars to entities */
    if ( outenc == UTF16 || outenc == UTF16LE || outenc == UTF16BE )
    {
        AddChar( pprint, c );
        return;
    }
#endif

    /* use numeric entities only  for XML */
    if ( cfgBool(doc, TidyXmlTags) )
    {
        /* if ASCII use numeric entities for chars > 127 */
        if ( c > 127 && outenc == ASCII )
        {
            TY_(tmbsnprintf)(entity, sizeof(entity), "&#%u;", c);
            AddString( pprint, entity );
            return;
        }

        /* otherwise output char raw */
        AddChar( pprint, c );
        return;
    }

    /* default treatment for ASCII */
    if ( outenc == ASCII && (c > 126 || (c < ' ' && c != '\t')) )
    {
        uint vers = TY_(HTMLVersion)( doc );
        if (!cfgBool(doc, TidyNumEntities) && (p = TY_(EntityName)(c, vers)) )
            TY_(tmbsnprintf)(entity, sizeof(entity), "&%s;", p);
        else
            TY_(tmbsnprintf)(entity, sizeof(entity), "&#%u;", c);

        AddString( pprint, entity );
        return;
    }

    AddChar( pprint, c );
}

static uint IncrWS( uint start, uint end, uint indent, int ixWS )
{
  if ( ixWS > 0 )
  {
    uint st = start + MIN( (uint)ixWS, indent );
    start = MIN( st, end );
  }
  return start;
}
/* 
  The line buffer is uint not char so we can
  hold Unicode values unencoded. The translation
  to UTF-8 is deferred to the TY_(WriteChar)() routine called
  to flush the line buffer.
*/
static void PPrintText( TidyDocImpl* doc, uint mode, uint indent,
                        Node* node  )
{
    uint start = node->start;
    uint end = node->end;
    uint ix, c = 0;
    int  ixNL = TextEndsWithNewline( doc->lexer, node, mode );
    int  ixWS = TextStartsWithWhitespace( doc->lexer, node, start, mode );
    if ( ixNL > 0 )
      end -= ixNL;
    start = IncrWS( start, end, indent, ixWS );

    for ( ix = start; ix < end; ++ix )
    {
        CheckWrapIndent( doc, indent );
        /*
        if ( CheckWrapIndent(doc, indent) )
        {
            ixWS = TextStartsWithWhitespace( doc->lexer, node, ix );
            ix = IncrWS( ix, end, indent, ixWS );
        }
        */
        c = (byte) doc->lexer->lexbuf[ix];

        /* look for UTF-8 multibyte character */
        if ( c > 0x7F )
             ix += TY_(GetUTF8)( doc->lexer->lexbuf + ix, &c );

        if ( c == '\n' )
        {
            TY_(PFlushLine)( doc, indent );
            ixWS = TextStartsWithWhitespace( doc->lexer, node, ix+1, mode );
            ix = IncrWS( ix, end, indent, ixWS );
        }
        else
        {
            PPrintChar( doc, c, mode );
        }
    }
}

#if 0
static void PPrintString( TidyDocImpl* doc, uint indent, ctmbstr str )
{
    while ( *str != '\0' )
        AddChar( &doc->pprint, *str++ );
}
#endif /* 0 */


static void PPrintAttrValue( TidyDocImpl* doc, uint indent,
                             ctmbstr value, uint delim, Bool wrappable, Bool scriptAttr )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Bool scriptlets = cfgBool(doc, TidyWrapScriptlets);

    uint mode = PREFORMATTED | ATTRIBVALUE;
    if ( wrappable )
        mode = NORMAL | ATTRIBVALUE;

    /* look for ASP, Tango or PHP instructions for computed attribute value */
    if ( value && value[0] == '<' )
    {
        if ( value[1] == '%' || value[1] == '@'||
             TY_(tmbstrncmp)(value, "<?php", 5) == 0 )
            mode |= CDATA;
    }

    if ( delim == 0 )
        delim = '"';

    AddChar( pprint, '=' );

    /* don't wrap after "=" for xml documents */
    if ( !cfgBool(doc, TidyXmlOut) || cfgBool(doc, TidyXhtmlOut) )
    {
        SetWrap( doc, indent );
        CheckWrapIndent( doc, indent );
        /*
        if ( !SetWrap(doc, indent) )
            PCondFlushLine( doc, indent );
        */
    }

    AddChar( pprint, delim );

    if ( value )
    {
        uint wraplen = cfg( doc, TidyWrapLen );
        int attrStart = SetInAttrVal( pprint );
        int strStart = ClearInString( pprint );

        while (*value != '\0')
        {
            uint c = *value;

            if ( wrappable && c == ' ' )
                SetWrapAttr( doc, indent, attrStart, strStart );

            if ( wrappable && pprint->wraphere > 0 &&
                 GetSpaces(pprint) + pprint->linelen >= wraplen )
                WrapAttrVal( doc );

            if ( c == delim )
            {
                ctmbstr entity = (c == '"' ? "&quot;" : "&#39;");
                AddString( pprint, entity );
                ++value;
                continue;
            }
            else if (c == '"')
            {
                if ( cfgBool(doc, TidyQuoteMarks) )
                    AddString( pprint, "&quot;" );
                else
                    AddChar( pprint, c );

                if ( delim == '\'' && scriptAttr && scriptlets )
                    strStart = ToggleInString( pprint );

                ++value;
                continue;
            }
            else if ( c == '\'' )
            {
                if ( cfgBool(doc, TidyQuoteMarks) )
                    AddString( pprint, "&#39;" );
                else
                    AddChar( pprint, c );

                if ( delim == '"' && scriptAttr && scriptlets )
                    strStart = ToggleInString( pprint );

                ++value;
                continue;
            }

            /* look for UTF-8 multibyte character */
            if ( c > 0x7F )
                 value += TY_(GetUTF8)( value, &c );
            ++value;

            if ( c == '\n' )
            {
                /* No indent inside Javascript literals */
                TY_(PFlushLine)( doc, (strStart < 0
                                       && !cfgBool(doc, TidyLiteralAttribs) ?
                                       indent : 0) );
                continue;
            }
            PPrintChar( doc, c, mode );
        }
        ClearInAttrVal( pprint );
        ClearInString( pprint );
    }
    AddChar( pprint, delim );
}

static uint AttrIndent( TidyDocImpl* doc, Node* node, AttVal* ARG_UNUSED(attr) )
{
  uint spaces = cfg( doc, TidyIndentSpaces );
  uint xtra = 2;  /* 1 for the '<', another for the ' ' */
  if ( node->element == NULL )
    return spaces;

  if ( !TY_(nodeHasCM)(node, CM_INLINE) ||
       !ShouldIndent(doc, node->parent ? node->parent: node) )
    return xtra + TY_(tmbstrlen)( node->element );

  if ( NULL != (node = TY_(FindContainer)(node)) )
    return xtra + TY_(tmbstrlen)( node->element );
  return spaces;
}

static Bool AttrNoIndentFirst( /*TidyDocImpl* doc,*/ Node* node, AttVal* attr )
{
  return ( attr==node->attributes );
  
  /*&& 
           ( InsideHead(doc, node) ||
             !TY_(nodeHasCM)(node, CM_INLINE) ) );
             */
}

static void PPrintAttribute( TidyDocImpl* doc, uint indent,
                             Node *node, AttVal *attr )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Bool xmlOut    = cfgBool( doc, TidyXmlOut );
    Bool xhtmlOut  = cfgBool( doc, TidyXhtmlOut );
    Bool wrapAttrs = cfgBool( doc, TidyWrapAttVals );
    Bool ucAttrs   = cfgBool( doc, TidyUpperCaseAttrs );
    Bool indAttrs  = cfgBool( doc, TidyIndentAttributes );
    uint xtra      = AttrIndent( doc, node, attr );
    Bool first     = AttrNoIndentFirst( /*doc,*/ node, attr );
    tmbstr name    = attr->attribute;
    Bool wrappable = no;
    tchar c;

    /* fix for odd attribute indentation bug triggered by long values */
    if (!indAttrs)
      xtra = 0;

    if ( indAttrs )
    {
        if ( TY_(nodeIsElement)(node) && !first )
        {
            indent += xtra;
            PCondFlushLine( doc, indent );
        }
        else
          indAttrs = no;
    }

    CheckWrapIndent( doc, indent );

    if ( !xmlOut && !xhtmlOut && attr->dict )
    {
        if ( TY_(IsScript)(doc, name) )
            wrappable = cfgBool( doc, TidyWrapScriptlets );
        else if (!(attrIsCONTENT(attr) || attrIsVALUE(attr) || attrIsALT(attr)) && wrapAttrs )
            wrappable = yes;
    }

    if ( !first && !SetWrap(doc, indent) )
    {
        TY_(PFlushLine)( doc, indent+xtra );  /* Put it on next line */
    }
    else if ( pprint->linelen > 0 )
    {
        AddChar( pprint, ' ' );
    }

    /* Attribute name */
    while (*name)
    {
        c = (unsigned char)*name;

        if (c > 0x7F)
            name += TY_(GetUTF8)(name, &c);
        else if (ucAttrs)
            c = TY_(ToUpper)(c);

        AddChar(pprint, c);
        ++name;
    }

/* fix for bug 732038 */
#if 0
    /* If not indenting attributes, bump up indent for 
    ** value after putting out name.
    */
    if ( !indAttrs )
        indent += xtra;
#endif

    CheckWrapIndent( doc, indent );
 
    if ( attr->value == NULL )
    {
        Bool isB = TY_(IsBoolAttribute)(attr);
        Bool scriptAttr = TY_(attrIsEvent)(attr);

        if ( xmlOut )
            PPrintAttrValue( doc, indent, isB ? attr->attribute : NULLSTR,
                             attr->delim, no, scriptAttr );

        else if ( !isB && !TY_(IsNewNode)(node) )
            PPrintAttrValue( doc, indent, "", attr->delim, yes, scriptAttr );

        else 
            SetWrap( doc, indent );
    }
    else
        PPrintAttrValue( doc, indent, attr->value, attr->delim, wrappable, no );
}

static void PPrintAttrs( TidyDocImpl* doc, uint indent, Node *node )
{
    TidyPrintImpl* pprint = &doc->pprint;
    AttVal* av;

    /* add xml:space attribute to pre and other elements */
    if ( cfgBool(doc, TidyXmlOut) && cfgBool(doc, TidyXmlSpace) &&
         !TY_(GetAttrByName)(node, "xml:space") &&
         TY_(XMLPreserveWhiteSpace)(doc, node) )
    {
        TY_(AddAttribute)( doc, node, "xml:space", "preserve" );
    }

    for ( av = node->attributes; av; av = av->next )
    {
        if ( av->attribute != NULL )
        {
            PPrintAttribute( doc, indent, node, av );
        }
        else if ( av->asp != NULL )
        {
            AddChar( pprint, ' ' );
            PPrintAsp( doc, indent, av->asp );
        }
        else if ( av->php != NULL )
        {
            AddChar( pprint, ' ' );
            PPrintPhp( doc, indent, av->php );
        }
    }
}

Bool TY_(TextNodeEndWithSpace)( Lexer *lexer, Node *node )
{
    if (TY_(nodeIsText)(node) && node->end > node->start)
    {
        uint i, c = '\0'; /* initialised to avoid warnings */
        for (i = node->start; i < node->end; ++i)
        {
            c = (byte) lexer->lexbuf[i];
            if ( c > 0x7F )
                i += TY_(GetUTF8)( lexer->lexbuf + i, &c );
        }

        if ( c == ' ' || c == '\n' )
            return yes;
    }
    return no;
}

/*
 Line can be wrapped immediately after inline start tag provided
 if follows a text node ending in a space, or it follows a <br>,
 or its parent is an inline element that that rule applies to.
 This behaviour was reverse engineered from Netscape 3.0.

 Line wrapping can occur if an element is not empty and before a block
 level. For instance:
 <p><span>
 x</span>y</p>
 will display properly. Whereas
 <p><img />
 x</p> won't.
*/
static Bool AfterSpaceImp(Lexer *lexer, Node *node, Bool isEmpty)
{
    Node *prev;

    if ( !TY_(nodeCMIsInline)(node) )
        return yes;

    prev = node->prev;
    if (prev)
    {
        if (TY_(nodeIsText)(prev))
            return TY_(TextNodeEndWithSpace)( lexer, prev );
        else if (nodeIsBR(prev))
            return yes;

        return no;
    }

    if ( isEmpty && !TY_(nodeCMIsInline)(node->parent) )
        return no;

    return AfterSpaceImp(lexer, node->parent, isEmpty);
}

static Bool AfterSpace(Lexer *lexer, Node *node)
{
    return AfterSpaceImp(lexer, node, TY_(nodeCMIsEmpty)(node));
}

static void PPrintTag( TidyDocImpl* doc,
                       uint mode, uint indent, Node *node )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Bool uc = cfgBool( doc, TidyUpperCaseTags );
    Bool xhtmlOut = cfgBool( doc, TidyXhtmlOut );
    Bool xmlOut = cfgBool( doc, TidyXmlOut );
    tchar c;
    tmbstr s = node->element;

    AddChar( pprint, '<' );

    if ( node->type == EndTag )
        AddChar( pprint, '/' );

    if (s)
    {
        while (*s)
        {
            c = (unsigned char)*s;

            if (c > 0x7F)
                s += TY_(GetUTF8)(s, &c);
            else if (uc)
                c = TY_(ToUpper)(c);

            AddChar(pprint, c);
            ++s;
        }
    }

    PPrintAttrs( doc, indent, node );

    if ( (xmlOut || xhtmlOut) &&
         (node->type == StartEndTag || TY_(nodeCMIsEmpty)(node)) )
    {
        AddChar( pprint, ' ' );   /* Space is NS compatibility hack <br /> */
        AddChar( pprint, '/' );   /* Required end tag marker */
    }

    AddChar( pprint, '>' );

    if ( (node->type != StartEndTag || xhtmlOut) && !(mode & PREFORMATTED) )
    {
        uint wraplen = cfg( doc, TidyWrapLen );
        CheckWrapIndent( doc, indent );

        if ( indent + pprint->linelen < wraplen )
        {
            /* wrap after start tag if is <br/> or if it's not inline.
               Technically, it would be safe to call only AfterSpace.
               However, it would disrupt the existing algorithm. So let's
               leave as is. Note that AfterSpace returns true for non inline
               elements but can still be false for some <br>. So it has to
               stay as well. */
            if (!(mode & NOWRAP)
                && (!TY_(nodeCMIsInline)(node) || nodeIsBR(node))
                && AfterSpace(doc->lexer, node))
            {
                pprint->wraphere = pprint->linelen;
            }
        }
        /* flush the current buffer only if it is known to be safe,
           i.e. it will not introduce some spurious white spaces.
           See bug #996484 */
        else if ( mode & NOWRAP ||
                  nodeIsBR(node) || AfterSpace(doc->lexer, node))
            PCondFlushLine( doc, indent );
    }
}

static void PPrintEndTag( TidyDocImpl* doc, uint ARG_UNUSED(mode),
                          uint ARG_UNUSED(indent), Node *node )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Bool uc = cfgBool( doc, TidyUpperCaseTags );
    tmbstr s = node->element;
    tchar c;

   /*
     Netscape ignores SGML standard by not ignoring a
     line break before </A> or </U> etc. To avoid rendering 
     this as an underlined space, I disable line wrapping
     before inline end tags by the #if 0 ... #endif
   */
#if 0
    if ( !(mode & NOWRAP) )
        SetWrap( doc, indent );
#endif

    AddString( pprint, "</" );

    if (s)
    {
        while (*s)
        {
             c = (unsigned char)*s;

             if (c > 0x7F)
                 s += TY_(GetUTF8)(s, &c);
             else if (uc)
                 c = TY_(ToUpper)(c);

             AddChar(pprint, c);
             ++s;
        }
    }

    AddChar( pprint, '>' );
}

static void PPrintComment( TidyDocImpl* doc, uint indent, Node* node )
{
    TidyPrintImpl* pprint = &doc->pprint;

    SetWrap( doc, indent );
    AddString( pprint, "<!--" );

#if 0
    SetWrap( doc, indent );
#endif

    PPrintText(doc, COMMENT, 0, node);

#if 0
    SetWrap( doc, indent );
    AddString( pprint, "--" );
#endif

    AddString(pprint, "--");
    AddChar( pprint, '>' );
    if ( node->linebreak && node->next )
        TY_(PFlushLine)( doc, indent );
}

static void PPrintDocType( TidyDocImpl* doc, uint indent, Node *node )
{
    TidyPrintImpl* pprint = &doc->pprint;
    uint wraplen = cfg( doc, TidyWrapLen );
    uint spaces = cfg( doc, TidyIndentSpaces );
    AttVal* fpi = TY_(GetAttrByName)(node, "PUBLIC");
    AttVal* sys = TY_(GetAttrByName)(node, "SYSTEM");

    /* todo: handle non-ASCII characters in FPI / SI / node->element */

    SetWrap( doc, indent );
    PCondFlushLine( doc, indent );

    AddString( pprint, "<!DOCTYPE " );
    SetWrap( doc, indent );
    if (node->element)
    {
        AddString(pprint, node->element);
    }

    if (fpi && fpi->value)
    {
        AddString(pprint, " PUBLIC ");
        AddChar(pprint, fpi->delim);
        AddString(pprint, fpi->value);
        AddChar(pprint, fpi->delim);
    }

    if (fpi && fpi->value && sys && sys->value)
    {
        uint i = pprint->linelen - (TY_(tmbstrlen)(sys->value) + 2) - 1;
        if (!(i>0&&TY_(tmbstrlen)(sys->value)+2+i<wraplen&&i<=(spaces?spaces:2)*2))
            i = 0;

        PCondFlushLine(doc, i);
        if (pprint->linelen)
            AddChar(pprint, ' ');
    }
    else if (sys && sys->value)
    {
        AddString(pprint, " SYSTEM ");
    }

    if (sys && sys->value)
    {
        AddChar(pprint, sys->delim);
        AddString(pprint, sys->value);
        AddChar(pprint, sys->delim);
    }

    if (node->content)
    {
        PCondFlushLine(doc, indent);
        AddChar(pprint, '[');
        PPrintText(doc, CDATA, 0, node->content);
        AddChar(pprint, ']');
    }

    SetWrap( doc, 0 );
    AddChar( pprint, '>' );
    PCondFlushLine( doc, indent );
}

static void PPrintPI( TidyDocImpl* doc, uint indent, Node *node )
{
    TidyPrintImpl* pprint = &doc->pprint;
    tchar c;
    tmbstr s;

    SetWrap( doc, indent );
    AddString( pprint, "<?" );

    s = node->element;

    while (s && *s)
    {
        c = (unsigned char)*s;
        if (c > 0x7F)
            s += TY_(GetUTF8)(s, &c);
        AddChar(pprint, c);
        ++s;
    }

    /* set CDATA to pass < and > unescaped */
    PPrintText( doc, CDATA, indent, node );

    if (cfgBool(doc, TidyXmlOut) ||
        cfgBool(doc, TidyXhtmlOut) || node->closed)
        AddChar( pprint, '?' );

    AddChar( pprint, '>' );
    PCondFlushLine( doc, indent );
}

static void PPrintXmlDecl( TidyDocImpl* doc, uint indent, Node *node )
{
    AttVal* att;
    uint saveWrap;
    TidyPrintImpl* pprint = &doc->pprint;
    Bool ucAttrs;
    SetWrap( doc, indent );
    saveWrap = WrapOff( doc );

    /* no case translation for XML declaration pseudo attributes */
    ucAttrs = cfgBool(doc, TidyUpperCaseAttrs);
    TY_(SetOptionBool)(doc, TidyUpperCaseAttrs, no);

    AddString( pprint, "<?xml" );

    /* Force order of XML declaration attributes */
    /* PPrintAttrs( doc, indent, node ); */
    if ( NULL != (att = TY_(AttrGetById)(node, TidyAttr_VERSION)) )
      PPrintAttribute( doc, indent, node, att );
    if ( NULL != (att = TY_(AttrGetById)(node, TidyAttr_ENCODING)) )
      PPrintAttribute( doc, indent, node, att );
    if ( NULL != (att = TY_(GetAttrByName)(node, "standalone")) )
      PPrintAttribute( doc, indent, node, att );

    /* restore old config value */
    TY_(SetOptionBool)(doc, TidyUpperCaseAttrs, ucAttrs);

    if ( node->end <= 0 || doc->lexer->lexbuf[node->end - 1] != '?' )
        AddChar( pprint, '?' );
    AddChar( pprint, '>' );
    WrapOn( doc, saveWrap );
    TY_(PFlushLine)( doc, indent );
}

/* note ASP and JSTE share <% ... %> syntax */
static void PPrintAsp( TidyDocImpl* doc, uint indent, Node *node )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Bool wrapAsp  = cfgBool( doc, TidyWrapAsp );
    Bool wrapJste = cfgBool( doc, TidyWrapJste );
    uint saveWrap = WrapOffCond( doc, !wrapAsp || !wrapJste );

#if 0
    SetWrap( doc, indent );
#endif
    AddString( pprint, "<%" );
    PPrintText( doc, (wrapAsp ? CDATA : COMMENT), indent, node );
    AddString( pprint, "%>" );

    /* PCondFlushLine( doc, indent ); */
    WrapOn( doc, saveWrap );
}

/* JSTE also supports <# ... #> syntax */
static void PPrintJste( TidyDocImpl* doc, uint indent, Node *node )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Bool wrapAsp = cfgBool( doc, TidyWrapAsp );
    uint saveWrap = WrapOffCond( doc, !wrapAsp  );

    AddString( pprint, "<#" );
    PPrintText( doc, (cfgBool(doc, TidyWrapJste) ? CDATA : COMMENT),
                indent, node );
    AddString( pprint, "#>" );

    /* PCondFlushLine( doc, indent ); */
    WrapOn( doc, saveWrap );
}

/* PHP is based on XML processing instructions */
static void PPrintPhp( TidyDocImpl* doc, uint indent, Node *node )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Bool wrapPhp = cfgBool( doc, TidyWrapPhp );
    uint saveWrap = WrapOffCond( doc, !wrapPhp  );
#if 0
    SetWrap( doc, indent );
#endif

    AddString( pprint, "<?" );
    PPrintText( doc, (wrapPhp ? CDATA : COMMENT),
                indent, node );
    AddString( pprint, "?>" );

    /* PCondFlushLine( doc, indent ); */
    WrapOn( doc, saveWrap );
}

static void PPrintCDATA( TidyDocImpl* doc, uint indent, Node *node )
{
    uint saveWrap;
    TidyPrintImpl* pprint = &doc->pprint;
    Bool indentCData = cfgBool( doc, TidyIndentCdata );
    if ( !indentCData )
        indent = 0;

    PCondFlushLine( doc, indent );
    saveWrap = WrapOff( doc );        /* disable wrapping */

    AddString( pprint, "<![CDATA[" );
    PPrintText( doc, COMMENT, indent, node );
    AddString( pprint, "]]>" );

    PCondFlushLine( doc, indent );
    WrapOn( doc, saveWrap );          /* restore wrapping */
}

static void PPrintSection( TidyDocImpl* doc, uint indent, Node *node )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Bool wrapSect = cfgBool( doc, TidyWrapSection );
    uint saveWrap = WrapOffCond( doc, !wrapSect  );
#if 0
    SetWrap( doc, indent );
#endif

    AddString( pprint, "<![" );
    PPrintText( doc, (wrapSect ? CDATA : COMMENT),
                indent, node );
    AddString( pprint, "]>" );

    /* PCondFlushLine( doc, indent ); */
    WrapOn( doc, saveWrap );
}


#if 0
/*
** Print script and style elements. For XHTML, wrap the content as follows:
**
**     JavaScript:
**         //<![CDATA[
**             content
**         //]]>
**     VBScript:
**         '<![CDATA[
**             content
**         ']]>
**     CSS:
**         / *<![CDATA[* /     Extra spaces to keep compiler happy
**             content
**         / *]]>* /
**     other:
**         <![CDATA[
**             content
**         ]]>
*/
#endif

static ctmbstr CDATA_START           = "<![CDATA[";
static ctmbstr CDATA_END             = "]]>";
static ctmbstr JS_COMMENT_START      = "//";
static ctmbstr JS_COMMENT_END        = "";
static ctmbstr VB_COMMENT_START      = "\'";
static ctmbstr VB_COMMENT_END        = "";
static ctmbstr CSS_COMMENT_START     = "/*";
static ctmbstr CSS_COMMENT_END       = "*/";
static ctmbstr DEFAULT_COMMENT_START = "";
static ctmbstr DEFAULT_COMMENT_END   = "";

static Bool InsideHead( TidyDocImpl* doc, Node *node )
{
  if ( nodeIsHEAD(node) )
    return yes;

  if ( node->parent != NULL )
    return InsideHead( doc, node->parent );

  return no;
}

/* Is text node and already ends w/ a newline?
 
   Used to pretty print CDATA/PRE text content.
   If it already ends on a newline, it is not
   necessary to print another before printing end tag.
*/
static int TextEndsWithNewline(Lexer *lexer, Node *node, uint mode )
{
    if ( (mode & (CDATA|COMMENT)) && TY_(nodeIsText)(node) && node->end > node->start )
    {
        uint ch, ix = node->end - 1;
        /* Skip non-newline whitespace. */
        while ( ix >= node->start && (ch = (lexer->lexbuf[ix] & 0xff))
                && ( ch == ' ' || ch == '\t' || ch == '\r' ) )
            --ix;

        if ( lexer->lexbuf[ ix ] == '\n' )
          return node->end - ix - 1; /* #543262 tidy eats all memory */
    }
    return -1;
}

static int TextStartsWithWhitespace( Lexer *lexer, Node *node, uint start, uint mode )
{
    assert( node != NULL );
    if ( (mode & (CDATA|COMMENT)) && TY_(nodeIsText)(node) && node->end > node->start && start >= node->start )
    {
        uint ch, ix = start;
        /* Skip whitespace. */
        while ( ix < node->end && (ch = (lexer->lexbuf[ix] & 0xff))
                && ( ch==' ' || ch=='\t' || ch=='\r' ) )
            ++ix;

        if ( ix > start )
          return ix - start;
    }
    return -1;
}

static Bool HasCDATA( Lexer* lexer, Node* node )
{
    /* Scan forward through the textarray. Since the characters we're
    ** looking for are < 0x7f, we don't have to do any UTF-8 decoding.
    */
    ctmbstr start = lexer->lexbuf + node->start;
    int len = node->end - node->start + 1;

    if ( node->type != TextNode )
        return no;

    return ( NULL != TY_(tmbsubstrn)( start, len, CDATA_START ));
}


static
void PPrintScriptStyle( TidyDocImpl* doc, uint mode, uint indent, Node *node )
{
    TidyPrintImpl* pprint = &doc->pprint;
    Node*   content;
    ctmbstr commentStart = DEFAULT_COMMENT_START;
    ctmbstr commentEnd = DEFAULT_COMMENT_END;
    Bool    hasCData = no;
    int     contentIndent = -1;
    Bool    xhtmlOut = cfgBool( doc, TidyXhtmlOut );

    if ( InsideHead(doc, node) )
      TY_(PFlushLine)( doc, indent );

    PPrintTag( doc, mode, indent, node );

    /* use zero indent here, see http://tidy.sf.net/bug/729972 */
    TY_(PFlushLine)(doc, 0);

    if ( xhtmlOut && node->content != NULL )
    {
        AttVal* type = attrGetTYPE(node);

        if (AttrValueIs(type, "text/javascript"))
        {
            commentStart = JS_COMMENT_START;
            commentEnd = JS_COMMENT_END;
        }
        else if (AttrValueIs(type, "text/css"))
        {
            commentStart = CSS_COMMENT_START;
            commentEnd = CSS_COMMENT_END;
        }
        else if (AttrValueIs(type, "text/vbscript"))
        {
            commentStart = VB_COMMENT_START;
            commentEnd = VB_COMMENT_END;
        }

        hasCData = HasCDATA(doc->lexer, node->content);

        if (!hasCData)
        {
            uint saveWrap = WrapOff( doc );

            AddString( pprint, commentStart );
            AddString( pprint, CDATA_START );
            AddString( pprint, commentEnd );
            PCondFlushLine( doc, indent );

            WrapOn( doc, saveWrap );
        }
    }

    for ( content = node->content;
          content != NULL;
          content = content->next )
    {
        /*
          This is a bit odd, with the current code there can only
          be one child and the only caller of this function defines
          all these modes already...
        */
        TY_(PPrintTree)( doc, (mode | PREFORMATTED | NOWRAP | CDATA), 
                         indent, content );

        if ( content == node->last )
            contentIndent = TextEndsWithNewline( doc->lexer, content, CDATA );
    }

    if ( contentIndent < 0 )
    {
        PCondFlushLine( doc, indent );
        contentIndent = 0;
    }

    if ( xhtmlOut && node->content != NULL )
    {
        if ( ! hasCData )
        {
            uint saveWrap = WrapOff( doc );

            AddString( pprint, commentStart );
            AddString( pprint, CDATA_END );
            AddString( pprint, commentEnd );

            WrapOn( doc, saveWrap );
            PCondFlushLine( doc, indent );
        }
    }

    if ( node->content && pprint->indent[ 0 ].spaces != (int)indent )
    {
        pprint->indent[ 0 ].spaces = indent;
    }
    PPrintEndTag( doc, mode, indent, node );
    if ( cfgAutoBool(doc, TidyIndentContent) == TidyNoState
         && node->next != NULL &&
         !( TY_(nodeHasCM)(node, CM_INLINE) || TY_(nodeIsText)(node) ) )
        TY_(PFlushLine)( doc, indent );
}



static Bool ShouldIndent( TidyDocImpl* doc, Node *node )
{
    TidyTriState indentContent = cfgAutoBool( doc, TidyIndentContent );
    if ( indentContent == TidyNoState )
        return no;

    if ( nodeIsTEXTAREA(node) )
        return no;

    if ( indentContent == TidyAutoState )
    {
        if ( node->content && TY_(nodeHasCM)(node, CM_NO_INDENT) )
        {
            for ( node = node->content; node; node = node->next )
                if ( TY_(nodeHasCM)(node, CM_BLOCK) )
                    return yes;
            return no;
        }

        if ( TY_(nodeHasCM)(node, CM_HEADING) )
            return no;

        if ( nodeIsHTML(node) )
            return no;

        if ( nodeIsP(node) )
            return no;

        if ( nodeIsTITLE(node) )
            return no;

        /* http://tidy.sf.net/issue/1610888
           Indenting <div><img /></div> produces spurious lines with IE 6.x */
        if ( nodeIsDIV(node) && node->last && nodeIsIMG(node->last) )
            return no;
    }

    if ( TY_(nodeHasCM)(node, CM_FIELD | CM_OBJECT) )
        return yes;

    if ( nodeIsMAP(node) )
        return yes;

    return ( !TY_(nodeHasCM)( node, CM_INLINE ) && node->content );
}

/*
 Feature request #434940 - fix by Dave Raggett/Ignacio Vazquez-Abrams 21 Jun 01
 print just the content of the body element.
 useful when you want to reuse material from
 other documents.

 -- Sebastiano Vigna <vigna@dsi.unimi.it>
*/
void TY_(PrintBody)( TidyDocImpl* doc )
{
    Node *node = TY_(FindBody)( doc );

    if ( node )
    {
        for ( node = node->content; node != NULL; node = node->next )
            TY_(PPrintTree)( doc, NORMAL, 0, node );
    }
}

void TY_(PPrintTree)( TidyDocImpl* doc, uint mode, uint indent, Node *node )
{
    Node *content, *last;
    uint spaces = cfg( doc, TidyIndentSpaces );
    Bool xhtml = cfgBool( doc, TidyXhtmlOut );

    if ( node == NULL )
        return;

    if (node->type == TextNode)
    {
        PPrintText( doc, mode, indent, node );
    }
    else if ( node->type == CommentTag )
    {
        PPrintComment( doc, indent, node );
    }
    else if ( node->type == RootNode )
    {
        for ( content = node->content; content; content = content->next )
           TY_(PPrintTree)( doc, mode, indent, content );
    }
    else if ( node->type == DocTypeTag )
        PPrintDocType( doc, indent, node );
    else if ( node->type == ProcInsTag)
        PPrintPI( doc, indent, node );
    else if ( node->type == XmlDecl)
        PPrintXmlDecl( doc, indent, node );
    else if ( node->type == CDATATag)
        PPrintCDATA( doc, indent, node );
    else if ( node->type == SectionTag)
        PPrintSection( doc, indent, node );
    else if ( node->type == AspTag)
        PPrintAsp( doc, indent, node );
    else if ( node->type == JsteTag)
        PPrintJste( doc, indent, node );
    else if ( node->type == PhpTag)
        PPrintPhp( doc, indent, node );
    else if ( TY_(nodeCMIsEmpty)(node) ||
              (node->type == StartEndTag && !xhtml) )
    {
        if ( ! TY_(nodeHasCM)(node, CM_INLINE) )
            PCondFlushLine( doc, indent );

        if ( nodeIsBR(node) && node->prev &&
             !(nodeIsBR(node->prev) || (mode & PREFORMATTED)) &&
             cfgBool(doc, TidyBreakBeforeBR) )
            TY_(PFlushLine)( doc, indent );

        if ( nodeIsHR(node) )
        {
            /* insert extra newline for classic formatting */
            Bool classic = cfgBool( doc, TidyVertSpace );
            if (classic && node->parent && node->parent->content != node)
            {
                TY_(PFlushLine)( doc, indent );
            }
        }

        PPrintTag( doc, mode, indent, node );

        if (node->next)
        {
          if (nodeIsPARAM(node) || nodeIsAREA(node))
              PCondFlushLine(doc, indent);
          else if ((nodeIsBR(node) && !(mode & PREFORMATTED))
                   || nodeIsHR(node))
              TY_(PFlushLine)(doc, indent);
        }
    }
    else /* some kind of container element */
    {
        if ( node->type == StartEndTag )
            node->type = StartTag;

        if ( node->tag && 
             (node->tag->parser == TY_(ParsePre) || nodeIsTEXTAREA(node)) )
        {
            Bool classic  = cfgBool( doc, TidyVertSpace );
            uint indprev = indent;
            PCondFlushLine( doc, indent );

            PCondFlushLine( doc, indent );

            /* insert extra newline for classic formatting */
            if (classic && node->parent && node->parent->content != node)
            {
                TY_(PFlushLine)( doc, indent );
            }
            PPrintTag( doc, mode, indent, node );

            indent = 0;
            TY_(PFlushLine)( doc, indent );

            for ( content = node->content; content; content = content->next )
            {
                TY_(PPrintTree)( doc, (mode | PREFORMATTED | NOWRAP),
                                 indent, content );
            }
            PCondFlushLine( doc, indent );
            indent = indprev;
            PPrintEndTag( doc, mode, indent, node );

            if ( cfgAutoBool(doc, TidyIndentContent) == TidyNoState
                 && node->next != NULL )
                TY_(PFlushLine)( doc, indent );
        }
        else if ( nodeIsSTYLE(node) || nodeIsSCRIPT(node) )
        {
            PPrintScriptStyle( doc, (mode | PREFORMATTED | NOWRAP | CDATA),
                               indent, node );
        }
        else if ( TY_(nodeCMIsInline)(node) )
        {
            if ( cfgBool(doc, TidyMakeClean) )
            {
                /* replace <nobr>...</nobr> by &nbsp; or &#160; etc. */
                if ( nodeIsNOBR(node) )
                {
                    for ( content = node->content;
                          content != NULL;
                          content = content->next)
                        TY_(PPrintTree)( doc, mode|NOWRAP, indent, content );
                    return;
                }
            }

            /* otherwise a normal inline element */
            PPrintTag( doc, mode, indent, node );

            /* indent content for SELECT, TEXTAREA, MAP, OBJECT and APPLET */
            if ( ShouldIndent(doc, node) )
            {
                indent += spaces;
                PCondFlushLine( doc, indent );

                for ( content = node->content;
                      content != NULL;
                      content = content->next )
                    TY_(PPrintTree)( doc, mode, indent, content );

                indent -= spaces;
                PCondFlushLine( doc, indent );
                /* PCondFlushLine( doc, indent ); */
            }
            else
            {
                for ( content = node->content;
                      content != NULL;
                      content = content->next )
                    TY_(PPrintTree)( doc, mode, indent, content );
            }
            PPrintEndTag( doc, mode, indent, node );
        }
        else /* other tags */
        {
            Bool indcont  = ( cfgAutoBool(doc, TidyIndentContent) != TidyNoState );
            Bool indsmart = ( cfgAutoBool(doc, TidyIndentContent) == TidyAutoState );
            Bool hideend  = cfgBool( doc, TidyHideEndTags );
            Bool classic  = cfgBool( doc, TidyVertSpace );
            uint contentIndent = indent;

            /* insert extra newline for classic formatting */
            if (classic && node->parent && node->parent->content != node && !nodeIsHTML(node))
            {
                TY_(PFlushLine)( doc, indent );
            }

            if ( ShouldIndent(doc, node) )
                contentIndent += spaces;

            PCondFlushLine( doc, indent );
            if ( indsmart && node->prev != NULL )
                TY_(PFlushLine)( doc, indent );

            /* do not omit elements with attributes */
            if ( !hideend || !TY_(nodeHasCM)(node, CM_OMITST) ||
                 node->attributes != NULL )
            {
                PPrintTag( doc, mode, indent, node );

                if ( ShouldIndent(doc, node) )
                {
                    /* fix for bug 530791, don't wrap after */
                    /* <li> if first child is text node     */
                    if (!(nodeIsLI(node) && TY_(nodeIsText)(node->content)))
                        PCondFlushLine( doc, contentIndent );
                }
                else if ( TY_(nodeHasCM)(node, CM_HTML) || nodeIsNOFRAMES(node) ||
                          (TY_(nodeHasCM)(node, CM_HEAD) && !nodeIsTITLE(node)) )
                    TY_(PFlushLine)( doc, contentIndent );
            }

            last = NULL;
            for ( content = node->content; content; content = content->next )
            {
                /* kludge for naked text before block level tag */
                if ( last && !indcont && TY_(nodeIsText)(last) &&
                     content->tag && !TY_(nodeHasCM)(content, CM_INLINE) )
                {
                    /* TY_(PFlushLine)(fout, indent); */
                    TY_(PFlushLine)( doc, contentIndent );
                }

                TY_(PPrintTree)( doc, mode, contentIndent, content );
                last = content;
            }

            /* don't flush line for td and th */
            if ( ShouldIndent(doc, node) ||
                 ( !hideend &&
                   ( TY_(nodeHasCM)(node, CM_HTML) || 
                     nodeIsNOFRAMES(node) ||
                     (TY_(nodeHasCM)(node, CM_HEAD) && !nodeIsTITLE(node))
                   )
                 )
               )
            {
                PCondFlushLine( doc, indent );
                if ( !hideend || !TY_(nodeHasCM)(node, CM_OPT) )
                {
                    PPrintEndTag( doc, mode, indent, node );
                    /* TY_(PFlushLine)( doc, indent ); */
                }
            }
            else
            {
                if ( !hideend || !TY_(nodeHasCM)(node, CM_OPT) )
                {
                    /* newline before endtag for classic formatting */
                    if ( classic && !HasMixedContent(node) )
                        TY_(PFlushLine)( doc, indent );
                    PPrintEndTag( doc, mode, indent, node );
                }
            }

            if (!indcont && !hideend && !nodeIsHTML(node) && !classic)
                TY_(PFlushLine)( doc, indent );
            else if (classic && node->next != NULL && TY_(nodeHasCM)(node, CM_LIST|CM_DEFLIST|CM_TABLE|CM_BLOCK/*|CM_HEADING*/))
                TY_(PFlushLine)( doc, indent );
        }
    }
}

void TY_(PPrintXMLTree)( TidyDocImpl* doc, uint mode, uint indent, Node *node )
{
    Bool xhtmlOut = cfgBool( doc, TidyXhtmlOut );
    if (node == NULL)
        return;

    if ( node->type == TextNode)
    {
        PPrintText( doc, mode, indent, node );
    }
    else if ( node->type == CommentTag )
    {
        PCondFlushLine( doc, indent );
        PPrintComment( doc, indent, node);
        /* PCondFlushLine( doc, 0 ); */
    }
    else if ( node->type == RootNode )
    {
        Node *content;
        for ( content = node->content;
              content != NULL;
              content = content->next )
           TY_(PPrintXMLTree)( doc, mode, indent, content );
    }
    else if ( node->type == DocTypeTag )
        PPrintDocType( doc, indent, node );
    else if ( node->type == ProcInsTag )
        PPrintPI( doc, indent, node );
    else if ( node->type == XmlDecl )
        PPrintXmlDecl( doc, indent, node );
    else if ( node->type == CDATATag )
        PPrintCDATA( doc, indent, node );
    else if ( node->type == SectionTag )
        PPrintSection( doc, indent, node );
    else if ( node->type == AspTag )
        PPrintAsp( doc, indent, node );
    else if ( node->type == JsteTag)
        PPrintJste( doc, indent, node );
    else if ( node->type == PhpTag)
        PPrintPhp( doc, indent, node );
    else if ( TY_(nodeHasCM)(node, CM_EMPTY) ||
              (node->type == StartEndTag && !xhtmlOut) )
    {
        PCondFlushLine( doc, indent );
        PPrintTag( doc, mode, indent, node );
        /* TY_(PFlushLine)( doc, indent ); */
    }
    else /* some kind of container element */
    {
        uint spaces = cfg( doc, TidyIndentSpaces );
        Node *content;
        Bool mixed = no;
        uint cindent;

        for ( content = node->content; content; content = content->next )
        {
            if ( TY_(nodeIsText)(content) )
            {
                mixed = yes;
                break;
            }
        }

        PCondFlushLine( doc, indent );

        if ( TY_(XMLPreserveWhiteSpace)(doc, node) )
        {
            indent = 0;
            mixed = no;
            cindent = 0;
        }
        else if (mixed)
            cindent = indent;
        else
            cindent = indent + spaces;

        PPrintTag( doc, mode, indent, node );
        if ( !mixed && node->content )
            TY_(PFlushLine)( doc, cindent );
 
        for ( content = node->content; content; content = content->next )
            TY_(PPrintXMLTree)( doc, mode, cindent, content );

        if ( !mixed && node->content )
            PCondFlushLine( doc, indent );

        PPrintEndTag( doc, mode, indent, node );
        /* PCondFlushLine( doc, indent ); */
    }
}

/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
