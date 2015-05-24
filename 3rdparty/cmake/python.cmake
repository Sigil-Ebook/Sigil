cmake_minimum_required(VERSION 3.0)

project(python C)

include(ExternalProject)

# This needs to be updated when ever the bundled python version changes.
set( PYTHON_LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-prefix/lib/libpython3.4m.a CACHE INTERNAL "" )
set( PYTHON_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-prefix/include/python3.4m CACHE INTERNAL "" )
SET(PROJECT_C_FLAGS "")

ExternalProject_Add(${PROJECT_NAME}
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/python
    CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/python/configure --prefix=<INSTALL_DIR>
    BUILD_COMMAND ${MAKE})
