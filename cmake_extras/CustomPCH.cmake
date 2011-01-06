
# DON'T FORGET to include ${CMAKE_CURRENT_SOURCE_DIR} in include_directories for the compiler
# to see the header, and ${CMAKE_CURRENT_BINARY_DIR} for the compiler to see the GCC PCH 

# "sources" - unexpanded cmake variable holding all the source files
# "includes" - unexpanded cmake variable holding all include paths the PCH needs to know about
# "target_name" - the name of the a special target used to build the PCH for GCC
# "header_name" - the name of the PCH header, without the extension; "stdafx" or something similar;
#                  note that the source file compiling the header needs to have the same name 
macro( precompiled_header sources includes target_name header_name )

    # MSVC precompiled headers cmake code
    if ( MSVC )
        set_source_files_properties( ${header_name}.cpp PROPERTIES COMPILE_FLAGS "/Yc${header_name}.h" )
            
        foreach( src_file ${${sources}} )
            if( ${src_file} MATCHES ".*cpp$" )
                set_source_files_properties( ${src_file} PROPERTIES COMPILE_FLAGS "/Yu${header_name}.h" )
            endif()
        endforeach()

        # ${header_name}.cpp has to come before ${header_name}.h, 
        # otherwise we get a linker error...
        list( INSERT ${sources} 0 ${header_name}.h )
        list( INSERT ${sources} 0 ${header_name}.cpp )

    # GCC precompiled headers cmake code
    # We don't do this on Macs since GCC there goes haywire
    # when you try to generate a PCH with two "-arch" flags
    elseif( CMAKE_COMPILER_IS_GNUCXX AND NOT APPLE )

        # Get the compiler flags for this build type
        string( TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" flags_for_build_name )
        set( compile_flags ${${flags_for_build_name}} )
        
        # Add all the Qt include directories
        foreach( item ${${includes}} )
            list( APPEND compile_flags "-I${item}" )
        endforeach()

        # Get the list of all build-independent preprocessor definitions
        get_directory_property( defines_global COMPILE_DEFINITIONS )
        list( APPEND defines ${defines_global} )

        # Get the list of all build-dependent preprocessor definitions
        string( TOUPPER "COMPILE_DEFINITIONS_${CMAKE_BUILD_TYPE}" defines_for_build_name )
        get_directory_property( defines_build ${defines_for_build_name} )
        list( APPEND defines ${defines_build} )

        # Add the "-D" prefix to all of them
        foreach( item ${defines} )
            list( APPEND all_define_flags "-D${item}" )
        endforeach()
        
        list( APPEND compile_flags ${all_define_flags} ) 
        
        # Prepare the compile flags var for passing to GCC
        separate_arguments( compile_flags )
        
        # Finally, build the precompiled header.
        # We don't add the buil command to add_custom_target
        # because that would force a PCH rebuild even when
        # the ${header_name}.h file hasn't changed. We add it to
        # a special add_custom_command to work around this problem.        
        add_custom_target( ${target_name} ALL
                           DEPENDS ${header_name}.h.gch
                         )
        
        add_custom_command( OUTPUT ${header_name}.h.gch 
                            COMMAND ${CMAKE_CXX_COMPILER} ${compile_flags} ${CMAKE_CURRENT_SOURCE_DIR}/${header_name}.h -o ${header_name}.h.gch
                            MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/${header_name}.h
                            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                            VERBATIM )
    endif() 
endmacro()

# Xcode PCH support. Has to be called *AFTER* the target is created.  
# "header_name" - the name of the PCH header, without the extension; "stdafx" or something similar;
#                  note that the source file compiling the header needs to have the same name 
macro( xcode_pch header_name )
    if( APPLE )                   
        set_target_properties(
            ${PROJECT_NAME} 
            PROPERTIES
            XCODE_ATTRIBUTE_GCC_PREFIX_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/${PCH_NAME}.h"
            XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
        )
    endif()
endmacro() 
