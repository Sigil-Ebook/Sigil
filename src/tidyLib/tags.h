#ifndef __TAGS_H__
#define __TAGS_H__

/* tags.h -- recognize HTML tags

  (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2006/12/15 10:17:55 $ 
    $Revision: 1.20 $ 

  The HTML tags are stored as 8 bit ASCII strings.
  Use lookupw() to find a tag given a wide char string.

*/

#include "forward.h"
#include "attrdict.h"

typedef void (Parser)( TidyDocImpl* doc, Node *node, GetTokenMode mode );
typedef void (CheckAttribs)( TidyDocImpl* doc, Node *node );

/*
 Tag dictionary node
*/

/* types of tags that the user can define */
typedef enum
{
    tagtype_null = 0,
    tagtype_empty = 1,
    tagtype_inline = 2,
    tagtype_block = 4,
    tagtype_pre = 8
} UserTagType;

struct _Dict
{
    TidyTagId       id;
    tmbstr          name;
    uint            versions;
    AttrVersion const *    attrvers;
    uint            model;
    Parser*         parser;
    CheckAttribs*   chkattrs;
    Dict*           next;
};

#if !defined(ELEMENT_HASH_LOOKUP)
#define ELEMENT_HASH_LOOKUP 1
#endif

#if ELEMENT_HASH_LOOKUP
enum
{
    ELEMENT_HASH_SIZE=178u
};

struct _DictHash
{
    Dict const*         tag;
    struct _DictHash*   next;
};

typedef struct _DictHash DictHash;
#endif

struct _TidyTagImpl
{
    Dict* xml_tags;                /* placeholder for all xml tags */
    Dict* declared_tag_list;       /* User declared tags */
#if ELEMENT_HASH_LOOKUP
    DictHash* hashtab[ELEMENT_HASH_SIZE];
#endif
};

typedef struct _TidyTagImpl TidyTagImpl;

/* interface for finding tag by name */
const Dict* TY_(LookupTagDef)( TidyTagId tid );
Bool    TY_(FindTag)( TidyDocImpl* doc, Node *node );
Parser* TY_(FindParser)( TidyDocImpl* doc, Node *node );
void    TY_(DefineTag)( TidyDocImpl* doc, UserTagType tagType, ctmbstr name );
void    TY_(FreeDeclaredTags)( TidyDocImpl* doc, UserTagType tagType ); /* tagtype_null to free all */

TidyIterator   TY_(GetDeclaredTagList)( TidyDocImpl* doc );
ctmbstr        TY_(GetNextDeclaredTag)( TidyDocImpl* doc, UserTagType tagType,
                                        TidyIterator* iter );

void TY_(InitTags)( TidyDocImpl* doc );
void TY_(FreeTags)( TidyDocImpl* doc );


/* Parser methods for tags */

Parser TY_(ParseHTML);
Parser TY_(ParseHead);
Parser TY_(ParseTitle);
Parser TY_(ParseScript);
Parser TY_(ParseFrameSet);
Parser TY_(ParseNoFrames);
Parser TY_(ParseBody);
Parser TY_(ParsePre);
Parser TY_(ParseList);
Parser TY_(ParseDefList);
Parser TY_(ParseBlock);
Parser TY_(ParseInline);
Parser TY_(ParseEmpty);
Parser TY_(ParseTableTag);
Parser TY_(ParseColGroup);
Parser TY_(ParseRowGroup);
Parser TY_(ParseRow);
Parser TY_(ParseSelect);
Parser TY_(ParseOptGroup);
Parser TY_(ParseText);

CheckAttribs TY_(CheckAttributes);

/* 0 == TidyTag_UNKNOWN */
#define TagId(node)        ((node) && (node)->tag ? (node)->tag->id : TidyTag_UNKNOWN)
#define TagIsId(node, tid) ((node) && (node)->tag && (node)->tag->id == tid)

Bool TY_(nodeIsText)( Node* node );
Bool TY_(nodeIsElement)( Node* node );

Bool TY_(nodeHasText)( TidyDocImpl* doc, Node* node );

#if 0
/* Compare & result to operand.  If equal, then all bits
** requested are set.
*/
Bool nodeMatchCM( Node* node, uint contentModel );
#endif

/* True if any of the bits requested are set.
*/
Bool TY_(nodeHasCM)( Node* node, uint contentModel );

Bool TY_(nodeCMIsBlock)( Node* node );
Bool TY_(nodeCMIsInline)( Node* node );
Bool TY_(nodeCMIsEmpty)( Node* node );


Bool TY_(nodeIsHeader)( Node* node );     /* H1, H2, ..., H6 */
uint TY_(nodeHeaderLevel)( Node* node );  /* 1, 2, ..., 6 */

#define nodeIsHTML( node )       TagIsId( node, TidyTag_HTML )
#define nodeIsHEAD( node )       TagIsId( node, TidyTag_HEAD )
#define nodeIsTITLE( node )      TagIsId( node, TidyTag_TITLE )
#define nodeIsBASE( node )       TagIsId( node, TidyTag_BASE )
#define nodeIsMETA( node )       TagIsId( node, TidyTag_META )
#define nodeIsBODY( node )       TagIsId( node, TidyTag_BODY )
#define nodeIsFRAMESET( node )   TagIsId( node, TidyTag_FRAMESET )
#define nodeIsFRAME( node )      TagIsId( node, TidyTag_FRAME )
#define nodeIsIFRAME( node )     TagIsId( node, TidyTag_IFRAME )
#define nodeIsNOFRAMES( node )   TagIsId( node, TidyTag_NOFRAMES )
#define nodeIsHR( node )         TagIsId( node, TidyTag_HR )
#define nodeIsH1( node )         TagIsId( node, TidyTag_H1 )
#define nodeIsH2( node )         TagIsId( node, TidyTag_H2 )
#define nodeIsPRE( node )        TagIsId( node, TidyTag_PRE )
#define nodeIsLISTING( node )    TagIsId( node, TidyTag_LISTING )
#define nodeIsP( node )          TagIsId( node, TidyTag_P )
#define nodeIsUL( node )         TagIsId( node, TidyTag_UL )
#define nodeIsOL( node )         TagIsId( node, TidyTag_OL )
#define nodeIsDL( node )         TagIsId( node, TidyTag_DL )
#define nodeIsDIR( node )        TagIsId( node, TidyTag_DIR )
#define nodeIsLI( node )         TagIsId( node, TidyTag_LI )
#define nodeIsDT( node )         TagIsId( node, TidyTag_DT )
#define nodeIsDD( node )         TagIsId( node, TidyTag_DD )
#define nodeIsTABLE( node )      TagIsId( node, TidyTag_TABLE )
#define nodeIsCAPTION( node )    TagIsId( node, TidyTag_CAPTION )
#define nodeIsTD( node )         TagIsId( node, TidyTag_TD )
#define nodeIsTH( node )         TagIsId( node, TidyTag_TH )
#define nodeIsTR( node )         TagIsId( node, TidyTag_TR )
#define nodeIsCOL( node )        TagIsId( node, TidyTag_COL )
#define nodeIsCOLGROUP( node )   TagIsId( node, TidyTag_COLGROUP )
#define nodeIsBR( node )         TagIsId( node, TidyTag_BR )
#define nodeIsA( node )          TagIsId( node, TidyTag_A )
#define nodeIsLINK( node )       TagIsId( node, TidyTag_LINK )
#define nodeIsB( node )          TagIsId( node, TidyTag_B )
#define nodeIsI( node )          TagIsId( node, TidyTag_I )
#define nodeIsSTRONG( node )     TagIsId( node, TidyTag_STRONG )
#define nodeIsEM( node )         TagIsId( node, TidyTag_EM )
#define nodeIsBIG( node )        TagIsId( node, TidyTag_BIG )
#define nodeIsSMALL( node )      TagIsId( node, TidyTag_SMALL )
#define nodeIsPARAM( node )      TagIsId( node, TidyTag_PARAM )
#define nodeIsOPTION( node )     TagIsId( node, TidyTag_OPTION )
#define nodeIsOPTGROUP( node )   TagIsId( node, TidyTag_OPTGROUP )
#define nodeIsIMG( node )        TagIsId( node, TidyTag_IMG )
#define nodeIsMAP( node )        TagIsId( node, TidyTag_MAP )
#define nodeIsAREA( node )       TagIsId( node, TidyTag_AREA )
#define nodeIsNOBR( node )       TagIsId( node, TidyTag_NOBR )
#define nodeIsWBR( node )        TagIsId( node, TidyTag_WBR )
#define nodeIsFONT( node )       TagIsId( node, TidyTag_FONT )
#define nodeIsLAYER( node )      TagIsId( node, TidyTag_LAYER )
#define nodeIsSPACER( node )     TagIsId( node, TidyTag_SPACER )
#define nodeIsCENTER( node )     TagIsId( node, TidyTag_CENTER )
#define nodeIsSTYLE( node )      TagIsId( node, TidyTag_STYLE )
#define nodeIsSCRIPT( node )     TagIsId( node, TidyTag_SCRIPT )
#define nodeIsNOSCRIPT( node )   TagIsId( node, TidyTag_NOSCRIPT )
#define nodeIsFORM( node )       TagIsId( node, TidyTag_FORM )
#define nodeIsTEXTAREA( node )   TagIsId( node, TidyTag_TEXTAREA )
#define nodeIsBLOCKQUOTE( node ) TagIsId( node, TidyTag_BLOCKQUOTE )
#define nodeIsAPPLET( node )     TagIsId( node, TidyTag_APPLET )
#define nodeIsOBJECT( node )     TagIsId( node, TidyTag_OBJECT )
#define nodeIsDIV( node )        TagIsId( node, TidyTag_DIV )
#define nodeIsSPAN( node )       TagIsId( node, TidyTag_SPAN )
#define nodeIsINPUT( node )      TagIsId( node, TidyTag_INPUT )
#define nodeIsQ( node )          TagIsId( node, TidyTag_Q )
#define nodeIsLABEL( node )      TagIsId( node, TidyTag_LABEL )
#define nodeIsH3( node )         TagIsId( node, TidyTag_H3 )
#define nodeIsH4( node )         TagIsId( node, TidyTag_H4 )
#define nodeIsH5( node )         TagIsId( node, TidyTag_H5 )
#define nodeIsH6( node )         TagIsId( node, TidyTag_H6 )
#define nodeIsADDRESS( node )    TagIsId( node, TidyTag_ADDRESS )
#define nodeIsXMP( node )        TagIsId( node, TidyTag_XMP )
#define nodeIsSELECT( node )     TagIsId( node, TidyTag_SELECT )
#define nodeIsBLINK( node )      TagIsId( node, TidyTag_BLINK )
#define nodeIsMARQUEE( node )    TagIsId( node, TidyTag_MARQUEE )
#define nodeIsEMBED( node )      TagIsId( node, TidyTag_EMBED )
#define nodeIsBASEFONT( node )   TagIsId( node, TidyTag_BASEFONT )
#define nodeIsISINDEX( node )    TagIsId( node, TidyTag_ISINDEX )
#define nodeIsS( node )          TagIsId( node, TidyTag_S )
#define nodeIsSTRIKE( node )     TagIsId( node, TidyTag_STRIKE )
#define nodeIsSUB( node )        TagIsId( node, TidyTag_SUB )
#define nodeIsSUP( node )        TagIsId( node, TidyTag_SUP )
#define nodeIsU( node )          TagIsId( node, TidyTag_U )
#define nodeIsMENU( node )       TagIsId( node, TidyTag_MENU )
#define nodeIsBUTTON( node )     TagIsId( node, TidyTag_BUTTON )


#endif /* __TAGS_H__ */
