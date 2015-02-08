########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 3.0 ) 

project( gumbo )

file( GLOB SOURCES *.c *.h )
			
if( MSVC )
    set ( EXTRA_INC include/)
    include_directories( SYSTEM ${EXTRA_INC} )
    set_source_files_properties( utf8.c PROPERTIES LANGUAGE CXX )
endif()

add_library( gumbo ${SOURCES} ) 

# Special compiler and linker flags for MSVC
if( MSVC )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL" ) 
	set_target_properties( gumbo PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()

if( UNIX AND NOT APPLE )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99" )
endif()
