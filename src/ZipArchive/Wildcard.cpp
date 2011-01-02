////////////////////////////////////////////////////////////////////////////////
// This source file is part of the ZipArchive library source distribution and
// is Copyrighted 2000 - 2010 by Artpol Software - Tadeusz Dracz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// For the licensing details refer to the License.txt file.
//
// Web Site: http://www.artpol-software.com
////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Wildcard.h"

namespace ZipArchiveLib
{

bool CWildcard::IsPatternValid(LPCTSTR lpszPattern, int* iErrorType)
{
	try
	{
		/* loop through pattern to EOS */
		while (*lpszPattern)
		{
			/* determine pattern type */
			switch (*lpszPattern)
			{
				/* check literal escape, it cannot be at end of pattern */
			case _T('\\'):
				if (!*++lpszPattern)
					throw patternEsc;
				lpszPattern++;
				break;
				
				/* the [..] construct must be well formed */
			case _T('['):
				lpszPattern++;
				
				/* if the next character is ']' then bad pattern */
				if (*lpszPattern == _T(']'))
					throw patternEmpty;
				
				/* if end of pattern here then bad pattern */
				if (!*lpszPattern)
					throw patternClose;
				
				/* loop to end of [..] construct */
				while (*lpszPattern != _T(']'))
				{
					/* check for literal escape */
					if (*lpszPattern == _T('\\'))
					{
						lpszPattern++;
						
						/* if end of pattern here then bad pattern */
						if (!*lpszPattern++)
							throw patternEsc;
					}
					else  lpszPattern++;
					
					/* if end of pattern here then bad pattern */
					if (!*lpszPattern)
						throw patternClose;
					
					/* if this a range */
					if (*lpszPattern == _T('-'))
					{
						/* we must have an end of range */
						if (!*++lpszPattern || *lpszPattern == ']')
							throw patternRange;
						else
						{
							
							/* check for literal escape */
							if (*lpszPattern == _T('\\'))
								lpszPattern++;
							
								/* if end of pattern here
							then bad pattern           */
							if (!*lpszPattern++)
								throw patternEsc;
						}
					}
				}
				break;
				
				/* all other characters are valid pattern elements */
			case '*':
			case '?':
			default:
				lpszPattern++;                              /* "normal" character */
				break;
			}
		}
		throw patternValid;
	}
	catch (int i)
	{
		if (iErrorType)
			*iErrorType = i;
		return i == patternValid;	
	}
	
	
}

bool CWildcard::IsPattern(LPCTSTR lpszPattern)
{
	while (*lpszPattern)
	{
        switch (*lpszPattern++)
        {
        case _T('?'):
        case _T('*'):
        case _T('['):
        case _T('\\'):
			return true;
        }
	}
	return false;
	
}

bool CWildcard::IsMatch(LPCTSTR lpszText, int *iRetCode)
{
	CZipString sz;
	if (!m_bCaseSensitive)
	{
		sz = lpszText;
		sz.MakeLower();
		lpszText = (LPCTSTR)sz;
	}
	int i = Match((LPCTSTR)m_szPattern, lpszText);
	if (iRetCode)
		*iRetCode = i;
	return i == matchValid;
}

int CWildcard::MatchAfterStar(LPCTSTR p, LPCTSTR t)
{
      int iMatch = matchNone;
      TCHAR nextp;

      /* pass over existing ? and * in pattern */

      while ( *p == _T('?') || *p == _T('*') )
      {
            /* take one char for each ? and + */

            if (*p == _T('?'))
            {
                  /* if end of text then no match */
                  if (!*t++)
                        return matchAbort;
            }

            /* move to next char in pattern */

            p++;
      }

      /* if end of pattern we have matched regardless of text left */

      if (!*p)
            return matchValid;

      /* get the next character to match which must be a literal or '[' */

      nextp = *p;
      if (nextp == _T('\\'))
      {
            nextp = p[1];

            /* if end of text then we have a bad pattern */

            if (!nextp)
                  return matchPattern;
      }

      /* Continue until we run out of text or definite result seen */

      do
      {
            /* a precondition for matching is that the next character
               in the pattern match the next character in the text or that
               the next pattern char is the beginning of a range.  Increment
               text pointer as we go here */

            if (nextp == *t || nextp == _T('['))
                  iMatch = Match(p, t);

			/* try finding another precondition */
			if (iMatch == matchPattern)
				iMatch = matchNone;
            /* if the end of text is reached then no iMatch */

            if (!*t++)
                  iMatch = matchAbort;

      } while ( iMatch != matchValid && 
                iMatch != matchAbort);

      /* return result */

      return iMatch;
}


int CWildcard::Match(LPCTSTR lpszPattern, LPCTSTR lpszText)
{
	TCHAR range_start, range_end;  /* start and end in range */	
	bool bInvert;             /* is this [..] or [!..] */
	bool bMemberMatch;       /* have I matched the [..] construct? */
	bool bLoop;               /* should I terminate? */
	
	for ( ; *lpszPattern; lpszPattern++, lpszText++)
	{
	/* if this is the end of the text
		then this is the end of the match */
		
		if (!*lpszText)
		{
			if ( *lpszPattern == _T('*') && *++lpszPattern == _T('\0') )
				return matchValid;
			else
				return matchAbort;
		}
		
		/* determine and react to pattern type */
		
		switch (*lpszPattern)
		{
		case _T('?'):                     /* single any character match */
			break;
			
		case _T('*'):                     /* multiple any character match */
			return MatchAfterStar (lpszPattern, lpszText);
			
			/* [..] construct, single member/exclusion character match */
		case _T('['):
			{
				/* move to beginning of range */
				
				lpszPattern++;
				
				/* check if this is a member match or exclusion match */
				
				bInvert = false;
				if (*lpszPattern == _T('!') || *lpszPattern == _T('^'))
				{
					bInvert = true;
					lpszPattern++;
				}
				
				/* if closing bracket here or at range start then we have a
				malformed pattern */
				
				if (*lpszPattern == _T(']'))
					return matchPattern;
				
				bMemberMatch = false;
				bLoop = true;
				
				while (bLoop)
				{
					/* if end of construct then bLoop is done */
					
					if (*lpszPattern == _T(']'))
					{
						bLoop = false;
						continue;
					}
					
					/* matching a '!', '^', '-', '\' or a ']' */
					
					if (*lpszPattern == _T('\\'))
						range_start = range_end = *++lpszPattern;
					else  
						range_start = range_end = *lpszPattern;
					
					/* if end of pattern then bad pattern (Missing ']') */
					
					if (!*lpszPattern)
						return matchPattern;
					
					/* check for range bar */
					if (*++lpszPattern == _T('-'))
					{
						/* get the range end */
						
						range_end = *++lpszPattern;
						
						/* if end of pattern or construct
						then bad pattern */
						
						if (range_end == _T('\0') || range_end == _T(']'))
							return matchPattern;
						/* special character range end */
						if (range_end == _T('\\'))
						{
							range_end = *++lpszPattern;
							
							/* if end of text then
							we have a bad pattern */
							if (!range_end)
								return matchPattern;
						}
						
						/* move just beyond this range */
						lpszPattern++;
					}
					
					/* if the text character is in range then match found.
					make sure the range letters have the proper
					relationship to one another before comparison */
					
					if (range_start < range_end)
					{
						if (*lpszText >= range_start && *lpszText <= range_end)
						{
							bMemberMatch = true;
							bLoop = false;
						}
					}
					else
					{
						if (*lpszText >= range_end && *lpszText <= range_start)
						{
							bMemberMatch = true;
							bLoop = false;
						}
					}
				}
				
				/* if there was a match in an exclusion set then no match */
				/* if there was no match in a member set then no match */
				
				if ((bInvert && bMemberMatch) || !(bInvert || bMemberMatch))
					return matchRange;
				
				/* if this is not an exclusion then skip the rest of
				the [...] construct that already matched. */
				
				if (bMemberMatch)
				{
					while (*lpszPattern != _T(']'))
					{
						/* bad pattern (Missing ']') */
						if (!*lpszPattern)
							return matchPattern;
						
						/* skip exact match */
						if (*lpszPattern == _T('\\'))
						{
							lpszPattern++;
							
							/* if end of text then
							we have a bad pattern */
							
							if (!*lpszPattern)
								return matchPattern;
						}
						
						/* move to next pattern char */
						
						lpszPattern++;
					}
				}
				break;
			}
			case _T('\\'):  /* next character is quoted and must match exactly */
				
				/* move pattern pointer to quoted char and fall through */
				
				lpszPattern++;
				
				/* if end of text then we have a bad pattern */
				
				if (!*lpszPattern)
					return matchPattern;
				
				/* must match this character exactly */
				
			default:
				if (*lpszPattern != *lpszText)
					return matchPattern;
			}
	  }
	  /* if end of text not reached then the pattern fails */
	
		if (*lpszText)
			return matchEnd;
		else  
			return matchValid;
}


} // namespace
