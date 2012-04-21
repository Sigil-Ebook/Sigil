/* tagask.c -- Interrogate node type

  (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2006/09/12 15:14:44 $ 
    $Revision: 1.6 $ 

*/

#include "tidy-int.h"
#include "tags.h"
#include "tidy.h"

Bool TIDY_CALL tidyNodeIsText( TidyNode tnod )
{ return TY_(nodeIsText)( tidyNodeToImpl(tnod) );
}
Bool tidyNodeCMIsBlock( TidyNode tnod ); /* not exported yet */
Bool tidyNodeCMIsBlock( TidyNode tnod )
{ return TY_(nodeCMIsBlock)( tidyNodeToImpl(tnod) );
}
Bool tidyNodeCMIsInline( TidyNode tnod ); /* not exported yet */
Bool tidyNodeCMIsInline( TidyNode tnod )
{ return TY_(nodeCMIsInline)( tidyNodeToImpl(tnod) );
}
Bool tidyNodeCMIsEmpty( TidyNode tnod ); /* not exported yet */
Bool tidyNodeCMIsEmpty( TidyNode tnod )
{ return TY_(nodeCMIsEmpty)( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsHeader( TidyNode tnod )
{ return TY_(nodeIsHeader)( tidyNodeToImpl(tnod) );
}

Bool TIDY_CALL tidyNodeIsHTML( TidyNode tnod )
{ return nodeIsHTML( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsHEAD( TidyNode tnod )
{ return nodeIsHEAD( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsTITLE( TidyNode tnod )
{ return nodeIsTITLE( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsBASE( TidyNode tnod )
{ return nodeIsBASE( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsMETA( TidyNode tnod )
{ return nodeIsMETA( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsBODY( TidyNode tnod )
{ return nodeIsBODY( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsFRAMESET( TidyNode tnod )
{ return nodeIsFRAMESET( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsFRAME( TidyNode tnod )
{ return nodeIsFRAME( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsIFRAME( TidyNode tnod )
{ return nodeIsIFRAME( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsNOFRAMES( TidyNode tnod )
{ return nodeIsNOFRAMES( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsHR( TidyNode tnod )
{ return nodeIsHR( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsH1( TidyNode tnod )
{ return nodeIsH1( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsH2( TidyNode tnod )
{ return nodeIsH2( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsPRE( TidyNode tnod )
{ return nodeIsPRE( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsLISTING( TidyNode tnod )
{ return nodeIsLISTING( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsP( TidyNode tnod )
{ return nodeIsP( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsUL( TidyNode tnod )
{ return nodeIsUL( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsOL( TidyNode tnod )
{ return nodeIsOL( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsDL( TidyNode tnod )
{ return nodeIsDL( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsDIR( TidyNode tnod )
{ return nodeIsDIR( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsLI( TidyNode tnod )
{ return nodeIsLI( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsDT( TidyNode tnod )
{ return nodeIsDT( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsDD( TidyNode tnod )
{ return nodeIsDD( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsTABLE( TidyNode tnod )
{ return nodeIsTABLE( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsCAPTION( TidyNode tnod )
{ return nodeIsCAPTION( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsTD( TidyNode tnod )
{ return nodeIsTD( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsTH( TidyNode tnod )
{ return nodeIsTH( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsTR( TidyNode tnod )
{ return nodeIsTR( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsCOL( TidyNode tnod )
{ return nodeIsCOL( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsCOLGROUP( TidyNode tnod )
{ return nodeIsCOLGROUP( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsBR( TidyNode tnod )
{ return nodeIsBR( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsA( TidyNode tnod )
{ return nodeIsA( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsLINK( TidyNode tnod )
{ return nodeIsLINK( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsB( TidyNode tnod )
{ return nodeIsB( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsI( TidyNode tnod )
{ return nodeIsI( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsSTRONG( TidyNode tnod )
{ return nodeIsSTRONG( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsEM( TidyNode tnod )
{ return nodeIsEM( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsBIG( TidyNode tnod )
{ return nodeIsBIG( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsSMALL( TidyNode tnod )
{ return nodeIsSMALL( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsPARAM( TidyNode tnod )
{ return nodeIsPARAM( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsOPTION( TidyNode tnod )
{ return nodeIsOPTION( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsOPTGROUP( TidyNode tnod )
{ return nodeIsOPTGROUP( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsIMG( TidyNode tnod )
{ return nodeIsIMG( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsMAP( TidyNode tnod )
{ return nodeIsMAP( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsAREA( TidyNode tnod )
{ return nodeIsAREA( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsNOBR( TidyNode tnod )
{ return nodeIsNOBR( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsWBR( TidyNode tnod )
{ return nodeIsWBR( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsFONT( TidyNode tnod )
{ return nodeIsFONT( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsLAYER( TidyNode tnod )
{ return nodeIsLAYER( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsSPACER( TidyNode tnod )
{ return nodeIsSPACER( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsCENTER( TidyNode tnod )
{ return nodeIsCENTER( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsSTYLE( TidyNode tnod )
{ return nodeIsSTYLE( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsSCRIPT( TidyNode tnod )
{ return nodeIsSCRIPT( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsNOSCRIPT( TidyNode tnod )
{ return nodeIsNOSCRIPT( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsFORM( TidyNode tnod )
{ return nodeIsFORM( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsTEXTAREA( TidyNode tnod )
{ return nodeIsTEXTAREA( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsBLOCKQUOTE( TidyNode tnod )
{ return nodeIsBLOCKQUOTE( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsAPPLET( TidyNode tnod )
{ return nodeIsAPPLET( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsOBJECT( TidyNode tnod )
{ return nodeIsOBJECT( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsDIV( TidyNode tnod )
{ return nodeIsDIV( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsSPAN( TidyNode tnod )
{ return nodeIsSPAN( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsINPUT( TidyNode tnod )
{ return nodeIsINPUT( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsQ( TidyNode tnod )
{ return nodeIsQ( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsLABEL( TidyNode tnod )
{ return nodeIsLABEL( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsH3( TidyNode tnod )
{ return nodeIsH3( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsH4( TidyNode tnod )
{ return nodeIsH4( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsH5( TidyNode tnod )
{ return nodeIsH5( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsH6( TidyNode tnod )
{ return nodeIsH6( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsADDRESS( TidyNode tnod )
{ return nodeIsADDRESS( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsXMP( TidyNode tnod )
{ return nodeIsXMP( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsSELECT( TidyNode tnod )
{ return nodeIsSELECT( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsBLINK( TidyNode tnod )
{ return nodeIsBLINK( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsMARQUEE( TidyNode tnod )
{ return nodeIsMARQUEE( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsEMBED( TidyNode tnod )
{ return nodeIsEMBED( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsBASEFONT( TidyNode tnod )
{ return nodeIsBASEFONT( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsISINDEX( TidyNode tnod )
{ return nodeIsISINDEX( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsS( TidyNode tnod )
{ return nodeIsS( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsSTRIKE( TidyNode tnod )
{ return nodeIsSTRIKE( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsU( TidyNode tnod )
{ return nodeIsU( tidyNodeToImpl(tnod) );
}
Bool TIDY_CALL tidyNodeIsMENU( TidyNode tnod )
{ return nodeIsMENU( tidyNodeToImpl(tnod) );
}


/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
