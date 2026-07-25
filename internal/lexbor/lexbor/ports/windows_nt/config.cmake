if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
    if(NOT DEFINED LEXBOR_OPTIMIZATION_LEVEL)
        set(LEXBOR_OPTIMIZATION_LEVEL "/O2")
    endif()

    add_definitions(/wd4100 /wd4255 /wd4820 /wd4668)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    if(NOT DEFINED LEXBOR_OPTIMIZATION_LEVEL)
        set(LEXBOR_OPTIMIZATION_LEVEL "-O2")
    endif()

    if(NOT DEFINED LEXBOR_C_FLAGS)
        set(LEXBOR_C_FLAGS "-Wall -pedantic -pipe -std=c99 -Wno-pedantic-ms-format")
    endif()
endif()

message(STATUS "Set Windows definitions")
