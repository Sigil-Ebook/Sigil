########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 2.8 )

PROJECT(pcre)

# Configuration checks

INCLUDE(CheckIncludeFile)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckTypeSize)

CHECK_INCLUDE_FILE(dirent.h     HAVE_DIRENT_H)
CHECK_INCLUDE_FILE(stdint.h     HAVE_STDINT_H)
CHECK_INCLUDE_FILE(inttypes.h   HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(sys/stat.h   HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/types.h  HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(unistd.h     HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(windows.h    HAVE_WINDOWS_H)

CHECK_FUNCTION_EXISTS(bcopy     HAVE_BCOPY)
CHECK_FUNCTION_EXISTS(memmove   HAVE_MEMMOVE)
CHECK_FUNCTION_EXISTS(strerror  HAVE_STRERROR)
CHECK_FUNCTION_EXISTS(strtoll   HAVE_STRTOLL)
CHECK_FUNCTION_EXISTS(strtoq    HAVE_STRTOQ)
CHECK_FUNCTION_EXISTS(_strtoi64 HAVE__STRTOI64)

CHECK_TYPE_SIZE("long long"             LONG_LONG)
CHECK_TYPE_SIZE("unsigned long long"    UNSIGNED_LONG_LONG)

# Prepare build configuration

SET(pcre_have_long_long 0)
SET(pcre_have_ulong_long 0)

IF(HAVE_LONG_LONG)
        SET(pcre_have_long_long 1)
ENDIF(HAVE_LONG_LONG)

IF(HAVE_UNSIGNED_LONG_LONG)
        SET(pcre_have_ulong_long 1)
ENDIF(HAVE_UNSIGNED_LONG_LONG)

# Output files
CONFIGURE_FILE(config-cmake.h.in
               ${PROJECT_BINARY_DIR}/config.h
               @ONLY)

CONFIGURE_FILE(pcre.h.generic
               ${PROJECT_BINARY_DIR}/pcre.h
               COPYONLY)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/pcre_chartables.c.dist
                    ${PROJECT_BINARY_DIR}/pcre_chartables.c
                    COPYONLY)

# Source code

SET(PCRE_HEADERS ${PROJECT_BINARY_DIR}/pcre.h)

SET(PCRE_SOURCES
  ${PROJECT_BINARY_DIR}/pcre_chartables.c
  pcre_compile.c
  pcre_config.c
  pcre_dfa_exec.c
  pcre_exec.c
  pcre_fullinfo.c
  pcre_get.c
  pcre_globals.c
  pcre_info.c
  pcre_newline.c
  pcre_maketables.c
  pcre_ord2utf8.c
  pcre_refcount.c
  pcre_study.c
  pcre_tables.c
  pcre_try_flipped.c
  pcre_ucd.c
  pcre_valid_utf8.c
  pcre_version.c
  pcre_xclass.c
)

# Build setup

ADD_DEFINITIONS(-DHAVE_CONFIG_H)

IF(MSVC)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
ENDIF(MSVC)

SET(CMAKE_INCLUDE_CURRENT_DIR 1)
# needed to make sure to not link debug libs
# against release libs and vice versa
IF(WIN32)
  SET(CMAKE_DEBUG_POSTFIX "d")
ENDIF(WIN32)

ADD_LIBRARY(${PROJECT_NAME} ${PCRE_HEADERS} ${PCRE_SOURCES} ${PROJECT_BINARY_DIR}/config.h)

# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
	set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
