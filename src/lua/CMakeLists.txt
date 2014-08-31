project( lua ) 

if (NOT NO_BUNDLED_LUA)
    add_subdirectory( lua )
    add_subdirectory( lfs )
endif ()
