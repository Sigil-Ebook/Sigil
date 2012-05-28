# Try to find the MiniZip lib
# Once done this will define:
#
#  MINIZIP_FOUND - system has MiniZip
#  MINIZIP_INCLUDE_DIRS - the MiniZip include directory
#  MINIZIP_LIBRARIES - The libraries needed to use MiniZip 
#
# Copyright (c) 2012, John Schember <john@nachtimwald.com>
# Copyright (c) 2011, Dan Horák <dan[at]danny.cz>
# Copyright (c) 2012, Hans de Goede <hdegoede@redhat.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (MINIZIP_INCLUDE_DIRS)
  # Already in cache, be silent
  set(MINIZIP_FIND_QUIETLY TRUE)
endif (MINIZIP_INCLUDE_DIRS)

if (NOT MINIZIP_INCLUDE_DIRS)
    find_path(MINIZIP_INCLUDE_DIRS NAMES unzip.h zip.h PATH_SUFFIXES minizip)
    set(MINIZIP_INCLUDE_DIRS ${MINIZIP_INCLUDE_DIRS}/minizip CACHE PATH "MiniZip include directory")
endif (NOT MINIZIP_INCLUDE_DIRS)

FIND_LIBRARY(MINIZIP_LIBRARIES NAMES minizip)

if (MINIZIP_INCLUDE_DIRS AND MINIZIP_LIBRARIES)
   set(MINIZIP_FOUND TRUE)
endif (MINIZIP_INCLUDE_DIRS AND MINIZIP_LIBRARIES)

if (MINIZIP_FOUND)
   if (NOT MINIZIP_FIND_QUIETLY)
      message(STATUS "Found MiniZip: ${MINIZIP_LIBRARIES}")
   endif (NOT MINIZIP_FIND_QUIETLY)
else (MINIZIP_FOUND)
    if (MINIZIP_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find MiniZip")
    else (MINIZIP_FIND_REQUIRED)
      message(STATUS "Could NOT find MiniZip")
    endif (MINIZIP_FIND_REQUIRED)
endif (MINIZIP_FOUND)

MARK_AS_ADVANCED(MINIZIP_INCLUDE_DIRS MINIZIP_LIBRARIES)

