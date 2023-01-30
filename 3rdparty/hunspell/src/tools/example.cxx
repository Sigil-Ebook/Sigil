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
#include <fstream>

#include "../hunspell/hunspell.hxx"

using namespace std;

int main(int argc, char** argv) {

  /* first parse the command line options */

  if (argc < 4) {
    fprintf(stderr, "example (now it works with more dictionary files):\n");
    fprintf(stderr,
            "example affix_file dictionary_file(s) file_of_words_to_check\n");
    exit(1);
  }

  /* open the words to check list */
  std::ifstream wtclst(argv[argc - 1], std::ios_base::in);
  if (!wtclst.is_open()) {
    fprintf(stderr, "Error - could not open file of words to check\n");
    exit(1);
  }

  Hunspell* pMS = new Hunspell(argv[1], argv[2]);

  // load extra dictionaries
  if (argc > 4)
    for (int k = 3; k < argc - 1; ++k)
      pMS->add_dic(argv[k]);

  std::string buf;
  while (std::getline(wtclst, buf)) {
    int dp = pMS->spell(buf);
    if (dp) {
      fprintf(stdout, "\"%s\" is okay\n", buf.c_str());
      fprintf(stdout, "\n");
    } else {
      fprintf(stdout, "\"%s\" is incorrect!\n", buf.c_str());
      fprintf(stdout, "   suggestions:\n");
      std::vector<std::string> wlst = pMS->suggest(buf.c_str());
      for (size_t i = 0; i < wlst.size(); ++i) {
        fprintf(stdout, "    ...\"%s\"\n", wlst[i].c_str());
      }
      fprintf(stdout, "\n");
    }
    // for the same of testing this code path
    // do an analysis here and throw away the results
    pMS->analyze(buf);
  }

  delete pMS;
  return 0;
}
