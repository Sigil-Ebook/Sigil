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
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckTypeSize)

CHECK_INCLUDE_FILE(dirent.h     HAVE_DIRENT_H)
CHECK_INCLUDE_FILE(stdint.h     HAVE_STDINT_H)
CHECK_INCLUDE_FILE(inttypes.h   HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE(sys/stat.h   HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE(sys/types.h  HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILE(unistd.h     HAVE_UNISTD_H)
CHECK_INCLUDE_FILE(windows.h    HAVE_WINDOWS_H)

CHECK_INCLUDE_FILE_CXX(type_traits.h            HAVE_TYPE_TRAITS_H)
CHECK_INCLUDE_FILE_CXX(bits/type_traits.h       HAVE_BITS_TYPE_TRAITS_H)

CHECK_FUNCTION_EXISTS(bcopy     HAVE_BCOPY)
CHECK_FUNCTION_EXISTS(memmove   HAVE_MEMMOVE)
CHECK_FUNCTION_EXISTS(strerror  HAVE_STRERROR)
CHECK_FUNCTION_EXISTS(strtoll   HAVE_STRTOLL)
CHECK_FUNCTION_EXISTS(strtoq    HAVE_STRTOQ)
CHECK_FUNCTION_EXISTS(_strtoi64 HAVE__STRTOI64)

CHECK_TYPE_SIZE("long long"             LONG_LONG)
CHECK_TYPE_SIZE("unsigned long long"    UNSIGNED_LONG_LONG)

# Prepare build configuration

SET(pcre_have_type_traits 0)
SET(pcre_have_bits_type_traits 0)

IF(HAVE_TYPE_TRAITS_H)
        SET(pcre_have_type_traits 1)
ENDIF(HAVE_TYPE_TRAITS_H)

IF(HAVE_BITS_TYPE_TRAITS_H)
        SET(pcre_have_bits_type_traits 1)
ENDIF(HAVE_BITS_TYPE_TRAITS_H)

SET(pcre_have_long_long 0)
SET(pcre_have_ulong_long 0)

IF(HAVE_LONG_LONG)
        SET(pcre_have_long_long 1)
ENDIF(HAVE_LONG_LONG)

IF(HAVE_UNSIGNED_LONG_LONG)
        SET(pcre_have_ulong_long 1)
ENDIF(HAVE_UNSIGNED_LONG_LONG)

# Our configuration

SET(PCRE_LINK_SIZE "2")
SET(PCRE_POSIX_MALLOC_THRESHOLD "10")
SET(PCRE_MATCH_LIMIT "10000000")
SET(PCRE_MATCH_LIMIT_RECURSION "MATCH_LIMIT")


SET(SUPPORT_PCRE16 1)
SET(SUPPORT_UTF 1)
SET(SUPPORT_UCP 1)
SET(PCRE_SUPPORT_UTF ON)
SET(SUPPORT_JIT 1)
# LF
SET(NEWLINE "10")

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
  pcre16_byte_order.c
  pcre16_chartables.c
  pcre16_compile.c
  pcre16_config.c
  pcre16_dfa_exec.c
  pcre16_exec.c
  pcre16_fullinfo.c
  pcre16_get.c
  pcre16_globals.c
  pcre16_jit_compile.c
  pcre16_maketables.c
  pcre16_newline.c
  pcre16_ord2utf16.c
  pcre16_refcount.c
  pcre16_string_utils.c
  pcre16_study.c
  pcre16_tables.c
  pcre16_ucd.c
  pcre16_utf16_utils.c
  pcre16_valid_utf16.c
  pcre16_version.c
  pcre16_xclass.c
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
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL" ) 
	set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
