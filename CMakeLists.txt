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
set( PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR}/package )

set( SIGIL_MAJOR_VERSION 0 )
set( SIGIL_MINOR_VERSION 1 )
set( SIGIL_REVISION_VERSION 4 )
set( SIGIL_FULL_VERSION ${SIGIL_MAJOR_VERSION}.${SIGIL_MINOR_VERSION}.${SIGIL_REVISION_VERSION} )

# Check if platform is 64 bit
if( NOT APPLE )
   if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
     set( 64_BIT_PLATFORM 0 )
   else()
     set( 64_BIT_PLATFORM 1 )
   endif()
endif()

add_subdirectory( src/tidyLib )
add_subdirectory( src/ZipArchive )
add_subdirectory( src/Sigil )


