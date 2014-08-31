project( lua C ) 

cmake_policy( SET CMP0022 OLD)
cmake_policy( SET CMP0038 OLD)
cmake_policy( SET CMP0042 OLD)

SET( LUA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src CACHE STRING "" )

include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

SET(RL "")
if( APPLE )
    set( CMAKE_C_FLAGS "-DLUA_USE_MACOSX" )
    SET(RL "readline")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set( CMAKE_C_FLAGS "-DLUA_USE_LINUX" )
    SET(RL "readline")
elseif(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
    set( CMAKE_C_FLAGS "-DLUA_USE_POSIX -DLUA_USE_DLOPEN" )
endif()

file( GLOB SOURCES src/*.c )
list( REMOVE_ITEM SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lua.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/luac.c
)

set( CMAKE_DEBUG_POSTFIX "" )
add_library( ${PROJECT_NAME} SHARED ${SOURCES}) 
target_link_libraries( ${PROJECT_NAME} ${PROJECT_NAME} ${RL} )

add_executable( lua-bin src/lua.c) 
target_link_libraries( lua-bin ${PROJECT_NAME} lua)
add_dependencies (lua-bin lua)

if(APPLE)
    add_custom_command (TARGET lua-bin POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/liblua.dylib" "./liblua.dylib" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lua-bin")
endif()

# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
