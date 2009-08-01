/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-2005 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ifndef ZCONF_H
#define ZCONF_H

#if _MSC_VER > 1000
	#pragma warning (disable : 4131)
	#pragma warning (disable : 4115)
	#pragma warning (disable : 4127)
	#pragma warning (disable : 4100)
	#pragma warning (disable : 4244)
	#pragma warning (disable : 4702)
	#pragma warning (disable : 4206)
#endif


/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 */
#ifdef Z_PREFIX
//#  define deflateInit		zarch_deflateInit
#  define deflateInit_		zarch_deflateInit_
#  define deflate			zarch_deflate
#  define deflateEnd		zarch_deflateEnd
//#  define deflateInit2	zarch_deflateInit2
#  define deflateInit2_		zarch_deflateInit2_
#  define deflateSetDictionary zarch_deflateSetDictionary
#  define deflateCopy		zarch_deflateCopy
#  define deflateReset		zarch_deflateReset
#  define deflateParams		zarch_deflateParams
#  define deflateBound		zarch_deflateBound
#  define deflatePrime      zarch_deflatePrime
#  define deflateSetHeader	zarch_deflateSetHeader
#  define deflateTune		zarch_deflateTune
//#  define inflateInit		zarch_inflateInit
//#  define inflateInit2	zarch_inflateInit2
#  define inflateInit2_		zarch_inflateInit2_
#  define inflateInit_		zarch_inflateInit_
#  define inflate			zarch_inflate
#  define inflateEnd		zarch_inflateEnd
#  define inflateSetDictionary zarch_inflateSetDictionary
#  define inflateSync		zarch_inflateSync
#  define inflateSyncPoint zarch_inflateSyncPoint
#  define inflateCopy		zarch_inflateCopy
#  define inflateReset		zarch_inflateReset
//#  define inflateBackInit	zarch_inflateBackInit
#  define inflateBackInit_	zarch_inflateBackInit_
#  define inflateBack       zarch_inflateBack
#  define inflateBackEnd    zarch_inflateBackEnd
#  define inflatePrime		zarch_inflatePrime
#  define inflateGetHeader	zarch_inflateGetHeader
#  define compress			zarch_compress
#  define compress2			zarch_compress2
#  define compressBound		zarch_compressBound
#  define uncompress		zarch_uncompress
#  define adler32			zarch_adler32
#  define adler32_combine	zarch_adler32_combine
#  define crc32_combine		zarch_crc32_combine
#  define deflate_copyright zarch_deflate_copyright
#  define inflate_copyright zarch_inflate_copyright
#  define crc32				zarch_crc32
#  define get_crc_table		zarch_get_crc_table
#  define zError            zarch_zError
#  define z_stream			zarch_z_stream
#  define z_stream_s		zarch_z_stream_s
#  define alloc_func        zarch_alloc_func
#  define free_func         zarch_free_func
#  define in_func           zarch_in_func
#  define out_func          zarch_out_func
#  define Byte				zarch_Byte
#  define uInt				zarch_uInt
#  define uLong				zarch_uLong
#  define uLongLong			zarch_uLongLong
#  define Bytef				zarch_Bytef
#  define charf				zarch_charf
#  define intf				zarch_intf
#  define uIntf				zarch_uIntf
#  define uLongf			zarch_uLongf
#  define voidpf			zarch_voidpf
#  define voidp				zarch_voidp
#  define deflate_state		zarch_deflate_state
#  define deflate_slow		zarch_deflate_slow
#  define deflate_fast		zarch_deflate_fast
#  define deflate_stored	zarch_deflate_stored
#  define z_streamp			zarch_z_streamp
#  define deflate_rle		zarch_deflate_rle
#  define inflate_state		zarch_inflate_state
#  define inflate_fast		zarch_inflate_fast
#  define inflate_table		zarch_inflate_table
#  define updatewindow		zarch_updatewindow
//#  define inflate_mode	zarch_inflate_mode
//#  define send_bits		zarch_send_bits
#  define zlibVersion		zarch_zlibVersion
#  define zlibCompileFlags	zarch_zlibCompileFlags
#  define zError			zarch_zError
#  define _tr_init			zarch_tr_init
#  define _tr_tally			zarch_tr_tally
#  define _tr_flush_block	zarch_tr_flush_block
#  define _tr_align			zarch_tr_align
#  define _tr_stored_block	zarch_tr_stored_block
#  define _dist_code		zarch_dist_code
#  define _length_code		zarch_length_code
#endif

#if defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#endif
#if (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
#  define OS2
#endif
#if defined(_WINDOWS) && !defined(WINDOWS)
#  define WINDOWS
#endif
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
#  ifndef WIN32
#  define WIN32
#  endif
#endif
#if (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
#  if !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
#    ifndef SYS16BIT
#      define SYS16BIT
#    endif
#  endif
#endif

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#ifdef SYS16BIT
#  define MAXSEG_64K
#endif
#ifdef MSDOS
#  define UNALIGNED_OK
#endif

#ifdef __STDC_VERSION__
#  ifndef STDC
#    define STDC
#  endif
#  if __STDC_VERSION__ >= 199901L
#    ifndef STDC99
#      define STDC99
#    endif
#  endif
#endif
#if !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#  define STDC
#endif
#if !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#  define STDC
#endif
#if !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#  define STDC
#endif
#if !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#  define STDC
#endif

#if defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
#  define STDC
#endif

#ifndef STDC
#  ifndef const /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define const       /* note: need a more gentle solution here */
#  endif
#endif

/* Some Mac compilers merge all .h files incorrectly: */
#if defined(__MWERKS__)||defined(applec)||defined(THINK_C)||defined(__SC__)
#  define NO_DUMMY_DECL
#endif

/* Maximum value for memLevel in deflateInit2 */
#ifndef MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  else
#    define MAX_MEM_LEVEL 9
#  endif
#endif

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#ifndef MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#endif

/* The memory requirements for deflate are (in bytes):
            (1 << (windowBits+2)) +  (1 << (memLevel+9))
 that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
 plus a few kilobytes for small objects. For example, if you want to reduce
 the default memory requirements from 256K to 128K, compile with
     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
 Of course this will generally degrade compression (there's no free lunch).

   The memory requirements for inflate are (in bytes) 1 << windowBits
 that is, 32K for windowBits=15 (default value) plus a few kilobytes
 for small objects.
*/

                        /* Type declarations */

#ifndef OF /* function prototypes */
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#ifdef SYS16BIT
#  if defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    else
#      define FAR far
#    endif
#  endif
#  if (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    else
#      define FAR far
#    endif
#  endif
#endif

#if defined(WINDOWS) || defined(WIN32)
   /* If building or using zlib as a DLL, define ZLIB_DLL.
    * This is not mandatory, but it offers a little performance increase.
    */
#  ifdef ZLIB_DLL
#    if defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef ZLIB_INTERNAL
#        define ZEXTERN extern __declspec(dllexport)
#      else
#        define ZEXTERN extern __declspec(dllimport)
#      endif
#    endif
#  endif  /* ZLIB_DLL */
   /* If building or using zlib with the WINAPI/WINAPIV calling convention,
    * define ZLIB_WINAPI.
    * Caution: the standard ZLIB1.DLL is NOT compiled using ZLIB_WINAPI.
    */
#  ifdef ZLIB_WINAPI
#    ifdef FAR
#      undef FAR
#    endif
#    include <windows.h>
     /* No need for _export, use ZLIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define ZEXPORT WINAPI
#    ifdef WIN32
#      define ZEXPORTVA WINAPIV
#    else
#      define ZEXPORTVA FAR CDECL
#    endif
#  endif
#endif

#if defined (__BEOS__)
#  ifdef ZLIB_DLL
#    ifdef ZLIB_INTERNAL
#      define ZEXPORT   __declspec(dllexport)
#      define ZEXPORTVA __declspec(dllexport)
#    else
#      define ZEXPORT   __declspec(dllimport)
#      define ZEXPORTVA __declspec(dllimport)
#    endif
#  endif
#endif

#ifndef ZEXTERN
#  define ZEXTERN extern
#endif
#ifndef ZEXPORT
#  define ZEXPORT
#endif
#ifndef ZEXPORTVA
#  define ZEXPORTVA
#endif

#ifndef FAR
#  define FAR
#endif

#if !defined(__MACTYPES__)
typedef unsigned char  Byte;  /* 8 bits */
#endif
typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */

#include "../_features.h"

	typedef unsigned long uLongLong;

#ifdef SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#else
   typedef Byte  FAR Bytef;
#endif
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;

#ifdef STDC
   typedef void const *voidpc;
   typedef void FAR   *voidpf;
   typedef void       *voidp;
#else
   typedef Byte const *voidpc;
   typedef Byte FAR   *voidpf;
   typedef Byte       *voidp;
#endif

#if 0           /* HAVE_UNISTD_H -- this line is updated by ./configure */
#  include <sys/types.h> /* for off_t */
#  include <unistd.h>    /* for SEEK_* and off_t */
#  ifdef VMS
#    include <unixio.h>   /* for off_t */
#  endif
#  define z_off_t  off_t
#endif
#ifndef SEEK_SET
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#endif
#ifndef z_off_t
#  define  z_off_t long
#endif

#if defined(__OS400__)
#define NO_vsnprintf
#endif

#if defined(__MVS__)
#  define NO_vsnprintf
#  ifdef FAR
#    undef FAR
#  endif
#endif

/* MVS linker does not support external names larger than 8 bytes */
#if defined(__MVS__)
#   pragma map(deflateInit_,"DEIN")
#   pragma map(deflateInit2_,"DEIN2")
#   pragma map(deflateEnd,"DEEND")
#   pragma map(deflateBound,"DEBND")
#   pragma map(inflateInit_,"ININ")
#   pragma map(inflateInit2_,"ININ2")
#   pragma map(inflateEnd,"INEND")
#   pragma map(inflateSync,"INSY")
#   pragma map(inflateSetDictionary,"INSEDI")
#   pragma map(compressBound,"CMBND")
#   pragma map(inflate_table,"INTABL")
#   pragma map(inflate_fast,"INFA")
#   pragma map(inflate_copyright,"INCOPY")
#endif

#endif /* ZCONF_H */
