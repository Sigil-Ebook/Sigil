# Try to find the XercesExtensions lib
# Once done this will define:
#
#  XERCESEXTENSIONS_FOUND - system has XercesExtensions
#  XERCESEXTENSIONS_INCLUDE_DIRS - the XercesExtensions include directory
#  XERCESEXTENSIONS_LIBRARIES - The libraries needed to use XercesExtensions
#
# Copyright (c) 2011, Dan Horák <dan[at]danny.cz>
# Copyright (c) 2012, Hans de Goede <hdegoede@redhat.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (XERCESEXTENSIONS_INCLUDE_DIRS)
  # Already in cache, be silent
  set(XERCESEXTENSIONS_FIND_QUIETLY TRUE)
endif (XERCESEXTENSIONS_INCLUDE_DIRS)

#FIND_PATH(XERCESEXTENSIONS_INCLUDE_DIRS XercesInit.h)

if( NOT XERCESEXTENSIONS_INCLUDE_DIRS )
    find_path(XERCESEXTENSIONS_INCLUDE_DIRS XercesInit.h PATH_SUFFIXES XercesExtensions)
    #now XercesInit.h was inside a XercesExtensions subdirectory so we need to
    #add that to the include dir
    set(XERCESEXTENSIONS_INCLUDE_DIRS ${XERCESEXTENSIONS_INCLUDE_DIRS}/XercesExtensions CACHE PATH "XercesExtensions include directory")
endif( NOT XERCESEXTENSIONS_INCLUDE_DIRS )


FIND_LIBRARY(XERCESEXTENSIONS_LIBRARIES NAMES XercesExtensions)

if (XERCESEXTENSIONS_INCLUDE_DIRS AND XERCESEXTENSIONS_LIBRARIES)
   set(XERCESEXTENSIONS_FOUND TRUE)
endif (XERCESEXTENSIONS_INCLUDE_DIRS AND XERCESEXTENSIONS_LIBRARIES)


if (XERCESEXTENSIONS_FOUND)
   if (NOT XERCESEXTENSIONS_FIND_QUIETLY)
      message(STATUS "Found XercesExtensions: ${XERCESEXTENSIONS_LIBRARIES}")
   endif (NOT XERCESEXTENSIONS_FIND_QUIETLY)
else (XERCESEXTENSIONS_FOUND)
    if (XERCESEXTENSIONS_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find XercesExtensions")
    else (XERCESEXTENSIONS_FIND_REQUIRED)
      message(STATUS "Could NOT find XercesExtensions")
    endif (XERCESEXTENSIONS_FIND_REQUIRED)
endif (XERCESEXTENSIONS_FOUND)

MARK_AS_ADVANCED(XERCESEXTENSIONS_INCLUDE_DIRS XERCESEXTENSIONS_LIBRARIES)
