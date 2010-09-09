########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

# The interesting parts of Boost have been extracted using 
# the BCP tool:
#   http://www.boost.org/doc/libs/1_44_0/tools/bcp/doc/html/index.html
#    
# bcp call: bcp boost/bind.hpp boost/lambda/lambda.hpp boost/exception/all.hpp boost/tuple/tuple_io.hpp boost/tuple/tuple_comparison.hpp boost/filesystem.hpp boost/regex.hpp boost/foreach.hpp boost/program_options.hpp boost/smart_ptr.hpp boost/algorithm/string_regex.hpp boost/thread.hpp boost/unordered_map.hpp boost/unordered_set.hpp boost/format.hpp ../BoostParts

# WARNING: When doing this from fresh boost sources, note that
# in the file /boost/config/user.hpp the BOOST_ALL_NO_LIB
# define has been uncommented. Otherwise, we will get linker 
# errors on MSVC because of boost auto-linking.

# NOTE: boost/filesystem.hpp was also changed to force the use
# of the v3 lib

cmake_minimum_required( VERSION 2.8 ) 

project( BoostParts )

set( SOURCES  
    libs/date_time/src/gregorian/date_generators.cpp   
    libs/date_time/src/gregorian/gregorian_types.cpp
    libs/date_time/src/gregorian/greg_month.cpp
    libs/date_time/src/gregorian/greg_names.hpp
    libs/date_time/src/gregorian/greg_weekday.cpp
    libs/date_time/src/posix_time/posix_time_types.cpp
    libs/thread/src/tss_null.cpp
    libs/filesystem/v3/src/operations.cpp
    libs/filesystem/v3/src/path.cpp
    libs/filesystem/v3/src/portability.cpp
    libs/filesystem/v3/src/utf8_codecvt_facet.cpp
    libs/filesystem/v3/src/windows_file_codecvt.cpp
    libs/filesystem/v3/src/path_traits.cpp
    libs/filesystem/v3/src/unique_path.cpp
    libs/filesystem/v3/src/codecvt_error_category.cpp
    libs/program_options/src/cmdline.cpp
    libs/program_options/src/convert.cpp
    libs/program_options/src/parsers.cpp
    libs/program_options/src/split.cpp
    libs/program_options/src/value_semantic.cpp
    libs/program_options/src/winmain.cpp
    libs/program_options/src/config_file.cpp
    libs/program_options/src/options_description.cpp
    libs/program_options/src/positional_options.cpp
    libs/program_options/src/utf8_codecvt_facet.cpp
    libs/program_options/src/variables_map.cpp
    libs/regex/src/cpp_regex_traits.cpp
    libs/regex/src/cregex.cpp
    libs/regex/src/c_regex_traits.cpp
    libs/regex/src/fileiter.cpp
    libs/regex/src/icu.cpp
    libs/regex/src/instances.cpp
    libs/regex/src/posix_api.cpp
    libs/regex/src/regex.cpp
    libs/regex/src/regex_debug.cpp
    libs/regex/src/regex_raw_buffer.cpp
    libs/regex/src/regex_traits_defaults.cpp
    libs/regex/src/static_mutex.cpp
    libs/regex/src/usinstances.cpp
    libs/regex/src/w32_regex_traits.cpp
    libs/regex/src/wc_regex_traits.cpp
    libs/regex/src/wide_posix_api.cpp
    libs/regex/src/winstances.cpp
    libs/smart_ptr/src/sp_collector.cpp
    libs/smart_ptr/src/sp_debug_hooks.cpp
    libs/system/src/error_code.cpp
    ) 
    
set( WINSOURCES 
    libs/thread/src/win32/thread.cpp
    libs/thread/src/win32/timeconv.inl
    libs/thread/src/win32/tss_dll.cpp
    libs/thread/src/win32/tss_pe.cpp
    )
    
set( UNIXSOURCES
    libs/thread/src/pthread/once.cpp
    libs/thread/src/pthread/thread.cpp
    libs/thread/src/pthread/timeconv.inl
    )
    
if ( WIN32 )
    list( APPEND SOURCES ${WINSOURCES} )
else()
    list( APPEND SOURCES ${UNIXSOURCES} )
endif()
    
#############################################################################

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
            
add_library( BoostParts ${SOURCES} ) 
 
#############################################################################
 
# Special compiler and linker flags for MSVC
if( MSVC )
    add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t-  )
    set( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
endif()
