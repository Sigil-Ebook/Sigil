/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * Copyright (C) 2002-2022 Németh László
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Hunspell is based on MySpell which is Copyright (C) 2002 Kevin Hendricks.
 *
 * Contributor(s): David Einstein, Davide Prina, Giuseppe Modugno,
 * Gianluca Turconi, Simon Brouwer, Noll János, Bíró Árpád,
 * Goldman Eleonóra, Sarlós Tamás, Bencsáth Boldizsár, Halácsy Péter,
 * Dvornik László, Gefferth András, Nagy Viktor, Varga Dániel, Chris Halls,
 * Rene Engelhard, Bram Moolenaar, Dafydd Jones, Harri Pitkänen
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

// glibc < 3.0 (for mkstemp)
#ifndef __USE_MISC
#define __USE_MISC
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include <string.h>
#include <config.h>
#include "../hunspell/atypes.hxx"
#include "../hunspell/hunspell.hxx"
#include "../hunspell/csutil.hxx"
#include "../hunspell/hunzip.hxx"

#define HUNSPELL_VERSION VERSION
#define INPUTLEN 50

#define HUNSPELL_PIPE_HEADING                                                  \
  "@(#) International Ispell Version 3.2.06 (but really Hunspell " VERSION ")" \
                                                                           "\n"
#define HUNSPELL_HEADING "Hunspell "
#define ODF_EXT "odt|ott|odp|otp|odg|otg|ods|ots"
#define ENTITY_APOS "&apos;"
#define UTF8_APOS "\xe2\x80\x99"

// for debugging only
//#define LOG

#define DEFAULTDICNAME "default"

#ifdef WIN32

#define LIBDIR "C:\\Hunspell\\"
#define USEROOODIR { "Application Data\\OpenOffice.org 2\\user\\wordbook" }
#define OOODIR                                                 \
  "C:\\Program files\\OpenOffice.org 2.4\\share\\dict\\ooo\\;" \
  "C:\\Program files\\OpenOffice.org 2.3\\share\\dict\\ooo\\;" \
  "C:\\Program files\\OpenOffice.org 2.2\\share\\dict\\ooo\\;" \
  "C:\\Program files\\OpenOffice.org 2.1\\share\\dict\\ooo\\;" \
  "C:\\Program files\\OpenOffice.org 2.0\\share\\dict\\ooo\\"
#define HOME "%USERPROFILE%\\"
#define DICBASENAME "hunspell_"
#define LOGFILE "C:\\Hunspell\\log"
#define DIRSEPCH '\\'
#define DIRSEP "\\"
#define PATHSEP ";"

#ifdef __MINGW32__
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

#include "../parsers/textparser.hxx"
#include "../parsers/htmlparser.hxx"
#include "../parsers/latexparser.hxx"
#include "../parsers/manparser.hxx"
#include "../parsers/firstparser.hxx"
#include "../parsers/xmlparser.hxx"
#include "../parsers/odfparser.hxx"

#else

// Not Windows
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include "../parsers/textparser.hxx"
#include "../parsers/htmlparser.hxx"
#include "../parsers/latexparser.hxx"
#include "../parsers/manparser.hxx"
#include "../parsers/firstparser.hxx"
#include "../parsers/xmlparser.hxx"
#include "../parsers/odfparser.hxx"

#define LIBDIR                \
  "/usr/share/hunspell:"      \
  "/usr/share/myspell:"       \
  "/usr/share/myspell/dicts:" \
  "/Library/Spelling"
#define USEROOODIR {                  \
  ".openoffice.org/3/user/wordbook", \
  ".openoffice.org2/user/wordbook",  \
  ".openoffice.org2.0/user/wordbook",\
  "Library/Spelling" }
#define OOODIR                                       \
  "/opt/openoffice.org/basis3.0/share/dict/ooo:"     \
  "/usr/lib/openoffice.org/basis3.0/share/dict/ooo:" \
  "/opt/openoffice.org2.4/share/dict/ooo:"           \
  "/usr/lib/openoffice.org2.4/share/dict/ooo:"       \
  "/opt/openoffice.org2.3/share/dict/ooo:"           \
  "/usr/lib/openoffice.org2.3/share/dict/ooo:"       \
  "/opt/openoffice.org2.2/share/dict/ooo:"           \
  "/usr/lib/openoffice.org2.2/share/dict/ooo:"       \
  "/opt/openoffice.org2.1/share/dict/ooo:"           \
  "/usr/lib/openoffice.org2.1/share/dict/ooo:"       \
  "/opt/openoffice.org2.0/share/dict/ooo:"           \
  "/usr/lib/openoffice.org2.0/share/dict/ooo"
#define HOME getenv("HOME")
#define DICBASENAME ".hunspell_"
#define LOGFILE "/tmp/hunspell.log"
#define DIRSEPCH '/'
#define DIRSEP "/"
#define PATHSEP ":"
#endif

#ifdef HAVE_ICONV
#include <iconv.h>
#include <errno.h>
char text_conv[MAXLNLEN];
#endif

#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif
#ifdef HAVE_LANGINFO_H
# include <langinfo.h>
#endif
#ifdef ENABLE_NLS
# include <libintl.h>
#else
# undef gettext
# define gettext(Msgid) ((const char *) (Msgid))
# undef textdomain
# define textdomain(Domainname) ((const char *) (Domainname))
#endif

#ifdef HAVE_CURSES_H
#ifdef HAVE_NCURSESW_CURSES_H
#include <ncursesw/curses.h>
#else
#include <curses.h>
#endif
#endif

#ifdef HAVE_READLINE
#include <readline/readline.h>
#else
#define readline scanline
#endif

// file formats:

enum { FMT_TEXT, FMT_LATEX, FMT_HTML, FMT_MAN, FMT_FIRST, FMT_XML, FMT_ODF };

// global variables

std::string wordchars;
char* dicpath = NULL;
const w_char* wordchars_utf16 = NULL;
std::vector<w_char> new_wordchars_utf16;
int wordchars_utf16_len;
char* dicname = NULL;
char* privdicname = NULL;
const char* currentfilename = NULL;

int modified;  // modified file sign
bool multiple_files; // for listing file names in pipe interface

enum {
  NORMAL,
  BADWORD,     // print only bad words
  WORDFILTER,  // print only bad words from 1 word/line input
  BADLINE,     // print only lines with bad words
  STEM,        // stem input words
  ANALYZE,     // analyze input words
  PIPE,        // print only stars for LyX compatibility
  AUTO0,       // search typical error (based on SuggestMgr::suggest())
  AUTO,        // automatic spelling to standard output
  AUTO2,       // automatic spelling to standard output with sed log
  AUTO3,
  SUFFIX  // print suffixes that can be attached to a given word
};        // automatic spelling to standard output with gcc error format
int filter_mode = NORMAL;
int printgood = 0;  // print only good words and lines
int showpath = 0;   // show detected path of the dictionary
int checkurl = 0;   // check URLs and mail addresses
int checkapos = 0;  // force typographic apostrophe
int warn = 0;  // warn potential mistakes (dictionary words with WARN flags)
const char* ui_enc = NULL;  // locale character encoding (default for I/O)
const char* io_enc = NULL;  // I/O character encoding

#define DMAX 10  // maximal count of loaded dictionaries

const char* dic_enc[DMAX];  // dictionary encoding
char* path = NULL;
int dmax = 0;  // dictionary count

// functions

#ifdef HAVE_ICONV
static const char* fix_encoding_name(const char* enc) {
  if (strcmp(enc, "TIS620-2533") == 0)
    enc = "TIS620";
  return enc;
}
#endif

/* change character encoding */
std::string chenc(const std::string& st, const char* enc1, const char* enc2) {
#ifndef HAVE_ICONV
  (void)enc1;
  (void)enc2;
  return st;
#else
  if (st.empty())
    return st;

  if (!enc1 || !enc2 || strcmp(enc1, enc2) == 0)
    return st;

  std::string out(st.size() < 15 ? 15 : st.size(), '\0');
  size_t c1(st.size());
  size_t c2(out.size());
  ICONV_CONST char* source = (ICONV_CONST char*) &st[0];
  char* dest = &out[0];
  iconv_t conv = iconv_open(fix_encoding_name(enc2), fix_encoding_name(enc1));
  if (conv == (iconv_t)-1) {
    fprintf(stderr, gettext("error - iconv_open: %s -> %s\n"), enc2, enc1);
  } else {
    size_t res;
    while ((res = iconv(conv, &source, &c1, &dest, &c2)) == size_t(-1)) {
      if (errno == E2BIG) {
        //c2 is zero or close to zero
        size_t next_start = out.size() - c2;
        c2 += c1*2;
        out.resize(out.size() + c1*2);
        dest = &out[next_start];
      } else
        break;
    }
    if (res == (size_t)-1) {
      fprintf(stderr, gettext("error - iconv: %s -> %s\n"), enc2, enc1);
    }
    iconv_close(conv);
    out.resize(dest - &out[0]);
    return out;
  }

  return st;
#endif
}

TextParser* get_parser(int format, const char* extension, Hunspell* pMS) {
  TextParser* p = NULL;
  int io_utf8 = 0;
  const char* denc = pMS->get_dict_encoding().c_str();
#ifdef HAVE_ICONV
  if (io_enc) {
    if ((strcmp(io_enc, "UTF-8") == 0) || (strcmp(io_enc, "utf-8") == 0) ||
        (strcmp(io_enc, "UTF8") == 0) || (strcmp(io_enc, "utf8") == 0)) {
      io_utf8 = 1;
      io_enc = "UTF-8";
    }
  } else if (ui_enc) {
    io_enc = ui_enc;
    if (strcmp(ui_enc, "UTF-8") == 0)
      io_utf8 = 1;
  } else {
    io_enc = denc;
    if (strcmp(denc, "UTF-8") == 0)
      io_utf8 = 1;
  }

  if (io_utf8) {
    const std::vector<w_char>& vec_wordchars_utf16 = pMS->get_wordchars_utf16();
    const std::string& vec_wordchars = pMS->get_wordchars_cpp();
    wordchars_utf16_len = vec_wordchars_utf16.size();
    wordchars_utf16 = wordchars_utf16_len ? vec_wordchars_utf16.data() : NULL;
    if ((strcmp(denc, "UTF-8") != 0) && !vec_wordchars.empty()) {
      const char* wchars = vec_wordchars.c_str();
      size_t c1 = vec_wordchars.size();
      size_t c2 = MAXLNLEN;
      char* dest = text_conv;
      iconv_t conv = iconv_open("UTF-8", fix_encoding_name(denc));
      if (conv == (iconv_t)-1) {
        fprintf(stderr, gettext("error - iconv_open: UTF-8 -> %s\n"), denc);
        wordchars_utf16 = NULL;
        wordchars_utf16_len = 0;
      } else {
        iconv(conv, (ICONV_CONST char**)&wchars, &c1, &dest, &c2);
        iconv_close(conv);
        u8_u16(new_wordchars_utf16, text_conv);
        std::sort(new_wordchars_utf16.begin(), new_wordchars_utf16.end());
        wordchars_utf16 = new_wordchars_utf16.data();
        wordchars_utf16_len = new_wordchars_utf16.size();
      }
    }
  } else {
    // 8-bit input encoding
    // detect letters by unicodeisalpha() for tokenization
    char letters[MAXLNLEN];
    char* pletters = letters;
    char ch[2];
    char u8[10];
    *pletters = '\0';
    iconv_t conv = iconv_open("UTF-8", fix_encoding_name(io_enc));
    if (conv == (iconv_t)-1) {
      fprintf(stderr, gettext("error - iconv_open: UTF-8 -> %s\n"), io_enc);
    } else {
      for (int i = 32; i < 256; i++) {
        size_t c1 = 1;
        size_t c2 = 10;
        char* dest = u8;
        u8[0] = '\0';
        char* ch8bit = ch;
        ch[0] = (char)i;
        ch[1] = '\0';
        size_t res = iconv(conv, (ICONV_CONST char**)&ch8bit, &c1, &dest, &c2);
        if (res != (size_t)-1) {
          std::vector<w_char> w;
          u8_u16(w, std::string(u8, dest));
          unsigned short idx = w.empty() ? 0 : (w[0].h << 8) + w[0].l;
          if (unicodeisalpha(idx)) {
            *pletters = (char)i;
            pletters++;
          }
        }
      }
      iconv_close(conv);
    }
    *pletters = '\0';

    // UTF-8 wordchars -> 8 bit wordchars
    const std::string& vec_wordchars = pMS->get_wordchars_cpp();
    size_t len = vec_wordchars.size();
    if (len) {
      if ((strcmp(denc, "UTF-8") == 0)) {
        len = pMS->get_wordchars_utf16().size();
      }
      char* dest = letters + strlen(letters);  // append wordchars
      size_t c1 = len + 1;
      size_t c2 = len + 1;
      conv = iconv_open(fix_encoding_name(io_enc), fix_encoding_name(denc));
      if (conv == (iconv_t)-1) {
        fprintf(stderr, gettext("error - iconv_open: %s -> %s\n"), io_enc,
                denc);
      } else {
        const char* wchars = vec_wordchars.c_str();
        iconv(conv, (ICONV_CONST char**)&wchars, &c1, &dest, &c2);
        iconv_close(conv);
        *dest = '\0';
      }
    }
    if (*letters)
      wordchars.assign(letters);
  }
#else
  if (strcmp(denc, "UTF-8") == 0) {
    const std::vector<w_char>& vec_wordchars_utf16 = pMS->get_wordchars_utf16();
    wordchars_utf16 = (vec_wordchars_utf16.size() == 0) ? NULL : vec_wordchars_utf16.data();
    wordchars_utf16_len = vec_wordchars_utf16.size();
    io_utf8 = 1;
  } else {
    std::string casechars = get_casechars(denc);
    std::string wchars = pMS->get_wordchars_cpp();
    wordchars = casechars + wchars;
  }
  io_enc = denc;
#endif

  if (io_utf8) {
    switch (format) {
      case FMT_LATEX:
        p = new LaTeXParser(wordchars_utf16, wordchars_utf16_len);
        break;
      case FMT_HTML:
        p = new HTMLParser(wordchars_utf16, wordchars_utf16_len);
        break;
      case FMT_MAN:
        p = new ManParser(wordchars_utf16, wordchars_utf16_len);
        break;
      case FMT_XML:
        p = new XMLParser(wordchars_utf16, wordchars_utf16_len);
        break;
      case FMT_ODF:
        p = new ODFParser(wordchars_utf16, wordchars_utf16_len);
        break;
      case FMT_FIRST:
        p = new FirstParser(wordchars.c_str());
    }
  } else {
    switch (format) {
      case FMT_LATEX:
        p = new LaTeXParser(wordchars.c_str());
        break;
      case FMT_HTML:
        p = new HTMLParser(wordchars.c_str());
        break;
      case FMT_MAN:
        p = new ManParser(wordchars.c_str());
        break;
      case FMT_XML:
        p = new XMLParser(wordchars.c_str());
        break;
      case FMT_ODF:
        p = new ODFParser(wordchars.c_str());
        break;
      case FMT_FIRST:
        p = new FirstParser(wordchars.c_str());
    }
  }

  if ((!p) && (extension)) {
    if ((strcmp(extension, "html") == 0) || (strcmp(extension, "htm") == 0) ||
        (strcmp(extension, "xhtml") == 0)) {
      if (io_utf8) {
        p = new HTMLParser(wordchars_utf16, wordchars_utf16_len);
      } else {
        p = new HTMLParser(wordchars.c_str());
      }
    } else if ((strcmp(extension, "xml") == 0)) {
      if (io_utf8) {
        p = new XMLParser(wordchars_utf16, wordchars_utf16_len);
      } else {
        p = new XMLParser(wordchars.c_str());
      }
    } else if (((strlen(extension) == 3) &&
                (strstr(ODF_EXT, extension) != NULL)) ||
               ((strlen(extension) == 4) && (extension[0] == 'f') &&
                (strstr(ODF_EXT, extension + 1) != NULL))) {
      if (io_utf8) {
        p = new ODFParser(wordchars_utf16, wordchars_utf16_len);
      } else {
        p = new ODFParser(wordchars.c_str());
      }
    } else if (((extension[0] > '0') && (extension[0] <= '9'))) {
      if (io_utf8) {
        p = new ManParser(wordchars_utf16, wordchars_utf16_len);
      } else {
        p = new ManParser(wordchars.c_str());
      }
    } else if ((strcmp(extension, "tex") == 0)) {
      if (io_utf8) {
        p = new LaTeXParser(wordchars_utf16, wordchars_utf16_len);
      } else {
        p = new LaTeXParser(wordchars.c_str());
      }
    }
  }
  if (!p) {
    if (io_utf8) {
      p = new TextParser(wordchars_utf16, wordchars_utf16_len);
    } else {
      p = new TextParser(wordchars.c_str());
    }
  }
  p->set_url_checking(checkurl);
  return p;
}

#ifdef LOG
void log(char* message) {
  FILE* f = fopen(LOGFILE, "a");
  if (f) {
    fprintf(f, "%s\n", message);
    fclose(f);
  } else {
    fprintf(stderr, "Logfile...");
  }
}
#endif

int putdic(const std::string& in_word, Hunspell* pMS) {
  std::string word = chenc(in_word, ui_enc, dic_enc[0]);

  std::string buf;
  pMS->input_conv(word.c_str(), buf);
  word = buf;

  if (word.empty())
    return 0;

  int ret(0);
  size_t w = word.find('/', 1);
  if (w == std::string::npos) {
    if (word[0] == '*')
      ret = pMS->remove(word.substr(1));
    else
      ret = pMS->add(word);
  } else {
    std::string affix = word.substr(w + 1);
    word.resize(w);
    if (!affix.empty() && affix[0] == '/') // word//pattern (back comp.)
        affix.erase(0, 1);
    ret = pMS->add_with_affix(word, affix);  // word/pattern
  }
  return ret;
}

void load_privdic(const char* filename, Hunspell* pMS) {
  std::ifstream dic;
  dic.open(filename, std::ios_base::in);
  if (dic.is_open()) {
    std::string buf;
    while (std::getline(dic, buf)) {
      putdic(buf, pMS);
    }
  }
}

bool exist(const char* filename) {
  std::ifstream f;
  f.open(filename, std::ios_base::in);
  if (f.is_open()) {
    return true;
  }
  return false;
}

int save_privdic(const std::string& filename, const std::string& filename2, std::vector<std::string>& w) {
  FILE* dic = fopen(filename.c_str(), "r");
  if (dic) {
    fclose(dic);
    dic = fopen(filename.c_str(), "a");
  } else {
    dic = fopen(filename2.c_str(), "a");
  }
  if (!dic)
    return 0;
  for (size_t i = 0; i < w.size(); ++i) {
    w[i] = chenc(w[i], io_enc, ui_enc);
    fprintf(dic, "%s\n", w[i].c_str());
  }
  fclose(dic);
  return 1;
}

const char* basename(const char* s, char c) {
  const char* p = s + strlen(s);
  while ((*p != c) && (p != s))
    p--;
  if (*p == c)
    p++;
  return p;
}

char* mystrdup(const char* s) {
  char* d = NULL;
  if (s) {
    int sl = strlen(s) + 1;
    d = (char*)malloc(sl);
    if (d)
      memcpy(d, s, sl);
  }
  return d;
}

#ifdef HAVE_CURSES_H
char* scanline(const char* message) {
  char input[INPUTLEN];
  printw("%s", message);
  echo();
  getnstr(input, INPUTLEN);
  noecho();
  return mystrdup(input);
}
#endif

// check words in the dictionaries (and set first checked dictionary)
bool check(Hunspell** pMS, int* d, const std::string& token, int* info, std::string* root) {
  for (int i = 0; i < dmax; ++i) {
    std::string buf = chenc(token, io_enc, dic_enc[*d]);
    mystrrep(buf, ENTITY_APOS, "'");
    if (checkapos && buf.find('\'') != std::string::npos)
      return false;
    // 8-bit encoded dictionaries need ASCII apostrophes (eg. English
    // dictionaries)
    if (strcmp(dic_enc[*d], "UTF-8") != 0)
      mystrrep(buf, UTF8_APOS, "'");
    if ((pMS[*d]->spell(buf, info, root) &&
         !(warn && (*info & SPELL_WARN))) ||
        // UTF-8 encoded dictionaries with ASCII apostrophes, but without ICONV
        // support,
        // need also ASCII apostrophes (eg. French dictionaries)
        ((strcmp(dic_enc[*d], "UTF-8") == 0) &&
         buf.find(UTF8_APOS) != std::string::npos &&
         pMS[*d]->spell(mystrrep(buf, UTF8_APOS, "'"), info, root) &&
         !(warn && (*info & SPELL_WARN)))) {
      return true;
    }
    if (++(*d) == dmax)
      *d = 0;
  }
  return false;
}

static bool is_zipped_odf(TextParser* parser, const char* extension) {
  // ODFParser and not flat ODF
  return dynamic_cast<ODFParser*>(parser) && (extension && extension[0] != 'f');
}

static bool secure_filename(const char* filename) {
  const char* hasapostrophe = strchr(filename, '\'');
  if (hasapostrophe)
    return false;
  return true;
}

char* mymkdtemp(char *templ) {
#ifdef WIN32
  (void)templ;
  char *odftmpdir = tmpnam(NULL);
  if (!odftmpdir) {
    return NULL;
  }
  if (system((std::string("mkdir ") + odftmpdir).c_str()) != 0) {
    return NULL;
  }
  return odftmpdir;
#else
  return mkdtemp(templ);
#endif
}

void pipe_interface(Hunspell** pMS, int format, FILE* fileid, char* filename) {
  char buf[MAXLNLEN];
  std::vector<std::string> dicwords;
  int pos;
  int bad;
  int lineno = 0;
  int terse_mode = 0;
  int verbose_mode = 0;
  int d = 0;
  char* odftmpdir = NULL;

  std::string filename_prefix = (multiple_files) ? filename + std::string(": ") : "";

  const char* extension = (filename) ? basename(filename, '.') : NULL;
  TextParser* parser = get_parser(format, extension, pMS[0]);
  char tmpdirtemplate[] = "/tmp/hunspellXXXXXX";

  bool bZippedOdf = is_zipped_odf(parser, extension);
  // access content.xml of ODF
  if (bZippedOdf) {
    odftmpdir = mymkdtemp(tmpdirtemplate);
    if (!odftmpdir) {
      perror(gettext("Can't create tmp dir"));
      exit(1);
    }
    // break 1-line XML of zipped ODT documents at </style:style> and </text:p>
    // to avoid tokenization problems (fgets could stop within an XML tag)
    std::ostringstream sbuf;
    sbuf << "unzip -p \"" << filename << "\" content.xml | sed "
            "\"s/\\(<\\/text:p>\\|<\\/style:style>\\)\\(.\\)/\\1\\n\\2/g;s/<\\/\\?text:span[^>]*>//g\" "
            ">" << odftmpdir << "/content.xml";
    if (!secure_filename(filename) || system(sbuf.str().c_str()) != 0) {
      if (secure_filename(filename))
        perror(gettext("Can't open inputfile"));
      else
        fprintf(stderr, gettext("Can't open %s.\n"), filename);
      if (system((std::string("rmdir ") + odftmpdir).c_str()) != 0) {
        perror("temp dir delete failed");
      }
      exit(1);
    }
    std::string file(odftmpdir);
    file.append("/content.xml");
    fileid = fopen(file.c_str(), "r");
    if (fileid == NULL) {
      perror(gettext("Can't open inputfile"));
      if (system((std::string("rmdir ") + odftmpdir).c_str()) != 0) {
        perror("temp dir delete failed");
      }
      exit(1);
    }
  }

  if (filter_mode == NORMAL) {
    fprintf(stdout, "%s", gettext(HUNSPELL_HEADING));
    fprintf(stdout, HUNSPELL_VERSION);
    const std::string& version = pMS[0]->get_version_cpp();
    if (!version.empty())
      fprintf(stdout, " - %s", version.c_str());
    fprintf(stdout, "\n");
    fflush(stdout);
  }

nextline:
  while (fgets(buf, MAXLNLEN, fileid)) {
    buf[strcspn(buf, "\n")] = 0;
    lineno++;
#ifdef LOG
    log(buf);
#endif
    bad = 0;
    pos = 0;

    // execute commands
    if (filter_mode == PIPE) {
      pos = -1;
      switch (buf[0]) {
        case '%': {
          verbose_mode = terse_mode = 0;
          break;
        }
        case '!': {
          terse_mode = 1;
          break;
        }
        case '`': {
          verbose_mode = 1;
          break;
        }
        case '+': {
          delete parser;
          parser = get_parser(FMT_LATEX, NULL, pMS[0]);
          parser->set_url_checking(checkurl);
          break;
        }
        case '-': {
          delete parser;
          parser = get_parser(format, NULL, pMS[0]);
          break;
        }
        case '@': {
          putdic(buf + 1, pMS[d]);
          break;
        }
        case '*': {
          std::string word(buf + 1);
          dicwords.push_back(word);
          putdic(word, pMS[d]);
          break;
        }
        case '#': {
          std::string sbuf;
          if (HOME) {
            sbuf.append(HOME);
          } else {
            fprintf(stderr, "%s", gettext("error - missing HOME variable\n"));
            continue;
          }
#ifndef WIN32
          sbuf.append("/");
#endif
          size_t offset = sbuf.size();
          if (!privdicname) {
            sbuf.append(DICBASENAME);
            sbuf.append(basename(dicname, DIRSEPCH));
          } else {
            sbuf.append(privdicname);
          }
          if (save_privdic(sbuf.substr(offset), sbuf, dicwords)) {
            dicwords.clear();
          }
          break;
        }
        case '^': {
          pos = 1;
          break;
        }

        default: {
          pos = 0;
          break;
        }

      }  // end switch
    }    // end filter_mode == PIPE

    if (pos >= 0) {
      parser->put_line(buf + pos);
      std::string token;
      while (parser->next_token(token)) {
        token = parser->get_word(token);
        mystrrep(token, ENTITY_APOS, "'");
        switch (filter_mode) {
          case BADWORD: {
            int info;
            if (!check(pMS, &d, token, &info, NULL)) {
              bad = 1;
              if (!printgood)
                fprintf(stdout, "%s%s\n", filename_prefix.c_str(), token.c_str());
            } else {
              if (printgood)
                fprintf(stdout, "%s%s\n", filename_prefix.c_str(), token.c_str());
            }
            continue;
          }

          case WORDFILTER: {
            int info;
            if (!check(pMS, &d, parser->get_word(token), &info, NULL)) {
              if (!printgood)
                fprintf(stdout, "%s\n", buf);
            } else {
              if (printgood)
                fprintf(stdout, "%s\n", buf);
            }
            goto nextline;
          }

          case BADLINE: {
            int info;
            if (!check(pMS, &d, parser->get_word(token), &info, NULL)) {
              bad = 1;
            }
            continue;
          }

          case AUTO0:
          case AUTO:
          case AUTO2:
          case AUTO3: {
            FILE* f = (filter_mode == AUTO) ? stderr : stdout;
            int info;
            if (!check(pMS, &d, parser->get_word(token), &info, NULL)) {
              bad = 1;
              std::vector<std::string> wlst =
                  pMS[d]->suggest(chenc(parser->get_word(token), io_enc, dic_enc[d]));
              if (!wlst.empty()) {
                parser->change_token(chenc(wlst[0], dic_enc[d], io_enc).c_str());
                if (filter_mode == AUTO3) {
                  fprintf(f, "%s:%d: Locate: %s | Try: %s\n", currentfilename,
                          lineno, token.c_str(), chenc(wlst[0], dic_enc[d], io_enc).c_str());
                } else if (filter_mode == AUTO2) {
                  fprintf(f, "%ds/%s/%s/g; # %s\n", lineno, token.c_str(),
                          chenc(wlst[0], dic_enc[d], io_enc).c_str(), buf);
                } else {
                  fprintf(f, gettext("Line %d: %s -> "), lineno,
                          chenc(token, io_enc, ui_enc).c_str());
                  fprintf(f, "%s\n", chenc(wlst[0], dic_enc[d], ui_enc).c_str());
                }
              }
            }
            continue;
          }

          case STEM: {
            std::vector<std::string> result =
              pMS[d]->stem(chenc(token, io_enc, dic_enc[d]));
            for (size_t i = 0; i < result.size(); ++i) {
              fprintf(stdout, "%s %s\n", token.c_str(),
                      chenc(result[i], dic_enc[d], ui_enc).c_str());
            }
            if (result.empty() && !token.empty() && token[token.size() - 1] == '.') {
              token.resize(token.size() - 1);
              result = pMS[d]->stem(token);
              for (size_t i = 0; i < result.size(); ++i) {
                fprintf(stdout, "%s %s\n", token.c_str(),
                        chenc(result[i], dic_enc[d], ui_enc).c_str());
              }
            }
            if (result.empty())
              fprintf(stdout, "%s\n", chenc(token, dic_enc[d], ui_enc).c_str());
            fprintf(stdout, "\n");
            continue;
          }

          case SUFFIX: {
            std::vector<std::string> wlst = pMS[d]->suffix_suggest(token);
            for (size_t j = 0; j < wlst.size(); ++j) {
              fprintf(stdout, "Suffix Suggestions are %s \n",
                      chenc(wlst[j], dic_enc[d], io_enc).c_str());
            }
            fflush(stdout);
            continue;
          }
          case ANALYZE: {
            std::vector<std::string> result =
              pMS[d]->analyze(chenc(token, io_enc, dic_enc[d]));
            for (size_t i = 0; i < result.size(); ++i) {
              fprintf(stdout, "%s %s\n", token.c_str(),
                      chenc(result[i], dic_enc[d], ui_enc).c_str());
            }
            if (result.empty() && !token.empty() && token[token.size() - 1] == '.') {
              token.resize(token.size() - 1);
              result = pMS[d]->analyze(token);
              for (size_t i = 0; i < result.size(); ++i) {
                fprintf(stdout, "%s %s\n", token.c_str(),
                        chenc(result[i], dic_enc[d], ui_enc).c_str());
              }
            }
            if (result.empty())
              fprintf(stdout, "%s\n", chenc(token, dic_enc[d], ui_enc).c_str());
            fprintf(stdout, "\n");
            continue;
          }

          case PIPE: {
            int info;
            std::string root;
            if (check(pMS, &d, parser->get_word(token), &info, &root)) {
              if (!terse_mode) {
                if (verbose_mode)
                  fprintf(stdout, "* %s\n", token.c_str());
                else
                  fprintf(stdout, "*\n");
              }
              fflush(stdout);
            } else {
              int byte_offset = parser->get_tokenpos() + pos;
              int char_offset = 0;
              if (strcmp(io_enc, "UTF-8") == 0) {
                for (int i = 0; i < byte_offset; i++) {
                  if ((buf[i] & 0xc0) != 0x80)
                    char_offset++;
                }
              } else {
                char_offset = byte_offset;
              }
              std::vector<std::string> wlst =
                pMS[d]->suggest(chenc(token, io_enc, dic_enc[d]));
              if (wlst.empty()) {
                fprintf(stdout, "# %s %d", token.c_str(), char_offset);
              } else {
                fprintf(stdout, "& %s %u %d: ", token.c_str(), static_cast<unsigned int>(wlst.size()), char_offset);
                fprintf(stdout, "%s", chenc(wlst[0], dic_enc[d], io_enc).c_str());
              }
              for (size_t j = 1; j < wlst.size(); ++j) {
                  fprintf(stdout, ", %s", chenc(wlst[j], dic_enc[d], io_enc).c_str());
              }
              fprintf(stdout, "\n");
              fflush(stdout);
            }
            continue;
          }
          case NORMAL: {
            int info;
            std::string root;
            if (check(pMS, &d, token, &info, &root)) {
              if (info & SPELL_COMPOUND) {
                fprintf(stdout, "-\n");
              } else if (!root.empty()) {
                fprintf(stdout, "+ %s\n", chenc(root, dic_enc[d], ui_enc).c_str());
              } else {
                fprintf(stdout, "*\n");
              }
              fflush(stdout);
            } else {
              int byte_offset = parser->get_tokenpos() + pos;
              int char_offset = 0;
              if (strcmp(io_enc, "UTF-8") == 0) {
                for (int i = 0; i < byte_offset; i++) {
                  if ((buf[i] & 0xc0) != 0x80)
                    char_offset++;
                }
              } else {
                char_offset = byte_offset;
              }
              std::vector<std::string> wlst =
                pMS[d]->suggest(chenc(token, io_enc, dic_enc[d]));
              if (wlst.empty()) {
                fprintf(stdout, "# %s %d", chenc(token, io_enc, ui_enc).c_str(),
                        char_offset);
              } else {
                fprintf(stdout, "& %s %u %d: ", chenc(token, io_enc, ui_enc).c_str(),
                        static_cast<unsigned int>(wlst.size()), char_offset);
                fprintf(stdout, "%s", chenc(wlst[0], dic_enc[d], ui_enc).c_str());
              }
              for (size_t j = 1; j < wlst.size(); ++j) {
                fprintf(stdout, ", %s", chenc(wlst[j], dic_enc[d], ui_enc).c_str());
              }
              fprintf(stdout, "\n");
              fflush(stdout);
            }
          }
        }
      }

      switch (filter_mode) {
        case AUTO: {
          std::string pLine = parser->get_line();
          fprintf(stdout, "%s\n", pLine.c_str());
          break;
        }

        case BADLINE: {
          if (((printgood) && (!bad)) || (!printgood && (bad)))
            fprintf(stdout, "%s\n", buf);
          break;
        }

        case PIPE:
        case NORMAL: {
          fprintf(stdout, "\n");
          fflush(stdout);
          break;
        }
      }
    }  // if
  }    // while

  if (bZippedOdf) {
    fclose(fileid);
    std::ostringstream sbuf;
    sbuf << odftmpdir << "/content.xml";
    if (remove(sbuf.str().c_str()) != 0) {
      perror("temp file delete failed");
    }
    sbuf.str("");
    sbuf << "rmdir " << odftmpdir;
    if (system(sbuf.str().c_str()) != 0) {
      perror("temp dir delete failed");
    }
  }

  delete parser;
}  // pipe_interface

#ifdef HAVE_READLINE

#ifdef HAVE_CURSES_H
static const char* rltext;

// set base text of input line
static int set_rltext() {
  if (rltext) {
    rl_insert_text(rltext);
    rltext = NULL;
    rl_startup_hook = (rl_hook_func_t*)NULL;
  }
  return 0;
}

#endif

// Readline escape
static int rl_escape(int count, int key) {
  rl_delete_text(0, rl_end);
  rl_done = 1;
  return 0;
}
#endif

#ifdef HAVE_CURSES_H
int expand_tab(std::string& dest, const std::string& in_src) {
  dest.clear();
  const char *src = in_src.c_str();
  int u8 = ((ui_enc != NULL) && (strcmp(ui_enc, "UTF-8") == 0)) ? 1 : 0;
  int chpos = 0;
  for (int j = 0; (src[j] != '\0') && (src[j] != '\r'); j++) {
    if (src[j] == '\t') {
      int end = 8 - (chpos % 8);
      for (int k = 0; k < end; k++) {
        dest.push_back(' ');
        chpos++;
      }
    } else {
      dest.push_back(src[j]);
      if (!u8 || (src[j] & 0xc0) != 0x80)
        chpos++;
    }
  }
  return chpos;
}

// UTF-8-aware version of strncpy (but output is always null terminated)
// What we should deal in is cursor position cells in a terminal emulator,
// i.e. the number of visual columns occupied like wcwidth/wcswidth does
// What we're really current doing is to deal in the number of characters,
// like mbstowcs which isn't quite correct, but close enough for western
// text in UTF-8
void strncpyu8(std::string& dest, const std::string& in_src, int begin, int n) {
  dest.clear();
  const char *src = in_src.c_str();
  if (n) {
    int u8 = ((ui_enc != NULL) && (strcmp(ui_enc, "UTF-8") == 0)) ? 1 : 0;
    for (int i = 0; i < begin + n;) {
      if (!*src)
        break;  // source is at it's end
      if (!u8 || (*src & 0xc0) != 0x80)
        i++;            // new character
      if (i > begin) {  // copy char (w/ utf-8 bytes)
        dest.push_back(*src++);
        while (u8 && (*src & 0xc0) == 0x80)
          dest.push_back(*src++);
      } else {  // skip char (w/ utf-8 bytes)
        ++src;
        while (u8 && (*src & 0xc0) == 0x80)
          ++src;
      }
    }
  }
}

// See strncpyu8 for gotchas
int strlenu8(const std::string& in_src) {
  const char *src = in_src.c_str();
  int u8 = ((ui_enc != NULL) && (strcmp(ui_enc, "UTF-8") == 0)) ? 1 : 0;
  int i = 0;
  while (*src) {
    if (!u8 || (*src & 0xc0) != 0x80)
      i++;
    ++src;
  }
  return i;
}

void dialogscreen(TextParser* parser,
                  std::string& token,
                  char* filename,
                  int forbidden,
                  std::vector<std::string>& wlst) {
  int x, y;
  getmaxyx(stdscr, y, x);
  clear();

  if (forbidden & SPELL_FORBIDDEN)
    printw(gettext("FORBIDDEN!"));
  else if (forbidden & SPELL_WARN)
    printw(gettext("Spelling mistake?"));

  printw(gettext("\t%s\t\tFile: %s\n\n"), chenc(token, io_enc, ui_enc).c_str(),
         filename);

  // handle long lines and tabulators
  std::string lines[MAXPREVLINE];
  std::string prevLine;
  for (int i = 0; i < MAXPREVLINE; i++) {
    prevLine = parser->get_prevline(i);
    expand_tab(lines[i], chenc(prevLine, io_enc, ui_enc));
  }

  prevLine = parser->get_prevline(0);
  std::string line = prevLine.substr(0, parser->get_tokenpos());
  std::string line2;
  int tokenbeg = expand_tab(line2, chenc(line, io_enc, ui_enc));

  prevLine = parser->get_prevline(0);
  line = prevLine.substr(0, parser->get_tokenpos() + token.size());
  int tokenend = expand_tab(line2, chenc(line, io_enc, ui_enc));

  int rowindex = (tokenend - 1) / x;
  int beginrow = rowindex - tokenbeg / x;
  if (beginrow >= MAXPREVLINE)
    beginrow = MAXPREVLINE - 1;

  int ri = rowindex;
  int prevline = 0;

  for (int i = 0; i < MAXPREVLINE; i++) {
    strncpyu8(line, lines[prevline], x * rowindex, x);
    mvprintw(MAXPREVLINE + 1 - i, 0, "%s", line.c_str());
    const bool finished = i == MAXPREVLINE - 1;
    if (!finished) {
      rowindex--;
      if (rowindex == -1) {
        prevline++;
        rowindex = strlenu8(lines[prevline]) / x;
      }
    }
  }

  strncpyu8(line, lines[0], x * (ri - beginrow), tokenbeg % x);
  mvprintw(MAXPREVLINE + 1 - beginrow, 0, "%s", line.c_str());
  attron(A_REVERSE);
  printw("%s", chenc(token, io_enc, ui_enc).c_str());
  attroff(A_REVERSE);

  mvprintw(MAXPREVLINE + 2, 0, "\n");
  for (size_t i = 0; i < wlst.size(); ++i) {
    if ((wlst.size() > 10) && (i < 10)) {
      printw(" 0%zu: %s\n", i, chenc(wlst[i], io_enc, ui_enc).c_str());
    } else {
      printw(" %zu: %s\n", i, chenc(wlst[i], io_enc, ui_enc).c_str());
    }
  }

  /* TRANSLATORS: the capital letters are shortcuts, mark one letter similarly
     in your translation and translate the standalone letter accordingly later
     */
  mvprintw(y - 3, 0, "%s\n", gettext("\n[SPACE] R)epl A)ccept I)nsert U)ncap "
                                     "S)tem Q)uit e(X)it or ? for help\n"));
}

std::string lower_first_char(const std::string& token, const char* ioenc, int langnum) {
  std::string utf8str = chenc(token, ioenc, "UTF-8");
  std::vector<w_char> u;
  u8_u16(u, utf8str);
  if (!u.empty()) {
    unsigned short idx = (u[0].h << 8) + u[0].l;
    idx = unicodetolower(idx, langnum);
    u[0].h = (unsigned char)(idx >> 8);
    u[0].l = (unsigned char)(idx & 0x00FF);
  }
  std::string scratch;
  u16_u8(scratch, u);
  return chenc(scratch, "UTF-8", ioenc);
}

// for terminal interface
int dialog(TextParser* parser,
           Hunspell* pMS,
           std::string& token,
           char* filename,
           std::vector<std::string>& wlst,
           int forbidden) {
  std::vector<std::string> dicwords;
  int c;

  dialogscreen(parser, token, filename, forbidden, wlst);

  char firstletter = '\0';

  while ((c = getch())) {
    switch (c) {
      case '0':
      case '1':
        if ((firstletter == '\0') && (wlst.size() > 10)) {
          firstletter = c;
          break;
        }
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        modified = 1;
        if (firstletter == '1') {
          c += 10;
        }
        c -= '0';
        if (c >= static_cast<int>(wlst.size()))
          break;
        if (checkapos) {
          std::string sbuf(wlst[c]);
          mystrrep(sbuf, "'", UTF8_APOS);
          parser->change_token(sbuf.c_str());
        } else {
          parser->change_token(wlst[c].c_str());
        }
        return 0;
      case ' ':
        return 0;
      case '?':
        clear();
        printw(gettext(
            "Whenever a word is found that is not in the dictionary\n"
            "it is printed on the first line of the screen.  If the "
            "dictionary\n"
            "contains any similar words, they are listed with a number\n"
            "next to each one.  You have the option of replacing the word\n"
            "completely, or choosing one of the suggested words.\n"));
        printw(gettext("\nCommands are:\n\n"));
        printw(gettext("R	Replace the misspelled word completely.\n"));
        printw(gettext("Space	Accept the word this time only.\n"));
        printw(
            gettext("A	Accept the word for the rest of this session.\n"));
        printw(gettext(
            "I	Accept the word, and put it in your private dictionary.\n"));
        printw(gettext(
            "U	Accept and add lowercase version to private dictionary.\n"));
        printw(
            gettext("S\tAsk a stem and a model word and store them in the "
                    "private dictionary.\n"
                    "\tThe stem will be accepted also with the affixes of the "
                    "model word.\n"));
        printw(gettext("0-n	Replace with one of the suggested words.\n"));
        printw(gettext(
            "X	Write the rest of this file, ignoring misspellings, and start "
            "next file.\n"));
        printw(
            gettext("Q	Quit immediately. Asks for confirmation. Leaves file "
                    "unchanged.\n"));
        printw(gettext("^Z	Suspend program. Restart with fg command.\n"));
        printw(gettext("?	Show this help screen.\n"));
        printw(gettext("\n-- Type space to continue -- \n"));
        while (getch() != ' ')
          ;
      // fall-through
      case 12: {
        dialogscreen(parser, token, filename, forbidden, wlst);
        break;
      }
      default: {
        /* TRANSLATORS: translate this letter according to the shortcut letter
           used
           previously in the  translation of "R)epl" before */
        if (c == (gettext("r"))[0]) {
          modified = 1;

#ifdef HAVE_READLINE
          endwin();
          rltext = "";
          if (rltext && *rltext)
            rl_startup_hook = set_rltext;
#endif
          char* temp = readline(gettext("Replace with: "));
#ifdef HAVE_READLINE
          initscr();
          cbreak();
#endif

          if ((!temp) || (temp[0] == '\0')) {
            free(temp);
            dialogscreen(parser, token, filename, forbidden, wlst);
            break;
          }

          std::string i(temp);
          free(temp);
          if (checkapos) {
            mystrrep(i, "'", UTF8_APOS);
          }
          parser->change_token(i.c_str());

          return 2;  // replace
        }
        /* TRANSLATORS: translate these letters according to the shortcut letter
           used
           previously in the  translation of "U)ncap" and I)nsert before */
        int u_key = gettext("u")[0];
        int i_key = gettext("i")[0];

        if (c == u_key || c == i_key) {
          std::string word = (c == i_key)
                      ? token
                      : lower_first_char(token, io_enc, pMS->get_langnum());
          dicwords.push_back(word);
          std::string sbuf;
          // save
          if (HOME) {
            sbuf.append(HOME);
          } else {
            fprintf(stderr, gettext("error - missing HOME variable\n"));
            break;
          }
#ifndef WIN32
          sbuf.append("/");
#endif
          size_t offset = sbuf.size();
          if (!privdicname) {
            sbuf.append(DICBASENAME);
            sbuf.append(basename(dicname, DIRSEPCH));
          } else {
            sbuf.append(privdicname);
          }
          if (save_privdic(sbuf.substr(offset), sbuf, dicwords)) {
            dicwords.clear();
          } else {
            fprintf(stderr, gettext("Cannot update personal dictionary."));
            break;
          }
        }  // no break
        /* TRANSLATORS: translate this letter according to the shortcut letter
           used
           previously in the  translation of "U)ncap" and I)nsert before */
        if ((c == (gettext("u"))[0]) || (c == (gettext("i"))[0]) ||
            (c == (gettext("a"))[0])) {
          modified = 1;
          putdic(token, pMS);
          return 0;
        }
        /* TRANSLATORS: translate this letter according to the shortcut letter
           used
           previously in the  translation of "S)tem" before */
        if (c == (gettext("s"))[0]) {
          modified = 1;

          std::string w(token);
          size_t n_last_of = w.find_last_of('-');
          if (n_last_of != std::string::npos) {
            w.resize(n_last_of);
          }

#ifdef HAVE_READLINE
          endwin();
          rltext = w.c_str();
          if (rltext && *rltext)
            rl_startup_hook = set_rltext;
#endif
          char* temp = readline(gettext("New word (stem): "));

          if ((!temp) || (temp[0] == '\0')) {
            free(temp);
#ifdef HAVE_READLINE
            initscr();
            cbreak();
#endif
            dialogscreen(parser, token, filename, forbidden, wlst);
            break;
          }

          w.assign(temp);
          free(temp);

#ifdef HAVE_READLINE
          initscr();
          cbreak();
#endif
          dialogscreen(parser, token, filename, forbidden, wlst);
          refresh();

#ifdef HAVE_READLINE
          endwin();
          rltext = "";
          if (rltext && *rltext)
            rl_startup_hook = set_rltext;
#endif
          temp = readline(gettext("Model word (a similar dictionary word): "));

#ifdef HAVE_READLINE
          initscr();
          cbreak();
#endif

          if ((!temp) || (temp[0] == '\0')) {
            free(temp);
            dialogscreen(parser, token, filename, forbidden, wlst);
            break;
          }

          std::string w2(temp);
          free(temp);

          std::string w3;
          w3.append(w);
          w3.append("/");
          w3.append(w2);

          if (!putdic(w3, pMS)) {
            dicwords.push_back(w3);

            w3.clear();
            w3.append(w);
            w3.append("-/");
            w3.append(w2);
            w3.append("-");
            if (putdic(w3, pMS)) {
              dicwords.push_back(w3);
            }
            // save
            std::string sbuf;
            if (HOME) {
              sbuf.append(HOME);
            } else {
              fprintf(stderr, gettext("error - missing HOME variable\n"));
              continue;
            }
#ifndef WIN32
            sbuf.append("/");
#endif
            size_t offset = sbuf.size();
            if (!privdicname) {
              sbuf.append(DICBASENAME);
              sbuf.append(basename(dicname, DIRSEPCH));
            } else {
              sbuf.append(privdicname);
            }
            if (save_privdic(sbuf.substr(offset), sbuf, dicwords)) {
              dicwords.clear();
            } else {
              fprintf(stderr, gettext("Cannot update personal dictionary."));
              break;
            }

          } else {
            dialogscreen(parser, token, filename, forbidden, wlst);
            printw(gettext(
                "Model word must be in the dictionary. Press any key!"));
            getch();
            dialogscreen(parser, token, filename, forbidden, wlst);
            break;
          }
          return 0;
        }
        /* TRANSLATORS: translate this letter according to the shortcut letter
           used
           previously in the  translation of "e(X)it" before */
        if (c == (gettext("x"))[0]) {
          return 1;
        }
        /* TRANSLATORS: translate this letter according to the shortcut letter
           used
           previously in the  translation of "Q)uit" before */
        if (c == (gettext("q"))[0]) {
          if (modified) {
            printw(
                gettext("Are you sure you want to throw away your changes? "));
            /* TRANSLATORS: translate this letter according to the shortcut
             * letter y)es */
            if (getch() == (gettext("y"))[0]) {
              return -1;
            }
            dialogscreen(parser, token, filename, forbidden, wlst);
            break;
          } else {
            return -1;
          }
        }
      }
    }
  }
  return 0;
}

int interactive_line(TextParser* parser,
                     Hunspell** pMS,
                     char* filename,
                     FILE* tempfile) {
  int dialogexit = 0;
  int info = 0;
  int d = 0;
  std::string token;
  while (parser->next_token(token)) {
    if (!check(pMS, &d, parser->get_word(token), &info, NULL)) {
      std::vector<std::string> wlst;
      dialogscreen(parser, token, filename, info, wlst);  // preview
      refresh();
      std::string dicbuf = chenc(parser->get_word(token), io_enc, dic_enc[d]);
      wlst = pMS[d]->suggest(mystrrep(dicbuf, ENTITY_APOS, "'").c_str());
      if (wlst.empty()) {
        dialogexit = dialog(parser, pMS[d], token, filename, wlst, info);
      } else {
        for (size_t j = 0; j < wlst.size(); ++j) {
          wlst[j] = chenc(wlst[j], dic_enc[d], io_enc);
        }
        dialogexit = dialog(parser, pMS[d], token, filename, wlst, info);
      }
    }
    if ((dialogexit == -1) || (dialogexit == 1))
      goto ki2;
  }

ki2:
  fprintf(tempfile, "%s", parser->get_line().c_str());
  return dialogexit;
}

void interactive_interface(Hunspell** pMS, char* filename, int format) {
  char buf[MAXLNLEN];
  char* odffilename = NULL;
  char* odftmpdir = NULL;  // external zip works only with temporary directories
                            // (option -j)

  FILE* text = fopen(filename, "r");
  if (!text) {
    perror(gettext("Can't open inputfile"));
    endwin();
    exit(1);
  }

  int dialogexit;
  int check = 1;

  const char* extension = basename(filename, '.');
  TextParser* parser = get_parser(format, extension, pMS[0]);
  char tmpdirtemplate[] = "/tmp/hunspellXXXXXX";

  bool bZippedOdf = is_zipped_odf(parser, extension);
  // access content.xml of ODF
  if (bZippedOdf) {
    odftmpdir = mymkdtemp(tmpdirtemplate);
    if (!odftmpdir) {
      perror(gettext("Can't create tmp dir"));
      endwin();
      exit(1);
    }
    fclose(text);
    // break 1-line XML of zipped ODT documents at </style:style> and </text:p>
    // to avoid tokenization problems (fgets could stop within an XML tag)
    std::ostringstream sbuf;
    sbuf << "unzip -p \"" << filename << "\" content.xml | sed "
            "\"s/\\(<\\/text:p>\\|<\\/style:style>\\)\\(.\\)/\\1\\n\\2/g\" "
            ">" << odftmpdir << "/content.xml";
    if (!secure_filename(filename) || system(sbuf.str().c_str()) != 0) {
      if (secure_filename(filename))
        perror(gettext("Can't open inputfile"));
      else
        fprintf(stderr, gettext("Can't open %s.\n"), filename);
      endwin();
      (void)system((std::string("rmdir ") + odftmpdir).c_str());
      exit(1);
    }
    odffilename = filename;
    std::string file(odftmpdir);
    file.append("/content.xml");
    filename = mystrdup(file.c_str());
    text = fopen(filename, "r");
    if (!text) {
      perror(gettext("Can't open inputfile"));
      endwin();
      (void)system((std::string("rmdir ") + odftmpdir).c_str());
      exit(1);
    }
  }

  FILE* tempfile = tmpfile();

  if (!tempfile) {
    perror(gettext("Can't create tempfile"));
    delete parser;
    fclose(text);
    endwin();
    exit(1);
  }

  while (fgets(buf, MAXLNLEN, text)) {
    if (check) {
      parser->put_line(buf);
      dialogexit = interactive_line(
          parser, pMS, odffilename ? odffilename : filename, tempfile);
      switch (dialogexit) {
        case -1: {
          clear();
          refresh();
          fclose(tempfile);  // automatically deleted when closed
          if (bZippedOdf) {
            if (remove(filename) != 0) {
              perror("temp file delete failed");
            }
            std::ostringstream sbuf;
            sbuf << "rmdir " << odftmpdir;
            if (system(sbuf.str().c_str()) != 0) {
              perror("temp dir delete failed");
            }
            free(filename);
          }
          endwin();
          exit(0);
        }
        case 1: {
          check = 0;
        }
      }
    } else {
      fprintf(tempfile, "%s", buf);
    }
  }
  fclose(text);

  if (modified) {
    rewind(tempfile);
    text = fopen(filename, "wb");
    if (text == NULL)
      perror(gettext("Can't open outputfile"));
    else {
      size_t n;
      while ((n = fread(buf, 1, MAXLNLEN, tempfile)) > 0) {
        if (fwrite(buf, 1, n, text) != n)
          perror("write failed");
      }
      fclose(text);
      if (bZippedOdf && odffilename) {
        std::ostringstream sbuf;
        sbuf << "zip -j '" << odffilename << "' " << filename;
        if (system(sbuf.str().c_str()) != 0)
          perror("write failed");
      }
    }
  }

  if (bZippedOdf) {
    if (remove(filename) != 0) {
      perror("temp file delete failed");
    }
    std::ostringstream sbuf;
    sbuf << "rmdir " << odftmpdir;
    if (system(sbuf.str().c_str()) != 0) {
      perror("temp dir delete failed");
    }
    free(filename);
  }

  delete parser;
  fclose(tempfile);  // automatically deleted when closed
}

#endif

char* exist2(char* dir, int len, const char* name, const char* ext) {
  std::string buf;
  const char* sep = (len == 0) ? "" : DIRSEP;
  buf.assign(dir, len);
  buf.append(sep);
  buf.append(name);
  buf.append(ext);
  if (exist(buf.c_str()))
    return mystrdup(buf.c_str());
  buf.append(HZIP_EXTENSION);
  if (exist(buf.c_str())) {
    buf.erase(buf.size() - strlen(HZIP_EXTENSION));
    return mystrdup(buf.c_str());
  }
  return NULL;
}

#if !defined(WIN32) || defined(__MINGW32__)
int listdicpath(char* dir, int len) {
  std::string buf;
  const char* sep = (len == 0) ? "" : DIRSEP;
  buf.assign(dir, len);
  buf.append(sep);
  DIR* d = opendir(buf.c_str());
  if (!d)
    return 0;
  struct dirent* de;
  while ((de = readdir(d))) {
    len = strlen(de->d_name);
    if ((len > 4 && strcmp(de->d_name + len - 4, ".dic") == 0) ||
        (len > 7 && strcmp(de->d_name + len - 7, ".dic.hz") == 0)) {
      char* s = mystrdup(de->d_name);
      s[len - ((s[len - 1] == 'z') ? 7 : 4)] = '\0';
      fprintf(stderr, "%s%s\n", buf.c_str(), s);
      free(s);
    }
  }
  closedir(d);
  return 1;
}
#endif

// search existing path for file "name + ext"
char* search(char* begin, char* name, const char* ext) {
  char* end = begin;
  while (1) {
    while (!((*end == *PATHSEP) || (*end == '\0')))
      end++;
    char* res = NULL;
    if (name) {
      res = exist2(begin, int(end - begin), name, ext);
    } else {
#if !defined(WIN32) || defined(__MINGW32__)
      listdicpath(begin, end - begin);
#endif
    }
    if ((*end == '\0') || res)
      return res;
    end++;
    begin = end;
  }
}

int main(int argc, char** argv) {
  std::string buf;
  Hunspell* pMS[DMAX];
  char* key = NULL;
  int arg_files = -1;  // first filename argumentum position in argv
  int format = FMT_TEXT;
  int argstate = 0;

#ifdef HAVE_LOCALE_H
  setlocale(LC_ALL, "");
#endif
#ifdef HAVE_LANGINFO_H
  ui_enc = nl_langinfo(CODESET);
#endif
  textdomain("hunspell"); //for gettext

#ifdef HAVE_READLINE
  rl_set_key("\x1b\x1b", rl_escape, rl_get_keymap());
  rl_bind_key('\t', rl_insert);
#endif

#ifdef LOG
  log("START");
#endif

  for (int i = 1; i < argc; i++) {
#ifdef LOG
    log(argv[i]);
#endif

    if (argstate == 1) {
      if (dicname)
        free(dicname);
      dicname = mystrdup(argv[i]);
      argstate = 0;
    } else if (argstate == 2) {
      if (privdicname)
        free(privdicname);
      privdicname = mystrdup(argv[i]);
      argstate = 0;
    } else if (argstate == 3) {
      io_enc = argv[i];
      argstate = 0;
    } else if (argstate == 4) {
      key = argv[i];
      argstate = 0;
    } else if (strcmp(argv[i], "-d") == 0)
      argstate = 1;
    else if (strcmp(argv[i], "-p") == 0)
      argstate = 2;
    else if (strcmp(argv[i], "-i") == 0)
      argstate = 3;
    else if (strcmp(argv[i], "-P") == 0)
      argstate = 4;
    else if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
      fprintf(stderr, "%s", gettext("Usage: hunspell [OPTION]... [FILE]...\n"));
      fprintf(stderr, "%s", gettext("Check spelling of each FILE. Without FILE, "
                              "check standard input.\n\n"));
      fprintf(stderr, "%s", gettext("  -1\t\tcheck only first field in lines "
                              "(delimiter = tabulator)\n"));
      fprintf(stderr, "%s", gettext("  -a\t\tIspell's pipe interface\n"));
      fprintf(stderr, "%s", gettext("  --check-url\tcheck URLs, e-mail addresses and "
                              "directory paths\n"));
      fprintf(
          stderr, "%s",
          gettext(
              "  --check-apostrophe\tcheck Unicode typographic apostrophe\n"));
      fprintf(stderr, "%s",
              gettext("  -d d[,d2,...]\tuse d (d2 etc.) dictionaries\n"));
      fprintf(stderr, "%s", gettext("  -D\t\tshow available dictionaries\n"));
      fprintf(stderr, "%s", gettext("  -G\t\tprint only correct words or lines\n"));
      fprintf(stderr, "%s", gettext("  -h, --help\tdisplay this help and exit\n"));
      fprintf(stderr, "%s", gettext("  -H\t\tHTML input file format\n"));
      fprintf(stderr, "%s", gettext("  -i enc\tinput encoding\n"));
      fprintf(stderr, "%s", gettext("  -l\t\tprint misspelled words\n"));
      fprintf(stderr, "%s", gettext("  -L\t\tprint lines with misspelled words\n"));
      fprintf(stderr, "%s",
              gettext("  -m \t\tanalyze the words of the input text\n"));
      fprintf(stderr, "%s", gettext("  -n\t\tnroff/troff input file format\n"));
      fprintf(
          stderr, "%s",
          gettext(
              "  -O\t\tOpenDocument (ODF or Flat ODF) input file format\n"));
      fprintf(stderr, "%s", gettext("  -p dict\tset dict custom dictionary\n"));
      fprintf(stderr, "%s",
              gettext("  -r\t\twarn of the potential mistakes (rare words)\n"));
      fprintf(
          stderr, "%s",
          gettext("  -P password\tset password for encrypted dictionaries\n"));
      fprintf(stderr, "%s", gettext("  -s \t\tstem the words of the input text\n"));
      fprintf(stderr, "%s", gettext("  -S \t\tsuffix words of the input text\n"));
      fprintf(stderr, "%s", gettext("  -t\t\tTeX/LaTeX input file format\n"));
      fprintf(stderr, "%s", gettext("  -v, --version\tprint version number\n"));
      fprintf(stderr, "%s",
              gettext("  -vv\t\tprint Ispell compatible version number\n"));
      fprintf(stderr, "%s", gettext("  -w\t\tprint misspelled words (= lines) from "
                              "one word/line input.\n"));
      fprintf(stderr, "%s", gettext("  -X\t\tXML input file format\n\n"));
      fprintf(
          stderr, "%s",
          gettext(
              "Example: hunspell -d en_US file.txt    # interactive spelling\n"
              "         hunspell -i utf-8 file.txt    # check UTF-8 encoded "
              "file\n"
              "         hunspell -l *.odt             # print misspelled words "
              "of ODF files\n\n"
              "         # Quick fix of ODF documents by personal dictionary "
              "creation\n\n"
              "         # 1 Make a reduced list from misspelled and unknown "
              "words:\n\n"
              "         hunspell -l *.odt | sort | uniq >words\n\n"
              "         # 2 Delete misspelled words of the file by a text "
              "editor.\n"
              "         # 3 Use this personal dictionary to fix the deleted "
              "words:\n\n"
              "         hunspell -p words *.odt\n\n"));
      fprintf(stderr, "%s", gettext("Bug reports: http://hunspell.github.io/\n"));
      exit(0);
    } else if ((strcmp(argv[i], "-vv") == 0) || (strcmp(argv[i], "-v") == 0) ||
               (strcmp(argv[i], "--version") == 0)) {
      fprintf(stdout, "%s", gettext(HUNSPELL_PIPE_HEADING));
      fprintf(stdout, "\n");
      if (strcmp(argv[i], "-vv") != 0) {
        fprintf(stdout, "%s",
                gettext("\nCopyright (C) 2002-2022 L\303\241szl\303\263 "
                        "N\303\251meth. License: MPL/GPL/LGPL.\n\n"
                        "Based on OpenOffice.org's Myspell library.\n"
                        "Myspell's copyright (C) Kevin Hendricks, 2001-2002, "
                        "License: BSD.\n\n"));
        fprintf(stdout, "%s", gettext("This is free software; see the source for "
                                "copying conditions.  There is NO\n"
                                "warranty; not even for MERCHANTABILITY or "
                                "FITNESS FOR A PARTICULAR PURPOSE,\n"
                                "to the extent permitted by law.\n"));
      }
      exit(0);
    } else if ((strcmp(argv[i], "-a") == 0)) {
      filter_mode = PIPE;
    } else if ((strcmp(argv[i], "-m") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell:   Make possible root/affix combinations that aren't in the
       dictionary.
       hunspell: Analyze the words of the input text
      */
      if (filter_mode != PIPE)
        filter_mode = ANALYZE;
    } else if ((strcmp(argv[i], "-s") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell:   Stop itself with a SIGTSTP signal after each line of input.
       hunspell: Stem the words of the input text
      */
      if (filter_mode != PIPE)
        filter_mode = STEM;
    } else if ((strcmp(argv[i], "-S") == 0)) {
      if (filter_mode != PIPE)
        filter_mode = SUFFIX;
    } else if ((strcmp(argv[i], "-t") == 0)) {
      format = FMT_LATEX;
    } else if ((strcmp(argv[i], "-n") == 0)) {
      format = FMT_MAN;
    } else if ((strcmp(argv[i], "-H") == 0)) {
      format = FMT_HTML;
    } else if ((strcmp(argv[i], "-X") == 0)) {
      format = FMT_XML;
    } else if ((strcmp(argv[i], "-O") == 0)) {
      format = FMT_ODF;
    } else if ((strcmp(argv[i], "-l") == 0)) {
      filter_mode = BADWORD;
    } else if ((strcmp(argv[i], "-w") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell:   Specify additional characters that can be part of a word.
       hunspell: Print misspelled words (= lines) from one word/line input
      */
      if (filter_mode != PIPE)
        filter_mode = WORDFILTER;
    } else if ((strcmp(argv[i], "-L") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell:   Number of lines of context to be shown at the bottom of the
       screen
       hunspell: Print lines with misspelled words
      */
      if (filter_mode != PIPE)
        filter_mode = BADLINE;
    } else if ((strcmp(argv[i], "-u") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell: None
       hunspell: Show typical misspellings
      */
      if (filter_mode != PIPE)
        filter_mode = AUTO0;
    } else if ((strcmp(argv[i], "-U") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell: None
       hunspell: Automatic correction of typical misspellings to stdout
      */
      if (filter_mode != PIPE)
        filter_mode = AUTO;
    } else if ((strcmp(argv[i], "-u2") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell: None
       hunspell: Print typical misspellings in sed format
      */
      if (filter_mode != PIPE)
        filter_mode = AUTO2;
    } else if ((strcmp(argv[i], "-u3") == 0)) {
      /*
       if -a was used, don't override, i.e. keep ispell compatability
       ispell: None
       hunspell: Print typical misspellings in gcc error format
      */
      if (filter_mode != PIPE)
        filter_mode = AUTO3;
    } else if ((strcmp(argv[i], "-G") == 0)) {
      printgood = 1;
    } else if ((strcmp(argv[i], "-1") == 0)) {
      format = FMT_FIRST;
    } else if ((strcmp(argv[i], "-D") == 0)) {
      showpath = 1;
    } else if ((strcmp(argv[i], "-r") == 0)) {
      warn = 1;
    } else if ((strcmp(argv[i], "--check-url") == 0)) {
      checkurl = 1;
    } else if ((strcmp(argv[i], "--check-apostrophe") == 0)) {
      checkapos = 1;
    } else if ((arg_files == -1) &&
               ((argv[i][0] != '-') && (argv[i][0] != '\0'))) {
      arg_files = i;
      if (!exist(argv[i])) {  // first check (before time-consuming dic. load)
        fprintf(stderr, gettext("Can't open %s.\n"), argv[i]);
#ifdef HAVE_CURSES_H
        endwin();
#endif
        exit(1);
      }
    }
  }

  multiple_files = (arg_files > 0) && (argc - arg_files > 1);

  if (printgood && (filter_mode == NORMAL))
    filter_mode = BADWORD;

  if (!dicname) {
    if (!(dicname = getenv("DICTIONARY"))) {
      /*
       * Search in order of LC_ALL, LC_MESSAGES &
       * LANG
      */
      const char* tests[] = {"LC_ALL", "LC_MESSAGES", "LANG"};
      for (size_t i = 0; i < sizeof(tests) / sizeof(const char*); ++i) {
        if ((dicname = getenv(tests[i])) && strcmp(dicname, "") != 0) {
          dicname = mystrdup(dicname);
          char* dot = strchr(dicname, '.');
          if (dot)
            *dot = '\0';
          char* at = strchr(dicname, '@');
          if (at)
            *at = '\0';
          break;
        }
      }

      if (dicname &&
          ((strcmp(dicname, "C") == 0) || (strcmp(dicname, "POSIX") == 0))) {
        free(dicname);
        dicname = mystrdup("en_US");
      }

      if (!dicname) {
        dicname = mystrdup(DEFAULTDICNAME);
      }
    } else {
      dicname = mystrdup(dicname);
    }
  }

  {
    std::string path_std_str = ".";
    path_std_str.append(PATHSEP); // <- check path in local directory
    path_std_str.append(PATHSEP); // <- check path in root directory
    if (getenv("DICPATH")) {
      path_std_str.append(getenv("DICPATH")).append(PATHSEP);
    }
    path_std_str.append(LIBDIR).append(PATHSEP);
    if (HOME) {
      const char * userooodir[] = USEROOODIR;
      for(size_t i = 0; i < sizeof(userooodir)/sizeof(userooodir[0]); ++i) {
        path_std_str += HOME;
#ifndef _WIN32
        path_std_str += DIRSEP;
#endif
        path_std_str.append(userooodir[i]).append(PATHSEP);
      }
      path_std_str.append(OOODIR);
    }
    path = mystrdup(path_std_str.c_str());
  }

  if (showpath) {
    fprintf(stderr, gettext("SEARCH PATH:\n%s\n"), path);
    fprintf(
        stderr, "%s",
        gettext(
            "AVAILABLE DICTIONARIES (path is not mandatory for -d option):\n"));
    search(path, NULL, NULL);
  }

  if (!privdicname)
    privdicname = mystrdup(getenv("WORDLIST"));

  char* dicplus = strchr(dicname, ',');
  if (dicplus)
    *dicplus = '\0';
  char* aff = search(path, dicname, ".aff");
  char* dic = search(path, dicname, ".dic");
  if (aff && dic) {
    if (showpath) {
      fprintf(stderr, gettext("LOADED DICTIONARY:\n%s\n%s\n"), aff, dic);
    }
    pMS[0] = new Hunspell(aff, dic, key);
    dic_enc[0] = pMS[0]->get_dict_encoding().c_str();
    dmax = 1;
    while (dicplus) {
      char* dicname2 = dicplus + 1;
      dicplus = strchr(dicname2, ',');
      if (dicplus)
        *dicplus = '\0';
      free(aff);
      free(dic);
      aff = search(path, dicname2, ".aff");
      dic = search(path, dicname2, ".dic");
      if (aff && dic) {
        if (dmax < DMAX) {
          pMS[dmax] = new Hunspell(aff, dic, key);
          dic_enc[dmax] = pMS[dmax]->get_dict_encoding().c_str();
          dmax++;
          if (showpath) {
            fprintf(stderr, gettext("LOADED DICTIONARY:\n%s\n%s\n"), aff, dic);
          }
        } else
          fprintf(stderr, gettext("error - %s exceeds dictionary limit.\n"),
                  dicname2);
      } else if (dic)
        pMS[dmax - 1]->add_dic(dic);
    }
  } else {
    fprintf(stderr, gettext("Can't open affix or dictionary files for "
                            "dictionary named \"%s\".\n"),
            dicname);
    exit(1);
  }

  if (showpath && -1 == arg_files) {
      exit(0);
  }

  /* open the private dictionaries */
  if (HOME) {
    buf.assign(HOME);
#ifndef WIN32
    buf.append("/");
#endif
    buf.append(DICBASENAME);
    buf.append(basename(dicname, DIRSEPCH));
    load_privdic(buf.c_str(), pMS[0]);
    buf.assign(HOME);
#ifndef WIN32
    buf.append("/");
#endif
    if (!privdicname) {
      buf.assign(DICBASENAME);
      buf.append(basename(dicname, DIRSEPCH));
      load_privdic(buf.c_str(), pMS[0]);
    } else {
      buf.append(privdicname);
      load_privdic(buf.c_str(), pMS[0]);
      buf.assign(privdicname);
      load_privdic(buf.c_str(), pMS[0]);
    }
  }

  /*
     If in pipe mode, output pipe mode version string only when
     hunspell has properly been started.
     Emacs and may be others relies in the English version format.
     Do not gettextize.
  */
  if (filter_mode == PIPE) {
    fprintf(stdout, HUNSPELL_PIPE_HEADING);
    fflush(stdout);
  }

  if (arg_files == -1) {
    pipe_interface(pMS, format, stdin, NULL);
  } else if (filter_mode != NORMAL) {
    for (int i = arg_files; i < argc; i++) {
      if (exist(argv[i])) {
        modified = 0;
        currentfilename = argv[i];
        FILE* f = fopen(argv[i], "r");
        pipe_interface(pMS, format, f, argv[i]);
        fclose(f);
      } else {
        fprintf(stderr, gettext("Can't open %s.\n"), argv[i]);
        exit(1);
      }
    }
  } else /*filter_mode == NORMAL*/ {
#ifdef HAVE_CURSES_H
    initscr();
    cbreak();
    noecho();
    nonl();
    intrflush(stdscr, FALSE);

    for (int i = arg_files; i < argc; i++) {
      if (exist(argv[i])) {
        modified = 0;
        interactive_interface(pMS, argv[i], format);
      } else {
        fprintf(stderr, gettext("Can't open %s.\n"), argv[i]);
        endwin();
        exit(1);
      }
    }

    clear();
    refresh();
    endwin();
#else
    fprintf(
        stderr, "%s",
        gettext(
            "Hunspell has been compiled without Ncurses user interface.\n"));
#endif
  }

  if (dicname)
    free(dicname);
  if (privdicname)
    free(privdicname);
  if (path)
    free(path);
  if (aff)
    free(aff);
  if (dic)
    free(dic);
  for (int i = 0; i < dmax; i++)
    delete pMS[i];
  return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
