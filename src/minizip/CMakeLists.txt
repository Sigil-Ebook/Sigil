cmake_minimum_required(VERSION 2.8)

project(minizip C)

file(GLOB SOURCES *.c *.h)

# Remove the Windows specific io file from the build list on non-Windows
# platforms.
if(NOT WIN32)
    file(GLOB TO_REMOVE iowin32.*)
    list(REMOVE_ITEM SOURCES ${TO_REMOVE})
endif(NOT WIN32)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${ZLIB_INCLUDE_DIRS}
)

# OS X doesn't have fopen64.
if(APPLE)
    add_definitions(-DUSE_FILE32API)
endif()

link_directories(${PROJECT_BINARY_DIR}/lib)
add_library(${PROJECT_NAME} ${SOURCES})

target_link_libraries( ${PROJECT_NAME} ${ZLIB_LIBRARIES} )

# Special compiler and linker flags for MSVC
if( MSVC )
	add_definitions( /D_CRT_SECURE_NO_WARNINGS /DUNICODE /D_UNICODE /Zc:wchar_t-  )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()

