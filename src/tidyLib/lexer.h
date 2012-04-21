#ifndef __LEXER_H__
#define __LEXER_H__

/* lexer.h -- Lexer for html parser
  
   (c) 1998-2008 (W3C) MIT, ERCIM, Keio University
   See tidy.h for the copyright notice.
  
   CVS Info:
    $Author: arnaud02 $ 
    $Date: 2008/03/22 21:06:11 $ 
    $Revision: 1.41 $ 

*/

/*
  Given an input source, it returns a sequence of tokens.

     GetToken(source) gets the next token
     UngetToken(source) provides one level undo

  The tags include an attribute list:

    - linked list of attribute/value nodes
    - each node has 2 NULL-terminated strings.
    - entities are replaced in attribute values

  white space is compacted if not in preformatted mode
  If not in preformatted mode then leading white space
  is discarded and subsequent white space sequences
  compacted to single space characters.

  If XmlTags is no then Tag names are folded to upper
  case and attribute names to lower case.

 Not yet done:
    -   Doctype subset and marked sections
*/

#ifdef __cplusplus
extern "C" {
#endif

#include "forward.h"

/* lexer character types
*/
#define digit       1u
#define letter      2u
#define namechar    4u
#define white       8u
#define newline     16u
#define lowercase   32u
#define uppercase   64u
#define digithex    128u


/* node->type is one of these values
*/
typedef enum
{
  RootNode,
  DocTypeTag,
  CommentTag,
  ProcInsTag,
  TextNode,
  StartTag,
  EndTag,
  StartEndTag,
  CDATATag,
  SectionTag,
  AspTag,
  JsteTag,
  PhpTag,
  XmlDecl
} NodeType;



/* lexer GetToken states
*/
typedef enum
{
  LEX_CONTENT,
  LEX_GT,
  LEX_ENDTAG,
  LEX_STARTTAG,
  LEX_COMMENT,
  LEX_DOCTYPE,
  LEX_PROCINSTR,
  LEX_CDATA,
  LEX_SECTION,
  LEX_ASP,
  LEX_JSTE,
  LEX_PHP,
  LEX_XMLDECL
} LexerState;

/* ParseDocTypeDecl state constants */
typedef enum
{
  DT_INTERMEDIATE,
  DT_DOCTYPENAME,
  DT_PUBLICSYSTEM,
  DT_QUOTEDSTRING,
  DT_INTSUBSET
} ParseDocTypeDeclState;

/* content model shortcut encoding

   Descriptions are tentative.
*/
#define CM_UNKNOWN      0
/* Elements with no content. Map to HTML specification. */
#define CM_EMPTY        (1 << 0)
/* Elements that appear outside of "BODY". */
#define CM_HTML         (1 << 1)
/* Elements that can appear within HEAD. */
#define CM_HEAD         (1 << 2)
/* HTML "block" elements. */
#define CM_BLOCK        (1 << 3)
/* HTML "inline" elements. */
#define CM_INLINE       (1 << 4)
/* Elements that mark list item ("LI"). */
#define CM_LIST         (1 << 5)
/* Elements that mark definition list item ("DL", "DT"). */
#define CM_DEFLIST      (1 << 6)
/* Elements that can appear inside TABLE. */
#define CM_TABLE        (1 << 7)
/* Used for "THEAD", "TFOOT" or "TBODY". */
#define CM_ROWGRP       (1 << 8)
/* Used for "TD", "TH" */
#define CM_ROW          (1 << 9)
/* Elements whose content must be protected against white space movement.
   Includes some elements that can found in forms. */
#define CM_FIELD        (1 << 10)
/* Used to avoid propagating inline emphasis inside some elements
   such as OBJECT or APPLET. */
#define CM_OBJECT       (1 << 11)
/* Elements that allows "PARAM". */
#define CM_PARAM        (1 << 12)
/* "FRAME", "FRAMESET", "NOFRAMES". Used in ParseFrameSet. */
#define CM_FRAMES       (1 << 13)
/* Heading elements (h1, h2, ...). */
#define CM_HEADING      (1 << 14)
/* Elements with an optional end tag. */
#define CM_OPT          (1 << 15)
/* Elements that use "align" attribute for vertical position. */
#define CM_IMG          (1 << 16)
/* Elements with inline and block model. Used to avoid calling InlineDup. */
#define CM_MIXED        (1 << 17)
/* Elements whose content needs to be indented only if containing one 
   CM_BLOCK element. */
#define CM_NO_INDENT    (1 << 18)
/* Elements that are obsolete (such as "dir", "menu"). */
#define CM_OBSOLETE     (1 << 19)
/* User defined elements. Used to determine how attributes wihout value
   should be printed. */
#define CM_NEW          (1 << 20)
/* Elements that cannot be omitted. */
#define CM_OMITST       (1 << 21)

/* If the document uses just HTML 2.0 tags and attributes described
** it as HTML 2.0 Similarly for HTML 3.2 and the 3 flavors of HTML 4.0.
** If there are proprietary tags and attributes then describe it as
** HTML Proprietary. If it includes the xml-lang or xmlns attributes
** but is otherwise HTML 2.0, 3.2 or 4.0 then describe it as one of the
** flavors of Voyager (strict, loose or frameset).
*/

/* unknown */
#define xxxx                   0u

/* W3C defined HTML/XHTML family document types */
#define HT20                   1u
#define HT32                   2u
#define H40S                   4u
#define H40T                   8u
#define H40F                  16u
#define H41S                  32u
#define H41T                  64u
#define H41F                 128u
#define X10S                 256u
#define X10T                 512u
#define X10F                1024u
#define XH11                2048u
#define XB10                4096u

/* proprietary stuff */
#define VERS_SUN            8192u
#define VERS_NETSCAPE      16384u
#define VERS_MICROSOFT     32768u

/* special flag */
#define VERS_XML           65536u

/* compatibility symbols */
#define VERS_UNKNOWN       (xxxx)
#define VERS_HTML20        (HT20)
#define VERS_HTML32        (HT32)
#define VERS_HTML40_STRICT (H40S|H41S|X10S)
#define VERS_HTML40_LOOSE  (H40T|H41T|X10T)
#define VERS_FRAMESET      (H40F|H41F|X10F)
#define VERS_XHTML11       (XH11)
#define VERS_BASIC         (XB10)

/* meta symbols */
#define VERS_HTML40        (VERS_HTML40_STRICT|VERS_HTML40_LOOSE|VERS_FRAMESET)
#define VERS_IFRAME        (VERS_HTML40_LOOSE|VERS_FRAMESET)
#define VERS_LOOSE         (VERS_HTML20|VERS_HTML32|VERS_IFRAME)
#define VERS_EVENTS        (VERS_HTML40|VERS_XHTML11)
#define VERS_FROM32        (VERS_HTML32|VERS_HTML40)
#define VERS_FROM40        (VERS_HTML40|VERS_XHTML11|VERS_BASIC)
#define VERS_XHTML         (X10S|X10T|X10F|XH11|XB10)

/* all W3C defined document types */
#define VERS_ALL           (VERS_HTML20|VERS_HTML32|VERS_FROM40)

/* all proprietary types */
#define VERS_PROPRIETARY   (VERS_NETSCAPE|VERS_MICROSOFT|VERS_SUN)

/* Linked list of class names and styles
*/
struct _Style;
typedef struct _Style TagStyle;

struct _Style
{
    tmbstr tag;
    tmbstr tag_class;
    tmbstr properties;
    TagStyle *next;
};


/* Linked list of style properties
*/
struct _StyleProp;
typedef struct _StyleProp StyleProp;

struct _StyleProp
{
    tmbstr name;
    tmbstr value;
    StyleProp *next;
};




/* Attribute/Value linked list node
*/

struct _AttVal
{
    AttVal*           next;
    const Attribute*  dict;
    Node*             asp;
    Node*             php;
    int               delim;
    tmbstr            attribute;
    tmbstr            value;
};



/*
  Mosaic handles inlines via a separate stack from other elements
  We duplicate this to recover from inline markup errors such as:

     <i>italic text
     <p>more italic text</b> normal text

  which for compatibility with Mosaic is mapped to:

     <i>italic text</i>
     <p><i>more italic text</i> normal text

  Note that any inline end tag pop's the effect of the current
  inline start tag, so that </b> pop's <i> in the above example.
*/
struct _IStack
{
    IStack*     next;
    const Dict* tag;        /* tag's dictionary definition */
    tmbstr      element;    /* name (NULL for text nodes) */
    AttVal*     attributes;
};


/* HTML/XHTML/XML Element, Comment, PI, DOCTYPE, XML Decl,
** etc. etc.
*/

struct _Node
{
    Node*       parent;         /* tree structure */
    Node*       prev;
    Node*       next;
    Node*       content;
    Node*       last;

    AttVal*     attributes;
    const Dict* was;            /* old tag when it was changed */
    const Dict* tag;            /* tag's dictionary definition */

    tmbstr      element;        /* name (NULL for text nodes) */

    uint        start;          /* start of span onto text array */
    uint        end;            /* end of span onto text array */
    NodeType    type;           /* TextNode, StartTag, EndTag etc. */

    uint        line;           /* current line of document */
    uint        column;         /* current column of document */

    Bool        closed;         /* true if closed by explicit end tag */
    Bool        implicit;       /* true if inferred */
    Bool        linebreak;      /* true if followed by a line break */

#ifdef TIDY_STORE_ORIGINAL_TEXT
    tmbstr      otext;
#endif
};


/*
  The following are private to the lexer
  Use NewLexer() to create a lexer, and
  FreeLexer() to free it.
*/

struct _Lexer
{
#if 0  /* Move to TidyDocImpl */
    StreamIn* in;           /* document content input */
    StreamOut* errout;      /* error output stream */

    uint badAccess;         /* for accessibility errors */
    uint badLayout;         /* for bad style errors */
    uint badChars;          /* for bad character encodings */
    uint badForm;           /* for mismatched/mispositioned form tags */
    uint warnings;          /* count of warnings in this document */
    uint errors;            /* count of errors */
#endif

    uint lines;             /* lines seen */
    uint columns;           /* at start of current token */
    Bool waswhite;          /* used to collapse contiguous white space */
    Bool pushed;            /* true after token has been pushed back */
    Bool insertspace;       /* when space is moved after end tag */
    Bool excludeBlocks;     /* Netscape compatibility */
    Bool exiled;            /* true if moved out of table */
    Bool isvoyager;         /* true if xmlns attribute on html element */
    uint versions;          /* bit vector of HTML versions */
    uint doctype;           /* version as given by doctype (if any) */
    uint versionEmitted;    /* version of doctype emitted */
    Bool bad_doctype;       /* e.g. if html or PUBLIC is missing */
    uint txtstart;          /* start of current node */
    uint txtend;            /* end of current node */
    LexerState state;       /* state of lexer's finite state machine */

    Node* token;            /* last token returned by GetToken() */
    Node* itoken;           /* last duplicate inline returned by GetToken() */
    Node* root;             /* remember root node of the document */
    Node* parent;           /* remember parent node for CDATA elements */
    
    Bool seenEndBody;       /* true if a </body> tag has been encountered */
    Bool seenEndHtml;       /* true if a </html> tag has been encountered */

    /*
      Lexer character buffer

      Parse tree nodes span onto this buffer
      which contains the concatenated text
      contents of all of the elements.

      lexsize must be reset for each file.
    */
    tmbstr lexbuf;          /* MB character buffer */
    uint lexlength;         /* allocated */
    uint lexsize;           /* used */

    /* Inline stack for compatibility with Mosaic */
    Node* inode;            /* for deferring text node */
    IStack* insert;         /* for inferring inline tags */
    IStack* istack;
    uint istacklength;      /* allocated */
    uint istacksize;        /* used */
    uint istackbase;        /* start of frame */

    TagStyle *styles;          /* used for cleaning up presentation markup */

    TidyAllocator* allocator; /* allocator */

#if 0
    TidyDocImpl* doc;       /* Pointer back to doc for error reporting */
#endif 
};


/* Lexer Functions
*/

/* choose what version to use for new doctype */
int TY_(HTMLVersion)( TidyDocImpl* doc );

/* everything is allowed in proprietary version of HTML */
/* this is handled here rather than in the tag/attr dicts */

void TY_(ConstrainVersion)( TidyDocImpl* doc, uint vers );

Bool TY_(IsWhite)(uint c);
Bool TY_(IsDigit)(uint c);
Bool TY_(IsLetter)(uint c);
Bool TY_(IsNewline)(uint c);
Bool TY_(IsNamechar)(uint c);
Bool TY_(IsXMLLetter)(uint c);
Bool TY_(IsXMLNamechar)(uint c);

/* Bool IsLower(uint c); */
Bool TY_(IsUpper)(uint c);
uint TY_(ToLower)(uint c);
uint TY_(ToUpper)(uint c);

Lexer* TY_(NewLexer)( TidyDocImpl* doc );
void TY_(FreeLexer)( TidyDocImpl* doc );

/* store character c as UTF-8 encoded byte stream */
void TY_(AddCharToLexer)( Lexer *lexer, uint c );

/*
  Used for elements and text nodes
  element name is NULL for text nodes
  start and end are offsets into lexbuf
  which contains the textual content of
  all elements in the parse tree.

  parent and content allow traversal
  of the parse tree in any direction.
  attributes are represented as a linked
  list of AttVal nodes which hold the
  strings for attribute/value pairs.
*/
Node* TY_(NewNode)( TidyAllocator* allocator, Lexer* lexer );


/* used to clone heading nodes when split by an <HR> */
Node* TY_(CloneNode)( TidyDocImpl* doc, Node *element );

/* free node's attributes */
void TY_(FreeAttrs)( TidyDocImpl* doc, Node *node );

/* doesn't repair attribute list linkage */
void TY_(FreeAttribute)( TidyDocImpl* doc, AttVal *av );

/* detach attribute from node */
void TY_(DetachAttribute)( Node *node, AttVal *attr );

/* detach attribute from node then free it
*/
void TY_(RemoveAttribute)( TidyDocImpl* doc, Node *node, AttVal *attr );

/*
  Free document nodes by iterating through peers and recursing
  through children. Set next to NULL before calling FreeNode()
  to avoid freeing peer nodes. Doesn't patch up prev/next links.
 */
void TY_(FreeNode)( TidyDocImpl* doc, Node *node );

Node* TY_(TextToken)( Lexer *lexer );

/* used for creating preformatted text from Word2000 */
Node* TY_(NewLineNode)( Lexer *lexer );

/* used for adding a &nbsp; for Word2000 */
Node* TY_(NewLiteralTextNode)(Lexer *lexer, ctmbstr txt );

void TY_(AddStringLiteral)( Lexer* lexer, ctmbstr str );
/* void AddStringLiteralLen( Lexer* lexer, ctmbstr str, int len ); */

/* find element */
Node* TY_(FindDocType)( TidyDocImpl* doc );
Node* TY_(FindHTML)( TidyDocImpl* doc );
Node* TY_(FindHEAD)( TidyDocImpl* doc );
Node* TY_(FindTITLE)(TidyDocImpl* doc);
Node* TY_(FindBody)( TidyDocImpl* doc );
Node* TY_(FindXmlDecl)(TidyDocImpl* doc);

/* Returns containing block element, if any */
Node* TY_(FindContainer)( Node* node );

/* add meta element for Tidy */
Bool TY_(AddGenerator)( TidyDocImpl* doc );

uint TY_(ApparentVersion)( TidyDocImpl* doc );

ctmbstr TY_(HTMLVersionNameFromCode)( uint vers, Bool isXhtml );

Bool TY_(WarnMissingSIInEmittedDocType)( TidyDocImpl* doc );

Bool TY_(SetXHTMLDocType)( TidyDocImpl* doc );


/* fixup doctype if missing */
Bool TY_(FixDocType)( TidyDocImpl* doc );

/* ensure XML document starts with <?xml version="1.0"?> */
/* add encoding attribute if not using ASCII or UTF-8 output */
Bool TY_(FixXmlDecl)( TidyDocImpl* doc );

Node* TY_(InferredTag)(TidyDocImpl* doc, TidyTagId id);

void TY_(UngetToken)( TidyDocImpl* doc );


/*
  modes for GetToken()

  MixedContent   -- for elements which don't accept PCDATA
  Preformatted   -- white space preserved as is
  IgnoreMarkup   -- for CDATA elements such as script, style
*/
typedef enum
{
  IgnoreWhitespace,
  MixedContent,
  Preformatted,
  IgnoreMarkup,
  CdataContent
} GetTokenMode;

Node* TY_(GetToken)( TidyDocImpl* doc, GetTokenMode mode );

void TY_(InitMap)(void);


/* create a new attribute */
AttVal* TY_(NewAttribute)( TidyDocImpl* doc );

/* create a new attribute with given name and value */
AttVal* TY_(NewAttributeEx)( TidyDocImpl* doc, ctmbstr name, ctmbstr value,
                             int delim );

/* insert attribute at the end of attribute list of a node */
void TY_(InsertAttributeAtEnd)( Node *node, AttVal *av );

/* insert attribute at the start of attribute list of a node */
void TY_(InsertAttributeAtStart)( Node *node, AttVal *av );

/*************************************
  In-line Stack functions
*************************************/


/* duplicate attributes */
AttVal* TY_(DupAttrs)( TidyDocImpl* doc, AttVal* attrs );

/*
  push a copy of an inline node onto stack
  but don't push if implicit or OBJECT or APPLET
  (implicit tags are ones generated from the istack)

  One issue arises with pushing inlines when
  the tag is already pushed. For instance:

      <p><em>text
      <p><em>more text

  Shouldn't be mapped to

      <p><em>text</em></p>
      <p><em><em>more text</em></em>
*/
void TY_(PushInline)( TidyDocImpl* doc, Node* node );

/* pop inline stack */
void TY_(PopInline)( TidyDocImpl* doc, Node* node );

Bool TY_(IsPushed)( TidyDocImpl* doc, Node* node );
Bool TY_(IsPushedLast)( TidyDocImpl* doc, Node *element, Node *node );

/*
  This has the effect of inserting "missing" inline
  elements around the contents of blocklevel elements
  such as P, TD, TH, DIV, PRE etc. This procedure is
  called at the start of ParseBlock. when the inline
  stack is not empty, as will be the case in:

    <i><h1>italic heading</h1></i>

  which is then treated as equivalent to

    <h1><i>italic heading</i></h1>

  This is implemented by setting the lexer into a mode
  where it gets tokens from the inline stack rather than
  from the input stream.
*/
int TY_(InlineDup)( TidyDocImpl* doc, Node *node );

/*
 defer duplicates when entering a table or other
 element where the inlines shouldn't be duplicated
*/
void TY_(DeferDup)( TidyDocImpl* doc );
Node* TY_(InsertedToken)( TidyDocImpl* doc );

/* stack manipulation for inline elements */
Bool TY_(SwitchInline)( TidyDocImpl* doc, Node* element, Node* node );
Bool TY_(InlineDup1)( TidyDocImpl* doc, Node* node, Node* element );

#ifdef __cplusplus
}
#endif


#endif /* __LEXER_H__ */
