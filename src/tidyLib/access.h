#ifndef __ACCESS_H__
#define __ACCESS_H__

/* access.h -- carry out accessibility checks

  Copyright University of Toronto
  Portions (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.
  
  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2006/09/12 15:14:44 $ 
    $Revision: 1.7 $ 

*/

/*********************************************************************
* AccessibilityChecks
*
* Carries out processes for all accessibility checks.  Traverses
* through all the content within the tree and evaluates the tags for
* accessibility.
*
* To perform the following checks, 'AccessibilityChecks' must be
* called AFTER the tree structure has been formed.
*
* If, in the command prompt, there is no specification of which
* accessibility priorities to check, no accessibility checks will be 
* performed.  (ie. '1' for priority 1, '2' for priorities 1 and 2, 
*                  and '3') for priorities 1, 2 and 3.)
*
* Copyright University of Toronto
* Programmed by: Mike Lam and Chris Ridpath
* Modifications by : Terry Teague (TRT)
*
*********************************************************************/


#include "forward.h"

#if SUPPORT_ACCESSIBILITY_CHECKS

/* The accessibility checks to perform depending on user's desire.

   1. priority 1
   2. priority 1 & 2
   3. priority 1, 2, & 3
*/

/* Determines if the client-side text link is found within the document
typedef struct AreaLinks
{
    struct AreaLinks* next;
    char* link;
    Bool HasBeenFound;
} AreaLinks;
*/

enum {
  TEXTBUF_SIZE=128u
};

struct _TidyAccessImpl;
typedef struct _TidyAccessImpl TidyAccessImpl;

struct _TidyAccessImpl
{
    /* gets set from Tidy variable AccessibilityCheckLevel */
    int PRIORITYCHK;

    /* Number of characters that are found within the concatenated text */
    int counter;

    /* list of characters in the text nodes found within a container element */
    tmbchar textNode[ TEXTBUF_SIZE ]; 

    /* The list of characters found within one text node */
    tmbchar text[ TEXTBUF_SIZE ]; 

    /* Number of frame elements found within a frameset */
    int numFrames; 

    /* Number of 'longdesc' attributes found within a frameset */
    int HasCheckedLongDesc; 

    int  CheckedHeaders;
    int  ListElements;
    int  OtherListElements;

    /* For 'USEMAP' identifier */
    Bool HasUseMap; 
    Bool HasName; 
    Bool HasMap;

    /* For tracking nodes that are deleted from the original parse tree - TRT */
    /* Node *access_tree; */

    Bool HasTH;
    Bool HasValidFor;
    Bool HasValidId;
    Bool HasValidRowHeaders;
    Bool HasValidColumnHeaders;
    Bool HasInvalidRowHeader;
    Bool HasInvalidColumnHeader;
    int  ForID;

    /* List containing map-links
    AreaLinks* links;
    AreaLinks* start;
    AreaLinks* current;
    */

};


/* 
    Determines which error/warning message should be displayed,
    depending on the error code that was called.

    Offset accessibility error codes by FIRST_ACCESS_ERR to avoid conflict with
    other error codes defined in message.h and used in localize.c.
*/
enum accessErrorCodes
{
                           FIRST_ACCESS_ERR = 1000,    /* must be first */
 
    /* [1.1.1.1] */        IMG_MISSING_ALT,
    /* [1.1.1.2] */        IMG_ALT_SUSPICIOUS_FILENAME,
    /* [1.1.1.3] */        IMG_ALT_SUSPICIOUS_FILE_SIZE,
    /* [1.1.1.4] */        IMG_ALT_SUSPICIOUS_PLACEHOLDER,
    /* [1.1.1.10] */       IMG_ALT_SUSPICIOUS_TOO_LONG,
    /* [1.1.1.11] */       IMG_MISSING_ALT_BULLET,
    /* [1.1.1.12] */       IMG_MISSING_ALT_H_RULE,
    /* [1.1.2.1] */        IMG_MISSING_LONGDESC_DLINK,
    /* [1.1.2.2] */        IMG_MISSING_DLINK,
    /* [1.1.2.3] */        IMG_MISSING_LONGDESC,
    /* [1.1.2.5] */        LONGDESC_NOT_REQUIRED,
    /* [1.1.3.1] */        IMG_BUTTON_MISSING_ALT, 
    /* [1.1.4.1] */        APPLET_MISSING_ALT,
    /* [1.1.5.1] */        OBJECT_MISSING_ALT,
    /* [1.1.6.1] */        AUDIO_MISSING_TEXT_WAV,
    /* [1.1.6.2] */        AUDIO_MISSING_TEXT_AU,
    /* [1.1.6.3] */        AUDIO_MISSING_TEXT_AIFF,
    /* [1.1.6.4] */        AUDIO_MISSING_TEXT_SND,
    /* [1.1.6.5] */        AUDIO_MISSING_TEXT_RA,
    /* [1.1.6.6] */        AUDIO_MISSING_TEXT_RM,
    /* [1.1.8.1] */        FRAME_MISSING_LONGDESC,
    /* [1.1.9.1] */        AREA_MISSING_ALT,
    /* [1.1.10.1] */       SCRIPT_MISSING_NOSCRIPT,
    /* [1.1.12.1] */       ASCII_REQUIRES_DESCRIPTION,
    /* [1.2.1.1] */        IMG_MAP_SERVER_REQUIRES_TEXT_LINKS,
    /* [1.4.1.1] */        MULTIMEDIA_REQUIRES_TEXT,
    /* [1.5.1.1] */        IMG_MAP_CLIENT_MISSING_TEXT_LINKS,
    /* [2.1.1.1] */        INFORMATION_NOT_CONVEYED_IMAGE,
    /* [2.1.1.2] */        INFORMATION_NOT_CONVEYED_APPLET,
    /* [2.1.1.3] */        INFORMATION_NOT_CONVEYED_OBJECT,
    /* [2.1.1.4] */        INFORMATION_NOT_CONVEYED_SCRIPT,
    /* [2.1.1.5] */        INFORMATION_NOT_CONVEYED_INPUT,
    /* [2.2.1.1] */        COLOR_CONTRAST_TEXT,
    /* [2.2.1.2] */        COLOR_CONTRAST_LINK,
    /* [2.2.1.3] */        COLOR_CONTRAST_ACTIVE_LINK,
    /* [2.2.1.4] */        COLOR_CONTRAST_VISITED_LINK,
    /* [3.2.1.1] */        DOCTYPE_MISSING,
    /* [3.3.1.1] */        STYLE_SHEET_CONTROL_PRESENTATION,
    /* [3.5.1.1] */        HEADERS_IMPROPERLY_NESTED,
    /* [3.5.2.1] */        POTENTIAL_HEADER_BOLD,
    /* [3.5.2.2] */        POTENTIAL_HEADER_ITALICS,
    /* [3.5.2.3] */        POTENTIAL_HEADER_UNDERLINE,
    /* [3.5.3.1] */        HEADER_USED_FORMAT_TEXT,
    /* [3.6.1.1] */        LIST_USAGE_INVALID_UL,
    /* [3.6.1.2] */        LIST_USAGE_INVALID_OL,
    /* [3.6.1.4] */        LIST_USAGE_INVALID_LI,
    /* [4.1.1.1] */        INDICATE_CHANGES_IN_LANGUAGE,
    /* [4.3.1.1] */        LANGUAGE_NOT_IDENTIFIED,
    /* [4.3.1.1] */        LANGUAGE_INVALID,
    /* [5.1.2.1] */        DATA_TABLE_MISSING_HEADERS,
    /* [5.1.2.2] */        DATA_TABLE_MISSING_HEADERS_COLUMN,
    /* [5.1.2.3] */        DATA_TABLE_MISSING_HEADERS_ROW,
    /* [5.2.1.1] */        DATA_TABLE_REQUIRE_MARKUP_COLUMN_HEADERS,
    /* [5.2.1.2] */        DATA_TABLE_REQUIRE_MARKUP_ROW_HEADERS,
    /* [5.3.1.1] */        LAYOUT_TABLES_LINEARIZE_PROPERLY,
    /* [5.4.1.1] */        LAYOUT_TABLE_INVALID_MARKUP,
    /* [5.5.1.1] */        TABLE_MISSING_SUMMARY,
    /* [5.5.1.2] */        TABLE_SUMMARY_INVALID_NULL,
    /* [5.5.1.3] */        TABLE_SUMMARY_INVALID_SPACES,
    /* [5.5.1.6] */        TABLE_SUMMARY_INVALID_PLACEHOLDER,
    /* [5.5.2.1] */        TABLE_MISSING_CAPTION,
    /* [5.6.1.1] */        TABLE_MAY_REQUIRE_HEADER_ABBR,
    /* [5.6.1.2] */        TABLE_MAY_REQUIRE_HEADER_ABBR_NULL,
    /* [5.6.1.3] */        TABLE_MAY_REQUIRE_HEADER_ABBR_SPACES,
    /* [6.1.1.1] */        STYLESHEETS_REQUIRE_TESTING_LINK,
    /* [6.1.1.2] */        STYLESHEETS_REQUIRE_TESTING_STYLE_ELEMENT,
    /* [6.1.1.3] */        STYLESHEETS_REQUIRE_TESTING_STYLE_ATTR,
    /* [6.2.1.1] */        FRAME_SRC_INVALID,
    /* [6.2.2.1] */        TEXT_EQUIVALENTS_REQUIRE_UPDATING_APPLET,
    /* [6.2.2.2] */        TEXT_EQUIVALENTS_REQUIRE_UPDATING_SCRIPT,
    /* [6.2.2.3] */        TEXT_EQUIVALENTS_REQUIRE_UPDATING_OBJECT,
    /* [6.3.1.1] */        PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_SCRIPT,
    /* [6.3.1.2] */        PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_OBJECT,
    /* [6.3.1.3] */        PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_EMBED,
    /* [6.3.1.4] */        PROGRAMMATIC_OBJECTS_REQUIRE_TESTING_APPLET,
    /* [6.5.1.1] */        FRAME_MISSING_NOFRAMES,
    /* [6.5.1.2] */        NOFRAMES_INVALID_NO_VALUE,
    /* [6.5.1.3] */        NOFRAMES_INVALID_CONTENT,
    /* [6.5.1.4] */        NOFRAMES_INVALID_LINK,
    /* [7.1.1.1] */        REMOVE_FLICKER_SCRIPT,
    /* [7.1.1.2] */        REMOVE_FLICKER_OBJECT,
    /* [7.1.1.3] */        REMOVE_FLICKER_EMBED,
    /* [7.1.1.4] */        REMOVE_FLICKER_APPLET,
    /* [7.1.1.5] */        REMOVE_FLICKER_ANIMATED_GIF,
    /* [7.2.1.1] */        REMOVE_BLINK_MARQUEE,
    /* [7.4.1.1] */        REMOVE_AUTO_REFRESH,
    /* [7.5.1.1] */        REMOVE_AUTO_REDIRECT,
    /* [8.1.1.1] */        ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_SCRIPT,
    /* [8.1.1.2] */        ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_OBJECT,
    /* [8.1.1.3] */        ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_APPLET,
    /* [8.1.1.4] */        ENSURE_PROGRAMMATIC_OBJECTS_ACCESSIBLE_EMBED,
    /* [9.1.1.1] */        IMAGE_MAP_SERVER_SIDE_REQUIRES_CONVERSION,
    /* [9.3.1.1] */        SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_DOWN,
    /* [9.3.1.2] */        SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_UP,
    /* [9.3.1.3] */        SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_CLICK,
    /* [9.3.1.4] */        SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_OVER,
    /* [9.3.1.5] */        SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_OUT,
    /* [9.3.1.6] */        SCRIPT_NOT_KEYBOARD_ACCESSIBLE_ON_MOUSE_MOVE,
    /* [10.1.1.1] */       NEW_WINDOWS_REQUIRE_WARNING_NEW,
    /* [10.1.1.2] */       NEW_WINDOWS_REQUIRE_WARNING_BLANK,
    /* [10.2.1.1] */       LABEL_NEEDS_REPOSITIONING_BEFORE_INPUT,
    /* [10.2.1.2] */       LABEL_NEEDS_REPOSITIONING_AFTER_INPUT,
    /* [10.4.1.1] */       FORM_CONTROL_REQUIRES_DEFAULT_TEXT,
    /* [10.4.1.2] */       FORM_CONTROL_DEFAULT_TEXT_INVALID_NULL,
    /* [10.4.1.3] */       FORM_CONTROL_DEFAULT_TEXT_INVALID_SPACES,
    /* [11.2.1.1] */       REPLACE_DEPRECATED_HTML_APPLET,
    /* [11.2.1.2] */       REPLACE_DEPRECATED_HTML_BASEFONT,
    /* [11.2.1.3] */       REPLACE_DEPRECATED_HTML_CENTER,
    /* [11.2.1.4] */       REPLACE_DEPRECATED_HTML_DIR,
    /* [11.2.1.5] */       REPLACE_DEPRECATED_HTML_FONT,
    /* [11.2.1.6] */       REPLACE_DEPRECATED_HTML_ISINDEX,
    /* [11.2.1.7] */       REPLACE_DEPRECATED_HTML_MENU,
    /* [11.2.1.8] */       REPLACE_DEPRECATED_HTML_S,
    /* [11.2.1.9] */       REPLACE_DEPRECATED_HTML_STRIKE,
    /* [11.2.1.10] */      REPLACE_DEPRECATED_HTML_U,
    /* [12.1.1.1] */       FRAME_MISSING_TITLE,
    /* [12.1.1.2] */       FRAME_TITLE_INVALID_NULL,
    /* [12.1.1.3] */       FRAME_TITLE_INVALID_SPACES,
    /* [12.4.1.1] */       ASSOCIATE_LABELS_EXPLICITLY,
    /* [12.4.1.2] */       ASSOCIATE_LABELS_EXPLICITLY_FOR,
    /* [12.4.1.3] */       ASSOCIATE_LABELS_EXPLICITLY_ID,
    /* [13.1.1.1] */       LINK_TEXT_NOT_MEANINGFUL,
    /* [13.1.1.2] */       LINK_TEXT_MISSING,
    /* [13.1.1.3] */       LINK_TEXT_TOO_LONG,
    /* [13.1.1.4] */       LINK_TEXT_NOT_MEANINGFUL_CLICK_HERE,
    /* [13.1.1.5] */       LINK_TEXT_NOT_MEANINGFUL_MORE,
    /* [13.1.1.6] */       LINK_TEXT_NOT_MEANINGFUL_FOLLOW_THIS,
    /* [13.2.1.1] */       METADATA_MISSING,
    /* [13.2.1.2] */       METADATA_MISSING_LINK,
    /* [13.2.1.3] */       METADATA_MISSING_REDIRECT_AUTOREFRESH,
    /* [13.10.1.1] */      SKIPOVER_ASCII_ART,
    
    LAST_ACCESS_ERR    /* must be last */
};


void TY_(AccessibilityHelloMessage)( TidyDocImpl* doc );
void TY_(DisplayHTMLTableAlgorithm)( TidyDocImpl* doc );

/************************************************************
* AccessibilityChecks
*
* Traverses through the individual nodes of the tree
* and checks attributes and elements for accessibility.
* after the tree structure has been formed.
************************************************************/

void TY_(AccessibilityChecks)( TidyDocImpl* doc );


#endif /* SUPPORT_ACCESSIBILITY_CHECKS */
#endif /* __ACCESS_H__ */
