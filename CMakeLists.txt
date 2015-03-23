########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 3.0 ) 

project( Sigil ) 

set( CMAKE_DEBUG_POSTFIX "d" )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )
set( PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR}/package )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_extras" )

set( SIGIL_MAJOR_VERSION 0 )
set( SIGIL_MINOR_VERSION 8 )
set( SIGIL_REVISION_VERSION 4 )
set( SIGIL_FULL_VERSION ${SIGIL_MAJOR_VERSION}.${SIGIL_MINOR_VERSION}.${SIGIL_REVISION_VERSION} )

# Apple users building with XCode notes:
# If building with XCode you need to set the target and location
# of the SDK. If you are building with the Command Line Tools
# you do not need to specify the follwing options.
# It is recommened to set them when running cmake using -D
# rather than editing this file directly.
# CMAKE_OSX_DEPLOYMENT_TARGET "10.8"
# CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.8.sdk"


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

if( NOT FORCE_BUNDLED_COPIES )
    find_package( ZLIB )
    find_package( PkgConfig )
    find_package( MiniZip )
    pkg_check_modules( HUNSPELL hunspell )
    if ( NOT APPLE )
        pkg_check_modules( PCRE libpcre16 )
    endif()
endif()

# look for a specific lib python version
find_package( PythonLibs 3.4 REQUIRED )

# gumbo-parser we bundle it as our main xhtml/html5 parser
add_subdirectory( src/gumbo )
set( GUMBO_LIBRARIES gumbo )
set( GUMBO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/gumbo )

if( FORCE_BUNDLED_COPIES OR NOT ZLIB_FOUND )
    add_subdirectory( src/zlib )
    set( ZLIB_LIBRARIES zlib )
    set( ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/zlib ${CMAKE_BINARY_DIR}/src/zlib )
endif()

if( FORCE_BUNDLED_COPIES OR NOT MINIZIP_FOUND )
    add_subdirectory( src/minizip )
    set( MINIZIP_LIBRARIES minizip )
    set( MINIZIP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/minizip )
endif()


if( FORCE_BUNDLED_COPIES OR NOT HUNSPELL_FOUND )
    add_subdirectory( src/hunspell )
    set( HUNSPELL_LIBRARIES hunspell )
    set( HUNSPELL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/hunspell )
endif()

if( FORCE_BUNDLED_COPIES OR NOT PCRE_FOUND OR APPLE )
    add_subdirectory( src/pcre )
    set( PCRE_LIBRARIES pcre )
    set( PCRE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/pcre ${CMAKE_BINARY_DIR}/src/pcre )
endif()

add_subdirectory( src/Sigil )
