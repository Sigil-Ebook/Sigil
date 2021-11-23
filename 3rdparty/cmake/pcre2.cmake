cmake_minimum_required(VERSION 3.0)

project(pcre2 C)

INCLUDE(CheckIncludeFile)
INCLUDE(CheckIncludeFileCXX)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckTypeSize)

set( PCRE2_LIBRARIES ${PROJECT_NAME}-16 CACHE INTERNAL "" )
set( PCRE2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/src CACHE INTERNAL "" )

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

# OPTION(BUILD_STATIC_LIBS "Build static libraries." ON)
# OPTION(PCRE2_BUILD_PCRE2_8 "Build 8 bit PCRE2 library" OFF)
# OPTION(PCRE2_BUILD_PCRE2_16 "Build 16 bit PCRE2 library" ON)
# OPTION(PCRE2_BUILD_PCRE2_32 "Build 32 bit PCRE2 library" OFF)

OPTION(PCRE2_STATIC_PIC "Build the static library with the option position independent code enabled." ON)

SET(PCRE2_LINK_SIZE "2")
SET(PCRE2_PARENS_NEST_LIMIT "250")
SET(PCRE2_HEAP_LIMIT "20000000")
SET(PCRE2_MATCH_LIMIT "10000000")
SET(PCRE2_MATCH_LIMIT_DEPTH "MATCH_LIMIT")

SET(SUPPORT_PCRE2_16 1)

SET(PCRE2_NEWLINE "LF")
SET(NEWLINE_DEFAULT "2")

SET(PCRE2_SUPPORT_UNICODE ON)
SET(SUPPORT_UNICODE 1)

SET(SUPPORT_UCP 1)

SET(PCRE2_SUPPORT_JIT ON)
SET(SUPPORT_JIT 1)

# Output files
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/config-cmake.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/config.h @ONLY)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/src/pcre2.h.generic
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/pcre2.h COPYONLY)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/src/pcre2_chartables.c.dist
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/pcre2_chartables.c COPYONLY)

SET(PCRE2_HEADERS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/pcre2.h)

SET(SOURCES
    ${PROJECT_NAME}/src/pcre2_auto_possess.c
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/pcre2_chartables.c
    ${PROJECT_NAME}/src/pcre2_compile.c
    ${PROJECT_NAME}/src/pcre2_config.c
    ${PROJECT_NAME}/src/pcre2_context.c
    ${PROJECT_NAME}/src/pcre2_convert.c
    ${PROJECT_NAME}/src/pcre2_dfa_match.c
    ${PROJECT_NAME}/src/pcre2_error.c
    ${PROJECT_NAME}/src/pcre2_extuni.c
    ${PROJECT_NAME}/src/pcre2_find_bracket.c
    ${PROJECT_NAME}/src/pcre2_jit_compile.c
    ${PROJECT_NAME}/src/pcre2_maketables.c
    ${PROJECT_NAME}/src/pcre2_match.c
    ${PROJECT_NAME}/src/pcre2_match_data.c
    ${PROJECT_NAME}/src/pcre2_newline.c
    ${PROJECT_NAME}/src/pcre2_ord2utf.c
    ${PROJECT_NAME}/src/pcre2_pattern_info.c
    ${PROJECT_NAME}/src/pcre2_script_run.c
    ${PROJECT_NAME}/src/pcre2_serialize.c
    ${PROJECT_NAME}/src/pcre2_string_utils.c
    ${PROJECT_NAME}/src/pcre2_study.c
    ${PROJECT_NAME}/src/pcre2_substitute.c
    ${PROJECT_NAME}/src/pcre2_substring.c
    ${PROJECT_NAME}/src/pcre2_tables.c
    ${PROJECT_NAME}/src/pcre2_ucd.c
    ${PROJECT_NAME}/src/pcre2_valid_utf.c
    ${PROJECT_NAME}/src/pcre2_xclass.c
)

add_library(${PROJECT_NAME}-16 ${SOURCES})
target_include_directories(${PROJECT_NAME}-16 PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/src
)
set_target_properties(${PROJECT_NAME}-16 PROPERTIES 
    COMPILE_DEFINITIONS PCRE2_CODE_UNIT_WIDTH=16
    COMPILE_FLAGS "${PROJECT_C_FLAGS}")

TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME}-16 PUBLIC PCRE2_STATIC)

IF(PCRE2_STATIC_PIC)
    SET_TARGET_PROPERTIES(${PROJECT_NAME}-16 PROPERTIES POSITION_INDEPENDENT_CODE 1)
ENDIF(PCRE2_STATIC_PIC)


# Special compiler and linker flags for MSVC
if( MSVC )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL" )
    set_target_properties( ${PROJECT_NAME}-16 PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
