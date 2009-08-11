########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 2.6 ) 

project( Sigil ) 

set( CMAKE_OSX_ARCHITECTURES "ppc;i386" )
set( CMAKE_DEBUG_POSTFIX "d" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )

add_subdirectory( src/tidyLib )
add_subdirectory( src/ZipArchive )
add_subdirectory( src/Sigil )


