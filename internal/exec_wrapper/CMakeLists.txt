########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 3.18 ) 

project(exec_wrapper C )

set( SOURCES exec.c )

add_library( exec_wrapper SHARED ${SOURCES} )
set_target_properties(exec_wrapper PROPERTIES 
    PREFIX ""
    OUTPUT_NAME "exec_wrapper"
    SUFFIX ".so"
)

if( UNIX AND NOT APPLE )
    # cmake sets -fPIC on by default with shared libs
	# set( CMAKE_C_FLAGS "-fPIC -O2 -DDEBUG=1 -ldl" )
	set( CMAKE_C_FLAGS "-fPIC -O2 -ldl" )
endif()
