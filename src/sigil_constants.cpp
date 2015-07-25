#include <QString>
#include <QStringList>
#include "sigil_constants.h"

#if _WIN32
const QString sep = ";";
const QString python_sys_path = "\python";
const QStringList python_src_paths{"\LIB", "\DLLS", "\site-packages"};
#endif

#if !defined(_WIN32) && !defined(__APPLE__)
const QString sigil_extra_root = QString(getenv("SIGIL_EXTRA_ROOT"));
const QString sigil_share_root = QString(SIGIL_SHARE_ROOT);
const QString python_sys_path = "/python/lib/python3.4";
const QString sep = ":";
#if __x86_64__ || __ppc64__
const QStringList python_src_paths{"/plat-x86_64-linux-gnu", "/plat-linux", "/lib-dynload", "site-packages"};
#else
const QStringList python_src_paths{"/plat-i386-linux-gnu", "/plat-linux", "/lib-dynload", "site-packages"};
#endif
#endif
