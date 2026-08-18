cmake_minimum_required(VERSION 3.16)
project(lexborcss VERSION 1.0 LANGUAGES C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( LEXBOR_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "" )
set( LEXBOR_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}  CACHE INTERNAL "" )

set(SOURCES
    lexbor/core/array.c
    lexbor/core/array.h
    lexbor/core/array_obj.c
    lexbor/core/array_obj.h
    lexbor/core/avl.c
    lexbor/core/avl.h
    lexbor/core/base.h
    lexbor/core/bst.c
    lexbor/core/bst.h
    lexbor/core/bst_map.c
    lexbor/core/bst_map.h
    lexbor/core/conv.c
    lexbor/core/conv.h
    lexbor/core/core.h
    lexbor/core/def.h
    lexbor/core/diyfp.c
    lexbor/core/diyfp.h
    lexbor/core/dobject.c
    lexbor/core/dobject.h
    lexbor/core/dtoa.c
    lexbor/core/dtoa.h
    lexbor/core/fs.h
    lexbor/core/hash.c
    lexbor/core/hash.h
    lexbor/core/in.c
    lexbor/core/in.h
    lexbor/core/lexbor.h
    lexbor/core/mem.c
    lexbor/core/mem.h
    lexbor/core/mraw.c
    lexbor/core/mraw.h
    lexbor/core/perf.h
    lexbor/core/plog.c
    lexbor/core/plog.h
    lexbor/core/print.c
    lexbor/core/print.h
    lexbor/core/sbst.h
    lexbor/core/serialize.c
    lexbor/core/serialize.h
    lexbor/core/shs.c
    lexbor/core/shs.h
    lexbor/core/str.c
    lexbor/core/str.h
    lexbor/core/str_res.h
    lexbor/core/strtod.c
    lexbor/core/strtod.h
    lexbor/core/swar.h
    lexbor/core/types.h
    lexbor/core/utils.c
    lexbor/core/utils.h
    lexbor/css/at_rule.c
    lexbor/css/at_rule.h
    lexbor/css/at_rule/const.h
    lexbor/css/at_rule/res.h
    lexbor/css/at_rule/state.c
    lexbor/css/at_rule/state.h
    lexbor/css/at_rule/types.h
    lexbor/css/base.h
    lexbor/css/blank.c
    lexbor/css/blank.h
    lexbor/css/css.c
    lexbor/css/css.h
    lexbor/css/declaration.c
    lexbor/css/declaration.h
    lexbor/css/log.c
    lexbor/css/log.h
    lexbor/css/parser.c
    lexbor/css/parser.h
    lexbor/css/property.c
    lexbor/css/property.h
    lexbor/css/property/const.h
    lexbor/css/property/res.h
    lexbor/css/property/state.c
    lexbor/css/property/state.h
    lexbor/css/property/types.h
    lexbor/css/rule.c
    lexbor/css/rule.h
    lexbor/css/selectors/base.h
    lexbor/css/selectors/pseudo.c
    lexbor/css/selectors/pseudo.h
    lexbor/css/selectors/pseudo_const.h
    lexbor/css/selectors/pseudo_res.h
    lexbor/css/selectors/pseudo_state.c
    lexbor/css/selectors/pseudo_state.h
    lexbor/css/selectors/selector.c
    lexbor/css/selectors/selector.h
    lexbor/css/selectors/selectors.c
    lexbor/css/selectors/selectors.h
    lexbor/css/selectors/state.c
    lexbor/css/selectors/state.h
    lexbor/css/state.c
    lexbor/css/state.h
    lexbor/css/stylesheet.c
    lexbor/css/stylesheet.h
    lexbor/css/syntax/anb.c
    lexbor/css/syntax/anb.h
    lexbor/css/syntax/base.h
    lexbor/css/syntax/parser.c
    lexbor/css/syntax/parser.h
    lexbor/css/syntax/res.h
    lexbor/css/syntax/state.c
    lexbor/css/syntax/state.h
    lexbor/css/syntax/state_res.h
    lexbor/css/syntax/syntax.c
    lexbor/css/syntax/syntax.h
    lexbor/css/syntax/token.c
    lexbor/css/syntax/token.h
    lexbor/css/syntax/token_res.h
    lexbor/css/syntax/tokenizer.c
    lexbor/css/syntax/tokenizer.h
    lexbor/css/syntax/tokenizer/error.c
    lexbor/css/syntax/tokenizer/error.h
    lexbor/css/unit.c
    lexbor/css/unit.h
    lexbor/css/unit/const.h
    lexbor/css/unit/res.h
    lexbor/css/value.c
    lexbor/css/value.h
    lexbor/css/value/const.h
    lexbor/css/value/res.h
)

if( WIN32 )
    set(ADDL_SOURCES
        lexbor/ports/windows_nt/lexbor/core/fs.c
        lexbor/ports/windows_nt/lexbor/core/memory.c
        lexbor/ports/windows_nt/lexbor/core/perf.c
    )
endif()

if( UNIX )
    set(ADDL_SOURCES
        lexbor/ports/posix/lexbor/core/fs.c
        lexbor/ports/posix/lexbor/core/memory.c
        lexbor/ports/posix/lexbor/core/perf.c
    )
    set( PROJECT_C_FLAGS ${PROJECT_C_FLAGS} "-Wall -Wextra -Wno-unused-parameter -Wno-unused-function" )
endif()

set( ALL_SOURCES ${SOURCES} ${ADDL_SOURCES} )


ADD_LIBRARY(${PROJECT_NAME} ${ALL_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}
)

target_compile_definitions(lexborcss PUBLIC
    LEXBOR_STATIC
)

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PROJECT_C_FLAGS}")

if( MSVC )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
        set( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
