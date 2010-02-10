########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################


cmake_minimum_required( VERSION 2.8 ) 

project( ZipArchive )

file( GLOB SOURCES *.cpp *.h )
file( GLOB SOURCES_ZLIB zlib/*.c zlib/*.h )
file( GLOB SOURCES_BZIP2 bzip2/*.c bzip2/*.h )

# Adds folders for Visual Studio solution explorer
SOURCE_GROUP( zlib FILES ${SOURCES_ZLIB} )
SOURCE_GROUP( bzip2 FILES ${SOURCES_BZIP2} )

list( APPEND SOURCES ${SOURCES_ZLIB} ${SOURCES_BZIP2} )

# Adding resource (RC) files for windows
if (MSVC)
	list( APPEND SOURCES ZipArchive.rc)
endif()

add_library( ZipArchive ${SOURCES} ) 

# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
	set_target_properties( ZipArchive PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()

# Special compiler and linker flags for GCC
if( UNIX )
	add_definitions( -D_ZIP_SYSTEM_LINUX -D_ZIP_BZIP2_INTERNAL )
endif()


