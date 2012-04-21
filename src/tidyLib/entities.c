/* entities.c -- recognize HTML ISO entities

  (c) 1998-2008 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: hoehrmann $ 
    $Date: 2008/08/09 11:55:27 $ 
    $Revision: 1.19 $ 

  Entity handling can be static because there are no config or
  document-specific values.  Lookup table is 100% defined at 
  compile time.

*/

#include <stdio.h>
#include "entities.h"
#include "tidy-int.h"
#include "tmbstr.h"

struct _entity;
typedef struct _entity entity;

struct _entity
{
    ctmbstr name;
    uint    versions;
    uint    code;
};


static const entity entities[] =
{
    /*
    ** Markup pre-defined character entities
    */
    { "quot",    VERS_ALL|VERS_XML,    34 },
    { "amp",     VERS_ALL|VERS_XML,    38 },
    { "apos",    VERS_FROM40|VERS_XML, 39 },
    { "lt",      VERS_ALL|VERS_XML,    60 },
    { "gt",      VERS_ALL|VERS_XML,    62 },

    /*
    ** Latin-1 character entities
    */
    { "nbsp",     VERS_ALL,      160 },
    { "iexcl",    VERS_ALL,      161 },
    { "cent",     VERS_ALL,      162 },
    { "pound",    VERS_ALL,      163 },
    { "curren",   VERS_ALL,      164 },
    { "yen",      VERS_ALL,      165 },
    { "brvbar",   VERS_ALL,      166 },
    { "sect",     VERS_ALL,      167 },
    { "uml",      VERS_ALL,      168 },
    { "copy",     VERS_ALL,      169 },
    { "ordf",     VERS_ALL,      170 },
    { "laquo",    VERS_ALL,      171 },
    { "not",      VERS_ALL,      172 },
    { "shy",      VERS_ALL,      173 },
    { "reg",      VERS_ALL,      174 },
    { "macr",     VERS_ALL,      175 },
    { "deg",      VERS_ALL,      176 },
    { "plusmn",   VERS_ALL,      177 },
    { "sup2",     VERS_ALL,      178 },
    { "sup3",     VERS_ALL,      179 },
    { "acute",    VERS_ALL,      180 },
    { "micro",    VERS_ALL,      181 },
    { "para",     VERS_ALL,      182 },
    { "middot",   VERS_ALL,      183 },
    { "cedil",    VERS_ALL,      184 },
    { "sup1",     VERS_ALL,      185 },
    { "ordm",     VERS_ALL,      186 },
    { "raquo",    VERS_ALL,      187 },
    { "frac14",   VERS_ALL,      188 },
    { "frac12",   VERS_ALL,      189 },
    { "frac34",   VERS_ALL,      190 },
    { "iquest",   VERS_ALL,      191 },
    { "Agrave",   VERS_ALL,      192 },
    { "Aacute",   VERS_ALL,      193 },
    { "Acirc",    VERS_ALL,      194 },
    { "Atilde",   VERS_ALL,      195 },
    { "Auml",     VERS_ALL,      196 },
    { "Aring",    VERS_ALL,      197 },
    { "AElig",    VERS_ALL,      198 },
    { "Ccedil",   VERS_ALL,      199 },
    { "Egrave",   VERS_ALL,      200 },
    { "Eacute",   VERS_ALL,      201 },
    { "Ecirc",    VERS_ALL,      202 },
    { "Euml",     VERS_ALL,      203 },
    { "Igrave",   VERS_ALL,      204 },
    { "Iacute",   VERS_ALL,      205 },
    { "Icirc",    VERS_ALL,      206 },
    { "Iuml",     VERS_ALL,      207 },
    { "ETH",      VERS_ALL,      208 },
    { "Ntilde",   VERS_ALL,      209 },
    { "Ograve",   VERS_ALL,      210 },
    { "Oacute",   VERS_ALL,      211 },
    { "Ocirc",    VERS_ALL,      212 },
    { "Otilde",   VERS_ALL,      213 },
    { "Ouml",     VERS_ALL,      214 },
    { "times",    VERS_ALL,      215 },
    { "Oslash",   VERS_ALL,      216 },
    { "Ugrave",   VERS_ALL,      217 },
    { "Uacute",   VERS_ALL,      218 },
    { "Ucirc",    VERS_ALL,      219 },
    { "Uuml",     VERS_ALL,      220 },
    { "Yacute",   VERS_ALL,      221 },
    { "THORN",    VERS_ALL,      222 },
    { "szlig",    VERS_ALL,      223 },
    { "agrave",   VERS_ALL,      224 },
    { "aacute",   VERS_ALL,      225 },
    { "acirc",    VERS_ALL,      226 },
    { "atilde",   VERS_ALL,      227 },
    { "auml",     VERS_ALL,      228 },
    { "aring",    VERS_ALL,      229 },
    { "aelig",    VERS_ALL,      230 },
    { "ccedil",   VERS_ALL,      231 },
    { "egrave",   VERS_ALL,      232 },
    { "eacute",   VERS_ALL,      233 },
    { "ecirc",    VERS_ALL,      234 },
    { "euml",     VERS_ALL,      235 },
    { "igrave",   VERS_ALL,      236 },
    { "iacute",   VERS_ALL,      237 },
    { "icirc",    VERS_ALL,      238 },
    { "iuml",     VERS_ALL,      239 },
    { "eth",      VERS_ALL,      240 },
    { "ntilde",   VERS_ALL,      241 },
    { "ograve",   VERS_ALL,      242 },
    { "oacute",   VERS_ALL,      243 },
    { "ocirc",    VERS_ALL,      244 },
    { "otilde",   VERS_ALL,      245 },
    { "ouml",     VERS_ALL,      246 },
    { "divide",   VERS_ALL,      247 },
    { "oslash",   VERS_ALL,      248 },
    { "ugrave",   VERS_ALL,      249 },
    { "uacute",   VERS_ALL,      250 },
    { "ucirc",    VERS_ALL,      251 },
    { "uuml",     VERS_ALL,      252 },
    { "yacute",   VERS_ALL,      253 },
    { "thorn",    VERS_ALL,      254 },
    { "yuml",     VERS_ALL,      255 },

    /*
    ** Extended Entities defined in HTML 4: Symbols 
    */
    { "fnof",     VERS_FROM40,   402 },
    { "Alpha",    VERS_FROM40,   913 },
    { "Beta",     VERS_FROM40,   914 },
    { "Gamma",    VERS_FROM40,   915 },
    { "Delta",    VERS_FROM40,   916 },
    { "Epsilon",  VERS_FROM40,   917 },
    { "Zeta",     VERS_FROM40,   918 },
    { "Eta",      VERS_FROM40,   919 },
    { "Theta",    VERS_FROM40,   920 },
    { "Iota",     VERS_FROM40,   921 },
    { "Kappa",    VERS_FROM40,   922 },
    { "Lambda",   VERS_FROM40,   923 },
    { "Mu",       VERS_FROM40,   924 },
    { "Nu",       VERS_FROM40,   925 },
    { "Xi",       VERS_FROM40,   926 },
    { "Omicron",  VERS_FROM40,   927 },
    { "Pi",       VERS_FROM40,   928 },
    { "Rho",      VERS_FROM40,   929 },
    { "Sigma",    VERS_FROM40,   931 },
    { "Tau",      VERS_FROM40,   932 },
    { "Upsilon",  VERS_FROM40,   933 },
    { "Phi",      VERS_FROM40,   934 },
    { "Chi",      VERS_FROM40,   935 },
    { "Psi",      VERS_FROM40,   936 },
    { "Omega",    VERS_FROM40,   937 },
    { "alpha",    VERS_FROM40,   945 },
    { "beta",     VERS_FROM40,   946 },
    { "gamma",    VERS_FROM40,   947 },
    { "delta",    VERS_FROM40,   948 },
    { "epsilon",  VERS_FROM40,   949 },
    { "zeta",     VERS_FROM40,   950 },
    { "eta",      VERS_FROM40,   951 },
    { "theta",    VERS_FROM40,   952 },
    { "iota",     VERS_FROM40,   953 },
    { "kappa",    VERS_FROM40,   954 },
    { "lambda",   VERS_FROM40,   955 },
    { "mu",       VERS_FROM40,   956 },
    { "nu",       VERS_FROM40,   957 },
    { "xi",       VERS_FROM40,   958 },
    { "omicron",  VERS_FROM40,   959 },
    { "pi",       VERS_FROM40,   960 },
    { "rho",      VERS_FROM40,   961 },
    { "sigmaf",   VERS_FROM40,   962 },
    { "sigma",    VERS_FROM40,   963 },
    { "tau",      VERS_FROM40,   964 },
    { "upsilon",  VERS_FROM40,   965 },
    { "phi",      VERS_FROM40,   966 },
    { "chi",      VERS_FROM40,   967 },
    { "psi",      VERS_FROM40,   968 },
    { "omega",    VERS_FROM40,   969 },
    { "thetasym", VERS_FROM40,   977 },
    { "upsih",    VERS_FROM40,   978 },
    { "piv",      VERS_FROM40,   982 },
    { "bull",     VERS_FROM40,  8226 },
    { "hellip",   VERS_FROM40,  8230 },
    { "prime",    VERS_FROM40,  8242 },
    { "Prime",    VERS_FROM40,  8243 },
    { "oline",    VERS_FROM40,  8254 },
    { "frasl",    VERS_FROM40,  8260 },
    { "weierp",   VERS_FROM40,  8472 },
    { "image",    VERS_FROM40,  8465 },
    { "real",     VERS_FROM40,  8476 },
    { "trade",    VERS_FROM40,  8482 },
    { "alefsym",  VERS_FROM40,  8501 },
    { "larr",     VERS_FROM40,  8592 },
    { "uarr",     VERS_FROM40,  8593 },
    { "rarr",     VERS_FROM40,  8594 },
    { "darr",     VERS_FROM40,  8595 },
    { "harr",     VERS_FROM40,  8596 },
    { "crarr",    VERS_FROM40,  8629 },
    { "lArr",     VERS_FROM40,  8656 },
    { "uArr",     VERS_FROM40,  8657 },
    { "rArr",     VERS_FROM40,  8658 },
    { "dArr",     VERS_FROM40,  8659 },
    { "hArr",     VERS_FROM40,  8660 },
    { "forall",   VERS_FROM40,  8704 },
    { "part",     VERS_FROM40,  8706 },
    { "exist",    VERS_FROM40,  8707 },
    { "empty",    VERS_FROM40,  8709 },
    { "nabla",    VERS_FROM40,  8711 },
    { "isin",     VERS_FROM40,  8712 },
    { "notin",    VERS_FROM40,  8713 },
    { "ni",       VERS_FROM40,  8715 },
    { "prod",     VERS_FROM40,  8719 },
    { "sum",      VERS_FROM40,  8721 },
    { "minus",    VERS_FROM40,  8722 },
    { "lowast",   VERS_FROM40,  8727 },
    { "radic",    VERS_FROM40,  8730 },
    { "prop",     VERS_FROM40,  8733 },
    { "infin",    VERS_FROM40,  8734 },
    { "ang",      VERS_FROM40,  8736 },
    { "and",      VERS_FROM40,  8743 },
    { "or",       VERS_FROM40,  8744 },
    { "cap",      VERS_FROM40,  8745 },
    { "cup",      VERS_FROM40,  8746 },
    { "int",      VERS_FROM40,  8747 },
    { "there4",   VERS_FROM40,  8756 },
    { "sim",      VERS_FROM40,  8764 },
    { "cong",     VERS_FROM40,  8773 },
    { "asymp",    VERS_FROM40,  8776 },
    { "ne",       VERS_FROM40,  8800 },
    { "equiv",    VERS_FROM40,  8801 },
    { "le",       VERS_FROM40,  8804 },
    { "ge",       VERS_FROM40,  8805 },
    { "sub",      VERS_FROM40,  8834 },
    { "sup",      VERS_FROM40,  8835 },
    { "nsub",     VERS_FROM40,  8836 },
    { "sube",     VERS_FROM40,  8838 },
    { "supe",     VERS_FROM40,  8839 },
    { "oplus",    VERS_FROM40,  8853 },
    { "otimes",   VERS_FROM40,  8855 },
    { "perp",     VERS_FROM40,  8869 },
    { "sdot",     VERS_FROM40,  8901 },
    { "lceil",    VERS_FROM40,  8968 },
    { "rceil",    VERS_FROM40,  8969 },
    { "lfloor",   VERS_FROM40,  8970 },
    { "rfloor",   VERS_FROM40,  8971 },
    { "lang",     VERS_FROM40,  9001 },
    { "rang",     VERS_FROM40,  9002 },
    { "loz",      VERS_FROM40,  9674 },
    { "spades",   VERS_FROM40,  9824 },
    { "clubs",    VERS_FROM40,  9827 },
    { "hearts",   VERS_FROM40,  9829 },
    { "diams",    VERS_FROM40,  9830 },

    /*
    ** Extended Entities defined in HTML 4: Special (less Markup at top)
    */
    { "OElig",    VERS_FROM40,   338 },
    { "oelig",    VERS_FROM40,   339 },
    { "Scaron",   VERS_FROM40,   352 },
    { "scaron",   VERS_FROM40,   353 },
    { "Yuml",     VERS_FROM40,   376 },
    { "circ",     VERS_FROM40,   710 },
    { "tilde",    VERS_FROM40,   732 },
    { "ensp",     VERS_FROM40,  8194 },
    { "emsp",     VERS_FROM40,  8195 },
    { "thinsp",   VERS_FROM40,  8201 },
    { "zwnj",     VERS_FROM40,  8204 },
    { "zwj",      VERS_FROM40,  8205 },
    { "lrm",      VERS_FROM40,  8206 },
    { "rlm",      VERS_FROM40,  8207 },
    { "ndash",    VERS_FROM40,  8211 },
    { "mdash",    VERS_FROM40,  8212 },
    { "lsquo",    VERS_FROM40,  8216 },
    { "rsquo",    VERS_FROM40,  8217 },
    { "sbquo",    VERS_FROM40,  8218 },
    { "ldquo",    VERS_FROM40,  8220 },
    { "rdquo",    VERS_FROM40,  8221 },
    { "bdquo",    VERS_FROM40,  8222 },
    { "dagger",   VERS_FROM40,  8224 },
    { "Dagger",   VERS_FROM40,  8225 },
    { "permil",   VERS_FROM40,  8240 },
    { "lsaquo",   VERS_FROM40,  8249 },
    { "rsaquo",   VERS_FROM40,  8250 },
    { "euro",     VERS_FROM40,  8364 },
    { NULL,       VERS_UNKNOWN, 0 }
};


/* Pure static implementation.  Trades off lookup speed
** for faster setup time (well, none actually).
** Optimization of comparing 1st character buys enough
** speed that hash doesn't improve things without > 500
** items in list.
*/
static const entity* entitiesLookup( ctmbstr s )
{
    tmbchar ch = (tmbchar)( s ? *s : 0 );
    const entity *np;
    for ( np = entities; ch && np && np->name; ++np )
        if ( ch == *np->name && TY_(tmbstrcmp)(s, np->name) == 0 )
            return np;
    return NULL;
}

#if 0
/* entity starting with "&" returns zero on error */
uint EntityCode( ctmbstr name, uint versions )
{
    const entity* np;
    assert( name && name[0] == '&' );

    /* numeric entitity: name = "&#" followed by number */
    if ( name[1] == '#' )
    {
        uint c = 0;  /* zero on missing/bad number */
        Bool isXml = ( (versions & VERS_XML) == VERS_XML );

        /* 'x' prefix denotes hexadecimal number format */
        if ( name[2] == 'x' || (!isXml && name[2] == 'X') )
            sscanf( name+3, "%x", &c );
        else
            sscanf( name+2, "%u", &c );

        return (uint) c;
    }

   /* Named entity: name ="&" followed by a name */
    if ( NULL != (np = entitiesLookup(name+1)) )
    {
        /* Only recognize entity name if version supports it.  */
        if ( np->versions & versions )
            return np->code;
    }

    return 0;   /* zero signifies unknown entity name */
}
#endif

Bool TY_(EntityInfo)( ctmbstr name, Bool isXml, uint* code, uint* versions )
{
    const entity* np;
    assert( name && name[0] == '&' );
    assert( code != NULL );
    assert( versions != NULL );

    /* numeric entitity: name = "&#" followed by number */
    if ( name[1] == '#' )
    {
        uint c = 0;  /* zero on missing/bad number */

        /* 'x' prefix denotes hexadecimal number format */
        if ( name[2] == 'x' || (!isXml && name[2] == 'X') )
            sscanf( name+3, "%x", &c );
        else
            sscanf( name+2, "%u", &c );

        *code = c;
        *versions = VERS_ALL;
        return yes;
    }

    /* Named entity: name ="&" followed by a name */
    if ( NULL != (np = entitiesLookup(name+1)) )
    {
        *code = np->code;
        *versions = np->versions;
        return yes;
    }

    *code = 0;
    *versions = ( isXml ? VERS_XML : VERS_PROPRIETARY );
    return no;
}


ctmbstr TY_(EntityName)( uint ch, uint versions )
{
    ctmbstr entnam = NULL;
    const entity *ep;

    for ( ep = entities; ep->name != NULL; ++ep )
    {
        if ( ep->code == ch )
        {
            if ( ep->versions & versions )
                entnam = ep->name;
            break; /* Found code. Stop search. */
        }
    }
    return entnam;
}

/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
