# Try to find the FlightCrew lib
# Once done this will define:
#
#  FLIGHTCREW_FOUND - system has FlightCrew
#  FLIGHTCREW_INCLUDE_DIRS - the FlightCrew include directory
#  FLIGHTCREW_LIBRARIES - The libraries needed to use FlightCrew
#
# Copyright (c) 2011, Dan Horák <dan[at]danny.cz>
# Copyright (c) 2012, Hans de Goede <hdegoede@redhat.com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.


if (FLIGHTCREW_INCLUDE_DIRS)
  # Already in cache, be silent
  set(FLIGHTCREW_FIND_QUIETLY TRUE)
endif (FLIGHTCREW_INCLUDE_DIRS)

#FIND_PATH(FLIGHTCREW_INCLUDE_DIRS flightcrew.h)

if( NOT FLIGHTCREW_INCLUDE_DIRS )
    find_path(FLIGHTCREW_INCLUDE_DIRS flightcrew.h PATH_SUFFIXES FlightCrew)
    #now flightcrew.h was inside a FlightCrew subdirectory so we need to
    #add that to the include dir
    set(FLIGHTCREW_INCLUDE_DIRS ${FLIGHTCREW_INCLUDE_DIRS}/FlightCrew CACHE PATH "FlightCrew include directory")
endif( NOT FLIGHTCREW_INCLUDE_DIRS )


FIND_LIBRARY(FLIGHTCREW_LIBRARIES NAMES FlightCrew)

if (FLIGHTCREW_INCLUDE_DIRS AND FLIGHTCREW_LIBRARIES)
   set(FLIGHTCREW_FOUND TRUE)
endif (FLIGHTCREW_INCLUDE_DIRS AND FLIGHTCREW_LIBRARIES)


if (FLIGHTCREW_FOUND)
   if (NOT FLIGHTCREW_FIND_QUIETLY)
      message(STATUS "Found FlightCrew: ${FLIGHTCREW_LIBRARIES}")
   endif (NOT FLIGHTCREW_FIND_QUIETLY)
else (FLIGHTCREW_FOUND)
    if (FLIGHTCREW_FIND_REQUIRED)
      message(FATAL_ERROR "Could NOT find FlightCrew")
    else (FLIGHTCREW_FIND_REQUIRED)
      message(STATUS "Could NOT find FlightCrew")
    endif (FLIGHTCREW_FIND_REQUIRED)
endif (FLIGHTCREW_FOUND)

MARK_AS_ADVANCED(FLIGHTCREW_INCLUDE_DIRS FLIGHTCREW_LIBRARIES)
