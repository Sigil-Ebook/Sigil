########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 3.18 ) 

project(wrapper_exec C )

set( SOURCES exec.c )

add_library( wrapper_exec SHARED ${SOURCES} )
set_target_properties(wrapper_exec PROPERTIES 
    PREFIX ""
    OUTPUT_NAME "wrapper_exec"
    SUFFIX ".so"
)

if( UNIX AND NOT APPLE )
        # cmake sets -fPIC on by default with shared libs
	set( CMAKE_C_RELEASE_FLAGS "${CMAKE_C_FLAGS} -W1 -O2 -DDEBUG=1 -ldl" )
	set( CMAKE_C_DEBUG_FLAGS "${CMAKE_C_FLAGS} -W1 -O2 -DDEBUG=1 -ldl" )
endif()
