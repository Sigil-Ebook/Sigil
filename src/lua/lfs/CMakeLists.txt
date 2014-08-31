project( lfs C ) 

cmake_policy( SET CMP0042 OLD)

include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LUA_INCLUDE_DIR}
)

file( GLOB SOURCES src/*.c )

set( CMAKE_DEBUG_POSTFIX "" )
add_library( ${PROJECT_NAME} SHARED ${SOURCES}) 
target_link_libraries (${PROJECT_NAME} lua)
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
add_dependencies (${PROJECT_NAME} lua)
add_dependencies (${PROJECT_NAME} lua-bin)

if(APPLE)
    add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.dylib" "./${PROJECT_NAME}.dylib" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.dylib")
    add_custom_command (TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/liblua.dylib" "./liblua.dylib" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.dylib")
endif()


# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
