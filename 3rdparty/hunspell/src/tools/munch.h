/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
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
 * The Original Code is Hunspell, based on MySpell.
 *
 * The Initial Developers of the Original Code are
 * Kevin Hendricks (MySpell) and Németh László (Hunspell).
 * Portions created by the Initial Developers are Copyright (C) 2002-2005
 * the Initial Developers. All Rights Reserved.
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

/* munch header file */

#define MAX_LN_LEN 200
#define MAX_WD_LEN 200
#define MAX_PREFIXES 2048
#define MAX_SUFFIXES 2048
#define MAX_ROOTS 20
#define MAX_WORDS 5000

#define ROTATE_LEN 5

#define ROTATE(v, q) \
  (v) = ((v) << (q)) | (((v) >> (32 - q)) & ((1 << (q)) - 1));

#define SET_SIZE 256

#define XPRODUCT (1 << 0)

/* the affix table entry */

struct affent {
  char* appnd;
  char* strip;
  short appndl;
  short stripl;
  char achar;
  char xpflg;
  short numconds;
  char conds[SET_SIZE];
};

struct affixptr {
  struct affent* aep;
  int num;
};

/* the prefix and suffix table */
int numpfx; /* Number of prefixes in table */
int numsfx; /* Number of suffixes in table */

/* the prefix table */
struct affixptr ptable[MAX_PREFIXES];

/* the suffix table */
struct affixptr stable[MAX_SUFFIXES];

/* data structure to store results of lookups */
struct matches {
  struct hentry* hashent; /* hash table entry */
  struct affent* prefix;  /* Prefix used, or NULL */
  struct affent* suffix;  /* Suffix used, or NULL */
};

int numroots;                    /* number of root words found */
struct matches roots[MAX_ROOTS]; /* list of root words found */

/* hashing stuff */

struct hentry {
  char* word;
  char* affstr;
  struct hentry* next;
  int keep;
};

int tablesize;
struct hentry* tableptr;

/* unmunch stuff */

int numwords; /* number of words found */
struct dwords {
  char* word;
  int pallow;
};

struct dwords wlist[MAX_WORDS]; /* list words found */

/* the routines */

int parse_aff_file(FILE* afflst);

void encodeit(struct affent* ptr, char* cs);

int load_tables(FILE* wrdlst);

int hash(const char*);

int add_word(char*);

struct hentry* lookup(const char*);

void aff_chk(const char* word, int len);

void pfx_chk(const char* word, int len, struct affent* ep, int num);

void suf_chk(const char* word,
             int len,
             struct affent* ep,
             int num,
             struct affent* pfxent,
             int cpflag);

void add_affix_char(struct hentry* hent, char ac);

int expand_rootword(const char*, int, const char*);

void pfx_add(const char* word, int len, struct affent* ep, int num);

void suf_add(const char* word, int len, struct affent* ep, int num);

char* mystrsep(char** stringp, const char delim);

char* mystrdup(const char* s);

void mychomp(char* s);
