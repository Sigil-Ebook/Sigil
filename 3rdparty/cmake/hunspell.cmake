cmake_minimum_required( VERSION 3.0 )

PROJECT(hunspell CXX)

set( HUNSPELL_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "" )
set( HUNSPELL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME} CACHE INTERNAL "" )

SET(PROJECT_C_FLAGS "")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/extra/hunspell/config.h
               ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/config.h COPYONLY)

SET(SOURCES
    ${PROJECT_NAME}/src/hunspell/affentry.cxx
    ${PROJECT_NAME}/src/hunspell/affixmgr.cxx
    ${PROJECT_NAME}/src/hunspell/csutil.cxx
    ${PROJECT_NAME}/src/hunspell/dictmgr.cxx
    ${PROJECT_NAME}/src/hunspell/filemgr.cxx
    ${PROJECT_NAME}/src/hunspell/hashmgr.cxx
    ${PROJECT_NAME}/src/hunspell/hunspell.cxx
    ${PROJECT_NAME}/src/hunspell/hunzip.cxx
    ${PROJECT_NAME}/src/hunspell/phonet.cxx
    ${PROJECT_NAME}/src/hunspell/replist.cxx
    ${PROJECT_NAME}/src/hunspell/suggestmgr.cxx
)
if( APPLE )
    set(CMAKE_MACOSX_RPATH 1)
endif()

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}/src/hunspell
)

# Special compiler and linker flags for MSVC
if( MSVC )
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS /DBUILDING_LIBHUNSPELL)
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" )
endif()
