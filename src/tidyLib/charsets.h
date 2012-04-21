/* charsets.h -- character set information and mappings

  (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  $Id: charsets.h,v 1.2 2006/09/12 15:14:44 arnaud02 Exp $
*/

uint TY_(GetEncodingIdFromName)(ctmbstr name);
uint TY_(GetEncodingIdFromCodePage)(uint cp);
uint TY_(GetEncodingCodePageFromName)(ctmbstr name);
uint TY_(GetEncodingCodePageFromId)(uint id);
ctmbstr TY_(GetEncodingNameFromId)(uint id);
ctmbstr TY_(GetEncodingNameFromCodePage)(uint cp);
