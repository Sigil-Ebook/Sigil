/* win32tc.c -- Interface to Win32 transcoding routines

  (c) 1998-2008 (W3C) MIT, ERCIM, Keio University
  See tidy.h for the copyright notice.

  $Id: win32tc.c,v 1.12 2008/08/09 11:55:27 hoehrmann Exp $
*/

/* keep these here to keep file non-empty */
#include "tidy.h"
#include "forward.h"
#include "streamio.h"
#include "tmbstr.h"
#include "utf8.h"

#ifdef TIDY_WIN32_MLANG_SUPPORT

#define VC_EXTRALEAN
#define CINTERFACE
#define COBJMACROS

#include <windows.h>
#include <mlang.h>

#undef COBJMACROS
#undef CINTERFACE
#undef VC_EXTRALEAN

/* maximum number of bytes for a single character */
#define TC_INBUFSIZE  16

/* maximum number of characters per byte sequence */
#define TC_OUTBUFSIZE 16

#define CreateMLangObject(p) \
  CoCreateInstance( \
        &CLSID_CMLangConvertCharset, \
        NULL, \
        CLSCTX_ALL, \
        &IID_IMLangConvertCharset, \
        (VOID **)&p);


/* Character Set to Microsoft Windows Codepage Identifier map,     */
/* from <rotor/sscli/clr/src/classlibnative/nls/encodingdata.cpp>. */

/* note: the 'safe' field indicates whether this encoding can be   */
/* read/written character-by-character; this does not apply to     */
/* various stateful encodings such as ISO-2022 or UTF-7, these     */
/* must be read/written as a complete stream. It is possible that  */
/* some 'unsafe' encodings are marked as 'save'.                   */

/* todo: cleanup; Tidy should use only a single mapping table to   */
/* circumvent unsupported aliases in other transcoding libraries,  */
/* enable reverse lookup of encoding names and ease maintenance.   */

static struct _nameWinCPMap
{
    tmbstr name;
    uint wincp;
    Bool safe;
} const NameWinCPMap[] = {
  { "cp037",                                            37, yes },
  { "csibm037",                                         37, yes },
  { "ebcdic-cp-ca",                                     37, yes },
  { "ebcdic-cp-nl",                                     37, yes },
  { "ebcdic-cp-us",                                     37, yes },
  { "ebcdic-cp-wt",                                     37, yes },
  { "ibm037",                                           37, yes },
  { "cp437",                                           437, yes },
  { "cspc8codepage437",                                437, yes },
  { "ibm437",                                          437, yes },
  { "cp500",                                           500, yes },
  { "csibm500",                                        500, yes },
  { "ebcdic-cp-be",                                    500, yes },
  { "ebcdic-cp-ch",                                    500, yes },
  { "ibm500",                                          500, yes },
  { "asmo-708",                                        708, yes },
  { "dos-720",                                         720, yes },
  { "ibm737",                                          737, yes },
  { "ibm775",                                          775, yes },
  { "cp850",                                           850, yes },
  { "ibm850",                                          850, yes },
  { "cp852",                                           852, yes },
  { "ibm852",                                          852, yes },
  { "cp855",                                           855, yes },
  { "ibm855",                                          855, yes },
  { "cp857",                                           857, yes },
  { "ibm857",                                          857, yes },
  { "ccsid00858",                                      858, yes },
  { "cp00858",                                         858, yes },
  { "cp858",                                           858, yes },
  { "ibm00858",                                        858, yes },
  { "pc-multilingual-850+euro",                        858, yes },
  { "cp860",                                           860, yes },
  { "ibm860",                                          860, yes },
  { "cp861",                                           861, yes },
  { "ibm861",                                          861, yes },
  { "cp862",                                           862, yes },
  { "dos-862",                                         862, yes },
  { "ibm862",                                          862, yes },
  { "cp863",                                           863, yes },
  { "ibm863",                                          863, yes },
  { "cp864",                                           864, yes },
  { "ibm864",                                          864, yes },
  { "cp865",                                           865, yes },
  { "ibm865",                                          865, yes },
  { "cp866",                                           866, yes },
  { "ibm866",                                          866, yes },
  { "cp869",                                           869, yes },
  { "ibm869",                                          869, yes },
  { "cp870",                                           870, yes },
  { "csibm870",                                        870, yes },
  { "ebcdic-cp-roece",                                 870, yes },
  { "ebcdic-cp-yu",                                    870, yes },
  { "ibm870",                                          870, yes },
  { "dos-874",                                         874, yes },
  { "iso-8859-11",                                     874, yes },
  { "tis-620",                                         874, yes },
  { "windows-874",                                     874, yes },
  { "cp875",                                           875, yes },
  { "csshiftjis",                                      932, yes },
  { "cswindows31j",                                    932, yes },
  { "ms_kanji",                                        932, yes },
  { "shift-jis",                                       932, yes },
  { "shift_jis",                                       932, yes },
  { "sjis",                                            932, yes },
  { "x-ms-cp932",                                      932, yes },
  { "x-sjis",                                          932, yes },
  { "chinese",                                         936, yes },
  { "cn-gb",                                           936, yes },
  { "csgb2312",                                        936, yes },
  { "csgb231280",                                      936, yes },
  { "csiso58gb231280",                                 936, yes },
  { "gb2312",                                          936, yes },
  { "gb2312-80",                                       936, yes },
  { "gb231280",                                        936, yes },
  { "gb_2312-80",                                      936, yes },
  { "gbk",                                             936, yes },
  { "iso-ir-58",                                       936, yes },
  { "csksc56011987",                                   949, yes },
  { "iso-ir-149",                                      949, yes },
  { "korean",                                          949, yes },
  { "ks-c-5601",                                       949, yes },
  { "ks-c5601",                                        949, yes },
  { "ks_c_5601",                                       949, yes },
  { "ks_c_5601-1987",                                  949, yes },
  { "ks_c_5601-1989",                                  949, yes },
  { "ks_c_5601_1987",                                  949, yes },
  { "ksc5601",                                         949, yes },
  { "ksc_5601",                                        949, yes },
  { "big5",                                            950, yes },
  { "big5-hkscs",                                      950, yes },
  { "cn-big5",                                         950, yes },
  { "csbig5",                                          950, yes },
  { "x-x-big5",                                        950, yes },
  { "cp1026",                                         1026, yes },
  { "csibm1026",                                      1026, yes },
  { "ibm1026",                                        1026, yes },
  { "ibm01047",                                       1047, yes },
  { "ccsid01140",                                     1140, yes },
  { "cp01140",                                        1140, yes },
  { "ebcdic-us-37+euro",                              1140, yes },
  { "ibm01140",                                       1140, yes },
  { "ccsid01141",                                     1141, yes },
  { "cp01141",                                        1141, yes },
  { "ebcdic-de-273+euro",                             1141, yes },
  { "ibm01141",                                       1141, yes },
  { "ccsid01142",                                     1142, yes },
  { "cp01142",                                        1142, yes },
  { "ebcdic-dk-277+euro",                             1142, yes },
  { "ebcdic-no-277+euro",                             1142, yes },
  { "ibm01142",                                       1142, yes },
  { "ccsid01143",                                     1143, yes },
  { "cp01143",                                        1143, yes },
  { "ebcdic-fi-278+euro",                             1143, yes },
  { "ebcdic-se-278+euro",                             1143, yes },
  { "ibm01143",                                       1143, yes },
  { "ccsid01144",                                     1144, yes },
  { "cp01144",                                        1144, yes },
  { "ebcdic-it-280+euro",                             1144, yes },
  { "ibm01144",                                       1144, yes },
  { "ccsid01145",                                     1145, yes },
  { "cp01145",                                        1145, yes },
  { "ebcdic-es-284+euro",                             1145, yes },
  { "ibm01145",                                       1145, yes },
  { "ccsid01146",                                     1146, yes },
  { "cp01146",                                        1146, yes },
  { "ebcdic-gb-285+euro",                             1146, yes },
  { "ibm01146",                                       1146, yes },
  { "ccsid01147",                                     1147, yes },
  { "cp01147",                                        1147, yes },
  { "ebcdic-fr-297+euro",                             1147, yes },
  { "ibm01147",                                       1147, yes },
  { "ccsid01148",                                     1148, yes },
  { "cp01148",                                        1148, yes },
  { "ebcdic-international-500+euro",                  1148, yes },
  { "ibm01148",                                       1148, yes },
  { "ccsid01149",                                     1149, yes },
  { "cp01149",                                        1149, yes },
  { "ebcdic-is-871+euro",                             1149, yes },
  { "ibm01149",                                       1149, yes },
  { "iso-10646-ucs-2",                                1200, yes },
  { "ucs-2",                                          1200, yes },
  { "unicode",                                        1200, yes },
  { "utf-16",                                         1200, yes },
  { "utf-16le",                                       1200, yes },
  { "unicodefffe",                                    1201, yes },
  { "utf-16be",                                       1201, yes },
  { "windows-1250",                                   1250, yes },
  { "x-cp1250",                                       1250, yes },
  { "windows-1251",                                   1251, yes },
  { "x-cp1251",                                       1251, yes },
  { "windows-1252",                                   1252, yes },
  { "x-ansi",                                         1252, yes },
  { "windows-1253",                                   1253, yes },
  { "windows-1254",                                   1254, yes },
  { "windows-1255",                                   1255, yes },
  { "cp1256",                                         1256, yes },
  { "windows-1256",                                   1256, yes },
  { "windows-1257",                                   1257, yes },
  { "windows-1258",                                   1258, yes },
  { "johab",                                          1361, yes },
  { "macintosh",                                     10000, yes },
  { "x-mac-japanese",                                10001, yes },
  { "x-mac-chinesetrad",                             10002, yes },
  { "x-mac-korean",                                  10003, yes },
  { "x-mac-arabic",                                  10004, yes },
  { "x-mac-hebrew",                                  10005, yes },
  { "x-mac-greek",                                   10006, yes },
  { "x-mac-cyrillic",                                10007, yes },
  { "x-mac-chinesesimp",                             10008, yes },
  { "x-mac-romanian",                                10010, yes },
  { "x-mac-ukrainian",                               10017, yes },
  { "x-mac-thai",                                    10021, yes },
  { "x-mac-ce",                                      10029, yes },
  { "x-mac-icelandic",                               10079, yes },
  { "x-mac-turkish",                                 10081, yes },
  { "x-mac-croatian",                                10082, yes },
  { "x-chinese-cns",                                 20000, yes },
  { "x-cp20001",                                     20001, yes },
  { "x-chinese-eten",                                20002, yes },
  { "x-cp20003",                                     20003, yes },
  { "x-cp20004",                                     20004, yes },
  { "x-cp20005",                                     20005, yes },
  { "irv",                                           20105, yes },
  { "x-ia5",                                         20105, yes },
  { "din_66003",                                     20106, yes },
  { "german",                                        20106, yes },
  { "x-ia5-german",                                  20106, yes },
  { "sen_850200_b",                                  20107, yes },
  { "swedish",                                       20107, yes },
  { "x-ia5-swedish",                                 20107, yes },
  { "norwegian",                                     20108, yes },
  { "ns_4551-1",                                     20108, yes },
  { "x-ia5-norwegian",                               20108, yes },
  { "ansi_x3.4-1968",                                20127, yes },
  { "ansi_x3.4-1986",                                20127, yes },
  { "ascii",                                         20127, yes },
  { "cp367",                                         20127, yes },
  { "csascii",                                       20127, yes },
  { "ibm367",                                        20127, yes },
  { "iso-ir-6",                                      20127, yes },
  { "iso646-us",                                     20127, yes },
  { "iso_646.irv:1991",                              20127, yes },
  { "us",                                            20127, yes },
  { "us-ascii",                                      20127, yes },
  { "x-cp20261",                                     20261, yes },
  { "x-cp20269",                                     20269, yes },
  { "cp273",                                         20273, yes },
  { "csibm273",                                      20273, yes },
  { "ibm273",                                        20273, yes },
  { "csibm277",                                      20277, yes },
  { "ebcdic-cp-dk",                                  20277, yes },
  { "ebcdic-cp-no",                                  20277, yes },
  { "ibm277",                                        20277, yes },
  { "cp278",                                         20278, yes },
  { "csibm278",                                      20278, yes },
  { "ebcdic-cp-fi",                                  20278, yes },
  { "ebcdic-cp-se",                                  20278, yes },
  { "ibm278",                                        20278, yes },
  { "cp280",                                         20280, yes },
  { "csibm280",                                      20280, yes },
  { "ebcdic-cp-it",                                  20280, yes },
  { "ibm280",                                        20280, yes },
  { "cp284",                                         20284, yes },
  { "csibm284",                                      20284, yes },
  { "ebcdic-cp-es",                                  20284, yes },
  { "ibm284",                                        20284, yes },
  { "cp285",                                         20285, yes },
  { "csibm285",                                      20285, yes },
  { "ebcdic-cp-gb",                                  20285, yes },
  { "ibm285",                                        20285, yes },
  { "cp290",                                         20290, yes },
  { "csibm290",                                      20290, yes },
  { "ebcdic-jp-kana",                                20290, yes },
  { "ibm290",                                        20290, yes },
  { "cp297",                                         20297, yes },
  { "csibm297",                                      20297, yes },
  { "ebcdic-cp-fr",                                  20297, yes },
  { "ibm297",                                        20297, yes },
  { "cp420",                                         20420, yes },
  { "csibm420",                                      20420, yes },
  { "ebcdic-cp-ar1",                                 20420, yes },
  { "ibm420",                                        20420, yes },
  { "cp423",                                         20423, yes },
  { "csibm423",                                      20423, yes },
  { "ebcdic-cp-gr",                                  20423, yes },
  { "ibm423",                                        20423, yes },
  { "cp424",                                         20424, yes },
  { "csibm424",                                      20424, yes },
  { "ebcdic-cp-he",                                  20424, yes },
  { "ibm424",                                        20424, yes },
  { "x-ebcdic-koreanextended",                       20833, yes },
  { "csibmthai",                                     20838, yes },
  { "ibm-thai",                                      20838, yes },
  { "cskoi8r",                                       20866, yes },
  { "koi",                                           20866, yes },
  { "koi8",                                          20866, yes },
  { "koi8-r",                                        20866, yes },
  { "koi8r",                                         20866, yes },
  { "cp871",                                         20871, yes },
  { "csibm871",                                      20871, yes },
  { "ebcdic-cp-is",                                  20871, yes },
  { "ibm871",                                        20871, yes },
  { "cp880",                                         20880, yes },
  { "csibm880",                                      20880, yes },
  { "ebcdic-cyrillic",                               20880, yes },
  { "ibm880",                                        20880, yes },
  { "cp905",                                         20905, yes },
  { "csibm905",                                      20905, yes },
  { "ebcdic-cp-tr",                                  20905, yes },
  { "ibm905",                                        20905, yes },
  { "ccsid00924",                                    20924, yes },
  { "cp00924",                                       20924, yes },
  { "ebcdic-latin9--euro",                           20924, yes },
  { "ibm00924",                                      20924, yes },
  { "x-cp20936",                                     20936, yes },
  { "x-cp20949",                                     20949, yes },
  { "cp1025",                                        21025, yes },
  { "x-cp21027",                                     21027, yes },
  { "koi8-ru",                                       21866, yes },
  { "koi8-u",                                        21866, yes },
  { "cp819",                                         28591, yes },
  { "csisolatin1",                                   28591, yes },
  { "ibm819",                                        28591, yes },
  { "iso-8859-1",                                    28591, yes },
  { "iso-ir-100",                                    28591, yes },
  { "iso8859-1",                                     28591, yes },
  { "iso_8859-1",                                    28591, yes },
  { "iso_8859-1:1987",                               28591, yes },
  { "l1",                                            28591, yes },
  { "latin1",                                        28591, yes },
  { "csisolatin2",                                   28592, yes },
  { "iso-8859-2",                                    28592, yes },
  { "iso-ir-101",                                    28592, yes },
  { "iso8859-2",                                     28592, yes },
  { "iso_8859-2",                                    28592, yes },
  { "iso_8859-2:1987",                               28592, yes },
  { "l2",                                            28592, yes },
  { "latin2",                                        28592, yes },
  { "csisolatin3",                                   28593, yes },
  { "iso-8859-3",                                    28593, yes },
  { "iso-ir-109",                                    28593, yes },
  { "iso_8859-3",                                    28593, yes },
  { "iso_8859-3:1988",                               28593, yes },
  { "l3",                                            28593, yes },
  { "latin3",                                        28593, yes },
  { "csisolatin4",                                   28594, yes },
  { "iso-8859-4",                                    28594, yes },
  { "iso-ir-110",                                    28594, yes },
  { "iso_8859-4",                                    28594, yes },
  { "iso_8859-4:1988",                               28594, yes },
  { "l4",                                            28594, yes },
  { "latin4",                                        28594, yes },
  { "csisolatincyrillic",                            28595, yes },
  { "cyrillic",                                      28595, yes },
  { "iso-8859-5",                                    28595, yes },
  { "iso-ir-144",                                    28595, yes },
  { "iso_8859-5",                                    28595, yes },
  { "iso_8859-5:1988",                               28595, yes },
  { "arabic",                                        28596, yes },
  { "csisolatinarabic",                              28596, yes },
  { "ecma-114",                                      28596, yes },
  { "iso-8859-6",                                    28596, yes },
  { "iso-ir-127",                                    28596, yes },
  { "iso_8859-6",                                    28596, yes },
  { "iso_8859-6:1987",                               28596, yes },
  { "csisolatingreek",                               28597, yes },
  { "ecma-118",                                      28597, yes },
  { "elot_928",                                      28597, yes },
  { "greek",                                         28597, yes },
  { "greek8",                                        28597, yes },
  { "iso-8859-7",                                    28597, yes },
  { "iso-ir-126",                                    28597, yes },
  { "iso_8859-7",                                    28597, yes },
  { "iso_8859-7:1987",                               28597, yes },
  { "csisolatinhebrew",                              28598, yes },
  { "hebrew",                                        28598, yes },
  { "iso-8859-8",                                    28598, yes },
  { "iso-ir-138",                                    28598, yes },
  { "iso_8859-8",                                    28598, yes },
  { "iso_8859-8:1988",                               28598, yes },
  { "logical",                                       28598, yes },
  { "visual",                                        28598, yes },
  { "csisolatin5",                                   28599, yes },
  { "iso-8859-9",                                    28599, yes },
  { "iso-ir-148",                                    28599, yes },
  { "iso_8859-9",                                    28599, yes },
  { "iso_8859-9:1989",                               28599, yes },
  { "l5",                                            28599, yes },
  { "latin5",                                        28599, yes },
  { "iso-8859-13",                                   28603, yes },
  { "csisolatin9",                                   28605, yes },
  { "iso-8859-15",                                   28605, yes },
  { "iso_8859-15",                                   28605, yes },
  { "l9",                                            28605, yes },
  { "latin9",                                        28605, yes },
  { "x-europa",                                      29001, yes },
  { "iso-8859-8-i",                                  38598, yes },
  { "iso-2022-jp",                                   50220,  no },
  { "csiso2022jp",                                   50221,  no },
  { "csiso2022kr",                                   50225,  no },
  { "iso-2022-kr",                                   50225,  no },
  { "iso-2022-kr-7",                                 50225,  no },
  { "iso-2022-kr-7bit",                              50225,  no },
  { "cp50227",                                       50227,  no },
  { "x-cp50227",                                     50227,  no },
  { "cp930",                                         50930, yes },
  { "x-ebcdic-japaneseanduscanada",                  50931, yes },
  { "cp933",                                         50933, yes },
  { "cp935",                                         50935, yes },
  { "cp937",                                         50937, yes },
  { "cp939",                                         50939, yes },
  { "cseucpkdfmtjapanese",                           51932, yes },
  { "euc-jp",                                        51932, yes },
  { "extended_unix_code_packed_format_for_japanese", 51932, yes },
  { "iso-2022-jpeuc",                                51932, yes },
  { "x-euc",                                         51932, yes },
  { "x-euc-jp",                                      51932, yes },
  { "euc-cn",                                        51936, yes },
  { "x-euc-cn",                                      51936, yes },
  { "cseuckr",                                       51949, yes },
  { "euc-kr",                                        51949, yes },
  { "iso-2022-kr-8",                                 51949, yes },
  { "iso-2022-kr-8bit",                              51949, yes },
  { "hz-gb-2312",                                    52936,  no },
  { "gb18030",                                       54936, yes },
  { "x-iscii-de",                                    57002, yes },
  { "x-iscii-be",                                    57003, yes },
  { "x-iscii-ta",                                    57004, yes },
  { "x-iscii-te",                                    57005, yes },
  { "x-iscii-as",                                    57006, yes },
  { "x-iscii-or",                                    57007, yes },
  { "x-iscii-ka",                                    57008, yes },
  { "x-iscii-ma",                                    57009, yes },
  { "x-iscii-gu",                                    57010, yes },
  { "x-iscii-pa",                                    57011, yes },
  { "csunicode11utf7",                               65000,  no },
  { "unicode-1-1-utf-7",                             65000,  no },
  { "unicode-2-0-utf-7",                             65000,  no },
  { "utf-7",                                         65000,  no },
  { "x-unicode-1-1-utf-7",                           65000,  no },
  { "x-unicode-2-0-utf-7",                           65000,  no },
  { "unicode-1-1-utf-8",                             65001, yes },
  { "unicode-2-0-utf-8",                             65001, yes },
  { "utf-8",                                         65001, yes },
  { "x-unicode-1-1-utf-8",                           65001, yes },
  { "x-unicode-2-0-utf-8",                           65001, yes },

  /* final entry */
  { NULL,                                                0,  no }
};

uint TY_(Win32MLangGetCPFromName)(TidyAllocator *allocator, ctmbstr encoding)
{
    uint i;
    tmbstr enc;

    /* ensure name is in lower case */
    enc = TY_(tmbstrdup)(allocator,encoding);
    enc = TY_(tmbstrtolower)(enc);

    for (i = 0; NameWinCPMap[i].name; ++i)
    {
        if (TY_(tmbstrcmp)(NameWinCPMap[i].name, enc) == 0)
        {
            IMLangConvertCharset * p = NULL;
            uint wincp = NameWinCPMap[i].wincp;
            HRESULT hr;

            TidyFree(allocator, enc);

            /* currently no support for unsafe encodings */
            if (!NameWinCPMap[i].safe)
                return 0;

            /* hack for config.c */
            CoInitialize(NULL);
            hr = CreateMLangObject(p);

            if (hr != S_OK || !p)
            {
                wincp = 0;
            }
            else
            {
                hr = IMLangConvertCharset_Initialize(p, wincp, 1200, 0);

                if (hr != S_OK)
                    wincp = 0;

                IMLangConvertCharset_Release(p);
                p = NULL;
            }

            CoUninitialize();

            return wincp;
        }
    }

    TidyFree(allocator, enc);
    return 0;
}

Bool TY_(Win32MLangInitInputTranscoder)(StreamIn * in, uint wincp)
{
    IMLangConvertCharset * p = NULL;
    HRESULT hr;

    assert( in != NULL );

    CoInitialize(NULL);

    if (wincp == 0)
    {
        /* no codepage found for this encoding */
        return no;
    }

    hr = CreateMLangObject(p);

    if (hr != S_OK || !p)
    {
        /* MLang not supported */
        return no;
    }

    hr = IMLangConvertCharset_Initialize(p, wincp, 1200, 0);

    if (hr != S_OK)
    {
        /* encoding not supported, insufficient memory, etc. */
        return no;
    }

    in->mlang = p;

    return yes;
}

void TY_(Win32MLangUninitInputTranscoder)(StreamIn * in)
{
    IMLangConvertCharset * p;

    assert( in != NULL );

    p = (IMLangConvertCharset *)in->mlang;
    if (p)
    {
        IMLangConvertCharset_Release(p);
        p = NULL;
        in->mlang = NULL;
    }

    CoUninitialize();
}

#if 0
Bool Win32MLangInitOutputTranscoder(TidyAllocator *allocator, StreamOut * out, tmbstr encoding)
{
    IMLangConvertCharset * p = NULL;
    HRESULT hr;
    uint wincp;

    assert( out != NULL );

    CoInitialize(NULL);

    wincp = TY_(Win32MLangGetCPFromName)(allocator, encoding);
    if (wincp == 0)
    {
        /* no codepage found for this encoding */
        return no;
    }

    hr = CreateMLangObject(p);

    if (hr != S_OK || !p)
    {
        /* MLang not supported */
        return no;
    }

    IMLangConvertCharset_Initialize(p, 1200, wincp, MLCONVCHARF_NOBESTFITCHARS);

    if (hr != S_OK)
    {
        /* encoding not supported, insufficient memory, etc. */
        return no;
    }

    out->mlang = p;

    return yes;
}

void Win32MLangUninitOutputTranscoder(StreamOut * out)
{
    IMLangConvertCharset * p;

    assert( out != NULL );

    p = (IMLangConvertCharset *)out->mlang;
    if (p)
    {
        IMLangConvertCharset_Release(p);
        p = NULL;
        out->mlang = NULL;
    }

    CoUninitialize();
}
#endif

int TY_(Win32MLangGetChar)(byte firstByte, StreamIn * in, uint * bytesRead)
{
    IMLangConvertCharset * p;
    TidyInputSource * source;
    CHAR inbuf[TC_INBUFSIZE] = { 0 };
    WCHAR outbuf[TC_OUTBUFSIZE] = { 0 };
    HRESULT hr = S_OK;
    size_t inbufsize = 0;

    assert( in != NULL );
    assert( &in->source != NULL );
    assert( bytesRead != NULL );
    assert( in->mlang != NULL );

    p = (IMLangConvertCharset *)in->mlang;
    source = &in->source;

    inbuf[inbufsize++] = (CHAR)firstByte;

    while(inbufsize < TC_INBUFSIZE)
    {
        UINT outbufsize = TC_OUTBUFSIZE;
        UINT readNow = inbufsize;
        int nextByte = EndOfStream;

        hr = IMLangConvertCharset_DoConversionToUnicode(p, inbuf, &readNow, outbuf, &outbufsize);

        assert( hr == S_OK );
        assert( outbufsize <= 2 );

        if (outbufsize == 2)
        {
            /* U+10000-U+10FFFF are returned as a pair of surrogates */
            tchar m = (tchar)outbuf[0];
            tchar n = (tchar)outbuf[1];
            assert( TY_(IsHighSurrogate)(n) && TY_(IsLowSurrogate)(m) );
            *bytesRead = readNow;
            return (int)TY_(CombineSurrogatePair)(n, m);
        }

        if (outbufsize == 1)
        {
            /* we found the character   */
            /* set bytesRead and return */
            *bytesRead = readNow;
            return (int)outbuf[0];
        }

        /* we need more bytes */
        nextByte = source->getByte(source->sourceData);

        if (nextByte == EndOfStream)
        {
            /* todo: error message for broken stream? */

            *bytesRead = readNow;
            return EndOfStream;
        }

        inbuf[inbufsize++] = (CHAR)nextByte;
    }

    /* No full character found after reading TC_INBUFSIZE bytes, */
    /* give up to read this stream, it's obviously unreadable.   */

    /* todo: error message for broken stream? */
    return EndOfStream;
}

Bool Win32MLangIsConvertible(tchar c, StreamOut * out)
{
    IMLangConvertCharset * p;
    UINT i = 1;
    HRESULT hr;
    WCHAR inbuf[2] = { 0 };
    UINT inbufsize = 0;

    assert( c != 0 );
    assert( c <= 0x10FFFF );
    assert( out != NULL );
    assert( out->mlang != NULL );

    if (c > 0xFFFF)
    {
        tchar high = 0;
        tchar low = 0;

        TY_(SplitSurrogatePair)(c, &low, &high);

        inbuf[inbufsize++] = (WCHAR)low;
        inbuf[inbufsize++] = (WCHAR)high;
    }
    else
        inbuf[inbufsize++] = (WCHAR)c;

    p = (IMLangConvertCharset *)out->mlang;
    hr = IMLangConvertCharset_DoConversionFromUnicode(p, inbuf, &inbufsize, NULL, NULL);

    return hr == S_OK ? yes : no;
}

void Win32MLangPutChar(tchar c, StreamOut * out, uint * bytesWritten)
{
    IMLangConvertCharset * p;
    TidyOutputSink * sink;
    CHAR outbuf[TC_OUTBUFSIZE] = { 0 };
    UINT outbufsize = TC_OUTBUFSIZE;
    HRESULT hr = S_OK;
    WCHAR inbuf[2] = { 0 };
    UINT inbufsize = 0;
    uint i;

    assert( c != 0 );
    assert( c <= 0x10FFFF );
    assert( bytesWritten != NULL );
    assert( out != NULL );
    assert( &out->sink != NULL );
    assert( out->mlang != NULL );

    p = (IMLangConvertCharset *)out->mlang;
    sink = &out->sink;

    if (c > 0xFFFF)
    {
        tchar high = 0;
        tchar low = 0;

        TY_(SplitSurrogatePair)(c, &low, &high);

        inbuf[inbufsize++] = (WCHAR)low;
        inbuf[inbufsize++] = (WCHAR)high;
    }
    else
        inbuf[inbufsize++] = (WCHAR)c;

    hr = IMLangConvertCharset_DoConversionFromUnicode(p, inbuf, &inbufsize, outbuf, &outbufsize);
    
    assert( hr == S_OK );
    assert( outbufsize > 0 );
    assert( inbufsize == 1 || inbufsize == 2 );

    for (i = 0; i < outbufsize; ++i)
        sink->putByte(sink->sinkData, (byte)(outbuf[i]));

    *bytesWritten = outbufsize;

    return;
}

#endif /* TIDY_WIN32_MLANG_SUPPORT */

/*
 * local variables:
 * mode: c
 * indent-tabs-mode: nil
 * c-basic-offset: 4
 * eval: (c-set-offset 'substatement-open 0)
 * end:
 */
