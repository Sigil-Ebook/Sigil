/* attrask.c -- Interrogate attribute type

  (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.
  
  CVS Info:
    $Author: arnaud02 $ 
    $Date: 2006/09/12 15:14:44 $ 
    $Revision: 1.5 $ 

*/

#include "tidy-int.h"
#include "tidy.h"
#include "attrs.h"

Bool TIDY_CALL tidyAttrIsHREF( TidyAttr tattr )
{
    return attrIsHREF( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsSRC( TidyAttr tattr )
{
    return attrIsSRC( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsID( TidyAttr tattr )
{
    return attrIsID( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsNAME( TidyAttr tattr )
{
    return attrIsNAME( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsSUMMARY( TidyAttr tattr )
{
    return attrIsSUMMARY( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsALT( TidyAttr tattr )
{
    return attrIsALT( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsLONGDESC( TidyAttr tattr )
{
    return attrIsLONGDESC( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsUSEMAP( TidyAttr tattr )
{
    return attrIsUSEMAP( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsISMAP( TidyAttr tattr )
{
    return attrIsISMAP( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsLANGUAGE( TidyAttr tattr )
{
    return attrIsLANGUAGE( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsTYPE( TidyAttr tattr )
{
    return attrIsTYPE( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsVALUE( TidyAttr tattr )
{
    return attrIsVALUE( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsCONTENT( TidyAttr tattr )
{
    return attrIsCONTENT( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsTITLE( TidyAttr tattr )
{
    return attrIsTITLE( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsXMLNS( TidyAttr tattr )
{
    return attrIsXMLNS( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsDATAFLD( TidyAttr tattr )
{
    return attrIsDATAFLD( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsWIDTH( TidyAttr tattr )
{
    return attrIsWIDTH( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsHEIGHT( TidyAttr tattr )
{
    return attrIsHEIGHT( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsFOR( TidyAttr tattr )
{
    return attrIsFOR( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsSELECTED( TidyAttr tattr )
{
    return attrIsSELECTED( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsCHECKED( TidyAttr tattr )
{
    return attrIsCHECKED( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsLANG( TidyAttr tattr )
{
    return attrIsLANG( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsTARGET( TidyAttr tattr )
{
    return attrIsTARGET( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsHTTP_EQUIV( TidyAttr tattr )
{
    return attrIsHTTP_EQUIV( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsREL( TidyAttr tattr )
{
    return attrIsREL( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsEvent( TidyAttr tattr )
{
    return TY_(attrIsEvent)( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnMOUSEMOVE( TidyAttr tattr )
{
    return attrIsOnMOUSEMOVE( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnMOUSEDOWN( TidyAttr tattr )
{
    return attrIsOnMOUSEDOWN( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnMOUSEUP( TidyAttr tattr )
{
    return attrIsOnMOUSEUP( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnCLICK( TidyAttr tattr )
{
    return attrIsOnCLICK( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnMOUSEOVER( TidyAttr tattr )
{
    return attrIsOnMOUSEOVER( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnMOUSEOUT( TidyAttr tattr )
{
    return attrIsOnMOUSEOUT( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnKEYDOWN( TidyAttr tattr )
{
    return attrIsOnKEYDOWN( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnKEYUP( TidyAttr tattr )
{
    return attrIsOnKEYUP( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnKEYPRESS( TidyAttr tattr )
{
    return attrIsOnKEYPRESS( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnFOCUS( TidyAttr tattr )
{
    return attrIsOnFOCUS( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsOnBLUR( TidyAttr tattr )
{
    return attrIsOnBLUR( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsBGCOLOR( TidyAttr tattr )
{
    return attrIsBGCOLOR( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsLINK( TidyAttr tattr )
{
    return attrIsLINK( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsALINK( TidyAttr tattr )
{
    return attrIsALINK( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsVLINK( TidyAttr tattr )
{
    return attrIsVLINK( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsTEXT( TidyAttr tattr )
{
    return attrIsTEXT( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsSTYLE( TidyAttr tattr )
{
    return attrIsSTYLE( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsABBR( TidyAttr tattr )
{
    return attrIsABBR( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsCOLSPAN( TidyAttr tattr )
{
    return attrIsCOLSPAN( tidyAttrToImpl(tattr) );
}
Bool TIDY_CALL tidyAttrIsROWSPAN( TidyAttr tattr )
{
    return attrIsROWSPAN( tidyAttrToImpl(tattr) );
}

/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
