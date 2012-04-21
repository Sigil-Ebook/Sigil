#ifndef __PARSER_H__
#define __PARSER_H__

/* parser.h -- HTML Parser

  (c) 1998-2007 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.
  
  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2007/05/30 16:47:31 $ 
    $Revision: 1.14 $ 

*/

#include "forward.h"

Bool TY_(CheckNodeIntegrity)(Node *node);

Bool TY_(TextNodeEndWithSpace)( Lexer *lexer, Node *node );

/*
 used to determine how attributes
 without values should be printed
 this was introduced to deal with
 user defined tags e.g. Cold Fusion
*/
Bool TY_(IsNewNode)(Node *node);

void TY_(CoerceNode)(TidyDocImpl* doc, Node *node, TidyTagId tid, Bool obsolete, Bool expected);

/* extract a node and its children from a markup tree */
Node *TY_(RemoveNode)(Node *node);

/* remove node from markup tree and discard it */
Node *TY_(DiscardElement)( TidyDocImpl* doc, Node *element);

/* insert node into markup tree as the firt element
 of content of element */
void TY_(InsertNodeAtStart)(Node *element, Node *node);

/* insert node into markup tree as the last element
 of content of "element" */
void TY_(InsertNodeAtEnd)(Node *element, Node *node);

/* insert node into markup tree before element */
void TY_(InsertNodeBeforeElement)(Node *element, Node *node);

/* insert node into markup tree after element */
void TY_(InsertNodeAfterElement)(Node *element, Node *node);

Node *TY_(TrimEmptyElement)( TidyDocImpl* doc, Node *element );
Node* TY_(DropEmptyElements)(TidyDocImpl* doc, Node* node);


/* assumes node is a text node */
Bool TY_(IsBlank)(Lexer *lexer, Node *node);

Bool TY_(IsJavaScript)(Node *node);

/*
  HTML is the top level element
*/
void TY_(ParseDocument)( TidyDocImpl* doc );



/*
  XML documents
*/
Bool TY_(XMLPreserveWhiteSpace)( TidyDocImpl* doc, Node *element );

void TY_(ParseXMLDocument)( TidyDocImpl* doc );

#endif /* __PARSER_H__ */
