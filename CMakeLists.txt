###A#####################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

# Hard minimum version of cmake for all sub projects and Sigil
cmake_minimum_required( VERSION 3.18 ) 

project( Sigil )

# Change this URL if the location of the CI Windows Qt6 download ever changes
set (WINQTURL "https://github.com/dougmassay/win-qtwebkit-5.212/releases/download/v5.212-1")

# Used to keep downloaded Qt and PySide6 versions in sync.
# Check https://github.com/dougmassay/win-qtwebkit-5.212/releases/ to
# see what versions are available. Defaults to 6.7.3.
if(NOT DEFINED QTVER)
    set(QTVER 6.8.2)
endif()

# Set to 1 to create and use a virtual Python venv on Windows and Linux.
# Use in conjunction with QTVER to download/install correct PySide6 module
if ( NOT DEFINED USE_VIRT_PY )
    set ( USE_VERT_PY 0 )
endif()

# Set to 1 to download and use a custom-built Qt from WINQTURL.
# Use in conjunction with QTVER to clarify which version of Qt to download.
if ( NOT DEFINED DOWNLOAD_QT )
    set ( DOWNLOAD_QT 0 )
endif()

# Set Inno minimum Windows version 
# Windows 10 (1809)
set ( WIN_MIN_VERSION 10.0.17763 )

set( CMAKE_DEBUG_POSTFIX "d" )
# Avoid the whole Release/Debug folder creation with Visual Studio
if (MSVC_IDE)
   set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/bin")
   set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/bin")
   set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_BINARY_DIR}/bin")
   set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/lib")
   set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/lib")
   set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_BINARY_DIR}/lib")
   set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/lib")
   set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/lib")
   set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO "${PROJECT_BINARY_DIR}/lib")
endif()
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )
set( PACKAGE_DIRECTORY ${PROJECT_BINARY_DIR}/package )
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_extras" )

set( SIGIL_MAJOR_VERSION 2 )
set( SIGIL_MINOR_VERSION 5 )
set( SIGIL_REVISION_VERSION 0 )
set( SIGIL_FULL_VERSION ${SIGIL_MAJOR_VERSION}.${SIGIL_MINOR_VERSION}.${SIGIL_REVISION_VERSION} )

if( UNIX AND NOT APPLE )
   include(GNUInstallDirs)

   # Make sure the correct rpath is set for the sigil executable
   # on Linux -- for both the build and the installed versions.
   # Needed to make sure the installed shared hunspell and gumbo
   # libraries are found/used at runtime.

   # use, i.e. don't skip the full RPATH for the build tree
   SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

   # when building, don't use the install RPATH already
   # (but later on when installing)
   SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

   SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_FULL_LIBDIR}/sigil")

   # add the automatically determined parts of the RPATH
   # which point to directories outside the build tree to the install RPATH.
   # This will ensure the Qt5 lib directory is always included after install.
   SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

   # Because GCC's link time optimization is willfully choosing to suck.
   add_compile_options(-fno-lto)

   if( NOT DEFINED APPIMAGE_BUILD )
       set( APPIMAGE_BUILD 0 )
   endif()
endif()

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
    pkg_check_modules( PCRE2 libpcre2-16 )
endif()

message(STATUS "CMake version in use: ${CMAKE_VERSION}")

message(STATUS "Using FindPython3 cmake module")
# Find Python3 module for both the build and bundling
if ( APPLE )
    # cmake's FindPython3 does *not* find frameworks based on precendence in PATH
    # so set some pretty specific hints using our own FindPython3MacFramework.cmake first
    find_package(Python3MacFramework 3.9 QUIET)
    find_package(Python3 3.9 COMPONENTS Interpreter Development)
endif()
if ( WIN32 )
    if ( DOWNLOAD_QT )
        include(windownloadqt.cmake)
        set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/Qt${QTVER}/lib/cmake")
        set(Qt6_Dir "${CMAKE_CURRENT_BINARY_DIR}/Qt${QTVER}/lib/cmake/Qt6")
        if(NOT EXISTS "${Qt6_Dir}")
            message(FATAL_ERROR "Qt6 download/configure unsuccessful!")
        endif()
    endif()
    if ( USE_VIRT_PY )
        include(winvirtpy.cmake)
        # Set hints so virtual Python environement is found first
        set(Python3_FIND_VIRTUAL ONLY)
        set(Python3_FIND_REGISTRY NEVER)
        set(Python3_FIND_STRATEGY LOCATION)
        # Trick cmake's findPython3 module into thinking the virtual python is activated
        set(VIRT_PATH "${CMAKE_CURRENT_BINARY_DIR}/sigilpy")
        set(ENV{VIRTUAL_ENV} "${VIRT_PATH}")
        if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/sigilpy/Scripts/python.exe")
            message(FATAL_ERROR "Virtual Python setup unsuccessful!")
        endif()
    endif()
    find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter Development)
endif()
if( UNIX AND NOT APPLE )
    if ( USE_VIRT_PY )
        # Must be run in an environment with the venv activated
        set ( Python3_FIND_VIRTUAL ONLY )
        add_definitions( -DLINUX_VIRT_PY )
    endif()
    find_package(Python3 3.9 COMPONENTS Interpreter Development)
endif()


# gumbo-parser it is our main xhtml/html5 parser.
# We have an internal version because it diverges from Google's and GitHub's
# versions and neither want's our epub specific changes.
add_subdirectory( internal/gumbo )

# Add external process wrapper for appimages
if( APPIMAGE_BUILD )
    add_subdirectory( internal/exec_wrapper)
endif()

add_subdirectory( 3rdparty/ )
add_subdirectory( src/ )

