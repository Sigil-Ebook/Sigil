cmake_minimum_required(VERSION 3.0)

project(pcre C)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckTypeSize)

set( PCRE_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "" )
set( PCRE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME} CACHE INTERNAL "" )

SET(PROJECT_C_FLAGS "-DHAVE_CONFIG_H")

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
SET(PCRE_PARENS_NEST_LIMIT "250")

SET(SUPPORT_PCRE16 1)
SET(SUPPORT_UTF 1)
SET(SUPPORT_UCP 1)
SET(PCRE_SUPPORT_UTF ON)
SET(SUPPORT_JIT 1)
SET(NEWLINE "10") # LF

# Output files
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/config-cmake.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/config.h @ONLY)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/pcre.h.generic
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/pcre.h COPYONLY)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/pcre_chartables.c.dist
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/pcre_chartables.c COPYONLY)

SET(SOURCES
    ${PROJECT_NAME}/pcre16_byte_order.c
    ${PROJECT_NAME}/pcre16_chartables.c
    ${PROJECT_NAME}/pcre16_compile.c
    ${PROJECT_NAME}/pcre16_config.c
    ${PROJECT_NAME}/pcre16_dfa_exec.c
    ${PROJECT_NAME}/pcre16_exec.c
    ${PROJECT_NAME}/pcre16_fullinfo.c
    ${PROJECT_NAME}/pcre16_get.c
    ${PROJECT_NAME}/pcre16_globals.c
    ${PROJECT_NAME}/pcre16_jit_compile.c
    ${PROJECT_NAME}/pcre16_maketables.c
    ${PROJECT_NAME}/pcre16_newline.c
    ${PROJECT_NAME}/pcre16_ord2utf16.c
    ${PROJECT_NAME}/pcre16_refcount.c
    ${PROJECT_NAME}/pcre16_string_utils.c
    ${PROJECT_NAME}/pcre16_study.c
    ${PROJECT_NAME}/pcre16_tables.c
    ${PROJECT_NAME}/pcre16_ucd.c
    ${PROJECT_NAME}/pcre16_utf16_utils.c
    ${PROJECT_NAME}/pcre16_valid_utf16.c
    ${PROJECT_NAME}/pcre16_version.c
    ${PROJECT_NAME}/pcre16_xclass.c
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/pcre_chartables.c
)

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}
)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PROJECT_C_FLAGS}")

# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /D_CRT_SECURE_NO_WARNINGS /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
