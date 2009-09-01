########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 2.6 ) 

# We use the lower case name
# on UNIX systems other than Mac OS X
if ( WIN32 OR APPLE )
    project( Sigil )
else()
    project( sigil )
endif()

find_package( Qt4 REQUIRED )

set( QT_USE_QTWEBKIT TRUE )
set( QT_USE_QTSVG TRUE )
set( QT_USE_QTXML TRUE ) 
set( QT_USE_QTMAIN TRUE )

include( ${QT_USE_FILE} )

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS    AddMetadata.h 
                        About.h
                        BookViewEditor.h
                        CodeViewEditor.h 
                        TOCEditor.h 
                        MainWindow.h
                        MetaEditor.h )

set( HEADERS Book.h 
            BookNormalization.h 
            CleanSource.h
            constants.h 
            ExportEPUB.h 
            Exporter.h 
            ExporterFactory.h 
            ExportSGF.h 
            FolderKeeper.h 
            Headings.h
            ImportEPUB.h 
            Importer.h 
            ImporterFactory.h 
            ImportHTML.h 
            ImportTXT.h 
            ImportSGF.h
            LineNumberArea.h
            Metadata.h
            NCXWriter.h 
            OPFWriter.h 
            SigilMarkup.h 
            stdafx.h
            Utility.h
            ViewEditor.h
            XHTMLHighlighter.h 
            XHTMLDoc.h
            XMLWriter.h  )
            
set( CPP_FILES  AddMetadata.cpp 
            About.cpp 
            Book.cpp 
            BookNormalization.cpp 
            BookViewEditor.cpp
            CleanSource.cpp 
            CodeViewEditor.cpp 
            ExportEPUB.cpp 
            ExporterFactory.cpp 
            ExportSGF.cpp
            FolderKeeper.cpp 
            Headings.cpp
            ImportEPUB.cpp 
            ImporterFactory.cpp 
            ImportHTML.cpp 
            ImportTXT.cpp 
            ImportSGF.cpp 
            main.cpp 
            LineNumberArea.cpp 
            MainWindow.cpp 
            Metadata.cpp 
            MetaEditor.cpp 
            NCXWriter.cpp 
            OPFWriter.cpp 
            SigilMarkup.cpp 
            TOCEditor.cpp 
            Utility.cpp
            XHTMLHighlighter.cpp
            XHTMLDoc.cpp
            XMLWriter.cpp  )
            
set( UI_FILES   Form_Files/main.ui 
                Form_Files/MetaEditor.ui 
                Form_Files/AddMetadata.ui 
                Form_Files/About.ui 
                Form_Files/TOCEditor.ui )
            
set( QRC_FILES  Resource_Files/main/main.qrc
                Resource_Files/About/About.qrc
                Resource_Files/data/data.qrc
                Resource_Files/javascript/javascript.qrc )

# Runs MOC on specifed files
QT4_WRAP_CPP( MOC_FILES_CPP ${QOBJECT_HEADERS} )

# Runs UIC on specified files
QT4_WRAP_UI( UI_FILES_H ${UI_FILES} )

# Runs RCC on specified files
QT4_ADD_RESOURCES( QRC_FILES_CPP ${QRC_FILES} )


# VS precompiled headers cmake code
if ( MSVC )
    set_source_files_properties( stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h" )
        
    foreach( src_file ${CPP_FILES} )
        set_source_files_properties( ${src_file} PROPERTIES COMPILE_FLAGS "/Yustdafx.h" )
    endforeach()
    
    list( APPEND CPP_FILES stdafx.cpp )
endif()

# Adds folders for Visual Studio solution explorer
SOURCE_GROUP( "Generated Files" FILES ${MOC_FILES_CPP} ${UI_FILES_H} ${QRC_FILES_CPP} )
SOURCE_GROUP( "Form Files" FILES ${UI_FILES} )
SOURCE_GROUP( "Resource Files" FILES ${QRC_FILES} )

set( SOURCES ${CPP_FILES} ${HEADERS} ${QOBJECT_HEADERS} ${MOC_FILES_CPP} ${UI_FILES_H} ${QRC_FILES_CPP} )

# Adding resource (RC) files for Windows
if ( MSVC )
    list( APPEND SOURCES Resource_Files/icon/icon.rc )
    SOURCE_GROUP( "Resource Files" FILES Resource_Files/icon/icon.rc )
endif()

# Apple bundle configuration
if( APPLE )
   set( MACOSX_BUNDLE_INFO_STRING "Sigil" )
   set( MACOSX_BUNDLE_BUNDLE_NAME "Sigil" )
   set( MACOSX_BUNDLE_ICON_FILE "Sigil.icns" )

   # Copy the icon file
   exec_program( "mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/Resources")
   exec_program( "cp ${PROJECT_SOURCE_DIR}/Resource_Files/icon/Sigil.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/Resources")
endif()

# We need to pick up the ui*.h and moc*.cpp files,
# and the headers for the linked-to libraries
include_directories( ${CMAKE_CURRENT_BINARY_DIR} ../ZipArchive ../tidyLib )

link_directories ( ${PROJECT_BINARY_DIR}/lib ) 

# We make bundles for Mac OS X
if ( APPLE )
    add_executable( ${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} )
else()
    add_executable( ${PROJECT_NAME} ${SOURCES} )
endif()

target_link_libraries( ${PROJECT_NAME} tidyLib ZipArchive ${QT_LIBRARIES} )

# Special compiler and linker flags for MSVC
if( MSVC )
    add_definitions( /DUNICODE /D_UNICODE /Zc:wchar_t- )
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG" )
endif()

# No console window shown on launch
if ( WIN32 )
    set_target_properties( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS" )
endif()

# For Mac, add frameworks and make a DMG
if( APPLE )
    ADD_CUSTOM_TARGET( addframeworks ALL COMMAND ${QT_BINARY_DIR}/macdeployqt Sigil.app
                WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )

    ADD_CUSTOM_TARGET( makedmg COMMAND ${QT_BINARY_DIR}/macdeployqt Sigil.app -dmg -no-plugins
                WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
endif()

# You can change the install location by 
# running cmake like this:
#
#   cmake -DCMAKE_INSTALL_PREFIX=/new/install/prefix
#
# By default, the prefix is "/usr/local"
# 
if( UNIX AND NOT APPLE )
    install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
endif()
