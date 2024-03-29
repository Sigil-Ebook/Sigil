project(3rdparty)

# Note: All files in the cmake dir will be included as if they were in this file.
#       The paths in each .cmake file are relative to this directory not the
#       cmake directory.

IF(MSVC)
    add_definitions( /D_CRT_SECURE_NO_WARNINGS /DUNICODE /D_UNICODE )
ENDIF()

if( NOT USE_SYSTEM_LIBS OR NOT ZLIB_FOUND )
    MESSAGE(STATUS "Using Bundled ZLIB")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/zlib.cmake)
else ()
    MESSAGE(STATUS "Using System ZLIB")
endif()

if( NOT USE_SYSTEM_LIBS OR NOT MINIZIP_FOUND )
    MESSAGE(STATUS "Using Bundled MiniZip")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/minizip.cmake)
else ()
    MESSAGE(STATUS "Using System MiniZip")
endif()

if( NOT USE_SYSTEM_LIBS OR NOT HUNSPELL_FOUND )
    MESSAGE(STATUS "Using Bundled Hunspell")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/hunspell.cmake)
else ()
    MESSAGE(STATUS "Using System Hunspell")
endif()

if( NOT USE_SYSTEM_LIBS OR NOT PCRE2_FOUND OR APPLE )
    MESSAGE(STATUS "Using Bundled PCRE2")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/pcre2.cmake)
else ()
    MESSAGE(STATUS "Using System PCRE2")
endif()

if (SYSTEM_LIBS_REQUIRED)
    if (NOT ZLIB_FOUND OR NOT MINIZIP_FOUND OR NOT HUNSPELL_FOUND OR (NOT APPLE AND NOT PCRE2_FOUND))
        #if (NOT ZLIB_FOUND OR NOT MINIZIP_FOUND OR (NOT APPLE AND NOT PCRE2_FOUND))
        MESSAGE(FATAL_ERROR "Could not find all required system libraries...")
    endif()
endif()

if( (UNIX AND NOT APPLE) AND BUILD_PATCHED_LIBXML2 )
      include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ext_libxml2.cmake)
endif()
