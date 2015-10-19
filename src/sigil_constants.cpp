#include <QString>
#include <QStringList>
#include "sigil_constants.h"

#if _WIN32
const QString PATH_LIST_DELIM = ";";
const QString PYTHON_MAIN_PATH = "/Python3";
const QStringList PYTHON_SYS_PATHS = QStringList() << "/Lib" << "/DLLs" << "/Lib/site-packages";
#endif

#if __APPLE__
const QString PATH_LIST_DELIM = ":";
const QString PYTHON_MAIN_PATH = "/Frameworks/Python.framework/Versions/3.4/lib/python3.4";
const QStringList PYTHON_SYS_PATHS = QStringList () << "/plat-darwin" << "/lib-dynload" << "/site-packages";
#endif

#if !defined(_WIN32) && !defined(__APPLE__)
const QString PATH_LIST_DELIM = ":";
// Runtime env var override of Sigil's 'share/sigil' directory
const QString sigil_extra_root = QString(getenv("SIGIL_EXTRA_ROOT"));
// Standard build-time location of Sigil's 'share/sigil' directory and the hunspell dictionaries location.
// Set in src/CMakeLists.txt with the lines:
// set_source_files_properties( sigil_constants.cpp PROPERTIES COMPILE_DEFINITIONS
//         SIGIL_SHARE_ROOT="${SIGIL_SHARE_ROOT}" HUNSPELL_DICT_DIR="${HUNSPELL_DICT_DIR}" )
const QString sigil_share_root = QString(SIGIL_SHARE_ROOT);
const QString hunspell_dict_dir = QString(HUNSPELL_DICT_DIR);
const QString PYTHON_MAIN_PATH = "/python3/lib/python3.4";
#if __x86_64__ || __ppc64__
const QStringList PYTHON_SYS_PATHS = QStringList () << "/plat-x86_64-linux-gnu" << "/plat-linux" << "/lib-dynload" << "/site-packages";
#else
const QStringList PYTHON_SYS_PATHS = QStringList () << "/plat-i386-linux-gnu" << "/plat-linux" << "/lib-dynload" << "/site-packages";
#endif
#endif
