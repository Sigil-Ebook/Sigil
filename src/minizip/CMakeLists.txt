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
)
if( ZLIB_FOUND )
    include_directories( ${ZLIB_INCLUDE_DIRS} )
else()
    include_directories( ${zlib_SOURCE_DIR} ${zlib_BINARY_DIR} )
endif()

# OS X doesn't have fopen64.
if(APPLE)
    add_definitions(-DUSE_FILE32API)
endif()

link_directories(${PROJECT_BINARY_DIR}/lib)
add_library(${PROJECT_NAME} ${SOURCES})

if( FORCE_BUNDLED_COPIES OR NOT ZLIB_FOUND )
    target_link_libraries( ${PROJECT_NAME} zlib )
else()
    target_link_libraries( ${PROJECT_NAME} ${ZLIB_LIBRARIES} )
endif()
