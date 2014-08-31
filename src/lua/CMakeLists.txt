project( lua ) 

SET( LUA_TOPLEVEL_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "" )

if (NOT NO_BUNDLED_LUA)
    add_subdirectory( lua )
    add_subdirectory( lfs )
endif ()
