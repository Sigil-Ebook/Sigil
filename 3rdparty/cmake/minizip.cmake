cmake_minimum_required(VERSION 3.0)

project(minizip C)

set( MINIZIP_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "" )
set( MINIZIP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME} CACHE INTERNAL "" )

SET(PROJECT_C_FLAGS "")

SET(SOURCES
    ${PROJECT_NAME}/ioapi.c
    ${PROJECT_NAME}/mztools.c
    ${PROJECT_NAME}/unzip.c
    ${PROJECT_NAME}/zip.c
)

if(WIN32)
    list(APPEND SOURCES
        ${PROJECT_NAME}/iowin32.c
    )
endif()

# OS X doesn't have fopen64.
if(APPLE)
    set(PROJECT_C_FLAGS "${PROJECT_C_FLAGS} -DUSE_FILE32API")
endif()

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}
    ${ZLIB_INCLUDE_DIRS}
)
target_link_libraries( ${PROJECT_NAME} ${ZLIB_LIBRARIES} )
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PROJECT_C_FLAGS}")

# Special compiler and linker flags for MSVC
if( MSVC )
	set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
	set( CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
