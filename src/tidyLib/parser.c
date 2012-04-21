/* parser.c -- HTML Parser

  (c) 1998-2007 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.
  
  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2008/03/22 20:02:42 $ 
    $Revision: 1.187 $ 

*/

#include "tidy-int.h"
#include "lexer.h"
#include "parser.h"
#include "message.h"
#include "clean.h"
#include "tags.h"
#include "tmbstr.h"

#ifdef AUTO_INPUT_ENCODING
#include "charsets.h"
#endif

Bool TY_(CheckNodeIntegrity)(Node *node)
{
#ifndef NO_NODE_INTEGRITY_CHECK
    Node *child;

    if (node->prev)
    {
        if (node->prev->next != node)
            return no;
    }

    if (node->next)
    {
        if (node->next == node || node->next->prev != node)
            return no;
    }

    if (node->parent)
    {
        if (node->prev == NULL && node->parent->content != node)
            return no;

        if (node->next == NULL && node->parent->last != node)
            return no;
    }

    for (child = node->content; child; child = child->next)
        if ( child->parent != node || !TY_(CheckNodeIntegrity)(child) )
            return no;

#endif
    return yes;
}

/*
 used to determine how attributes
 without values should be printed
 this was introduced to deal with
 user defined tags e.g. Cold Fusion
*/
Bool TY_(IsNewNode)(Node *node)
{
    if (node && node->tag)
    {
        return (node->tag->model & CM_NEW);
    }
    return yes;
}

void TY_(CoerceNode)(TidyDocImpl* doc, Node *node, TidyTagId tid, Bool obsolete, Bool unexpected)
{
    const Dict* tag = TY_(LookupTagDef)(tid);
    Node* tmp = TY_(InferredTag)(doc, tag->id);

    if (obsolete)
        TY_(ReportWarning)(doc, node, tmp, OBSOLETE_ELEMENT);
    else if (unexpected)
        TY_(ReportError)(doc, node, tmp, REPLACING_UNEX_ELEMENT);
    else
        TY_(ReportNotice)(doc, node, tmp, REPLACING_ELEMENT);

    TidyDocFree(doc, tmp->element);
    TidyDocFree(doc, tmp);

    node->was = node->tag;
    node->tag = tag;
    node->type = StartTag;
    node->implicit = yes;
    TidyDocFree(doc, node->element);
    node->element = TY_(tmbstrdup)(doc->allocator, tag->name);
}

/* extract a node and its children from a markup tree */
Node *TY_(RemoveNode)(Node *node)
{
    if (node->prev)
        node->prev->next = node->next;

    if (node->next)
        node->next->prev = node->prev;

    if (node->parent)
    {
        if (node->parent->content == node)
            node->parent->content = node->next;

        if (node->parent->last == node)
            node->parent->last = node->prev;
    }

    node->parent = node->prev = node->next = NULL;
    return node;
}

/* remove node from markup tree and discard it */
Node *TY_(DiscardElement)( TidyDocImpl* doc, Node *element )
{
    Node *next = NULL;

    if (element)
    {
        next = element->next;
        TY_(RemoveNode)(element);
        TY_(FreeNode)( doc, element);
    }

    return next;
}

/*
 insert "node" into markup tree as the firt element
 of content of "element"
*/
void TY_(InsertNodeAtStart)(Node *element, Node *node)
{
    node->parent = element;

    if (element->content == NULL)
        element->last = node;
    else
        element->content->prev = node;

    node->next = element->content;
    node->prev = NULL;
    element->content = node;
}

/*
 insert "node" into markup tree as the last element
 of content of "element"
*/
void TY_(InsertNodeAtEnd)(Node *element, Node *node)
{
    node->parent = element;
    node->prev = element->last;

    if (element->last != NULL)
        element->last->next = node;
    else
        element->content = node;

    element->last = node;
}

/*
 insert "node" into markup tree in place of "element"
 which is moved to become the child of the node
*/
static void InsertNodeAsParent(Node *element, Node *node)
{
    node->content = element;
    node->last = element;
    node->parent = element->parent;
    element->parent = node;

    if (node->parent->content == element)
        node->parent->content = node;

    if (node->parent->last == element)
        node->parent->last = node;

    node->prev = element->prev;
    element->prev = NULL;

    if (node->prev)
        node->prev->next = node;

    node->next = element->next;
    element->next = NULL;

    if (node->next)
        node->next->prev = node;
}

/* insert "node" into markup tree before "element" */
void TY_(InsertNodeBeforeElement)(Node *element, Node *node)
{
    Node *parent;

    parent = element->parent;
    node->parent = parent;
    node->next = element;
    node->prev = element->prev;
    element->prev = node;

    if (node->prev)
        node->prev->next = node;

    if (parent->content == element)
        parent->content = node;
}

/* insert "node" into markup tree after "element" */
void TY_(InsertNodeAfterElement)(Node *element, Node *node)
{
    Node *parent;

    parent = element->parent;
    node->parent = parent;

    /* AQ - 13 Jan 2000 fix for parent == NULL */
    if (parent != NULL && parent->last == element)
        parent->last = node;
    else
    {
        node->next = element->next;
        /* AQ - 13 Jan 2000 fix for node->next == NULL */
        if (node->next != NULL)
            node->next->prev = node;
    }

    element->next = node;
    node->prev = element;
}

static Bool CanPrune( TidyDocImpl* doc, Node *element )
{
    if ( TY_(nodeIsText)(element) )
        return yes;

    if ( element->content )
        return no;

    if ( element->tag == NULL )
        return no;

    if ( element->tag->model & CM_BLOCK && element->attributes != NULL )
        return no;

    if ( nodeIsA(element) && element->attributes != NULL )
        return no;

    if ( nodeIsP(element) && !cfgBool(doc, TidyDropEmptyParas) )
        return no;

    if ( element->tag->model & CM_ROW )
        return no;

    if ( element->tag->model & CM_EMPTY )
        return no;

    if ( nodeIsAPPLET(element) )
        return no;

    if ( nodeIsOBJECT(element) )
        return no;

    if ( nodeIsSCRIPT(element) && attrGetSRC(element) )
        return no;

    if ( nodeIsTITLE(element) )
        return no;

    /* #433359 - fix by Randy Waki 12 Mar 01 */
    if ( nodeIsIFRAME(element) )
        return no;

    /* fix for bug 770297 */
    if (nodeIsTEXTAREA(element))
        return no;

    if ( attrGetID(element) || attrGetNAME(element) )
        return no;

    /* fix for bug 695408; a better fix would look for unknown and    */
    /* known proprietary attributes that make the element significant */
    if (attrGetDATAFLD(element))
        return no;

    /* fix for bug 723772, don't trim new-...-tags */
    if (element->tag->id == TidyTag_UNKNOWN)
        return no;

    if (nodeIsBODY(element))
        return no;

    if (nodeIsCOLGROUP(element))
        return no;

    return yes;
}

/* return next element */
Node *TY_(TrimEmptyElement)( TidyDocImpl* doc, Node *element )
{
    if ( CanPrune(doc, element) )
    {
       if (element->type != TextNode)
            TY_(ReportNotice)(doc, element, NULL, TRIM_EMPTY_ELEMENT);

        return TY_(DiscardElement)(doc, element);
    }
    return element->next;
}

Node* TY_(DropEmptyElements)(TidyDocImpl* doc, Node* node)
{
    Node* next;

    while (node)
    {
        next = node->next;

        if (node->content)
            TY_(DropEmptyElements)(doc, node->content);

        if (!TY_(nodeIsElement)(node) &&
            !(TY_(nodeIsText)(node) && !(node->start < node->end)))
        {
            node = next;
            continue;
        }

        next = TY_(TrimEmptyElement)(doc, node);
        node = next;
    }

    return node;
}

/* 
  errors in positioning of form start or end tags
  generally require human intervention to fix
*/
static void BadForm( TidyDocImpl* doc )
{
    doc->badForm = yes;
    /* doc->errors++; */
}

/*
  This maps 
       <em>hello </em><strong>world</strong>
  to
       <em>hello</em> <strong>world</strong>

  If last child of element is a text node
  then trim trailing white space character
  moving it to after element's end tag.
*/
static void TrimTrailingSpace( TidyDocImpl* doc, Node *element, Node *last )
{
    Lexer* lexer = doc->lexer;
    byte c;

    if (TY_(nodeIsText)(last))
    {
        if (last->end > last->start)
        {
            c = (byte) lexer->lexbuf[ last->end - 1 ];

            if (   c == ' '
#ifdef COMMENT_NBSP_FIX
                || c == 160
#endif
               )
            {
#ifdef COMMENT_NBSP_FIX
                /* take care with <td>&nbsp;</td> */
                if ( c == 160 && 
                     ( element->tag == doc->tags.tag_td || 
                       element->tag == doc->tags.tag_th )
                   )
                {
                    if (last->end > last->start + 1)
                        last->end -= 1;
                }
                else
#endif
                {
                    last->end -= 1;
                    if ( (element->tag->model & CM_INLINE) &&
                         !(element->tag->model & CM_FIELD) )
                        lexer->insertspace = yes;
                }
            }
        }
    }
}

#if 0
static Node *EscapeTag(Lexer *lexer, Node *element)
{
    Node *node = NewNode(lexer->allocator, lexer);

    node->start = lexer->lexsize;
    AddByte(lexer, '<');

    if (element->type == EndTag)
        AddByte(lexer, '/');

    if (element->element)
    {
        char *p;
        for (p = element->element; *p != '\0'; ++p)
            AddByte(lexer, *p);
    }
    else if (element->type == DocTypeTag)
    {
        uint i;
        AddStringLiteral( lexer, "!DOCTYPE " );
        for (i = element->start; i < element->end; ++i)
            AddByte(lexer, lexer->lexbuf[i]);
    }

    if (element->type == StartEndTag)
        AddByte(lexer, '/');

    AddByte(lexer, '>');
    node->end = lexer->lexsize;

    return node;
}
#endif /* 0 */

/* Only true for text nodes. */
Bool TY_(IsBlank)(Lexer *lexer, Node *node)
{
    Bool isBlank = TY_(nodeIsText)(node);
    if ( isBlank )
        isBlank = ( node->end == node->start ||       /* Zero length */
                    ( node->end == node->start+1      /* or one blank. */
                      && lexer->lexbuf[node->start] == ' ' ) );
    return isBlank;
}

/*
  This maps 
       <p>hello<em> world</em>
  to
       <p>hello <em>world</em>

  Trims initial space, by moving it before the
  start tag, or if this element is the first in
  parent's content, then by discarding the space
*/
static void TrimInitialSpace( TidyDocImpl* doc, Node *element, Node *text )
{
    Lexer* lexer = doc->lexer;
    Node *prev, *node;

    if ( TY_(nodeIsText)(text) && 
         lexer->lexbuf[text->start] == ' ' && 
         text->start < text->end )
    {
        if ( (element->tag->model & CM_INLINE) &&
             !(element->tag->model & CM_FIELD) )
        {
            prev = element->prev;

            if (TY_(nodeIsText)(prev))
            {
                if (prev->end == 0 || lexer->lexbuf[prev->end - 1] != ' ')
                    lexer->lexbuf[(prev->end)++] = ' ';

                ++(element->start);
            }
            else /* create new node */
            {
                node = TY_(NewNode)(lexer->allocator, lexer);
                node->start = (element->start)++;
                node->end = element->start;
                lexer->lexbuf[node->start] = ' ';
                TY_(InsertNodeBeforeElement)(element ,node);
            }
        }

        /* discard the space in current node */
        ++(text->start);
    }
}

static Bool IsPreDescendant(Node* node)
{
    Node *parent = node->parent;

    while (parent)
    {
        if (parent->tag && parent->tag->parser == TY_(ParsePre))
            return yes;

        parent = parent->parent;
    }

    return no;
}

static Bool CleanTrailingWhitespace(TidyDocImpl* doc, Node* node)
{
    Node* next;

    if (!TY_(nodeIsText)(node))
        return no;

    if (node->parent->type == DocTypeTag)
        return no;

    if (IsPreDescendant(node))
        return no;

    if (node->parent->tag && node->parent->tag->parser == TY_(ParseScript))
        return no;

    next = node->next;

    /* <p>... </p> */
    if (!next && !TY_(nodeHasCM)(node->parent, CM_INLINE))
        return yes;

    /* <div><small>... </small><h3>...</h3></div> */
    if (!next && node->parent->next && !TY_(nodeHasCM)(node->parent->next, CM_INLINE))
        return yes;

    if (!next)
        return no;

    if (nodeIsBR(next))
        return yes;

    if (TY_(nodeHasCM)(next, CM_INLINE))
        return no;

    /* <a href='/'>...</a> <p>...</p> */
    if (next->type == StartTag)
        return yes;

    /* <strong>...</strong> <hr /> */
    if (next->type == StartEndTag)
        return yes;

    /* evil adjacent text nodes, Tidy should not generate these :-( */
    if (TY_(nodeIsText)(next) && next->start < next->end
        && TY_(IsWhite)(doc->lexer->lexbuf[next->start]))
        return yes;

    return no;
}

static Bool CleanLeadingWhitespace(TidyDocImpl* ARG_UNUSED(doc), Node* node)
{
    if (!TY_(nodeIsText)(node))
        return no;

    if (node->parent->type == DocTypeTag)
        return no;

    if (IsPreDescendant(node))
        return no;

    if (node->parent->tag && node->parent->tag->parser == TY_(ParseScript))
        return no;

    /* <p>...<br> <em>...</em>...</p> */
    if (nodeIsBR(node->prev))
        return yes;

    /* <p> ...</p> */
    if (node->prev == NULL && !TY_(nodeHasCM)(node->parent, CM_INLINE))
        return yes;

    /* <h4>...</h4> <em>...</em> */
    if (node->prev && !TY_(nodeHasCM)(node->prev, CM_INLINE) &&
        TY_(nodeIsElement)(node->prev))
        return yes;

    /* <p><span> ...</span></p> */
    if (!node->prev && !node->parent->prev && !TY_(nodeHasCM)(node->parent->parent, CM_INLINE))
        return yes;

    return no;
}

static void CleanSpaces(TidyDocImpl* doc, Node* node)
{
    Node* next;

    while (node)
    {
        next = node->next;

        if (TY_(nodeIsText)(node) && CleanLeadingWhitespace(doc, node))
            while (node->start < node->end && TY_(IsWhite)(doc->lexer->lexbuf[node->start]))
                ++(node->start);

        if (TY_(nodeIsText)(node) && CleanTrailingWhitespace(doc, node))
            while (node->end > node->start && TY_(IsWhite)(doc->lexer->lexbuf[node->end - 1]))
                --(node->end);

        if (TY_(nodeIsText)(node) && !(node->start < node->end))
        {
            TY_(RemoveNode)(node);
            TY_(FreeNode)(doc, node);
            node = next;

            continue;
        }

        if (node->content)
            CleanSpaces(doc, node->content);

        node = next;
    }
}

/* 
  Move initial and trailing space out.
  This routine maps:

       hello<em> world</em>
  to
       hello <em>world</em>
  and
       <em>hello </em><strong>world</strong>
  to
       <em>hello</em> <strong>world</strong>
*/
static void TrimSpaces( TidyDocImpl* doc, Node *element)
{
    Node* text = element->content;

    if (nodeIsPRE(element) || IsPreDescendant(element))
        return;

    if (TY_(nodeIsText)(text))
        TrimInitialSpace(doc, element, text);

    text = element->last;

    if (TY_(nodeIsText)(text))
        TrimTrailingSpace(doc, element, text);
}

static Bool DescendantOf( Node *element, TidyTagId tid )
{
    Node *parent;
    for ( parent = element->parent;
          parent != NULL;
          parent = parent->parent )
    {
        if ( TagIsId(parent, tid) )
            return yes;
    }
    return no;
}

static Bool InsertMisc(Node *element, Node *node)
{
    if (node->type == CommentTag ||
        node->type == ProcInsTag ||
        node->type == CDATATag ||
        node->type == SectionTag ||
        node->type == AspTag ||
        node->type == JsteTag ||
        node->type == PhpTag )
    {
        TY_(InsertNodeAtEnd)(element, node);
        return yes;
    }

    if ( node->type == XmlDecl )
    {
        Node* root = element;
        while ( root && root->parent )
            root = root->parent;
        if ( root && !(root->content && root->content->type == XmlDecl))
        {
          TY_(InsertNodeAtStart)( root, node );
          return yes;
        }
    }

    /* Declared empty tags seem to be slipping through
    ** the cracks.  This is an experiment to figure out
    ** a decent place to pick them up.
    */
    if ( node->tag &&
         TY_(nodeIsElement)(node) &&
         TY_(nodeCMIsEmpty)(node) && TagId(node) == TidyTag_UNKNOWN &&
         (node->tag->versions & VERS_PROPRIETARY) != 0 )
    {
        TY_(InsertNodeAtEnd)(element, node);
        return yes;
    }

    return no;
}


static void ParseTag( TidyDocImpl* doc, Node *node, GetTokenMode mode )
{
    Lexer* lexer = doc->lexer;
    /*
       Fix by GLP 2000-12-21.  Need to reset insertspace if this 
       is both a non-inline and empty tag (base, link, meta, isindex, hr, area).
    */
    if (node->tag->model & CM_EMPTY)
    {
        lexer->waswhite = no;
        if (node->tag->parser == NULL)
            return;
    }
    else if (!(node->tag->model & CM_INLINE))
        lexer->insertspace = no;

    if (node->tag->parser == NULL)
        return;

    if (node->type == StartEndTag)
        return;

    (*node->tag->parser)( doc, node, mode );
}

/*
 the doctype has been found after other tags,
 and needs moving to before the html element
*/
static void InsertDocType( TidyDocImpl* doc, Node *element, Node *doctype )
{
    Node* existing = TY_(FindDocType)( doc );
    if ( existing )
    {
        TY_(ReportError)(doc, element, doctype, DISCARDING_UNEXPECTED );
        TY_(FreeNode)( doc, doctype );
    }
    else
    {
        TY_(ReportError)(doc, element, doctype, DOCTYPE_AFTER_TAGS );
        while ( !nodeIsHTML(element) )
            element = element->parent;
        TY_(InsertNodeBeforeElement)( element, doctype );
    }
}

/*
 move node to the head, where element is used as starting
 point in hunt for head. normally called during parsing
*/
static void MoveToHead( TidyDocImpl* doc, Node *element, Node *node )
{
    Node *head;

    TY_(RemoveNode)( node );  /* make sure that node is isolated */

    if ( TY_(nodeIsElement)(node) )
    {
        TY_(ReportError)(doc, element, node, TAG_NOT_ALLOWED_IN );

        head = TY_(FindHEAD)(doc);
        assert(head != NULL);

        TY_(InsertNodeAtEnd)(head, node);

        if ( node->tag->parser )
            ParseTag( doc, node, IgnoreWhitespace );
    }
    else
    {
        TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED);
        TY_(FreeNode)( doc, node );
    }
}

/* moves given node to end of body element */
static void MoveNodeToBody( TidyDocImpl* doc, Node* node )
{
    Node* body = TY_(FindBody)( doc );
    if ( body )
    {
        TY_(RemoveNode)( node );
        TY_(InsertNodeAtEnd)( body, node );
    }
}

static void AddClassNoIndent( TidyDocImpl* doc, Node *node )
{
    ctmbstr sprop =
        "padding-left: 2ex; margin-left: 0ex"
        "; margin-top: 0ex; margin-bottom: 0ex";
    if ( !cfgBool(doc, TidyDecorateInferredUL) )
        return;
    if ( cfgBool(doc, TidyMakeClean) )
        TY_(AddStyleAsClass)( doc, node, sprop );
    else
        TY_(AddStyleProperty)( doc, node, sprop );
}

/*
   element is node created by the lexer
   upon seeing the start tag, or by the
   parser when the start tag is inferred
*/
void TY_(ParseBlock)( TidyDocImpl* doc, Node *element, GetTokenMode mode)
{
    Lexer* lexer = doc->lexer;
    Node *node;
    Bool checkstack = yes;
    uint istackbase = 0;

    if ( element->tag->model & CM_EMPTY )
        return;

    if ( nodeIsFORM(element) && 
         DescendantOf(element, TidyTag_FORM) )
        TY_(ReportError)(doc, element, NULL, ILLEGAL_NESTING );

    /*
     InlineDup() asks the lexer to insert inline emphasis tags
     currently pushed on the istack, but take care to avoid
     propagating inline emphasis inside OBJECT or APPLET.
     For these elements a fresh inline stack context is created
     and disposed of upon reaching the end of the element.
     They thus behave like table cells in this respect.
    */
    if (element->tag->model & CM_OBJECT)
    {
        istackbase = lexer->istackbase;
        lexer->istackbase = lexer->istacksize;
    }

    if (!(element->tag->model & CM_MIXED))
        TY_(InlineDup)( doc, NULL );

    mode = IgnoreWhitespace;

    while ((node = TY_(GetToken)(doc, mode /*MixedContent*/)) != NULL)
    {
        /* end tag for this element */
        if (node->type == EndTag && node->tag &&
            (node->tag == element->tag || element->was == node->tag))
        {
            TY_(FreeNode)( doc, node );

            if (element->tag->model & CM_OBJECT)
            {
                /* pop inline stack */
                while (lexer->istacksize > lexer->istackbase)
                    TY_(PopInline)( doc, NULL );
                lexer->istackbase = istackbase;
            }

            element->closed = yes;
            TrimSpaces( doc, element );
            return;
        }

        if ( nodeIsBODY( node ) && DescendantOf( element, TidyTag_HEAD ))
        {
            /*  If we're in the HEAD, close it before proceeding.
                This is an extremely rare occurance, but has been observed.
            */
            TY_(UngetToken)( doc );
            break;
        }

        if ( nodeIsHTML(node) || nodeIsHEAD(node) || nodeIsBODY(node) )
        {
            if ( TY_(nodeIsElement)(node) )
                TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
            TY_(FreeNode)( doc, node );
            continue;
        }


        if (node->type == EndTag)
        {
            if (node->tag == NULL)
            {
                TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
                TY_(FreeNode)( doc, node );
                continue;
            }
            else if ( nodeIsBR(node) )
                node->type = StartTag;
            else if ( nodeIsP(node) )
            {
                /* Cannot have a block inside a paragraph, so no checking
                   for an ancestor is necessary -- but we _can_ have
                   paragraphs inside a block, so change it to an implicit
                   empty paragraph, to be dealt with according to the user's
                   options
                */
                node->type = StartEndTag;
                node->implicit = yes;
#if OBSOLETE
                TY_(CoerceNode)(doc, node, TidyTag_BR, no, no);
                TY_(FreeAttrs)( doc, node ); /* discard align attribute etc. */
                TY_(InsertNodeAtEnd)( element, node );
                node = InferredTag(doc, TidyTag_BR);
#endif
            }
            else if (DescendantOf( element, node->tag->id ))
            {
                /* 
                  if this is the end tag for an ancestor element
                  then infer end tag for this element
                */
                TY_(UngetToken)( doc );
                break;
#if OBSOLETE
                Node *parent;
                for ( parent = element->parent;
                      parent != NULL; 
                      parent = parent->parent )
                {
                    if (node->tag == parent->tag)
                    {
                        if (!(element->tag->model & CM_OPT))
                            TY_(ReportError)(doc, element, node, MISSING_ENDTAG_BEFORE );

                        TY_(UngetToken)( doc );

                        if (element->tag->model & CM_OBJECT)
                        {
                            /* pop inline stack */
                            while (lexer->istacksize > lexer->istackbase)
                                TY_(PopInline)( doc, NULL );
                            lexer->istackbase = istackbase;
                        }

                        TrimSpaces( doc, element );
                        return;
                    }
                }
#endif
            }
            else
            {
                /* special case </tr> etc. for stuff moved in front of table */
                if ( lexer->exiled
                     && (TY_(nodeHasCM)(node, CM_TABLE) || nodeIsTABLE(node)) )
                {
                    TY_(UngetToken)( doc );
                    TrimSpaces( doc, element );
                    return;
                }
            }
        }

        /* mixed content model permits text */
        if (TY_(nodeIsText)(node))
        {
            if ( checkstack )
            {
                checkstack = no;
                if (!(element->tag->model & CM_MIXED))
                {
                    if ( TY_(InlineDup)(doc, node) > 0 )
                        continue;
                }
            }

            TY_(InsertNodeAtEnd)(element, node);
            mode = MixedContent;

            /*
              HTML4 strict doesn't allow mixed content for
              elements with %block; as their content model
            */
            /*
              But only body, map, blockquote, form and
              noscript have content model %block;
            */
            if ( nodeIsBODY(element)       ||
                 nodeIsMAP(element)        ||
                 nodeIsBLOCKQUOTE(element) ||
                 nodeIsFORM(element)       ||
                 nodeIsNOSCRIPT(element) )
                TY_(ConstrainVersion)( doc, ~VERS_HTML40_STRICT );
            continue;
        }

        if ( InsertMisc(element, node) )
            continue;

        /* allow PARAM elements? */
        if ( nodeIsPARAM(node) )
        {
            if ( TY_(nodeHasCM)(element, CM_PARAM) && TY_(nodeIsElement)(node) )
            {
                TY_(InsertNodeAtEnd)(element, node);
                continue;
            }

            /* otherwise discard it */
            TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
            TY_(FreeNode)( doc, node );
            continue;
        }

        /* allow AREA elements? */
        if ( nodeIsAREA(node) )
        {
            if ( nodeIsMAP(element) && TY_(nodeIsElement)(node) )
            {
                TY_(InsertNodeAtEnd)(element, node);
                continue;
            }

            /* otherwise discard it */
            TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
            TY_(FreeNode)( doc, node );
            continue;
        }

        /* ignore unknown start/end tags */
        if ( node->tag == NULL )
        {
            TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
            TY_(FreeNode)( doc, node );
            continue;
        }

        /*
          Allow CM_INLINE elements here.

          Allow CM_BLOCK elements here unless
          lexer->excludeBlocks is yes.

          LI and DD are special cased.

          Otherwise infer end tag for this element.
        */

        if ( !TY_(nodeHasCM)(node, CM_INLINE) )
        {
            if ( !TY_(nodeIsElement)(node) )
            {
                if ( nodeIsFORM(node) )
                    BadForm( doc );

                TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
                TY_(FreeNode)( doc, node );
                continue;
            }

            /* #427671 - Fix by Randy Waki - 10 Aug 00 */
            /*
             If an LI contains an illegal FRAME, FRAMESET, OPTGROUP, or OPTION
             start tag, discard the start tag and let the subsequent content get
             parsed as content of the enclosing LI.  This seems to mimic IE and
             Netscape, and avoids an infinite loop: without this check,
             ParseBlock (which is parsing the LI's content) and ParseList (which
             is parsing the LI's parent's content) repeatedly defer to each
             other to parse the illegal start tag, each time inferring a missing
             </li> or <li> respectively.

             NOTE: This check is a bit fragile.  It specifically checks for the
             four tags that happen to weave their way through the current series
             of tests performed by ParseBlock and ParseList to trigger the
             infinite loop.
            */
            if ( nodeIsLI(element) )
            {
                if ( nodeIsFRAME(node)    ||
                     nodeIsFRAMESET(node) ||
                     nodeIsOPTGROUP(node) ||
                     nodeIsOPTION(node) )
                {
                    TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
                    TY_(FreeNode)( doc, node );  /* DSR - 27Apr02 avoid memory leak */
                    continue;
                }
            }

            if ( nodeIsTD(element) || nodeIsTH(element) )
            {
                /* if parent is a table cell, avoid inferring the end of the cell */

                if ( TY_(nodeHasCM)(node, CM_HEAD) )
                {
                    MoveToHead( doc, element, node );
                    continue;
                }

                if ( TY_(nodeHasCM)(node, CM_LIST) )
                {
                    TY_(UngetToken)( doc );
                    node = TY_(InferredTag)(doc, TidyTag_UL);
                    AddClassNoIndent(doc, node);
                    lexer->excludeBlocks = yes;
                }
                else if ( TY_(nodeHasCM)(node, CM_DEFLIST) )
                {
                    TY_(UngetToken)( doc );
                    node = TY_(InferredTag)(doc, TidyTag_DL);
                    lexer->excludeBlocks = yes;
                }

                /* infer end of current table cell */
                if ( !TY_(nodeHasCM)(node, CM_BLOCK) )
                {
                    TY_(UngetToken)( doc );
                    TrimSpaces( doc, element );
                    return;
                }
            }
            else if ( TY_(nodeHasCM)(node, CM_BLOCK) )
            {
                if ( lexer->excludeBlocks )
                {
                    if ( !TY_(nodeHasCM)(element, CM_OPT) )
                        TY_(ReportError)(doc, element, node, MISSING_ENDTAG_BEFORE );

                    TY_(UngetToken)( doc );

                    if ( TY_(nodeHasCM)(element, CM_OBJECT) )
                        lexer->istackbase = istackbase;

                    TrimSpaces( doc, element );
                    return;
                }
            }
            else /* things like list items */
            {
                if (node->tag->model & CM_HEAD)
                {
                    MoveToHead( doc, element, node );
                    continue;
                }

                /*
                 special case where a form start tag
                 occurs in a tr and is followed by td or th
                */

                if ( nodeIsFORM(element) &&
                     nodeIsTD(element->parent) &&
                     element->parent->implicit )
                {
                    if ( nodeIsTD(node) )
                    {
                        TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
                        TY_(FreeNode)( doc, node );
                        continue;
                    }

                    if ( nodeIsTH(node) )
                    {
                        TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
                        TY_(FreeNode)( doc, node );
                        node = element->parent;
                        TidyDocFree(doc, node->element);
                        node->element = TY_(tmbstrdup)(doc->allocator, "th");
                        node->tag = TY_(LookupTagDef)( TidyTag_TH );
                        continue;
                    }
                }

                if ( !TY_(nodeHasCM)(element, CM_OPT) && !element->implicit )
                    TY_(ReportError)(doc, element, node, MISSING_ENDTAG_BEFORE );

                TY_(UngetToken)( doc );

                if ( TY_(nodeHasCM)(node, CM_LIST) )
                {
                    if ( element->parent && element->parent->tag &&
                         element->parent->tag->parser == TY_(ParseList) )
                    {
                        TrimSpaces( doc, element );
                        return;
                    }

                    node = TY_(InferredTag)(doc, TidyTag_UL);
                    AddClassNoIndent(doc, node);
                }
                else if ( TY_(nodeHasCM)(node, CM_DEFLIST) )
                {
                    if ( nodeIsDL(element->parent) )
                    {
                        TrimSpaces( doc, element );
                        return;
                    }

                    node = TY_(InferredTag)(doc, TidyTag_DL);
                }
                else if ( TY_(nodeHasCM)(node, CM_TABLE) || TY_(nodeHasCM)(node, CM_ROW) )
                {
                    /* http://tidy.sf.net/issue/1316307 */
                    /* In exiled mode, return so table processing can 
                       continue. */
                    if (lexer->exiled)
                        return;
                    node = TY_(InferredTag)(doc, TidyTag_TABLE);
                }
                else if ( TY_(nodeHasCM)(element, CM_OBJECT) )
                {
                    /* pop inline stack */
                    while ( lexer->istacksize > lexer->istackbase )
                        TY_(PopInline)( doc, NULL );
                    lexer->istackbase = istackbase;
                    TrimSpaces( doc, element );
                    return;

                }
                else
                {
                    TrimSpaces( doc, element );
                    return;
                }
            }
        }

        /* parse known element */
        if (TY_(nodeIsElement)(node))
        {
            if (node->tag->model & CM_INLINE)
            {
                if (checkstack && !node->implicit)
                {
                    checkstack = no;

                    if (!(element->tag->model & CM_MIXED)) /* #431731 - fix by Randy Waki 25 Dec 00 */
                    {
                        if ( TY_(InlineDup)(doc, node) > 0 )
                            continue;
                    }
                }

                mode = MixedContent;
            }
            else
            {
                checkstack = yes;
                mode = IgnoreWhitespace;
            }

            /* trim white space before <br> */
            if ( nodeIsBR(node) )
                TrimSpaces( doc, element );

            TY_(InsertNodeAtEnd)(element, node);
            
            if (node->implicit)
                TY_(ReportError)(doc, element, node, INSERTING_TAG );

            ParseTag( doc, node, IgnoreWhitespace /*MixedContent*/ );
            continue;
        }

        /* discard unexpected tags */
        if (node->type == EndTag)
            TY_(PopInline)( doc, node );  /* if inline end tag */

        TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
        TY_(FreeNode)( doc, node );
        continue;
    }

    if (!(element->tag->model & CM_OPT))
        TY_(ReportError)(doc, element, node, MISSING_ENDTAG_FOR);

    if (element->tag->model & CM_OBJECT)
    {
        /* pop inline stack */
        while ( lexer->istacksize > lexer->istackbase )
            TY_(PopInline)( doc, NULL );
        lexer->istackbase = istackbase;
    }

    TrimSpaces( doc, element );
}

void TY_(ParseInline)( TidyDocImpl* doc, Node *element, GetTokenMode mode )
{
    Lexer* lexer = doc->lexer;
    Node *node, *parent;

    if (element->tag->model & CM_EMPTY)
        return;

    /*
     ParseInline is used for some block level elements like H1 to H6
     For such elements we need to insert inline emphasis tags currently
     on the inline stack. For Inline elements, we normally push them
     onto the inline stack provided they aren't implicit or OBJECT/APPLET.
     This test is carried out in PushInline and PopInline, see istack.c

     InlineDup(...) is not called for elements with a CM_MIXED (inline and
     block) content model, e.g. <del> or <ins>, otherwise constructs like 

       <p>111<a name='foo'>222<del>333</del>444</a>555</p>
       <p>111<span>222<del>333</del>444</span>555</p>
       <p>111<em>222<del>333</del>444</em>555</p>

     will get corrupted.
    */
    if ((TY_(nodeHasCM)(element, CM_BLOCK) || nodeIsDT(element)) &&
        !TY_(nodeHasCM)(element, CM_MIXED))
        TY_(InlineDup)(doc, NULL);
    else if (TY_(nodeHasCM)(element, CM_INLINE))
        TY_(PushInline)(doc, element);

    if ( nodeIsNOBR(element) )
        doc->badLayout |= USING_NOBR;
    else if ( nodeIsFONT(element) )
        doc->badLayout |= USING_FONT;

    /* Inline elements may or may not be within a preformatted element */
    if (mode != Preformatted)
        mode = MixedContent;

    while ((node = TY_(GetToken)(doc, mode)) != NULL)
    {
        /* end tag for current element */
        if (node->tag == element->tag && node->type == EndTag)
        {
            if (element->tag->model & CM_INLINE)
                TY_(PopInline)( doc, node );

            TY_(FreeNode)( doc, node );

            if (!(mode & Preformatted))
                TrimSpaces(doc, element);

            /*
             if a font element wraps an anchor and nothing else
             then move the font element inside the anchor since
             otherwise it won't alter the anchor text color
            */
            if ( nodeIsFONT(element) && 
                 element->content && element->content == element->last )
            {
                Node *child = element->content;

                if ( nodeIsA(child) )
                {
                    child->parent = element->parent;
                    child->next = element->next;
                    child->prev = element->prev;

                    element->next = NULL;
                    element->prev = NULL;
                    element->parent = child;

                    element->content = child->content;
                    element->last = child->last;
                    child->content = element;

                    TY_(FixNodeLinks)(child);
                    TY_(FixNodeLinks)(element);
                }
            }

            element->closed = yes;
            TrimSpaces( doc, element );
            return;
        }

        /* <u>...<u>  map 2nd <u> to </u> if 1st is explicit */
        /* (see additional conditions below) */
        /* otherwise emphasis nesting is probably unintentional */
        /* big, small, sub, sup have cumulative effect to leave them alone */
        if ( node->type == StartTag
             && node->tag == element->tag
             && TY_(IsPushed)( doc, node )
             && !node->implicit
             && !element->implicit
             && node->tag && (node->tag->model & CM_INLINE)
             && !nodeIsA(node)
             && !nodeIsFONT(node)
             && !nodeIsBIG(node)
             && !nodeIsSMALL(node)
             && !nodeIsSUB(node)
             && !nodeIsSUP(node)
             && !nodeIsQ(node)
             && !nodeIsSPAN(node)
           )
        {
            /* proceeds only if "node" does not have any attribute and
               follows a text node not finishing with a space */
            if (element->content != NULL && node->attributes == NULL
                && TY_(nodeIsText)(element->last)
                && !TY_(TextNodeEndWithSpace)(doc->lexer, element->last) )
            {
                TY_(ReportWarning)(doc, element, node, COERCE_TO_ENDTAG_WARN);
                node->type = EndTag;
                TY_(UngetToken)(doc);
                continue;
            }

            if (node->attributes == NULL || element->attributes == NULL)
                TY_(ReportWarning)(doc, element, node, NESTED_EMPHASIS);
        }
        else if ( TY_(IsPushed)(doc, node) && node->type == StartTag && 
                  nodeIsQ(node) )
        {
            TY_(ReportWarning)(doc, element, node, NESTED_QUOTATION);
        }

        if ( TY_(nodeIsText)(node) )
        {
            /* only called for 1st child */
            if ( element->content == NULL && !(mode & Preformatted) )
                TrimSpaces( doc, element );

            if ( node->start >= node->end )
            {
                TY_(FreeNode)( doc, node );
                continue;
            }

            TY_(InsertNodeAtEnd)(element, node);
            continue;
        }

        /* mixed content model so allow text */
        if (InsertMisc(element, node))
            continue;

        /* deal with HTML tags */
        if ( nodeIsHTML(node) )
        {
            if ( TY_(nodeIsElement)(node) )
            {
                TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED );
                TY_(FreeNode)( doc, node );
                continue;
            }

            /* otherwise infer end of inline element */
            TY_(UngetToken)( doc );

            if (!(mode & Preformatted))
                TrimSpaces(doc, element);

            return;
        }

        /* within <dt> or <pre> map <p> to <br> */
        if ( nodeIsP(node) &&
             node->type == StartTag &&
             ( (mode & Preformatted) ||
               nodeIsDT(element) || 
               DescendantOf(element, TidyTag_DT )
             )
           )
        {
            node->tag = TY_(LookupTagDef)( TidyTag_BR );
            TidyDocFree(doc, node->element);
            node->element = TY_(tmbstrdup)(doc->allocator, "br");
            TrimSpaces(doc, element);
            TY_(InsertNodeAtEnd)(element, node);
            continue;
        }

        /* <p> allowed within <address> in HTML 4.01 Transitional */
        if ( nodeIsP(node) &&
             node->type == StartTag &&
             nodeIsADDRESS(element) )
        {
            TY_(ConstrainVersion)( doc, ~VERS_HTML40_STRICT );
            TY_(InsertNodeAtEnd)(element, node);
            (*node->tag->parser)( doc, node, mode );
            continue;
        }

        /* ignore unknown and PARAM tags */
        if ( node->tag == NULL || nodeIsPARAM(node) )
        {
            TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node );
            continue;
        }

        if ( nodeIsBR(node) && node->type == EndTag )
            node->type = StartTag;

        if ( node->type == EndTag )
        {
           /* coerce </br> to <br> */
           if ( nodeIsBR(node) )
                node->type = StartTag;
           else if ( nodeIsP(node) )
           {
               /* coerce unmatched </p> to <br><br> */
                if ( !DescendantOf(element, TidyTag_P) )
                {
                    TY_(CoerceNode)(doc, node, TidyTag_BR, no, no);
                    TrimSpaces( doc, element );
                    TY_(InsertNodeAtEnd)( element, node );
                    node = TY_(InferredTag)(doc, TidyTag_BR);
                    TY_(InsertNodeAtEnd)( element, node ); /* todo: check this */
                    continue;
                }
           }
           else if ( TY_(nodeHasCM)(node, CM_INLINE)
                     && !nodeIsA(node)
                     && !TY_(nodeHasCM)(node, CM_OBJECT)
                     && TY_(nodeHasCM)(element, CM_INLINE) )
            {
                /* allow any inline end tag to end current element */

                /* http://tidy.sf.net/issue/1426419 */
                /* but, like the browser, retain an earlier inline element.
                   This is implemented by setting the lexer into a mode
                   where it gets tokens from the inline stack rather than
                   from the input stream. Check if the scenerio fits. */
                if ( !nodeIsA(element)
                     && (node->tag != element->tag)
                     && TY_(IsPushed)( doc, node )
                     && TY_(IsPushed)( doc, element ) )
                {
                    /* we have something like
                       <b>bold <i>bold and italic</b> italics</i> */
                    if ( TY_(SwitchInline)( doc, element, node ) )
                    {
                        TY_(ReportError)(doc, element, node, NON_MATCHING_ENDTAG);
                        TY_(UngetToken)( doc ); /* put this back */
                        TY_(InlineDup1)( doc, NULL, element ); /* dupe the <i>, after </b> */
                        if (!(mode & Preformatted))
                            TrimSpaces( doc, element );
                        return; /* close <i>, but will re-open it, after </b> */
                    }
                }
                TY_(PopInline)( doc, element );

                if ( !nodeIsA(element) )
                {
                    if ( nodeIsA(node) && node->tag != element->tag )
                    {
                       TY_(ReportError)(doc, element, node, MISSING_ENDTAG_BEFORE );
                       TY_(UngetToken)( doc );
                    }
                    else
                    {
                        TY_(ReportError)(doc, element, node, NON_MATCHING_ENDTAG);
                        TY_(FreeNode)( doc, node);
                    }

                    if (!(mode & Preformatted))
                        TrimSpaces(doc, element);

                    return;
                }

                /* if parent is <a> then discard unexpected inline end tag */
                TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }  /* special case </tr> etc. for stuff moved in front of table */
            else if ( lexer->exiled
                     && (TY_(nodeHasCM)(node, CM_TABLE) || nodeIsTABLE(node)) )
            {
                TY_(UngetToken)( doc );
                TrimSpaces(doc, element);
                return;
            }
        }

        /* allow any header tag to end current header */
        if ( TY_(nodeHasCM)(node, CM_HEADING) && TY_(nodeHasCM)(element, CM_HEADING) )
        {

            if ( node->tag == element->tag )
            {
                TY_(ReportError)(doc, element, node, NON_MATCHING_ENDTAG );
                TY_(FreeNode)( doc, node);
            }
            else
            {
                TY_(ReportError)(doc, element, node, MISSING_ENDTAG_BEFORE );
                TY_(UngetToken)( doc );
            }

            if (!(mode & Preformatted))
                TrimSpaces(doc, element);

            return;
        }

        /*
           an <A> tag to ends any open <A> element
           but <A href=...> is mapped to </A><A href=...>
        */
        /* #427827 - fix by Randy Waki and Bjoern Hoehrmann 23 Aug 00 */
        /* if (node->tag == doc->tags.tag_a && !node->implicit && TY_(IsPushed)(doc, node)) */
        if ( nodeIsA(node) && !node->implicit && 
             (nodeIsA(element) || DescendantOf(element, TidyTag_A)) )
        {
            /* coerce <a> to </a> unless it has some attributes */
            /* #427827 - fix by Randy Waki and Bjoern Hoehrmann 23 Aug 00 */
            /* other fixes by Dave Raggett */
            /* if (node->attributes == NULL) */
            if (node->type != EndTag && node->attributes == NULL)
            {
                node->type = EndTag;
                TY_(ReportError)(doc, element, node, COERCE_TO_ENDTAG);
                /* TY_(PopInline)( doc, node ); */
                TY_(UngetToken)( doc );
                continue;
            }

            TY_(UngetToken)( doc );
            TY_(ReportError)(doc, element, node, MISSING_ENDTAG_BEFORE);
            /* TY_(PopInline)( doc, element ); */

            if (!(mode & Preformatted))
                TrimSpaces(doc, element);

            return;
        }

        if (element->tag->model & CM_HEADING)
        {
            if ( nodeIsCENTER(node) || nodeIsDIV(node) )
            {
                if (!TY_(nodeIsElement)(node))
                {
                    TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED);
                    TY_(FreeNode)( doc, node);
                    continue;
                }

                TY_(ReportError)(doc, element, node, TAG_NOT_ALLOWED_IN);

                /* insert center as parent if heading is empty */
                if (element->content == NULL)
                {
                    InsertNodeAsParent(element, node);
                    continue;
                }

                /* split heading and make center parent of 2nd part */
                TY_(InsertNodeAfterElement)(element, node);

                if (!(mode & Preformatted))
                    TrimSpaces(doc, element);

                element = TY_(CloneNode)( doc, element );
                TY_(InsertNodeAtEnd)(node, element);
                continue;
            }

            if ( nodeIsHR(node) )
            {
                if ( !TY_(nodeIsElement)(node) )
                {
                    TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED);
                    TY_(FreeNode)( doc, node);
                    continue;
                }

                TY_(ReportError)(doc, element, node, TAG_NOT_ALLOWED_IN);

                /* insert hr before heading if heading is empty */
                if (element->content == NULL)
                {
                    TY_(InsertNodeBeforeElement)(element, node);
                    continue;
                }

                /* split heading and insert hr before 2nd part */
                TY_(InsertNodeAfterElement)(element, node);

                if (!(mode & Preformatted))
                    TrimSpaces(doc, element);

                element = TY_(CloneNode)( doc, element );
                TY_(InsertNodeAfterElement)(node, element);
                continue;
            }
        }

        if ( nodeIsDT(element) )
        {
            if ( nodeIsHR(node) )
            {
                Node *dd;
                if ( !TY_(nodeIsElement)(node) )
                {
                    TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED);
                    TY_(FreeNode)( doc, node);
                    continue;
                }

                TY_(ReportError)(doc, element, node, TAG_NOT_ALLOWED_IN);
                dd = TY_(InferredTag)(doc, TidyTag_DD);

                /* insert hr within dd before dt if dt is empty */
                if (element->content == NULL)
                {
                    TY_(InsertNodeBeforeElement)(element, dd);
                    TY_(InsertNodeAtEnd)(dd, node);
                    continue;
                }

                /* split dt and insert hr within dd before 2nd part */
                TY_(InsertNodeAfterElement)(element, dd);
                TY_(InsertNodeAtEnd)(dd, node);

                if (!(mode & Preformatted))
                    TrimSpaces(doc, element);

                element = TY_(CloneNode)( doc, element );
                TY_(InsertNodeAfterElement)(dd, element);
                continue;
            }
        }


        /* 
          if this is the end tag for an ancestor element
          then infer end tag for this element
        */
        if (node->type == EndTag)
        {
            for (parent = element->parent;
                    parent != NULL; parent = parent->parent)
            {
                if (node->tag == parent->tag)
                {
                    if (!(element->tag->model & CM_OPT) && !element->implicit)
                        TY_(ReportError)(doc, element, node, MISSING_ENDTAG_BEFORE);

                    if( TY_(IsPushedLast)( doc, element, node ) ) 
                        TY_(PopInline)( doc, element );
                    TY_(UngetToken)( doc );

                    if (!(mode & Preformatted))
                        TrimSpaces(doc, element);

                    return;
                }
            }
        }

        /* block level tags end this element */
        if (!(node->tag->model & CM_INLINE) &&
            !(element->tag->model & CM_MIXED))
        {
            if ( !TY_(nodeIsElement)(node) )
            {
                TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }

            if (!(element->tag->model & CM_OPT))
                TY_(ReportError)(doc, element, node, MISSING_ENDTAG_BEFORE);

            if (node->tag->model & CM_HEAD && !(node->tag->model & CM_BLOCK))
            {
                MoveToHead(doc, element, node);
                continue;
            }

            /*
               prevent anchors from propagating into block tags
               except for headings h1 to h6
            */
            if ( nodeIsA(element) )
            {
                if (node->tag && !(node->tag->model & CM_HEADING))
                    TY_(PopInline)( doc, element );
                else if (!(element->content))
                {
                    TY_(DiscardElement)( doc, element );
                    TY_(UngetToken)( doc );
                    return;
                }
            }

            TY_(UngetToken)( doc );

            if (!(mode & Preformatted))
                TrimSpaces(doc, element);

            return;
        }

        /* parse inline element */
        if (TY_(nodeIsElement)(node))
        {
            if (node->implicit)
                TY_(ReportError)(doc, element, node, INSERTING_TAG);

            /* trim white space before <br> */
            if ( nodeIsBR(node) )
                TrimSpaces(doc, element);
            
            TY_(InsertNodeAtEnd)(element, node);
            ParseTag(doc, node, mode);
            continue;
        }

        /* discard unexpected tags */
        TY_(ReportError)(doc, element, node, DISCARDING_UNEXPECTED);
        TY_(FreeNode)( doc, node );
        continue;
    }

    if (!(element->tag->model & CM_OPT))
        TY_(ReportError)(doc, element, node, MISSING_ENDTAG_FOR);

}

void TY_(ParseEmpty)(TidyDocImpl* doc, Node *element, GetTokenMode mode)
{
    Lexer* lexer = doc->lexer;
    if ( lexer->isvoyager )
    {
        Node *node = TY_(GetToken)( doc, mode);
        if ( node )
        {
            if ( !(node->type == EndTag && node->tag == element->tag) )
            {
                TY_(ReportError)(doc, element, node, ELEMENT_NOT_EMPTY);
                TY_(UngetToken)( doc );
            }
            else
            {
                TY_(FreeNode)( doc, node );
            }
        }
    }
}

void TY_(ParseDefList)(TidyDocImpl* doc, Node *list, GetTokenMode mode)
{
    Lexer* lexer = doc->lexer;
    Node *node, *parent;

    if (list->tag->model & CM_EMPTY)
        return;

    lexer->insert = NULL;  /* defer implicit inline start tags */

    while ((node = TY_(GetToken)( doc, IgnoreWhitespace)) != NULL)
    {
        if (node->tag == list->tag && node->type == EndTag)
        {
            TY_(FreeNode)( doc, node);
            list->closed = yes;
            return;
        }

        /* deal with comments etc. */
        if (InsertMisc(list, node))
            continue;

        if (TY_(nodeIsText)(node))
        {
            TY_(UngetToken)( doc );
            node = TY_(InferredTag)(doc, TidyTag_DT);
            TY_(ReportError)(doc, list, node, MISSING_STARTTAG);
        }

        if (node->tag == NULL)
        {
            TY_(ReportError)(doc, list, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /* 
          if this is the end tag for an ancestor element
          then infer end tag for this element
        */
        if (node->type == EndTag)
        {
            Bool discardIt = no;
            if ( nodeIsFORM(node) )
            {
                BadForm( doc );
                TY_(ReportError)(doc, list, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node );
                continue;
            }

            for (parent = list->parent;
                    parent != NULL; parent = parent->parent)
            {
               /* Do not match across BODY to avoid infinite loop
                  between ParseBody and this parser,
                  See http://tidy.sf.net/bug/1098012. */
                if (nodeIsBODY(parent))
                {
                    discardIt = yes;
                    break;
                }
                if (node->tag == parent->tag)
                {
                    TY_(ReportError)(doc, list, node, MISSING_ENDTAG_BEFORE);

                    TY_(UngetToken)( doc );
                    return;
                }
            }
            if (discardIt)
            {
                TY_(ReportError)(doc, list, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }
        }

        /* center in a dt or a dl breaks the dl list in two */
        if ( nodeIsCENTER(node) )
        {
            if (list->content)
                TY_(InsertNodeAfterElement)(list, node);
            else /* trim empty dl list */
            {
                TY_(InsertNodeBeforeElement)(list, node);

/* #540296 tidy dumps with empty definition list */
#if 0
                TY_(DiscardElement)(list);
#endif
            }

            /* #426885 - fix by Glenn Carroll 19 Apr 00, and
                         Gary Dechaines 11 Aug 00 */
            /* ParseTag can destroy node, if it finds that
             * this <center> is followed immediately by </center>.
             * It's awkward but necessary to determine if this
             * has happened.
             */
            parent = node->parent;

            /* and parse contents of center */
            lexer->excludeBlocks = no;
            ParseTag( doc, node, mode);
            lexer->excludeBlocks = yes;

            /* now create a new dl element,
             * unless node has been blown away because the
             * center was empty, as above.
             */
            if (parent->last == node)
            {
                list = TY_(InferredTag)(doc, TidyTag_DL);
                TY_(InsertNodeAfterElement)(node, list);
            }
            continue;
        }

        if ( !(nodeIsDT(node) || nodeIsDD(node)) )
        {
            TY_(UngetToken)( doc );

            if (!(node->tag->model & (CM_BLOCK | CM_INLINE)))
            {
                TY_(ReportError)(doc, list, node, TAG_NOT_ALLOWED_IN);
                return;
            }

            /* if DD appeared directly in BODY then exclude blocks */
            if (!(node->tag->model & CM_INLINE) && lexer->excludeBlocks)
                return;

            node = TY_(InferredTag)(doc, TidyTag_DD);
            TY_(ReportError)(doc, list, node, MISSING_STARTTAG);
        }

        if (node->type == EndTag)
        {
            TY_(ReportError)(doc, list, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }
        
        /* node should be <DT> or <DD>*/
        TY_(InsertNodeAtEnd)(list, node);
        ParseTag( doc, node, IgnoreWhitespace);
    }

    TY_(ReportError)(doc, list, node, MISSING_ENDTAG_FOR);
}

static Bool FindLastLI( Node *list, Node **lastli )
{
    Node *node;

    *lastli = NULL;
    for ( node = list->content; node ; node = node->next )
        if ( nodeIsLI(node) && node->type == StartTag )
            *lastli=node;
    return *lastli ? yes:no;
}

void TY_(ParseList)(TidyDocImpl* doc, Node *list, GetTokenMode ARG_UNUSED(mode))
{
    Lexer* lexer = doc->lexer;
    Node *node, *parent, *lastli;
    Bool wasblock;

    if (list->tag->model & CM_EMPTY)
        return;

    lexer->insert = NULL;  /* defer implicit inline start tags */

    while ((node = TY_(GetToken)( doc, IgnoreWhitespace)) != NULL)
    {
        if (node->tag == list->tag && node->type == EndTag)
        {
            TY_(FreeNode)( doc, node);
            list->closed = yes;
            return;
        }

        /* deal with comments etc. */
        if (InsertMisc(list, node))
            continue;

        if (node->type != TextNode && node->tag == NULL)
        {
            TY_(ReportError)(doc, list, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /* 
          if this is the end tag for an ancestor element
          then infer end tag for this element
        */
        if (node->type == EndTag)
        {
            if ( nodeIsFORM(node) )
            {
                BadForm( doc );
                TY_(ReportError)(doc, list, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node );
                continue;
            }

            if (TY_(nodeHasCM)(node,CM_INLINE))
            {
                TY_(ReportError)(doc, list, node, DISCARDING_UNEXPECTED);
                TY_(PopInline)( doc, node );
                TY_(FreeNode)( doc, node);
                continue;
            }

            for ( parent = list->parent;
                  parent != NULL; parent = parent->parent )
            {
               /* Do not match across BODY to avoid infinite loop
                  between ParseBody and this parser,
                  See http://tidy.sf.net/bug/1053626. */
                if (nodeIsBODY(parent))
                    break;
                if (node->tag == parent->tag)
                {
                    TY_(ReportError)(doc, list, node, MISSING_ENDTAG_BEFORE);
                    TY_(UngetToken)( doc );
                    return;
                }
            }

            TY_(ReportError)(doc, list, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        if ( !nodeIsLI(node) )
        {
            TY_(UngetToken)( doc );

            if (TY_(nodeHasCM)(node,CM_BLOCK) && lexer->excludeBlocks)
            {
                TY_(ReportError)(doc, list, node, MISSING_ENDTAG_BEFORE);
                return;
            }
            /* http://tidy.sf.net/issue/1316307 */
            /* In exiled mode, return so table processing can continue. */
            else if ( lexer->exiled
                      && (TY_(nodeHasCM)(node, CM_TABLE|CM_ROWGRP|CM_ROW)
                          || nodeIsTABLE(node)) )
                return;

            /* http://tidy.sf.net/issue/836462
               If "list" is an unordered list, insert the next tag within 
               the last <li> to preserve the numbering to match the visual 
               rendering of most browsers. */    
            if ( nodeIsOL(list) && FindLastLI(list, &lastli) )
            {
                /* Create a node for error reporting */
                node = TY_(InferredTag)(doc, TidyTag_LI);
                TY_(ReportError)(doc, list, node, MISSING_STARTTAG );
                TY_(FreeNode)( doc, node);
                node = lastli;
            }
            else
            {
                /* Add an inferred <li> */
                wasblock = TY_(nodeHasCM)(node,CM_BLOCK);
                node = TY_(InferredTag)(doc, TidyTag_LI);
                /* Add "display: inline" to avoid a blank line after <li> with 
                   Internet Explorer. See http://tidy.sf.net/issue/836462 */
                TY_(AddStyleProperty)( doc, node,
                                       wasblock
                                       ? "list-style: none; display: inline"
                                       : "list-style: none" 
                                       );
                TY_(ReportError)(doc, list, node, MISSING_STARTTAG );
                TY_(InsertNodeAtEnd)(list,node);
            }
        }
        else
            /* node is <LI> */
            TY_(InsertNodeAtEnd)(list,node);

        ParseTag( doc, node, IgnoreWhitespace);
    }

    TY_(ReportError)(doc, list, node, MISSING_ENDTAG_FOR);
}

/*
 unexpected content in table row is moved to just before
 the table in accordance with Netscape and IE. This code
 assumes that node hasn't been inserted into the row.
*/
static void MoveBeforeTable( TidyDocImpl* ARG_UNUSED(doc), Node *row,
                             Node *node )
{
    Node *table;

    /* first find the table element */
    for (table = row->parent; table; table = table->parent)
    {
        if ( nodeIsTABLE(table) )
        {
            TY_(InsertNodeBeforeElement)( table, node );
            return;
        }
    }
    /* No table element */
    TY_(InsertNodeBeforeElement)( row->parent, node );
}

/*
 if a table row is empty then insert an empty cell
 this practice is consistent with browser behavior
 and avoids potential problems with row spanning cells
*/
static void FixEmptyRow(TidyDocImpl* doc, Node *row)
{
    Node *cell;

    if (row->content == NULL)
    {
        cell = TY_(InferredTag)(doc, TidyTag_TD);
        TY_(InsertNodeAtEnd)(row, cell);
        TY_(ReportError)(doc, row, cell, MISSING_STARTTAG);
    }
}

void TY_(ParseRow)(TidyDocImpl* doc, Node *row, GetTokenMode ARG_UNUSED(mode))
{
    Lexer* lexer = doc->lexer;
    Node *node;
    Bool exclude_state;

    if (row->tag->model & CM_EMPTY)
        return;

    while ((node = TY_(GetToken)(doc, IgnoreWhitespace)) != NULL)
    {
        if (node->tag == row->tag)
        {
            if (node->type == EndTag)
            {
                TY_(FreeNode)( doc, node);
                row->closed = yes;
                FixEmptyRow( doc, row);
                return;
            }

            /* New row start implies end of current row */
            TY_(UngetToken)( doc );
            FixEmptyRow( doc, row);
            return;
        }

        /* 
          if this is the end tag for an ancestor element
          then infer end tag for this element
        */
        if ( node->type == EndTag )
        {
            if ( (TY_(nodeHasCM)(node, CM_HTML|CM_TABLE) || nodeIsTABLE(node))
                 && DescendantOf(row, TagId(node)) )
            {
                TY_(UngetToken)( doc );
                return;
            }

            if ( nodeIsFORM(node) || TY_(nodeHasCM)(node, CM_BLOCK|CM_INLINE) )
            {
                if ( nodeIsFORM(node) )
                    BadForm( doc );

                TY_(ReportError)(doc, row, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }

            if ( nodeIsTD(node) || nodeIsTH(node) )
            {
                TY_(ReportError)(doc, row, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }
        }

        /* deal with comments etc. */
        if (InsertMisc(row, node))
            continue;

        /* discard unknown tags */
        if (node->tag == NULL && node->type != TextNode)
        {
            TY_(ReportError)(doc, row, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /* discard unexpected <table> element */
        if ( nodeIsTABLE(node) )
        {
            TY_(ReportError)(doc, row, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /* THEAD, TFOOT or TBODY */
        if ( TY_(nodeHasCM)(node, CM_ROWGRP) )
        {
            TY_(UngetToken)( doc );
            return;
        }

        if (node->type == EndTag)
        {
            TY_(ReportError)(doc, row, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /*
          if text or inline or block move before table
          if head content move to head
        */

        if (node->type != EndTag)
        {
            if ( nodeIsFORM(node) )
            {
                TY_(UngetToken)( doc );
                node = TY_(InferredTag)(doc, TidyTag_TD);
                TY_(ReportError)(doc, row, node, MISSING_STARTTAG);
            }
            else if ( TY_(nodeIsText)(node)
                      || TY_(nodeHasCM)(node, CM_BLOCK | CM_INLINE) )
            {
                MoveBeforeTable( doc, row, node );
                TY_(ReportError)(doc, row, node, TAG_NOT_ALLOWED_IN);
                lexer->exiled = yes;
                exclude_state = lexer->excludeBlocks;
                lexer->excludeBlocks = no;

                if (node->type != TextNode)
                    ParseTag( doc, node, IgnoreWhitespace);

                lexer->exiled = no;
                lexer->excludeBlocks = exclude_state;
                continue;
            }
            else if (node->tag->model & CM_HEAD)
            {
                TY_(ReportError)(doc, row, node, TAG_NOT_ALLOWED_IN);
                MoveToHead( doc, row, node);
                continue;
            }
        }

        if ( !(nodeIsTD(node) || nodeIsTH(node)) )
        {
            TY_(ReportError)(doc, row, node, TAG_NOT_ALLOWED_IN);
            TY_(FreeNode)( doc, node);
            continue;
        }
        
        /* node should be <TD> or <TH> */
        TY_(InsertNodeAtEnd)(row, node);
        exclude_state = lexer->excludeBlocks;
        lexer->excludeBlocks = no;
        ParseTag( doc, node, IgnoreWhitespace);
        lexer->excludeBlocks = exclude_state;

        /* pop inline stack */

        while ( lexer->istacksize > lexer->istackbase )
            TY_(PopInline)( doc, NULL );
    }

}

void TY_(ParseRowGroup)(TidyDocImpl* doc, Node *rowgroup, GetTokenMode ARG_UNUSED(mode))
{
    Lexer* lexer = doc->lexer;
    Node *node, *parent;

    if (rowgroup->tag->model & CM_EMPTY)
        return;

    while ((node = TY_(GetToken)(doc, IgnoreWhitespace)) != NULL)
    {
        if (node->tag == rowgroup->tag)
        {
            if (node->type == EndTag)
            {
                rowgroup->closed = yes;
                TY_(FreeNode)( doc, node);
                return;
            }

            TY_(UngetToken)( doc );
            return;
        }

        /* if </table> infer end tag */
        if ( nodeIsTABLE(node) && node->type == EndTag )
        {
            TY_(UngetToken)( doc );
            return;
        }

        /* deal with comments etc. */
        if (InsertMisc(rowgroup, node))
            continue;

        /* discard unknown tags */
        if (node->tag == NULL && node->type != TextNode)
        {
            TY_(ReportError)(doc, rowgroup, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /*
          if TD or TH then infer <TR>
          if text or inline or block move before table
          if head content move to head
        */

        if (node->type != EndTag)
        {
            if ( nodeIsTD(node) || nodeIsTH(node) )
            {
                TY_(UngetToken)( doc );
                node = TY_(InferredTag)(doc, TidyTag_TR);
                TY_(ReportError)(doc, rowgroup, node, MISSING_STARTTAG);
            }
            else if ( TY_(nodeIsText)(node)
                      || TY_(nodeHasCM)(node, CM_BLOCK|CM_INLINE) )
            {
                MoveBeforeTable( doc, rowgroup, node );
                TY_(ReportError)(doc, rowgroup, node, TAG_NOT_ALLOWED_IN);
                lexer->exiled = yes;

                if (node->type != TextNode)
                    ParseTag(doc, node, IgnoreWhitespace);

                lexer->exiled = no;
                continue;
            }
            else if (node->tag->model & CM_HEAD)
            {
                TY_(ReportError)(doc, rowgroup, node, TAG_NOT_ALLOWED_IN);
                MoveToHead(doc, rowgroup, node);
                continue;
            }
        }

        /* 
          if this is the end tag for ancestor element
          then infer end tag for this element
        */
        if (node->type == EndTag)
        {
            if ( nodeIsFORM(node) || TY_(nodeHasCM)(node, CM_BLOCK|CM_INLINE) )
            {
                if ( nodeIsFORM(node) )
                    BadForm( doc );

                TY_(ReportError)(doc, rowgroup, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }

            if ( nodeIsTR(node) || nodeIsTD(node) || nodeIsTH(node) )
            {
                TY_(ReportError)(doc, rowgroup, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }

            for ( parent = rowgroup->parent;
                  parent != NULL;
                  parent = parent->parent )
            {
                if (node->tag == parent->tag)
                {
                    TY_(UngetToken)( doc );
                    return;
                }
            }
        }

        /*
          if THEAD, TFOOT or TBODY then implied end tag

        */
        if (node->tag->model & CM_ROWGRP)
        {
            if (node->type != EndTag)
            {
                TY_(UngetToken)( doc );
                return;
            }
        }

        if (node->type == EndTag)
        {
            TY_(ReportError)(doc, rowgroup, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }
        
        if ( !nodeIsTR(node) )
        {
            node = TY_(InferredTag)(doc, TidyTag_TR);
            TY_(ReportError)(doc, rowgroup, node, MISSING_STARTTAG);
            TY_(UngetToken)( doc );
        }

       /* node should be <TR> */
        TY_(InsertNodeAtEnd)(rowgroup, node);
        ParseTag(doc, node, IgnoreWhitespace);
    }

}

void TY_(ParseColGroup)(TidyDocImpl* doc, Node *colgroup, GetTokenMode ARG_UNUSED(mode))
{
    Node *node, *parent;

    if (colgroup->tag->model & CM_EMPTY)
        return;

    while ((node = TY_(GetToken)(doc, IgnoreWhitespace)) != NULL)
    {
        if (node->tag == colgroup->tag && node->type == EndTag)
        {
            TY_(FreeNode)( doc, node);
            colgroup->closed = yes;
            return;
        }

        /* 
          if this is the end tag for an ancestor element
          then infer end tag for this element
        */
        if (node->type == EndTag)
        {
            if ( nodeIsFORM(node) )
            {
                BadForm( doc );
                TY_(ReportError)(doc, colgroup, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }

            for ( parent = colgroup->parent;
                  parent != NULL;
                  parent = parent->parent )
            {
                if (node->tag == parent->tag)
                {
                    TY_(UngetToken)( doc );
                    return;
                }
            }
        }

        if (TY_(nodeIsText)(node))
        {
            TY_(UngetToken)( doc );
            return;
        }

        /* deal with comments etc. */
        if (InsertMisc(colgroup, node))
            continue;

        /* discard unknown tags */
        if (node->tag == NULL)
        {
            TY_(ReportError)(doc, colgroup, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        if ( !nodeIsCOL(node) )
        {
            TY_(UngetToken)( doc );
            return;
        }

        if (node->type == EndTag)
        {
            TY_(ReportError)(doc, colgroup, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }
        
        /* node should be <COL> */
        TY_(InsertNodeAtEnd)(colgroup, node);
        ParseTag(doc, node, IgnoreWhitespace);
    }
}

void TY_(ParseTableTag)(TidyDocImpl* doc, Node *table, GetTokenMode ARG_UNUSED(mode))
{
    Lexer* lexer = doc->lexer;
    Node *node, *parent;
    uint istackbase;

    TY_(DeferDup)( doc );
    istackbase = lexer->istackbase;
    lexer->istackbase = lexer->istacksize;
    
    while ((node = TY_(GetToken)(doc, IgnoreWhitespace)) != NULL)
    {
        if (node->tag == table->tag && node->type == EndTag)
        {
            TY_(FreeNode)( doc, node);
            lexer->istackbase = istackbase;
            table->closed = yes;
            return;
        }

        /* deal with comments etc. */
        if (InsertMisc(table, node))
            continue;

        /* discard unknown tags */
        if (node->tag == NULL && node->type != TextNode)
        {
            TY_(ReportError)(doc, table, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /* if TD or TH or text or inline or block then infer <TR> */

        if (node->type != EndTag)
        {
            if ( nodeIsTD(node) || nodeIsTH(node) || nodeIsTABLE(node) )
            {
                TY_(UngetToken)( doc );
                node = TY_(InferredTag)(doc, TidyTag_TR);
                TY_(ReportError)(doc, table, node, MISSING_STARTTAG);
            }
            else if ( TY_(nodeIsText)(node) ||TY_(nodeHasCM)(node,CM_BLOCK|CM_INLINE) )
            {
                TY_(InsertNodeBeforeElement)(table, node);
                TY_(ReportError)(doc, table, node, TAG_NOT_ALLOWED_IN);
                lexer->exiled = yes;

                if (node->type != TextNode) 
                    ParseTag(doc, node, IgnoreWhitespace);

                lexer->exiled = no;
                continue;
            }
            else if (node->tag->model & CM_HEAD)
            {
                MoveToHead(doc, table, node);
                continue;
            }
        }

        /* 
          if this is the end tag for an ancestor element
          then infer end tag for this element
        */
        if (node->type == EndTag)
        {
            if ( nodeIsFORM(node) )
            {
                BadForm( doc );
                TY_(ReportError)(doc, table, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }

            /* best to discard unexpected block/inline end tags */
            if ( TY_(nodeHasCM)(node, CM_TABLE|CM_ROW) ||
                 TY_(nodeHasCM)(node, CM_BLOCK|CM_INLINE) )
            {
                TY_(ReportError)(doc, table, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }

            for ( parent = table->parent;
                  parent != NULL;
                  parent = parent->parent )
            {
                if (node->tag == parent->tag)
                {
                    TY_(ReportError)(doc, table, node, MISSING_ENDTAG_BEFORE );
                    TY_(UngetToken)( doc );
                    lexer->istackbase = istackbase;
                    return;
                }
            }
        }

        if (!(node->tag->model & CM_TABLE))
        {
            TY_(UngetToken)( doc );
            TY_(ReportError)(doc, table, node, TAG_NOT_ALLOWED_IN);
            lexer->istackbase = istackbase;
            return;
        }

        if (TY_(nodeIsElement)(node))
        {
            TY_(InsertNodeAtEnd)(table, node);
            ParseTag(doc, node, IgnoreWhitespace);
            continue;
        }

        /* discard unexpected text nodes and end tags */
        TY_(ReportError)(doc, table, node, DISCARDING_UNEXPECTED);
        TY_(FreeNode)( doc, node);
    }

    TY_(ReportError)(doc, table, node, MISSING_ENDTAG_FOR);
    lexer->istackbase = istackbase;
}

/* acceptable content for pre elements */
static Bool PreContent( TidyDocImpl* ARG_UNUSED(doc), Node* node )
{
    /* p is coerced to br's, Text OK too */
    if ( nodeIsP(node) || TY_(nodeIsText)(node) )
        return yes;

    if ( node->tag == NULL ||
         nodeIsPARAM(node) ||
         !TY_(nodeHasCM)(node, CM_INLINE|CM_NEW) )
        return no;

    return yes;
}

void TY_(ParsePre)( TidyDocImpl* doc, Node *pre, GetTokenMode ARG_UNUSED(mode) )
{
    Node *node;

    if (pre->tag->model & CM_EMPTY)
        return;

    TY_(InlineDup)( doc, NULL ); /* tell lexer to insert inlines if needed */

    while ((node = TY_(GetToken)(doc, Preformatted)) != NULL)
    {
        if ( node->type == EndTag && 
             (node->tag == pre->tag || DescendantOf(pre, TagId(node))) )
        {
            if (nodeIsBODY(node) || nodeIsHTML(node))
            {
                TY_(ReportError)(doc, pre, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)(doc, node);
                continue;
            }
            if (node->tag == pre->tag)
            {
                TY_(FreeNode)(doc, node);
            }
            else
            {
                TY_(ReportError)(doc, pre, node, MISSING_ENDTAG_BEFORE );
                TY_(UngetToken)( doc );
            }
            pre->closed = yes;
            TrimSpaces(doc, pre);
            return;
        }

        if (TY_(nodeIsText)(node))
        {
            TY_(InsertNodeAtEnd)(pre, node);
            continue;
        }

        /* deal with comments etc. */
        if (InsertMisc(pre, node))
            continue;

        if (node->tag == NULL)
        {
            TY_(ReportError)(doc, pre, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)(doc, node);
            continue;
        }

        /* strip unexpected tags */
        if ( !PreContent(doc, node) )
        {
            Node *newnode;

            /* fix for http://tidy.sf.net/bug/772205 */
            if (node->type == EndTag)
            {
                /* http://tidy.sf.net/issue/1590220 */ 
               if ( doc->lexer->exiled
                   && (TY_(nodeHasCM)(node, CM_TABLE) || nodeIsTABLE(node)) )
               {
                  TY_(UngetToken)(doc);
                  TrimSpaces(doc, pre);
                  return;
               }

               TY_(ReportError)(doc, pre, node, DISCARDING_UNEXPECTED);
               TY_(FreeNode)(doc, node);
               continue;
            }
            /* http://tidy.sf.net/issue/1590220 */
            else if (TY_(nodeHasCM)(node, CM_TABLE|CM_ROW)
                     || nodeIsTABLE(node) )
            {
                if (!doc->lexer->exiled)
                    /* No missing close warning if exiled. */
                    TY_(ReportError)(doc, pre, node, MISSING_ENDTAG_BEFORE);

                TY_(UngetToken)(doc);
                return;
            }

            /*
              This is basically what Tidy 04 August 2000 did and far more accurate
              with respect to browser behaivour than the code commented out above.
              Tidy could try to propagate the <pre> into each disallowed child where
              <pre> is allowed in order to replicate some browsers behaivour, but
              there are a lot of exceptions, e.g. Internet Explorer does not propagate
              <pre> into table cells while Mozilla does. Opera 6 never propagates
              <pre> into blocklevel elements while Opera 7 behaves much like Mozilla.

              Tidy behaves thus mostly like Opera 6 except for nested <pre> elements
              which are handled like Mozilla takes them (Opera6 closes all <pre> after
              the first </pre>).

              There are similar issues like replacing <p> in <pre> with <br>, for
              example

                <pre>...<p>...</pre>                 (Input)
                <pre>...<br>...</pre>                (Tidy)
                <pre>...<br>...</pre>                (Opera 7 and Internet Explorer)
                <pre>...<br><br>...</pre>            (Opera 6 and Mozilla)

                <pre>...<p>...</p>...</pre>          (Input)
                <pre>...<br>......</pre>             (Tidy, BUG!)
                <pre>...<br>...<br>...</pre>         (Internet Explorer)
                <pre>...<br><br>...<br><br>...</pre> (Mozilla, Opera 6)
                <pre>...<br>...<br><br>...</pre>     (Opera 7)
                
              or something similar, they could also be closing the <pre> and propagate
              the <pre> into the newly opened <p>.

              Todo: IMG, OBJECT, APPLET, BIG, SMALL, SUB, SUP, FONT, and BASEFONT are
              dissallowed in <pre>, Tidy neither detects this nor does it perform any
              cleanup operation. Tidy should at least issue a warning if it encounters
              such constructs.

              Todo: discarding </p> is abviously a bug, it should be replaced by <br>.
            */
            TY_(InsertNodeAfterElement)(pre, node);
            TY_(ReportError)(doc, pre, node, MISSING_ENDTAG_BEFORE);
            ParseTag(doc, node, IgnoreWhitespace);

            newnode = TY_(InferredTag)(doc, TidyTag_PRE);
            TY_(ReportError)(doc, pre, newnode, INSERTING_TAG);
            pre = newnode;
            TY_(InsertNodeAfterElement)(node, pre);

            continue;
        }

        if ( nodeIsP(node) )
        {
            if (node->type == StartTag)
            {
                TY_(ReportError)(doc, pre, node, USING_BR_INPLACE_OF);

                /* trim white space before <p> in <pre>*/
                TrimSpaces(doc, pre);
            
                /* coerce both <p> and </p> to <br> */
                TY_(CoerceNode)(doc, node, TidyTag_BR, no, no);
                TY_(FreeAttrs)( doc, node ); /* discard align attribute etc. */
                TY_(InsertNodeAtEnd)( pre, node );
            }
            else
            {
                TY_(ReportError)(doc, pre, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
            }
            continue;
        }

        if ( TY_(nodeIsElement)(node) )
        {
            /* trim white space before <br> */
            if ( nodeIsBR(node) )
                TrimSpaces(doc, pre);
            
            TY_(InsertNodeAtEnd)(pre, node);
            ParseTag(doc, node, Preformatted);
            continue;
        }

        /* discard unexpected tags */
        TY_(ReportError)(doc, pre, node, DISCARDING_UNEXPECTED);
        TY_(FreeNode)( doc, node);
    }

    TY_(ReportError)(doc, pre, node, MISSING_ENDTAG_FOR);
}

void TY_(ParseOptGroup)(TidyDocImpl* doc, Node *field, GetTokenMode ARG_UNUSED(mode))
{
    Lexer* lexer = doc->lexer;
    Node *node;

    lexer->insert = NULL;  /* defer implicit inline start tags */

    while ((node = TY_(GetToken)(doc, IgnoreWhitespace)) != NULL)
    {
        if (node->tag == field->tag && node->type == EndTag)
        {
            TY_(FreeNode)( doc, node);
            field->closed = yes;
            TrimSpaces(doc, field);
            return;
        }

        /* deal with comments etc. */
        if (InsertMisc(field, node))
            continue;

        if ( node->type == StartTag && 
             (nodeIsOPTION(node) || nodeIsOPTGROUP(node)) )
        {
            if ( nodeIsOPTGROUP(node) )
                TY_(ReportError)(doc, field, node, CANT_BE_NESTED);

            TY_(InsertNodeAtEnd)(field, node);
            ParseTag(doc, node, MixedContent);
            continue;
        }

        /* discard unexpected tags */
        TY_(ReportError)(doc, field, node, DISCARDING_UNEXPECTED );
        TY_(FreeNode)( doc, node);
    }
}


void TY_(ParseSelect)(TidyDocImpl* doc, Node *field, GetTokenMode ARG_UNUSED(mode))
{
    Lexer* lexer = doc->lexer;
    Node *node;

    lexer->insert = NULL;  /* defer implicit inline start tags */

    while ((node = TY_(GetToken)(doc, IgnoreWhitespace)) != NULL)
    {
        if (node->tag == field->tag && node->type == EndTag)
        {
            TY_(FreeNode)( doc, node);
            field->closed = yes;
            TrimSpaces(doc, field);
            return;
        }

        /* deal with comments etc. */
        if (InsertMisc(field, node))
            continue;

        if ( node->type == StartTag && 
             ( nodeIsOPTION(node)   ||
               nodeIsOPTGROUP(node) ||
               nodeIsSCRIPT(node)) 
           )
        {
            TY_(InsertNodeAtEnd)(field, node);
            ParseTag(doc, node, IgnoreWhitespace);
            continue;
        }

        /* discard unexpected tags */
        TY_(ReportError)(doc, field, node, DISCARDING_UNEXPECTED);
        TY_(FreeNode)( doc, node);
    }

    TY_(ReportError)(doc, field, node, MISSING_ENDTAG_FOR);
}

void TY_(ParseText)(TidyDocImpl* doc, Node *field, GetTokenMode mode)
{
    Lexer* lexer = doc->lexer;
    Node *node;

    lexer->insert = NULL;  /* defer implicit inline start tags */

    if ( nodeIsTEXTAREA(field) )
        mode = Preformatted;
    else
        mode = MixedContent;  /* kludge for font tags */

    while ((node = TY_(GetToken)(doc, mode)) != NULL)
    {
        if (node->tag == field->tag && node->type == EndTag)
        {
            TY_(FreeNode)( doc, node);
            field->closed = yes;
            TrimSpaces(doc, field);
            return;
        }

        /* deal with comments etc. */
        if (InsertMisc(field, node))
            continue;

        if (TY_(nodeIsText)(node))
        {
            /* only called for 1st child */
            if (field->content == NULL && !(mode & Preformatted))
                TrimSpaces(doc, field);

            if (node->start >= node->end)
            {
                TY_(FreeNode)( doc, node);
                continue;
            }

            TY_(InsertNodeAtEnd)(field, node);
            continue;
        }

        /* for textarea should all cases of < and & be escaped? */

        /* discard inline tags e.g. font */
        if (   node->tag 
            && node->tag->model & CM_INLINE
            && !(node->tag->model & CM_FIELD)) /* #487283 - fix by Lee Passey 25 Jan 02 */
        {
            TY_(ReportError)(doc, field, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /* terminate element on other tags */
        if (!(field->tag->model & CM_OPT))
            TY_(ReportError)(doc, field, node, MISSING_ENDTAG_BEFORE);

        TY_(UngetToken)( doc );
        TrimSpaces(doc, field);
        return;
    }

    if (!(field->tag->model & CM_OPT))
        TY_(ReportError)(doc, field, node, MISSING_ENDTAG_FOR);
}


void TY_(ParseTitle)(TidyDocImpl* doc, Node *title, GetTokenMode ARG_UNUSED(mode))
{
    Node *node;
    while ((node = TY_(GetToken)(doc, MixedContent)) != NULL)
    {
        if (node->tag == title->tag && node->type == StartTag)
        {
            TY_(ReportError)(doc, title, node, COERCE_TO_ENDTAG);
            node->type = EndTag;
            TY_(UngetToken)( doc );
            continue;
        }
        else if (node->tag == title->tag && node->type == EndTag)
        {
            TY_(FreeNode)( doc, node);
            title->closed = yes;
            TrimSpaces(doc, title);
            return;
        }

        if (TY_(nodeIsText)(node))
        {
            /* only called for 1st child */
            if (title->content == NULL)
                TrimInitialSpace(doc, title, node);

            if (node->start >= node->end)
            {
                TY_(FreeNode)( doc, node);
                continue;
            }

            TY_(InsertNodeAtEnd)(title, node);
            continue;
        }

        /* deal with comments etc. */
        if (InsertMisc(title, node))
            continue;

        /* discard unknown tags */
        if (node->tag == NULL)
        {
            TY_(ReportError)(doc, title, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /* pushback unexpected tokens */
        TY_(ReportError)(doc, title, node, MISSING_ENDTAG_BEFORE);
        TY_(UngetToken)( doc );
        TrimSpaces(doc, title);
        return;
    }

    TY_(ReportError)(doc, title, node, MISSING_ENDTAG_FOR);
}

/*
  This isn't quite right for CDATA content as it recognises
  tags within the content and parses them accordingly.
  This will unfortunately screw up scripts which include
  < + letter,  < + !, < + ?  or  < + / + letter
*/

void TY_(ParseScript)(TidyDocImpl* doc, Node *script, GetTokenMode ARG_UNUSED(mode))
{
    Node *node;
    
    doc->lexer->parent = script;
    node = TY_(GetToken)(doc, CdataContent);
    doc->lexer->parent = NULL;

    if (node)
    {
        TY_(InsertNodeAtEnd)(script, node);
    }
    else
    {
        /* handle e.g. a document like "<script>" */
        TY_(ReportError)(doc, script, NULL, MISSING_ENDTAG_FOR);
        return;
    }

    node = TY_(GetToken)(doc, IgnoreWhitespace);

    if (!(node && node->type == EndTag && node->tag &&
        node->tag->id == script->tag->id))
    {
        TY_(ReportError)(doc, script, node, MISSING_ENDTAG_FOR);

        if (node)
            TY_(UngetToken)(doc);
    }
    else
    {
        TY_(FreeNode)(doc, node);
    }
}

Bool TY_(IsJavaScript)(Node *node)
{
    Bool result = no;
    AttVal *attr;

    if (node->attributes == NULL)
        return yes;

    for (attr = node->attributes; attr; attr = attr->next)
    {
        if ( (attrIsLANGUAGE(attr) || attrIsTYPE(attr))
             && AttrContains(attr, "javascript") )
        {
            result = yes;
            break;
        }
    }

    return result;
}

void TY_(ParseHead)(TidyDocImpl* doc, Node *head, GetTokenMode ARG_UNUSED(mode))
{
    Lexer* lexer = doc->lexer;
    Node *node;
    int HasTitle = 0;
    int HasBase = 0;

    while ((node = TY_(GetToken)(doc, IgnoreWhitespace)) != NULL)
    {
        if (node->tag == head->tag && node->type == EndTag)
        {
            TY_(FreeNode)( doc, node);
            head->closed = yes;
            break;
        }

        /* find and discard multiple <head> elements */
        /* find and discard <html> in <head> elements */
        if ((node->tag == head->tag || nodeIsHTML(node)) && node->type == StartTag)
        {
            TY_(ReportError)(doc, head, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)(doc, node);
            continue;
        }

        if (TY_(nodeIsText)(node))
        {
            TY_(ReportError)(doc, head, node, TAG_NOT_ALLOWED_IN);
            TY_(UngetToken)( doc );
            break;
        }

        if (node->type == ProcInsTag && node->element &&
            TY_(tmbstrcmp)(node->element, "xml-stylesheet") == 0)
        {
            TY_(ReportError)(doc, head, node, TAG_NOT_ALLOWED_IN);
            TY_(InsertNodeBeforeElement)(TY_(FindHTML)(doc), node);
            continue;
        }

        /* deal with comments etc. */
        if (InsertMisc(head, node))
            continue;

        if (node->type == DocTypeTag)
        {
            InsertDocType(doc, head, node);
            continue;
        }

        /* discard unknown tags */
        if (node->tag == NULL)
        {
            TY_(ReportError)(doc, head, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }
        
        /*
         if it doesn't belong in the head then
         treat as implicit end of head and deal
         with as part of the body
        */
        if (!(node->tag->model & CM_HEAD))
        {
            /* #545067 Implicit closing of head broken - warn only for XHTML input */
            if ( lexer->isvoyager )
                TY_(ReportError)(doc, head, node, TAG_NOT_ALLOWED_IN );
            TY_(UngetToken)( doc );
            break;
        }

        if (TY_(nodeIsElement)(node))
        {
            if ( nodeIsTITLE(node) )
            {
                ++HasTitle;

                if (HasTitle > 1)
                    TY_(ReportError)(doc, head, node,
                                     head ?
                                     TOO_MANY_ELEMENTS_IN : TOO_MANY_ELEMENTS);
            }
            else if ( nodeIsBASE(node) )
            {
                ++HasBase;

                if (HasBase > 1)
                    TY_(ReportError)(doc, head, node,
                                     head ?
                                     TOO_MANY_ELEMENTS_IN : TOO_MANY_ELEMENTS);
            }
            else if ( nodeIsNOSCRIPT(node) )
            {
                TY_(ReportError)(doc, head, node, TAG_NOT_ALLOWED_IN);
            }

#ifdef AUTO_INPUT_ENCODING
            else if (nodeIsMETA(node))
            {
                AttVal * httpEquiv = AttrGetById(node, TidyAttr_HTTP_EQUIV);
                AttVal * content = AttrGetById(node, TidyAttr_CONTENT);
                if (httpEquiv && AttrValueIs(httpEquiv, "Content-Type") && AttrHasValue(content))
                {
                    tmbstr val, charset;
                    uint end = 0;
                    val = charset = TY_(tmbstrdup)(doc->allocator, content->value);
                    val = TY_(tmbstrtolower)(val);
                    val = strstr(content->value, "charset");
                    
                    if (val)
                        val += 7;

                    while(val && *val && (TY_(IsWhite)((tchar)*val) ||
                          *val == '=' || *val == '"' || *val == '\''))
                        ++val;

                    while(val && val[end] && !(TY_(IsWhite)((tchar)val[end]) ||
                          val[end] == '"' || val[end] == '\'' || val[end] == ';'))
                        ++end;

                    if (val && end)
                    {
                        tmbstr encoding = TY_(tmbstrndup)(doc->allocator,val, end);
                        uint id = TY_(GetEncodingIdFromName)(encoding);

                        /* todo: detect mismatch with BOM/XMLDecl/declared */
                        /* todo: error for unsupported encodings */
                        /* todo: try to re-init transcoder */
                        /* todo: change input/output encoding settings */
                        /* todo: store id in StreamIn */

                        TidyDocFree(doc, encoding);
                    }

                    TidyDocFree(doc, charset);
                }
            }
#endif /* AUTO_INPUT_ENCODING */

            TY_(InsertNodeAtEnd)(head, node);
            ParseTag(doc, node, IgnoreWhitespace);
            continue;
        }

        /* discard unexpected text nodes and end tags */
        TY_(ReportError)(doc, head, node, DISCARDING_UNEXPECTED);
        TY_(FreeNode)( doc, node);
    }
}

void TY_(ParseBody)(TidyDocImpl* doc, Node *body, GetTokenMode mode)
{
    Lexer* lexer = doc->lexer;
    Node *node;
    Bool checkstack, iswhitenode;

    mode = IgnoreWhitespace;
    checkstack = yes;

    TY_(BumpObject)( doc, body->parent );

    while ((node = TY_(GetToken)(doc, mode)) != NULL)
    {
        /* find and discard multiple <body> elements */
        if (node->tag == body->tag && node->type == StartTag)
        {
            TY_(ReportError)(doc, body, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)(doc, node);
            continue;
        }

        /* #538536 Extra endtags not detected */
        if ( nodeIsHTML(node) )
        {
            if (TY_(nodeIsElement)(node) || lexer->seenEndHtml) 
                TY_(ReportError)(doc, body, node, DISCARDING_UNEXPECTED);
            else
                lexer->seenEndHtml = 1;

            TY_(FreeNode)( doc, node);
            continue;
        }

        if ( lexer->seenEndBody && 
             ( node->type == StartTag ||
               node->type == EndTag   ||
               node->type == StartEndTag ) )
        {
            TY_(ReportError)(doc, body, node, CONTENT_AFTER_BODY );
        }

        if ( node->tag == body->tag && node->type == EndTag )
        {
            body->closed = yes;
            TrimSpaces(doc, body);
            TY_(FreeNode)( doc, node);
            lexer->seenEndBody = 1;
            mode = IgnoreWhitespace;

            if ( nodeIsNOFRAMES(body->parent) )
                break;

            continue;
        }

        if ( nodeIsNOFRAMES(node) )
        {
            if (node->type == StartTag)
            {
                TY_(InsertNodeAtEnd)(body, node);
                TY_(ParseBlock)(doc, node, mode);
                continue;
            }

            if (node->type == EndTag && nodeIsNOFRAMES(body->parent) )
            {
                TrimSpaces(doc, body);
                TY_(UngetToken)( doc );
                break;
            }
        }

        if ( (nodeIsFRAME(node) || nodeIsFRAMESET(node))
             && nodeIsNOFRAMES(body->parent) )
        {
            TrimSpaces(doc, body);
            TY_(UngetToken)( doc );
            break;
        }
        
        iswhitenode = no;

        if ( TY_(nodeIsText)(node) &&
             node->end <= node->start + 1 &&
             lexer->lexbuf[node->start] == ' ' )
            iswhitenode = yes;

        /* deal with comments etc. */
        if (InsertMisc(body, node))
            continue;

        /* #538536 Extra endtags not detected */
#if 0
        if ( lexer->seenEndBody == 1 && !iswhitenode )
        {
            ++lexer->seenEndBody;
            TY_(ReportError)(doc, body, node, CONTENT_AFTER_BODY);
        }
#endif

        /* mixed content model permits text */
        if (TY_(nodeIsText)(node))
        {
            if (iswhitenode && mode == IgnoreWhitespace)
            {
                TY_(FreeNode)( doc, node);
                continue;
            }

            /* HTML 2 and HTML4 strict don't allow text here */
            TY_(ConstrainVersion)(doc, ~(VERS_HTML40_STRICT | VERS_HTML20));

            if (checkstack)
            {
                checkstack = no;

                if ( TY_(InlineDup)(doc, node) > 0 )
                    continue;
            }

            TY_(InsertNodeAtEnd)(body, node);
            mode = MixedContent;
            continue;
        }

        if (node->type == DocTypeTag)
        {
            InsertDocType(doc, body, node);
            continue;
        }
        /* discard unknown  and PARAM tags */
        if ( node->tag == NULL || nodeIsPARAM(node) )
        {
            TY_(ReportError)(doc, body, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /*
          Netscape allows LI and DD directly in BODY
          We infer UL or DL respectively and use this
          Bool to exclude block-level elements so as
          to match Netscape's observed behaviour.
        */
        lexer->excludeBlocks = no;
        
        if ( nodeIsINPUT(node) ||
             (!TY_(nodeHasCM)(node, CM_BLOCK) && !TY_(nodeHasCM)(node, CM_INLINE))
           )
        {
            /* avoid this error message being issued twice */
            if (!(node->tag->model & CM_HEAD))
                TY_(ReportError)(doc, body, node, TAG_NOT_ALLOWED_IN);

            if (node->tag->model & CM_HTML)
            {
                /* copy body attributes if current body was inferred */
                if ( nodeIsBODY(node) && body->implicit 
                     && body->attributes == NULL )
                {
                    body->attributes = node->attributes;
                    node->attributes = NULL;
                }

                TY_(FreeNode)( doc, node);
                continue;
            }

            if (node->tag->model & CM_HEAD)
            {
                MoveToHead(doc, body, node);
                continue;
            }

            if (node->tag->model & CM_LIST)
            {
                TY_(UngetToken)( doc );
                node = TY_(InferredTag)(doc, TidyTag_UL);
                AddClassNoIndent(doc, node);
                lexer->excludeBlocks = yes;
            }
            else if (node->tag->model & CM_DEFLIST)
            {
                TY_(UngetToken)( doc );
                node = TY_(InferredTag)(doc, TidyTag_DL);
                lexer->excludeBlocks = yes;
            }
            else if (node->tag->model & (CM_TABLE | CM_ROWGRP | CM_ROW))
            {
                TY_(UngetToken)( doc );
                node = TY_(InferredTag)(doc, TidyTag_TABLE);
                lexer->excludeBlocks = yes;
            }
            else if ( nodeIsINPUT(node) )
            {
                TY_(UngetToken)( doc );
                node = TY_(InferredTag)(doc, TidyTag_FORM);
                lexer->excludeBlocks = yes;
            }
            else
            {
                if ( !TY_(nodeHasCM)(node, CM_ROW | CM_FIELD) )
                {
                    TY_(UngetToken)( doc );
                    return;
                }

                /* ignore </td> </th> <option> etc. */
                TY_(FreeNode)( doc, node );
                continue;
            }
        }

        if (node->type == EndTag)
        {
            if ( nodeIsBR(node) )
                node->type = StartTag;
            else if ( nodeIsP(node) )
            {
                node->type = StartEndTag;
                node->implicit = yes;
#if OBSOLETE
                TY_(CoerceNode)(doc, node, TidyTag_BR, no, no);
                FreeAttrs( doc, node ); /* discard align attribute etc. */
                TY_(InsertNodeAtEnd)(body, node);
                node = TY_(InferredTag)(doc, TidyTag_BR);
#endif
            }
            else if ( TY_(nodeHasCM)(node, CM_INLINE) )
                TY_(PopInline)( doc, node );
        }

        if (TY_(nodeIsElement)(node))
        {
            if ( TY_(nodeHasCM)(node, CM_INLINE) && !TY_(nodeHasCM)(node, CM_MIXED) )
            {
                /* HTML4 strict doesn't allow inline content here */
                /* but HTML2 does allow img elements as children of body */
                if ( nodeIsIMG(node) )
                    TY_(ConstrainVersion)(doc, ~VERS_HTML40_STRICT);
                else
                    TY_(ConstrainVersion)(doc, ~(VERS_HTML40_STRICT|VERS_HTML20));

                if (checkstack && !node->implicit)
                {
                    checkstack = no;

                    if ( TY_(InlineDup)(doc, node) > 0 )
                        continue;
                }

                mode = MixedContent;
            }
            else
            {
                checkstack = yes;
                mode = IgnoreWhitespace;
            }

            if (node->implicit)
                TY_(ReportError)(doc, body, node, INSERTING_TAG);

            TY_(InsertNodeAtEnd)(body, node);
            ParseTag(doc, node, mode);
            continue;
        }

        /* discard unexpected tags */
        TY_(ReportError)(doc, body, node, DISCARDING_UNEXPECTED);
        TY_(FreeNode)( doc, node);
    }
}

void TY_(ParseNoFrames)(TidyDocImpl* doc, Node *noframes, GetTokenMode mode)
{
    Lexer* lexer = doc->lexer;
    Node *node;

    if ( cfg(doc, TidyAccessibilityCheckLevel) == 0 )
    {
        doc->badAccess |=  BA_USING_NOFRAMES;
    }
    mode = IgnoreWhitespace;

    while ( (node = TY_(GetToken)(doc, mode)) != NULL )
    {
        if ( node->tag == noframes->tag && node->type == EndTag )
        {
            TY_(FreeNode)( doc, node);
            noframes->closed = yes;
            TrimSpaces(doc, noframes);
            return;
        }

        if ( nodeIsFRAME(node) || nodeIsFRAMESET(node) )
        {
            TrimSpaces(doc, noframes);
            if (node->type == EndTag)
            {
                TY_(ReportError)(doc, noframes, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);       /* Throw it away */
            }
            else
            {
                TY_(ReportError)(doc, noframes, node, MISSING_ENDTAG_BEFORE);
                TY_(UngetToken)( doc );
            }
            return;
        }

        if ( nodeIsHTML(node) )
        {
            if (TY_(nodeIsElement)(node))
                TY_(ReportError)(doc, noframes, node, DISCARDING_UNEXPECTED);

            TY_(FreeNode)( doc, node);
            continue;
        }

        /* deal with comments etc. */
        if (InsertMisc(noframes, node))
            continue;

        if ( nodeIsBODY(node) && node->type == StartTag )
        {
            Bool seen_body = lexer->seenEndBody;
            TY_(InsertNodeAtEnd)(noframes, node);
            ParseTag(doc, node, IgnoreWhitespace /*MixedContent*/);

            /* fix for bug http://tidy.sf.net/bug/887259 */
            if (seen_body && TY_(FindBody)(doc) != node)
            {
                TY_(CoerceNode)(doc, node, TidyTag_DIV, no, no);
                MoveNodeToBody(doc, node);
            }
            continue;
        }

        /* implicit body element inferred */
        if (TY_(nodeIsText)(node) || (node->tag && node->type != EndTag))
        {
            Node *body = TY_(FindBody)( doc );
            if ( body || lexer->seenEndBody )
            {
                if ( body == NULL )
                {
                    TY_(ReportError)(doc, noframes, node, DISCARDING_UNEXPECTED);
                    TY_(FreeNode)( doc, node);
                    continue;
                }
                if ( TY_(nodeIsText)(node) )
                {
                    TY_(UngetToken)( doc );
                    node = TY_(InferredTag)(doc, TidyTag_P);
                    TY_(ReportError)(doc, noframes, node, CONTENT_AFTER_BODY );
                }
                TY_(InsertNodeAtEnd)( body, node );
            }
            else
            {
                TY_(UngetToken)( doc );
                node = TY_(InferredTag)(doc, TidyTag_BODY);
                if ( cfgBool(doc, TidyXmlOut) )
                    TY_(ReportError)(doc, noframes, node, INSERTING_TAG);
                TY_(InsertNodeAtEnd)( noframes, node );
            }

            ParseTag( doc, node, IgnoreWhitespace /*MixedContent*/ );
            continue;
        }

        /* discard unexpected end tags */
        TY_(ReportError)(doc, noframes, node, DISCARDING_UNEXPECTED);
        TY_(FreeNode)( doc, node);
    }

    TY_(ReportError)(doc, noframes, node, MISSING_ENDTAG_FOR);
}

void TY_(ParseFrameSet)(TidyDocImpl* doc, Node *frameset, GetTokenMode ARG_UNUSED(mode))
{
    Lexer* lexer = doc->lexer;
    Node *node;

    if ( cfg(doc, TidyAccessibilityCheckLevel) == 0 )
    {
        doc->badAccess |= BA_USING_FRAMES;
    }
    
    while ((node = TY_(GetToken)(doc, IgnoreWhitespace)) != NULL)
    {
        if (node->tag == frameset->tag && node->type == EndTag)
        {
            TY_(FreeNode)( doc, node);
            frameset->closed = yes;
            TrimSpaces(doc, frameset);
            return;
        }

        /* deal with comments etc. */
        if (InsertMisc(frameset, node))
            continue;

        if (node->tag == NULL)
        {
            TY_(ReportError)(doc, frameset, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue; 
        }

        if (TY_(nodeIsElement)(node))
        {
            if (node->tag && node->tag->model & CM_HEAD)
            {
                MoveToHead(doc, frameset, node);
                continue;
            }
        }

        if ( nodeIsBODY(node) )
        {
            TY_(UngetToken)( doc );
            node = TY_(InferredTag)(doc, TidyTag_NOFRAMES);
            TY_(ReportError)(doc, frameset, node, INSERTING_TAG);
        }

        if (node->type == StartTag && (node->tag->model & CM_FRAMES))
        {
            TY_(InsertNodeAtEnd)(frameset, node);
            lexer->excludeBlocks = no;
            ParseTag(doc, node, MixedContent);
            continue;
        }
        else if (node->type == StartEndTag && (node->tag->model & CM_FRAMES))
        {
            TY_(InsertNodeAtEnd)(frameset, node);
            continue;
        }

        /* discard unexpected tags */
#if SUPPORT_ACCESSIBILITY_CHECKS
        /* WAI [6.5.1.4] link is being discarded outside of NOFRAME */
        if ( nodeIsA(node) )
           doc->badAccess |= BA_INVALID_LINK_NOFRAMES;
#endif

        TY_(ReportError)(doc, frameset, node, DISCARDING_UNEXPECTED);
        TY_(FreeNode)( doc, node);
    }

    TY_(ReportError)(doc, frameset, node, MISSING_ENDTAG_FOR);
}

void TY_(ParseHTML)(TidyDocImpl* doc, Node *html, GetTokenMode mode)
{
    Node *node, *head;
    Node *frameset = NULL;
    Node *noframes = NULL;

    TY_(SetOptionBool)( doc, TidyXmlTags, no );

    for (;;)
    {
        node = TY_(GetToken)(doc, IgnoreWhitespace);

        if (node == NULL)
        {
            node = TY_(InferredTag)(doc, TidyTag_HEAD);
            break;
        }

        if ( nodeIsHEAD(node) )
            break;

        if (node->tag == html->tag && node->type == EndTag)
        {
            TY_(ReportError)(doc, html, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        /* find and discard multiple <html> elements */
        if (node->tag == html->tag && node->type == StartTag)
        {
            TY_(ReportError)(doc, html, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)(doc, node);
            continue;
        }

        /* deal with comments etc. */
        if (InsertMisc(html, node))
            continue;

        TY_(UngetToken)( doc );
        node = TY_(InferredTag)(doc, TidyTag_HEAD);
        break;
    }

    head = node;
    TY_(InsertNodeAtEnd)(html, head);
    TY_(ParseHead)(doc, head, mode);

    for (;;)
    {
        node = TY_(GetToken)(doc, IgnoreWhitespace);

        if (node == NULL)
        {
            if (frameset == NULL) /* implied body */
            {
                node = TY_(InferredTag)(doc, TidyTag_BODY);
                TY_(InsertNodeAtEnd)(html, node);
                TY_(ParseBody)(doc, node, mode);
            }

            return;
        }

        /* robustly handle html tags */
        if (node->tag == html->tag)
        {
            if (node->type != StartTag && frameset == NULL)
                TY_(ReportError)(doc, html, node, DISCARDING_UNEXPECTED);

            TY_(FreeNode)( doc, node);
            continue;
        }

        /* deal with comments etc. */
        if (InsertMisc(html, node))
            continue;

        /* if frameset document coerce <body> to <noframes> */
        if ( nodeIsBODY(node) )
        {
            if (node->type != StartTag)
            {
                TY_(ReportError)(doc, html, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }

            if ( cfg(doc, TidyAccessibilityCheckLevel) == 0 )
            {
                if (frameset != NULL)
                {
                    TY_(UngetToken)( doc );

                    if (noframes == NULL)
                    {
                        noframes = TY_(InferredTag)(doc, TidyTag_NOFRAMES);
                        TY_(InsertNodeAtEnd)(frameset, noframes);
                        TY_(ReportError)(doc, html, noframes, INSERTING_TAG);
                    }
                    else
                    {
                        if (noframes->type == StartEndTag)
                            noframes->type = StartTag;
                    }

                    ParseTag(doc, noframes, mode);
                    continue;
                }
            }

            TY_(ConstrainVersion)(doc, ~VERS_FRAMESET);
            break;  /* to parse body */
        }

        /* flag an error if we see more than one frameset */
        if ( nodeIsFRAMESET(node) )
        {
            if (node->type != StartTag)
            {
                TY_(ReportError)(doc, html, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }

            if (frameset != NULL)
                TY_(ReportFatal)(doc, html, node, DUPLICATE_FRAMESET);
            else
                frameset = node;

            TY_(InsertNodeAtEnd)(html, node);
            ParseTag(doc, node, mode);

            /*
              see if it includes a noframes element so
              that we can merge subsequent noframes elements
            */

            for (node = frameset->content; node; node = node->next)
            {
                if ( nodeIsNOFRAMES(node) )
                    noframes = node;
            }
            continue;
        }

        /* if not a frameset document coerce <noframes> to <body> */
        if ( nodeIsNOFRAMES(node) )
        {
            if (node->type != StartTag)
            {
                TY_(ReportError)(doc, html, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                continue;
            }

            if (frameset == NULL)
            {
                TY_(ReportError)(doc, html, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
                node = TY_(InferredTag)(doc, TidyTag_BODY);
                break;
            }

            if (noframes == NULL)
            {
                noframes = node;
                TY_(InsertNodeAtEnd)(frameset, noframes);
            }
            else
                TY_(FreeNode)( doc, node);

            ParseTag(doc, noframes, mode);
            continue;
        }

        if (TY_(nodeIsElement)(node))
        {
            if (node->tag && node->tag->model & CM_HEAD)
            {
                MoveToHead(doc, html, node);
                continue;
            }

            /* discard illegal frame element following a frameset */
            if ( frameset != NULL && nodeIsFRAME(node) )
            {
                TY_(ReportError)(doc, html, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)(doc, node);
                continue;
            }
        }

        TY_(UngetToken)( doc );

        /* insert other content into noframes element */

        if (frameset)
        {
            if (noframes == NULL)
            {
                noframes = TY_(InferredTag)(doc, TidyTag_NOFRAMES);
                TY_(InsertNodeAtEnd)(frameset, noframes);
            }
            else
            {
                TY_(ReportError)(doc, html, node, NOFRAMES_CONTENT);
                if (noframes->type == StartEndTag)
                    noframes->type = StartTag;
            }

            TY_(ConstrainVersion)(doc, VERS_FRAMESET);
            ParseTag(doc, noframes, mode);
            continue;
        }

        node = TY_(InferredTag)(doc, TidyTag_BODY);
        TY_(ReportError)(doc, html, node, INSERTING_TAG );
        TY_(ConstrainVersion)(doc, ~VERS_FRAMESET);
        break;
    }

    /* node must be body */

    TY_(InsertNodeAtEnd)(html, node);
    ParseTag(doc, node, mode);
}

static Bool nodeCMIsOnlyInline( Node* node )
{
    return TY_(nodeHasCM)( node, CM_INLINE ) && !TY_(nodeHasCM)( node, CM_BLOCK );
}

static void EncloseBodyText(TidyDocImpl* doc)
{
    Node* node;
    Node* body = TY_(FindBody)(doc);

    if (!body)
        return;

    node = body->content;

    while (node)
    {
        if ((TY_(nodeIsText)(node) && !TY_(IsBlank)(doc->lexer, node)) ||
            (TY_(nodeIsElement)(node) && nodeCMIsOnlyInline(node)))
        {
            Node* p = TY_(InferredTag)(doc, TidyTag_P);
            TY_(InsertNodeBeforeElement)(node, p);
            while (node && (!TY_(nodeIsElement)(node) || nodeCMIsOnlyInline(node)))
            {
                Node* next = node->next;
                TY_(RemoveNode)(node);
                TY_(InsertNodeAtEnd)(p, node);
                node = next;
            }
            TrimSpaces(doc, p);
            continue;
        }
        node = node->next;
    }
}

/* <form>, <blockquote> and <noscript> do not allow #PCDATA in
   HTML 4.01 Strict (%block; model instead of %flow;).
  When requested, text nodes in these elements are wrapped in <p>. */
static void EncloseBlockText(TidyDocImpl* doc, Node* node)
{
    Node *next;
    Node *block;

    while (node)
    {
        next = node->next;

        if (node->content)
            EncloseBlockText(doc, node->content);

        if (!(nodeIsFORM(node) || nodeIsNOSCRIPT(node) ||
              nodeIsBLOCKQUOTE(node))
            || !node->content)
        {
            node = next;
            continue;
        }

        block = node->content;

        if ((TY_(nodeIsText)(block) && !TY_(IsBlank)(doc->lexer, block)) ||
            (TY_(nodeIsElement)(block) && nodeCMIsOnlyInline(block)))
        {
            Node* p = TY_(InferredTag)(doc, TidyTag_P);
            TY_(InsertNodeBeforeElement)(block, p);
            while (block &&
                   (!TY_(nodeIsElement)(block) || nodeCMIsOnlyInline(block)))
            {
                Node* tempNext = block->next;
                TY_(RemoveNode)(block);
                TY_(InsertNodeAtEnd)(p, block);
                block = tempNext;
            }
            TrimSpaces(doc, p);
            continue;
        }

        node = next;
    }
}

static void ReplaceObsoleteElements(TidyDocImpl* doc, Node* node)
{
    Node *next;

    while (node)
    {
        next = node->next;

        if (nodeIsDIR(node) || nodeIsMENU(node))
            TY_(CoerceNode)(doc, node, TidyTag_UL, yes, yes);

        if (nodeIsXMP(node) || nodeIsLISTING(node) ||
            (node->tag && node->tag->id == TidyTag_PLAINTEXT))
            TY_(CoerceNode)(doc, node, TidyTag_PRE, yes, yes);

        if (node->content)
            ReplaceObsoleteElements(doc, node->content);

        node = next;
    }
}

static void AttributeChecks(TidyDocImpl* doc, Node* node)
{
    Node *next;

    while (node)
    {
        next = node->next;

        if (TY_(nodeIsElement)(node))
        {
            if (node->tag->chkattrs)
                node->tag->chkattrs(doc, node);
            else
                TY_(CheckAttributes)(doc, node);
        }

        if (node->content)
            AttributeChecks(doc, node->content);

        assert( next != node ); /* http://tidy.sf.net/issue/1603538 */
        node = next;
    }
}

/*
  HTML is the top level element
*/
void TY_(ParseDocument)(TidyDocImpl* doc)
{
    Node *node, *html, *doctype = NULL;

    while ((node = TY_(GetToken)(doc, IgnoreWhitespace)) != NULL)
    {
        if (node->type == XmlDecl)
        {
            if (TY_(FindXmlDecl)(doc) && doc->root.content)
            {
                TY_(ReportError)(doc, &doc->root, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)(doc, node);
                continue;
            }
            if (node->line != 1 || (node->line == 1 && node->column != 1))
            {
                TY_(ReportError)(doc, &doc->root, node, SPACE_PRECEDING_XMLDECL);
            }
        }
#ifdef AUTO_INPUT_ENCODING
        if (node->type == XmlDecl)
        {
            AttVal* encoding = GetAttrByName(node, "encoding");
            if (AttrHasValue(encoding))
            {
                uint id = TY_(GetEncodingIdFromName)(encoding->value);

                /* todo: detect mismatch with BOM/XMLDecl/declared */
                /* todo: error for unsupported encodings */
                /* todo: try to re-init transcoder */
                /* todo: change input/output encoding settings */
                /* todo: store id in StreamIn */
            }
        }
#endif /* AUTO_INPUT_ENCODING */

        /* deal with comments etc. */
        if (InsertMisc( &doc->root, node ))
            continue;

        if (node->type == DocTypeTag)
        {
            if (doctype == NULL)
            {
                TY_(InsertNodeAtEnd)( &doc->root, node);
                doctype = node;
            }
            else
            {
                TY_(ReportError)(doc, &doc->root, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
            }
            continue;
        }

        if (node->type == EndTag)
        {
            TY_(ReportError)(doc, &doc->root, node, DISCARDING_UNEXPECTED);
            TY_(FreeNode)( doc, node);
            continue;
        }

        if (node->type == StartTag && nodeIsHTML(node))
        {
            AttVal *xmlns;

            xmlns = TY_(AttrGetById)(node, TidyAttr_XMLNS);

            if (AttrValueIs(xmlns, XHTML_NAMESPACE))
            {
                Bool htmlOut = cfgBool( doc, TidyHtmlOut );
                doc->lexer->isvoyager = yes;                  /* Unless plain HTML */
                TY_(SetOptionBool)( doc, TidyXhtmlOut, !htmlOut ); /* is specified, output*/
                TY_(SetOptionBool)( doc, TidyXmlOut, !htmlOut );   /* will be XHTML. */

                /* adjust other config options, just as in config.c */
                if ( !htmlOut )
                {
                    TY_(SetOptionBool)( doc, TidyUpperCaseTags, no );
                    TY_(SetOptionBool)( doc, TidyUpperCaseAttrs, no );
                }
            }
        }

        if ( node->type != StartTag || !nodeIsHTML(node) )
        {
            TY_(UngetToken)( doc );
            html = TY_(InferredTag)(doc, TidyTag_HTML);
        }
        else
            html = node;

        if (!TY_(FindDocType)(doc))
            TY_(ReportError)(doc, NULL, NULL, MISSING_DOCTYPE);

        TY_(InsertNodeAtEnd)( &doc->root, html);
        TY_(ParseHTML)( doc, html, IgnoreWhitespace );
        break;
    }

#if SUPPORT_ACCESSIBILITY_CHECKS
    /* do this before any more document fixes */
    if ( cfg( doc, TidyAccessibilityCheckLevel ) > 0 )
        TY_(AccessibilityChecks)( doc );
#endif /* #if SUPPORT_ACCESSIBILITY_CHECKS */

    if (!TY_(FindHTML)(doc))
    {
        /* a later check should complain if <body> is empty */
        html = TY_(InferredTag)(doc, TidyTag_HTML);
        TY_(InsertNodeAtEnd)( &doc->root, html);
        TY_(ParseHTML)(doc, html, IgnoreWhitespace);
    }

    if (!TY_(FindTITLE)(doc))
    {
        Node* head = TY_(FindHEAD)(doc);
        TY_(ReportError)(doc, head, NULL, MISSING_TITLE_ELEMENT);
        TY_(InsertNodeAtEnd)(head, TY_(InferredTag)(doc, TidyTag_TITLE));
    }

    AttributeChecks(doc, &doc->root);
    ReplaceObsoleteElements(doc, &doc->root);
    TY_(DropEmptyElements)(doc, &doc->root);
    CleanSpaces(doc, &doc->root);

    if (cfgBool(doc, TidyEncloseBodyText))
        EncloseBodyText(doc);
    if (cfgBool(doc, TidyEncloseBlockText))
        EncloseBlockText(doc, &doc->root);
}

Bool TY_(XMLPreserveWhiteSpace)( TidyDocImpl* doc, Node *element)
{
    AttVal *attribute;

    /* search attributes for xml:space */
    for (attribute = element->attributes; attribute; attribute = attribute->next)
    {
        if (attrIsXML_SPACE(attribute))
        {
            if (AttrValueIs(attribute, "preserve"))
                return yes;

            return no;
        }
    }

    if (element->element == NULL)
        return no;
        
    /* kludge for html docs without explicit xml:space attribute */
    if (nodeIsPRE(element)    ||
        nodeIsSCRIPT(element) ||
        nodeIsSTYLE(element)  ||
        TY_(FindParser)(doc, element) == TY_(ParsePre))
        return yes;

    /* kludge for XSL docs */
    if ( TY_(tmbstrcasecmp)(element->element, "xsl:text") == 0 )
        return yes;

    return no;
}

/*
  XML documents
*/
static void ParseXMLElement(TidyDocImpl* doc, Node *element, GetTokenMode mode)
{
    Lexer* lexer = doc->lexer;
    Node *node;

    /* if node is pre or has xml:space="preserve" then do so */

    if ( TY_(XMLPreserveWhiteSpace)(doc, element) )
        mode = Preformatted;

    while ((node = TY_(GetToken)(doc, mode)) != NULL)
    {
        if (node->type == EndTag &&
           node->element && element->element &&
           TY_(tmbstrcmp)(node->element, element->element) == 0)
        {
            TY_(FreeNode)( doc, node);
            element->closed = yes;
            break;
        }

        /* discard unexpected end tags */
        if (node->type == EndTag)
        {
            if (element)
                TY_(ReportFatal)(doc, element, node, UNEXPECTED_ENDTAG_IN);
            else
                TY_(ReportFatal)(doc, element, node, UNEXPECTED_ENDTAG);

            TY_(FreeNode)( doc, node);
            continue;
        }

        /* parse content on seeing start tag */
        if (node->type == StartTag)
            ParseXMLElement( doc, node, mode );

        TY_(InsertNodeAtEnd)(element, node);
    }

    /*
     if first child is text then trim initial space and
     delete text node if it is empty.
    */

    node = element->content;

    if (TY_(nodeIsText)(node) && mode != Preformatted)
    {
        if ( lexer->lexbuf[node->start] == ' ' )
        {
            node->start++;

            if (node->start >= node->end)
                TY_(DiscardElement)( doc, node );
        }
    }

    /*
     if last child is text then trim final space and
     delete the text node if it is empty
    */

    node = element->last;

    if (TY_(nodeIsText)(node) && mode != Preformatted)
    {
        if ( lexer->lexbuf[node->end - 1] == ' ' )
        {
            node->end--;

            if (node->start >= node->end)
                TY_(DiscardElement)( doc, node );
        }
    }
}

void TY_(ParseXMLDocument)(TidyDocImpl* doc)
{
    Node *node, *doctype = NULL;

    TY_(SetOptionBool)( doc, TidyXmlTags, yes );

    while ((node = TY_(GetToken)(doc, IgnoreWhitespace)) != NULL)
    {
        /* discard unexpected end tags */
        if (node->type == EndTag)
        {
            TY_(ReportError)(doc, NULL, node, UNEXPECTED_ENDTAG);
            TY_(FreeNode)( doc, node);
            continue;
        }

         /* deal with comments etc. */
        if (InsertMisc( &doc->root, node))
            continue;

        if (node->type == DocTypeTag)
        {
            if (doctype == NULL)
            {
                TY_(InsertNodeAtEnd)( &doc->root, node);
                doctype = node;
            }
            else
            {
                TY_(ReportError)(doc, &doc->root, node, DISCARDING_UNEXPECTED);
                TY_(FreeNode)( doc, node);
            }
            continue;
        }

        if (node->type == StartEndTag)
        {
            TY_(InsertNodeAtEnd)( &doc->root, node);
            continue;
        }

       /* if start tag then parse element's content */
        if (node->type == StartTag)
        {
            TY_(InsertNodeAtEnd)( &doc->root, node );
            ParseXMLElement( doc, node, IgnoreWhitespace );
            continue;
        }

        TY_(ReportError)(doc, &doc->root, node, DISCARDING_UNEXPECTED);
        TY_(FreeNode)( doc, node);
    }

    /* ensure presence of initial <?xml version="1.0"?> */
    if ( cfgBool(doc, TidyXmlDecl) )
        TY_(FixXmlDecl)( doc );
}

/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
