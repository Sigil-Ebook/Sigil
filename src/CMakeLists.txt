########################################################
#
#  This is a CMake configuration file.
#  To use it you need CMake which can be
#  downloaded from here:
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################


# Print a message and fail for people who don't
# read the build instructions and then complain
# when the build process fails for them.
if ( ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR} )
    message( FATAL_ERROR "You are trying to run CMake from the <top_folder>/src/ directory, "
                         "instead of just from the <top_folder> directory.\nDO NOT DO THIS.\n"
                         "The correct way looks like this:\n"
                         "  cmake -G '<generator_name>' /path/to/topmost/folder/in/source/package\n"
                         "You will probably now need to first clean your build directory." )
endif()

# Source File Group Definitions
set( MAIN_FILES
    sigil_constants.h
    sigil_constants.cpp
    sigil_exception.h
    main.cpp
    )

set( BOOK_MANIPULATION_FILES
    BookManipulation/Book.cpp
    BookManipulation/Book.h
    BookManipulation/BookReports.cpp
    BookManipulation/BookReports.h
    BookManipulation/Index.cpp
    BookManipulation/Index.h
    BookManipulation/CleanSource.cpp
    BookManipulation/CleanSource.h
    BookManipulation/FolderKeeper.cpp
    BookManipulation/FolderKeeper.h
    BookManipulation/Headings.cpp
    BookManipulation/Headings.h
    BookManipulation/HTMLMetadata.cpp
    BookManipulation/HTMLMetadata.h
    BookManipulation/XhtmlDoc.cpp
    BookManipulation/XhtmlDoc.h
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
    ResourceObjects/ImageResource.cpp
    ResourceObjects/ImageResource.h
    ResourceObjects/AudioResource.cpp
    ResourceObjects/AudioResource.h
    ResourceObjects/VideoResource.cpp
    ResourceObjects/VideoResource.h
    ResourceObjects/MiscTextResource.cpp
    ResourceObjects/MiscTextResource.h
    ResourceObjects/SVGResource.cpp
    ResourceObjects/SVGResource.h
    ResourceObjects/FontResource.cpp
    ResourceObjects/FontResource.h
    ResourceObjects/OPFResource.cpp
    ResourceObjects/OPFResource.h
    ResourceObjects/NavProcessor.cpp
    ResourceObjects/NavProcessor.h
    ResourceObjects/NCXResource.cpp
    ResourceObjects/NCXResource.h
    ResourceObjects/XMLResource.cpp
    ResourceObjects/XMLResource.h
    )

set( DIALOG_FILES
    Dialogs/About.cpp
    Dialogs/About.h
    Dialogs/AddSemantics.cpp
    Dialogs/AddSemantics.h
    Dialogs/AddMetadata.cpp
    Dialogs/AddMetadata.h
    Dialogs/ClipboardHistorySelector.h
    Dialogs/ClipboardHistorySelector.cpp
    Dialogs/DeleteFiles.cpp
    Dialogs/DeleteFiles.h
    Dialogs/DeleteStyles.cpp
    Dialogs/DeleteStyles.h
    Dialogs/EditTOC.cpp
    Dialogs/EditTOC.h
    Dialogs/EmptyLayout.cpp
    Dialogs/EmptyLayout.h
    Dialogs/ManageRepos.cpp
    Dialogs/ManageRepos.h
    Dialogs/OpenWithName.cpp
    Dialogs/OpenWithName.h
    Dialogs/RepoLog.cpp
    Dialogs/RepoLog.h
    Dialogs/ChgViewer.cpp
    Dialogs/ChgViewer.h
    Dialogs/SourceViewer.cpp
    Dialogs/SourceViewer.h
    Dialogs/ListSelector.h
    Dialogs/CPCompare.cpp
    Dialogs/CPCompare.h
    Dialogs/RERenamer.cpp
    Dialogs/RERenamer.h
    Dialogs/RETable.cpp
    Dialogs/RETable.h
    Dialogs/SelectCharacter.cpp
    Dialogs/SelectCharacter.h
    Dialogs/SelectCheckpoint.cpp
    Dialogs/SelectCheckpoint.h
    Dialogs/SelectFolder.cpp
    Dialogs/SelectFolder.h
    Dialogs/SelectHyperlink.cpp
    Dialogs/SelectHyperlink.h
    Dialogs/SelectId.cpp
    Dialogs/SelectId.h
    Dialogs/SelectIndexTitle.cpp
    Dialogs/SelectIndexTitle.h
    Dialogs/SelectFiles.cpp
    Dialogs/SelectFiles.h
    Dialogs/AddAutomateTool.cpp
    Dialogs/AddAutomateTool.h
    Dialogs/AddAutomatePlugin.cpp
    Dialogs/AddAutomatePlugin.h
    Dialogs/AutomateEditor.cpp
    Dialogs/AutomateEditor.h
    Dialogs/MetaEditor.cpp
    Dialogs/MetaEditor.h
    Dialogs/TreeItem.cpp
    Dialogs/TreeItem.h
    Dialogs/TreeModel.cpp
    Dialogs/TreeModel.h
    Dialogs/MetaEditorItemDelegate.cpp
    Dialogs/MetaEditorItemDelegate.h
    Dialogs/HeadingSelector.cpp
    Dialogs/HeadingSelector.h
    Dialogs/PluginRunner.cpp
    Dialogs/PluginRunner.h
    Dialogs/Preferences.cpp
    Dialogs/Preferences.h
    Dialogs/PreferenceWidgets/AppearanceWidget.cpp
    Dialogs/PreferenceWidgets/AppearanceWidget.h
    Dialogs/PreferenceWidgets/GeneralSettingsWidget.cpp
    Dialogs/PreferenceWidgets/GeneralSettingsWidget.h
    Dialogs/PreferenceWidgets/KeyboardShortcutsWidget.cpp
    Dialogs/PreferenceWidgets/KeyboardShortcutsWidget.h
    Dialogs/PreferenceWidgets/LanguageWidget.cpp
    Dialogs/PreferenceWidgets/LanguageWidget.h
    Dialogs/PreferenceWidgets/PreferencesWidget.h
    Dialogs/PreferenceWidgets/SpellCheckWidget.cpp
    Dialogs/PreferenceWidgets/SpellCheckWidget.h
    Dialogs/PreferenceWidgets/PreserveEntitiesWidget.cpp
    Dialogs/PreferenceWidgets/PreserveEntitiesWidget.h
    Dialogs/PreferenceWidgets/PluginWidget.cpp
    Dialogs/PreferenceWidgets/PluginWidget.h
    Dialogs/RenameTemplate.cpp
    Dialogs/RenameTemplate.h
    Dialogs/Reports.cpp
    Dialogs/Reports.h
    Dialogs/ReportsWidgets/AllFilesWidget.cpp
    Dialogs/ReportsWidgets/AllFilesWidget.h
    Dialogs/ReportsWidgets/HTMLFilesWidget.cpp
    Dialogs/ReportsWidgets/HTMLFilesWidget.h
    Dialogs/ReportsWidgets/LinksWidget.cpp
    Dialogs/ReportsWidgets/LinksWidget.h
    Dialogs/ReportsWidgets/ImageFilesWidget.cpp
    Dialogs/ReportsWidgets/ImageFilesWidget.h
    Dialogs/ReportsWidgets/CSSFilesWidget.cpp
    Dialogs/ReportsWidgets/CSSFilesWidget.h
    Dialogs/ReportsWidgets/ClassesInHTMLFilesWidget.cpp
    Dialogs/ReportsWidgets/ClassesInHTMLFilesWidget.h
    Dialogs/ReportsWidgets/StylesInCSSFilesWidget.cpp
    Dialogs/ReportsWidgets/StylesInCSSFilesWidget.h
    Dialogs/ReportsWidgets/CharactersInHTMLFilesWidget.cpp
    Dialogs/ReportsWidgets/CharactersInHTMLFilesWidget.h
    Dialogs/ReportsWidgets/ReportsWidget.h
    Dialogs/LinkStylesheets.cpp
    Dialogs/LinkStylesheets.h
    Dialogs/LinkJavascripts.cpp
    Dialogs/LinkJavascripts.h
    Dialogs/SearchEditor.cpp
    Dialogs/SearchEditor.h
    Dialogs/Controls.cpp
    Dialogs/Controls.h
    Dialogs/SearchEditorItemDelegate.cpp
    Dialogs/SearchEditorItemDelegate.h
    Dialogs/CountsReport.cpp
    Dialogs/CountsReport.h
    Dialogs/DryRunReplace.cpp
    Dialogs/DryRunReplace.h
    Dialogs/ReplacementChooser.cpp
    Dialogs/ReplacementChooser.h
    Dialogs/StyledTextDelegate.cpp
    Dialogs/StyledTextDelegate.h
    Dialogs/ClipEditor.cpp
    Dialogs/ClipEditor.h
    Dialogs/IndexEditor.cpp
    Dialogs/IndexEditor.h
    Dialogs/SpellcheckEditor.cpp
    Dialogs/SpellcheckEditor.h
    Dialogs/ViewImage.cpp
    Dialogs/ViewImage.h
    Dialogs/ViewAV.cpp
    Dialogs/ViewAV.h
    Dialogs/ViewFont.cpp
    Dialogs/ViewFont.h
    Dialogs/Inspector.cpp
    Dialogs/Inspector.h
    )

set( WIDGET_FILES
    Widgets/ImageView.cpp
    Widgets/ImageView.h
    Widgets/AVView.cpp
    Widgets/AVView.h
    Widgets/FontView.cpp
    Widgets/FontView.h
    Widgets/Navigator.h
    Widgets/Navigator2.h
    Widgets/TextView.cpp
    Widgets/TextView.h
    Widgets/TextDocument.h
    Widgets/TextDocument.cpp
    Widgets/TVLineNumberArea.cpp
    Widgets/TVLineNumberArea.h
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
    Importers/ImporterFactory.cpp
    Importers/ImporterFactory.h
    Importers/ImportHTML.cpp
    Importers/ImportHTML.h
    Importers/ImportTXT.cpp
    Importers/ImportTXT.h
    )

set ( QUERY_FILES
    Query/CNode.cpp
    Query/CNode.h
    Query/CObject.cpp
    Query/CObject.h
    Query/CParser.cpp
    Query/CParser.h
    Query/CQueryUtil.cpp
    Query/CQueryUtil.h
    Query/CSelection.cpp
    Query/CSelection.h
    Query/CSelector.cpp
    Query/CSelector.h
    )


set( PARSERS_FILES
    Parsers/CSSInfo.cpp
    Parsers/CSSInfo.h
    Parsers/CSSToolbox.cpp
    Parsers/CSSToolbox.h
    Parsers/HTMLStyleInfo.cpp
    Parsers/HTMLStyleInfo.h
    Parsers/qCSSParser.cpp
    Parsers/qCSSParser.h
    Parsers/qCSSUtils.cpp
    Parsers/qCSSUtils.h
    Parsers/qCSSProperties.cpp
    Parsers/qCSSProperties.h
    Parsers/GumboInterface.h
    Parsers/GumboInterface.cpp
    Parsers/TagAtts.cpp
    Parsers/TagAtts.h
    Parsers/QuickParser.cpp
    Parsers/QuickParser.h
    Parsers/TagLister.cpp
    Parsers/TagLister.h
    Parsers/OPFParser.cpp
    Parsers/OPFParser.h
   )

set( EMBEDPYTHON_FILES
    EmbedPython/DiffRec.h
    EmbedPython/PyObjectPtr.h
    EmbedPython/PyObjectPtr.cpp
    EmbedPython/EmbeddedPython.h
    EmbedPython/EmbeddedPython.cpp
    EmbedPython/PythonRoutines.h
    EmbedPython/PythonRoutines.cpp
    )

set( MISC_FILES
    Misc/AppEventFilter.cpp
    Misc/AppEventFilter.h
    Misc/AsciiFy.cpp
    Misc/AsciiFy.h
    Misc/UpdateChecker.cpp
    Misc/UpdateChecker.h
    Misc/URLInterceptor.cpp
    Misc/URLInterceptor.h
    Misc/URLSchemeHandler.cpp
    Misc/URLSchemeHandler.h
    Misc/Utility.cpp
    Misc/Utility.h
    Misc/SleepFunctions.h
    Misc/FindReplaceQLineEdit.cpp
    Misc/FindReplaceQLineEdit.h
    Misc/FilenameDelegate.cpp
    Misc/FilenameDelegate.h
    Misc/XHTMLHighlighter.cpp
    Misc/XHTMLHighlighter.h
    Misc/XHTMLHighlighter2.cpp
    Misc/XHTMLHighlighter2.h
    Misc/CSSHighlighter.cpp
    Misc/CSSHighlighter.h
    Misc/HTMLEncodingResolver.cpp
    Misc/HTMLEncodingResolver.h
    Misc/HTMLSpellCheck.cpp
    Misc/HTMLSpellCheck.h
    Misc/HTMLSpellCheckML.cpp
    Misc/HTMLSpellCheckML.h
    Misc/PasteTargetComboBox.cpp
    Misc/PasteTargetComboBox.h
    Misc/PasteTarget.h
    Misc/Plugin.cpp
    Misc/Plugin.h
    Misc/PluginDB.cpp
    Misc/PluginDB.h
    Misc/QCodePage437Codec.cpp
    Misc/QCodePage437Codec.h
    Misc/SearchOperations.cpp
    Misc/SearchOperations.h
    Misc/SigilDarkStyle.cpp
    Misc/SigilDarkStyle.h
    Misc/Language.cpp
    Misc/Language.h
    Misc/DescriptiveInfo.h
    Misc/GuideItems.h
    Misc/GuideItems.cpp
    Misc/Landmarks.h
    Misc/Landmarks.cpp
    Misc/MarcRelators.cpp
    Misc/MarcRelators.h
    Misc/UILanguage.cpp
    Misc/UILanguage.h
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
    Misc/Ncx20051Dtd.cpp
    Misc/FontObfuscation.cpp
    Misc/FontObfuscation.h
    Misc/TempFolder.cpp
    Misc/TempFolder.h
    Misc/OpenExternally.cpp
    Misc/OpenExternally.h
    Misc/TOCHTMLWriter.cpp
    Misc/TOCHTMLWriter.h
    Misc/NumericItem.h
    Misc/LocaleAwareItem.h
    Misc/ValidationResult.h
    Misc/ValidationResult.cpp
    Misc/WrapIndicator.h
    Misc/XMLEntities.cpp
    Misc/XMLEntities.h
    Misc/MediaTypes.cpp
    Misc/MediaTypes.h
    )

set( MISC_EDITORS_FILES
    MiscEditors/ClipEditorModel.cpp
    MiscEditors/ClipEditorModel.h
    MiscEditors/ClipEditorTreeView.cpp
    MiscEditors/ClipEditorTreeView.h
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
    MiscEditors/SearchEditorModel.cpp
    MiscEditors/SearchEditorModel.h
    )

set( SPCRE_FILES
    PCRE2/SPCRE.cpp
    PCRE2/SPCRE.h
    PCRE2/PCRECache.cpp
    PCRE2/PCRECache.h
    PCRE2/PCREReplaceTextBuilder.cpp
    PCRE2/PCREReplaceTextBuilder.h
    PCRE2/PCREErrors.cpp
    PCRE2/PCREErrors.h
    )

set( VIEW_EDITOR_FILES
    ViewEditors/CodeViewEditor.cpp
    ViewEditors/CodeViewEditor.h
    ViewEditors/WebEngPage.cpp
    ViewEditors/WebEngPage.h
    ViewEditors/SimplePage.cpp
    ViewEditors/SimplePage.h
    ViewEditors/ViewPreview.cpp
    ViewEditors/ViewPreview.h
    ViewEditors/LineNumberArea.cpp
    ViewEditors/LineNumberArea.h
    ViewEditors/Searchable.cpp
    ViewEditors/Searchable.h
    ViewEditors/Zoomable.h
    ViewEditors/ElementIndex.h
    ViewEditors/ViewEditor.h
    ViewEditors/Viewer.h
    ViewEditors/Overlay.h
     )

set( MAINUI_FILES
    MainUI/MainApplication.cpp
    MainUI/MainApplication.h
    MainUI/MainWindow.cpp
    MainUI/MainWindow.h
    MainUI/FindReplace.cpp
    MainUI/FindReplace.h
    MainUI/BookBrowser.cpp
    MainUI/BookBrowser.h
    MainUI/ClipsWindow.cpp
    MainUI/ClipsWindow.h
    MainUI/PreviewWindow.cpp
    MainUI/PreviewWindow.h
    MainUI/TableOfContents.cpp
    MainUI/TableOfContents.h
    MainUI/OPFModel.cpp
    MainUI/OPFModel.h
    MainUI/OPFModelItem.cpp
    MainUI/OPFModelItem.h
    MainUI/TOCModel.cpp
    MainUI/TOCModel.h
    MainUI/ValidationResultsView.cpp
    MainUI/ValidationResultsView.h
    )

set( TAB_FILES
    Tabs/TabBar.cpp
    Tabs/TabBar.h
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
    Tabs/AVTab.cpp
    Tabs/AVTab.h
    Tabs/FontTab.cpp
    Tabs/FontTab.h
    Tabs/ImageTab.cpp
    Tabs/ImageTab.h
    Tabs/MiscTextTab.cpp
    Tabs/MiscTextTab.h
    Tabs/SVGTab.cpp
    Tabs/SVGTab.h
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
    Form_Files/ClipboardHistorySelector.ui
    Form_Files/DeleteFiles.ui
    Form_Files/DeleteStyles.ui
    Form_Files/ManageRepos.ui
    Form_Files/OpenWithName.ui
    Form_Files/RERenamer.ui
    Form_Files/RETable.ui
    Form_Files/SelectCharacter.ui
    Form_Files/SelectFolder.ui
    Form_Files/SelectHyperlink.ui
    Form_Files/SelectId.ui
    Form_Files/SelectIndexTitle.ui
    Form_Files/SelectFiles.ui
    Form_Files/MetaEditor.ui
    Form_Files/AddAutomateTool.ui
    Form_Files/AddAutomatePlugin.ui
    Form_Files/AutomateEditor.ui
    Form_Files/EmptyLayout.ui
    Form_Files/AddMetadata.ui
    Form_Files/AddSemantics.ui
    Form_Files/About.ui
    Form_Files/EditTOC.ui
    Form_Files/HeadingSelector.ui
    Form_Files/FindReplace.ui
    Form_Files/PAppearanceWidget.ui
    Form_Files/Preferences.ui
    Form_Files/PKeyboardShortcutsWidget.ui
    Form_Files/PGeneralSettingsWidget.ui
    Form_Files/PLanguageWidget.ui
    Form_Files/PSpellCheckWidget.ui
    Form_Files/PPreserveEntitiesWidget.ui
    Form_Files/PPluginWidget.ui
    Form_Files/RenameTemplate.ui
    Form_Files/Reports.ui
    Form_Files/ReportsAllFilesWidget.ui
    Form_Files/ReportsHTMLFilesWidget.ui
    Form_Files/ReportsLinksWidget.ui
    Form_Files/ReportsImageFilesWidget.ui
    Form_Files/ReportsCSSFilesWidget.ui
    Form_Files/ReportsClassesInHTMLFilesWidget.ui
    Form_Files/ReportsStylesInCSSFilesWidget.ui
    Form_Files/ReportsCharactersInHTMLFilesWidget.ui
    Form_Files/LinkStylesheets.ui
    Form_Files/LinkJavascripts.ui
    Form_Files/SelectCheckpoint.ui
    Form_Files/SearchEditor.ui
    Form_Files/ClipEditor.ui
    Form_Files/IndexEditor.ui
    Form_Files/SpellcheckEditor.ui
    Form_Files/PluginRunner.ui
    Form_Files/Controls.ui
    Form_Files/CountsReport.ui
    Form_Files/DryRunReplace.ui
    Form_Files/ReplacementChooser.ui
    )

set( SOURCEUPDATE_FILES
    SourceUpdates/PerformHTMLUpdates.cpp
    SourceUpdates/PerformHTMLUpdates.h
    SourceUpdates/PerformOPFUpdates.cpp
    SourceUpdates/PerformOPFUpdates.h
    SourceUpdates/PerformNCXUpdates.cpp
    SourceUpdates/PerformNCXUpdates.h
    SourceUpdates/PerformCSSUpdates.cpp
    SourceUpdates/PerformCSSUpdates.h
    SourceUpdates/PerformXMLUpdates.cpp
    SourceUpdates/PerformXMLUpdates.h
    SourceUpdates/AnchorUpdates.cpp
    SourceUpdates/AnchorUpdates.h
    SourceUpdates/FragmentUpdates.cpp
    SourceUpdates/FragmentUpdates.h
    SourceUpdates/LinkUpdates.cpp
    SourceUpdates/LinkUpdates.h
    SourceUpdates/JavascriptUpdates.cpp
    SourceUpdates/JavascriptUpdates.h
    SourceUpdates/WordUpdates.cpp
    SourceUpdates/WordUpdates.h
    SourceUpdates/UniversalUpdates.cpp
    SourceUpdates/UniversalUpdates.h
    )

set( QRC_FILES
    Resource_Files/About/About.qrc
    Resource_Files/javascript/javascript.qrc
    Resource_Files/icon/icon.qrc
    Resource_Files/dark/dark.qrc
    )
if( UNIX AND NOT APPLE )
    if ( USE_ALT_ICONS )
        LIST( APPEND QRC_FILES Resource_Files/icon/app_icons_alt/app_icons.qrc )
    else()
        LIST( APPEND QRC_FILES Resource_Files/icon/app_icons_orig/app_icons.qrc )
    endif()
endif()

set( MAIN_BINARY_QRC_FILES
    Resource_Files/main/main.qrc
    )
set( FLUENT_BINARY_QRC_FILES
    Resource_Files/fluent/fluent.qrc
    )
set( MATERIAL_BINARY_QRC_FILES
    Resource_Files/material/material.qrc
    )

if ( APPLE )
    LIST( APPEND MISC_FILES Misc/macos_menu_and_window_fixes.mm )
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework AppKit")
endif()

file( GLOB TS_FILES Resource_Files/ts/sigil_*.ts )

file( GLOB DIC_FILES Resource_Files/dictionaries/* )

file( GLOB PLUGIN_FILES_PYTHON Resource_Files/plugin_launchers/python/* )

file( GLOB EXAMPLE_FILES Resource_Files/examples/* )

set( LINUX_DESKTOP_FILE
    Resource_Files/freedesktop/sigil.desktop
    )

set( LINUX_DESKTOP_ICON_FILE
    app_icon_48.png
    )

set( LINUX_LAUNCH_INSTALL_SCRIPT
    Resource_Files/bash/sigil-sh_install
    )

#############################################################################
# We use the lower case name
# on UNIX systems other than Mac OS X
#############################################################################
if ( WIN32 OR APPLE )
    project( Sigil )
else()
    project( sigil )
endif()

# Building Sigil with Qt6 requires cmake 3.16+
# and a C++17 standards compatible compiler
if (${USE_QT6})
    cmake_minimum_required( VERSION 3.16 )

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    include(qt6sigil.cmake)

# Building Sigil with Qt5 requires cmake 3.0+
# and a C++11 standards compatible compiler
else()
    cmake_minimum_required( VERSION 3.0.0 )

    if (NOT MSVC)
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
        CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
        if(COMPILER_SUPPORTS_CXX11)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        elseif(COMPILER_SUPPORTS_CXX0X)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
            # Give gcc compilers that fall through the cracks a shot.
            if(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 4.8)
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
                message("-- fpermissive CXX flag being used for gcc ${CMAKE_CXX_COMPILER_VERSION}")
            endif()
        else()
            message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
        endif()
    endif()

    include(qt5sigil.cmake)
endif()
