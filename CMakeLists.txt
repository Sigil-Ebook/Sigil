########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 2.8 ) 

project( Sigil ) 

set( CMAKE_DEBUG_POSTFIX "d" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )
set( PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR}/package )

set( SIGIL_MAJOR_VERSION 0 )
set( SIGIL_MINOR_VERSION 4 )
set( SIGIL_REVISION_VERSION 2 )
set( SIGIL_FULL_VERSION ${SIGIL_MAJOR_VERSION}.${SIGIL_MINOR_VERSION}.${SIGIL_REVISION_VERSION} )

# Check if platform is 64 bit
if( NOT APPLE )
   if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
     set( 64_BIT_PLATFORM 0 )
   else()
     set( 64_BIT_PLATFORM 1 )
   endif()
endif()

# Profiler configuration for GCC
if( CMAKE_COMPILER_IS_GNUCXX  )
    set( CMAKE_BUILD_TYPE profiling )
    set( CMAKE_CXX_FLAGS_PROFILING "-O2 -DNDEBUG -pg -g -fno-omit-frame-pointer -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls" )

    # After creating the new build type,
    # we clear the default back to empty
    set( CMAKE_BUILD_TYPE  )
endif()

# We need this to make sure g++ and the linker
# create exes that can run on 10.6+
set( CMAKE_OSX_DEPLOYMENT_TARGET "10.6" )

# 10.6 is the required minimum OS X version.
set( CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.6.sdk" )

# Universal builds for mac
set( CMAKE_OSX_ARCHITECTURES "i386;x86_64" )

# This disables the building of the test runner app for FlightCrew
set( NO_TEST_EXE 1 )

add_subdirectory( src/tidyLib )
add_subdirectory( src/ZipArchive )
add_subdirectory( src/BoostParts )
add_subdirectory( src/Xerces )
add_subdirectory( src/XercesExtensions )
add_subdirectory( src/zlib )
add_subdirectory( src/zipios )
add_subdirectory( src/pcre )
add_subdirectory( src/FlightCrew )
add_subdirectory( src/Sigil )
