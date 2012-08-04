########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 2.8 )

#############################################################################

# Print a message and fail for people who don't
# read the build instructions and then complain
# when the build process fails for them.
if ( ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR} )
    message( FATAL_ERROR "You are trying to run CMake from the <top_folder>/src/Sigil directory, "
                         "instead of just from the <top_folder> directory.\nDO NOT DO THIS.\n"
                         "The correct way looks like this:\n"
                         "  cmake -G '<generator_name>' /path/to/topmost/folder/in/source/package\n"
                         "You will probably now need to first clean your build directory." )
endif()

#############################################################################

# We use the lower case name
# on UNIX systems other than Mac OS X
if ( WIN32 OR APPLE )
    project( Sigil )
else()
    project( sigil )
endif()

#############################################################################

find_package( Qt4 4.7.0 COMPONENTS QtCore QtGui QtMain QtNetwork QtWebkit QtSvg QtXml QtXmlPatterns REQUIRED )
include( ${QT_USE_FILE} )

# These headers need to go through Qt's MOC compiler
set( QOBJECT_HEADERS
    ViewEditors/BookViewEditor.h
    ViewEditors/BookViewPreview.h
    ViewEditors/CodeViewEditor.h 
    Dialogs/AddMetadata.h 
    Dialogs/About.h
    Dialogs/HeadingSelector.h 
    Dialogs/ViewClasses.h
    Dialogs/ViewHTML.h
    Dialogs/ViewImages.h
    Dialogs/SelectImages.h
    Dialogs/MetaEditor.h
    Dialogs/MetaEditorItemDelegate.h
    Dialogs/Preferences.h
    Dialogs/PreferenceWidgets/PreferencesWidget.h
    Dialogs/PreferenceWidgets/KeyboardShortcutsWidget.h
    Dialogs/PreferenceWidgets/SpellCheckWidget.h
    Dialogs/RenameTemplate.h
    Dialogs/LinkStylesheets.h
    Dialogs/SearchEditor.h
    Dialogs/ClipboardEditor.h
    Dialogs/IndexEditor.h
    Tabs/TabManager.h
    Tabs/ContentTab.h 
    Tabs/FlowTab.h
    Tabs/TextTab.h
    Tabs/CSSTab.h 
    Tabs/ImageTab.h
    Tabs/XPGTTab.h
    Tabs/OPFTab.h
    Tabs/NCXTab.h
    Tabs/WellFormedCheckComponent.h
    Tabs/XMLTab.h
    MainUI/MainWindow.h
    MainUI/FindReplace.h
    MainUI/BookBrowser.h
    MainUI/TableOfContents.h
    MainUI/OPFModel.h
    MainUI/NCXModel.h
    MainUI/ValidationResultsView.h
    Misc/AppEventFilter.h
    Misc/FindFields.h
    Misc/SettingsStore.h
    Misc/UILanguage.h
    Misc/UpdateChecker.h 
    Misc/RasterizeImageResource.h
    Misc/WrapIndicator.h
    MiscEditors/SearchEditorItemDelegate.h
    MiscEditors/SearchEditorModel.h
    MiscEditors/ClipboardEditorTreeView.h
    MiscEditors/ClipboardEditorModel.h
    MiscEditors/IndexEditorModel.h
    Misc/NumericItem.h
    Misc/SelectImagePreviewer.h
    ResourceObjects/Resource.h
    ResourceObjects/TextResource.h
    ResourceObjects/HTMLResource.h
    ResourceObjects/CSSResource.h
    ResourceObjects/XPGTResource.h
    ResourceObjects/ImageResource.h
    ResourceObjects/FontResource.h
    ResourceObjects/NCXResource.h
    ResourceObjects/OPFResource.h
    ResourceObjects/XMLResource.h
    BookManipulation/Book.h
    BookManipulation/FolderKeeper.h
    BookManipulation/Metadata.h
    )

set( MAIN_FILES
    sigil_constants.h
    sigil_exception.h
    main.cpp
    )
    
set( BOOK_MANIPULATION_FILES 
    BookManipulation/Book.cpp
    BookManipulation/Book.h
    BookManipulation/BookNormalization.cpp
    BookManipulation/BookNormalization.h
    BookManipulation/Index.cpp
    BookManipulation/Index.h
    BookManipulation/CleanSource.cpp
    BookManipulation/CleanSource.h
    BookManipulation/FolderKeeper.cpp
    BookManipulation/FolderKeeper.h
    BookManipulation/Headings.cpp
    BookManipulation/Headings.h
    BookManipulation/Metadata.cpp
    BookManipulation/Metadata.h
    BookManipulation/XhtmlDoc.cpp
    BookManipulation/XhtmlDoc.h
    BookManipulation/GuideSemantics.cpp
    BookManipulation/GuideSemantics.h
    BookManipulation/XercesCppUse.h
    BookManipulation/XercesHUse.h
    )

set( RESOURCE_OBJECT_FILES
    ResourceObjects/Resource.cpp
    ResourceObjects/Resource.h
    ResourceObjects/TextResource.cpp
    ResourceObjects/TextResource.h
    ResourceObjects/HTMLResource.cpp
    ResourceObjects/HTMLResource.h
    ResourceObjects/CSSResource.cpp
    ResourceObjects/CSSResource.h
    ResourceObjects/XPGTResource.cpp
    ResourceObjects/XPGTResource.h
    ResourceObjects/ImageResource.cpp
    ResourceObjects/ImageResource.h
    ResourceObjects/FontResource.cpp
    ResourceObjects/FontResource.h
    ResourceObjects/OPFResource.cpp
    ResourceObjects/OPFResource.h
    ResourceObjects/NCXResource.cpp
    ResourceObjects/NCXResource.h
    ResourceObjects/XMLResource.cpp
    ResourceObjects/XMLResource.h
    )
        
set( DIALOG_FILES
    Dialogs/About.cpp
    Dialogs/About.h
    Dialogs/AddMetadata.cpp
    Dialogs/AddMetadata.h
    Dialogs/ViewClasses.cpp
    Dialogs/ViewClasses.h
    Dialogs/ViewHTML.cpp
    Dialogs/ViewHTML.h
    Dialogs/ViewImages.cpp
    Dialogs/ViewImages.h
    Dialogs/SelectImages.cpp
    Dialogs/SelectImages.h
    Dialogs/MetaEditor.cpp
    Dialogs/MetaEditor.h
    Dialogs/MetaEditorItemDelegate.cpp
    Dialogs/MetaEditorItemDelegate.h
    Dialogs/HeadingSelector.cpp
    Dialogs/HeadingSelector.h
    Dialogs/Preferences.cpp
    Dialogs/Preferences.h
    Dialogs/PreferenceWidgets/KeyboardShortcutsWidget.cpp
    Dialogs/PreferenceWidgets/KeyboardShortcutsWidget.h
    Dialogs/PreferenceWidgets/LanguageWidget.cpp
    Dialogs/PreferenceWidgets/LanguageWidget.h
    Dialogs/PreferenceWidgets/SpellCheckWidget.cpp
    Dialogs/PreferenceWidgets/SpellCheckWidget.h
    Dialogs/RenameTemplate.cpp
    Dialogs/RenameTemplate.h
    Dialogs/LinkStylesheets.cpp
    Dialogs/LinkStylesheets.h
    Dialogs/SearchEditor.cpp
    Dialogs/SearchEditor.h
    Dialogs/ClipboardEditor.cpp
    Dialogs/ClipboardEditor.h
    Dialogs/IndexEditor.cpp
    Dialogs/IndexEditor.h
    )

set( EXPORTER_FILES
    Exporters/ExportEPUB.cpp
    Exporters/ExportEPUB.h
    Exporters/Exporter.h
    Exporters/ExporterFactory.cpp
    Exporters/ExporterFactory.h
    Exporters/NCXWriter.cpp
    Exporters/NCXWriter.h
    Exporters/XMLWriter.cpp
    Exporters/XMLWriter.h
    Exporters/EncryptionXmlWriter.cpp
    Exporters/EncryptionXmlWriter.h
    )
    
set( IMPORTER_FILES
    Importers/ImportEPUB.cpp
    Importers/ImportEPUB.h
    Importers/Importer.cpp
    Importers/Importer.h
    Importers/ImportOEBPS.cpp
    Importers/ImportOEBPS.h
    Importers/ImporterFactory.cpp
    Importers/ImporterFactory.h
    Importers/ImportHTML.cpp
    Importers/ImportHTML.h
    Importers/ImportTXT.cpp
    Importers/ImportTXT.h
    )
    
set( MISC_FILES    
    Misc/AppEventFilter.cpp
    Misc/AppEventFilter.h
    Misc/UpdateChecker.cpp
    Misc/UpdateChecker.h
    Misc/Utility.cpp
    Misc/Utility.h
    Misc/SleepFunctions.h
    Misc/FindFields.cpp
    Misc/FindFields.h
    Misc/FindReplaceQLineEdit.cpp
    Misc/FindReplaceQLineEdit.h
    Misc/FilenameDelegate.cpp
    Misc/FilenameDelegate.h
    Misc/XHTMLHighlighter.cpp
    Misc/XHTMLHighlighter.h
    Misc/CSSHighlighter.cpp
    Misc/CSSHighlighter.h
    Misc/HTMLEncodingResolver.cpp
    Misc/HTMLEncodingResolver.h
    Misc/HTMLSpellCheck.cpp
    Misc/HTMLSpellCheck.h
    Misc/RasterizeImageResource.cpp
    Misc/RasterizeImageResource.h
    Misc/SearchOperations.cpp
    Misc/SearchOperations.h
    Misc/Language.cpp
    Misc/UILanguage.cpp
    Misc/SettingsStore.cpp
    Misc/SettingsStore.h
    Misc/SpellCheck.cpp
    Misc/SpellCheck.h
    Misc/KeyboardShortcut.cpp
    Misc/KeyboardShortcut.h
    Misc/KeyboardShortcut_p.h
    Misc/KeyboardShortcutManager.cpp
    Misc/KeyboardShortcutManager.h
    Misc/XhtmlEntitiesDtd.cpp
    Misc/FontObfuscation.cpp
    Misc/FontObfuscation.h
    Misc/TempFolder.cpp
    Misc/TempFolder.h
    Misc/TOCHTMLWriter.cpp
    Misc/TOCHTMLWriter.h
    Misc/NumericItem.h
    Misc/WrapIndicator.h
    Misc/SelectImagePreviewer.h
    )

set( MISC_EDITORS_FILES    
    MiscEditors/ClipboardEditorModel.cpp
    MiscEditors/ClipboardEditorModel.h
    MiscEditors/ClipboardEditorTreeView.cpp
    MiscEditors/ClipboardEditorTreeView.h
    MiscEditors/IndexEditorModel.cpp
    MiscEditors/IndexEditorModel.h
    MiscEditors/IndexEditorTreeView.cpp
    MiscEditors/IndexEditorTreeView.h
    MiscEditors/IndexEntries.cpp
    MiscEditors/IndexEntries.h
    MiscEditors/IndexHTMLWriter.cpp
    MiscEditors/IndexHTMLWriter.h
    MiscEditors/SearchEditorTreeView.cpp
    MiscEditors/SearchEditorTreeView.h
    MiscEditors/SearchEditorItemDelegate.cpp
    MiscEditors/SearchEditorItemDelegate.h
    MiscEditors/SearchEditorModel.cpp
    MiscEditors/SearchEditorModel.h
    )

set( SPCRE_FILES
    PCRE/SPCRE.cpp
    PCRE/SPCRE.h
    PCRE/PCRECache.cpp
    PCRE/PCRECache.h
    PCRE/PCREReplaceTextBuilder.cpp
    PCRE/PCREReplaceTextBuilder.h
    )

set( VIEW_EDITOR_FILES
    ViewEditors/BookViewEditor.cpp
    ViewEditors/BookViewEditor.h
    ViewEditors/BookViewPreview.cpp
    ViewEditors/BookViewPreview.h
    ViewEditors/CodeViewEditor.cpp
    ViewEditors/CodeViewEditor.h
    ViewEditors/LineNumberArea.cpp
    ViewEditors/LineNumberArea.h
    ViewEditors/Searchable.cpp
    ViewEditors/Searchable.h
    ViewEditors/Zoomable.h 
    ViewEditors/ViewEditor.h     
    )

set( MAINUI_FILES 
    MainUI/MainWindow.cpp
    MainUI/MainWindow.h
    MainUI/FindReplace.cpp
    MainUI/FindReplace.h
    MainUI/BookBrowser.cpp
    MainUI/BookBrowser.h
    MainUI/TableOfContents.cpp
    MainUI/TableOfContents.h
    MainUI/OPFModel.cpp
    MainUI/OPFModel.h
    MainUI/OPFModelItem.cpp
    MainUI/OPFModelItem.h
    MainUI/NCXModel.cpp
    MainUI/NCXModel.h
    MainUI/ValidationResultsView.cpp
    MainUI/ValidationResultsView.h
    )

set( TAB_FILES
    Tabs/TabManager.cpp
    Tabs/TabManager.h
    Tabs/FlowTab.cpp
    Tabs/FlowTab.h 
    Tabs/ContentTab.cpp
    Tabs/ContentTab.h 
    Tabs/TextTab.cpp
    Tabs/TextTab.h
    Tabs/CSSTab.cpp
    Tabs/CSSTab.h
    Tabs/XPGTTab.cpp
    Tabs/XPGTTab.h
    Tabs/ImageTab.cpp
    Tabs/ImageTab.h
    Tabs/OPFTab.cpp
    Tabs/OPFTab.h
    Tabs/NCXTab.cpp
    Tabs/NCXTab.h
    Tabs/WellFormedCheckComponent.cpp
    Tabs/WellFormedCheckComponent.h
    Tabs/XMLTab.cpp
    Tabs/XMLTab.h
    Tabs/WellFormedContent.h
    )
            
set( UI_FILES   
    Form_Files/main.ui 
    Form_Files/ViewClasses.ui
    Form_Files/ViewHTML.ui
    Form_Files/ViewImages.ui
    Form_Files/SelectImages.ui
    Form_Files/MetaEditor.ui 
    Form_Files/AddMetadata.ui 
    Form_Files/About.ui 
    Form_Files/HeadingSelector.ui 
    Form_Files/FindReplace.ui
    Form_Files/Preferences.ui
    Form_Files/PKeyboardShortcutsWidget.ui
    Form_Files/PLanguageWidget.ui
    Form_Files/PSpellCheckWidget.ui
    Form_Files/RenameTemplate.ui 
    Form_Files/LinkStylesheets.ui 
    Form_Files/SearchEditor.ui
    Form_Files/ClipboardEditor.ui
    Form_Files/IndexEditor.ui 
    )
    
set( SOURCEUPDATE_FILES   
    SourceUpdates/PerformXMLUpdates.cpp
    SourceUpdates/PerformXMLUpdates.h
    SourceUpdates/PerformHTMLUpdates.cpp 
    SourceUpdates/PerformHTMLUpdates.h
    SourceUpdates/PerformOPFUpdates.cpp
    SourceUpdates/PerformOPFUpdates.h
    SourceUpdates/PerformNCXUpdates.cpp
    SourceUpdates/PerformNCXUpdates.h
    SourceUpdates/PerformCSSUpdates.cpp
    SourceUpdates/PerformCSSUpdates.h
    SourceUpdates/AnchorUpdates.cpp
    SourceUpdates/AnchorUpdates.h
    SourceUpdates/LinkUpdates.cpp
    SourceUpdates/LinkUpdates.h
    SourceUpdates/UniversalUpdates.cpp
    SourceUpdates/UniversalUpdates.h
    )
            
set( QRC_FILES  
    Resource_Files/main/main.qrc
    Resource_Files/About/About.qrc
    Resource_Files/icon/icon.qrc  
    )

file( GLOB TS_FILES Resource_Files/ts/sigil_*.ts )

file( GLOB DIC_FILES Resource_Files/dictionaries/* )

file( GLOB EXAMPLE_FILES Resource_Files/examples/* )

set( LINUX_DESKTOP_FILE
    Resource_Files/freedesktop/sigil.desktop
    )

set( LINUX_DESKTOP_ICON_FILE
    Resource_Files/icon/app_icon_48.png
    )
    
set( RAW_SOURCES ${MAIN_FILES} ${TAB_FILES} ${SOURCEUPDATE_FILES} ${BOOK_MANIPULATION_FILES} ${RESOURCE_OBJECT_FILES} ${DIALOG_FILES} ${EXPORTER_FILES} ${IMPORTER_FILES} ${MISC_FILES} ${MISC_EDITORS_FILES} ${SPCRE_FILES} ${VIEW_EDITOR_FILES} ${MAINUI_FILES} )

#############################################################################

# Runs MOC on specifed files
qt4_wrap_cpp( MOC_FILES_CPP ${QOBJECT_HEADERS} )

# Runs UIC on specified files
qt4_wrap_ui( UI_FILES_H ${UI_FILES} )

# Runs RCC on specified files
qt4_add_resources( QRC_FILES_CPP ${QRC_FILES} )

# Runs lrelease on the specified files
qt4_add_translation( QM_FILES ${TS_FILES} )

# Define the Sigil version string for use in source files
set_source_files_properties( Dialogs/About.cpp PROPERTIES COMPILE_DEFINITIONS SIGIL_FULL_VERSION="${SIGIL_FULL_VERSION}" )
set_source_files_properties( Misc/Utility.cpp PROPERTIES COMPILE_DEFINITIONS SIGIL_FULL_VERSION="${SIGIL_FULL_VERSION}" )

#############################################################################

# Adds folders for Visual Studio solution explorer (and for Xcode explorer)
source_group( "Generated"         FILES ${MOC_FILES_CPP} ${UI_FILES_H} ${QRC_FILES_CPP} )
source_group( "Forms"             FILES ${UI_FILES} )
source_group( "Resource Files"    FILES ${QRC_FILES} )
source_group( "Book Manipulation" FILES ${BOOK_MANIPULATION_FILES} )
source_group( "Resource Objects"  FILES ${RESOURCE_OBJECT_FILES} )
source_group( "Dialogs"           FILES ${DIALOG_FILES} )
source_group( "Exporters"         FILES ${EXPORTER_FILES} )
source_group( "Importers"         FILES ${IMPORTER_FILES} )
source_group( "Misc"              FILES ${MISC_FILES} )
source_group( "MiscEditors"       FILES ${MISC_EDITORS_FILES} )
source_group( "SPCRE"             FILES ${SPCRE_FILES} )
source_group( "View Editors"      FILES ${VIEW_EDITOR_FILES} )
source_group( "Tabs"              FILES ${TAB_FILES} )
source_group( "MainUI"            FILES ${MAINUI_FILES} )
source_group( "Main Files"        FILES ${MAIN_FILES} )
source_group( "Source Updates"    FILES ${SOURCEUPDATE_FILES} )


#############################################################################

set( ALL_SOURCES ${RAW_SOURCES} ${MOC_FILES_CPP} ${UI_FILES_H} ${QRC_FILES_CPP} ${QM_FILES} )

# Adding resource (RC) files for Windows
if ( MSVC )
    configure_file( Resource_files/windows/version.rc.in ${PROJECT_BINARY_DIR}/version.rc )
    set( WINDOWS_RC_FILES
         Resource_Files/icon/icon.rc
         ${PROJECT_BINARY_DIR}/version.rc
         )
    list( APPEND ALL_SOURCES ${WINDOWS_RC_FILES} )
    source_group( "Resources" FILES {$WINDOWS_RC_FILES} )
endif()

#############################################################################

# Apple bundle configuration
if( APPLE )
    # Copy the icon files
    # TODO: put these in some sort of add_custom_command( TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ...)
    # also delete the Sigil.app folder as PRE_BUILD of Sigil executable
    
    if( CMAKE_GENERATOR STREQUAL Xcode )
        exec_program( "mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/Resources")
        exec_program( "cp ${PROJECT_SOURCE_DIR}/Resource_Files/icon/Sigil.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/Resources")
        exec_program( "cp ${PROJECT_SOURCE_DIR}/Resource_Files/icon/sgf.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/Resources")
        exec_program( "cp ${PROJECT_SOURCE_DIR}/Resource_Files/icon/epub.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/Resources")
        # Create translation directory.
        exec_program( "mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/translations" )
        exec_program( "mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/hunspell_dictionaries" )
    else()
        exec_program( "mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/Resources")
        exec_program( "cp ${PROJECT_SOURCE_DIR}/Resource_Files/icon/Sigil.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/Resources")
        exec_program( "cp ${PROJECT_SOURCE_DIR}/Resource_Files/icon/sgf.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/Resources")
        exec_program( "cp ${PROJECT_SOURCE_DIR}/Resource_Files/icon/epub.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/Resources")
        # Create translation directory.
        exec_program( "mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/translations")
        exec_program( "mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/hunspell_dictionaries" )
    endif()

    # Copy the PLIST file...
    exec_program( "cp ${PROJECT_SOURCE_DIR}/Resource_Files/mac/MacOSXBundleInfo.plist ${PROJECT_BINARY_DIR}")
    
    # ...and set the Sigil version string
    exec_program( "sed -i -e 's/SGVERSION/${SIGIL_FULL_VERSION}/g' ${PROJECT_BINARY_DIR}/MacOSXBundleInfo.plist")
endif()

#############################################################################

# We need to pick up the ui*.h files,
# and the headers for the linked-to libraries.
# The directories are in reverse order because we're using before to
# put our include dirs before any system ones.
include_directories( BEFORE
                     ${FLIGHTCREW_INCLUDE_DIRS}
                     ${HUNSPELL_INCLUDE_DIRS}
                     ${PCRE_INCLUDE_DIRS}
                     ${tidyLib_SOURCE_DIR}
                     ${MINIZIP_INCLUDE_DIRS}
                     ${ZLIB_INCLUDE_DIRS}
                     ${XERCESEXTENSIONS_INCLUDE_DIRS}
                     ${XERCES_INCLUDE_DIRS}
                     ${BOOST_INCLUDE_DIRS}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )

#############################################################################

# We make bundles for Mac OS X
if ( APPLE )
    add_executable( ${PROJECT_NAME} MACOSX_BUNDLE ${ALL_SOURCES} )
    set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_BINARY_DIR}/MacOSXBundleInfo.plist )
# ...and a normal executable for everything else.
else()
    add_executable( ${PROJECT_NAME} WIN32 ${ALL_SOURCES} )
endif()

target_link_libraries( ${PROJECT_NAME} ${HUNSPELL_LIBRARIES} ${PCRE_LIBRARIES} tidyLib ${MINIZIP_LIBRARIES} ${BOOST_LIBS} ${XERCES_LIBRARIES} ${XERCESEXTENSIONS_LIBRARIES} ${FLIGHTCREW_LIBRARIES} ${QT_LIBRARIES} )

#############################################################################

# needed for correct static header inclusion
if( FORCE_BUNDLED_COPIES OR NOT XERCES_FOUND )
    add_definitions( -DXERCES_STATIC_LIBRARY )
endif()
if( FORCE_BUNDLED_COPIES OR NOT PCRE_FOUND )
    add_definitions( -DPCRE_STATIC )
endif()
if( FORCE_BUNDLED_COPIES OR NOT HUNSPELL_FOUND )
    add_definitions( -DHUNSPELL_STATIC )
endif()

# Speed up stirng building
add_definitions( -DQT_USE_FAST_CONCATENATION )
add_definitions( -DQT_USE_FAST_OPERATOR_PLUS )

#############################################################################

# "Link time code generation" flags for MSVC
if( MSVC )
    add_definitions( /DUNICODE /D_UNICODE )
    # The /Zc:wchar_t- flag can't go into add_definitions
    # because the RC compiler picks it up too and it provokes a name clash
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t- /MP")
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG" )

# "Print all warnings" flag for GCC
elseif( CMAKE_COMPILER_IS_GNUCXX )
    add_definitions( -Wall )
endif()


#############################################################################

# For Mac, add frameworks and make a DMG
if( APPLE )
    if(CMAKE_GENERATOR STREQUAL Xcode)
        set( WORK_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release" )
    else()
        set( WORK_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" )
    endif()

    add_custom_target(  addframeworks ALL 
                        COMMAND ${QT_BINARY_DIR}/macdeployqt Sigil.app
                        WORKING_DIRECTORY ${WORK_DIR}
                        DEPENDS ${PROJECT_NAME} )

    add_custom_target(  makedmg 
                        COMMAND ${QT_BINARY_DIR}/macdeployqt Sigil.app -dmg -no-plugins
                        WORKING_DIRECTORY ${WORK_DIR}
                        DEPENDS addframeworks )

    add_custom_command( TARGET addframeworks POST_BUILD COMMAND cp ${PROJECT_BINARY_DIR}/*.qm ${WORK_DIR}/Sigil.app/Contents/translations/ )
    add_custom_command( TARGET addframeworks POST_BUILD COMMAND cp ${CMAKE_SOURCE_DIR}/src/Sigil/Resource_Files/dictionaries/* ${WORK_DIR}/Sigil.app/Contents/hunspell_dictionaries )
    add_custom_command( TARGET addframeworks POST_BUILD COMMAND cp -r ${CMAKE_SOURCE_DIR}/src/Sigil/Resource_Files/javascript/ckeditor ${WORK_DIR}/Sigil.app/Contents )
                
# For Linux and Windows, provide binary installers.
# For this to work on Linux, InstallJammer needs to be installed and on the system path.
# For this to work on Windows, Inno Setup's iscc compiler needs to be installed and on the system path.
elseif (MSVC)
    set( TEMP_PACKAGE_DIR ${CMAKE_BINARY_DIR}/temp_folder )
    set( MAIN_PACKAGE_DIR ${TEMP_PACKAGE_DIR}/Sigil )
    set( OUTPUT_PACKAGE_DIR ${CMAKE_BINARY_DIR}/installer )
    
    # ISS conf file for the Inno Setup compiler
    # We first create a CMake configured version of the ISS file,
    # and then we copy it to the temp folder every time we need to build the installer.
    set( ISS_MAIN_LOCATION ${CMAKE_SOURCE_DIR}/installer/Sigil.iss  )
    set( ISS_CONFIGURED_LOCATION ${CMAKE_BINARY_DIR}/Sigil_configured.iss )
    set( ISS_TEMP_LOCATION ${CMAKE_BINARY_DIR}/temp_folder/Sigil_configured.iss )

    # Specify platform var for Inno and installjammer
    if ( 64_BIT_PLATFORM )
        # Used in the ISS CMake configuration
        set( ISS_ARCH "x64" )
        set( ISS_SETUP_FILENAME_PLATFORM "-x64" )
    endif()
    
    # Creates a copy of the ISS file in ${ISS_CONFIGURED_LOCATION} and then configures it 
    # Used in the ISS CMake configuration
    set( LICENSE_LOCATION ${CMAKE_SOURCE_DIR}/installer/win_installer_note.txt  )
    configure_file( ${ISS_MAIN_LOCATION} ${ISS_CONFIGURED_LOCATION} )

    # Run Inno Setup's iscc compiler (*AFTER* all the PRE_BUILD custom commands execute)
    add_custom_target(  makeinstaller 
        COMMAND cmake -E echo "For this to work, Inno Setup's iscc compiler needs to be installed and on the system path."
        COMMAND iscc ${ISS_TEMP_LOCATION} )

    # We need to copy the files that will be used to make the installer to
    # a temporary directory. On the MSVC compiler the PRE_BUILD custom commands
    # can be added directly, but on other generators we need an intermediate target
    # since the PRE_BUILD condition is not supported. Using the intermediate for
    # MSVC makes it unnecessarily recompile that target every time the project is built.
    # So we use the direct way on MSVC, and the intemediate way for other generators.
    if( MSVC_IDE )
        set( TARGET_FOR_COPY makeinstaller )
        add_dependencies( makeinstaller ${PROJECT_NAME} )
    else()
        set( TARGET_FOR_COPY copyfiles )
        
        # The intermediate target for copying
        add_custom_target(  copyfiles
                            COMMENT "Copying installer files to temporary location..."
                            DEPENDS ${PROJECT_NAME} )
                            
        add_dependencies( makeinstaller copyfiles )
    endif()
    
    # Copy ISS file to temp folder location
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD 
                        COMMAND cmake -E copy ${ISS_CONFIGURED_LOCATION} ${ISS_TEMP_LOCATION} )     
    
    # Copy Qt runtime libs
    set( QT_LIBS QtCore QtGui QtNetwork QtSvg QtWebKit QtXml QtXmlPatterns phonon )
    
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/ )    
    foreach( lib ${QT_LIBS} )
        set( location location-NOTFOUND )
        find_file( location ${lib} NAMES ${lib}4.dll lib${lib}.so.4 PATHS ${QT_LIBRARY_DIR} )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E copy ${location} ${MAIN_PACKAGE_DIR}/ )
    endforeach( lib )

    # Copy codecs plugins
    set( QT_CODECS qcncodecs qjpcodecs qkrcodecs qtwcodecs )
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/codecs/ )    
    foreach( lib ${QT_CODECS} )
        set( location location-NOTFOUND )
        find_file( location ${lib} NAMES ${lib}4.dll lib${lib}.so PATHS ${QT_PLUGINS_DIR}/codecs/ )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E copy ${location} ${MAIN_PACKAGE_DIR}/codecs/ )
    endforeach( lib )
    
    # Copy iconengines plugins
    set( QT_ICONENGINES qsvgicon )
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/iconengines/ )
    foreach( lib ${QT_ICONENGINES} )
        set( location location-NOTFOUND )
        find_file( location ${lib} NAMES ${lib}4.dll lib${lib}.so PATHS ${QT_PLUGINS_DIR}/iconengines/ )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E copy ${location} ${MAIN_PACKAGE_DIR}/iconengines/ )
    endforeach( lib )    

    # Copy imageformats plugins
    set( QT_IMAGEFORMATS qgif qico qjpeg qmng qsvg qtiff )
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/imageformats/ )
    foreach( lib ${QT_IMAGEFORMATS} )   
        set( location location-NOTFOUND )
        find_file( location ${lib} NAMES ${lib}4.dll lib${lib}.so PATHS ${QT_PLUGINS_DIR}/imageformats/ )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E copy ${location} ${MAIN_PACKAGE_DIR}/imageformats/ )
    endforeach( lib )

    # Copy the translation qm files
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/translations/ )
    foreach( QM ${QM_FILES} )
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy ${QM} ${MAIN_PACKAGE_DIR}/translations/ )
    endforeach( QM )

    # Copy the dictionary files
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/hunspell_dictionaries/ )
    foreach( DIC ${DIC_FILES} )
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy ${DIC} ${MAIN_PACKAGE_DIR}/hunspell_dictionaries/ )
    endforeach( DIC )

    # Copy the example files
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/examples/ )
    foreach( EXAMPLE ${EXAMPLE_FILES} )
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy ${EXAMPLE} ${MAIN_PACKAGE_DIR}/examples/ )
    endforeach( EXAMPLE )

    # Copy CKE
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/ckeditor/ )
    add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy_directory ${PROJECT_SOURCE_DIR}/Resource_Files/javascript/ckeditor ${MAIN_PACKAGE_DIR}/ckeditor/ )

    # Set the path of the application executable
    if( MSVC_IDE )
        set( EXE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} )
    else()
        set( EXE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} )
    endif()

    # Copy the application executable
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD 
                        COMMAND cmake -E copy ${EXE_PATH} ${MAIN_PACKAGE_DIR} )
                        
    # We need to copy the CRT dlls
    # Add -DWIN_INSTALLER_USE_64BIT_CRT=1 to the cmake call if you want to build
    # an installer for the x64 verison of Sigil. This will make sure that the
    # correct CRT libs are included in the installer.
    if ( WIN_INSTALLER_USE_64BIT_CRT )
        message( STATUS "Using the 64 bit CRT in the Sigil Windows installer" )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD 
            COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/installer/msvc_crt/x64/msvcp100.dll ${MAIN_PACKAGE_DIR} )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD 
            COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/installer/msvc_crt/x64/msvcr100.dll ${MAIN_PACKAGE_DIR} )
    else()
        message( STATUS "Using the 32 bit CRT in the Sigil Windows installer" )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD 
            COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/installer/msvc_crt/x86/msvcp100.dll ${MAIN_PACKAGE_DIR} )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD 
            COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/installer/msvc_crt/x86/msvcr100.dll ${MAIN_PACKAGE_DIR} )            
    endif()

    # Copy the Changelog
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD 
                        COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/ChangeLog.txt ${MAIN_PACKAGE_DIR} )
    
    # Copy the license file
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD 
                        COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/COPYING.txt ${MAIN_PACKAGE_DIR} )

    # The folder for miscellaneous files (icon files etc.)
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/misc/ )
                        
    # Copy the icon file (used on Linux for the application icon)
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD 
                        COMMAND cmake -E copy ${PROJECT_SOURCE_DIR}/Resource_Files/icon/app_icon_48.png ${MAIN_PACKAGE_DIR}/misc/ )
                        
    # Copy the SGF ICO file (used on Windows as the SGF file type icon)
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD 
                        COMMAND cmake -E copy ${PROJECT_SOURCE_DIR}/Resource_Files/icon/sgf.ico ${MAIN_PACKAGE_DIR}/misc/ )
                        
    # Remove the temp directory used for building the installer 
    add_custom_command( TARGET makeinstaller POST_BUILD 
                        COMMAND cmake -E remove_directory ${TEMP_PACKAGE_DIR}
                        COMMENT "Removing temporary directory..." )
endif()

#############################################################################

# You can change the install location by 
# running cmake like this:
#
#   cmake -DCMAKE_INSTALL_PREFIX=/new/install/prefix
#
# By default, the prefix is "/usr/local"
# 
if( UNIX AND NOT APPLE )
    install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
    install( FILES ${LINUX_DESKTOP_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications )
    install( FILES ${LINUX_DESKTOP_ICON_FILE} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pixmaps RENAME sigil.png )
    install( FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/sigil/translations )
    install( FILES ${DIC_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/sigil/hunspell_dictionaries )
    install( FILES ${EXAMPLE_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/sigil/examples )
    install( DIRECTORY Resource_Files/javascript/ckeditor DESTINATION ${CMAKE_INSTALL_PREFIX}/share/sigil )
endif()

