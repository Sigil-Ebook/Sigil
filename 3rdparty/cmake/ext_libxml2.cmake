include(ExternalProject)

MESSAGE(STATUS "The git, libtool, autoconf, and automake packages are required for compiling libxml2")

find_program(GIT git)
if(NOT GIT)
    message(FATAL_ERROR "The git program is needed for compiling libxml2. Install the git package.")
endif()

find_program(LIBTOOL libtoolize)
if (NOT LIBTOOL)
    message(FATAL_ERROR "The libtoolize program is needed for compiling libxml2. Install the libtool package.")
endif()

find_program(AUTOCONF autoreconf)
if (NOT AUTOCONF)
    message(FATAL_ERROR "The autoreconf program is needed for compiling libxml2. Install the autoconf package.")
endif()

find_program(AUTOMAKE automake)
if (NOT AUTOMAKE)
    message(FATAL_ERROR "The automake program is needed for compiling libxml2. Install the automake package.")
endif()

ExternalProject_Add(
    libxml2

    GIT_REPOSITORY "https://github.com/GNOME/libxml2.git"
    GIT_TAG "d8083bf77955b7879c1290f0c0a24ab8cc70f7fb"
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/libxml2
    UPDATE_COMMAND ""
    PATCH_COMMAND patch < ${CMAKE_CURRENT_SOURCE_DIR}/extra/libxml2/libxml2_bug_fix.patch
    CONFIGURE_COMMAND ${CMAKE_CURRENT_BINARY_DIR}/libxml2/autogen.sh --prefix=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libxml2 --disable-static --enable-shared --without-python
    TEST_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
)

ExternalProject_Add_Step(
    libxml2 CopyToLib
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/libxml2/.libs/libxml2.so.2.9.4 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libsigilxml2.so.2.9.4
    COMMAND strip --strip-unneeded ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libsigilxml2.so.2.9.4
    COMMAND ln -sfr ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libsigilxml2.so.2.9.4 ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libxml2.so.2
    DEPENDEES build
)
