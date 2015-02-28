#include <QString>
#include "sigil_constants.h"

#if !defined(_WIN32) && !defined(__APPLE__)
sigil_extra_root = QString(getenv("SIGIL_EXTRA_ROOT"));
#endif
