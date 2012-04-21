#ifndef __CLEAN_H__
#define __CLEAN_H__

/* clean.h -- clean up misuse of presentation markup

  (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info:
    $Author: arnaud02 $ 
    $Date: 2006/09/12 15:14:44 $ 
    $Revision: 1.14 $ 

*/

void TY_(FixNodeLinks)(Node *node);

void TY_(FreeStyles)( TidyDocImpl* doc );

/* Add class="foo" to node
*/
void TY_(AddStyleAsClass)( TidyDocImpl* doc, Node *node, ctmbstr stylevalue );
void TY_(AddStyleProperty)(TidyDocImpl* doc, Node *node, ctmbstr property );

void TY_(CleanDocument)( TidyDocImpl* doc );

/* simplifies <b><b> ... </b> ...</b> etc. */
void TY_(NestedEmphasis)( TidyDocImpl* doc, Node* node );

/* replace i by em and b by strong */
void TY_(EmFromI)( TidyDocImpl* doc, Node* node );

/*
 Some people use dir or ul without an li
 to indent the content. The pattern to
 look for is a list with a single implicit
 li. This is recursively replaced by an
 implicit blockquote.
*/
void TY_(List2BQ)( TidyDocImpl* doc, Node* node );

/*
 Replace implicit blockquote by div with an indent
 taking care to reduce nested blockquotes to a single
 div with the indent set to match the nesting depth
*/
void TY_(BQ2Div)( TidyDocImpl* doc, Node* node );


void TY_(DropSections)( TidyDocImpl* doc, Node* node );


/*
 This is a major clean up to strip out all the extra stuff you get
 when you save as web page from Word 2000. It doesn't yet know what
 to do with VML tags, but these will appear as errors unless you
 declare them as new tags, such as o:p which needs to be declared
 as inline.
*/
void TY_(CleanWord2000)( TidyDocImpl* doc, Node *node);

Bool TY_(IsWord2000)( TidyDocImpl* doc );

/* where appropriate move object elements from head to body */
void TY_(BumpObject)( TidyDocImpl* doc, Node *html );

/* This is disabled due to http://tidy.sf.net/bug/681116 */
#if 0
void TY_(FixBrakes)( TidyDocImpl* pDoc, Node *pParent );
#endif

void TY_(VerifyHTTPEquiv)( TidyDocImpl* pDoc, Node *pParent );

void TY_(DropComments)(TidyDocImpl* doc, Node* node);
void TY_(DropFontElements)(TidyDocImpl* doc, Node* node, Node **pnode);
void TY_(WbrToSpace)(TidyDocImpl* doc, Node* node);
void TY_(DowngradeTypography)(TidyDocImpl* doc, Node* node);
void TY_(ReplacePreformattedSpaces)(TidyDocImpl* doc, Node* node);
void TY_(NormalizeSpaces)(Lexer *lexer, Node *node);
void TY_(ConvertCDATANodes)(TidyDocImpl* doc, Node* node);

void TY_(FixAnchors)(TidyDocImpl* doc, Node *node, Bool wantName, Bool wantId);
void TY_(FixXhtmlNamespace)(TidyDocImpl* doc, Bool wantXmlns);
void TY_(FixLanguageInformation)(TidyDocImpl* doc, Node* node, Bool wantXmlLang, Bool wantLang);


#endif /* __CLEAN_H__ */
