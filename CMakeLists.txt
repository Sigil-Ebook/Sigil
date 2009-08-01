
cmake_minimum_required( VERSION 2.6 ) 

project( Sigil ) 

set( CMAKE_DEBUG_POSTFIX "d" )
set( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin )
set( LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib )

add_subdirectory( src/tidyLib )

#TODO: Switch sigil and ziparchive to cmake and then add them here.