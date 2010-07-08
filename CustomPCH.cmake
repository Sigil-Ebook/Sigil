macro( precompiled_header sources includes target_name )

    # MSVC precompiled headers cmake code
    if ( MSVC )
        set_source_files_properties( stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h" )
        set_source_files_properties( stdafx.h   PROPERTIES COMPILE_FLAGS "/Yustdafx.h" )
            
        foreach( src_file ${${sources}} )
            if( ${src_file} MATCHES ".*cpp$" )
                set_source_files_properties( ${src_file} PROPERTIES COMPILE_FLAGS "/Yustdafx.h" )
            endif()
        endforeach()

        # stdafx.cpp has to come before stdafx.h, 
        # otherwise we get a linker error...
        list( INSERT ${sources} 0 stdafx.h )
        list( INSERT ${sources} 0 stdafx.cpp )

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

        # Include the BoostParts directory
        #list( APPEND compile_flags "-I${BoostParts_SOURCE_DIR}" )

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
        
        # Finally, build the precompiled header
        add_custom_target(  ${target_name} ALL 
                            COMMAND ${CMAKE_CXX_COMPILER} ${compile_flags} ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h -o stdafx.h.gch
                            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                            VERBATIM )
    endif() 
endmacro()
