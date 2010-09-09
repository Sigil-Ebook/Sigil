########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 2.8 ) 

project( Xerces )

set( SOURCES )

# TODO: move both macros to special file
# Using the provided folder_path, recursively adds
# all the CPP, HPP and H source files to the SOURCES list.
macro( add_to_sources folder_path )
    file( GLOB_RECURSE TEMP ${folder_path}/*.cpp ${folder_path}/*.c )   
    list( LENGTH SOURCES len )    
    
    if ( ${len} EQUAL 0 )
        set( SOURCES ${TEMP} )        
    else()
        list( APPEND SOURCES ${TEMP} )
    endif()
endmacro()

# Using the provided folder_path, adds
# all the CPP, HPP and H source files to the SOURCES list.
# Only the folder_path is looked at, not the child folders. 
macro( add_to_sources_no_rec folder_path )
    file( GLOB TEMP ${folder_path}/*.cpp ${folder_path}/*.c )  
    list( LENGTH SOURCES len )    
    
    if ( ${len} EQUAL 0 )
        set( SOURCES ${TEMP} )        
    else()
        list( APPEND SOURCES ${TEMP} )
    endif()
endmacro()

add_to_sources( xercesc/dom )
add_to_sources( xercesc/framework )
add_to_sources( xercesc/internal )
add_to_sources( xercesc/parsers )
add_to_sources( xercesc/sax )
add_to_sources( xercesc/sax2 )
add_to_sources( xercesc/validators )
add_to_sources( xercesc/xinclude )

add_to_sources( xercesc/util/MsgLoaders/InMemory )
add_to_sources( xercesc/util/regx )

add_to_sources_no_rec( xercesc/util )
add_to_sources_no_rec( . )

# These files are included by hpp files and should not be directly compiled
file(GLOB to_remove xercesc/dom/impl/DOMDeepNodeListPool.c 
                    xercesc/framework/psvi/XSNamedMap.c 
                    xercesc/util/BaseRefVectorOf.c 
                    xercesc/util/CountedPointer.c 
                    xercesc/util/FlagJanitor.c 
                    xercesc/util/Hash2KeysSetOf.c 
                    xercesc/util/Janitor.c 
                    xercesc/util/KeyRefPair.c 
                    xercesc/util/KeyValuePair.c 
                    xercesc/util/LogicalPath.c 
                    xercesc/util/NameIdPool.c 
                    xercesc/util/RefArrayOf.c 
                    xercesc/util/RefArrayVectorOf.c 
                    xercesc/util/RefHash2KeysTableOf.c 
                    xercesc/util/RefHash3KeysIdPool.c 
                    xercesc/util/RefHashTableOf.c 
                    xercesc/util/RefStackOf.c 
                    xercesc/util/RefVectorOf.c 
                    xercesc/util/TransENameMap.c 
                    xercesc/util/ValueArrayOf.c 
                    xercesc/util/ValueHashTableOf.c 
                    xercesc/util/ValueStackOf.c 
                    xercesc/util/ValueVectorOf.c
                    )
                    
list(REMOVE_ITEM SOURCES ${to_remove} )

if( MSVC )
    add_to_sources( xercesc/util/Transcoders/Win32 )
    
    list( APPEND SOURCES
        xercesc/util/FileManagers/WindowsFileMgr.cpp
        xercesc/util/MutexManagers/WindowsMutexMgr.cpp )
        
    file( GLOB to_remove ./stricmp.c
                         ./strnicmp.c
                         )
    
    list( REMOVE_ITEM SOURCES ${to_remove} )
else()
    if( APPLE )
        add_to_sources( xercesc/util/Transcoders/MacOSUnicodeConverter )
    else()
        add_to_sources( xercesc/util/Transcoders/IconvGNU )
    endif()    
    
    list( APPEND SOURCES
        xercesc/util/FileManagers/PosixFileMgr.cpp
        xercesc/util/MutexManagers/PosixMutexMgr.cpp  )
endif()
        
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )

add_library( ${PROJECT_NAME} ${SOURCES} ) 

add_definitions( -DXERCES_STATIC_LIBRARY
                 -DXERCES_BUILDING_LIBRARY
                 -DXERCES_USE_MSGLOADER_INMEMORY
                 -DHAVE_LIMITS_H
                 -DHAVE_SYS_TIMEB_H
                 -DHAVE_FTIME
                 -DHAVE_WCSUPR
                 -DHAVE_WCSLWR
                 -DHAVE_WCSICMP
                 -DHAVE_WCSNICMP
                 -DHAVE_STRING_H
                 -DHAVE_STRCASECMP
                 -DHAVE_STRNCASECMP )

if( MSVC )
    add_definitions( -DUNICODE -D_UNICODE )
    add_definitions( -D_CRT_SECURE_NO_DEPRECATE
                     -DXERCES_PATH_DELIMITER_BACKSLASH
                     -DXERCES_USE_TRANSCODER_WINDOWS
                     -DXERCES_USE_FILEMGR_WINDOWS   
                     -DXERCES_USE_MUTEXMGR_WINDOWS
                     -DHAVE_STRICMP
                     -DHAVE_STRNICMP
                   )
                     
    # xerces wants to disable these warnings
    add_definitions( /wd4996 )
    add_definitions( /wd4355 )
    
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-")
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
    set_target_properties( ${PROJECT_NAME} PROPERTIES STATIC_LIBRARY_FLAGS "/LTCG" )
    
else()
    add_definitions( -DXERCES_USE_FILEMGR_POSIX
                     -DXERCES_USE_TRANSCODER_GNUICONV
                     -DXERCES_USE_MUTEXMGR_POSIX 
                   )                     
endif()
