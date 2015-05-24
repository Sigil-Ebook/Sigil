cmake_minimum_required( VERSION 3.0 )

PROJECT(zlib C)

set( ZLIB_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "" )
set( ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME} CACHE INTERNAL "" )

SET(PROJECT_C_FLAGS "")

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)

check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(stdint.h    HAVE_STDINT_H)
check_include_file(stddef.h    HAVE_STDDEF_H)
check_type_size(off64_t        OFF64_T)
check_function_exists(fseeko   HAVE_FSEEKO)
check_include_file(unistd.h    Z_HAVE_UNISTD_H)

if(HAVE_SYS_TYPES_H)
    SET(PROJECT_C_FLAGS "${PROJECT_C_FLAGS} -DHAVE_SYS_TYPES_H")
endif()
if(HAVE_STDINT_H)
    set(PROJECT_C_FLAGS "${PROJECT_C_FLAGS} -DHAVE_STDINT_H")
endif()
if(HAVE_STDDEF_H)
    set(PROJECT_C_FLAGS "${PROJECT_C_FLAGS} -DHAVE_STDDEF_H")
endif()
if(HAVE_OFF64_T)
    set(PROJECT_C_FLAGS "${PROJECT_C_FLAGS} -D_LARGEFILE64_SOURCE=1")
endif()
if(NOT HAVE_FSEEKO)
    set(PROJECT_C_FLAGS "${PROJECT_C_FLAGS} -DNO_FSEEKO")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/zconf.h.cmakein
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/zconf.h @ONLY)

set(SOURCES
    ${PROJECT_NAME}/adler32.c
    ${PROJECT_NAME}/compress.c
    ${PROJECT_NAME}/crc32.c
    ${PROJECT_NAME}/deflate.c
    ${PROJECT_NAME}/gzclose.c
    ${PROJECT_NAME}/gzlib.c
    ${PROJECT_NAME}/gzread.c
    ${PROJECT_NAME}/gzwrite.c
    ${PROJECT_NAME}/inflate.c
    ${PROJECT_NAME}/infback.c
    ${PROJECT_NAME}/inftrees.c
    ${PROJECT_NAME}/inffast.c
    ${PROJECT_NAME}/trees.c
    ${PROJECT_NAME}/uncompr.c
    ${PROJECT_NAME}/zutil.c
)

ADD_LIBRARY(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}
)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PROJECT_C_FLAGS}")

if( MSVC )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
