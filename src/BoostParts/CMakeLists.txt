########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 2.8 ) 

project( BoostParts )

set( SOURCES
    libs/smart_ptr/src/sp_collector.cpp
    libs/smart_ptr/src/sp_debug_hooks.cpp
    ) 
    
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
			
add_library( BoostParts ${SOURCES} ) 
 
# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
	set_target_properties( BoostParts PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()