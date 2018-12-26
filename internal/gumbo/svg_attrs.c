/* C code produced by gperf version 3.0.3 */
/* Command-line: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/gperf -m100 svg_attrs.gperf  */
/* Computed positions: -k'1,10,$' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 2 "svg_attrs.gperf"

#include "replacement.h"
#include <string.h>

#define TOTAL_KEYWORDS 58
#define MIN_WORD_LENGTH 4
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 5
#define MAX_HASH_VALUE 77
/* maximum key range = 73, duplicates = 0 */

#ifndef GPERF_DOWNCASE
#define GPERF_DOWNCASE 1
static unsigned char gperf_downcase[256] =
  {
      0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
     15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
     30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,
     45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
     60,  61,  62,  63,  64,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106,
    107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
    122,  91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103, 104,
    105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
    120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134,
    135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
    150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
    165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
    180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194,
    195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
    210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
    225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
    240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254,
    255
  };
#endif

#ifndef GPERF_CASE_MEMCMP
#define GPERF_CASE_MEMCMP 1
static int
gperf_case_memcmp (s1, s2, n)
     register const char *s1;
     register const char *s2;
     register unsigned int n;
{
  for (; n > 0;)
    {
      unsigned char c1 = gperf_downcase[(unsigned char)*s1++];
      unsigned char c2 = gperf_downcase[(unsigned char)*s2++];
      if (c1 == c2)
        {
          n--;
          continue;
        }
      return (int)c1 - (int)c2;
    }
  return 0;
}
#endif

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
hash (str, len)
     register const char *str;
     register unsigned int len;
{
  static const unsigned char asso_values[] =
    {
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78,  5, 78, 39, 14,  1,
      31, 31, 13, 13, 78, 78, 22, 25, 10,  2,
       7, 78, 22,  0,  1,  3,  1, 78,  0, 36,
      14, 17, 20, 78, 78, 78, 78,  5, 78, 39,
      14,  1, 31, 31, 13, 13, 78, 78, 22, 25,
      10,  2,  7, 78, 22,  0,  1,  3,  1, 78,
       0, 36, 14, 17, 20, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78, 78, 78,
      78, 78, 78, 78, 78, 78, 78, 78
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[9]];
      /*FALLTHROUGH*/
      case 9:
      case 8:
      case 7:
      case 6:
      case 5:
      case 4:
      case 3:
      case 2:
      case 1:
        hval += asso_values[(unsigned char)str[0]+2];
        break;
    }
  return hval + asso_values[(unsigned char)str[len - 1]];
}

static const unsigned char lengthtable[] =
  {
     0,  0,  0,  0,  0,  4,  0,  7,  7,  0,  8,  9, 10, 11,
    11, 11, 11, 10, 16, 18, 16, 12, 16, 11, 13, 11, 12, 11,
    16,  0, 17,  9,  9,  8,  9, 10, 13, 10, 12, 14,  8,  4,
    12, 19,  7,  9, 12, 12, 11, 14, 10, 19,  8, 16, 13, 16,
    16, 15, 10, 12,  0,  0, 13, 13, 13,  0,  0,  9, 16,  0,
     0,  0,  0,  0,  0,  0,  0, 17
  };

static const StringReplacement wordlist[] =
  {
    {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
    {(char*)0},
#line 58 "svg_attrs.gperf"
    {"refx", "refX"},
    {(char*)0},
#line 76 "svg_attrs.gperf"
    {"viewbox", "viewBox"},
#line 73 "svg_attrs.gperf"
    {"targetx", "targetX"},
    {(char*)0},
#line 27 "svg_attrs.gperf"
    {"calcmode", "calcMode"},
#line 46 "svg_attrs.gperf"
    {"maskunits", "maskUnits"},
#line 77 "svg_attrs.gperf"
    {"viewtarget", "viewTarget"},
#line 72 "svg_attrs.gperf"
    {"tablevalues", "tableValues"},
#line 43 "svg_attrs.gperf"
    {"markerunits", "markerUnits"},
#line 69 "svg_attrs.gperf"
    {"stitchtiles", "stitchTiles"},
#line 67 "svg_attrs.gperf"
    {"startoffset", "startOffset"},
#line 47 "svg_attrs.gperf"
    {"numoctaves", "numOctaves"},
#line 63 "svg_attrs.gperf"
    {"requiredfeatures", "requiredFeatures"},
#line 62 "svg_attrs.gperf"
    {"requiredextensions", "requiredExtensions"},
#line 65 "svg_attrs.gperf"
    {"specularexponent", "specularExponent"},
#line 70 "svg_attrs.gperf"
    {"surfacescale", "surfaceScale"},
#line 64 "svg_attrs.gperf"
    {"specularconstant", "specularConstant"},
#line 60 "svg_attrs.gperf"
    {"repeatcount", "repeatCount"},
#line 28 "svg_attrs.gperf"
    {"clippathunits", "clipPathUnits"},
#line 31 "svg_attrs.gperf"
    {"filterunits", "filterUnits"},
#line 40 "svg_attrs.gperf"
    {"lengthadjust", "lengthAdjust"},
#line 44 "svg_attrs.gperf"
    {"markerwidth", "markerWidth"},
#line 45 "svg_attrs.gperf"
    {"maskcontentunits", "maskContentUnits"},
    {(char*)0},
#line 41 "svg_attrs.gperf"
    {"limitingconeangle", "limitingConeAngle"},
#line 52 "svg_attrs.gperf"
    {"pointsatx", "pointsAtX"},
#line 61 "svg_attrs.gperf"
    {"repeatdur", "repeatDur"},
#line 39 "svg_attrs.gperf"
    {"keytimes", "keyTimes"},
#line 37 "svg_attrs.gperf"
    {"keypoints", "keyPoints"},
#line 38 "svg_attrs.gperf"
    {"keysplines", "keySplines"},
#line 34 "svg_attrs.gperf"
    {"gradientunits", "gradientUnits"},
#line 75 "svg_attrs.gperf"
    {"textlength", "textLength"},
#line 68 "svg_attrs.gperf"
    {"stddeviation", "stdDeviation"},
#line 57 "svg_attrs.gperf"
    {"primitiveunits", "primitiveUnits"},
#line 30 "svg_attrs.gperf"
    {"edgemode", "edgeMode"},
#line 59 "svg_attrs.gperf"
    {"refy", "refY"},
#line 66 "svg_attrs.gperf"
    {"spreadmethod", "spreadMethod"},
#line 56 "svg_attrs.gperf"
    {"preserveaspectratio", "preserveAspectRatio"},
#line 74 "svg_attrs.gperf"
    {"targety", "targetY"},
#line 54 "svg_attrs.gperf"
    {"pointsatz", "pointsAtZ"},
#line 42 "svg_attrs.gperf"
    {"markerheight", "markerHeight"},
#line 51 "svg_attrs.gperf"
    {"patternunits", "patternUnits"},
#line 26 "svg_attrs.gperf"
    {"baseprofile", "baseProfile"},
#line 71 "svg_attrs.gperf"
    {"systemlanguage", "systemLanguage"},
#line 80 "svg_attrs.gperf"
    {"zoomandpan", "zoomAndPan"},
#line 49 "svg_attrs.gperf"
    {"patterncontentunits", "patternContentUnits"},
#line 32 "svg_attrs.gperf"
    {"glyphref", "glyphRef"},
#line 78 "svg_attrs.gperf"
    {"xchannelselector", "xChannelSelector"},
#line 24 "svg_attrs.gperf"
    {"attributetype", "attributeType"},
#line 36 "svg_attrs.gperf"
    {"kernelunitlength", "kernelUnitLength"},
#line 79 "svg_attrs.gperf"
    {"ychannelselector", "yChannelSelector"},
#line 29 "svg_attrs.gperf"
    {"diffuseconstant", "diffuseConstant"},
#line 48 "svg_attrs.gperf"
    {"pathlength", "pathLength"},
#line 35 "svg_attrs.gperf"
    {"kernelmatrix", "kernelMatrix"},
    {(char*)0}, {(char*)0},
#line 55 "svg_attrs.gperf"
    {"preservealpha", "preserveAlpha"},
#line 23 "svg_attrs.gperf"
    {"attributename", "attributeName"},
#line 25 "svg_attrs.gperf"
    {"basefrequency", "baseFrequency"},
    {(char*)0}, {(char*)0},
#line 53 "svg_attrs.gperf"
    {"pointsaty", "pointsAtY"},
#line 50 "svg_attrs.gperf"
    {"patterntransform", "patternTransform"},
    {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
    {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
#line 33 "svg_attrs.gperf"
    {"gradienttransform", "gradientTransform"}
  };

const StringReplacement *
gumbo_get_svg_attr_replacement (register const char* str, register size_t len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      unsigned int key = hash (str, len);

      if (key <= MAX_HASH_VALUE)
        if (len == lengthtable[key])
          {
            register const char *s = wordlist[key].from;

            if (s && (((unsigned char)*str ^ (unsigned char)*s) & ~32) == 0 && !gperf_case_memcmp (str, s, len))
              return &wordlist[key];
          }
    }
  return 0;
}
