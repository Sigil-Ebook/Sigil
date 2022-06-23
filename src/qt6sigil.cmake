#############################################################################
#     Build Sigil against Qt6 - requires cmake 3.16+ and a C++17 compiler
#############################################################################

# quiet Qt 6 deprecat4ed warnings for now as we must support Qt 5.12.X and even earlier
# add_definitions(-DQT_NO_DEPRECATED_WARNINGS)
add_definitions(-DQT_IMPLICIT_QCHAR_CONSTRUCTION)

set(QT6_NEEDED 6.2)

set( PKGS_TO_FIND Core Core5Compat Network WebEngineCore WebEngineWidgets Widgets Xml Concurrent PrintSupport LinguistTools )
if (APPLE)
    list( APPEND PKGS_TO_FIND UiTools )
endif()
if ( WIN32 )
    # QtWinExtras not in Qt6 as of 6.2.2
    # list( APPEND PKGS_TO_FIND WinExtras )
    if ( USE_ALT_ICONS )
        set(APP_ICON_PATH "${CMAKE_SOURCE_DIR}/src/Resource_Files/icon/app_icons_alt/app.ico" )
    else()
        set(APP_ICON_PATH "${CMAKE_SOURCE_DIR}/src/Resource_Files/icon/app_icons_orig/app.ico" )
    endif()
endif()
find_package( Qt6 ${QT6_NEEDED} COMPONENTS ${PKGS_TO_FIND} REQUIRED )

set(CMAKE_AUTOMOC ON)

if ( NOT DEFINED PKG_SYSTEM_PYTHON )
    if (MSVC)
        set ( PKG_SYSTEM_PYTHON 1 )
    else()
        set ( PKG_SYSTEM_PYTHON 0 )
    endif()
endif()

if ( PKG_SYSTEM_PYTHON )
    add_definitions( -DBUNDLING_PYTHON )
endif()

# Default will work for building 32bit Sigil on Windows 32bit and
# building 64bit Sigil on Windows 64bit. Those building 32bit Sigil on
# Windows 64bit will probably have to set -DSYS_DLL_DIR="C:\Windows\SysWOW64"
# in the original CMAKE command.
if ( MSVC )
    if ( NOT DEFINED SYS_DLL_DIR )
        set( SYS_DLL_DIR "C:\\Windows\\System32" )
    endif()
else()
     set( SYS_DLL_DIR "" )
endif()

# use -DINSTALL_BUNDLED_DICTS=0 to disable the installation of the bundled
# hunspell directories. If disabled, the path to the system hunspell dictionaries
# will need to specified at run time with the environment variable SIGIL_DICTIONARIES
if ( NOT DEFINED INSTALL_BUNDLED_DICTS )
    set ( INSTALL_BUNDLED_DICTS 1 )
endif()

# use -DDISABLE_UPDATE_CHECK=1 to disable the builtin update check.
# Mainly for use by *nix distros whose Sigil packages can't make use of the
# new release downloads anyway.
if ( NOT DEFINED DISABLE_UPDATE_CHECK )
    set ( DISABLE_UPDATE_CHECK 0 )
endif()

set( RAW_SOURCES ${MAIN_FILES} ${TAB_FILES} ${SOURCEUPDATE_FILES} ${BOOK_MANIPULATION_FILES} ${RESOURCE_OBJECT_FILES} ${DIALOG_FILES} ${WIDGET_FILES} ${EXPORTER_FILES} ${IMPORTER_FILES} ${MISC_FILES} ${MISC_EDITORS_FILES} ${QUERY_FILES} ${PARSERS_FILES} ${EMBEDPYTHON_FILES} ${SPCRE_FILES} ${VIEW_EDITOR_FILES} ${MAINUI_FILES} )

#############################################################################

# Runs UIC on specified files
qt6_wrap_ui( UI_FILES_H ${UI_FILES} )
set_property( SOURCE ${UI_FILES_H} PROPERTY SKIP_AUTOMOC ON )
# Runs RCC on specified files
qt6_add_resources( QRC_FILES_CPP ${QRC_FILES} )
set_property( SOURCE ${QRC_FILES_CPP} PROPERTY SKIP_AUTOMOC ON )
qt6_add_binary_resources( main  ${MAIN_BINARY_QRC_FILES} )
qt6_add_binary_resources( fluent ${FLUENT_BINARY_QRC_FILES} )
qt6_add_binary_resources( material ${MATERIAL_BINARY_QRC_FILES} )
# Runs lrelease on the specified files
qt6_add_translation( QM_FILES ${TS_FILES} )

# Compiled binary resource files
set( EXT_RCC_FILES
    ${PROJECT_BINARY_DIR}/main.rcc
    ${PROJECT_BINARY_DIR}/fluent.rcc
    ${PROJECT_BINARY_DIR}/material.rcc
    )

# Define the Sigil version string for use in source files
set_source_files_properties( Dialogs/About.cpp PROPERTIES COMPILE_DEFINITIONS SIGIL_FULL_VERSION="${SIGIL_FULL_VERSION}" )
set_source_files_properties( Misc/Utility.cpp PROPERTIES COMPILE_DEFINITIONS SIGIL_FULL_VERSION="${SIGIL_FULL_VERSION}" )

#############################################################################

# Adds folders for Visual Studio solution explorer (and for Xcode explorer)
source_group( "Forms"             FILES ${UI_FILES} )
source_group( "Resource Files"    FILES ${QRC_FILES} )
source_group( "Book Manipulation" FILES ${BOOK_MANIPULATION_FILES} )
source_group( "Resource Objects"  FILES ${RESOURCE_OBJECT_FILES} )
source_group( "Dialogs"           FILES ${DIALOG_FILES} )
source_group( "Widgets"           FILES ${WIDGET_FILES} )
source_group( "Exporters"         FILES ${EXPORTER_FILES} )
source_group( "EmbedPython"       FILES ${EMBEDPYTHON_FILES} )
source_group( "Importers"         FILES ${IMPORTER_FILES} )
source_group( "Misc"              FILES ${MISC_FILES} )
source_group( "MiscEditors"       FILES ${MISC_EDITORS_FILES} )
source_group( "Query"             FILES ${QUERY_FILES} )
source_group( "Parsers"           FILES ${PARSERS_FILES} )
source_group( "SPCRE"             FILES ${SPCRE_FILES} )
source_group( "View Editors"      FILES ${VIEW_EDITOR_FILES} )
source_group( "Tabs"              FILES ${TAB_FILES} )
source_group( "MainUI"            FILES ${MAINUI_FILES} )
source_group( "Main Files"        FILES ${MAIN_FILES} )
source_group( "Source Updates"    FILES ${SOURCEUPDATE_FILES} )


#############################################################################

set( ALL_SOURCES ${RAW_SOURCES} ${UI_FILES_H} ${QRC_FILES_CPP} ${QM_FILES} )

# Location of the MathJax custom build for all platforms
set( MATHJAX_CUSTOM "${CMAKE_SOURCE_DIR}/src/Resource_Files/polyfills/custom-mathjax.min.js" )

# Adding resource (RC) files for Windows
# Grab the current year so copyright notice is updated on Windows file properties
string( TIMESTAMP SIGIL_BUILD_YEAR "%Y" )
if ( WIN32 )
    configure_file( Resource_files/windows/icon.rc.in ${PROJECT_BINARY_DIR}/icon.rc )
    configure_file( Resource_files/windows/version.rc.in ${PROJECT_BINARY_DIR}/version.rc )
    set( WINDOWS_RC_FILES
         ${PROJECT_BINARY_DIR}/icon.rc
         ${PROJECT_BINARY_DIR}/version.rc
         )
    list( APPEND ALL_SOURCES ${WINDOWS_RC_FILES} )
    source_group( "Resources" FILES {$WINDOWS_RC_FILES} )
endif()

#############################################################################

# Apple bundle configuration
if( APPLE )
    # TODO: put these in some sort of add_custom_command( TARGET ${PROJECT_NAME} PRE_BUILD COMMAND ...)
    # also delete the Sigil.app folder as PRE_BUILD of Sigil executable

    if ( USE_ALT_ICONS )
        set( ICON_SRC_PATH  ${PROJECT_SOURCE_DIR}/Resource_Files/icon/app_icons_alt/Sigil.icns )
    else()
        set( ICON_SRC_PATH  ${PROJECT_SOURCE_DIR}/Resource_Files/icon/app_icons_orig/Sigil.icns )
    endif()

    if( CMAKE_GENERATOR STREQUAL Xcode )
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/Resources")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/polyfills")
        exec_program("cp ${MATHJAX_CUSTOM} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/polyfills")
        exec_program("cp ${ICON_SRC_PATH} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/Resources")
        exec_program("cp ${PROJECT_SOURCE_DIR}/Resource_Files/icon/epub.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/Resources")
        # Create translation directory.
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/translations")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/hunspell_dictionaries")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/plugin_launchers")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/plugin_launchers/python")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/python3lib")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/lib")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release/Sigil.app/Contents/examples")
    else()
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/Resources")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/polyfills")
        exec_program("cp ${MATHJAX_CUSTOM} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/polyfills")
        exec_program("cp ${ICON_SRC_PATH} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/Resources")
        exec_program("cp ${PROJECT_SOURCE_DIR}/Resource_Files/icon/epub.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/Resources")
        # Create translation directory.
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/translations")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/hunspell_dictionaries")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/plugin_launchers")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/plugin_launchers/python")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/python3lib")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/lib")
        exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Sigil.app/Contents/examples")
    endif()

    if ( PKG_SYSTEM_PYTHON )
        configure_file( Resource_Files/python_pkg/osx_add_python_framework6.py ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/osx_add_python_framework6.py )
    endif()

    # Copy the PLIST file...
    exec_program("cp ${PROJECT_SOURCE_DIR}/Resource_Files/mac/MacOSXBundleInfo.plist ${PROJECT_BINARY_DIR}")

    # ...and set the Sigil version string
    exec_program("sed -i -e 's/SGVERSION/${SIGIL_FULL_VERSION}/g' ${PROJECT_BINARY_DIR}/MacOSXBundleInfo.plist")
endif()

#############################################################################

# We need to pick up the ui*.h files,
# and the headers for the linked-to libraries.
# The directories are in reverse order because we're using before to
# put our include dirs before any system ones.
include_directories( BEFORE
                     ${HUNSPELL_INCLUDE_DIRS}
                     ${PCRE2_INCLUDE_DIRS}
                     ${MINIZIP_INCLUDE_DIRS}
                     ${ZLIB_INCLUDE_DIRS}
                     ${PYTHON_INCLUDE_DIRS}
                     ${GUMBO_INCLUDE_DIRS}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )

#############################################################################

# We make bundles for Mac OS X 
if ( APPLE )
    add_executable( ${PROJECT_NAME} MACOSX_BUNDLE ${ALL_SOURCES} )
    set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_BINARY_DIR}/MacOSXBundleInfo.plist )
    set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_SKIP_BUILD_RPATH  TRUE)
# ...and a normal executable for everything else.
else()
    add_executable( ${PROJECT_NAME} WIN32 ${ALL_SOURCES} )
endif()

# LIBS_TO_LINK for all platforms
set( LIBS_TO_LINK ${HUNSPELL_LIBRARIES} ${PCRE2_LIBRARIES} ${GUMBO_LIBRARIES} ${MINIZIP_LIBRARIES}
                  Qt6::Core5Compat Qt6::Widgets  Qt6::Xml  Qt6::PrintSupport  Qt6::WebEngineCore  
                  Qt6::WebEngineWidgets  Qt6::Network  Qt6::Concurrent )

# Additions to LIBS_TO_LINK based on situation or platform
if (${USE_NEWER_FINDPYTHON3})
    message(STATUS "Using newer Python3::Python target to link to Python")
    list( APPEND LIBS_TO_LINK Python3::Python )
else()
    message(STATUS "Using older PYTHON_LIBRARIES CMAKE variable to link to Python")
    list( APPEND LIBS_TO_LINK ${PYTHON_LIBRARIES} )
endif()

# QtUiTools needed for PySide plugins
if ( APPLE )
    list( APPEND LIBS_TO_LINK Qt6::UiTools )
endif()

# QtWinExtras not in Qt6 as of 6.2.2
#if ( WIN32 )
#    list( APPEND LIBS_TO_LINK Qt6::WinExtras )
#endif()

target_link_libraries( ${PROJECT_NAME} ${LIBS_TO_LINK} )

#############################################################################

# needed for correct static header inclusion
if( NOT USE_SYSTEM_LIBS OR NOT PCRE2_FOUND )
    add_definitions( -DPCRE2_STATIC )
endif()

# Speed up stirng building
add_definitions( -DQT_USE_FAST_CONCATENATION )
add_definitions( -DQT_USE_FAST_OPERATOR_PLUS )

#############################################################################

# "Link time code generation" flags for MSVC
if( MSVC )
    add_definitions( /DUNICODE /D_UNICODE /DHAVE_ROUND )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" )
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG" )

# "Print all warnings" flag for GCC
elseif( CMAKE_COMPILER_IS_GNUCXX )
    add_definitions( -Wall )
endif()


#############################################################################

get_target_property(QMAKE_EXECUTABLE Qt6::qmake LOCATION)
function(QUERY_QMAKE VAR RESULT)
    exec_program(${QMAKE_EXECUTABLE} ARGS "-query ${VAR}" RETURN_VALUE return_code OUTPUT_VARIABLE output )
    if(NOT return_code)
        file(TO_CMAKE_PATH "${output}" output)
        set(${RESULT} ${output} PARENT_SCOPE)
    endif(NOT return_code)
endfunction(QUERY_QMAKE)

# For Mac, add frameworks and make a DMG
if( APPLE )
    query_qmake(QT_INSTALL_TRANSLATIONS QT_TRANSLATIONS_DIR)

    # Set a define that sigil_constants.cpp can then access
    set_property (
        SOURCE sigil_constants.cpp
        PROPERTY COMPILE_DEFINITIONS
        DONT_CHECK_UPDATES=${DISABLE_UPDATE_CHECK}
    )

    if(CMAKE_GENERATOR STREQUAL Xcode)
        set( WORK_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release" )
    else()
        set( WORK_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" )
    endif()
    set( MAIN_PACKAGE_DIR ${WORK_DIR}/Sigil.app )

    if ("${CODE_SIGN_ID}" STREQUAL "")
        add_custom_target(  addframeworks
                            COMMAND macdeployqt Sigil.app
                            WORKING_DIRECTORY ${WORK_DIR} )
#                            DEPENDS ${PROJECT_NAME} )
    else()
        add_custom_target(  addframeworks
                            COMMAND macdeployqt  Sigil.app -codesign="${CODE_SIGN_ID}"
                            WORKING_DIRECTORY ${WORK_DIR}
                            DEPENDS ${PROJECT_NAME} )
    endif()

    add_custom_target(  makedmg
                        COMMAND macdeployqt Sigil.app -dmg
                        WORKING_DIRECTORY ${WORK_DIR})

    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${PROJECT_BINARY_DIR}/*.qm ${WORK_DIR}/Sigil.app/Contents/translations/ )
    foreach( QM ${QM_FILES} )
        # Copy Qt's qm files that coincide with the above
        # message( "QM = ${QM}")
        string( REGEX REPLACE "(.*)(sigil_)(.*)(\\.qm)" "\\1;\\2;\\3;\\4" PATH_ELEMENTS "${QM}" )
        list( GET PATH_ELEMENTS 2 LANG_ID )
        # message( "LANG_ID = ${LANG_ID}")
        set( QTBASE_QM ${QT_TRANSLATIONS_DIR}/qtbase_${LANG_ID}.qm )
        # message( "QTBASE_QM = ${QTBASE_QM}" )
        if ( EXISTS ${QTBASE_QM} )
            add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${QTBASE_QM} ${WORK_DIR}/Sigil.app/Contents/translations/ )
        endif()
    endforeach( QM )
    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${CMAKE_SOURCE_DIR}/src/Resource_Files/dictionaries/* ${WORK_DIR}/Sigil.app/Contents/hunspell_dictionaries )
    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp -r ${CMAKE_SOURCE_DIR}/src/Resource_Files/plugin_launchers/python/* ${WORK_DIR}/Sigil.app/Contents/plugin_launchers/python )
    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp -r ${CMAKE_SOURCE_DIR}/src/Resource_Files/python3lib/* ${WORK_DIR}/Sigil.app/Contents/python3lib )

    if ( PKG_SYSTEM_PYTHON )
        if ( ${USE_NEWER_FINDPYTHON3} )
            set( PY_INTERP ${Python3_EXECUTABLE} )
        else()
            set( PY_INTERP ${PYTHON_EXECUTABLE} )
        endif()
        add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${PY_INTERP} ARGS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/osx_add_python_framework6.py )
        message(STATUS "Using ${PY_INTERP} to bundle python")
    endif()
    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${PROJECT_BINARY_DIR}/*.rcc ${WORK_DIR}/Sigil.app/Contents/Resources/ )
    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${CMAKE_SOURCE_DIR}/src/Resource_Files/examples/* ${WORK_DIR}/Sigil.app/Contents/examples )
    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../lib/*.dylib ${WORK_DIR}/Sigil.app/Contents/lib/ )
    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND install_name_tool -rpath ${CMAKE_BINARY_DIR}/lib "@executable_path/../lib" ${WORK_DIR}/Sigil.app/Contents/MacOS/Sigil )

# For Linux and Windows, provide binary installers.
# For this to work on Windows, Inno Setup's iscc compiler needs to be installed and on the system path.
elseif (MSVC)
    query_qmake(QT_INSTALL_LIBS QT_LIBRARY_DIR)
    query_qmake(QT_INSTALL_BINS QT_INSTALL_BINS)
    query_qmake(QT_INSTALL_PLUGINS QT_PLUGINS_DIR)
    query_qmake(QT_INSTALL_TRANSLATIONS QT_TRANSLATIONS_DIR)

    # Set a define that sigil_constants.cpp can then access
    set_property (
        SOURCE sigil_constants.cpp
        PROPERTY COMPILE_DEFINITIONS
        DONT_CHECK_UPDATES=${DISABLE_UPDATE_CHECK}
    )

    set( TEMP_PACKAGE_DIR ${CMAKE_BINARY_DIR}/temp_folder )
    set( MAIN_PACKAGE_DIR ${TEMP_PACKAGE_DIR}/Sigil )
    set( OUTPUT_PACKAGE_DIR ${CMAKE_BINARY_DIR}/installer )
    set( PYTHON_DEST_DIR ${MAIN_PACKAGE_DIR}/python3 )
    set( VCREDIST_VER "2017" )
    if ( USE_ALT_ICONS )
        set( ICON_SRC_PATH  ${PROJECT_SOURCE_DIR}/Resource_Files/icon/app_icons_alt )
    else()
        set( ICON_SRC_PATH  ${PROJECT_SOURCE_DIR}/Resource_Files/icon/app_icons_orig )
    endif()

    # ISS conf file for the Inno Setup compiler
    # We first create a CMake configured version of the ISS file,
    # and then we copy it to the temp folder every time we need to build the installer.
    set( ISS_MAIN_LOCATION ${CMAKE_SOURCE_DIR}/installer/Sigil.iss  )
    set( ISS_CONFIGURED_LOCATION ${CMAKE_BINARY_DIR}/Sigil_configured.iss )
    set( ISS_TEMP_LOCATION ${CMAKE_BINARY_DIR}/temp_folder/Sigil_configured.iss )

    # Specify platform var for Inno
    if ( 64_BIT_PLATFORM )
        # Used in the ISS CMake configuration
        set( ISS_ARCH "x64" )
        set( ISS_SETUP_FILENAME_PLATFORM "-x64" )
    endif()

    # Creates a copy of the ISS file in ${ISS_CONFIGURED_LOCATION} and then configures it
    # Used in the ISS CMake configuration
    set( LICENSE_LOCATION ${CMAKE_SOURCE_DIR}/installer/win_installer_note.txt  )
    configure_file( ${ISS_MAIN_LOCATION} ${ISS_CONFIGURED_LOCATION} )

    if ( PKG_SYSTEM_PYTHON )
        # Include PyQt5 with the bundled Python by default.
        # Pass -DPACKAGE_PYQT5=0 to initial cmake command to disable.
        if ( NOT DEFINED PACKAGE_PYSIDE6 )
            set( PACKAGE_PYSIDE6 1 )
        endif()
        configure_file( Resource_Files/python_pkg/python_paths6.py ${CMAKE_BINARY_DIR}/python_paths6.py )
        configure_file( Resource_Files/python_pkg/windows_python_gather6.py ${CMAKE_BINARY_DIR}/windows_python_gather6.py COPYONLY )
    endif()

    # Run Inno Setup's iscc compiler (*AFTER* all the PRE_BUILD custom commands execute)
    add_custom_target(  makeinstaller
        COMMAND cmake -E echo "For this to work, Inno Setup's iscc compiler needs to be installed and on the system path."
        COMMAND iscc ${ISS_TEMP_LOCATION} )

    # Intermediate step to create folders and copy files
    set( TARGET_FOR_COPY copyfiles )
    add_custom_target(  copyfiles
                        COMMENT "Copying installer files to temporary location..."
                        DEPENDS ${PROJECT_NAME} )

    add_dependencies( makeinstaller copyfiles )

    # Copy ISS file to temp folder location
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD
                        COMMAND cmake -E copy ${ISS_CONFIGURED_LOCATION} ${ISS_TEMP_LOCATION} )

    # windeployqt
    if ( PACKAGE_PYSIDE6 )
        # Sigil doesn't need QtUiTools or QtOpenGlWidgets to function,
        # but Pyside6 needs them to use the QUiLoader feature.
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND
	        windeployqt.exe --release --no-translations --no-compiler-runtime --dir ${MAIN_PACKAGE_DIR} 
            --libdir ${MAIN_PACKAGE_DIR} -openglwidgets ${MAIN_PACKAGE_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} )
    else()
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND
            windeployqt.exe --release --no-translations --no-compiler-runtime --dir ${MAIN_PACKAGE_DIR} 
            --libdir ${MAIN_PACKAGE_DIR} ${MAIN_PACKAGE_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} )
    endif()

    # Because PySide6 needs Q6tUiTools, but windeploy can't deploy it for some stupid reason when Sigil doesn't need it!!
    set( UITOOLS ${QT_INSTALL_BINS}/Qt6UiTools.dll )
    if ( EXISTS ${UITOOLS} )
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy ${UITOOLS} ${MAIN_PACKAGE_DIR} )
    endif()

    # Copy the translation qm files
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/translations/ )
    foreach( QM ${QM_FILES} )
        # Copy Sigil's qm files
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy ${QM} ${MAIN_PACKAGE_DIR}/translations/ )
        # Copy Qt's qm files that coincide with the above
        # message( "QM = ${QM}")
        string( REGEX REPLACE "(.*)(sigil_)(.*)(\\.qm)" "\\1;\\2;\\3;\\4" PATH_ELEMENTS "${QM}" )
        list( GET PATH_ELEMENTS 2 LANG_ID )
        # message( "LANG_ID = ${LANG_ID}")
        set( QTBASE_QM ${QT_TRANSLATIONS_DIR}/qtbase_${LANG_ID}.qm )
        # message( "QTBASE_QM = ${QTBASE_QM}" )
        if ( EXISTS ${QTBASE_QM} )
            add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy ${QTBASE_QM} ${MAIN_PACKAGE_DIR}/translations/ )
        endif()
    endforeach( QM )

    # Extract the MathJax polyfill archive into the package directory
    set( MATHJAX_CUSTOM_DEST ${MAIN_PACKAGE_DIR}/polyfills )
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MATHJAX_CUSTOM_DEST} )
    add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy ${MATHJAX_CUSTOM} ${MATHJAX_CUSTOM_DEST} )

    # Copy the dictionary files
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/hunspell_dictionaries/ )
    foreach( DIC ${DIC_FILES} )
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy ${DIC} ${MAIN_PACKAGE_DIR}/hunspell_dictionaries/ )
    endforeach( DIC )

    # Copy the plugin launcher files
    add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/Resource_Files/plugin_launchers/python ${MAIN_PACKAGE_DIR}/plugin_launchers/python )

    # Copy python3lib
    add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy_directory ${CMAKE_SOURCE_DIR}/src/Resource_Files/python3lib ${MAIN_PACKAGE_DIR}/python3lib )

    # Create python virtual environment
    if ( PKG_SYSTEM_PYTHON )
        if ( ${USE_NEWER_FINDPYTHON3} )
            set( PY_INTERP ${Python3_EXECUTABLE} )
        else()
            set( PY_INTERP ${PYTHON_EXECUTABLE} )
        endif()
        message(STATUS "Using ${PY_INTERP} to bundle python")
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD
                            COMMAND ${PY_INTERP} ARGS ${CMAKE_BINARY_DIR}/windows_python_gather6.py )
    endif()

    # Add external binary resource files
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/iconthemes/ )
    add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy ${EXT_RCC_FILES} ${MAIN_PACKAGE_DIR}/iconthemes )

    # Copy the example files
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${MAIN_PACKAGE_DIR}/examples/ )
    foreach( EXAMPLE ${EXAMPLE_FILES} )
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND cmake -E copy ${EXAMPLE} ${MAIN_PACKAGE_DIR}/examples/ )
    endforeach( EXAMPLE )

    # Copy hunspell and gumbo dlls to program directory
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD
                        COMMAND cmake -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/hunspell.dll ${MAIN_PACKAGE_DIR} )
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD
                        COMMAND cmake -E copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/sigilgumbo.dll ${MAIN_PACKAGE_DIR} )

    # Set the path of the application executable
    set( EXE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} )

    # Copy the application executable
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD
                        COMMAND cmake -E copy ${EXE_PATH} ${MAIN_PACKAGE_DIR} )

    # We need to copy the CRT dlls
    # Add -DWIN_INSTALLER_USE_64BIT_CRT=1 to the cmake call if you want to build
    # an installer for the x64 verison of Sigil. This will make sure that the
    # correct CRT libs are included in the installer.
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND cmake -E make_directory ${TEMP_PACKAGE_DIR}/vendor/ )
    if ( WIN_INSTALLER_USE_64BIT_CRT )
        message( STATUS "Using the 64 bit CRT in the Sigil Windows installer" )
        message( STATUS "Ensure vcredist_x64.exe for VS${VCREDIST_VER} has been placed in installer/ if you plan on running makeinstaller" )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD
            COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/installer/vcredist_x64.exe ${TEMP_PACKAGE_DIR}/vendor/vcredist.exe )
    else()
        message( STATUS "Using the 32 bit CRT in the Sigil Windows installer" )
        message( STATUS "Ensure vcredist_x86.exe for VS${VCREDIST_VER} has been placed in installer/ if you plan on running makeinstaller" )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD
            COMMAND cmake -E copy ${CMAKE_SOURCE_DIR}/installer/vcredist_x86.exe ${TEMP_PACKAGE_DIR}/vendor/vcredist.exe )
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
                        COMMAND cmake -E copy ${ICON_SRC_PATH}/app_icon_48.png ${MAIN_PACKAGE_DIR}/misc/ )

    # Copy the SGF ICO file (used on Windows as the SGF file type icon)
    #add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD
    #                    COMMAND cmake -E copy ${PROJECT_SOURCE_DIR}/Resource_Files/icon/sgf.ico ${MAIN_PACKAGE_DIR}/misc/ )

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
# By default, the prefix is /usr/local
#
# You can also control the share installation location
#
#   cmake -DSHARE_INSTALL_PREFIX=/new/share/install/prefix
#
# By default, the prefix is the same as CMAKE_INSTALL_PREFIX
#
# Use 'make install' to install normally
#
#
if( UNIX AND NOT APPLE )
    # Leave these so package future packaging scripts can find Qt and its plugins.
    query_qmake(QT_INSTALL_LIBS QT_LIBRARY_DIR)
    query_qmake(QT_INSTALL_PLUGINS QT_PLUGINS_DIR)

    if ( NOT SHARE_INSTALL_PREFIX )
        set ( SHARE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} )
    endif()

    set ( SIGIL_SHARE_ROOT "${SHARE_INSTALL_PREFIX}/share/sigil" )

    # Set some defines that sigil_constants.cpp can then access
    set_property (
        SOURCE sigil_constants.cpp
        PROPERTY COMPILE_DEFINITIONS
        SIGIL_SHARE_ROOT="${SIGIL_SHARE_ROOT}" DICTS_ARE_BUNDLED=${INSTALL_BUNDLED_DICTS}
        EXTRA_DICT_DIRS="${EXTRA_DICT_DIRS}" MATHJAX3_DIR="${MATHJAX3_DIR}"
        DONT_CHECK_UPDATES=${DISABLE_UPDATE_CHECK}
    )

    # Launch script for sigil
    set( LINUX_LAUNCH_INSTALL_SCRIPT_CONFIGURED ${CMAKE_BINARY_DIR}/sigil-sh_install_configured )
    set( SIGIL_EXECUTABLE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} )

    # Destination directory for our custom MathJax
    set( MATHJAX_CUSTOM_DEST "${CMAKE_BINARY_DIR}/polyfills" )
    # Remove previous directories
    if ( EXISTS ${MATHJAX_CUSTOM_DEST} )
        file( REMOVE_RECURSE ${MATHJAX_CUSTOM_DEST} )
    endif()
    # Create the polyfills dir and copy the MathJax custom build into it
    if ( NOT DEFINED MATHJAX3_DIR )
        file( MAKE_DIRECTORY ${MATHJAX_CUSTOM_DEST} )
        execute_process( COMMAND cmake -E copy ${MATHJAX_CUSTOM} ${MATHJAX_CUSTOM_DEST} )
    endif()

    # Configure Linux launch script
    configure_file( ${LINUX_LAUNCH_INSTALL_SCRIPT} ${LINUX_LAUNCH_INSTALL_SCRIPT_CONFIGURED} )

    if ( CMAKE_BUILD_TYPE STREQUAL "Debug")
        set( HUNSPELL_NAME ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libhunspelld.so )
        set( GUMBO_NAME ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libsigilgumbod.so )
    else()
        set( HUNSPELL_NAME ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libhunspell.so )
        set( GUMBO_NAME ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libsigilgumbo.so )
    endif()

    # Standard Linux 'make install'
    install( TARGETS sigil DESTINATION ${CMAKE_INSTALL_LIBDIR}/sigil )
    install( PROGRAMS ${LINUX_LAUNCH_INSTALL_SCRIPT_CONFIGURED} DESTINATION ${CMAKE_INSTALL_BINDIR} RENAME ${PROJECT_NAME} )
    if( NOT USE_SYSTEM_LIBS OR NOT HUNSPELL_FOUND )
        install( PROGRAMS ${HUNSPELL_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/sigil )
    endif()
    install( PROGRAMS ${GUMBO_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/sigil )
    if( BUILD_PATCHED_LIBXML2 )
        install( PROGRAMS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libsigilxml2.so.2.9.4 DESTINATION ${CMAKE_INSTALL_LIBDIR}/sigil )
        install( FILES ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libxml2.so.2 DESTINATION ${CMAKE_INSTALL_LIBDIR}/sigil )
    endif()
    install( FILES ${LINUX_DESKTOP_FILE} DESTINATION ${SHARE_INSTALL_PREFIX}/share/applications/ )

    if ( USE_ALT_ICONS )
        set( ICON_SRC_PATH  ${PROJECT_SOURCE_DIR}/Resource_Files/icon/app_icons_alt )
    else()
        set( ICON_SRC_PATH  ${PROJECT_SOURCE_DIR}/Resource_Files/icon/app_icons_orig )
    endif()
    if( INSTALL_HICOLOR_ICONS )
        set( ICON_SIZE 16 32 48 64 128 256 512)
        foreach( SIZE ${ICON_SIZE} )
            install( FILES ${ICON_SRC_PATH}/app_icon_${SIZE}.png DESTINATION
                               ${SHARE_INSTALL_PREFIX}/share/icons/hicolor/${SIZE}x${SIZE}/apps RENAME sigil.png )
        endforeach( SIZE )
        install( FILES ${ICON_SRC_PATH}/app_icon_scalable.svg DESTINATION
                               ${SHARE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps RENAME sigil.svg )
    else()
        install( FILES ${ICON_SRC_PATH}/${LINUX_DESKTOP_ICON_FILE} DESTINATION ${SHARE_INSTALL_PREFIX}/share/pixmaps RENAME sigil.png )
    endif()
    install( FILES ${QM_FILES} DESTINATION ${SIGIL_SHARE_ROOT}/translations/ )
    if ( INSTALL_BUNDLED_DICTS )
        install( FILES ${DIC_FILES} DESTINATION ${SIGIL_SHARE_ROOT}/hunspell_dictionaries/ )
    endif()
    install( FILES ${EXT_RCC_FILES} DESTINATION ${SIGIL_SHARE_ROOT}/iconthemes/ )
    if ( MATHJAX3_DIR )
        # Fixme - we need to figure out how to specify svg with mml3 extension only for external mathjax
        # install( FILES ${CMAKE_SOURCE_DIR}/src/Resource_Files/polyfills/SIGIL_EBOOK_MML_SVG.js DESTINATION ${MATHJAX_DIR}/config/local/ )
    else()
        install( FILES ${MATHJAX_CUSTOM} DESTINATION ${SIGIL_SHARE_ROOT}/polyfills/ )
    endif()
    install( DIRECTORY ${CMAKE_SOURCE_DIR}/src/Resource_Files/plugin_launchers/python/ DESTINATION ${SIGIL_SHARE_ROOT}/plugin_launchers/python )
    install( DIRECTORY ${CMAKE_SOURCE_DIR}/src/Resource_Files/python3lib/ DESTINATION ${SIGIL_SHARE_ROOT}/python3lib )
    install( FILES ${EXAMPLE_FILES} DESTINATION ${SIGIL_SHARE_ROOT}/examples/ )
endif()
