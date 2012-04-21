#ifndef __WIN32TC_H__
#define __WIN32TC_H__
#ifdef TIDY_WIN32_MLANG_SUPPORT

/* win32tc.h -- Interface to Win32 transcoding routines

   (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
   See tidy.h for the copyright notice.

   $Id: win32tc.h,v 1.3 2006/12/29 16:31:09 arnaud02 Exp $
*/

uint TY_(Win32MLangGetCPFromName)(TidyAllocator *allocator,ctmbstr encoding);
Bool TY_(Win32MLangInitInputTranscoder)(StreamIn * in, uint wincp);
void TY_(Win32MLangUninitInputTranscoder)(StreamIn * in);
int TY_(Win32MLangGetChar)(byte firstByte, StreamIn * in, uint * bytesRead);

#endif /* TIDY_WIN32_MLANG_SUPPORT */
#endif /* __WIN32TC_H__ */
