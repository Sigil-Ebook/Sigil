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
set( SIGIL_REVISION_VERSION 700 )
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

if (USE_SYSTEM_LIBS)
    MESSAGE(STATUS "Trying to use System Libraries...")
    find_package( ZLIB )
    find_package( PkgConfig )
    find_package( MiniZip )
    pkg_check_modules( HUNSPELL hunspell )
    if ( NOT APPLE )
        pkg_check_modules( PCRE libpcre16 )
    endif()
    #find_package (PythonLibs 3.4)
endif()
find_package (PythonLibs 3.4)

# gumbo-parser it is our main xhtml/html5 parser.
# We have an internal version because it diverges from Google's and GitHub's
# versions and neither want's our epub specific changes.
add_subdirectory( internal/gumbo )
set( GUMBO_LIBRARIES gumbo )
set( GUMBO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/internal/gumbo )

add_subdirectory( 3rdparty/ )
add_subdirectory( src/ )
