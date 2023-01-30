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

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "../hunspell/hunspell.hxx"

#ifndef WIN32
using namespace std;
#endif

int main(int, char** argv) {
  /* first parse the command line options */

  for (int i = 1; i < 3; ++i)
    if (!argv[i]) {
      fprintf(stderr, "correct syntax is:\nanalyze affix_file");
      fprintf(stderr, " dictionary_file file_of_words_to_check\n");
      fprintf(stderr, "use two words per line for morphological generation\n");
      exit(1);
    }

  /* open the words to check list */

  FILE* wtclst = fopen(argv[3], "r");
  if (!wtclst) {
    fprintf(stderr, "Error - could not open file to check\n");
    exit(1);
  }

  Hunspell* pMS = new Hunspell(argv[1], argv[2]);
  char buf[100];
  while (fgets(buf, sizeof(buf), wtclst)) {
    buf[strcspn(buf, "\n")] = 0;
    if (*buf == '\0')
      continue;
    // morphgen demo
    char* s = strchr(buf, ' ');
    if (s) {
      *s = '\0';
      std::vector<std::string> result = pMS->generate(buf, s + 1);
      for (size_t i = 0; i < result.size(); ++i) {
        fprintf(stdout, "generate(%s, %s) = %s\n", buf, s + 1, result[i].c_str());
      }
      if (result.empty())
        fprintf(stdout, "generate(%s, %s) = NO DATA\n", buf, s + 1);
    } else {
      int dp = pMS->spell(std::string(buf));
      fprintf(stdout, "> %s\n", buf);
      if (dp) {
        std::vector<std::string> result = pMS->analyze(buf);
        for (size_t i = 0; i < result.size(); ++i) {
          fprintf(stdout, "analyze(%s) = %s\n", buf, result[i].c_str());
        }
        result = pMS->stem(buf);
        for (size_t i = 0; i < result.size(); ++i) {
          fprintf(stdout, "stem(%s) = %s\n", buf, result[i].c_str());
        }
      } else {
        fprintf(stdout, "Unknown word.\n");
      }
    }
  }
  delete pMS;
  fclose(wtclst);
  return 0;
}
