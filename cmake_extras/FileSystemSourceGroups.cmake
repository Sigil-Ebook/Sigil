
# Accepts a variable holding the source files
# and creates source groups (for VS, Xcode etc)
# that replicate the folder hierarchy on disk
macro( create_source_groups source_files_variable )
	foreach( source_file ${${source_files_variable}} )
		string( REGEX REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" relative_directory "${source_file}")
		string( REGEX REPLACE "[\\\\/][^\\\\/]*$" "" relative_directory "${relative_directory}")
		string( REGEX REPLACE "^[\\\\/]" "" relative_directory "${relative_directory}")

		if( WIN32 )
			string( REGEX REPLACE "/" "\\\\" relative_directory "${relative_directory}" )
		endif( WIN32 )

		source_group( "${relative_directory}" FILES ${source_file} )
	endforeach()
endmacro()