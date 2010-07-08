########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

# The interesting parts of Boost have been extracted using 
# the BCP tool:
#   http://www.boost.org/doc/libs/1_41_0/tools/bcp/bcp.html
#    
# bcp call: bcp bind lambda exception tuple /foo
#
# WARNING: When doing this from fresh boost sources, note that
# in the file /boost/config/user.hpp the BOOST_ALL_NO_LIB
# define has been uncommented. Otherwise, we will get linker 
# errors on MSVC because of boost auto-linking.


cmake_minimum_required( VERSION 2.8 ) 

project( BoostParts )

set( SOURCES  
    libs/date_time/src/gregorian/date_generators.cpp   
    libs/date_time/src/gregorian/gregorian_types.cpp
    libs/date_time/src/gregorian/greg_month.cpp
    libs/date_time/src/gregorian/greg_names.hpp
    libs/date_time/src/gregorian/greg_weekday.cpp
    libs/date_time/src/posix_time/posix_time_types.cpp
    libs/thread/src/tss_null.cpp    
    ) 
    
set( WINSOURCES 
    libs/thread/src/win32/thread.cpp
    libs/thread/src/win32/timeconv.inl
    libs/thread/src/win32/tss_dll.cpp
    libs/thread/src/win32/tss_pe.cpp
    )
    
set( UNIXSOURCES
    libs/thread/src/pthread/once.cpp
    libs/thread/src/pthread/thread.cpp
    libs/thread/src/pthread/timeconv.inl
    )
    
if ( WIN32 )
    list( APPEND SOURCES ${WINSOURCES} )
else()
    list( APPEND SOURCES ${UNIXSOURCES} )
endif()
    
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
			
add_library( BoostParts ${SOURCES} ) 
 
# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
	set_target_properties( BoostParts PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
