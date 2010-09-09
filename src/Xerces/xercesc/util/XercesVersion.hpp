/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id: XercesVersion.hpp 935350 2010-04-18 15:02:36Z borisk $
 */

#if !defined(XERCESC_INCLUDE_GUARD_XERCESVERSION_HPP)
#define XERCESC_INCLUDE_GUARD_XERCESVERSION_HPP

// ---------------------------------------------------------------------------
// X E R C E S   V E R S I O N   H E A D E R   D O C U M E N T A T I O N

/**
 * User Documentation for Xerces Version Values:
 *
 *
 *
 * Xerces  Notes:
 *
 * Xerces Committers Documentation:
 *
 *    Xerces committers normally only need to modify one or two of the
 *    following macros:
 *
 *      XERCES_VERSION_MAJOR
 *      XERCES_VERSION_MINOR
 *      XERCES_VERSION_REVISION
 *
 *    The integer values of these macros define the Xerces version number. All
 *    other constants and preprocessor macros are automatically generated from
 *    these three definitions.
 *
 *    The macro XERCES_GRAMMAR_SERIALIZATION_LEVEL has been added so that during
 *    development if users are using the latest code they can use the grammar
 *    serialization/deserialization features.  Whenever a change is made to the
 *    serialization code this macro should be incremented.
 *
 * Xerces User Documentation:
 *
 *  The following sections in the user documentation have examples based upon
 *  the following three version input values:
 *
 *    #define XERCES_VERSION_MAJOR 19
 *    #define XERCES_VERSION_MINOR 3
 *    #define XERCES_VERSION_REVISION 74
 *
 *  The minor and revision (patch level) numbers have two digits of resolution
 *  which means that '3' becomes '03' in this example. This policy guarantees
 *  that when using preprocessor macros, version 19.3.74 will be greater than
 *  version 1.94.74 since the first will expand to 190374 and the second to
 *  19474.
 *
 *  Preprocessor Macros:
 *
 *    _XERCES_VERSION defines the primary preprocessor macro that users will
 *    introduce into their code to perform conditional compilation where the
 *    version of Xerces is detected in order to enable or disable version
 *    specific capabilities. The value of _XERCES_VERSION for the above example
 *    will be 190374. To use it a user would perform an operation such as the
 *    following:
 *
 *      #if _XERCES_VERSION >= 190374
 *        // code specific to new version of Xerces...
 *      #else
 *        // old code here...
 *      #endif
 *
 *    XERCES_FULLVERSIONSTR is a preprocessor macro that expands to a string
 *    constant whose value, for the above example, will be "19_3_74".
 *
 *    XERCES_FULLVERSIONDOT is a preprocessor macro that expands to a string
 *    constant whose value, for the above example, will be "19.3.74".
 *
 *    XERCES_VERSIONSTR is a preprocessor macro that expands to a string
 *    constant whose value, for the above example, will be "19_3". This
 *    particular macro is very dangerous if it were to be used for comparing
 *    version numbers since ordering will not be guaranteed.
 *
 *    Xerces_DLLVersionStr is a preprocessor macro that expands to a string
 *    constant whose value, for the above example, will be "19_3_74". This
 *    macro is provided for backwards compatibility to pre-1.7 versions of
 *    Xerces.
 *
 *  String Constants:
 *
 *    gXercesVersionStr is a global string constant whose value corresponds to
 *    the value "19_3" for the above example.
 *
 *    gXercesFullVersionStr is a global string constant whose value corresponds
 *    to the value "19_3_74" for the above example.
 *
 *  Numeric Constants:
 *
 *    gXercesMajVersion is a global integer constant whose value corresponds to
 *    the major version number. For the above example its value will be 19.
 *
 *    gXercesMinVersion is a global integer constant whose value corresponds to
 *    the minor version number. For the above example its value will be 3.
 *
 *    gXercesRevision is a global integer constant whose value corresponds to
 *    the revision (patch) version number. For the above example its value will
 *    be 74.
 *
 */

// ---------------------------------------------------------------------------
// X E R C E S   V E R S I O N   S P E C I F I C A T I O N

/**
 * MODIFY THESE NUMERIC VALUES TO COINCIDE WITH XERCES VERSION
 * AND DO NOT MODIFY ANYTHING ELSE IN THIS VERSION HEADER FILE
 */

#define XERCES_VERSION_MAJOR 3
#define XERCES_VERSION_MINOR 1
#define XERCES_VERSION_REVISION 1

/***
 *
 * XERCES_GRAMMAR_SERIALIZATION_LEVEL = 4   SchemaAttDef, SchemaElementDecl serialize fPSVIScope
 * XERCES_GRAMMAR_SERIALIZATION_LEVEL = 5   XercesStep serializes the axis as an int
 * XERCES_GRAMMAR_SERIALIZATION_LEVEL = 6   added fIsExternal to XMLEntityDecl
 * XERCES_GRAMMAR_SERIALIZATION_LEVEL = 7   size of line/column fields has changed
 *
 ***/
#define XERCES_GRAMMAR_SERIALIZATION_LEVEL 7

/** DO NOT MODIFY BELOW THIS LINE */

/**
 * MAGIC THAT AUTOMATICALLY GENERATES THE FOLLOWING:
 *
 *	Xerces_DLLVersionStr, gXercesVersionStr, gXercesFullVersionStr,
 *	gXercesMajVersion, gXercesMinVersion, gXercesRevision
 */

// ---------------------------------------------------------------------------
// T W O   A R G U M E N T   C O N C A T E N A T I O N   M A C R O S

// two argument concatenation routines
#define CAT2_SEP_UNDERSCORE(a, b) #a "_" #b
#define CAT2_SEP_PERIOD(a, b) #a "." #b
#define CAT2_SEP_NIL(a, b) #a #b
#define CAT2_RAW_NUMERIC(a, b) a ## b

// two argument macro invokers
#define INVK_CAT2_SEP_UNDERSCORE(a,b) CAT2_SEP_UNDERSCORE(a,b)
#define INVK_CAT2_SEP_PERIOD(a,b)     CAT2_SEP_PERIOD(a,b)
#define INVK_CAT2_STR_SEP_NIL(a,b)    CAT2_SEP_NIL(a,b)
#define INVK_CAT2_RAW_NUMERIC(a,b)    CAT2_RAW_NUMERIC(a,b)

// ---------------------------------------------------------------------------
// T H R E E   A R G U M E N T   C O N C A T E N A T I O N   M A C R O S

// three argument concatenation routines
#define CAT3_SEP_UNDERSCORE(a, b, c) #a "_" #b "_" #c
#define CAT3_SEP_PERIOD(a, b, c) #a "." #b "." #c
#define CAT3_SEP_NIL(a, b, c) #a #b #c
#define CAT3_RAW_NUMERIC(a, b, c) a ## b ## c
#define CAT3_RAW_NUMERIC_SEP_UNDERSCORE(a, b, c) a ## _ ## b ## _ ## c

// three argument macro invokers
#define INVK_CAT3_SEP_UNDERSCORE(a,b,c) CAT3_SEP_UNDERSCORE(a,b,c)
#define INVK_CAT3_SEP_PERIOD(a,b,c)     CAT3_SEP_PERIOD(a,b,c)
#define INVK_CAT3_SEP_NIL(a,b,c)        CAT3_SEP_NIL(a,b,c)
#define INVK_CAT3_RAW_NUMERIC(a,b,c)    CAT3_RAW_NUMERIC(a,b,c)
#define INVK_CAT3_RAW_NUMERIC_SEP_UNDERSCORE(a,b,c)    CAT3_RAW_NUMERIC_SEP_UNDERSCORE(a,b,c)

// ---------------------------------------------------------------------------
// C A L C U L A T E   V E R S I O N   -   E X P A N D E D   F O R M

#define MULTIPLY(factor,value) factor * value
#define CALC_EXPANDED_FORM(a,b,c) ( MULTIPLY(10000,a) + MULTIPLY(100,b) + MULTIPLY(1,c) )

// ---------------------------------------------------------------------------
// X E R C E S   V E R S I O N   I N F O R M A T I O N

// Xerces version strings; these particular macros cannot be used for
// conditional compilation as they are not numeric constants

#define XERCES_FULLVERSIONSTR INVK_CAT3_SEP_UNDERSCORE(XERCES_VERSION_MAJOR,XERCES_VERSION_MINOR,XERCES_VERSION_REVISION)
#define XERCES_FULLVERSIONDOT INVK_CAT3_SEP_PERIOD(XERCES_VERSION_MAJOR,XERCES_VERSION_MINOR,XERCES_VERSION_REVISION)
#define XERCES_FULLVERSIONNUM INVK_CAT3_SEP_NIL(XERCES_VERSION_MAJOR,XERCES_VERSION_MINOR,XERCES_VERSION_REVISION)
#define XERCES_VERSIONSTR     INVK_CAT2_SEP_UNDERSCORE(XERCES_VERSION_MAJOR,XERCES_VERSION_MINOR)

// Xerces C++ Namespace string, concatenated with full version string
#define XERCES_PRODUCT xercesc
#define XERCES_CPP_NAMESPACE INVK_CAT3_RAW_NUMERIC_SEP_UNDERSCORE(XERCES_PRODUCT,XERCES_VERSION_MAJOR,XERCES_VERSION_MINOR)

// original from Xerces header
#define Xerces_DLLVersionStr XERCES_FULLVERSIONSTR

const char* const    gXercesVersionStr = XERCES_VERSIONSTR;
const char* const    gXercesFullVersionStr = XERCES_FULLVERSIONSTR;
const unsigned int   gXercesMajVersion = XERCES_VERSION_MAJOR;
const unsigned int   gXercesMinVersion = XERCES_VERSION_MINOR;
const unsigned int   gXercesRevision   = XERCES_VERSION_REVISION;

// Xerces version numeric constants that can be used for conditional
// compilation purposes.

#define _XERCES_VERSION CALC_EXPANDED_FORM (XERCES_VERSION_MAJOR,XERCES_VERSION_MINOR,XERCES_VERSION_REVISION)

#endif // XERCESVERSION_HPP
