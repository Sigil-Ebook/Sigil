#ifndef __TIDY_PLATFORM_H__
#define __TIDY_PLATFORM_H__

/* platform.h -- Platform specifics

  (c) 1998-2008 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2008/03/17 12:57:01 $ 
    $Revision: 1.66 $ 

*/

#ifdef __cplusplus
extern "C" {
#endif

/*
  Uncomment and edit one of the following #defines if you
  want to specify the config file at compile-time.
*/

/* #define TIDY_CONFIG_FILE "/etc/tidy_config.txt" */ /* original */
/* #define TIDY_CONFIG_FILE "/etc/tidyrc" */
/* #define TIDY_CONFIG_FILE "/etc/tidy.conf" */

/*
  Uncomment the following #define if you are on a system
  supporting the HOME environment variable.
  It enables tidy to find config files named ~/.tidyrc if 
  the HTML_TIDY environment variable is not set.
*/
/* #define TIDY_USER_CONFIG_FILE "~/.tidyrc" */

/*
  Uncomment the following #define if your
  system supports the call getpwnam(). 
  E.g. Unix and Linux.

  It enables tidy to find files named 
  ~your/foo for use in the HTML_TIDY environment
  variable or CONFIG_FILE or USER_CONFIGFILE or
  on the command line: -config ~joebob/tidy.cfg

  Contributed by Todd Lewis.
*/

/* #define SUPPORT_GETPWNAM */


/* Enable/disable support for Big5 and Shift_JIS character encodings */
#ifndef SUPPORT_ASIAN_ENCODINGS
#define SUPPORT_ASIAN_ENCODINGS 1
#endif

/* Enable/disable support for UTF-16 character encodings */
#ifndef SUPPORT_UTF16_ENCODINGS
#define SUPPORT_UTF16_ENCODINGS 1
#endif

/* Enable/disable support for additional accessibility checks */
#ifndef SUPPORT_ACCESSIBILITY_CHECKS
#define SUPPORT_ACCESSIBILITY_CHECKS 1
#endif


/* Convenience defines for Mac platforms */

#if defined(macintosh)
/* Mac OS 6.x/7.x/8.x/9.x, with or without CarbonLib - MPW or Metrowerks 68K/PPC compilers */
#define MAC_OS_CLASSIC
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Mac OS"
#endif

/* needed for access() */
#if !defined(_POSIX) && !defined(NO_ACCESS_SUPPORT)
#define NO_ACCESS_SUPPORT
#endif

#ifdef SUPPORT_GETPWNAM
#undef SUPPORT_GETPWNAM
#endif

#elif defined(__APPLE__) && defined(__MACH__)
/* Mac OS X (client) 10.x (or server 1.x/10.x) - gcc or Metrowerks MachO compilers */
#define MAC_OS_X
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Mac OS X"
#endif
#endif

#if defined(MAC_OS_CLASSIC) || defined(MAC_OS_X)
/* Any OS on Mac platform */
#define MAC_OS
#define FILENAMES_CASE_SENSITIVE 0
#define strcasecmp strcmp
#ifndef DFLT_REPL_CHARENC
#define DFLT_REPL_CHARENC MACROMAN
#endif
#endif

/* Convenience defines for BSD like platforms */
 
#if defined(__FreeBSD__)
#define BSD_BASED_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "FreeBSD"
#endif

#elif defined(__NetBSD__)
#define BSD_BASED_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "NetBSD"
#endif

#elif defined(__OpenBSD__)
#define BSD_BASED_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "OpenBSD"
#endif

#elif defined(__DragonFly__)
#define BSD_BASED_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "DragonFly"
#endif

#elif defined(__MINT__)
#define BSD_BASED_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "FreeMiNT"
#endif

#elif defined(__bsdi__)
#define BSD_BASED_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "BSD/OS"
#endif

#endif

/* Convenience defines for Windows platforms */
 
#if defined(WINDOWS) || defined(_WIN32)

#define WINDOWS_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Windows"
#endif

#if defined(__MWERKS__) || defined(__MSL__)
/* not available with Metrowerks Standard Library */

#ifdef SUPPORT_GETPWNAM
#undef SUPPORT_GETPWNAM
#endif

/* needed for setmode() */
#if !defined(NO_SETMODE_SUPPORT)
#define NO_SETMODE_SUPPORT
#endif

#define strcasecmp _stricmp

#endif

#if defined(__BORLANDC__)
#define strcasecmp stricmp
#endif

#define FILENAMES_CASE_SENSITIVE 0
#define SUPPORT_POSIX_MAPPED_FILES 0

#endif

/* Convenience defines for Linux platforms */
 
#if defined(linux) && defined(__alpha__)
/* Linux on Alpha - gcc compiler */
#define LINUX_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Linux/Alpha"
#endif

#elif defined(linux) && defined(__sparc__)
/* Linux on Sparc - gcc compiler */
#define LINUX_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Linux/Sparc"
#endif

#elif defined(linux) && (defined(__i386__) || defined(__i486__) || defined(__i586__) || defined(__i686__))
/* Linux on x86 - gcc compiler */
#define LINUX_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Linux/x86"
#endif

#elif defined(linux) && defined(__powerpc__)
/* Linux on PPC - gcc compiler */
#define LINUX_OS

#if defined(__linux__) && defined(__powerpc__)

/* #if #system(linux) */
/* MkLinux on PPC  - gcc (egcs) compiler */
/* #define MAC_OS_MKLINUX */
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "MkLinux"
#endif

#else

#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Linux/PPC"
#endif

#endif

#elif defined(linux) || defined(__linux__)
/* generic Linux */
#define LINUX_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Linux"
#endif

#endif

/* Convenience defines for Solaris platforms */
 
#if defined(sun)
#define SOLARIS_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Solaris"
#endif
#endif

/* Convenience defines for HPUX + gcc platforms */

#if defined(__hpux)
#define HPUX_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "HPUX"
#endif
#endif

/* Convenience defines for RISCOS + gcc platforms */

#if defined(__riscos__)
#define RISC_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "RISC OS"
#endif
#endif

/* Convenience defines for OS/2 + icc/gcc platforms */

#if defined(__OS2__) || defined(__EMX__)
#define OS2_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "OS/2"
#endif
#define FILENAMES_CASE_SENSITIVE 0
#define strcasecmp stricmp
#endif

/* Convenience defines for IRIX */

#if defined(__sgi)
#define IRIX_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "SGI IRIX"
#endif
#endif

/* Convenience defines for AIX */

#if defined(_AIX)
#define AIX_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "IBM AIX"
#endif
#endif


/* Convenience defines for BeOS platforms */

#if defined(__BEOS__)
#define BE_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "BeOS"
#endif
#endif

/* Convenience defines for Cygwin platforms */

#if defined(__CYGWIN__)
#define CYGWIN_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Cygwin"
#endif
#define FILENAMES_CASE_SENSITIVE 0
#endif

/* Convenience defines for OpenVMS */

#if defined(__VMS)
#define OPENVMS_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "OpenVMS"
#endif
#define FILENAMES_CASE_SENSITIVE 0
#endif

/* Convenience defines for DEC Alpha OSF + gcc platforms */

#if defined(__osf__)
#define OSF_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "DEC Alpha OSF"
#endif
#endif

/* Convenience defines for ARM platforms */

#if defined(__arm)
#define ARM_OS

#if defined(forARM) && defined(__NEWTON_H)

/* Using Newton C++ Tools ARMCpp compiler */
#define NEWTON_OS
#ifndef PLATFORM_NAME
#define PLATFORM_NAME "Newton"
#endif

#else

#ifndef PLATFORM_NAME
#define PLATFORM_NAME "ARM"
#endif

#endif

#endif

#include <ctype.h>
#include <stdio.h>
#include <setjmp.h>  /* for longjmp on error exit */
#include <stdlib.h>
#include <stdarg.h>  /* may need <varargs.h> for Unix V */
#include <string.h>
#include <assert.h>

#ifdef NEEDS_MALLOC_H
#include <malloc.h>
#endif

#ifdef SUPPORT_GETPWNAM
#include <pwd.h>
#endif

#ifdef NEEDS_UNISTD_H
#include <unistd.h>  /* needed for unlink on some Unix systems */
#endif

/* This can be set at compile time.  Usually Windows,
** except for Macintosh builds.
*/
#ifndef DFLT_REPL_CHARENC
#define DFLT_REPL_CHARENC WIN1252
#endif

/* By default, use case-sensitive filename comparison.
*/
#ifndef FILENAMES_CASE_SENSITIVE
#define FILENAMES_CASE_SENSITIVE 1
#endif


/*
  Tidy preserves the last modified time for the files it
  cleans up.
*/

/*
  If your platform doesn't support <utime.h> and the
  utime() function, or <sys/futime> and the futime()
  function then set PRESERVE_FILE_TIMES to 0.
  
  If your platform doesn't support <sys/utime.h> and the
  futime() function, then set HAS_FUTIME to 0.
  
  If your platform supports <utime.h> and the
  utime() function requires the file to be
  closed first, then set UTIME_NEEDS_CLOSED_FILE to 1.
*/

/* Keep old PRESERVEFILETIMES define for compatibility */
#ifdef PRESERVEFILETIMES
#undef PRESERVE_FILE_TIMES
#define PRESERVE_FILE_TIMES PRESERVEFILETIMES
#endif

#ifndef PRESERVE_FILE_TIMES
#if defined(RISC_OS) || defined(OPENVMS_OS) || defined(OSF_OS)
#define PRESERVE_FILE_TIMES 0
#else
#define PRESERVE_FILE_TIMES 1
#endif
#endif

#if PRESERVE_FILE_TIMES

#ifndef HAS_FUTIME
#if defined(CYGWIN_OS) || defined(BE_OS) || defined(OS2_OS) || defined(HPUX_OS) || defined(SOLARIS_OS) || defined(LINUX_OS) || defined(BSD_BASED_OS) || defined(MAC_OS) || defined(__MSL__) || defined(IRIX_OS) || defined(AIX_OS) || defined(__BORLANDC__)
#define HAS_FUTIME 0
#else
#define HAS_FUTIME 1
#endif
#endif

#ifndef UTIME_NEEDS_CLOSED_FILE
#if defined(SOLARIS_OS) || defined(BSD_BASED_OS) || defined(MAC_OS) || defined(__MSL__) || defined(LINUX_OS)
#define UTIME_NEEDS_CLOSED_FILE 1
#else
#define UTIME_NEEDS_CLOSED_FILE 0
#endif
#endif

#if defined(MAC_OS_X) || (!defined(MAC_OS_CLASSIC) && !defined(__MSL__))
#include <sys/types.h> 
#include <sys/stat.h>
#else
#include <stat.h>
#endif

#if HAS_FUTIME
#include <sys/utime.h>
#else
#include <utime.h>
#endif /* HASFUTIME */

/*
  MS Windows needs _ prefix for Unix file functions.
  Not required by Metrowerks Standard Library (MSL).
  
  Tidy uses following for preserving the last modified time.

  WINDOWS automatically set by Win16 compilers.
  _WIN32 automatically set by Win32 compilers.
*/
#if defined(_WIN32) && !defined(__MSL__) && !defined(__BORLANDC__)

#define futime _futime
#define fstat _fstat
#define utimbuf _utimbuf /* Windows seems to want utimbuf */
#define stat _stat
#define utime _utime
#define vsnprintf _vsnprintf
#endif /* _WIN32 */

#endif /* PRESERVE_FILE_TIMES */

/*
  MS Windows needs _ prefix for Unix file functions.
  Not required by Metrowerks Standard Library (MSL).
  
  WINDOWS automatically set by Win16 compilers.
  _WIN32 automatically set by Win32 compilers.
*/
#if defined(_WIN32) && !defined(__MSL__) && !defined(__BORLANDC__)

#ifndef __WATCOMC__
#define fileno _fileno
#define setmode _setmode
#endif

#define access _access
#define strcasecmp _stricmp

#if _MSC_VER > 1000
#pragma warning( disable : 4189 ) /* local variable is initialized but not referenced */
#pragma warning( disable : 4100 ) /* unreferenced formal parameter */
#pragma warning( disable : 4706 ) /* assignment within conditional expression */
#endif

#if _MSC_VER > 1300
#pragma warning( disable : 4996 ) /* disable depreciation warning */
#endif

#endif /* _WIN32 */

#if defined(_WIN32)

#if (defined(_USRDLL) || defined(_WINDLL)) && !defined(TIDY_EXPORT)
#define TIDY_EXPORT __declspec( dllexport ) 
#endif

#ifndef TIDY_CALL
#ifdef _WIN64
#  define TIDY_CALL __fastcall
#else
#  define TIDY_CALL __stdcall
#endif
#endif

#endif /* _WIN32 */

/* hack for gnu sys/types.h file which defines uint and ulong */

#if defined(BE_OS) || defined(SOLARIS_OS) || defined(BSD_BASED_OS) || defined(OSF_OS) || defined(IRIX_OS) || defined(AIX_OS)
#include <sys/types.h>
#endif
#if !defined(HPUX_OS) && !defined(CYGWIN_OS) && !defined(MAC_OS_X) && !defined(BE_OS) && !defined(SOLARIS_OS) && !defined(BSD_BASED_OS) && !defined(OSF_OS) && !defined(IRIX_OS) && !defined(AIX_OS) && !defined(LINUX_OS)
# undef uint
typedef unsigned int uint;
#endif
#if defined(HPUX_OS) || defined(CYGWIN_OS) || defined(MAC_OS) || defined(BSD_BASED_OS) || defined(_WIN32)
# undef ulong
typedef unsigned long ulong;
#endif

/*
With GCC 4,  __attribute__ ((visibility("default"))) can be used along compiling with tidylib 
with "-fvisibility=hidden". See http://gcc.gnu.org/wiki/Visibility and build/gmake/Makefile.
*/
/*
#if defined(__GNUC__) && __GNUC__ >= 4
#define TIDY_EXPORT __attribute__ ((visibility("default")))
#endif
*/

#ifndef TIDY_EXPORT /* Define it away for most builds */
#define TIDY_EXPORT 
#endif

#ifndef TIDY_STRUCT
#define TIDY_STRUCT
#endif

typedef unsigned char byte;

typedef uint tchar;         /* single, full character */
typedef char tmbchar;       /* single, possibly partial character */
#ifndef TMBSTR_DEFINED
typedef tmbchar* tmbstr;    /* pointer to buffer of possibly partial chars */
typedef const tmbchar* ctmbstr; /* Ditto, but const */
#define NULLSTR (tmbstr)""
#define TMBSTR_DEFINED
#endif

#ifndef TIDY_CALL
#define TIDY_CALL
#endif

#if defined(__GNUC__) || defined(__INTEL_COMPILER)
# define ARG_UNUSED(x) x __attribute__((unused))
#else
# define ARG_UNUSED(x) x
#endif

/* HAS_VSNPRINTF triggers the use of "vsnprintf", which is safe related to
   buffer overflow. Therefore, we make it the default unless HAS_VSNPRINTF
   has been defined. */
#ifndef HAS_VSNPRINTF
# define HAS_VSNPRINTF 1
#endif

#ifndef SUPPORT_POSIX_MAPPED_FILES
# define SUPPORT_POSIX_MAPPED_FILES 1
#endif

/*
  bool is a reserved word in some but
  not all C++ compilers depending on age
  work around is to avoid bool altogether
  by introducing a new enum called Bool
*/
/* We could use the C99 definition where supported
typedef _Bool Bool;
#define no (_Bool)0
#define yes (_Bool)1
*/
typedef enum
{
   no,
   yes
} Bool;

/* for NULL pointers 
#define null ((const void*)0)
extern void* null;
*/

#if defined(DMALLOC)
#include "dmalloc.h"
#endif

/* Opaque data structure.
*  Cast to implementation type struct within lib.
*  This will reduce inter-dependencies/conflicts w/ application code.
*/
#if 1
#define opaque_type( typenam )\
struct _##typenam { int _opaque; };\
typedef struct _##typenam const * typenam
#else
#define opaque_type(typenam) typedef const void* typenam
#endif

/* Opaque data structure used to pass back
** and forth to keep current position in a
** list or other collection.
*/
opaque_type( TidyIterator );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __TIDY_PLATFORM_H__ */


/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
