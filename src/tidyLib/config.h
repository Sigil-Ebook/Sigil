#ifndef __CONFIG_H__
#define __CONFIG_H__

/* config.h -- read config file and manage config properties
  
  (c) 1998-2006 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  CVS Info :

    $Author: arnaud02 $ 
    $Date: 2006/12/29 16:31:08 $ 
    $Revision: 1.14 $ 

  config files associate a property name with a value.

  // comments can start at the beginning of a line
  # comments can start at the beginning of a line
  name: short values fit onto one line
  name: a really long value that
   continues on the next line

  property names are case insensitive and should be less than
  60 characters in length and must start at the begining of
  the line, as whitespace at the start of a line signifies a
  line continuation.

*/

#include "forward.h"
#include "tidy.h"
#include "streamio.h"

struct _tidy_option;
typedef struct _tidy_option TidyOptionImpl;

typedef Bool (ParseProperty)( TidyDocImpl* doc, const TidyOptionImpl* opt );

struct _tidy_option
{
    TidyOptionId        id;
    TidyConfigCategory  category;   /* put 'em in groups */
    ctmbstr             name;       /* property name */
    TidyOptionType      type;       /* string, int or bool */
    ulong               dflt;       /* default for TidyInteger and TidyBoolean */
    ParseProperty*      parser;     /* parsing method, read-only if NULL */
    const ctmbstr*      pickList;   /* pick list */
    ctmbstr             pdflt;      /* default for TidyString */
};

typedef union
{
  ulong v;  /* Value for TidyInteger and TidyBoolean */
  char *p;  /* Value for TidyString */
} TidyOptionValue;

typedef struct _tidy_config
{
    TidyOptionValue value[ N_TIDY_OPTIONS + 1 ];     /* current config values */
    TidyOptionValue snapshot[ N_TIDY_OPTIONS + 1 ];  /* Snapshot of values to be restored later */

    /* track what tags user has defined to eliminate unnecessary searches */
    uint  defined_tags;

    uint c;           /* current char in input stream */
    StreamIn* cfgIn;  /* current input source */

} TidyConfigImpl;


typedef struct {
  TidyOptionId opt;          /**< Identifier. */
  ctmbstr doc;               /**< HTML text */
  TidyOptionId const *links; /**< Cross references.
                             Last element must be 'TidyUnknownOption'. */
} TidyOptionDoc;


const TidyOptionImpl* TY_(lookupOption)( ctmbstr optnam );
const TidyOptionImpl* TY_(getOption)( TidyOptionId optId );

TidyIterator TY_(getOptionList)( TidyDocImpl* doc );
const TidyOptionImpl* TY_(getNextOption)( TidyDocImpl* doc, TidyIterator* iter );

TidyIterator TY_(getOptionPickList)( const TidyOptionImpl* option );
ctmbstr TY_(getNextOptionPick)( const TidyOptionImpl* option, TidyIterator* iter );

const TidyOptionDoc* TY_(OptGetDocDesc)( TidyOptionId optId );

void TY_(InitConfig)( TidyDocImpl* doc );
void TY_(FreeConfig)( TidyDocImpl* doc );

/* Bool SetOptionValue( TidyDocImpl* doc, TidyOptionId optId, ctmbstr val ); */
Bool TY_(SetOptionInt)( TidyDocImpl* doc, TidyOptionId optId, ulong val );
Bool TY_(SetOptionBool)( TidyDocImpl* doc, TidyOptionId optId, Bool val );

Bool TY_(ResetOptionToDefault)( TidyDocImpl* doc, TidyOptionId optId );
void TY_(ResetConfigToDefault)( TidyDocImpl* doc );
void TY_(TakeConfigSnapshot)( TidyDocImpl* doc );
void TY_(ResetConfigToSnapshot)( TidyDocImpl* doc );

void TY_(CopyConfig)( TidyDocImpl* docTo, TidyDocImpl* docFrom );

int  TY_(ParseConfigFile)( TidyDocImpl* doc, ctmbstr cfgfil );
int  TY_(ParseConfigFileEnc)( TidyDocImpl* doc,
                              ctmbstr cfgfil, ctmbstr charenc );

int  TY_(SaveConfigFile)( TidyDocImpl* doc, ctmbstr cfgfil );
int  TY_(SaveConfigSink)( TidyDocImpl* doc, TidyOutputSink* sink );

/* returns false if unknown option, missing parameter, or
   option doesn't use parameter
*/
Bool  TY_(ParseConfigOption)( TidyDocImpl* doc, ctmbstr optnam, ctmbstr optVal );
Bool  TY_(ParseConfigValue)( TidyDocImpl* doc, TidyOptionId optId, ctmbstr optVal );

/* ensure that char encodings are self consistent */
Bool  TY_(AdjustCharEncoding)( TidyDocImpl* doc, int encoding );

Bool  TY_(ConfigDiffThanDefault)( TidyDocImpl* doc );
Bool  TY_(ConfigDiffThanSnapshot)( TidyDocImpl* doc );

int TY_(CharEncodingId)( TidyDocImpl* doc, ctmbstr charenc );
ctmbstr TY_(CharEncodingName)( int encoding );
ctmbstr TY_(CharEncodingOptName)( int encoding );

/* void SetEmacsFilename( TidyDocImpl* doc, ctmbstr filename ); */


#ifdef _DEBUG

/* Debug lookup functions will be type-safe and assert option type match */
ulong   TY_(_cfgGet)( TidyDocImpl* doc, TidyOptionId optId );
Bool    TY_(_cfgGetBool)( TidyDocImpl* doc, TidyOptionId optId );
TidyTriState TY_(_cfgGetAutoBool)( TidyDocImpl* doc, TidyOptionId optId );
ctmbstr TY_(_cfgGetString)( TidyDocImpl* doc, TidyOptionId optId );

#define cfg(doc, id)            TY_(_cfgGet)( (doc), (id) )
#define cfgBool(doc, id)        TY_(_cfgGetBool)( (doc), (id) )
#define cfgAutoBool(doc, id)    TY_(_cfgGetAutoBool)( (doc), (id) )
#define cfgStr(doc, id)         TY_(_cfgGetString)( (doc), (id) )

#else

/* Release build macros for speed */
#define cfg(doc, id)            ((doc)->config.value[ (id) ].v)
#define cfgBool(doc, id)        ((Bool) cfg(doc, id))
#define cfgAutoBool(doc, id)    ((TidyTriState) cfg(doc, id))
#define cfgStr(doc, id)         ((ctmbstr) (doc)->config.value[ (id) ].p)

#endif /* _DEBUG */

#endif /* __CONFIG_H__ */
