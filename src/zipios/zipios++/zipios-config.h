#ifndef ZIPIOS_CONFIG_GEN_H
#define ZIPIOS_CONFIG_GEN_H

#if defined(_MSC_VER)
#   include	"zipios-config.w32.h"
#else
#   include	"zipios-config.unix.h"
#endif

#endif // ZIPIOS_CONFIG_GEN_H