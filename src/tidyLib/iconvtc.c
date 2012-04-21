/* iconvtc.c -- Interface to iconv transcoding routines

  (c) 1998-2008 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  $Id: iconvtc.c,v 1.2 2008/08/09 11:55:27 hoehrmann Exp $
*/

#include "tidy.h"
#include "forward.h"
#include "streamio.h"

#ifdef TIDY_ICONV_SUPPORT

#include <iconv.h>

/* maximum number of bytes for a single character */
#define TC_INBUFSIZE  16

/* maximum number of characters per byte sequence */
#define TC_OUTBUFSIZE 16

Bool IconvInitInputTranscoder(void)
{
    return no;
}

void IconvUninitInputTranscoder(void)
{
    return;
}

int IconvGetChar(byte firstByte, StreamIn * in, uint * bytesRead)
{
    iconv_t cd;
    TidyInputSource * source;
    char inbuf[TC_INBUFSIZE] = { 0 };
    char outbuf[TC_OUTBUFSIZE] = { 0 };
    size_t inbufsize = 0;

    assert( in != NULL );
    assert( &in->source != NULL );
    assert( bytesRead != NULL );
    assert( in->iconvptr != 0 );

    cd = (iconv_t)in->iconvptr;
    source = &in->source;

    inbuf[inbufsize++] = (char)firstByte;
    
    while(inbufsize < TC_INBUFSIZE)
    {
        char * outbufptr = (char*)outbuf;
        char * inbufptr = (char*)inbuf;
        size_t readNow = inbufsize;
        size_t writeNow = TC_OUTBUFSIZE;
        size_t result = 0;
        int iconv_errno = 0;
        int nextByte = EndOfStream;

        result = iconv(cd, (const char**)&inbufptr, &readNow, (char**)&outbufptr, &writeNow);
        iconv_errno = errno;

        if (result != (size_t)(-1))
        {
            int c;

            /* create codepoint from UTF-32LE octets */
            c = (unsigned char)outbuf[0];
            c += (unsigned char)outbuf[1] << 8;
            c += (unsigned char)outbuf[2] << 16;
            c += (unsigned char)outbuf[3] << 32;

            /* set number of read bytes */
            *bytesRead = inbufsize;

            return c;
        }

        assert( iconv_errno != EILSEQ ); /* broken multibyte sequence */
        assert( iconv_errno != E2BIG );  /* not enough memory         */
        assert( iconv_errno == EINVAL ); /* incomplete sequence       */

        /* we need more bytes */
        nextByte = source->getByte(source->sourceData);

        if (nextByte == EndOfStream)
        {
            /* todo: error message for broken stream? */

            *bytesRead = inbufsize;
            return EndOfStream;
        }

        inbuf[inbufsize++] = (char)nextByte;
    }

    /* No full character found after reading TC_INBUFSIZE bytes, */
    /* give up to read this stream, it's obviously unreadable.   */

    /* todo: error message for broken stream? */
    return EndOfStream;
}

#endif /* TIDY_ICONV_SUPPORT */
