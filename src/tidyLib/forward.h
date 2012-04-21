#ifndef __FORWARD_H__
#define __FORWARD_H__

/* forward.h -- Forward declarations for major Tidy structures

  (c) 1998-2007 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2007/02/11 09:45:52 $ 
    $Revision: 1.7 $ 

  Avoids many include file circular dependencies.

  Try to keep this file down to the minimum to avoid
  cross-talk between modules.

  Header files include this file.  C files include tidy-int.h.

*/

#include "platform.h"
#include "tidy.h"

/* Internal symbols are prefixed to avoid clashes with other libraries */
#define TYDYAPPEND(str1,str2) str1##str2
#define TY_(str) TYDYAPPEND(prvTidy,str)

struct _StreamIn;
typedef struct _StreamIn StreamIn;

struct _StreamOut;
typedef struct _StreamOut StreamOut;

struct _TidyDocImpl;
typedef struct _TidyDocImpl TidyDocImpl;


struct _Dict;
typedef struct _Dict Dict;

struct _Attribute;
typedef struct _Attribute Attribute;

struct _AttVal;
typedef struct _AttVal AttVal;

struct _Node;
typedef struct _Node Node;

struct _IStack;
typedef struct _IStack IStack;

struct _Lexer;
typedef struct _Lexer Lexer;

extern TidyAllocator TY_(g_default_allocator);

/** Wrappers for easy memory allocation using an allocator */
#define TidyAlloc(allocator, size) ((allocator)->vtbl->alloc((allocator), (size)))
#define TidyRealloc(allocator, block, size) ((allocator)->vtbl->realloc((allocator), (block), (size)))
#define TidyFree(allocator, block) ((allocator)->vtbl->free((allocator), (block)))
#define TidyPanic(allocator, msg) ((allocator)->vtbl->panic((allocator), (msg)))
#define TidyClearMemory(block, size) memset((block), 0, (size))
 

#endif /* __FORWARD_H__ */
