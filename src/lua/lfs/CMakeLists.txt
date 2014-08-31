project( lfs C ) 

include_directories (
    ${CMAKE_CURRENT_BINARY_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${LUA_INCLUDE_DIR}
)

if(APPLE)
    set(LINK_FLAGS "-undefined dynamic_lookup -all_load")
endif()

file( GLOB SOURCES src/*.c )

add_library( ${PROJECT_NAME} MODULE ${SOURCES}) 
add_dependencies (${PROJECT_NAME} lua lua-bin)
set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS ${LINK_FLAGS} PREFIX "" DEBUG_POSTFIX "" SUFFIX ".so" )

# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL" ) 
endif()
