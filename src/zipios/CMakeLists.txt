########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 2.8 ) 

project( zipios )

file( GLOB_RECURSE SOURCES *.cpp *.h ) 

# We need to pick up the stdafx.h file
# and the headers for the linked-to libraries
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}
                     ${BoostParts_SOURCE_DIR}
                     ${zlib_SOURCE_DIR}
                     ${zlib_BINARY_DIR}
                     )

link_directories ( ${PROJECT_BINARY_DIR}/lib )

add_library( ${PROJECT_NAME} ${SOURCES} )

target_link_libraries( ${PROJECT_NAME} zlib BoostParts )

#############################################################################

# "Link time code generation" flags for MSVC
# TODO: split into special cmake file
if( MSVC )
    add_definitions( /DUNICODE /D_UNICODE /D_CRT_SECURE_NO_DEPRECATE )
    
    # This warning is present only at the highest warning level (/W4)
    # and is routinely disabled because it complains about valid 
    # constructs like "while (true)"
    add_definitions( /wd4127 )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-" )
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )

# "Print all warnings" flag for GCC
elseif( CMAKE_COMPILER_IS_GNUCXX )
    add_definitions( -Wall )
endif()

