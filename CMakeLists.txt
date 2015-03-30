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

if( NOT USE_SYSTEM_LIBS OR NOT ZLIB_FOUND )
    MESSAGE(STATUS "Using Bundled ZLIB")
    add_subdirectory( 3rdparty/zlib )
    set( ZLIB_LIBRARIES zlib )
    set( ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/zlib ${CMAKE_BINARY_DIR}/3rdparty/zlib )
else ()
    MESSAGE(STATUS "Using System ZLIB")
endif()

if( NOT USE_SYSTEM_LIBS OR NOT MINIZIP_FOUND )
    MESSAGE(STATUS "Using Bundled MiniZip")
    add_subdirectory( 3rdparty/minizip )
    set( MINIZIP_LIBRARIES minizip )
    set( MINIZIP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/minizip )
else ()
    MESSAGE(STATUS "Using System MiniZip")
endif()


if( NOT USE_SYSTEM_LIBS OR NOT HUNSPELL_FOUND )
    MESSAGE(STATUS "Using Bundled Hunspell")
    add_subdirectory( 3rdparty/hunspell )
    set( HUNSPELL_LIBRARIES hunspell )
    set( HUNSPELL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/hunspell )
else ()
    MESSAGE(STATUS "Using System Hunspell")
endif()

if( NOT USE_SYSTEM_LIBS OR NOT PCRE_FOUND OR APPLE )
    MESSAGE(STATUS "Using Bundled PCRE")
    add_subdirectory( 3rdparty/pcre )
    set( PCRE_LIBRARIES pcre )
    set( PCRE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/pcre ${CMAKE_BINARY_DIR}/3rdparty/pcre )
else ()
    MESSAGE(STATUS "Using System PCRE")
endif()

if (SYSTEM_LIBS_REQUIRED)
    if (NOT ZLIB_FOUND OR NOT MINIZIP_FOUND OR NOT HUNSPELL_FOUND OR (NOT APPLE AND NOT PCRE_FOUND))
        MESSAGE(FATAL_ERROR "Could not find all required system libraries...")
    endif()
endif()

add_subdirectory( src/ )
