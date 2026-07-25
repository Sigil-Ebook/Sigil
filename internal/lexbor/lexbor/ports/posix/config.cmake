if(NOT DEFINED LEXBOR_OPTIMIZATION_LEVEL)
    set(LEXBOR_OPTIMIZATION_LEVEL "-O2")
endif()

if(NOT DEFINED LEXBOR_C_FLAGS)
    set(LEXBOR_C_FLAGS "-Wall -pedantic -pipe -std=c99 -fPIC")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-D_POSIX_C_SOURCE=199309L)
endif()

if(LEXBOR_WITH_PERF)
    add_definitions(-DLEXBOR_WITH_PERF)
endif()
