########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 2.8 )

PROJECT(hunspell)

SET(HUNSPELL_HEADERS
    affentry.hxx
    affixmgr.hxx
    atypes.hxx
    baseaffix.hxx
    csutil.hxx
    dictmgr.hxx
    filemgr.hxx
    hashmgr.hxx
    htypes.hxx
    hunspell.hxx
    hunzip.hxx
    langnum.hxx
    phonet.hxx
    replist.hxx
    suggestmgr.hxx
    w_char.hxx
)

SET(HUNSPELL_SOURCES
    affentry.cxx
    affixmgr.cxx
    csutil.cxx
    dictmgr.cxx
    filemgr.cxx
    hashmgr.cxx
    hunspell.cxx
    hunzip.cxx
    phonet.cxx
    replist.cxx
    suggestmgr.cxx
)

ADD_LIBRARY(${PROJECT_NAME} ${HUNSPELL_HEADERS} ${HUNSPELL_SOURCES})

# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
	set_target_properties( tidyLib PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
