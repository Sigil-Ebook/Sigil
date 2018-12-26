/* C code produced by gperf version 3.0.3 */
/* Command-line: /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/gperf -m100 svg_tags.gperf  */
/* Computed positions: -k'3,7' */

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

#line 1 "svg_tags.gperf"

#include "replacement.h"
#include <string.h>

#define TOTAL_KEYWORDS 36
#define MIN_WORD_LENGTH 6
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 6
#define MAX_HASH_VALUE 42
/* maximum key range = 37, duplicates = 0 */

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
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 12,  2, 10, 22,
       1, 28, 15,  1, 43, 43, 43,  0,  9, 26,
       3, 17,  1, 11,  0, 22,  5, 43,  3,  2,
      43, 43, 43, 43, 43, 43, 43, 43, 12,  2,
      10, 22,  1, 28, 15,  1, 43, 43, 43,  0,
       9, 26,  3, 17,  1, 11,  0, 22,  5, 43,
       3,  2, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
      43, 43, 43, 43, 43, 43, 43
    };
  register unsigned int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[6]+1];
      /*FALLTHROUGH*/
      case 6:
      case 5:
      case 4:
      case 3:
        hval += asso_values[(unsigned char)str[2]];
        break;
    }
  return hval;
}

static const unsigned char lengthtable[] =
  {
     0,  0,  0,  0,  0,  0,  6,  0,  7,  7,  7,  8, 11, 12,
    12, 13, 11, 12, 16,  7,  7, 16, 11,  7, 19,  8, 13, 17,
    11, 12,  7,  8, 17,  8, 18,  8, 14, 12, 14, 14, 13,  7,
    14
  };

static const StringReplacement wordlist[] =
  {
    {(char*)0}, {(char*)0}, {(char*)0}, {(char*)0},
    {(char*)0}, {(char*)0},
#line 47 "svg_tags.gperf"
    {"fetile", "feTile"},
    {(char*)0},
#line 40 "svg_tags.gperf"
    {"femerge", "feMerge"},
#line 39 "svg_tags.gperf"
    {"feimage", "feImage"},
#line 35 "svg_tags.gperf"
    {"fefuncb", "feFuncB"},
#line 50 "svg_tags.gperf"
    {"glyphref", "glyphRef"},
#line 41 "svg_tags.gperf"
    {"femergenode", "feMergeNode"},
#line 42 "svg_tags.gperf"
    {"femorphology", "feMorphology"},
#line 21 "svg_tags.gperf"
    {"animatecolor", "animateColor"},
#line 22 "svg_tags.gperf"
    {"animatemotion", "animateMotion"},
#line 28 "svg_tags.gperf"
    {"fecomposite", "feComposite"},
#line 48 "svg_tags.gperf"
    {"feturbulence", "feTurbulence"},
#line 23 "svg_tags.gperf"
    {"animatetransform", "animateTransform"},
#line 37 "svg_tags.gperf"
    {"fefuncr", "feFuncR"},
#line 34 "svg_tags.gperf"
    {"fefunca", "feFuncA"},
#line 29 "svg_tags.gperf"
    {"feconvolvematrix", "feConvolveMatrix"},
#line 46 "svg_tags.gperf"
    {"fespotlight", "feSpotLight"},
#line 36 "svg_tags.gperf"
    {"fefuncg", "feFuncG"},
#line 27 "svg_tags.gperf"
    {"fecomponenttransfer", "feComponentTransfer"},
#line 18 "svg_tags.gperf"
    {"altglyph", "altGlyph"},
#line 26 "svg_tags.gperf"
    {"fecolormatrix", "feColorMatrix"},
#line 31 "svg_tags.gperf"
    {"fedisplacementmap", "feDisplacementMap"},
#line 19 "svg_tags.gperf"
    {"altglyphdef", "altGlyphDef"},
#line 20 "svg_tags.gperf"
    {"altglyphitem", "altGlyphItem"},
#line 33 "svg_tags.gperf"
    {"feflood", "feFlood"},
#line 24 "svg_tags.gperf"
    {"clippath", "clipPath"},
#line 30 "svg_tags.gperf"
    {"fediffuselighting", "feDiffuseLighting"},
#line 53 "svg_tags.gperf"
    {"textpath", "textPath"},
#line 45 "svg_tags.gperf"
    {"fespecularlighting", "feSpecularLighting"},
#line 43 "svg_tags.gperf"
    {"feoffset", "feOffset"},
#line 32 "svg_tags.gperf"
    {"fedistantlight", "feDistantLight"},
#line 44 "svg_tags.gperf"
    {"fepointlight", "fePointLight"},
#line 51 "svg_tags.gperf"
    {"lineargradient", "linearGradient"},
#line 52 "svg_tags.gperf"
    {"radialgradient", "radialGradient"},
#line 49 "svg_tags.gperf"
    {"foreignobject", "foreignObject"},
#line 25 "svg_tags.gperf"
    {"feblend", "feBlend"},
#line 38 "svg_tags.gperf"
    {"fegaussianblur", "feGaussianBlur"}
  };

const StringReplacement *
gumbo_get_svg_tag_replacement (register const char *str, register size_t len)
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
