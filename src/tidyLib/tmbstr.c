/* tmbstr.c -- Tidy string utility functions

  (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2006/12/29 16:31:08 $ 
    $Revision: 1.13 $ 

*/

#include "forward.h"
#include "tmbstr.h"
#include "lexer.h"

/* like strdup but using an allocator */
tmbstr TY_(tmbstrdup)( TidyAllocator *allocator, ctmbstr str )
{
    tmbstr s = NULL;
    if ( str )
    {
        uint len = TY_(tmbstrlen)( str );
        tmbstr cp = s = (tmbstr) TidyAlloc( allocator, 1+len );
        while ( 0 != (*cp++ = *str++) )
            /**/;
    }
    return s;
}

/* like strndup but using an allocator */
tmbstr TY_(tmbstrndup)( TidyAllocator *allocator, ctmbstr str, uint len )
{
    tmbstr s = NULL;
    if ( str && len > 0 )
    {
        tmbstr cp = s = (tmbstr) TidyAlloc( allocator, 1+len );
        while ( len-- > 0 &&  (*cp++ = *str++) )
          /**/;
        *cp = 0;
    }
    return s;
}

/* exactly same as strncpy */
uint TY_(tmbstrncpy)( tmbstr s1, ctmbstr s2, uint size )
{
    if ( s1 != NULL && s2 != NULL )
    {
        tmbstr cp = s1;
        while ( *s2 && --size )  /* Predecrement: reserve byte */
            *cp++ = *s2++;       /* for NULL terminator. */
        *cp = 0;
    }
    return size;
}

/* Allows expressions like:  cp += tmbstrcpy( cp, "joebob" );
*/
uint TY_(tmbstrcpy)( tmbstr s1, ctmbstr s2 )
{
    uint ncpy = 0;
    while (0 != (*s1++ = *s2++) )
        ++ncpy;
    return ncpy;
}

/* Allows expressions like:  cp += tmbstrcat( cp, "joebob" );
*/
uint TY_(tmbstrcat)( tmbstr s1, ctmbstr s2 )
{
    uint ncpy = 0;
    while ( *s1 )
        ++s1;

    while (0 != (*s1++ = *s2++) )
        ++ncpy;
    return ncpy;
}

/* exactly same as strcmp */
int TY_(tmbstrcmp)( ctmbstr s1, ctmbstr s2 )
{
    int c;
    while ((c = *s1) == *s2)
    {
        if (c == '\0')
            return 0;

        ++s1;
        ++s2;
    }

    return (*s1 > *s2 ? 1 : -1);
}

/* returns byte count, not char count */
uint TY_(tmbstrlen)( ctmbstr str )
{
    uint len = 0;
    if ( str ) 
    {
        while ( *str++ )
            ++len;
    }
    return len;
}

/*
 MS C 4.2 doesn't include strcasecmp.
 Note that tolower and toupper won't
 work on chars > 127.

 Neither does ToLower()!
*/
int TY_(tmbstrcasecmp)( ctmbstr s1, ctmbstr s2 )
{
    uint c;

    while (c = (uint)(*s1), TY_(ToLower)(c) == TY_(ToLower)((uint)(*s2)))
    {
        if (c == '\0')
            return 0;

        ++s1;
        ++s2;
    }

    return (*s1 > *s2 ? 1 : -1);
}

int TY_(tmbstrncmp)( ctmbstr s1, ctmbstr s2, uint n )
{
    uint c;

    while ((c = (byte)*s1) == (byte)*s2)
    {
        if (c == '\0')
            return 0;

        if (n == 0)
            return 0;

        ++s1;
        ++s2;
        --n;
    }

    if (n == 0)
        return 0;

    return (*s1 > *s2 ? 1 : -1);
}

int TY_(tmbstrncasecmp)( ctmbstr s1, ctmbstr s2, uint n )
{
    uint c;

    while (c = (uint)(*s1), TY_(ToLower)(c) == TY_(ToLower)((uint)(*s2)))
    {
        if (c == '\0')
            return 0;

        if (n == 0)
            return 0;

        ++s1;
        ++s2;
        --n;
    }

    if (n == 0)
        return 0;

    return (*s1 > *s2 ? 1 : -1);
}

#if 0
/* return offset of cc from beginning of s1,
** -1 if not found.
*/
int TY_(tmbstrnchr)( ctmbstr s1, uint maxlen, tmbchar cc )
{
    int i;
    ctmbstr cp = s1;

    for ( i = 0; (uint)i < maxlen; ++i, ++cp )
    {
        if ( *cp == cc )
            return i;
    }

    return -1;
}
#endif

ctmbstr TY_(tmbsubstrn)( ctmbstr s1, uint len1, ctmbstr s2 )
{
    uint len2 = TY_(tmbstrlen)(s2);
    int ix, diff = len1 - len2;

    for ( ix = 0; ix <= diff; ++ix )
    {
        if ( TY_(tmbstrncmp)(s1+ix, s2, len2) == 0 )
            return (ctmbstr) s1+ix;
    }
    return NULL;
}

#if 0
ctmbstr TY_(tmbsubstrncase)( ctmbstr s1, uint len1, ctmbstr s2 )
{
    uint len2 = TY_(tmbstrlen)(s2);
    int ix, diff = len1 - len2;

    for ( ix = 0; ix <= diff; ++ix )
    {
        if ( TY_(tmbstrncasecmp)(s1+ix, s2, len2) == 0 )
            return (ctmbstr) s1+ix;
    }
    return NULL;
}
#endif

ctmbstr TY_(tmbsubstr)( ctmbstr s1, ctmbstr s2 )
{
    uint len1 = TY_(tmbstrlen)(s1), len2 = TY_(tmbstrlen)(s2);
    int ix, diff = len1 - len2;

    for ( ix = 0; ix <= diff; ++ix )
    {
        if ( TY_(tmbstrncasecmp)(s1+ix, s2, len2) == 0 )
            return (ctmbstr) s1+ix;
    }
    return NULL;
}

/* Transform ASCII chars in string to lower case */
tmbstr TY_(tmbstrtolower)( tmbstr s )
{
    tmbstr cp;
    for ( cp=s; *cp; ++cp )
        *cp = (tmbchar) TY_(ToLower)( *cp );
    return s;
}

/* Transform ASCII chars in string to upper case */
tmbstr TY_(tmbstrtoupper)(tmbstr s)
{
    tmbstr cp;

    for (cp = s; *cp; ++cp)
        *cp = (tmbchar)TY_(ToUpper)(*cp);

    return s;
}

#if 0
Bool TY_(tmbsamefile)( ctmbstr filename1, ctmbstr filename2 )
{
#if FILENAMES_CASE_SENSITIVE
    return ( TY_(tmbstrcmp)( filename1, filename2 ) == 0 );
#else
    return ( TY_(tmbstrcasecmp)( filename1, filename2 ) == 0 );
#endif
}
#endif

int TY_(tmbvsnprintf)(tmbstr buffer, size_t count, ctmbstr format, va_list args)
{
    int retval;
#if HAS_VSNPRINTF
    retval = vsnprintf(buffer, count - 1, format, args);
    /* todo: conditionally null-terminate the string? */
    buffer[count - 1] = 0;
#else
    retval = vsprintf(buffer, format, args);
#endif /* HAS_VSNPRINTF */
    return retval;
}

int TY_(tmbsnprintf)(tmbstr buffer, size_t count, ctmbstr format, ...)
{
    int retval;
    va_list args;
    va_start(args, format);
#if HAS_VSNPRINTF
    retval = vsnprintf(buffer, count - 1, format, args);
    /* todo: conditionally null-terminate the string? */
    buffer[count - 1] = 0;
#else
    retval = vsprintf(buffer, format, args);
#endif /* HAS_VSNPRINTF */
    va_end(args);
    return retval;
}

/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
