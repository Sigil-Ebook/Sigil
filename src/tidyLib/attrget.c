/* attrget.c -- Locate attribute value by type

  (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.
  
  CVS Info:
    $Author: arnaud02 $ 
    $Date: 2006/09/12 15:14:44 $ 
    $Revision: 1.6 $ 

*/

#include "tidy-int.h"
#include "tags.h"
#include "attrs.h"
#include "tidy.h"

TidyAttr TIDY_CALL tidyAttrGetById( TidyNode tnod, TidyAttrId attId )
{
    Node* nimp = tidyNodeToImpl(tnod);
    return tidyImplToAttr( TY_(AttrGetById)( nimp, attId ) );
}
TidyAttr TIDY_CALL tidyAttrGetHREF( TidyNode tnod )
{
    return tidyImplToAttr( attrGetHREF( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetSRC( TidyNode tnod )
{
    return tidyImplToAttr( attrGetSRC( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetID( TidyNode tnod )
{
    return tidyImplToAttr( attrGetID( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetNAME( TidyNode tnod )
{
    return tidyImplToAttr( attrGetNAME( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetSUMMARY( TidyNode tnod )
{
    return tidyImplToAttr( attrGetSUMMARY( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetALT( TidyNode tnod )
{
    return tidyImplToAttr( attrGetALT( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetLONGDESC( TidyNode tnod )
{
    return tidyImplToAttr( attrGetLONGDESC( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetUSEMAP( TidyNode tnod )
{
    return tidyImplToAttr( attrGetUSEMAP( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetISMAP( TidyNode tnod )
{
    return tidyImplToAttr( attrGetISMAP( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetLANGUAGE( TidyNode tnod )
{
    return tidyImplToAttr( attrGetLANGUAGE( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetTYPE( TidyNode tnod )
{
    return tidyImplToAttr( attrGetTYPE( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetVALUE( TidyNode tnod )
{
    return tidyImplToAttr( attrGetVALUE( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetCONTENT( TidyNode tnod )
{
    return tidyImplToAttr( attrGetCONTENT( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetTITLE( TidyNode tnod )
{
    return tidyImplToAttr( attrGetTITLE( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetXMLNS( TidyNode tnod )
{
    return tidyImplToAttr( attrGetXMLNS( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetDATAFLD( TidyNode tnod )
{
    return tidyImplToAttr( attrGetDATAFLD( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetWIDTH( TidyNode tnod )
{
    return tidyImplToAttr( attrGetWIDTH( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetHEIGHT( TidyNode tnod )
{
    return tidyImplToAttr( attrGetHEIGHT( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetFOR( TidyNode tnod )
{
    return tidyImplToAttr( attrGetFOR( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetSELECTED( TidyNode tnod )
{
    return tidyImplToAttr( attrGetSELECTED( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetCHECKED( TidyNode tnod )
{
    return tidyImplToAttr( attrGetCHECKED( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetLANG( TidyNode tnod )
{
    return tidyImplToAttr( attrGetLANG( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetTARGET( TidyNode tnod )
{
    return tidyImplToAttr( attrGetTARGET( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetHTTP_EQUIV( TidyNode tnod )
{
    return tidyImplToAttr( attrGetHTTP_EQUIV( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetREL( TidyNode tnod )
{
    return tidyImplToAttr( attrGetREL( tidyNodeToImpl(tnod) ) );
}

TidyAttr TIDY_CALL tidyAttrGetOnMOUSEMOVE( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnMOUSEMOVE( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetOnMOUSEDOWN( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnMOUSEDOWN( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetOnMOUSEUP( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnMOUSEUP( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetOnCLICK( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnCLICK( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetOnMOUSEOVER( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnMOUSEOVER( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetOnMOUSEOUT( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnMOUSEOUT( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetOnKEYDOWN( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnKEYDOWN( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetOnKEYUP( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnKEYUP( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetOnKEYPRESS( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnKEYPRESS( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetOnFOCUS( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnFOCUS( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetOnBLUR( TidyNode tnod )
{
    return tidyImplToAttr( attrGetOnBLUR( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetBGCOLOR( TidyNode tnod )
{
    return tidyImplToAttr( attrGetBGCOLOR( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetLINK( TidyNode tnod )
{
    return tidyImplToAttr( attrGetLINK( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetALINK( TidyNode tnod )
{
    return tidyImplToAttr( attrGetALINK( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetVLINK( TidyNode tnod )
{
    return tidyImplToAttr( attrGetVLINK( tidyNodeToImpl(tnod) ) );
}

TidyAttr TIDY_CALL tidyAttrGetTEXT( TidyNode tnod )
{
    return tidyImplToAttr( attrGetTEXT( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetSTYLE( TidyNode tnod )
{
    return tidyImplToAttr( attrGetSTYLE( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetABBR( TidyNode tnod )
{
    return tidyImplToAttr( attrGetABBR( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetCOLSPAN( TidyNode tnod )
{
    return tidyImplToAttr( attrGetCOLSPAN( tidyNodeToImpl(tnod) ) );
}
TidyAttr TIDY_CALL tidyAttrGetROWSPAN( TidyNode tnod )
{
    return tidyImplToAttr( attrGetROWSPAN( tidyNodeToImpl(tnod) ) );
}

/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
