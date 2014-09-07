project( C ) 

SET( LUA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE STRING "" )

include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

SET(RL "")
if( APPLE )
    set( CMAKE_C_FLAGS "-DLUA_USE_MACOSX" )
    SET(RL "readline")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    FIND_PACKAGE( Readline )
    set( CMAKE_C_FLAGS "-DLUA_USE_LINUX" )
    SET(RL "readline")
elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    set( CMAKE_C_FLAGS "-DLUA_USE_POSIX -DLUA_USE_DLOPEN" )
endif()

file( GLOB SOURCES src/*.c )
# Can't build the lib with the files that programs that use the lb.
list( REMOVE_ITEM SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lua.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/luac.c
)

add_library( lua ${SOURCES}) 
target_link_libraries( lua ${RL} )
if( UNIX AND NOT APPLE )
    target_link_libraries (lua m dl )
endif()
set_target_properties( lua PROPERTIES DEBUG_POSTFIX "" )

# Lua intepter program.
add_executable( lua-bin src/lua.c) 
target_link_libraries( lua-bin lua)
set_target_properties( lua-bin PROPERTIES OUTPUT_NAME lua )
add_dependencies (lua-bin lua)

# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( lua PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
