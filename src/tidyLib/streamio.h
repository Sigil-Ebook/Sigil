#ifndef __STREAMIO_H__
#define __STREAMIO_H__

/* streamio.h -- handles character stream I/O

  (c) 1998-2007 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2007/07/22 09:33:26 $ 
    $Revision: 1.21 $ 

  Wrapper around Tidy input source and output sink
  that calls appropriate interfaces, and applies 
  necessary char encoding transformations: to/from
  ISO-10646 and/or UTF-8.

*/

#include "forward.h"
#include "buffio.h"
#include "fileio.h"

#ifdef __cplusplus
extern "C"
{
#endif
typedef enum
{
  FileIO,
  BufferIO,
  UserIO
} IOType;

/* states for ISO 2022

 A document in ISO-2022 based encoding uses some ESC sequences called
 "designator" to switch character sets. The designators defined and
 used in ISO-2022-JP are:

    "ESC" + "(" + ?     for ISO646 variants

    "ESC" + "$" + ?     and
    "ESC" + "$" + "(" + ?   for multibyte character sets
*/
typedef enum
{
  FSM_ASCII,
  FSM_ESC,
  FSM_ESCD,
  FSM_ESCDP,
  FSM_ESCP,
  FSM_NONASCII
} ISO2022State;

/************************
** Source
************************/

enum
{
    CHARBUF_SIZE=5,
    LASTPOS_SIZE=64
};

/* non-raw input is cleaned up*/
struct _StreamIn
{
    ISO2022State    state;     /* FSM for ISO2022 */
    Bool   pushed;
    TidyAllocator *allocator;
    tchar* charbuf;
    uint   bufpos;
    uint   bufsize;
    int    tabs;
    int    lastcols[LASTPOS_SIZE];
    unsigned short curlastpos; /* current last position in lastcols */ 
    unsigned short firstlastpos; /* first valid last position in lastcols */ 
    int    curcol;
    int    curline;
    int    encoding;
    IOType iotype;

    TidyInputSource source;

#ifdef TIDY_WIN32_MLANG_SUPPORT
    void* mlang;
#endif

#ifdef TIDY_STORE_ORIGINAL_TEXT
    tmbstr otextbuf;
    size_t otextsize;
    uint   otextlen;
#endif

    /* Pointer back to document for error reporting */
    TidyDocImpl* doc;
};

StreamIn* TY_(initStreamIn)( TidyDocImpl* doc, int encoding );
void TY_(freeStreamIn)(StreamIn* in);

StreamIn* TY_(FileInput)( TidyDocImpl* doc, FILE* fp, int encoding );
StreamIn* TY_(BufferInput)( TidyDocImpl* doc, TidyBuffer* content, int encoding );
StreamIn* TY_(UserInput)( TidyDocImpl* doc, TidyInputSource* source, int encoding );

int       TY_(ReadBOMEncoding)(StreamIn *in);
uint      TY_(ReadChar)( StreamIn* in );
void      TY_(UngetChar)( uint c, StreamIn* in );
Bool      TY_(IsEOF)( StreamIn* in );


/************************
** Sink
************************/

struct _StreamOut
{
    int   encoding;
    ISO2022State   state;     /* for ISO 2022 */
    uint  nl;

#ifdef TIDY_WIN32_MLANG_SUPPORT
    void* mlang;
#endif

    IOType iotype;
    TidyOutputSink sink;
};

StreamOut* TY_(FileOutput)( TidyDocImpl *doc, FILE* fp, int encoding, uint newln );
StreamOut* TY_(BufferOutput)( TidyDocImpl *doc, TidyBuffer* buf, int encoding, uint newln );
StreamOut* TY_(UserOutput)( TidyDocImpl *doc, TidyOutputSink* sink, int encoding, uint newln );

StreamOut* TY_(StdErrOutput)(void);
/* StreamOut* StdOutOutput(void); */
void       TY_(ReleaseStreamOut)( TidyDocImpl *doc, StreamOut* out );

void TY_(WriteChar)( uint c, StreamOut* out );
void TY_(outBOM)( StreamOut *out );

ctmbstr TY_(GetEncodingNameFromTidyId)(uint id);
ctmbstr TY_(GetEncodingOptNameFromTidyId)(uint id);
int TY_(GetCharEncodingFromOptName)(ctmbstr charenc);

/************************
** Misc
************************/

/* character encodings
*/
#define RAW         0
#define ASCII       1
#define LATIN0      2
#define LATIN1      3
#define UTF8        4
#define ISO2022     5
#define MACROMAN    6
#define WIN1252     7
#define IBM858      8

#if SUPPORT_UTF16_ENCODINGS
#define UTF16LE     9
#define UTF16BE     10
#define UTF16       11
#endif

/* Note that Big5 and SHIFTJIS are not converted to ISO 10646 codepoints
** (i.e., to Unicode) before being recoded into UTF-8. This may be
** confusing: usually UTF-8 implies ISO10646 codepoints.
*/
#if SUPPORT_ASIAN_ENCODINGS
#if SUPPORT_UTF16_ENCODINGS
#define BIG5        12
#define SHIFTJIS    13
#else
#define BIG5        9
#define SHIFTJIS    10
#endif
#endif

#ifdef TIDY_WIN32_MLANG_SUPPORT
/* hack: windows code page numbers start at 37 */
#define WIN32MLANG  36
#endif


/* char encoding used when replacing illegal SGML chars,
** regardless of specified encoding.  Set at compile time
** to either Windows or Mac.
*/
extern const int TY_(ReplacementCharEncoding);

/* Function for conversion from Windows-1252 to Unicode */
uint TY_(DecodeWin1252)(uint c);

/* Function to convert from MacRoman to Unicode */
uint TY_(DecodeMacRoman)(uint c);

#ifdef __cplusplus
}
#endif


/* Use numeric constants as opposed to escape chars (\r, \n)
** to avoid conflict Mac compilers that may re-define these.
*/
#define CR    0xD
#define LF    0xA

#if   defined(MAC_OS_CLASSIC)
#define DEFAULT_NL_CONFIG TidyCR
#elif defined(_WIN32) || defined(OS2_OS)
#define DEFAULT_NL_CONFIG TidyCRLF
#else
#define DEFAULT_NL_CONFIG TidyLF
#endif


#endif /* __STREAMIO_H__ */
