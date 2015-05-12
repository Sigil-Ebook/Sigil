#include <QString>
#include "sigil_constants.h"

#if !defined(_WIN32) && !defined(__APPLE__)
const QString sigil_extra_root = QString(getenv("SIGIL_EXTRA_ROOT"));
const QString sigil_share_root = QString(SIGIL_SHARE_ROOT);
#endif
