########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 2.8 )

# Print a message and fail for people who don't
# read the build instructions and then complain
# when the build process fails for them.
if ( ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR} )
    message( FATAL_ERROR "You are trying to run CMake from the <top_folder>/src/FlightCrew directory, "
                         "instead of just from the <top_folder> directory.\nDO NOT DO THIS.\n"
                         "The correct way looks like this:\n"
                         "  cmake -G '<generator_name>' /path/to/topmost/folder/in/source/package\n"
                         "You will probably now need to first clean your build directory." )
endif() 

#############################################################################

project( FlightCrew )

file( GLOB_RECURSE SOURCES *.h *.cpp *.xsd *.dtd )

# The test sources are a part of a different target, so we remove them
file( GLOB_RECURSE to_remove tests/*.h tests/*.cpp )
if( to_remove )
    list( REMOVE_ITEM SOURCES ${to_remove} )
endif() 

#############################################################################

# Creating source groups for VS, Xcode
include( ${CMAKE_SOURCE_DIR}/cmake_extras/FileSystemSourceGroups.cmake )
create_source_groups( SOURCES )

#############################################################################

set( PCH_NAME stdafx )

# stdafx.cpp is compiled separately as a PCH
file( GLOB to_remove ${PCH_NAME}.cpp )
list( REMOVE_ITEM SOURCES ${to_remove} )

#############################################################################

# creating PCH's for MSVC and GCC on Linux
include( ${CMAKE_SOURCE_DIR}/cmake_extras/CustomPCH.cmake )
set( ALL_INCLUDES ${BOOST_INCLUDE_DIRS}
                  ${XERCES_INCLUDE_DIRS} )
set( GCC_PCH_TARGET gccPCH_fc )

precompiled_header( SOURCES ALL_INCLUDES ${GCC_PCH_TARGET} ${PCH_NAME} )

#############################################################################

# We need to pick up the stdafx.h file (current source dir),
# the stdafx.h.gch file (current binary dir)
# and the headers for the linked-to libraries
include_directories( ${CMAKE_CURRENT_BINARY_DIR}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${BOOST_INCLUDE_DIRS} 
                     ${XERCES_INCLUDE_DIRS}
                     ${XERCESEXTENSIONS_INCLUDE_DIRS}
                     ../utf8-cpp
                   )

if( BUILD_SHARED_FC )
    # Windows clients also need to specify FC_BUILT_AS_DLL
    # when they want a dll, but NOT FC_DLL_EXPORTING
    add_definitions( -DFC_DLL_EXPORTING -DFC_BUILT_AS_DLL )
    add_library( ${PROJECT_NAME} SHARED ${SOURCES} )
else()
    add_library( ${PROJECT_NAME} ${SOURCES} )
endif()

target_link_libraries( ${PROJECT_NAME} ${BOOST_LIBS} ${XERCESEXTENSIONS_LIBRARIES} )

#############################################################################

# Xcode PCH support. Has to come after the target is created.              
xcode_pch( ${PCH_NAME} )

#############################################################################

# "Link time code generation" flags for MSVC
# TODO: split into special cmake file
if( MSVC )
    add_definitions( /DUNICODE /D_UNICODE /W4 )
    
    # This warning is present only at the highest warning level (/W4)
    # and is routinely disabled because it complains about valid 
    # constructs like "while (true)"
    add_definitions( /wd4127 )
    
    # The /Zc:wchar_t- flag can't go into add_definitions
    # because the RC compiler picks it up too and it provokes a name clash
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t- /MP")
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG" )

elseif( CMAKE_COMPILER_IS_GNUCXX )
    # "Print all warnings" flag for GCC
    add_definitions( -Wall )

    # Make sure the PCH is built for GCC
    add_dependencies( ${PROJECT_NAME} ${GCC_PCH_TARGET} )
endif()

# needed for correct Xerces header inclusion
if( FORCE_BUNDLED_COPIES OR NOT XERCES_FOUND )
    add_definitions( -DXERCES_STATIC_LIBRARY )
endif()

#############################################################################

# We don't build the tests when fc is built as a shared
# library on msvc since the tests need access to the fc
# internal apis which are not exported when building a DLL.
# If we tried to build them, we would get linker errors.
# We also don't build the tests if the user specified it.
if( NOT ( BUILD_SHARED_FC AND MSVC ) AND NOT NO_TEST_EXE )
    add_subdirectory( tests )
endif()
