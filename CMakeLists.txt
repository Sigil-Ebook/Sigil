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
set( SIGIL_REVISION_VERSION 2 )
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

# This disables the building of the test runner app for FlightCrew
set( NO_TEST_EXE 1 )

if( NOT FORCE_BUNDLED_COPIES )
    find_package( Boost COMPONENTS date_time filesystem program_options regex system thread )
    find_package( ZLIB )
    find_package( PkgConfig )
    find_package( MiniZip )
    pkg_check_modules( XERCES xerces-c>=3.1 )
    pkg_check_modules( HUNSPELL hunspell )
    if ( NOT APPLE )
        pkg_check_modules( PCRE libpcre16 )
    endif()
    find_package( XercesExtensions )
    find_package( FlightCrew )
endif()

# look for a specific lib python version
find_package( PythonLibs 3.4 REQUIRED )

# We must use the bundled Tidy because there are Sigil
# specific changes to it.
add_subdirectory( src/tidyLib )

if( FORCE_BUNDLED_COPIES OR NOT Boost_FOUND )
    add_subdirectory( src/BoostParts )
    # Set these so zipios doesn't complain.
    set( Boost_DATE_TIME_LIBRARY 1 )
    set( Boost_FILESYSTEM_LIBRARY 1 )
    set( Boost_PROGRAM_OPTIONS_LIBRARY 1 )
    set( Boost_REGEX_LIBRARY 1 )
    set( Boost_SYSTEM_LIBRARY 1 )
    set( Boost_THREAD_LIBRARY 1 )
    set( BOOST_LIBS BoostParts )
    set( BOOST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/BoostParts )
else()
    set( BOOST_LIBS ${Boost_DATE_TIME_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_REGEX_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} )
    set( BOOST_INCLUDE_DIRS ${Boost_INCLUDE_DIRS} )
endif()

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

if( FORCE_BUNDLED_COPIES OR NOT XERCES_FOUND )
    add_subdirectory( src/Xerces )
    set( XERCES_LIBRARIES Xerces )
    set( XERCES_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/Xerces )
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

if( NOT XERCESEXTENSIONS_FOUND )
    add_subdirectory( src/XercesExtensions )
    set( XERCESEXTENSIONS_LIBRARIES XercesExtensions )
    set( XERCESEXTENSIONS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/XercesExtensions )
endif()

if( NOT FLIGHTCREW_FOUND )
    add_subdirectory( src/FlightCrew )
    set( FLIGHTCREW_LIBRARIES FlightCrew )
    set( FLIGHTCREW_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src/FlightCrew )
endif()

add_subdirectory( src/Sigil )
