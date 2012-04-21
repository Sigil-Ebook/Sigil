/* buffio.c -- Treat buffer as an I/O stream.

  (c) 1998-2007 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2007/01/23 11:17:46 $ 
    $Revision: 1.14 $ 

  Requires buffer to automatically grow as bytes are added.
  Must keep track of current read and write points.

*/

#include "tidy.h"
#include "buffio.h"
#include "forward.h"

/**************
   TIDY
**************/

static int TIDY_CALL insrc_getByte( void* appData )
{
  TidyBuffer* buf = (TidyBuffer*) appData;
  return tidyBufGetByte( buf );
}
static Bool TIDY_CALL insrc_eof( void* appData )
{
  TidyBuffer* buf = (TidyBuffer*) appData;
  return tidyBufEndOfInput( buf );
}
static void TIDY_CALL insrc_ungetByte( void* appData, byte bv )
{
  TidyBuffer* buf = (TidyBuffer*) appData;
  tidyBufUngetByte( buf, bv );
}

void TIDY_CALL tidyInitInputBuffer( TidyInputSource* inp, TidyBuffer* buf )
{
  inp->getByte    = insrc_getByte;
  inp->eof        = insrc_eof;
  inp->ungetByte  = insrc_ungetByte;
  inp->sourceData = buf;
}

static void TIDY_CALL outsink_putByte( void* appData, byte bv )
{
  TidyBuffer* buf = (TidyBuffer*) appData;
  tidyBufPutByte( buf, bv );
}

void TIDY_CALL tidyInitOutputBuffer( TidyOutputSink* outp, TidyBuffer* buf )
{
  outp->putByte  = outsink_putByte;
  outp->sinkData = buf;
}


void TIDY_CALL tidyBufInit( TidyBuffer* buf )
{
    assert( buf != NULL );
    tidyBufInitWithAllocator( buf, NULL );
}

void TIDY_CALL tidyBufAlloc( TidyBuffer* buf, uint allocSize )
{
    tidyBufAllocWithAllocator( buf, NULL, allocSize );
}

void TIDY_CALL tidyBufInitWithAllocator( TidyBuffer* buf,
                                         TidyAllocator *allocator )
{
    assert( buf != NULL );
    TidyClearMemory( buf, sizeof(TidyBuffer) );
    buf->allocator = allocator ? allocator : &TY_(g_default_allocator);
}

void TIDY_CALL tidyBufAllocWithAllocator( TidyBuffer* buf,
                                          TidyAllocator *allocator,
                                          uint allocSize )
{
    tidyBufInitWithAllocator( buf, allocator );
    tidyBufCheckAlloc( buf, allocSize, 0 );
    buf->next = 0;
}

void TIDY_CALL tidyBufFree( TidyBuffer* buf )
{
    assert( buf != NULL );
    TidyFree(  buf->allocator, buf->bp );
    tidyBufInitWithAllocator( buf, buf->allocator );
}

void TIDY_CALL tidyBufClear( TidyBuffer* buf )
{
    assert( buf != NULL );
    if ( buf->bp )
    {
        TidyClearMemory( buf->bp, buf->allocated );
        buf->size = 0;
    }
    buf->next = 0;
}

/* Many users do not call tidyBufInit() or tidyBufAlloc() or their allocator
   counterparts. So by default, set the default allocator.
*/
static void setDefaultAllocator( TidyBuffer* buf )
{
    buf->allocator = &TY_(g_default_allocator);
}

/* Avoid thrashing memory by doubling buffer size
** until larger than requested size.
   buf->allocated is bigger than allocSize+1 so that a trailing null byte is
   always available.
*/
void TIDY_CALL tidyBufCheckAlloc( TidyBuffer* buf, uint allocSize, uint chunkSize )
{
    assert( buf != NULL );

    if ( !buf->allocator )
        setDefaultAllocator( buf );
        
    if ( 0 == chunkSize )
        chunkSize = 256;
    if ( allocSize+1 > buf->allocated )
    {
        byte* bp;
        uint allocAmt = chunkSize;
        if ( buf->allocated > 0 )
            allocAmt = buf->allocated;
        while ( allocAmt < allocSize+1 )
            allocAmt *= 2;

        bp = (byte*)TidyRealloc( buf->allocator, buf->bp, allocAmt );
        if ( bp != NULL )
        {
            TidyClearMemory( bp + buf->allocated, allocAmt - buf->allocated );
            buf->bp = bp;
            buf->allocated = allocAmt;
        }
    }
}

/* Attach buffer to a chunk O' memory w/out allocation */
void  TIDY_CALL tidyBufAttach( TidyBuffer* buf, byte* bp, uint size )
{
    assert( buf != NULL );
    buf->bp = bp;
    buf->size = buf->allocated = size;
    buf->next = 0;
    if ( !buf->allocator )
        setDefaultAllocator( buf );
}

/* Clear pointer to memory w/out deallocation */
void TIDY_CALL tidyBufDetach( TidyBuffer* buf )
{
    tidyBufInitWithAllocator( buf, buf->allocator );
}


/**************
   OUTPUT
**************/

void TIDY_CALL tidyBufAppend( TidyBuffer* buf, void* vp, uint size )
{
    assert( buf != NULL );
    if ( vp != NULL && size > 0 )
    {
        tidyBufCheckAlloc( buf, buf->size + size, 0 );
        memcpy( buf->bp + buf->size, vp, size );
        buf->size += size;
    }
}

void TIDY_CALL tidyBufPutByte( TidyBuffer* buf, byte bv )
{
    assert( buf != NULL );
    tidyBufCheckAlloc( buf, buf->size + 1, 0 );
    buf->bp[ buf->size++ ] = bv;
}


int TIDY_CALL tidyBufPopByte( TidyBuffer* buf )
{
    int bv = EOF;
    assert( buf != NULL );
    if ( buf->size > 0 )
      bv = buf->bp[ --buf->size ];
    return bv;
}

/**************
   INPUT
**************/

int TIDY_CALL tidyBufGetByte( TidyBuffer* buf )
{
    int bv = EOF;
    if ( ! tidyBufEndOfInput(buf) )
      bv = buf->bp[ buf->next++ ];
    return bv;
}

Bool TIDY_CALL tidyBufEndOfInput( TidyBuffer* buf )
{
    return ( buf->next >= buf->size );
}

void TIDY_CALL tidyBufUngetByte( TidyBuffer* buf, byte bv )
{
    if ( buf->next > 0 )
    {
        --buf->next;
        assert( bv == buf->bp[ buf->next ] );
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
