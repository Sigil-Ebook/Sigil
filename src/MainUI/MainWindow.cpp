/************************************************************************
**
**  Copyright (C) 2015-2021 Kevin B. Hendricks, Stratford Ontario Canada
**  Copyright (C) 2015-2021 Doug Massay
**  Copyright (C) 2012-2015 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012-2013 Dave Heiland
**  Copyright (C) 2009-2011 Strahinja Markovic  <strahinja.markovic@gmail.com>
**
**  This file is part of Sigil.
**
**  Sigil is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  Sigil is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Sigil.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include <QtCore/QFileInfo>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtConcurrent>
#include <QFuture>
#include <QtGui/QDesktopServices>
#include <QtGui/QImage>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QScreen>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QToolBar>
#include <QtWebEngineWidgets/QWebEngineSettings>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#ifdef Q_OS_WIN32
#include <QtWinExtras>
#endif
#include <QString>
#include <QStringList>
#include <QFont>
#include <QFontMetrics>
#include <QEvent>
#include <QWindowStateChangeEvent>
#include <QStyleFactory>
#include <QStyle>
#include <QDebug>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/Index.h"
#include "BookManipulation/FolderKeeper.h"
#include "Dialogs/About.h"
#include "Dialogs/ClipEditor.h"
#include "Dialogs/ClipboardHistorySelector.h"
#include "Dialogs/DeleteStyles.h"
#include "Dialogs/EditTOC.h"
#include "Dialogs/EmptyLayout.h"
#include "Dialogs/HeadingSelector.h"
#include "Dialogs/LinkStylesheets.h"
#include "Dialogs/LinkJavascripts.h"
#include "Dialogs/ManageRepos.h"
#include "Dialogs/AutomateEditor.h"
#include "Dialogs/MetaEditor.h"
#include "Dialogs/PluginRunner.h"
#include "Dialogs/Preferences.h"
#include "Dialogs/RepoLog.h"
#include "Dialogs/ChgViewer.h"
#include "Dialogs/CPCompare.h"
#include "Dialogs/SearchEditor.h"
#include "Dialogs/SelectCharacter.h"
#include "Dialogs/SelectCheckpoint.h"
#include "Dialogs/SelectFiles.h"
#include "Dialogs/SelectHyperlink.h"
#include "Dialogs/SelectId.h"
#include "Dialogs/SelectIndexTitle.h"
#include "Exporters/ExportEPUB.h"
#include "Exporters/ExporterFactory.h"
#include "Importers/ImporterFactory.h"
#include "Importers/ImportHTML.h"
#include "MainUI/BookBrowser.h"
#include "MainUI/ClipsWindow.h"
#include "MainUI/MainWindow.h"
#include "MainUI/FindReplace.h"
#include "MainUI/PreviewWindow.h"
#include "MainUI/TableOfContents.h"
#include "MainUI/ValidationResultsView.h"
#include "Misc/HTMLSpellCheck.h"
#include "Misc/HTMLSpellCheckML.h"
#include "Misc/KeyboardShortcutManager.h"
#include "Misc/Landmarks.h"
#include "Misc/MediaTypes.h"
#include "Misc/OpenExternally.h"
#include "Misc/Plugin.h"
#include "Misc/PluginDB.h"
#include "EmbedPython/PythonRoutines.h"
#include "Misc/SettingsStore.h"
#include "Misc/SleepFunctions.h"
#include "Misc/SpellCheck.h"
#include "Misc/TempFolder.h"
#include "Misc/TOCHTMLWriter.h"
#include "Misc/Utility.h"
#include "MiscEditors/IndexHTMLWriter.h"
#include "Parsers/HTMLStyleInfo.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NavProcessor.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "SourceUpdates/LinkUpdates.h"
#include "SourceUpdates/JavascriptUpdates.h"
#include "SourceUpdates/WordUpdates.h"
#include "SourceUpdates/FragmentUpdates.h"
#include "Tabs/FlowTab.h"
#include "Tabs/CSSTab.h"
#include "Tabs/OPFTab.h"
#include "Tabs/TabManager.h"
#include "MainUI/MainApplication.h"

#define DWINGEO if(0)
#define DBG if(0)

static const int TEXT_ELIDE_WIDTH   = 300;
static const QString SETTINGS_GROUP = "mainwindow";
const float ZOOM_STEP               = 0.1f;
const float ZOOM_MIN                = 0.09f;
const float ZOOM_MAX                = 5.0f;
const float ZOOM_NORMAL             = 1.0f;
static const int ZOOM_SLIDER_MIN    = 0;
static const int ZOOM_SLIDER_MAX    = 1000;
static const int ZOOM_SLIDER_MIDDLE = 500;
static const int ZOOM_SLIDER_WIDTH  = 140;

static const QString DONATE         = "https://sigil-ebook.com/donate";
static const QString SIGIL_WEBSITE  = "https://sigil-ebook.com/sigil";
static const QString USER_GUIDE_URL = "https://sigil-ebook.com//sigil/guide";

static const QString BOOK_BROWSER_NAME            = "bookbrowser";
static const QString FIND_REPLACE_NAME            = "findreplace";
static const QString TAB_MANAGER_NAME             = "tabmgr";
static const QString VALIDATION_RESULTS_VIEW_NAME = "validationresultsname";
static const QString TABLE_OF_CONTENTS_NAME       = "tableofcontents";
static const QString PREVIEW_WINDOW_NAME          = "previewwindow";
static const QString CLIPS_WINDOW_NAME            = "clipswindow";
static const QString FRAME_NAME                   = "managerframe";
static const QString TAB_STYLE_SHEET              = "#managerframe {border-top: 0px solid white;"
        "border-left: 1px solid grey;"
        "border-right: 1px solid grey;"
        "border-bottom: 1px solid grey;} ";
static const QString HTML_TOC_FILE = "TOC.xhtml";
static const QString HTML_INDEX_FILE = "Index.xhtml";
const QString HTML_COVER_FILENAME = "cover.xhtml";
const QString CUSTOM_ICON_THEME_FILENAME = "custom_icon_theme.rcc";
    
// External constant (sigil_contants.h) used to consolidate the upper clipboard history limit.
const int CLIPBOARD_HISTORY_MAX = 20;

static const QStringList SUPPORTED_SAVE_TYPE = QStringList() << "epub";

static const QString DEFAULT_FILENAME = "untitled.epub";
static const QString CUSTOM_PREVIEW_STYLE_FILENAME = "custom_preview_style.css";

QStringList MainWindow::s_RecentFiles = QStringList();

// This list needs to be kept in exact sync with AutomateEditor.cpp in Dialogs
static const QStringList AUTOMATE_TOOLS = QStringList() <<
    "AddCover" <<
    "CreateHTMLTOC" <<
    "DeleteUnusedMedia" <<
    "DeleteUnusedStyles" <<
    "GenerateNCXGuideFromNav" <<
    "GenerateTOC" <<
    "MendPrettifyHTML" <<
    "MendHTML" <<
    "ReformatCSSMultipleLines" <<
    "ReformatCSSSingleLines" <<
    "RemoveNCXGuideFromEpub3" <<
    "RepoCommit" <<
    "RunSavedSearchReplaceAll" <<
    "Save" <<
    "SetBookBrowserToAllCSS" <<
    "SetBookBrowserToAllHTML" <<
    "SetBookBrowserToAllImages" <<
    "SetBookBrowserToInitialSelection" <<
    "SplitOnSGFSectionMarkers" <<
    "StandardizeEpub" <<
    "UpdateManifestProperties" <<
    "ValidateStylesheetsWithW3C" <<
    "WellFormedCheckEpub";

MainWindow::MainWindow(const QString &openfilepath, 
                       const QString version,
                       bool is_internal,
                       QWidget *parent,
                       Qt::WindowFlags flags)
    :
    QMainWindow(parent, flags),
    m_LastOpenFileWarnings(QStringList()),
    m_IsInitialLoad(true),
    m_CurrentFilePath(QString()),
    m_CurrentFileName(QString()),
    m_Book(new Book()),
    m_LastFolderOpen(QString()),
    m_SaveACopyFilename(QString()),
    m_LastInsertedFile(QString()),
    m_TabManager(new TabManager(this)),
    m_BookBrowser(NULL),
    m_Clips(NULL),
    m_FindReplace(new FindReplace(this)),
    m_TableOfContents(NULL),
    m_ValidationResultsView(NULL),
    m_PreviewWindow(NULL),
    m_slZoomSlider(NULL),
    m_lbZoomLabel(NULL),
    c_SaveFilters(GetSaveFiltersMap()),
    c_LoadFilters(GetLoadFiltersMap()),
    m_casingChangeGroup(new QActionGroup(this)),
    m_SearchEditor(new SearchEditor(this)),
    m_ClipEditor(new ClipEditor(this)),
    m_IndexEditor(new IndexEditor(this)),
    m_SpellcheckEditor(new SpellcheckEditor(this)),
    m_SelectCharacter(new SelectCharacter(this)),
    m_ViewImage(new ViewImage(this)),
    m_Reports(new Reports(this)),
    m_preserveHeadingAttributes(true),
    m_LinkOrStyleBookmark(new LocationBookmark()),
    m_ClipboardHistorySelector(new ClipboardHistorySelector(this)),
    m_ClipboardHistoryLimit(CLIPBOARD_HISTORY_MAX),
    m_LastPasteTarget(NULL),
    m_ZoomPreview(false),
    m_LastWindowSize(QByteArray()),
    m_LastState(QByteArray()),
    m_FirstTime(true),
    m_PendingLastSizeUpdate(false),
    m_SaveLastEnabled(false),
    m_PreviousHTMLResource(NULL),
    m_PreviousHTMLText(QString()),
    m_PreviousHTMLLocation(QList<ElementIndex>()),
    m_menuPluginsInput(NULL),
    m_menuPluginsOutput(NULL),
    m_menuPluginsEdit(NULL),
    m_menuPluginsValidation(NULL),
    m_pluginList(QStringList()),
    m_SaveCSS(false),
    m_IsClosing(false),
    m_headingActionGroup(new QActionGroup(this)),
    m_UsingAutomate(false)
{
    createJumpList();
    ui.setupUi(this);
    // Telling Qt to delete this window
    // from memory when it is closed
    setAttribute(Qt::WA_DeleteOnClose);
    ExtendUI();
    PlatformSpecificTweaks();
    // Needs to come before signals connect and after ExtendUI
    // (avoiding side-effects)
    ReadSettings();
    // Ensure the UI is properly set to the saved view state.
    SetupPreviewTimer();
    ConnectSignalsToSlots();
    CreateRecentFilesActions();
    UpdateRecentFileActions();
    ChangeSignalsWhenTabChanges(NULL, m_TabManager->GetCurrentContentTab());
    LoadInitialFile(openfilepath, version, is_internal);
    loadPluginsMenu();
}

MainWindow::~MainWindow()
{
    // Make sure that any modeless windows that are visible are closed first
    // to prevent crashes on Windows.
    if (m_SelectCharacter && m_SelectCharacter->isVisible()) {
        m_SelectCharacter->close();
        m_SelectCharacter = NULL;
    }
    if (m_ViewImage && m_ViewImage->isVisible()) {
        m_ViewImage->close();
        m_ViewImage = NULL;
    }

#ifdef Q_OS_MAC  // speeds cleaningup of old modal dialogs
    if (m_ClipboardHistorySelector) delete m_ClipboardHistorySelector;
    if (m_LinkOrStyleBookmark) delete m_LinkOrStyleBookmark;
    if (m_Reports) delete m_Reports;
    if (m_ViewImage) delete m_ViewImage;
    if (m_SelectCharacter) delete m_SelectCharacter;
    if (m_SpellcheckEditor) delete m_SpellcheckEditor;
    if (m_IndexEditor) delete m_IndexEditor;
    if (m_ClipEditor) delete m_ClipEditor;
    if (m_SearchEditor) delete m_SearchEditor;
    if (m_casingChangeGroup) delete m_casingChangeGroup;
    if (m_headingActionGroup) delete m_headingActionGroup;
    if (m_lbZoomLabel) delete m_lbZoomLabel;
    if (m_slZoomSlider) delete m_slZoomSlider;
    if (m_ValidationResultsView) delete m_ValidationResultsView;
    if (m_TableOfContents) delete m_TableOfContents;
    if (m_FindReplace) delete m_FindReplace;
    if (m_Clips) delete m_Clips;
    if (m_BookBrowser) delete m_BookBrowser;
    if (m_TabManager) delete m_TabManager;
    if (m_PreviewWindow) delete m_PreviewWindow;
#endif

}


void MainWindow::createJumpList()
{
#ifdef Q_OS_WIN32
    QWinJumpList jumplist;
    jumplist.recent()->setVisible(true);
#endif
}

void MainWindow::RunAutomate1()
{
    QString automatefile = Utility::DefinePrefsDir() + "/automate01.txt";
    RunAutomate(automatefile);
}

void MainWindow::EditAutomate1()
{
    QString automatefile = Utility::DefinePrefsDir() + "/automate01.txt";
    EditAutomate(automatefile);
}

void MainWindow::RunAutomate2()
{
    QString automatefile = Utility::DefinePrefsDir() + "/automate02.txt";
    RunAutomate(automatefile);
}

void MainWindow::EditAutomate2()
{
    QString automatefile = Utility::DefinePrefsDir() + "/automate02.txt";
    EditAutomate(automatefile);
}

void MainWindow::RunAutomate3()
{
    QString automatefile = Utility::DefinePrefsDir() + "/automate03.txt";
    RunAutomate(automatefile);
}

void MainWindow::EditAutomate3()
{
    QString automatefile = Utility::DefinePrefsDir() + "/automate03.txt";
    EditAutomate(automatefile);
}

void MainWindow::EditAutomate(const QString &automatefile)
{
    AutomateEditor aedit(automatefile, this);
    if (aedit.exec() != QDialog::Accepted) {
        ShowMessageOnStatusBar(tr("Automate List Editor cancelled."));
        return;
    }
    ShowMessageOnStatusBar(tr("Automate List edited."));
}

void MainWindow::RunAutomate(const QString &automatefile)
{
    if (m_UsingAutomate) {
        ShowMessageOnStatusBar(tr("Error: Automation Already in Use"));
        return;
    }
    m_AutomateLog.clear();
    m_UsingAutomate = true;
    if (!QFile::exists(automatefile)) {
        ShowMessageOnStatusBar(tr("Missing Automation List") + ": " + automatefile );
        m_UsingAutomate = false;
        return;
    }
    QString data = Utility::ReadUnicodeTextFile(automatefile);
    QStringList datalines = data.split('\n');
    QStringList commands;
    foreach(QString aline, datalines) {
        QString cmd = aline.trimmed();
        if (!cmd.isEmpty()) commands << cmd;
    }
    if (!commands.isEmpty()) Automate(commands);
    m_UsingAutomate = false;
    m_AutomateLog.clear();
}


bool MainWindow::Automate(const QStringList &commands)
{
    // store away the set of resources selected in BookBrowser at the start
    QList<Resource*> selected_resources = m_BookBrowser->AllSelectedResources();
    PluginDB *pdb = PluginDB::instance();
    QHash<QString, Plugin *> plugins = pdb->all_plugins();
    QStringList plugin_names = plugins.keys();
    bool has_error = false;
    
    foreach(QString cmd , commands) {
        bool success = false;
        QString plugin_type;
        int validation_error_count = 0;

        // allow lines to be commented out by starting with #
        if (cmd.startsWith('#')) continue;
        ShowMessageOnStatusBar(cmd + " " + tr("running"));

        if (plugin_names.contains(cmd)) {
            PluginRunner prunner(m_TabManager, this);
            prunner.exec(cmd);

            qApp->processEvents();

            success = prunner.getResult() == "success";
            plugin_type = prunner.getPluginType();
        } else if (AUTOMATE_TOOLS.contains(cmd)) {
            if (cmd == "Save")                            success = Save();
            else if (cmd == "WellFormedCheckEpub")        success = WellFormedCheckEpub();
            else if (cmd == "MendPrettifyHTML")           success = MendPrettifyHTML();
            else if (cmd == "MendHTML")                   success = MendHTML();
            else if (cmd == "ValidateStylesheetsWithW3C") success = ValidateStylesheetsWithW3C();
            else if (cmd == "RepoCommit")                 success = RepoCommit();
            else if (cmd == "DeleteUnusedMedia")          success = DeleteUnusedMedia(true);
            else if (cmd == "DeleteUnusedStyles")         success = DeleteUnusedStyles(true);
            else if (cmd == "StandardizeEpub")            success = StandardizeEpub();
            else if (cmd == "SplitOnSGFSectionMarkers")   success = SplitOnSGFSectionMarkers();
            else if (cmd == "AddCover")                   success = AddCover();
            else if (cmd == "GenerateTOC")                success = GenerateTOC(true);
            else if (cmd == "CreateHTMLTOC")              success = CreateHTMLTOC();
            else if (cmd == "ReformatCSSMultipleLines")   success = ReformatAllStylesheets(true);
            else if (cmd == "ReformatCSSSingleLines")     success = ReformatAllStylesheets(false);
            // allow some control over what is selected in BookBrowser because some plugins
            // use that to control which files they work on
            else if (cmd == "SetBookBrowserToAllCSS")   {
                SelectResources(GetAllCSSResources());
                success = true;
            } else if (cmd == "SetBookBrowserToAllHTML")   {
                SelectResources(GetAllHTMLResources());
                success = true;
            } else if (cmd == "SetBookBrowserToAllImages") {
                SelectResources(m_BookBrowser->AllImageResources());
                success = true;
            } else if (cmd == "SetBookBrowserToInitialSelection") {
                // net initial selection against all existing resources in case some have been
                // deleted in earlier steps
                QList<Resource*> resources_to_select;
                QList<Resource*> all_resources = m_Book->GetFolderKeeper()->GetResourceList();
                foreach(Resource * resource, selected_resources) {
                    if (all_resources.contains(resource)) resources_to_select << resource;
                }
                SelectResources(resources_to_select);
                success = true;
            } else {
                // these tools are epub3 specific
                QString version = m_Book->GetOPF()->GetEpubVersion();
                if (version.startsWith('3')) {
                    if (cmd == "GenerateNCXGuideFromNav")     success = GenerateNCXGuideFromNav();
                    if (cmd == "RemoveNCXGuideFromEpub3")     success = RemoveNCXGuideFromEpub3();
                    if (cmd == "UpdateManifestProperties")    success = UpdateManifestProperties();
                } else {
                    ShowMessageOnStatusBar(cmd + " " + tr("skipped since not an epub3"));
                    success = true;
                }
            }
        // handle saved search and its full name parameter     
        } else if (cmd.startsWith("RunSavedSearchReplaceAll")) {
            QString fullname = cmd.mid(25, -1).trimmed();
            QList<SearchEditorModel::searchEntry*> search_entries = m_SearchEditor->GetEntriesFromFullName(fullname);
            if (!search_entries.isEmpty()) {
                // m_FindReplace handles deleting each searchEntry that was created with new
                // Temporarily reroute FindReplace Messages to the Status Bar so they are logged
                connect(m_FindReplace, SIGNAL(ShowMessageRequest(const QString &)),
                        this, SLOT(ShowMessageOnStatusBar(const QString &)));
                m_FindReplace->ReplaceAllSearch(search_entries);
                disconnect(m_FindReplace, SIGNAL(ShowMessageRequest(const QString &)),
                           this, SLOT(ShowMessageOnStatusBar(const QString &)));
                success = true;
            } else {
                ShowMessageOnStatusBar(tr("Missing or unknown Saved Search name") + ": " + cmd);
                success = false;
            }
        } else {
            ShowMessageOnStatusBar(tr("Missing or unknown plugin or tool") + ": " + cmd);
            has_error = true;
            break;
        }
        if (!success) {
            ShowMessageOnStatusBar(cmd + " " + tr("failed"));
            has_error = true;
            break;
        }
        if ((plugin_type == "validation") || (cmd == "WellFormedCheckEpub")) {
            validation_error_count = m_ValidationResultsView->ResultCount();
            if (validation_error_count > 0) {        
                // Try to pause to see if can ignore or not in a non-modal way
                ShowMessageOnStatusBar(tr("Validation tool") + ": " + cmd + " "
                                       + tr("found errors") + " " + QString::number(validation_error_count));
                QMessageBox msgBox;
                msgBox.setModal(false);
                msgBox.setText(tr("Validation tool found errors - Abort or Ignore?"));
                QPushButton * abortButton = msgBox.addButton(QMessageBox::Abort);
                QPushButton * ignoreButton = msgBox.addButton(QMessageBox::Ignore);
                bool button_clicked = false;
                connect(&msgBox, &QMessageBox::buttonClicked, this, [this, &button_clicked]() { button_clicked = true; });
                msgBox.show();
                while(!button_clicked) { qApp->processEvents(); }
                if (msgBox.clickedButton() != ignoreButton) {
                    has_error = true;
                    ShowMessageOnStatusBar(tr("Aborted due to Validation Errors"));
                    break;
                } else {
                    ShowMessageOnStatusBar(tr("Ignored Validation Errors"));
                }
            } else {
                ShowMessageOnStatusBar(tr("Validation Tool Reported No Problems Found"));
            }
        }
        qApp->processEvents();
        m_AutomateLog << "";
    }
    if (has_error) {
        ShowMessageOnStatusBar(tr("Automation List Failed"));
    } else {
        ShowMessageOnStatusBar(tr("Automation List Completed"));
    }
    RepoLog alog(tr("Automate Log"), m_AutomateLog.join('\n'), this);
    alog.exec();
    return has_error == false;
}


// Note on Mac OS X you may only add a QMenu or SubMenu to the MenuBar Once!
// Actions can be removed
void MainWindow::loadPluginsMenu()
{
    PluginDB *pdb = PluginDB::instance();

    m_menuPlugins = ui.menuPlugins;
    m_actionManagePlugins = ui.actionManage_Plugins;
    
    unloadPluginsMenu();

    connect(m_actionManagePlugins, SIGNAL(triggered()), this, SLOT(ManagePluginsDialog()));

    // Setup up for quick launch of plugins
    int i = 0;
    foreach(QAction* pa, m_qlactions){
        // Use the new signal/slot syntax and use a lambda to
        // eliminate the need for the obsoleted QSignalMapper.
        // [captured variables]() {...anonymous processing to do...;}
        connect(pa, &QAction::triggered, this, [this,i]() {
            MainWindow::QuickLaunchPlugin(i);
        });
        i++;
    }

    QHash<QString, Plugin *> plugins = pdb->all_plugins();

    // first set default icons for quick launch plugin buttons
    // Do we need this?  Aren't these set in Form_Files/main.ui
    i = 1;
    foreach(QAction* pa, m_qlactions) {
        QString resource = ":/main/plugin_" + QString::number(i) + "pips.svg";
        pa->setIcon(QIcon(resource));
        i++;
    }

    // now set any custom icons
    SettingsStore ss;
    QStringList namemap = ss.pluginMap();
    int pos = 0;
    foreach (QString name, namemap) {
        if (!name.isEmpty()) {
            Plugin * p = plugins.value(name);
            if (p != NULL) {
                QString iconpath = p->get_iconpath();
                if (!iconpath.isEmpty()) {
                    m_qlactions.at(pos)->setIcon(QIcon(iconpath));
                }
            }
        }
        pos++;
    }

    updateToolTipsOnPluginIcons();

    QStringList keys = plugins.keys();
    keys.sort(Qt::CaseInsensitive);
    m_pluginList = keys;

    foreach(QString key, keys) {
        Plugin *p = plugins.value(key);
        if (p == NULL) {
            continue;
        }
        QString pname = p->get_name();
        QString ptype = p->get_type();

        if (ptype == "input") {
            if (m_menuPluginsInput == NULL) {
                m_menuPluginsInput  = m_menuPlugins->addMenu(tr("Input"));
                connect(m_menuPluginsInput,  SIGNAL(triggered(QAction *)), this, SLOT(runPlugin(QAction *)));
            }
            m_menuPluginsInput->addAction(pname);
        } else if (ptype == "output") {
            if (m_menuPluginsOutput == NULL) {
                m_menuPluginsOutput = m_menuPlugins->addMenu(tr("Output"));
                connect(m_menuPluginsOutput, SIGNAL(triggered(QAction *)), this, SLOT(runPlugin(QAction *)));
            }
            m_menuPluginsOutput->addAction(pname);
        } else if (ptype == "edit") {
            if (m_menuPluginsEdit == NULL) {
                m_menuPluginsEdit = m_menuPlugins->addMenu(tr("Edit"));
                connect(m_menuPluginsEdit,   SIGNAL(triggered(QAction *)), this, SLOT(runPlugin(QAction *)));
            }
            m_menuPluginsEdit->addAction(pname);
        } else if (ptype == "validation") {
            if (m_menuPluginsValidation == NULL) {
                m_menuPluginsValidation = m_menuPlugins->addMenu(tr("Validation"));
                connect(m_menuPluginsValidation,   SIGNAL(triggered(QAction *)), this, SLOT(runPlugin(QAction *)));
            }
            m_menuPluginsValidation->addAction(pname);
        }
    }
}

// Keeps the Plugins menu and ManagePlugins submenu arround even if no plugins added yet
void MainWindow::unloadPluginsMenu()
{
    if (m_menuPlugins != NULL) {
        if (m_menuPluginsInput != NULL) {
            disconnect(m_menuPluginsInput, SIGNAL(triggered(QAction *)), this, SLOT(runPlugin(QAction *)));
            m_menuPluginsInput->clear();
            m_menuPlugins->removeAction(m_menuPluginsInput->menuAction());
            m_menuPluginsInput = NULL;
        }
        if (m_menuPluginsOutput != NULL) {
            disconnect(m_menuPluginsOutput, SIGNAL(triggered(QAction *)), this, SLOT(runPlugin(QAction *)));
            m_menuPluginsOutput->clear();
            m_menuPlugins->removeAction(m_menuPluginsOutput->menuAction());
            m_menuPluginsOutput = NULL;
        }
        if (m_menuPluginsEdit != NULL) {
            disconnect(m_menuPluginsEdit, SIGNAL(triggered(QAction *)), this, SLOT(runPlugin(QAction *)));
            m_menuPluginsEdit->clear();
            m_menuPlugins->removeAction(m_menuPluginsEdit->menuAction());
            m_menuPluginsEdit = NULL;
        }
        if (m_menuPluginsValidation != NULL) {
            disconnect(m_menuPluginsValidation, SIGNAL(triggered(QAction *)), this, SLOT(runPlugin(QAction *)));
            m_menuPluginsValidation->clear();
            m_menuPlugins->removeAction(m_menuPluginsValidation->menuAction());
            m_menuPluginsValidation = NULL;
        }
    }
    disconnect(m_actionManagePlugins, SIGNAL(triggered()), this, SLOT(ManagePluginsDialog()));
    foreach(QAction * pa, m_qlactions) {
        disconnect(pa, &QAction::triggered, this, nullptr);
    }
}

bool MainWindow::StandardizeEpub()
{
    SaveTabData();

    // ask to be sure
    QMessageBox::StandardButton button_pressed;
    button_pressed = QMessageBox::warning(this, tr("Sigil"), 
                                      tr("Are you sure you want to restructure this epub?\nThis action cannot be reversed."), 
                                      QMessageBox::Ok | QMessageBox::Cancel);
    if (button_pressed != QMessageBox::Ok) {
      return false;
    }
    
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // perform well-formed check on all the html resources
    QList<HTMLResource *> htmlresources = m_Book->GetHTMLResources();
    foreach (HTMLResource * hresource, htmlresources) {
        if (!hresource->FileIsWellFormed()) {
            QMessageBox::warning(this, tr("Sigil"), 
                                 tr("Restructure cancelled: %1, XML not well formed.").arg(hresource->ShortPathName()));
            QApplication::restoreOverrideCursor();
            return false;
        }
    }
    // make sure opf is in good shape as well
    OPFResource* opfresource = m_Book->GetOPF();
    if (!opfresource->FileIsWellFormed()) {
        QMessageBox::warning(this, tr("Sigil"),
                             tr("Restructure cancelled: %1, XML not well formed.").arg(opfresource->ShortPathName()));
        QApplication::restoreOverrideCursor();
        return false;
    }
    // ditto for ncx if one exists
    NCXResource* ncxresource = m_Book->GetNCX();
    if (ncxresource && !ncxresource->FileIsWellFormed()) {
        QMessageBox::warning(this, tr("Sigil"),
                             tr("Restructure cancelled: %1, XML not well formed.").arg(ncxresource->ShortPathName()));
        QApplication::restoreOverrideCursor();
        return false;
    }
    // we really should parse validate each css file here but
    // since we update css files using regular expressions, the full 
    // parseability is really not critical

    // first standardize the opf and ncx names
    QList<Resource*> resources;
    QStringList newfilenames;
    QString opfname = m_Book->GetOPF()->Filename();
    if (opfname != "content.opf") {
        resources << m_Book->GetOPF();
        newfilenames << "content.opf";
    }
    NCXResource * ncx = m_Book->GetNCX();
    if (ncx && (ncx->Filename() != "toc.ncx")) {
        resources << m_Book->GetNCX();
        newfilenames << "toc.ncx";
    }
    if (!newfilenames.isEmpty()) {
        m_BookBrowser->RenameResourceList(resources, newfilenames);
    }
    // handle any other name conflicts
    FixDuplicateFilenames();

    // handle case insensitive filesystems and clashing directory names
    bool fs_case_sensitive = false;
    QString mainfolder = m_Book->GetFolderKeeper()->GetFullPathToMainFolder();
    QString apath = mainfolder + "/oebps/images";
    QString bpath = mainfolder + "/OEBPS/Images";
    if (QFileInfo(apath) != QFileInfo(bpath)) {
        fs_case_sensitive = true;
    }
    DBG qDebug() << "file system is case sensitive: " << fs_case_sensitive;

    if (!fs_case_sensitive) {
        // opf is first to handle OEBPS before fighting with its subdirectories
        QStringList groups = QStringList() << "opf" << "Text" << "Styles" << "Images" 
                                           << "Fonts" << "Audio" << "Video" << "Misc";
        // try renaming all matching existing directories to what we want
        QDir mf(mainfolder);
        foreach(QString group, groups) {
            QString folderpath = m_Book->GetFolderKeeper()->GetStdFolderForGroup(group);
            apath = mainfolder + "/" + folderpath.toLower();
            bpath = mainfolder + "/" + folderpath;
            if (QFileInfo(apath).exists() && QFileInfo(apath).isDir()) {
                bool result = mf.rename(folderpath.toLower(), folderpath);
                DBG qDebug() << "rename directory: " << folderpath << result;
            }
        }
    }

    // finally move all content to their standard folders if need be
    MoveContentFilesToStdFolders();

    // update what is needed
    m_Book->GetFolderKeeper()->updateShortPathNames();
    QList<Resource*> allresources = m_Book->GetFolderKeeper()->GetResourceList();
    QStringList bookpaths;
    QStringList mtypes;
    foreach(Resource* res, allresources) {
        bookpaths << res->GetRelativePath();
        mtypes << res->GetMediaType();
    }
    m_Book->GetFolderKeeper()->SetGroupFolders(bookpaths, mtypes);
    m_BookBrowser->Refresh();
    m_Book->SetModified();
    QApplication::restoreOverrideCursor();
    ShowMessageOnStatusBar(tr("Restructure completed."));
    return true;
}

void MainWindow::FixDuplicateFilenames()
{
  QStringList bookpaths = m_Book->GetFolderKeeper()->GetAllBookPaths();
    QStringList problem_bookpaths;
    QSet<QString>UsedSet;
  
    foreach(QString bkpath, bookpaths) {
        QString aname =bkpath.split('/').last().toLower();
        if (UsedSet.contains(aname)) {
            problem_bookpaths << bkpath;
        }
        UsedSet.insert(aname);
    }
    foreach(QString bkpath, problem_bookpaths) {
        QString aname = bkpath.split('/').last().toLower();
        QString newname = m_Book->GetFolderKeeper()->GetUniqueFilenameVersion(aname);
        Resource * resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(bkpath);
        QList<Resource*> resources = QList<Resource*>() << resource;
        QStringList newfilenames = QStringList() << newname;
        m_BookBrowser->RenameResourceList(resources, newfilenames);
    }
    return;
}


void MainWindow::MoveContentFilesToStdFolders()
{
    QList<Resource*> resources = m_Book->GetFolderKeeper()->GetResourceList();
    QList<Resource*> resources_to_move;
    QStringList newbookpaths;
    QString MainFolder = m_Book->GetFolderKeeper()->GetFullPathToMainFolder();
    QDir epub_root(MainFolder);
    foreach(Resource* resource, resources) {
        QString group = MediaTypes::instance()->GetGroupFromMediaType(resource->GetMediaType(), "other");
        if ((group == "other") || group.isEmpty()) continue;
        QString stdfolder = m_Book->GetFolderKeeper()->GetStdFolderForGroup(group);
        QString filename = resource->Filename();
        QString newbookpath = filename;
        if (!stdfolder.isEmpty()) {
            newbookpath = stdfolder + "/" + filename;
        }
        if (newbookpath != resource->GetRelativePath()) {
            // remember to create the destination directory if needed
            if (!stdfolder.isEmpty()) {
                epub_root.mkpath(stdfolder);
            }
            resources_to_move << resource;
            newbookpaths << newbookpath;
        }
    }
    if (!newbookpaths.isEmpty()) {
        m_BookBrowser->MoveResourceList(resources_to_move, newbookpaths); 
    }
}

bool MainWindow::RepoCommit()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // make sure that the Sigil-Preferences directory has a "repo" folder
    QString localRepo = Utility::DefinePrefsDir() + "/repo";
    QDir repoDir(localRepo);
    if (!repoDir.exists()) {
        repoDir.mkpath(localRepo);
    }

    // ensure epub opf has valid bookid and retrieve it
    QString bookid = m_Book->GetOPF()->GetUUIDIdentifierValue();

    // collect additional book info (file name, title, datetime)
    QStringList bookinfo;
    bookinfo << QFileInfo(m_CurrentFileName).completeBaseName();
    bookinfo << m_Book->GetOPF()->GetPrimaryBookTitle();

    // follow epub3 spec and update modification date/time for every save and commit
    // manually set the book to be modified since modification date setting is normally
    // only done upon save or save-as so no need to set the modified flag
    bookinfo <<  m_Book->GetOPF()->AddModificationDateMeta();
    m_Book->SetModified();

    // finally force all changes to Disk
    SaveTabData();
    m_Book->GetFolderKeeper()->SuspendWatchingResources();
    m_Book->SaveAllResourcesToDisk();
    m_Book->GetFolderKeeper()->ResumeWatchingResources();

    // get epub root
    QString bookroot = m_Book->GetFolderKeeper()->GetFullPathToMainFolder();

    // get a full list of epub resource bookpaths
    QStringList bookfiles = m_Book->GetFolderKeeper()->GetAllBookPaths();

    // add in the META-INF/container.xml file
    bookfiles << "META-INF/container.xml";

    // now perform the commit using python in a separate thread since this
    // may take a while depending on the speed of the filesystem
    PythonRoutines pr;
    QFuture<QString> future = QtConcurrent::run(&pr, &PythonRoutines::PerformRepoCommitInPython, 
                                                localRepo, bookid, bookinfo, bookroot, bookfiles);
    future.waitForFinished();
    QString commit_result = future.result();

    if (commit_result.isEmpty()) {
        ShowMessageOnStatusBar(tr("Checkpoint generation failed."));
        QApplication::restoreOverrideCursor();
        return false;
    }

    QApplication::restoreOverrideCursor();
    ShowMessageOnStatusBar(tr("Checkpoint saved."));
    return true;
}

// handle both the current epub and the general case
void MainWindow::RepoCheckout(QString bookid, QString destdir, QString filename, bool loadnow)
{
    QString localRepo = Utility::DefinePrefsDir() + "/repo";

    if (destdir.isEmpty()) {
        destdir = Utility::DefinePrefsDir() + "/checkouts";
    }
    QDir coDir(destdir);
    if (!coDir.exists()) {
        coDir.mkpath(destdir);
    }

    if (bookid.isEmpty()) {
        // use current epub's bookid and create one if needed
        bookid = m_Book->GetOPF()->GetUUIDIdentifierValue();
    }

    if (filename.isEmpty()) {
        // use current epub's filename
        filename = QFileInfo(m_CurrentFileName).completeBaseName();
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // now perform the operation using python in a separate thread since this
    // may take a while depending on the speed of the filesystem
    PythonRoutines pr;
    QFuture<QStringList> future = QtConcurrent::run(&pr, &PythonRoutines::GetRepoTagsInPython, 
                                                         localRepo, bookid);
    future.waitForFinished();
    QStringList tag_results = future.result();
    if (tag_results.isEmpty()) {
        ShowMessageOnStatusBar(tr("Checkout Failed. No checkpoints found"));
        QApplication::restoreOverrideCursor();
        return;
    }

    QApplication::restoreOverrideCursor();

    // Now create a Dialog to allow the user to select a tag (checkpoint)
    QString tagname;
    SelectCheckpoint gettag(tag_results, this);
    if (gettag.exec() == QDialog::Accepted) {
        QStringList taglst  = gettag.GetSelectedEntries();
        if (!taglst.isEmpty()) {
            tagname = taglst.at(0);
        }
    }
    if (tagname.isEmpty()) {
        ShowMessageOnStatusBar(tr("Checkout Failed. No checkpoint selected"));
        return;
    }

    // Save the current state of open tabs, putting the current tab last
    QStringList open_tab_bookpaths;
    QList<int> open_tab_positions;
    Resource * current_resource = m_TabManager->GetCurrentContentTab()->GetLoadedResource();
    foreach(ContentTab* tab, m_TabManager->GetContentTabs()) {
        Resource* res = tab->GetLoadedResource();
        if (res != current_resource) {
            open_tab_bookpaths << res->GetRelativePath();
            open_tab_positions << tab->GetCursorPosition();
        }
    }
    open_tab_bookpaths << current_resource->GetRelativePath();
    open_tab_positions << m_TabManager->GetCurrentContentTab()->GetCursorPosition();
    
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QFuture<QString> afuture = QtConcurrent::run(&pr, &PythonRoutines::GenerateEpubFromTagInPython, 
                                                 localRepo, bookid, tagname, filename, destdir);
    afuture.waitForFinished();
    QString epub_result = afuture.result();
    if (epub_result.isEmpty()) {
        ShowMessageOnStatusBar(tr("Epub Generate from Tag Failed."));
        QApplication::restoreOverrideCursor();
        return;
    }
    QApplication::restoreOverrideCursor();
    ShowMessageOnStatusBar(tr("Epub Generation succeeded"));

    if (loadnow) {
        // on macOS bad things with checkpoints could happen if we have 
        // two different epubs open but both with the exact same book id
        // so treat macOS just like Linux and Windows when restoring from 
        // a checkpoint

        // For Linux and Windows (and macOS in this one case) will replace 
        // current book So Throw Up a Dialog to See if they want to proceed
        bool proceed = false;
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        msgBox.setWindowTitle(tr("Repository Checkout"));
        msgBox.setText(tr("Your current book will be replaced losing any unsaved changes ... Are you sure you want to proceed?"));
        QPushButton *yesButton = msgBox.addButton(QMessageBox::Yes);
        QPushButton *noButton =  msgBox.addButton(QMessageBox::No);
        msgBox.setDefaultButton(noButton);
        msgBox.exec();
        if (msgBox.clickedButton() == yesButton) {
            proceed = true;
        }
        if (proceed) {
            LoadFile(epub_result, true);
            // restore what we can of the open tabs
            for(int i=0; i < open_tab_bookpaths.length(); i++) {
                OpenFile(open_tab_bookpaths.at(i), -1, open_tab_positions.at(i));
            }
        }
    }
}

void MainWindow::RepoDiff(QString bookid)
{
    QString localRepo = Utility::DefinePrefsDir() + "/repo";
    QDir repoDir(localRepo);
    if (!repoDir.exists()) {
        // No repo folder, no checkpoints
        ShowMessageOnStatusBar(tr("Diff Failed. No checkpoints found"));
        return;
    }
    if (bookid.isEmpty()) {
        // use current epub's bookid and create one if needed
        bookid = m_Book->GetOPF()->GetUUIDIdentifierValue();
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // force all changes to Disk
    SaveTabData();
    m_Book->GetFolderKeeper()->SuspendWatchingResources();
    m_Book->SaveAllResourcesToDisk();
    m_Book->GetFolderKeeper()->ResumeWatchingResources();

    // Get tags using python in a separate thread since this
    // may take a while depending on the speed of the filesystem
    PythonRoutines pr;
    QFuture<QStringList> future = QtConcurrent::run(&pr, &PythonRoutines::GetRepoTagsInPython, 
                                                          localRepo, bookid);
    future.waitForFinished();
    QStringList tag_results = future.result();
    if (tag_results.isEmpty()) {
        ShowMessageOnStatusBar(tr("Diff Failed. No checkpoints found"));
        QApplication::restoreOverrideCursor();
        return;
    }
    QApplication::restoreOverrideCursor();

    // Now use a Dialog to allow the user to select the base tag
    QString chkpoint1;
    SelectCheckpoint gettagleft(tag_results, this);
    if (gettagleft.exec() == QDialog::Accepted) {
        QStringList taglst  = gettagleft.GetSelectedEntries();
        if (!taglst.isEmpty()) {
            chkpoint1 = taglst.at(0);
        }
    }
    if (chkpoint1.isEmpty()) {
        ShowMessageOnStatusBar(tr("Diff Failed. No checkpoint selected for comparison"));
        return;
    }

    // checkout this tag version and copy it to a tempfolder
    TempFolder destdir;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFuture<QString> cfuture = QtConcurrent::run(&pr, &PythonRoutines::CopyTagToDestDirInPython, 
                                                 localRepo, bookid, chkpoint1, destdir.GetPath() );
    cfuture.waitForFinished();
    QString copied = cfuture.result();
    QApplication::restoreOverrideCursor();

    // get  the status of the changes since that tag
    QString bookroot = m_Book->GetFolderKeeper()->GetFullPathToMainFolder();
    QStringList bookfiles = m_Book->GetFolderKeeper()->GetAllBookPaths();
    bookfiles << "META-INF/container.xml";

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFuture<QList<QStringList> > dfuture = QtConcurrent::run(&pr, 
                                             &PythonRoutines::GetCurrentStatusVsDestDirInPython, 
                                             bookroot, bookfiles, destdir.GetPath());
    dfuture.waitForFinished();
    QList<QStringList> sres = dfuture.result();
    QApplication::restoreOverrideCursor();
    // order is deleted, added, and modified
    QStringList dlist(sres.at(0));
    QStringList alist(sres.at(1));
    QStringList mlist(sres.at(2));

    if (dlist.isEmpty() && alist.isEmpty() && mlist.isEmpty()) {
        QMessageBox::information(this, tr("Results of Comparison"), tr("No differences were found."));
        return;
    } 

    // use CPCompare dialog modally to allow the user to explore the changes
    CPCompare comp(bookroot, destdir.GetPath(), dlist, alist, mlist, this);
    comp.exec();
}

void MainWindow::RepoManage()
{
    ManageRepos mr(this);
    mr.exec();
}

void MainWindow::launchExternalXEditor()
{
    // For simplicity for new users always launch the PageEdit external
    // editor with the opf so that all xhtml files are findable and editable.
    // Otherwise launch other external editors with the open html resource.

    // Launch external xhtml editor for current tab resource
    // ONLY if the current tab resource is a HTMLResource and
    // ONLY if an external editor path is set and still exists

    HTMLResource *html_resource = NULL;
    OPFResource * opf_resource = NULL;
    
    ContentTab *tab = GetCurrentContentTab();
    if (tab) {
        html_resource = qobject_cast<HTMLResource *>(tab->GetLoadedResource());
        opf_resource = qobject_cast<OPFResource *>(tab->GetLoadedResource());
    }

    SettingsStore ss;
    QString XEditorPath = ss.externalXEditorPath();
    if (XEditorPath.isEmpty()) {
        ShowMessageOnStatusBar(tr("No External Xhtml Editor has been specified:  See Preferences"));
        return;
    }

    // make sure XEditor Path actually exists
    QFileInfo xeditorinfo(XEditorPath);
    if (!xeditorinfo.exists()) {
        ShowMessageOnStatusBar(tr("Specified External Xhtml Editor path does not exist"));
        return;
    }

    //bool isPageEdit = ss.externalXEditorPath().contains("pageedit", Qt::CaseInsensitive);
    bool isPageEdit = (xeditorinfo.baseName().toLower() == "pageedit");
    // qDebug() << "External editor is PageEdit: " << isPageEdit;

    if (isPageEdit) {
        if (!html_resource && !opf_resource) {
            ShowMessageOnStatusBar(tr("PageEdit XHtml Editor works only on Html/OPF Resources"));
            return;
        }
    } else { 
        // PageEdit isn't being used, so only an open html resource will work
        if (!html_resource ) {
            ShowMessageOnStatusBar(tr("External XHtml Editor works only on Html Resources"));
            return;
        }
    }

    Resource * resource;

    if (isPageEdit) {
        resource = m_Book->GetOPF();

        // an OPF Resource could be used to access every xhtml file in the spine
        // so save all of these resources to disk and set a fswatcher on them
        QList<Resource *> all_resources = m_Book->GetFolderKeeper()->GetResourceList();
        QList<Resource*> spine_resources = m_Book->GetOPF()->GetSpineOrderResources(all_resources);

        int spinenum = 0;

        // first suspend file watching and then save all of these to disk
        // AND while doing so record where in the spine any current html tab might be
        m_Book->GetFolderKeeper()->SuspendWatchingResources();
        resource->SaveToDisk();
        int i = 0;
        foreach(Resource * spineres, spine_resources) {
            HTMLResource* xhtmlres = qobject_cast<HTMLResource *>(spineres);
            if (xhtmlres) {
                spineres->SaveToDisk();
            }
            if (html_resource && (html_resource == xhtmlres)) spinenum = i;
            i++;
        }
        m_Book->GetFolderKeeper()->ResumeWatchingResources();
        // after re-enabling file watching, add all of these to list of files to be watched
        foreach(Resource * spineres, spine_resources) {
            HTMLResource* xhtmlres = qobject_cast<HTMLResource *>(spineres);
            if (xhtmlres) {
                m_Book->GetFolderKeeper()->WatchResourceFile(spineres);
            }
        }

        if (OpenExternally::openFileWithXEditor(resource->GetFullPath(), XEditorPath, spinenum)) {
            m_Book->GetFolderKeeper()->WatchResourceFile(resource);
            ShowMessageOnStatusBar(tr("Executing PageEdit Xhtml Editor"));
            return;
        }

    } else {
        // Not PageEdit. Load single xhtml resource
        resource = qobject_cast<Resource *>(html_resource);
        m_Book->GetFolderKeeper()->SuspendWatchingResources();
        resource->SaveToDisk();
        m_Book->GetFolderKeeper()->ResumeWatchingResources();

        if (OpenExternally::openFile(resource->GetFullPath(), XEditorPath)) {
	        m_Book->GetFolderKeeper()->WatchResourceFile(resource);
            ShowMessageOnStatusBar(tr("Executing External Xhtml Editor"));
            return;
        }
    }
    ShowMessageOnStatusBar(tr("Failed to Launch External Xhtml Editor"));
}

void MainWindow::runPlugin(QAction *action)
{
    QString pname = action->text();
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    // Workaround for a bug in the KDE Qt5 plugin that injects accelerator shortcuts
    // into QAction->text(). So return the toolTip() text in those cases where it
    // differs from text(). Everybody else uses text().
    // https://bugs.kde.org/show_bug.cgi?format=multiple&id=345023
    QString altname = action->toolTip();
    if (pname != altname && !altname.isEmpty()) {
        pname = altname;
    }
#endif
    {
        PluginRunner prunner(m_TabManager, this);
        prunner.exec(pname);
    }
    qApp->processEvents();
}

void MainWindow::SelectResources(QList<Resource *> resources)
{
    return m_BookBrowser->SelectResources(resources);
}


QList <Resource *> MainWindow::GetValidSelectedHTMLResources()
{
    return m_BookBrowser->ValidSelectedHTMLResources();
}


QList <Resource *> MainWindow::GetValidSelectedCSSResources()
{
    return m_BookBrowser->ValidSelectedCSSResources();
}


QList <Resource*> MainWindow::GetTabbedHTMLResources()
{
    return m_TabManager->GetTabResourcesOfType(Resource::HTMLResourceType);
}


QList <Resource*> MainWindow::GetTabbedCSSResources()
{
    return m_TabManager->GetTabResourcesOfType(Resource::CSSResourceType);
}


QList <Resource *> MainWindow::GetAllHTMLResources()
{
    return m_BookBrowser->AllHTMLResources();
}


QList <Resource *> MainWindow::GetAllCSSResources()
{
    return m_BookBrowser->AllCSSResources();
}


QList <Resource *> MainWindow::GetOPFResource()
{
    QList<Resource *> resources;
    Resource * resource = m_Book->GetOPF(); 
    resources << resource;
    return resources;
}


QList <Resource *> MainWindow::GetNCXResource()
{
    QList<Resource *> resources;
    Resource * resource = m_Book->GetNCX();
    /* need not exist */
    if (resource) resources << resource;
    return resources;
}


QSharedPointer<Book> MainWindow::GetCurrentBook()
{
    return m_Book;
}


BookBrowser *MainWindow::GetBookBrowser()
{
    return m_BookBrowser;
}


ContentTab *MainWindow::GetCurrentContentTab()
{
    return m_TabManager->GetCurrentContentTab();
}

FlowTab *MainWindow::GetCurrentFlowTab()
{
    return qobject_cast<FlowTab *>(GetCurrentContentTab());
}

QString MainWindow::GetCurrentFilePath()
{
    return m_CurrentFilePath;
}

void MainWindow::ResetLinkOrStyleBookmark()
{
    ResetLocationBookmark(m_LinkOrStyleBookmark);
}

void MainWindow::ResetLocationBookmark(MainWindow::LocationBookmark *locationBookmark)
{
    locationBookmark->bookpath.clear();
    ui.actionGoBackFromLinkOrStyle->setEnabled(false);
}

void MainWindow::GoBackFromLinkOrStyle()
{
    GoToBookmark(m_LinkOrStyleBookmark);
    UpdateBrowserSelectionToTab();
}

void MainWindow::GoToBookmark(MainWindow::LocationBookmark *locationBookmark)
{
    if (locationBookmark->bookpath.isEmpty()) {
        return;
    }

    try {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(locationBookmark->bookpath);
        OpenResource(resource, -1, locationBookmark->cv_cursor_position, locationBookmark->bv_caret_location_update);
        ShowMessageOnStatusBar();
    } catch (ResourceDoesNotExist&) {
        // Nothing. Old file must have been deleted.
        ShowMessageOnStatusBar(tr("Navigation cancelled as location no longer exists."));
        ResetLocationBookmark(locationBookmark);
    }
}

void MainWindow::GoToPreviewLocation()
{
    DBG qDebug() << "In MainWindow: GoToPreviewLocation";
    FlowTab *flow_tab = GetCurrentFlowTab();
    if (flow_tab && flow_tab->GetLoadedResource()->Type() == Resource::HTMLResourceType) {
        flow_tab->GoToCaretLocation(m_PreviewWindow->GetCaretLocation());
    }
}

void MainWindow::BookmarkLocation()
{
    BookmarkLinkOrStyleLocation();
    ShowMessageOnStatusBar(tr("Location bookmarked."));
}

void MainWindow::BookmarkLinkOrStyleLocation()
{
    ResetLinkOrStyleBookmark();
    ContentTab *tab = GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    Resource *current_resource = tab->GetLoadedResource();
    m_LinkOrStyleBookmark->bookpath = current_resource->GetRelativePath();
    m_LinkOrStyleBookmark->cv_cursor_position = tab->GetCursorPosition();
    m_LinkOrStyleBookmark->bv_caret_location_update = tab->GetCaretLocationUpdate();
    ui.actionGoBackFromLinkOrStyle->setEnabled(!m_LinkOrStyleBookmark->bookpath.isEmpty());
}

void MainWindow::ScrollCVToFragment(const QString &fragment)
{
    ContentTab *tab = GetCurrentContentTab();
    if (tab != NULL) {
        HTMLResource * html_resource = qobject_cast<HTMLResource *>(tab->GetLoadedResource());
        if (html_resource) {
            FlowTab *flow_tab = qobject_cast<FlowTab *>(tab);
            if (flow_tab) {
                if (fragment.isEmpty()) {
                    flow_tab->ScrollToLine(1);
                } else {
                    flow_tab->ScrollToFragment(fragment);
                }
            }
        }
    }
}

void MainWindow::OpenUrl(const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }

    BookmarkLinkOrStyleLocation();

    // note: TabManager generates book: urls and PreviewWindow generates file: urls
    if (url.scheme() == "book" || url.scheme() == "file" || url.scheme() == "sigil") {
        Resource *resource = m_BookBrowser->GetUrlResource(url);

        if (resource == NULL) {
            ResetLinkOrStyleBookmark();
            return;
        }

        int line = -1;

        if (url.fragment().isEmpty()) {
            ;
            // If empty fragment force view to top of page
            line = 1;
        }

        OpenResource(resource, line, -1, QString(), url.fragment());
    } else {
        QMessageBox::StandardButton button_pressed;
        button_pressed = QMessageBox::warning(this, tr("Sigil"), tr("Are you sure you want to open this external link?\n\n%1").arg(url.toString()), QMessageBox::Ok | QMessageBox::Cancel);

        if (button_pressed == QMessageBox::Ok) {
            QDesktopServices::openUrl(url);
        }
    }
}

void MainWindow::OpenResource(Resource *resource,
                              int line_to_scroll_to,
                              int position_to_scroll_to,
                              const QString &caret_location_to_scroll_to,
                              const QUrl &fragment,
                              bool precede_current_tab)
{
    m_TabManager->OpenResource(resource, line_to_scroll_to, position_to_scroll_to, caret_location_to_scroll_to,
                              fragment, precede_current_tab);
}

void MainWindow::OpenResourceAndWaitUntilLoaded(Resource *resource,
        int line_to_scroll_to,
        int position_to_scroll_to,
        const QString &caret_location_to_scroll_to,
        const QUrl &fragment,
        bool precede_current_tab)
{
    OpenResource(resource, line_to_scroll_to, position_to_scroll_to, caret_location_to_scroll_to, fragment, precede_current_tab);
    while (!GetCurrentContentTab()->IsLoadingFinished()) {
        qApp->processEvents();
        SleepFunctions::msleep(100);
    }
}

void MainWindow::ResourceUpdatedFromDisk(Resource *resource)
{
    SettingsStore ss;
    QString message = QString(tr("File")) + " " + resource->ShortPathName() + " " + tr("was updated") + ".";
    int duration = 10000;

    if (resource->Type() == Resource::HTMLResourceType) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (!m_Book->IsDataOnDiskWellFormed(html_resource)) {
            OpenResource(resource, -1, -1, QString());
            message = QString(tr("Warning")) + ": " + message + " " + tr("The file was NOT well formed and may be corrupted.");
            duration = 20000;
        }
    }

    ShowMessageOnStatusBar(message, duration);
}

void MainWindow::ShowMessageOnStatusBar(const QString &message,
                                        int millisecond_duration)
{
    // It is only safe to add messages to the status bar on the GUI thread.
    Q_ASSERT(QThread::currentThread() == QCoreApplication::instance()->thread());
    // The MainWindow has to have a status bar initialised
    Q_ASSERT(statusBar());

    if (message.isEmpty()) {
        statusBar()->clearMessage();
    } else {
        if (m_UsingAutomate) m_AutomateLog << message;
        statusBar()->showMessage(message, millisecond_duration);
    }
}

void MainWindow::ShowLastOpenFileWarnings()
{
    if (!m_LastOpenFileWarnings.isEmpty()) {
        Utility::DisplayStdWarningDialog(
            "<p><b>" %
            tr("Opening this EPUB generated warnings.") %
            "</b></p><p>" %
            tr("Select Show Details for more information.") %
            "</p>",
            m_LastOpenFileWarnings.join("\n"));
        m_LastOpenFileWarnings.clear();
    }
}

void MainWindow::showEvent(QShowEvent *event)
{
    m_IsInitialLoad = false;

    QMainWindow::showEvent(event);
}

void MainWindow::DebugCurrentWidgetSizes() 
{
    DWINGEO {
        qDebug() << "visible: " << isVisible();
        qDebug() << "maximized: " << isMaximized();
        qDebug() << "full screen: " << isFullScreen();

        QRect r = geometry();
        qDebug() << "main window: " << r.x() << r.y() << r.width() << r.height();

        r = centralWidget()->geometry();
        qDebug() << "central widget: " << r.x() << r.y() << r.width() << r.height();

        r = m_TabManager->geometry();
        qDebug() << "tab manager: " << r.x() << r.y() << r.width() << r.height();

        r = m_FindReplace->geometry();
        qDebug() << "find replace: " << r.x() << r.y() << r.width() << r.height();
        qDebug() << "find replace visible: " << m_FindReplace->isVisible();
    }
}

// somehow this routine needs to detect that the mainwindow has
// been maximized or made fullscreen *before* that WindowState
// has been set.
bool MainWindow::isMaxOrFull() {
    bool result = isMaximized() || isFullScreen();
    return result; 
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    DWINGEO qDebug() << "------";
    DWINGEO qDebug() << "In moveEvent with maximized or full" << isMaxOrFull();

    // Workaround for Qt bug - see WriteSettings() for details.
    if (!m_PendingLastSizeUpdate && !isMaxOrFull()) {
        DWINGEO qDebug() << "issuing a LastSizeUpdate request";
        m_PendingLastSizeUpdate = true;
        // delay long enough for WindowState to be properly set if Maximized or FullScreened
        QTimer::singleShot(1000, this, SLOT(UpdateLastSizes()));
    }

    DWINGEO DebugCurrentWidgetSizes();

    QMainWindow::moveEvent(event);
}

// AAARRRRGGGGHHHHHHH!  This is invoked during the resize to fullscreen or maximize 
// *BEFORE* those WindowStates are actually set!!!!!  The Window size has already been 
// maximized or fullscreened but isFullScreen() and isMaximized() still returns FALSE.
// So we can not use it to record last known good sizes of these windows before maximize 
// or full screen is done by the user.
// Furthermore this can be invoked more than once for Maximize while it is adjusted in size
// to fit the available geometry

void MainWindow::resizeEvent(QResizeEvent *event)
{
    DWINGEO qDebug() << "------";
    DWINGEO qDebug() << "in ResizeEvent with maximized or full" << isMaxOrFull();
    DWINGEO qDebug() << "old size: " << event->oldSize();
    DWINGEO qDebug() << "new size: " << event->size();
    DWINGEO qDebug() << "primary screen total size: " << qApp->primaryScreen()->geometry();
    DWINGEO qDebug() << "primary screen available size: " << qApp->primaryScreen()->availableGeometry();

    // Workaround for Qt bug - see WriteSettings() for details.
    if (!m_PendingLastSizeUpdate && !isMaxOrFull()) {
        DWINGEO qDebug() << "issuing a LastSizeUpdate request";
        m_PendingLastSizeUpdate = true;
        // delay long enough for WindowState to be properly set if Maximize or FullScreen
        QTimer::singleShot(1000, this, SLOT(UpdateLastSizes()));
    }

    DWINGEO DebugCurrentWidgetSizes();

    QMainWindow::resizeEvent(event);
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    m_IsClosing = true;


    // stop any further UpdatePreview timer actions
    // and prevent UpdatePage from running
    if (m_PreviewTimer.isActive()) {
        m_PreviewTimer.stop();
    }
    disconnect(&m_PreviewTimer, SIGNAL(timeout()), this, SLOT(UpdatePreview()));

    DBG qDebug() << "in close event before maybe save";

    // this should be done first to save all geometry
    // extra saves should not be an issue if the window close is abandoned
    WriteSettings();

    if (MaybeSaveDialogSaysProceed()) {

        DBG qDebug() << "in close event after maybe save";

#ifdef Q_OS_MAC
        // since we are closing this window, disconnect signals that might be invoked
        // by a user during closing operations to help prevent segfaults on close
        disconnect(ui.actionNew,           SIGNAL(triggered()), this, SLOT(NewDefault()));
        disconnect(ui.actionNewEpub2,      SIGNAL(triggered()), this, SLOT(NewEpub2()));
        disconnect(ui.actionNewEpub3,      SIGNAL(triggered()), this, SLOT(NewEpub3()));
        disconnect(ui.actionOpen,          SIGNAL(triggered()), this, SLOT(Open()));
        disconnect(ui.actionPreferences,   SIGNAL(triggered()), this, SLOT(PreferencesDialog()));
#endif
        ShowMessageOnStatusBar(tr("Sigil is closing..."));

        KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();
        sm->removeActionsOf(this);

        // The user may have unsaved search/clip/index/meta entries if dialogs are open.
        // Prompt them to save or discard their changes if any.
        if (m_SearchEditor && m_SearchEditor->isVisible()) {
            m_SearchEditor->ForceClose();
        }

        if (m_ClipEditor && m_ClipEditor->isVisible()) {
            m_ClipEditor->ForceClose();
        }

        if (m_IndexEditor && m_IndexEditor->isVisible()) {
            m_IndexEditor->ForceClose();
        }

        if (m_SpellcheckEditor && m_SpellcheckEditor->isVisible()) {
            m_SpellcheckEditor->ForceClose();
        }

        if ((m_PreviewWindow)  && m_PreviewWindow->isVisible()) {
            DBG qDebug() << "in close event hiding Preview Window";
            m_PreviewWindow->hide();
        }

#ifdef Q_OS_MAC
        // Qt BUG:  macOS can not be left in fullscreen or maximized mode upon exit
        if (isFullScreen()) setWindowState(windowState() & ~Qt::WindowFullScreen);
        if (isMaximized()) setWindowState(windowState() & ~Qt::WindowMaximized);
#endif
        event->accept();
    } else {
        event->ignore();
        SetupPreviewTimer();
        // re-enable the ability to record good last normal sizes
        m_SaveLastEnabled = true;
        m_IsClosing = false;
    }
}

// quick and dirty way to map various actions for New()
// we could move these to exist only in the header
void MainWindow::NewDefault() { New("");    }
void MainWindow::NewEpub2()   { New("2.0"); }
void MainWindow::NewEpub3()   { New("3.0"); }

void MainWindow::New(const QString version)
{
    // The nasty IFDEFs are here to enable the multi-document
    // interface on the Mac; Lin and Win just use multiple
    // instances of the Sigil application
#ifndef Q_OS_MAC
    if (MaybeSaveDialogSaysProceed())
#endif
    {
#ifdef Q_OS_MAC
        MainWindow *new_window = new MainWindow("", version);
        new_window->show();
        new_window->activateWindow();
#else
        CreateNewBook(version);
#endif
    }

    ShowMessageOnStatusBar(tr("New file created."));
}


void MainWindow::Open()
{
    // The nasty IFDEFs are here to enable the multi-document
    // interface on the Mac; Lin and Win just use multiple
    // instances of the Sigil application
#ifndef Q_OS_MAC
    if (MaybeSaveDialogSaysProceed())
#endif
    {
        QStringList filters(c_LoadFilters.values());
        filters.removeDuplicates();
        QString filter_string = "";
        foreach(QString filter, filters) {
            filter_string += filter + ";;";
        }
        QString default_filter = c_LoadFilters.value("epub");
        QFileDialog::Options options = QFileDialog::Options();
#ifdef Q_OS_MAC
        options = options | QFileDialog::DontUseNativeDialog;
#endif
        QString filename = QFileDialog::getOpenFileName(this,
                           tr("Open File"),
                           m_LastFolderOpen,
                           filter_string,
                           &default_filter,
                           options
                                                       );

        if (!filename.isEmpty()) {
#ifdef Q_OS_MAC
            MainWindow *new_window = new MainWindow(filename);
            new_window->show();
            new_window->activateWindow();
            qApp->processEvents();
#else
            LoadFile(filename);
#endif
        }
    }
}


void MainWindow::OpenRecentFile()
{
    // The nasty IFDEFs are here to enable the multi-document
    // interface on the Mac; Lin and Win just use multiple
    // instances of the Sigil application
    QAction *action = qobject_cast<QAction *>(sender());

    if (action != NULL) {
#ifndef Q_OS_MAC

        if (MaybeSaveDialogSaysProceed())
#endif
        {
            // It is possible that the file in the recent file menu no longer exists.
            // Prompt the user asking them if they want to remove it.
            const QString &filename = action->data().toString();

            if (!QFile::exists(filename)) {
                QMessageBox::StandardButton button_pressed;
                const QString &msg = tr("This file no longer exists. Click OK to remove it from the menu.\n%1");
                button_pressed = QMessageBox::warning(this, tr("Sigil"),
                                                      msg.arg(filename), QMessageBox::Ok | QMessageBox::Cancel);

                if (button_pressed == QMessageBox::Ok) {
                    s_RecentFiles.removeAll(filename);
                    UpdateRecentFileActions();
                }

                return;
            }

#ifdef Q_OS_MAC
            MainWindow *new_window = new MainWindow(filename);
            new_window->show();
            new_window->activateWindow();
            qApp->processEvents();

#else
            LoadFile(filename);
#endif
        }
    }
}


bool MainWindow::Save()
{
    if (m_CurrentFilePath.isEmpty()) {
        return SaveAs();
    } else {
        QString extension = QFileInfo(m_CurrentFilePath).suffix().toLower();

        if (!SUPPORTED_SAVE_TYPE.contains(extension)) {
            return SaveAs();
        }

        return SaveFile(m_CurrentFilePath);
    }
}


bool MainWindow::SaveAs()
{
    QStringList filters(c_SaveFilters.values());
    filters.removeDuplicates();
    QString filter_string = "";
    foreach(QString filter, filters) {
        filter_string += filter + ";;";
    }
    QString save_path       = "";
    QString default_filter  = "";

    if (m_CurrentFilePath.isEmpty()) {
        m_CurrentFilePath = (m_CurrentFileName.isEmpty())?DEFAULT_FILENAME:m_CurrentFileName;
    }

    if (m_LastFolderOpen.isEmpty()) {
        m_LastFolderOpen = QDir::homePath();
    }

    // If we can save this file type, then we use the current filename
    if (c_SaveFilters.contains(QFileInfo(m_CurrentFilePath).suffix().toLower())) {
        save_path       = m_LastFolderOpen + "/" + QFileInfo(m_CurrentFilePath).fileName();
        default_filter  = c_SaveFilters.value(QFileInfo(m_CurrentFilePath).suffix().toLower());
    }
    // If not, we change the extension to EPUB
    else {
        save_path       = m_LastFolderOpen + "/" + QFileInfo(m_CurrentFilePath).completeBaseName() + ".epub";
        default_filter  = c_SaveFilters.value("epub");
    }
    QFileDialog::Options options = QFileDialog::Options();
#if !defined(Q_OS_WIN32)
    options = options | QFileDialog::DontUseNativeDialog;
#endif


    QString filename = QFileDialog::getSaveFileName(this,
                       tr("Save File"),
                       save_path,
                       filter_string,
                       &default_filter,
                       options
                                                   );

    // QFileDialog cancelled
    if (filename.isEmpty()) {
        // The only time m_CurrentFilePath should be cleared after the SaveAs dialog is
        // cancelled, is when it's populated with the "untitled.epub" default filename.
        if (m_CurrentFilePath == DEFAULT_FILENAME) {
            m_CurrentFilePath.clear();
        }
        return false;
    }

    QString extension = QFileInfo(filename).suffix().toLower();
    if (extension.isEmpty()) {
        filename += ".epub";
    }


    // Store the folder the user saved to
    m_LastFolderOpen = QFileInfo(filename).absolutePath();
    bool save_result = SaveFile(filename);

    if (!save_result) {
        m_CurrentFilePath.clear();
    }

    return save_result;
}


bool MainWindow::SaveACopy()
{
    if (m_CurrentFilePath.isEmpty()) {
        m_CurrentFilePath = (m_CurrentFileName.isEmpty())?DEFAULT_FILENAME:m_CurrentFileName;
    }

    if (m_SaveACopyFilename.isEmpty()) {
        m_SaveACopyFilename = m_LastFolderOpen + "/" + QFileInfo(m_CurrentFilePath).completeBaseName() + "_copy." + QFileInfo(m_CurrentFilePath).suffix();
    }

    QStringList filters(c_SaveFilters.values());
    filters.removeDuplicates();
    QString filter_string = "*.epub";
    QString default_filter  = "*.epub";
    QFileDialog::Options options = QFileDialog::Options();
#if !defined(Q_OS_WIN32)
    options = options | QFileDialog::DontUseNativeDialog;
#endif
    QString filename = QFileDialog::getSaveFileName(this,
                       tr("Save a Copy"),
                       m_SaveACopyFilename,
                       filter_string,
                       &default_filter,
                       options
                                                   );

    // QFileDialog cancelled
    if (filename.isEmpty()) {
        // The only time m_CurrentFilePath should be cleared after the SaveAs dialog is
        // cancelled, is when it's populated with the "untitled.epub" default filename.
        if (m_CurrentFilePath == DEFAULT_FILENAME) {
            m_CurrentFilePath.clear();
        }
        return false;
    }

    QString extension = QFileInfo(filename).suffix();

    if (extension.isEmpty()) {
        filename += ".epub";
    }

    // Store the filename the user saved to
    m_SaveACopyFilename = filename;
    return SaveFile(filename, false);
}


void MainWindow::CreateEpubLayout()
{
    SettingsStore ss;
    QString version = ss.defaultVersion();

    EmptyLayout edesign(version, this);
    edesign.exec();

    QStringList bookpaths = edesign.GetBookPaths();
    if (bookpaths.isEmpty()) {
        ShowMessageOnStatusBar(tr("Epub layout discarded."));
        return;
    }
 
    if (MaybeSaveDialogSaysProceed()) {
        CreateNewBook(version, bookpaths);
    }
    ShowMessageOnStatusBar(tr("New epub created."));
}


void MainWindow::Exit()
{
    DBG qDebug() << "In Exit";
    qApp->closeAllWindows();
#ifdef Q_OS_MAC
    MainWindow *mw;
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        mw = qobject_cast<MainWindow *>(w);
        if (mw && !mw->isHidden()) {
            return;
        }
    }
    qApp->quit();
#endif
}

void MainWindow::Find()
{
    SaveTabData();
    m_FindReplace->SetUpFindText();
    m_FindReplace->show();
}

void MainWindow::GoToLine()
{
    ContentTab *tab = GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    int line = QInputDialog::getInt(this, tr("Go To Line"), tr("Line #"), -1, 1);

    if (line >= 1) {
        OpenResource(tab->GetLoadedResource(), line, -1, QString());
    }
}

void MainWindow::ViewImageDialog(const QUrl &url)
{
    if (url.scheme() != "book") return;

    // non-modal dialog
    m_ViewImage->show();
    m_ViewImage->raise();
    m_ViewImage->activateWindow();

    QString image_bookpath = url.path().remove(0,1);
    try {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(image_bookpath);
        if (resource->Type() == Resource::ImageResourceType || resource->Type() == Resource::SVGResourceType) {
            m_ViewImage->ShowImage(resource->GetFullPath());
        }
    } catch (ResourceDoesNotExist&) {
        QMessageBox::warning(this, tr("Sigil"), tr("Image does not exist: ") + image_bookpath);
    }
}

void MainWindow::GoToLinkedStyleDefinition(const QString &element_name, const QString &style_class_name)
{
    // Invoked via a signal when the user has requested to navigate to a
    // style definition and none was found in the inline styles, so look
    // at the linked resources for this tab instead.
    ContentTab *tab = GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    Resource *current_resource = tab->GetLoadedResource();

    if (current_resource->Type() == Resource::HTMLResourceType) {
        BookmarkLinkOrStyleLocation();
        // Look in the linked stylesheets for a match
        QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();
        QStringList stylesheets = GetStylesheetsAlreadyLinked(current_resource);
        bool found_match = false;
        CSSResource *first_css_resource = 0;
        foreach(QString bookpath, stylesheets) {
            Resource * resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(bookpath);
            CSSResource *css_resource = qobject_cast<CSSResource*>( resource );
            if (!first_css_resource) {
                first_css_resource = css_resource;
            }
            if (css_resource) {
                CSSInfo css_info(css_resource->GetText());
                CSSInfo::CSSSelector *selector = css_info.getCSSSelectorForElementClass(element_name, style_class_name);

                // All of this is actually handled in CSSInfo and is NOT needed here

                // If we fail to find a matching element.class, search again with just the class
                // if (!style_class_name.isEmpty() && !selector) {
                //    selector = css_info.getCSSSelectorForElementClass("", style_class_name);
                // }
                //
                // If we fail to find a matching selector search again with just the element
                // if (style_class_name.isEmpty() && !selector) {
                //     selector = css_info.getCSSSelectorForElementClass(element_name, "");
                // }

                if (selector) {
                    m_TabManager->OpenResource(css_resource, -1, selector->pos);
                    found_match = true;
                    break;
                }
            }
        }

        if (!found_match) {
            QString display_name;

            if (style_class_name.isEmpty()) {
                display_name = element_name;
            } else {
                display_name = QString("\".%1\" " + tr("or") + " \"%2.%1\"").arg(style_class_name).arg(element_name);
            }

            // Open the first linked stylesheet if any
            if (first_css_resource) {
                OpenResource(first_css_resource, 1);
            }

            ShowMessageOnStatusBar(QString(tr("No CSS styles named") +  " " + display_name + " " + tr("found, or stylesheet not linked.")), 7000);

        }
    }
}


void MainWindow::ZoomIn()
{
    ZoomByStep(true);
}


void MainWindow::ZoomOut()
{
    ZoomByStep(false);
}


void MainWindow::ZoomReset()
{
    ZoomByFactor(ZOOM_NORMAL);
}


void MainWindow::IndexEditorDialog(IndexEditorModel::indexEntry *index_entry)
{
    SaveTabData();
    // non-modal dialog
    m_IndexEditor->show();
    m_IndexEditor->raise();
    m_IndexEditor->activateWindow();

    if (index_entry) {
        m_IndexEditor->AddEntry(false, index_entry, false);
    }
}

void MainWindow::SpellcheckEditorDialog()
{
    SaveTabData();
    // non-modal dialog
    m_SpellcheckEditor->show();
    m_SpellcheckEditor->raise();
    m_SpellcheckEditor->activateWindow();
}


void MainWindow::clearMemoryCaches()
{
    // the equivalent in QtWebEngine does not really exist
    // the closest thing is page()->profile() which gets you the QWebEngineProfile
    // You can then use on a page by page basis:
  
    //      clearHttpCache()
    //      clearVisitedLinks()

    //      setHttpCacheMaximumSize() in bytes but 0 means "auto"
    //      httpCacheMaximumSize()

    //      setHttpCacheType() (MemoryHtttpCache, DiskHttpCache, NoCache)
    //      httpCacheType()

    //      setCachPath()
    //      cachePath()
}


bool MainWindow::ProceedWithUndefinedUrlFragments()
{
    std::tuple<bool, QString, QString> result = m_Book->HasUndefinedURLFragments();
    if (std::get<0>(result)) {
        QMessageBox::StandardButton button_pressed;
        const QString &msg = tr("<html><p>The href <b>%1</b> found in <b>%2</b> does not exist (and there may be more)."
                                " Splitting or merging under these conditions can result in broken links.</p>"
                                "<p>Do you still wish to continue?</p></html>");

        button_pressed = QMessageBox::warning(this, tr("Sigil"),
                                msg.arg(std::get<1>(result), std::get<2>(result)),
                                QMessageBox::Yes | QMessageBox::No);
        if (button_pressed != QMessageBox::Yes) {
            return false;
        }
        return true;
    }
    return true;
}


bool MainWindow::AddCover()
{
    QString version = m_Book->GetOPF()->GetEpubVersion();
 
    // Get the image to use.
    QStringList selected_files;
    // Get just images, not svg files.
    QList<Resource *> image_resources = m_Book->GetFolderKeeper()->GetResourceListByType(Resource::ImageResourceType);
    QString title = tr("Add Cover");
    // SelectFiles returns the bookpaths of all selected resources
    SelectFiles select_files(title, image_resources, m_LastInsertedFile, this);
    
    if (select_files.exec() == QDialog::Accepted) {
        if (select_files.IsInsertFromDisk()) {
            // m_BookBrowser->AddExisting returns the full file paths
            QStringList added_book_paths = m_BookBrowser->AddExisting(false, true);
            foreach(QString bookpath, added_book_paths) {
                selected_files.append(bookpath);
            }
        } else {
            selected_files = select_files.SelectedImages();
        }
    }
    if (selected_files.count() == 0) {
        return false;
    }
    QString image_bookpath = selected_files.first();

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Find existing cover HTML file if there is one.
    HTMLResource *html_cover_resource = NULL;
    QList<HTMLResource *> html_resources;
    QList<Resource *> resources = GetAllHTMLResources();
    foreach(Resource * resource, resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);

        if (html_resource) {
            html_resources.append(html_resource);

            // Check if this is an existing cover file.
            QString semantic_code;
            if (version.startsWith('3')) {
                NavProcessor navproc(m_Book->GetConstOPF()->GetNavResource());
                semantic_code = navproc.GetLandmarkCodeForResource(html_resource);
            } else {
                semantic_code = m_Book->GetOPF()->GetGuideSemanticCodeForResource(html_resource);
            }
            if (semantic_code == "cover") {
                html_cover_resource = html_resource;
            } else if (resource->Filename().toLower() == HTML_COVER_FILENAME && html_cover_resource == NULL) {
                html_cover_resource = html_resource;
            }
        }
    }

    if (html_cover_resource != NULL) {
        QString msg = tr("An existing Cover file has been found.");
        if (!ProceedToOverwrite(msg, html_cover_resource->ShortPathName())) {
            html_cover_resource = NULL;
        }
    }

    // Populate the HTML cover file with the necessary text.
    // If a template file exists, use its text for the cover source.
    QString text = HTML_COVER_SOURCE;
    if (version.startsWith('3')) text = HTML5_COVER_SOURCE;
    QString cover_path = Utility::DefinePrefsDir() + "/" + HTML_COVER_FILENAME;
    if (QFile::exists(cover_path)) {
        text = Utility::ReadUnicodeTextFile(cover_path);
    }

    // Create an HTMLResource for the cover if it doesn't exist.
    if (html_cover_resource == NULL) {
        html_cover_resource = m_Book->CreateHTMLCoverFile(text);
    } else {
        html_cover_resource->SetText(text);
    }

    try {
        Resource *image_resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(image_bookpath);

        // Set cover semantics
        if (version.startsWith('3')) {
            NavProcessor navproc(m_Book->GetOPF()->GetNavResource());
            navproc.AddLandmarkCode(html_cover_resource, "cover", false);
        } else {
            m_Book->GetOPF()->AddGuideSemanticCode(html_cover_resource, "cover", false);
        }
        ImageResource *image_type_resource = qobject_cast<ImageResource *>(image_resource);
        if (image_type_resource) {
            if (!m_Book->GetOPF()->IsCoverImage(image_type_resource)) {
                m_Book->GetOPF()->SetResourceAsCoverImage(image_type_resource);
            }

            // Add the filename and dimensions of the image to the HTML source.
            QString image_relative_path = image_resource->GetRelativePathFromResource(html_cover_resource);
            image_relative_path = Utility::URLEncodePath(image_relative_path);
            QImage img(image_resource->GetFullPath());
            QString text = html_cover_resource->GetText();
            QString width = QString::number(img.width());
            QString height = QString::number(img.height());
            text.replace("SGC_IMAGE_FILENAME", image_relative_path);
            text.replace("SGC_IMAGE_WIDTH", width);
            text.replace("SGC_IMAGE_HEIGHT", height);
            html_cover_resource->SetText(text);

            // Finally, if epub3 update the html resource manifest properties
            if (version.startsWith('3')) {
                QList<Resource*> resources_to_update;
                resources_to_update.append(html_cover_resource);
                m_Book->GetOPF()->UpdateManifestProperties(resources_to_update);
            }
        } else {
            Utility::DisplayStdErrorDialog(tr("Unexpected error. Only image files can be used for the cover."));
            return false;
        }
    } catch (ResourceDoesNotExist&) {
        return false;
        //
    }

    m_BookBrowser->Refresh();
    m_Book->SetModified();
    MainWindow::clearMemoryCaches();
    OpenResourceAndWaitUntilLoaded(html_cover_resource);
    // Reload the tab to ensure it reflects updated image.
    FlowTab *flow_tab = GetCurrentFlowTab();
    if (flow_tab) {
        flow_tab->LoadTabContent();
        flow_tab->ScrollToTop();
    }

    ShowMessageOnStatusBar(tr("Cover added."));
    QApplication::restoreOverrideCursor();
    return true;
}


bool MainWindow::UpdateManifestProperties()
{
    QString version = m_Book->GetConstOPF()->GetEpubVersion();
    if (!version.startsWith('3')) {
        ShowMessageOnStatusBar(tr("Not Available for epub2."));
        return false;
    }
    SaveTabData();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QList<Resource *> resources = GetAllHTMLResources();
    m_Book->GetOPF()->UpdateManifestProperties(resources);
    m_Book->SetModified();
    ShowMessageOnStatusBar(tr("OPF Manifest Properties Updated."));
    QApplication::restoreOverrideCursor();
    return true;
}


bool MainWindow::RemoveNCXGuideFromEpub3()
{
    QString version = m_Book->GetConstOPF()->GetEpubVersion();
    if (!version.startsWith('3')) {
        ShowMessageOnStatusBar(tr("Not Available for epub2."));
        return false;
    }

    SaveTabData();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    NCXResource * ncx_resource = m_Book->GetNCX();
    if (ncx_resource) {
        m_Book->GetOPF()->RemoveNCXOnSpine();
        m_Book->GetFolderKeeper()->RemoveNCXFromFolder();
        ncx_resource->Delete();
    }

    // clear the guide
    m_Book->GetOPF()->ClearSemanticCodesInGuide();

    m_TableOfContents->Refresh();
    m_BookBrowser->BookContentModified();
    m_BookBrowser->Refresh();
    m_Book->SetModified();

    ShowMessageOnStatusBar(tr("NCX and Guide removed."));
    QApplication::restoreOverrideCursor();
    return true;
}


bool MainWindow::GenerateNCXGuideFromNav()
{
    QString version = m_Book->GetConstOPF()->GetEpubVersion();
    if (!version.startsWith('3')) {
        ShowMessageOnStatusBar(tr("Not Available for epub2."));
        return false;
    }

    SaveTabData();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // find existing nav document if there is one
    HTMLResource * nav_resource = m_Book->GetConstOPF()->GetNavResource();
    QString navbkpath = "";
    QString navdata = "";
    if (nav_resource) {
        navdata = CleanSource::Mend(nav_resource->GetText(),"3.0");
        navbkpath = nav_resource->GetRelativePath();
    }

    if ((!nav_resource) || navdata.isEmpty()) {
        ShowMessageOnStatusBar(tr("NCX and Guide generation failed."));
        QApplication::restoreOverrideCursor();
        return false;
    }

    NCXResource * ncx_resource = m_Book->GetNCX();
    // generate a new empty NCX if one does not exist in this epub3
    if (!ncx_resource) {
        ncx_resource = m_Book->GetFolderKeeper()->AddNCXToFolder(version);
        // We manually created an NCX file because there wasn't one in the manifest.
        // Need to create a new manifest id for it.
        // and take that manifest id and add it to the spine attribute
        QString NCXId = m_Book->GetOPF()->AddNCXItem(ncx_resource->GetFullPath(),"ncx");
        m_Book->GetOPF()->UpdateNCXOnSpine(NCXId);
    }

    QString ncxdir = Utility::startingDir(ncx_resource->GetRelativePath());

    QStringList mvalues = m_Book->GetConstOPF()->GetDCMetadataValues("dc:title");
    QString doctitle = "UNKNOWN";
    if (!mvalues.isEmpty()) {
        doctitle = mvalues.at(0);
    } 
    QString mainid = m_Book->GetConstOPF()->GetMainIdentifierValue();

    // Now build the ncx in python in a separate thread since may be an long job
    PythonRoutines pr;
    QFuture<QString> future = QtConcurrent::run(&pr, &PythonRoutines::GenerateNcxInPython, navdata, 
                                                navbkpath, ncxdir, doctitle, mainid);
    future.waitForFinished();
    QString ncxdata = future.result();

    if (ncxdata.isEmpty()) {
        ShowMessageOnStatusBar(tr("NCX and Guide generation failed."));
        QApplication::restoreOverrideCursor();
        return false;
    }

    ncx_resource->SetText(ncxdata);
    ncx_resource->SaveToDisk();

    // now create the opf guide from the nav
    // start by clearing whatever old info is in the guide now
    m_Book->GetOPF()->ClearSemanticCodesInGuide();

    // collect all of the current nav landmark codes
    NavProcessor navproc(nav_resource);
    QHash<QString, QString> nav_landmark_codes = navproc.GetLandmarkCodeForPaths();

    // Walk through all html resources and if they have a landmark code
    // lookup its equivalent in the guide and set it if it exists
    QList<Resource *> html_resources = GetAllHTMLResources();
    foreach(Resource * resource, html_resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (html_resource) {
            QString respath = resource->GetRelativePath();
            if (nav_landmark_codes.contains(respath)) {
                QString landmark_code = nav_landmark_codes[respath];
                QString guide_code =  Landmarks::instance()->GuideLandMapping(landmark_code);
                if (!guide_code.isEmpty()) {
                    m_Book->GetOPF()->AddGuideSemanticCode(html_resource, guide_code, false);
                }
            }
        }
    }

    m_TableOfContents->Refresh();
    m_BookBrowser->BookContentModified();
    m_BookBrowser->Refresh();
    m_Book->SetModified();

    ShowMessageOnStatusBar(tr("NCX and Guide generated."));
    QApplication::restoreOverrideCursor();
    return true;
}


void MainWindow::CreateIndex()
{
    SaveTabData();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // First handle the css file for the index
    bool found_css = false;
    Resource * styleresource = NULL;

    QList<Resource*> style_resources = m_Book->GetFolderKeeper()->GetResourceTypeAsGenericList<CSSResource>(false);
    foreach(Resource *resource, style_resources) {
        if (resource->Filename() == SGC_INDEX_CSS_FILENAME) {
            styleresource = resource;
            found_css = true;
            break;
        }
    }
    // If CSS file does not exist look for a default file
    // in preferences directory and if none create one.
    if (!found_css) {
        QString css_path = Utility::DefinePrefsDir() + "/" + SGC_INDEX_CSS_FILENAME;
        if (!QFile::exists(css_path)) {
            styleresource = m_BookBrowser->CreateIndexCSSFile();
        } else {
            styleresource = m_Book->GetFolderKeeper()->AddContentFileToFolder(css_path, true, "text/css");
        }
        CSSResource *css_resource = qobject_cast<CSSResource *> (styleresource);
        // Need to make sure InitialLoad is done in newly added css resource object to prevent
        // blank css issues after a save to disk
        if (css_resource) css_resource->InitialLoad();
    }

    // get semantic (guide/landmark) information for all resources
    QHash <QString, QString> semantic_types;
    QString version = m_Book->GetOPF()->GetEpubVersion();
    HTMLResource * nav_resource = m_Book->GetConstOPF()->GetNavResource();
    if (version.startsWith('3')) {
        if (nav_resource) {
            NavProcessor navproc(nav_resource);
            semantic_types = navproc.GetLandmarkCodeForPaths();
        }
    } else {
        semantic_types = m_Book->GetOPF()->GetSemanticCodeForPaths();
    }
    QStringList allow_index;
    allow_index  << "bodymatter" << "chapter" << "conclusion" << "division" << "epilogue" << 
                    "introduction" << "part" << "preamble" << "prologue" << "subchapter" <<  
                    "text" << "volume"; 

    HTMLResource *index_resource = NULL;

    QList<HTMLResource *> html_resources;

    // Turn the list of Resources that are really HTMLResources to a real list
    // of HTMLResources stripping out any front or back matter
    QList<Resource *> resources = GetAllHTMLResources();
    foreach(Resource * resource, resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);

        if (html_resource) {
            QString resource_path = html_resource->GetRelativePath();
            QString semantic_code;
            if (semantic_types.contains(resource_path)) {
                semantic_code = semantic_types[resource_path];
            }
            if (semantic_code.isEmpty() || allow_index.contains(semantic_code)) {
                html_resources.append(html_resource);
            }

            // Check if this is an existing index file.
            if (semantic_code == "index") {
                index_resource = html_resource;
            } else if (resource->Filename() == HTML_INDEX_FILE && html_resource == NULL) {
                index_resource = html_resource;
            }
        }
    }

    if (index_resource != NULL) {
        QString msg = tr("An existing Index file has been found.");
        if (!ProceedToOverwrite(msg, index_resource->ShortPathName())) {
            index_resource = NULL;
        }
    }

    // Close the tab so the focus saving doesn't overwrite the text were
    // replacing in the resource.
    if (index_resource != NULL) {
        m_TabManager->CloseTabForResource(index_resource);
    }

    // Create an HTMLResource for the INDEX if it doesn't exist.
    if (index_resource == NULL) {
        index_resource = m_Book->CreateEmptyHTMLFile();
        index_resource->RenameTo(HTML_INDEX_FILE);
    }

    // Make sure you not indexing the index page itself
    html_resources.removeOne(index_resource);

    // Skip indexing any epub3 nav document
    if (nav_resource) {
        html_resources.removeOne(nav_resource);
    }

    // Scan the book, add ids for any tag containing at least one index entry and store the
    // document index entry at the same time (including custom and from the index editor).
    if (!Index::BuildIndex(html_resources)) {
        QApplication::restoreOverrideCursor();
        return;
    }

    // Collect the information to fill int the appropriate templates
    QString indexbookpath = index_resource->GetRelativePath();
    QString stylebookpath = styleresource->GetRelativePath();

    // Write out the HTML index file.
    IndexHTMLWriter index(indexbookpath, stylebookpath);
    index_resource->SetText(index.WriteXML(version));

    // Normally Setting a semantic on a resource that already has it set will remove the semantic.
    //  Pass along toggle as false to disable this default behaviour
    if (version.startsWith('3')) {
        NavProcessor navproc(m_Book->GetOPF()->GetNavResource());
        navproc.AddLandmarkCode(index_resource, "index", false);
    } else {
        m_Book->GetOPF()->AddGuideSemanticCode(index_resource, "index", false);
    }
    m_Book->SetModified();
    m_BookBrowser->Refresh();
    OpenResource(index_resource);
    QApplication::restoreOverrideCursor();
}

void MainWindow::DeleteReportsStyles(QList<BookReports::StyleData *> reports_styles_to_delete)
{
    // html_filename and css_filename fields in StyleData have been converted to bookpaths

    // Convert the styles to CSS Selectors
    QHash<QString, QList<CSSInfo::CSSSelector *>> css_styles_to_delete;
    foreach(BookReports::StyleData * report_style, reports_styles_to_delete) {
        CSSInfo::CSSSelector *selector = new CSSInfo::CSSSelector();
        selector->text = report_style->css_selector_text;
        selector->pos = report_style->css_selector_position;
        QString css_filename = report_style->css_filename;
        css_styles_to_delete[css_filename].append(selector);
    }
    // Confirm which styles to delete
    DeleteStyles delete_styles(css_styles_to_delete, this);
    connect(&delete_styles, SIGNAL(OpenFileRequest(QString, int, int)), this, SLOT(OpenFile(QString, int, int)));

    if (delete_styles.exec() != QDialog::Accepted) {
        return;
    }

    css_styles_to_delete = delete_styles.GetStylesToDelete();

    if (css_styles_to_delete.count() < 1) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Actually delete the styles
    QHashIterator<QString, QList<CSSInfo::CSSSelector *>> stylesheets_to_delete(css_styles_to_delete);

    while (stylesheets_to_delete.hasNext()) {
        stylesheets_to_delete.next();
        DeleteCSSStyles(stylesheets_to_delete.key(), stylesheets_to_delete.value());
    }

    ShowMessageOnStatusBar(tr("Styles deleted."));

    QApplication::restoreOverrideCursor();
}


void MainWindow::ReportsDialog()
{
    ShowMessageOnStatusBar(tr("Reports Being Generated."));

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // since we do report file sizes, we have to flush all changes to disk
    SaveTabData();
    m_Book->GetFolderKeeper()->SuspendWatchingResources();
    m_Book->SaveAllResourcesToDisk();
    m_Book->GetFolderKeeper()->ResumeWatchingResources();

    if (!m_Book.data()->GetNonWellFormedHTMLFiles().isEmpty()) {
        QMessageBox::warning(this, tr("Sigil"), tr("Reports cancelled due to XML not well formed."));
        QApplication::restoreOverrideCursor();
        return;
    }

    DBG qDebug() << "Creating All of the Reports";
    m_Reports->CreateReports(m_Book);
    
    QApplication::restoreOverrideCursor();

    // non-modal dialog
    m_Reports->show();
    m_Reports->raise();
    m_Reports->activateWindow();
}

// This routine accepts a file_path that is a book path
void MainWindow::OpenFile(QString bookpath, int line, int position)
{
    if (bookpath.isEmpty()) {
        return;
    }
    // if position exists use it as is is more precise
    // otherwise use line
    if (position < 0) {
        if (line < 1) line = 1;
    }

    try {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(bookpath);
        OpenResource(resource, line, position);
    } catch (ResourceDoesNotExist&) {
        //
    }
}

// note the files_to_delete is a list of Resource Book Paths
// for safety
void MainWindow::DeleteFilenames(QStringList files_to_delete)
{
    if (files_to_delete.count() <= 0) {
        return;
    }

    QList <Resource *> resources;
    foreach(QString file_path, files_to_delete) {
        try {
            Resource *resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(file_path);
            resources.append(resource);
        } catch (ResourceDoesNotExist&) {
            continue;
        }
    }
    RemoveResources(resources);
}

bool MainWindow::DeleteCSSStyles(const QString &bookpath, QList<CSSInfo::CSSSelector *> css_selectors)
{
    // Save our tabs data as we will be modifying the underlying resources
    SaveTabData();
    bool is_modified = false;
    bool is_found = false;
    // Try our CSS resources first as most likely place for a style
    QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();
    foreach(Resource * resource, css_resources) {
        if (resource->GetRelativePath() == bookpath) {
            CSSResource *css_resource = qobject_cast<CSSResource *>(resource);
            is_found = true;
            is_modified = css_resource->DeleteCSStyles(css_selectors);
            break;
        }
    }

    if (!is_found) {
        // Try an inline style instead
        QList<Resource *> html_resources = GetAllHTMLResources();
        foreach(Resource * resource, html_resources) {
            if (resource->GetRelativePath() == bookpath) {
                HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
                is_modified = html_resource->DeleteCSStyles(css_selectors);
                break;
            }
        }
    }

    if (is_modified) {
        m_Book->SetModified();
    }

    return is_modified;
}

bool MainWindow::DeleteUnusedMedia(bool in_automate)
{
    SaveTabData();
    if (!m_Book.data()->GetNonWellFormedHTMLFiles().isEmpty()) {
        QMessageBox::warning(this, tr("Sigil"), tr("Delete Unused Media Files cancelled due to XML not well formed."));
        return false;
    }

    QRegularExpression url_file_search("url\\s*\\(\\s*['\"]?([^\\(\\)'\"]*)[\"']?\\)");

    QList<Resource *> resources;
    // hash key is media bookpath which returns a list of all html bookpaths that use it
    QHash<QString, QStringList> html_files_hash = m_Book->GetHTMLFilesUsingMedia();

    // Get file urls from HTML internal style tags
    QStringList style_bookpaths = m_Book->GetStyleUrlsInHTMLFiles();

    // Get files urls from CSS files
    QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();
    foreach(Resource *resource, css_resources) {
        CSSResource *css_resource = qobject_cast<CSSResource *>(resource);
        QString startdir = css_resource->GetFolder();
        CSSInfo css_info(css_resource->GetText());
            QStringList urllist = css_info.getAllPropertyValues("");
            foreach (QString url, urllist) {
                QRegularExpressionMatch match = url_file_search.match(url);
                if (match.hasMatch()) {
                    QString ahref = match.captured(1);
                    if (ahref.indexOf(":") == -1) {
                        style_bookpaths << Utility::buildBookPath(ahref, startdir);
                    }
                }
            }
    }

    style_bookpaths.removeDuplicates();

    foreach(Resource * resource, m_BookBrowser->AllMediaResources()) {
        QString filepath = resource->GetRelativePath();

        // Include the file in the list to delete if it was not referenced
        if (html_files_hash[filepath].count() == 0 && !style_bookpaths.contains(filepath)) {
            // If used as cover image, consider it referenced.
            ImageResource *image_resource = qobject_cast<ImageResource *>(resource);
            if (!image_resource || !m_Book->GetOPF()->IsCoverImage(image_resource)) {
                resources.append(resource);
            }
        }
    }

    if (resources.count() > 0) {
        RemoveResources(resources);
        ShowMessageOnStatusBar(tr("Unused media files deleted."));
    } else {
        if (in_automate) {
            ShowMessageOnStatusBar(tr("There are no unused image, video or audio files to delete."));
        } else {
            QMessageBox::information(this, tr("Sigil"), tr("There are no unused image, video or audio files to delete."));
        }
    }
    return true;
}

bool MainWindow::DeleteUnusedStyles(bool in_automate)
{
    SaveTabData();
    if (!m_Book.data()->GetNonWellFormedHTMLFiles().isEmpty()) {
        QMessageBox::warning(this, tr("Sigil"), tr("Delete Unused Styles cancelled due to XML not well formed."));
        return false;
    }

    // This one handles all selector types
    QList<BookReports::StyleData *> css_selector_usage = BookReports::GetAllCSSSelectorsUsed(m_Book, true);
    QList<BookReports::StyleData *> css_selectors_to_delete;
    foreach(BookReports::StyleData *selector, css_selector_usage) {
        if (selector->html_filename.isEmpty()) {
            css_selectors_to_delete.append(selector);
        }
    }

    if (css_selectors_to_delete.count() > 0) {
        DeleteReportsStyles(css_selectors_to_delete);
    } else {
        if (in_automate) {
            ShowMessageOnStatusBar(tr("There are no unused stylesheet selectors to delete."));
        } else {
            QMessageBox::information(this, tr("Sigil"), tr("There are no unused stylesheet selectors to delete."));
        }
    }
    qDeleteAll(css_selector_usage);
    return true;
}

void MainWindow::InsertFileDialog()
{
    SaveTabData();
    ShowMessageOnStatusBar();

    FlowTab *flow_tab = GetCurrentFlowTab();
    if (!flow_tab || !flow_tab->InsertFileEnabled()) {
        QMessageBox::warning(this, tr("Sigil"), tr("You cannot insert a file at this position."));
        return;
    }

    QStringList selected_files;
    QList<Resource *> media_resources = m_BookBrowser->AllMediaResources();

    QString title = tr("Insert File");
    SelectFiles select_files(title, media_resources, m_LastInsertedFile, this);

    if (select_files.exec() == QDialog::Accepted) {
        if (select_files.IsInsertFromDisk()) {
            InsertFilesFromDisk();
        } else {
            selected_files = select_files.SelectedImages();
            InsertFiles(selected_files);
        }
    }
}

// selected_files is a list of existing book paths
void MainWindow::InsertFiles(const QStringList &selected_files)
{
    if (!selected_files.isEmpty()) {

        FlowTab *flow_tab = GetCurrentFlowTab();
        if (!flow_tab) {
            return;
        }

        HTMLResource* tab_resource = qobject_cast<HTMLResource *>(flow_tab->GetLoadedResource());
        if (!tab_resource) {
            return;
        }

        if (flow_tab->InsertFileEnabled()) {
            foreach(QString selected_file, selected_files) {
                try {
                    Resource *resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(selected_file);
                    QString relative_path = resource->GetRelativePathFromResource(tab_resource);
                    relative_path = Utility::URLEncodePath(relative_path);

                    // extract just the filename without extension to create a text label
                    QString filename = resource->Filename();
                    if (filename.contains(".")) {
                        filename = filename.left(filename.lastIndexOf("."));
                    }

                    QString html;
                    if (resource->Type() == Resource::ImageResourceType || resource->Type() == Resource::SVGResourceType) {
                        html = QString("<img alt=\"%1\" src=\"%2\"/>").arg(filename).arg(relative_path);
                    } else if (resource->Type() == Resource::VideoResourceType) {
                        // When inserted in BV the filename will disappear
                        html = QString("<video controls=\"controls\" src=\"%1\">%2</video>").arg(relative_path).arg(filename);
                    } else if (resource->Type() == Resource::AudioResourceType) {
                        html = QString("<audio controls=\"controls\" src=\"%1\">%2</audio>").arg(relative_path).arg(filename);
                    }

                    flow_tab->InsertFile(html);
                } catch (ResourceDoesNotExist&) {
                    Utility::DisplayStdErrorDialog(tr("The file \"%1\" does not exist.") .arg(selected_file));
                }
            }
        }

        flow_tab->ResumeTabReloading();
        m_LastInsertedFile = selected_files.last();
    }
}

void MainWindow::InsertFilesFromDisk()
{
    // Prompt the user for the images to add.
    // Workaround for insert same image twice from disk causing a book view refresh
    // due to the linked resource being modified. Will perform the refresh afterwards.
    FlowTab *flow_tab = GetCurrentFlowTab();
    if (flow_tab) {
        flow_tab->SuspendTabReloading();
    }

    // We must disconnect the ResourcesAdded signal to avoid LoadTabContent being called
    // which results in the inserted image being cleared from the BV page immediately.
    disconnect(m_BookBrowser, SIGNAL(ResourcesAdded()), this, SLOT(ResourcesAddedOrDeletedOrMoved()));
    QStringList bookpaths = m_BookBrowser->AddExisting(true);
    connect(m_BookBrowser, SIGNAL(ResourcesAdded()), this, SLOT(ResourcesAddedOrDeletedOrMoved()));
    // Since we disconnected the signal we will have missed forced clearing of cache
    MainWindow::clearMemoryCaches();
    InsertFiles(bookpaths);
}

void MainWindow::InsertSpecialCharacter()
{
    // non-modal dialog
    m_SelectCharacter->show();
    m_SelectCharacter->raise();
    m_SelectCharacter->activateWindow();
}

void MainWindow::InsertId()
{
    SaveTabData();
    // Get current id attribute value if any
    ShowMessageOnStatusBar();

    FlowTab *flow_tab = GetCurrentFlowTab();
    if (!flow_tab || !flow_tab->InsertIdEnabled()) {
        QMessageBox::warning(this, tr("Sigil"), tr("You cannot insert an id at this position."));
        return;
    }

    QString id = flow_tab->GetAttributeId();

    HTMLResource *html_resource = qobject_cast<HTMLResource *>(flow_tab->GetLoadedResource());

    SelectId select_id(id, html_resource, m_Book, this);

    if (select_id.exec() == QDialog::Accepted) {
        QString selected_id = select_id.GetId();
        QRegularExpression invalid_id("(^[^A-Za-z]|[^A-Za-z0-9_:\\.-])");
        QRegularExpressionMatch mo = invalid_id.match(selected_id);

        if (mo.hasMatch()) {
            QMessageBox::warning(this, tr("Sigil"), tr("ID is invalid - must start with a letter, followed by letter number _ : - or ."));
            return;
        };

        if (!flow_tab->InsertId(select_id.GetId())) {
            QMessageBox::warning(this, tr("Sigil"), tr("You cannot insert an id at this position."));
        }
    }
}

void MainWindow::InsertHyperlink()
{
    SaveTabData();
    // Get current id attribute value if any
    ShowMessageOnStatusBar();

    FlowTab *flow_tab = GetCurrentFlowTab();
    if (!flow_tab || !flow_tab->InsertHyperlinkEnabled()) {
        QMessageBox::warning(this, tr("Sigil"), tr("You cannot insert a link at this position."));
        return;
    }

    QString href = flow_tab->GetAttributeHref();

    HTMLResource *html_resource = qobject_cast<HTMLResource *>(flow_tab->GetLoadedResource());
    QList<Resource *> resources = GetAllHTMLResources() + m_BookBrowser->AllMediaResources();
    SelectHyperlink select_hyperlink(href, html_resource, "html", resources, m_Book, this);

    if (select_hyperlink.exec() == QDialog::Accepted) {
        QString target = select_hyperlink.GetTarget();
        if (target.contains("<") || target.contains(">")) {
            QMessageBox::warning(this, tr("Sigil"), tr("Link is invalid - cannot contain '<' or '>'"));
            return;
        };

        if (!flow_tab->InsertHyperlink(target)) {
            QMessageBox::warning(this, tr("Sigil"), tr("You cannot insert a link at this position."));
        }
    }
}

void MainWindow::MarkForIndex()
{
    SaveTabData();
    ShowMessageOnStatusBar();

    FlowTab *flow_tab = GetCurrentFlowTab();
    if (!flow_tab || !flow_tab->MarkForIndexEnabled()) {
        QMessageBox::warning(this, tr("Sigil"), tr("You cannot mark an index at this position or without selecting text."));
        return;
    }

    // Get current id attribute value if any
    QString title = flow_tab->GetAttributeIndexTitle();
    SelectIndexTitle select_index_title(title, this);
    if (select_index_title.exec() == QDialog::Accepted) {
        QString entry = select_index_title.GetTitle();
        if (entry.contains("<") || entry.contains(">")) {
            QMessageBox::warning(this, tr("Sigil"), tr("Entry is invalid - cannot contain '<' or '>'"));
            return;
        };

        if (!flow_tab->MarkForIndex(entry)) {
            QMessageBox::warning(this, tr("Sigil"), tr("You cannot mark an index at this position."));
        }
    }
}

void MainWindow::ApplicationPaletteChanged()
{
    // we need to force a full reload of all Tabs and Preview Window
    // qDebug() << "ApplicationPaletteChanged";
    m_TabManager->ReopenTabs();
    UpdatePreview();
}

void MainWindow::ApplicationFocusChanged(QWidget *old, QWidget *now)
{
    QWidget *window = QApplication::activeWindow();

    if (!window || !now) {
        // Nothing to do - application is exiting
        return;
    }

    // We are only interested in focus events that take place in this MainWindow
    if (window != this) {
        return;
    }

    m_LastPasteTarget = dynamic_cast<PasteTarget *>(now);

    // Update the zoom target based on current window.
    if (m_PreviewWindow->HasFocus()) {
        m_ZoomPreview = true;
        float zoom_factor = GetZoomFactor();
        UpdateZoomLabel(zoom_factor);
        UpdateZoomSlider(zoom_factor);
    } else {
        ContentTab *tab = m_TabManager->GetCurrentContentTab();
        if (tab != NULL && tab->hasFocus()) {
            m_ZoomPreview = false;
            float zoom_factor = GetZoomFactor();
            UpdateZoomLabel(zoom_factor);
            UpdateZoomSlider(zoom_factor);
        }
    }
}

void MainWindow::QuickLaunchPlugin(int i)
{
    SettingsStore ss;
    QStringList namemap = ss.pluginMap();
    if ((i >= 0) && (namemap.count() > i)) {
        QString pname = namemap.at(i);
        if (m_pluginList.contains(pname)) {
            // QApplication keeps a single modalWindowList across multiple main
            // windows and this list is not updated until modal dialog is deleted
            { 
                PluginRunner prunner(m_TabManager, this);
                prunner.exec(pname);
            }
            qApp->processEvents();
        }
    }
}

void MainWindow::PasteTextIntoCurrentTarget(const QString &text)
{
    if (m_LastPasteTarget == NULL) {
        ShowMessageOnStatusBar(tr("Select the destination to paste into first."));
        return;
    }

    ShowMessageOnStatusBar();
    m_LastPasteTarget->PasteText(text);
}


void MainWindow::PasteClipIntoCurrentTarget(int clip_number)
{
    if (m_LastPasteTarget == NULL) {
        ShowMessageOnStatusBar(tr("Select the destination to paste into first."));
        return;
    }

    bool applied = m_LastPasteTarget->PasteClipNumber(clip_number);

    if (applied) {
        // Clear the statusbar afterwards but only if entries were pasted.
        ShowMessageOnStatusBar(tr("Pasted clip entry %1.").arg(clip_number));
    }
}

// How to deal with this as each clipEntry struct created with new and passed via
// emit signal to here?  Where and show should their memory be freed.
// Perhaps we need to make clipEntry a QObject instead of just a struct or use
// smart pointers
void MainWindow::PasteClipEntriesIntoCurrentTarget(const QList<ClipEditorModel::clipEntry *> &clips)
{
    if (m_LastPasteTarget == NULL) {
        ShowMessageOnStatusBar(tr("Select the destination to paste into first."));
        foreach(ClipEditorModel::clipEntry * entry, clips) {
            if (entry) delete entry;
        }
        return;
    }

    bool applied = m_LastPasteTarget->PasteClipEntries(clips);

    if (applied) {
        // Clear the statusbar afterwards but only if entries were pasted.
        ShowMessageOnStatusBar();
    }

    foreach(ClipEditorModel::clipEntry * entry, clips) {
        if (entry) delete entry;
    }

}

void MainWindow::PasteClipEntriesIntoPreviousTarget(const QList<ClipEditorModel::clipEntry *> &clips)
{
    ContentTab *tab = GetCurrentContentTab();
    if (tab == NULL)  return;
    FlowTab *flow_tab = qobject_cast<FlowTab *>(tab);
    if (flow_tab && flow_tab->PasteClipEntries(clips)) {
        flow_tab->setFocus();
        ShowMessageOnStatusBar();
        return;
    }
    CSSTab * css_tab = qobject_cast<CSSTab *>(tab);
    if (css_tab && css_tab->PasteClipEntries(clips)) {
        css_tab->setFocus();
        ShowMessageOnStatusBar();
    }
}

void MainWindow::MergeResources(QList <Resource *> resources)
{
    if (resources.isEmpty()) {
        return;
    }

    // Save the tab data
    SaveTabData();

    // Convert merge previous to merge selected so all files can be checked for validity
    if (resources.count() == 1) {
        Resource *resource = m_Book->PreviousResource(resources.first());

        if (!resource || resource == resources.first()) {
            QMessageBox::warning(this, tr("Sigil"), tr("One resource selected and there is no previous resource to merge into."));
            return;
        }

        resources.prepend(resource);
    } else {
        QMessageBox::StandardButton button_pressed;
        button_pressed = QMessageBox::warning(this, tr("Sigil"),
                                              tr("Are you sure you want to merge the selected files?\nThis action cannot be reversed."),
                                              QMessageBox::Ok | QMessageBox::Cancel);

        if (button_pressed != QMessageBox::Ok) {
            return;
        }
    }

    // Check if data is well formed
    QList<HTMLResource*> html_resources;
    foreach(Resource* resource, resources) {
        HTMLResource* htmlresource = qobject_cast<HTMLResource*>(resource);
        if (htmlresource) html_resources << htmlresource;
    }
    if (!m_Book->CheckHTMLFilesForWellFormedness(html_resources)) {
        QMessageBox::warning(this, tr("Sigil"), tr("Merge cancelled: XHTML files involved in merge are not well formed."));
            return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    // we need to check for duplicate ids being used in any of the files to be merged
    // and fix them here first before proceeding
    QHash<QString,QStringList> BookPathIds = m_Book->GetIdsInHTMLFiles();
    QSet<QString> Duplicates;
    QSet<QString> UsedIds;
    QHash<QString, QStringList> IdBookPaths;
    // for the set of files involved, invert BookPathIds to identify duplicates
    foreach(Resource * resource, resources) {
        QString bookpath = resource->GetRelativePath();
        QStringList ids=BookPathIds.value(bookpath, QStringList());
        foreach(QString id, ids) {
            QStringList bpaths = IdBookPaths.value(id, QStringList());
            bpaths << bookpath;
            if (bpaths.size() >= 2) Duplicates.insert(id);
            IdBookPaths[id] = bpaths;
            UsedIds.insert(id);
        }
    }
    QStringList Dups = Duplicates.toList();
    if (!Dups.isEmpty()) {
        // if duplicates exist, run the SourceUpdates/FragmentUpdates
        QHash<QString, QString> Updates;
        foreach(QString id, Dups) {
            // qDebug() << "Id duplicated: " << id << " in " << IdBookPaths[id];
            QStringList bpaths = IdBookPaths[id];
            for (int i=1; i < bpaths.size(); i++) {
                QString newid = Utility::GenerateUniqueId(id, UsedIds);
                Updates[bpaths.at(i) + "#" + id] = newid;
                UsedIds.insert(newid);
            }

        }
        QList<HTMLResource *> AllHTMLResources= m_Book->GetHTMLResources();
        FragmentUpdates::UpdateFragments(AllHTMLResources, Updates);
        QString version = m_Book->GetOPF()->GetEpubVersion();
        if (version.startsWith("2")) {
            m_Book->GetOPF()->UpdateGuideFragments(Updates);
        }
    }

    // Close all tabs being updated to prevent BV overwriting the new data
    foreach(Resource *resource, resources) {
        m_TabManager->CloseTabForResource(resource);
    }

    // Close the OPF tab
    bool opf_was_open = m_TabManager->CloseOPFTabIfOpen();

    Resource *resource_to_open = resources.first();
    Resource *failed_resource = m_Book->MergeResources(resources);

    if (failed_resource != NULL) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, tr("Sigil"), tr("Cannot merge file %1").arg(failed_resource->ShortPathName()));
        QApplication::setOverrideCursor(Qt::WaitCursor);
        resource_to_open = failed_resource;
    } else {
        m_BookBrowser->Refresh();
    }

    // Reopen OPF tab if it got closed
    if (opf_was_open) {
        OpenResource(m_Book->GetOPF());
    }
    OpenResource(resource_to_open);
    UpdateBrowserSelectionToTab();
    QApplication::restoreOverrideCursor();
    ShowMessageOnStatusBar(tr("Merge completed. You may need to regenerate or edit your Table Of Contents."));
}

void MainWindow::LinkStylesheetsToResources(QList <Resource *> resources)
{
    if (resources.isEmpty()) {
        return;
    }

    SaveTabData();

    // Check if data is well formed before saving
    foreach (Resource *r, resources) {
        HTMLResource *h = qobject_cast<HTMLResource *>(r);
        if (!h) {
            continue;
        }
        if (!h->FileIsWellFormed()) {
            QMessageBox::warning(this, tr("Sigil"), tr("Link Stylesheets cancelled: %1, XML not well formed.").arg(h->ShortPathName()));
            return;
        }
    }

    // Choose which stylesheets to link
    LinkStylesheets link(GetStylesheetsMap(resources), this);

    if (link.exec() != QDialog::Accepted) {
        return;
    }

    Resource *current_resource = NULL;
    ContentTab *tab = m_TabManager->GetCurrentContentTab();

    if (tab != NULL) {
        current_resource = tab->GetLoadedResource();
    }

    QStringList stylesheets = link.GetStylesheets();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // Convert HTML resources into HTMLResource types
    QList<HTMLResource *>html_resources;
    foreach(Resource *resource, resources) {
        html_resources.append(qobject_cast<HTMLResource *>(resource));
    }
    LinkUpdates::UpdateLinksInAllFiles(html_resources, stylesheets);
    m_Book->SetModified();

    if (current_resource && resources.contains(current_resource)) {
        OpenResource(current_resource);
    }

    SelectResources(resources);
    QApplication::restoreOverrideCursor();
}

void MainWindow::FindWord(QString word)
{
    SaveTabData();

    // Note the current tab if it is an HTML file.
    HTMLResource *current_html_resource = NULL;
    QString current_html_filename;
    FlowTab *flow_tab = GetCurrentFlowTab();
    if (flow_tab) {
        Resource *resource = flow_tab->GetLoadedResource();
        if (resource->Type() == Resource::HTMLResourceType) {
            current_html_resource = qobject_cast<HTMLResource *>(resource);
            current_html_filename = current_html_resource->ShortPathName();
        }
    }

    // Get list of files from current to end followed
    // by start to just before current file.
    QList<Resource *> html_resources;
    QList<Resource *> resources = GetAllHTMLResources();
    int passed_current = false;
    foreach(Resource *resource, resources) {
        if (!passed_current && resource->ShortPathName() != current_html_filename) {
            continue;
        }
        passed_current = true;
        html_resources.append(resource);
    }
    foreach(Resource * resource, resources) {
        html_resources.append(resource);
        if (resource->ShortPathName() == current_html_filename) {
            break;
        }
    }

    // Search for the word.
    QString default_lang = m_Book->GetConstOPF()->GetPrimaryBookLanguage();
    default_lang.replace('_','-');

    bool done_current = false;
    foreach (Resource *resource, html_resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (!html_resource) {
            continue;
        }
        int start_pos = 0;
        // Reset the start to current cursor position only if this is the
        // first time we are in the current file.
        if (resource->ShortPathName() == current_html_filename) {
            if (!done_current) {
                FlowTab *flow_tab = GetCurrentFlowTab();
                if (flow_tab) {
                    start_pos = flow_tab->GetCursorPosition();
                }
            }
            done_current = true;
        }
        QString text = html_resource->GetText();
        int found_pos = HTMLSpellCheckML::WordPosition(text, word, start_pos, default_lang);
        // int found_pos = HTMLSpellCheck::WordPosition(text, word, start_pos);
        if (found_pos >= 0) {
            if (resource->ShortPathName() != current_html_filename) {
                OpenResourceAndWaitUntilLoaded(resource, -1, found_pos);
            }
            FlowTab *flow_tab = GetCurrentFlowTab();
            if (flow_tab) {
                flow_tab->HighlightWord(HTMLSpellCheckML::textOf(word), found_pos);
                break;
            }
        }
    }
}

void MainWindow::UpdateWord(QString old_word, QString new_word)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    SaveTabData();

    QList<HTMLResource *> html_resources;
    QList<Resource *> resources = GetAllHTMLResources();
    foreach(Resource * resource, resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (html_resource) {
            html_resources.append(html_resource);
        }
    }
    QString default_lang = m_Book->GetOPF()->GetPrimaryBookLanguage();
    default_lang.replace('_','-');

    WordUpdates::UpdateWordInAllFiles(html_resources, default_lang, old_word, new_word);
    m_Book->SetModified();
    m_SpellcheckEditor->Refresh();
    ShowMessageOnStatusBar(tr("Word updated."));

    QApplication::restoreOverrideCursor();
}

QList<std::pair<QString, bool>> MainWindow::GetStylesheetsMap(QList<Resource *> resources)
{
    QList<std::pair<QString, bool>> stylesheet_map;
    QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();
    // Use the first resource to get a list of known linked stylesheets in order.
    QStringList checked_linked_bookpaths = GetStylesheetsAlreadyLinked(resources.at(0));
    // Then only consider them included if every selected resource includes
    // the same stylesheets in the same order.
    foreach(Resource * valid_resource, resources) {
        QStringList linked_bookpaths = GetStylesheetsAlreadyLinked(valid_resource);
        foreach(QString bookpath, checked_linked_bookpaths) {
            if (!linked_bookpaths.contains(bookpath)) {
                checked_linked_bookpaths.removeOne(bookpath);
            }
        }
    }
    // Save the paths included in all resources in order
    foreach(QString bookpath, checked_linked_bookpaths) {
        stylesheet_map.append(std::make_pair(bookpath, true));
    }
    // Save all the remaining paths and mark them not included
    foreach(Resource * resource, css_resources) {
        QString abookpath = resource->GetRelativePath();

        if (!checked_linked_bookpaths.contains(abookpath)) {
            stylesheet_map.append(std::make_pair(abookpath, false));
        }
    }
    return stylesheet_map;
}


QStringList MainWindow::GetStylesheetsAlreadyLinked(Resource *resource)
{
    HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
    QStringList linked_stylesheets;
    QStringList existing_stylesheets;
    foreach(Resource * css_resource, m_BookBrowser->AllCSSResources()) {
        existing_stylesheets.append(css_resource->GetRelativePath());
    }
    foreach(QString bookpath, html_resource->GetLinkedStylesheets()) {
        // Only list the stylesheet if it exists in the book
        if (existing_stylesheets.contains(bookpath)) {
            linked_stylesheets.append(bookpath);
        }
    }
    return linked_stylesheets;
}

void MainWindow::LinkJavascriptsToResources(QList <Resource *> resources)
{
    if (resources.isEmpty()) {
        return;
    }

    SaveTabData();

    // Check if data is well formed before saving                                                                   
    foreach (Resource *r, resources) {
        HTMLResource *h = qobject_cast<HTMLResource *>(r);
        if (!h) {
            continue;
        }
        if (!h->FileIsWellFormed()) {
            QMessageBox::warning(this, tr("Sigil"), tr("Link Javascripts cancelled: %1, XML not well formed.").arg(\
h->ShortPathName()));
            return;
        }
    }

    // Choose which javascripts to link                                                                             
    LinkJavascripts link(GetJavascriptsMap(resources), this);

    if (link.exec() != QDialog::Accepted) {
        return;
    }

    Resource *current_resource = NULL;
    ContentTab *tab = m_TabManager->GetCurrentContentTab();

    if (tab != NULL) {
        current_resource = tab->GetLoadedResource();
    }

    QStringList javascripts = link.GetJavascripts();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    // Convert HTML resources into HTMLResource types                                                               
    QList<HTMLResource *>html_resources;
    foreach(Resource *resource, resources) {
        html_resources.append(qobject_cast<HTMLResource *>(resource));
    } 
    JavascriptUpdates::UpdateJavascriptsInAllFiles(html_resources, javascripts);

    m_Book->GetOPF()->UpdateManifestProperties(resources);

    m_Book->SetModified();

    if (current_resource && resources.contains(current_resource)) {
        OpenResource(current_resource);
    }

    SelectResources(resources);
    QApplication::restoreOverrideCursor();
}

QList<std::pair<QString, bool>> MainWindow::GetJavascriptsMap(QList<Resource *> resources)
{
    QList<std::pair<QString, bool>> javascript_map;
    QStringList mtypes = QStringList() << "application/javascript" << "text/javascript";
    QList<Resource *> js_resources = m_Book->GetFolderKeeper()->GetResourceListByMediaTypes(mtypes);
    // Use the first resource to get a list of known linked javascripts in order.                                   
    QStringList checked_linked_bookpaths = GetJavascriptsAlreadyLinked(resources.at(0));
    // Then only consider them included if every selected resource includes                                         
    // the same javascripts in the same order.                                                                      
    foreach(Resource * valid_resource, resources) {
        QStringList linked_bookpaths = GetJavascriptsAlreadyLinked(valid_resource);
        foreach(QString bookpath, checked_linked_bookpaths) {
            if (!linked_bookpaths.contains(bookpath)) {
		        checked_linked_bookpaths.removeOne(bookpath);
            }
        }
    }
    // Save the paths included in all resources in order                                                            
    foreach(QString bookpath, checked_linked_bookpaths) {
        javascript_map.append(std::make_pair(bookpath, true));
    }
    // Save all the remaining paths and mark them not included                                                      
    foreach(Resource * resource, js_resources) {
	    QString abookpath = resource->GetRelativePath();

        if (!checked_linked_bookpaths.contains(abookpath)) {
            javascript_map.append(std::make_pair(abookpath, false));
	    }
    }
    return javascript_map;
}

QStringList MainWindow::GetJavascriptsAlreadyLinked(Resource *resource)
{
    HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
    QStringList linked_javascripts;
    QStringList existing_javascripts;
    QStringList mtypes = QStringList() << "application/javascript" << "text/javascript";
    foreach(Resource * js_resource, m_Book->GetFolderKeeper()->GetResourceListByMediaTypes(mtypes)) {
        existing_javascripts.append(js_resource->GetRelativePath());
    }
    foreach(QString bookpath, html_resource->GetLinkedJavascripts()) {
        // Only list the javascript if it exists in the book                                                        
        if (existing_javascripts.contains(bookpath)) {
            linked_javascripts.append(bookpath);
        }
    }
    return linked_javascripts;
}


void MainWindow::RemoveResources(QList<Resource *> resources)
{
    // Provide the open tab list to ensure one tab stays open
    if (resources.count() > 0) {
        m_BookBrowser->RemoveResources(m_TabManager->GetTabResources(), resources);
    } else {
        m_BookBrowser->RemoveSelection(m_TabManager->GetTabResources());
    }

    // check if user deleted the html resource last shown in Preview
    // and if removed update Preview's cache resource, text, and location
    QList<Resource *> current_resources = m_Book->GetFolderKeeper()->GetResourceListByType(Resource::HTMLResourceType);
    if (!current_resources.contains(m_PreviousHTMLResource)) {
        m_PreviousHTMLResource = NULL;
        m_PreviousHTMLText = "";
        m_PreviousHTMLLocation = QList<ElementIndex>();
    }

    ShowMessageOnStatusBar(tr("File(s) deleted."));
}

void MainWindow::EditTOCDialog()
{
    SaveTabData();

    QList<Resource *> resources = GetAllHTMLResources() + m_BookBrowser->AllMediaResources();
    EditTOC toc(m_Book, resources, this);

    if (toc.exec() != QDialog::Accepted) {
        ShowMessageOnStatusBar(tr("Edit Table of Contents cancelled."));
        return;
    }

    m_Book.data()->SetModified();
    ShowMessageOnStatusBar(tr("Table Of Contents edited."));
}

// For epub2 this set the NCX, for epub3 this sets the Nav TOC section
bool MainWindow::GenerateTOC(bool skip_selector)
{
    SaveTabData();
    QList<Resource *> resources = GetAllHTMLResources();

    if (resources.isEmpty()) {
        return false;
    }

    bool is_headings_changed = false;
    if (!skip_selector) {
        HeadingSelector toc(m_Book, this);

        if (toc.exec() != QDialog::Accepted) {
            ShowMessageOnStatusBar(tr("Generate TOC cancelled."));
            return false;
        }

        is_headings_changed = toc.IsBookChanged();
    }
    QApplication::setOverrideCursor(Qt::WaitCursor);

    bool is_toc_changed = false;

    QString version = m_Book->GetConstOPF()->GetEpubVersion();

    if (version.startsWith('3')) {
        NavProcessor navproc(m_Book->GetOPF()->GetNavResource());
        is_toc_changed = navproc.GenerateTOCFromBookContents(m_Book.data());
    } else {
        // Regenerate the NCX regardless of whether headings were changed, in case the user erased it.
        // using GetNCX() here will never return a nullptr on epub2
        is_toc_changed = m_Book->GetNCX()->GenerateNCXFromBookContents(m_Book.data());
    }
    if (is_headings_changed || is_toc_changed) {
        // Reload the current tab to see visual impact if user changed heading level(s)
        // It might not have been the current tab, but what the heck, possible user has the NCX open even.
        ResourcesAddedOrDeletedOrMoved();
        m_Book.data()->SetModified();
        ShowMessageOnStatusBar(tr("Table Of Contents generated."));
    } else {
        ShowMessageOnStatusBar(tr("No Table Of Contents changes were necessary."));
    }

    QApplication::restoreOverrideCursor();
    return true;
}


bool MainWindow::CreateHTMLTOC()
{
    SaveTabData();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString version = m_Book->GetOPF()->GetEpubVersion();

    CSSResource* css_resource;
    bool found_css = false;
    foreach(Resource *resource, m_BookBrowser->AllCSSResources()) {
        if (resource->Filename() == SGC_TOC_CSS_FILENAME) {
            css_resource = qobject_cast<CSSResource *> (resource);
            found_css = true;
        }
    }

    // If HTML TOC CSS file does not exist look for a default file
    // in preferences directory and if none create one.
    if (!found_css) {
        QString css_path = Utility::DefinePrefsDir() + "/" + SGC_TOC_CSS_FILENAME;
        if (QFile::exists(css_path)) {
            // Need to make sure InitialLoad is done in newly added css resource object to prevent
            // blank css issues after a save to disk
            Resource * resource = m_Book->GetFolderKeeper()->AddContentFileToFolder(css_path);
            css_resource = qobject_cast<CSSResource *> (resource);
            css_resource->InitialLoad();
        } else {
            css_resource = m_BookBrowser->CreateHTMLTOCCSSFile();
        }
    }

    HTMLResource *tocResource = NULL;
    HTMLResource *navResource = m_Book->GetOPF()->GetNavResource();
    QList<HTMLResource *> htmlResources;

    // list is built in spine order by the BookBrowser
    QList<Resource *> resources = GetAllHTMLResources();

    foreach(Resource * resource, resources) {
        HTMLResource *htmlResource = qobject_cast<HTMLResource *>(resource);
        if (htmlResource) {

            htmlResources.append(htmlResource);

            // prevent the nav resource from being chosen or used for an html toc
            if (htmlResource != navResource) {

                // Check if this is an existing HTML toc file.
                QString semantic_code;
                if (version.startsWith('3')) {
                    NavProcessor navproc(navResource);
                    semantic_code = navproc.GetLandmarkCodeForResource(htmlResource);
                } else {
                    semantic_code = m_Book->GetOPF()->GetGuideSemanticCodeForResource(htmlResource);
                }
                if (semantic_code == "toc") {
                    tocResource = htmlResource;
                } else if (resource->Filename() == HTML_TOC_FILE && tocResource == NULL) {
                    tocResource = htmlResource;
                }
            }
        }
    }
 
    if (tocResource != NULL) {

        QString msg = tr("An existing HTML Table of Contents file has been found.");
        if (!ProceedToOverwrite(msg, tocResource->ShortPathName())) {
            tocResource = NULL;
        }
    }

    // If you found an existing one, close the tab so the focus 
    // saving doesn't overwrite the text we are replacing in the resource.
    if (tocResource != NULL) {
        m_TabManager->CloseTabForResource(tocResource);
    }

    // Create the an HTMLResource for the TOC if it doesn't exit.
    if (tocResource == NULL) {
        tocResource = m_Book->CreateEmptyHTMLFile();
        tocResource->RenameTo(HTML_TOC_FILE);
        htmlResources.insert(0, tocResource);
        m_Book->GetOPF()->UpdateSpineOrder(htmlResources);
    }

    // Get Primary language from the OPF and use it to Translate "toc" for title
    QString lang = m_Book->GetOPF()->GetPrimaryBookLanguage();
    QString title = Landmarks::instance()->GetTitle("toc", lang); 
    TOCHTMLWriter toc(tocResource->GetRelativePath(), 
                      css_resource->GetRelativePath(),
                      m_TableOfContents->GetRootEntry(),
                      title);
    tocResource->SetText(toc.WriteXML(version));

    // For epub3 now allow multiple landmarks with the toc semantic set, this is legal as long
    // as only the toc nav exists uniquely

    // Setting a semantic on a resource that already has it set will remove the semantic.
    // Unless you pass toggle as false as the final parameter
    if (version.startsWith('3')) {
        NavProcessor navproc(navResource);
        navproc.AddLandmarkCode(tocResource, "toc", false);
    } else {
        m_Book->GetOPF()->AddGuideSemanticCode(tocResource, "toc", false);
    }
    m_Book->SetModified();
    m_BookBrowser->Refresh();
    OpenResource(tocResource);
    QApplication::restoreOverrideCursor();
    return true;
}


void MainWindow::ChangeCasing(QAction* act)
{
    ContentTab *tab = GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    QString name = act->objectName();
    Utility::Casing casing;

    if (name.contains("lowercase", Qt::CaseInsensitive)) {
        casing = Utility::Casing_Lowercase;
    }
    if (name.contains("uppercase", Qt::CaseInsensitive)) {
        casing = Utility::Casing_Uppercase;
    }
    if (name.contains("titlecase", Qt::CaseInsensitive)) {
        casing = Utility::Casing_Titlecase;
    }
    if (name.contains("capitalize", Qt::CaseInsensitive)) {
        casing = Utility::Casing_Capitalize;
    }

    tab->ChangeCasing(casing);
}

void MainWindow::MarkSelection()
{
    ContentTab *tab = GetCurrentContentTab();
    if (tab == NULL) {
        return;
    }
    bool marked = tab->MarkSelection();
    m_FindReplace->ShowHideMarkedText(marked);
    if (marked) {
        ShowMessageOnStatusBar(tr("Text selection marked."));
    } else {
        ShowMessageOnStatusBar(tr("Text selection unmarked."));
    }
}

void MainWindow::ClearMarkedText(ContentTab *old_tab)
{
    bool cleared = false;
    if (old_tab) {
        cleared = old_tab->ClearMarkedText();
    } else {
        ContentTab *tab = GetCurrentContentTab();
        if (tab == NULL) {
            return;
        }
        cleared = tab->ClearMarkedText();
    }
    if (cleared) {
        // Only show message if there was a selection to clear.
        ShowMessageOnStatusBar(tr("Text selection unmarked."));
    }

    m_FindReplace->ShowHideMarkedText(false);
}

void MainWindow::CodeView()
{
    UpdateBrowserSelectionToTab();
}

void MainWindow::SearchEditorDialog(SearchEditorModel::searchEntry *search_entry)
{
    // non-modal dialog
    m_SearchEditor->show();
    m_SearchEditor->raise();
    m_SearchEditor->activateWindow();

    if (search_entry) {
        m_SearchEditor->AddEntry(search_entry->is_group, search_entry, false);
    }
}

void MainWindow::ClipEditorDialog(ClipEditorModel::clipEntry *clip_entry)
{
    // non-modal dialog
    m_ClipEditor->show();
    m_ClipEditor->raise();
    m_ClipEditor->activateWindow();

    if (clip_entry) {
        m_ClipEditor->AddEntry(clip_entry->is_group, clip_entry, false);
    }
}

void MainWindow::CloseAllTabs()
{
    m_TabManager->CloseAllTabs();
}

void MainWindow::SaveTabData()
{
    m_TabManager->SaveTabData();
}

void MainWindow::MetaEditorDialog()
{
    MetaEditor medit(this);
    if (medit.exec() != QDialog::Accepted) {
        ShowMessageOnStatusBar(tr("Metadata Editor cancelled."));
        return;
    }
    ShowMessageOnStatusBar(tr("Metadata edited."));
    m_Book->SetModified();
}

void MainWindow::UserGuide()
{
    QDesktopServices::openUrl(QUrl(USER_GUIDE_URL));
}


void MainWindow::Donate()
{
    QDesktopServices::openUrl(QUrl(DONATE));
}


void MainWindow::SigilWebsite()
{
    QDesktopServices::openUrl(QUrl(SIGIL_WEBSITE));
}


void MainWindow::AboutDialog()
{
    About about(this);
    about.exec();
}


void MainWindow::PreferencesDialog()
{
    if (m_IsClosing) return;

    // QApplication keeps a single modalWindowList across multiple main
    // windows and this list is not updated until modal dialog is deleted
  
    Preferences prefers(this);
    prefers.exec();

    if (prefers.isReloadTabsRequired()) {
        m_TabManager->ReopenTabs();
        m_BookBrowser->Refresh();
    } else if (prefers.isRefreshBookBrowserRequired()) {
        m_BookBrowser->Refresh();
    } else if (prefers.isRefreshSpellingHighlightingRequired()) {
        RefreshSpellingHighlighting();
        // Make sure menu state is set
        SettingsStore settings;
        ui.actionAutoSpellCheck->setChecked(settings.spellCheck());
    }
    if (prefers.isRefreshClipHistoryLimitRequired()) {
        SettingsStore settings;
        m_ClipboardHistoryLimit = settings.clipboardHistoryLimit();
    }
    if (prefers.isReloadPreviewRequired()) {
        if (m_PreviewWindow) {
            UpdatePreview();
        }
    }

    if (m_SelectCharacter->isVisible()) {
        // To ensure any font size changes are immediately applied.
        m_SelectCharacter->show();
    }

    updateToolTipsOnPluginIcons();
}


void MainWindow::ManagePluginsDialog()
{
    if (m_IsClosing) return;

    Preferences prefers(this);
    prefers.makeActive(Preferences::PluginsPrefs);
    prefers.exec();

    // other preferences may have been changed as well
    if (prefers.isReloadTabsRequired()) {
        m_TabManager->ReopenTabs();
        m_BookBrowser->Refresh();
    } else if (prefers.isRefreshBookBrowserRequired()) {
        m_BookBrowser->Refresh();
    } else if (prefers.isRefreshSpellingHighlightingRequired()) {
        RefreshSpellingHighlighting();
        // Make sure menu state is set
        SettingsStore settings;
        ui.actionAutoSpellCheck->setChecked(settings.spellCheck());
    }
    if (prefers.isRefreshClipHistoryLimitRequired()) {
        SettingsStore settings;
        m_ClipboardHistoryLimit = settings.clipboardHistoryLimit();
    }

    if (m_SelectCharacter->isVisible()) {
        // To ensure any font size changes are immediately applied.
        m_SelectCharacter->show();
    }

    loadPluginsMenu();
}

void MainWindow::updateToolTipsOnPluginIcons()
{
    SettingsStore ss;
    QStringList namemap = ss.pluginMap();
    int i=0;
    foreach(QAction* pa, m_qlactions) {
        QString pname = tr("RunPlugin") + QString::number(i+1);
        if (namemap.count() > i) pname = namemap.at(i);
        pa->setToolTip(pname);
        i++;
    }
}

bool MainWindow::WellFormedCheckEpub()
{
    m_ValidationResultsView->ValidateCurrentBook();
    return true;
}


bool MainWindow::CharLessThan(const QChar &s1, const QChar &s2)
{
    return s1 < s2;
}


bool MainWindow::ValidateStylesheetsWithW3C()
{
    SaveTabData();
    QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();

    if (css_resources.isEmpty()) {
        ShowMessageOnStatusBar(tr("This EPUB does not contain any CSS stylesheets to validate."));
        return true;
    }

    foreach(Resource * resource, css_resources) {
        CSSResource *css_resource = qobject_cast<CSSResource *>(resource);
        css_resource->ValidateStylesheetWithW3C();
    }
    return true;
}


bool MainWindow::ReformatAllStylesheets(bool multiple_line_format)
{
    SaveTabData();
    QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();

    if (css_resources.isEmpty()) {
        ShowMessageOnStatusBar(tr("This EPUB does not contain any CSS stylesheets to reformat."));
        return true;
    }

    foreach(Resource * resource, css_resources) {
        CSSResource *css_resource = qobject_cast<CSSResource *>(resource);
        css_resource->ReformatCSS(multiple_line_format);
    }
    return true;
}

void MainWindow::ChangeSignalsWhenTabChanges(ContentTab *old_tab, ContentTab *new_tab)
{
    // qDebug() << "in ChangesSignalWhenTabChanges " << old_tab << new_tab;
    if (old_tab == new_tab) return;
    BreakTabConnections(old_tab);
    MakeTabConnections(new_tab);
    // Clear selection if the tab changed.
    ClearMarkedText(old_tab);
}


void MainWindow::UpdateMWState(bool set_tab_state)
{
    ContentTab *tab = GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    Resource::ResourceType type = tab->GetLoadedResource()->Type();

    if (type == Resource::HTMLResourceType) {
        if (set_tab_state) {
            FlowTab *ftab = qobject_cast<FlowTab *>(tab);

            if (ftab) {
                ftab->ReloadTabIfPending();
            }
            ClearMarkedText();
        }

        SetStateActionsCodeView();
    } else if (type == Resource::CSSResourceType) {
        SetStateActionsCSSView();
    } else if (type == Resource::XMLResourceType ||
               type == Resource::OPFResourceType ||
               type == Resource::NCXResourceType ||
               type == Resource::MiscTextResourceType ||
               type == Resource::SVGResourceType ||
               type == Resource::TextResourceType) {
        SetStateActionsRawView();
    } else {
        SetStateActionsStaticView();
    }
}


void MainWindow::UpdateUIOnTabChanges()
{
    ContentTab *tab = m_TabManager->GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    // Set enabled state based on selection change
    ui.actionCut                ->setEnabled(tab->CutEnabled());
    ui.actionCopy               ->setEnabled(tab->CopyEnabled());
    ui.actionPaste              ->setEnabled(tab->PasteEnabled());
    ui.actionDeleteLine         ->setEnabled(tab->DeleteLineEnabled());
    ui.actionAddToIndex         ->setEnabled(tab->AddToIndexEnabled());
    ui.actionMarkForIndex       ->setEnabled(tab->MarkForIndexEnabled());
    ui.actionRemoveFormatting   ->setEnabled(tab->RemoveFormattingEnabled());
    ui.actionRemoveTagPair      ->setEnabled(tab->RemoveTagPairEnabled());
    // Set whether icons are checked
    ui.actionBold           ->setChecked(tab->BoldChecked());
    ui.actionItalic         ->setChecked(tab->ItalicChecked());
    ui.actionUnderline      ->setChecked(tab->UnderlineChecked());
    ui.actionStrikethrough  ->setChecked(tab->StrikethroughChecked());
    ui.actionSubscript      ->setChecked(tab->SubscriptChecked());
    ui.actionSuperscript    ->setChecked(tab->SuperscriptChecked());
    ui.actionAlignLeft      ->setChecked(tab->AlignLeftChecked());
    ui.actionAlignRight     ->setChecked(tab->AlignRightChecked());
    ui.actionAlignCenter    ->setChecked(tab->AlignCenterChecked());
    ui.actionAlignJustify   ->setChecked(tab->AlignJustifyChecked());
    ui.actionInsertBulletedList ->setChecked(tab->BulletListChecked());
    ui.actionInsertNumberedList ->setChecked(tab->NumberListChecked());
    // State of zoom controls depends on current tab/view
    float zoom_factor = GetZoomFactor();
    UpdateZoomLabel(zoom_factor);
    UpdateZoomSlider(zoom_factor);
    UpdateCursorPositionLabel(tab->GetCursorLine(), tab->GetCursorColumn());
    SelectEntryOnHeadingToolbar(tab->GetCaretElementName());
}


void MainWindow::UpdateUIWhenTabsSwitch()
{
    ContentTab *tab = m_TabManager->GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    tab->UpdateDisplay();
    UpdateMWState();
    ClearMarkedText();
}


void MainWindow::UpdateUIOnTabCountChange()
{
    ui.actionNextTab       ->setEnabled(m_TabManager->GetTabCount() > 1);
    ui.actionPreviousTab   ->setEnabled(m_TabManager->GetTabCount() > 1);
    ui.actionCloseTab      ->setEnabled(m_TabManager->GetTabCount() > 1);
    ui.actionCloseOtherTabs->setEnabled(m_TabManager->GetTabCount() > 1);
}


void MainWindow::SetStateActionsCodeView()
{
    ui.actionPrintPreview->setEnabled(true);
    ui.actionPrint->setEnabled(true);
    ui.actionSplitSection->setEnabled(true);
    ui.actionInsertSGFSectionMarker->setEnabled(true);
    ui.actionInsertFile->setEnabled(true);
    ui.actionInsertSpecialCharacter->setEnabled(true);
    ui.actionInsertId->setEnabled(true);
    ui.actionInsertHyperlink->setEnabled(true);
    ui.actionInsertClosingTag->setEnabled(true);
    ui.actionUndo->setEnabled(true);
    ui.actionRedo->setEnabled(true);
    ui.actionPasteClipboardHistory->setEnabled(true);
    ui.actionBold         ->setEnabled(true);
    ui.actionItalic       ->setEnabled(true);
    ui.actionUnderline    ->setEnabled(true);
    ui.actionStrikethrough->setEnabled(true);
    ui.actionSubscript    ->setEnabled(true);
    ui.actionSuperscript  ->setEnabled(true);
    ui.actionAlignLeft   ->setEnabled(true);
    ui.actionAlignCenter ->setEnabled(true);
    ui.actionAlignRight  ->setEnabled(true);
    ui.actionAlignJustify->setEnabled(true);
    ui.actionDecreaseIndent->setEnabled(true);
    ui.actionIncreaseIndent->setEnabled(true);
    ui.actionTextDirectionLTR    ->setEnabled(true);
    ui.actionTextDirectionRTL    ->setEnabled(true);
    ui.actionTextDirectionDefault->setEnabled(true);
    ui.actionInsertBulletedList->setEnabled(false);
    ui.actionInsertNumberedList->setEnabled(false);
    ui.actionRemoveFormatting->setEnabled(true);
    ui.actionRemoveTagPair->setEnabled(true);
    ui.menuHeadings->setEnabled(true);
    ui.actionHeading1->setEnabled(true);
    ui.actionHeading2->setEnabled(true);
    ui.actionHeading3->setEnabled(true);
    ui.actionHeading4->setEnabled(true);
    ui.actionHeading5->setEnabled(true);
    ui.actionHeading6->setEnabled(true);
    ui.actionHeadingNormal->setEnabled(true);
    ui.actionInsertBulletedList ->setEnabled(true);
    ui.actionInsertNumberedList ->setEnabled(true);
    ui.actionCasingLowercase  ->setEnabled(true);
    ui.actionCasingUppercase  ->setEnabled(true);
    ui.actionCasingTitlecase ->setEnabled(true);
    ui.actionCasingCapitalize ->setEnabled(true);
    ui.actionFind->setEnabled(true);
    ui.actionFindNext->setEnabled(true);
    ui.actionFindPrevious->setEnabled(true);
    ui.actionReplaceCurrent->setEnabled(true);
    ui.actionReplaceNext->setEnabled(true);
    ui.actionReplacePrevious->setEnabled(true);
    ui.actionReplaceAll->setEnabled(true);
    ui.actionCount->setEnabled(true);
    ui.actionMarkSelection->setEnabled(true);
    ui.menuSearchCurrentFile->setEnabled(true);
    ui.actionFindNextInFile->setEnabled(true);
    ui.actionReplaceNextInFile->setEnabled(true);
    ui.actionReplaceAllInFile->setEnabled(true);
    ui.actionCountInFile->setEnabled(true);
    ui.actionGoToLine->setEnabled(true);
    ui.actionGoToLinkOrStyle->setEnabled(true);
    ui.actionAddMisspelledWord->setEnabled(true);
    ui.actionIgnoreMisspelledWord->setEnabled(true);
    ui.actionAutoSpellCheck->setEnabled(true);
    UpdateUIOnTabChanges();
    m_FindReplace->ShowHide();
}

void MainWindow::SetStateActionsCSSView()
{
    SetStateActionsRawView();
    ui.actionBold         ->setEnabled(true);
    ui.actionItalic       ->setEnabled(true);
    ui.actionUnderline    ->setEnabled(true);
    ui.actionStrikethrough->setEnabled(true);
    ui.actionAlignLeft   ->setEnabled(true);
    ui.actionAlignCenter ->setEnabled(true);
    ui.actionAlignRight  ->setEnabled(true);
    ui.actionAlignJustify->setEnabled(true);
    ui.actionTextDirectionLTR    ->setEnabled(true);
    ui.actionTextDirectionRTL    ->setEnabled(true);
    ui.actionTextDirectionDefault->setEnabled(true);
    UpdateUIOnTabChanges();
}

void MainWindow::SetStateActionsRawView()
{
    ui.actionPrintPreview->setEnabled(true);
    ui.actionPrint->setEnabled(true);
    ui.actionSplitSection->setEnabled(false);
    ui.actionInsertSGFSectionMarker->setEnabled(false);
    ui.actionInsertFile->setEnabled(false);
    ui.actionInsertSpecialCharacter->setEnabled(true);
    ui.actionInsertId->setEnabled(false);
    ui.actionInsertHyperlink->setEnabled(false);
    ui.actionInsertClosingTag->setEnabled(false);
    ui.actionUndo->setEnabled(true);
    ui.actionRedo->setEnabled(true);
    ui.actionPasteClipboardHistory->setEnabled(true);
    ui.actionBold         ->setEnabled(false);
    ui.actionItalic       ->setEnabled(false);
    ui.actionUnderline    ->setEnabled(false);
    ui.actionStrikethrough->setEnabled(false);
    ui.actionSubscript    ->setEnabled(false);
    ui.actionSuperscript  ->setEnabled(false);
    ui.actionAlignLeft   ->setEnabled(false);
    ui.actionAlignCenter ->setEnabled(false);
    ui.actionAlignRight  ->setEnabled(false);
    ui.actionAlignJustify->setEnabled(false);
    ui.actionDecreaseIndent->setEnabled(false);
    ui.actionIncreaseIndent->setEnabled(false);
    ui.actionTextDirectionLTR    ->setEnabled(false);
    ui.actionTextDirectionRTL    ->setEnabled(false);
    ui.actionTextDirectionDefault->setEnabled(false);
    ui.actionInsertBulletedList->setEnabled(false);
    ui.actionInsertNumberedList->setEnabled(false);
    ui.actionRemoveFormatting->setEnabled(false);
    ui.actionRemoveTagPair->setEnabled(false);
    ui.menuHeadings->setEnabled(false);
    ui.actionHeading1->setEnabled(false);
    ui.actionHeading2->setEnabled(false);
    ui.actionHeading3->setEnabled(false);
    ui.actionHeading4->setEnabled(false);
    ui.actionHeading5->setEnabled(false);
    ui.actionHeading6->setEnabled(false);
    ui.actionHeadingNormal->setEnabled(false);
    ui.actionCasingLowercase  ->setEnabled(true);
    ui.actionCasingUppercase  ->setEnabled(true);
    ui.actionCasingTitlecase ->setEnabled(true);
    ui.actionCasingCapitalize ->setEnabled(true);
    ui.actionFind->setEnabled(true);
    ui.actionFindNext->setEnabled(true);
    ui.actionFindPrevious->setEnabled(true);
    ui.actionReplaceCurrent->setEnabled(true);
    ui.actionReplaceNext->setEnabled(true);
    ui.actionReplacePrevious->setEnabled(true);
    ui.actionReplaceAll->setEnabled(true);
    ui.actionCount->setEnabled(true);
    ui.actionMarkSelection->setEnabled(true);
    ui.menuSearchCurrentFile->setEnabled(true);
    ui.actionFindNextInFile->setEnabled(true);
    ui.actionReplaceNextInFile->setEnabled(true);
    ui.actionReplaceAllInFile->setEnabled(true);
    ui.actionCountInFile->setEnabled(true);
    ui.actionGoToLine->setEnabled(true);
    ui.actionGoToLinkOrStyle->setEnabled(false);
    ui.actionAddMisspelledWord->setEnabled(false);
    ui.actionIgnoreMisspelledWord->setEnabled(false);
    ui.actionAutoSpellCheck->setEnabled(false);
    UpdateUIOnTabChanges();
    m_FindReplace->ShowHide();
}

void MainWindow::SetStateActionsStaticView()
{
    ui.actionPrintPreview->setEnabled(true);
    ui.actionPrint->setEnabled(true);
    ui.actionSplitSection->setEnabled(false);
    ui.actionInsertSGFSectionMarker->setEnabled(false);
    ui.actionInsertFile->setEnabled(false);
    ui.actionInsertSpecialCharacter->setEnabled(false);
    ui.actionInsertId->setEnabled(false);
    ui.actionInsertHyperlink->setEnabled(false);
    ui.actionInsertClosingTag->setEnabled(false);
    ui.actionUndo->setEnabled(false);
    ui.actionRedo->setEnabled(false);
    ui.actionPasteClipboardHistory->setEnabled(false);
    ui.actionBold         ->setEnabled(false);
    ui.actionItalic       ->setEnabled(false);
    ui.actionUnderline    ->setEnabled(false);
    ui.actionStrikethrough->setEnabled(false);
    ui.actionSubscript    ->setEnabled(false);
    ui.actionSuperscript  ->setEnabled(false);
    ui.actionAlignLeft   ->setEnabled(false);
    ui.actionAlignCenter ->setEnabled(false);
    ui.actionAlignRight  ->setEnabled(false);
    ui.actionAlignJustify->setEnabled(false);
    ui.actionDecreaseIndent->setEnabled(false);
    ui.actionIncreaseIndent->setEnabled(false);
    ui.actionTextDirectionLTR    ->setEnabled(false);
    ui.actionTextDirectionRTL    ->setEnabled(false);
    ui.actionTextDirectionDefault->setEnabled(false);
    ui.actionInsertBulletedList->setEnabled(false);
    ui.actionInsertNumberedList->setEnabled(false);
    ui.actionRemoveFormatting->setEnabled(false);
    ui.actionRemoveTagPair->setEnabled(false);
    ui.menuHeadings->setEnabled(false);
    ui.actionHeading1->setEnabled(false);
    ui.actionHeading2->setEnabled(false);
    ui.actionHeading3->setEnabled(false);
    ui.actionHeading4->setEnabled(false);
    ui.actionHeading5->setEnabled(false);
    ui.actionHeading6->setEnabled(false);
    ui.actionHeadingNormal->setEnabled(false);
    ui.actionCasingLowercase  ->setEnabled(false);
    ui.actionCasingUppercase  ->setEnabled(false);
    ui.actionCasingTitlecase ->setEnabled(false);
    ui.actionCasingCapitalize ->setEnabled(false);
    ui.actionFind->setEnabled(false);
    ui.actionFindNext->setEnabled(false);
    ui.actionFindPrevious->setEnabled(false);
    ui.actionReplaceCurrent->setEnabled(false);
    ui.actionReplaceNext->setEnabled(false);
    ui.actionReplacePrevious->setEnabled(false);
    ui.actionReplaceAll->setEnabled(false);
    ui.actionCount->setEnabled(false);
    ui.actionMarkSelection->setEnabled(false);
    ui.menuSearchCurrentFile->setEnabled(false);
    ui.actionFindNextInFile->setEnabled(false);
    ui.actionReplaceNextInFile->setEnabled(false);
    ui.actionReplaceAllInFile->setEnabled(false);
    ui.actionCountInFile->setEnabled(false);
    ui.actionGoToLine->setEnabled(false);
    ui.actionGoToLinkOrStyle->setEnabled(false);
    ui.actionAddMisspelledWord->setEnabled(false);
    ui.actionIgnoreMisspelledWord->setEnabled(false);
    ui.actionAutoSpellCheck->setEnabled(false);
    UpdateUIOnTabChanges();
    // Only hide window, don't save closed state since its temporary
    m_FindReplace->hide();
}

void MainWindow::SetupPreviewTimer()
{
    m_PreviewTimer.setSingleShot(true);
    m_PreviewTimer.setInterval(1000);
    connect(&m_PreviewTimer, SIGNAL(timeout()), this, SLOT(UpdatePreview()));
    m_PreviewTimer.stop();
}

void MainWindow::UpdatePreviewRequest()
{
    DBG qDebug() << "UpdatePreviewRequest has started its timer";
    if (m_PreviewTimer.isActive()) {
        m_PreviewTimer.stop();
    }
    m_PreviewTimer.start();
}

void MainWindow::UpdatePreviewCSSRequest()
{
    m_SaveCSS = true;
    UpdatePreviewRequest();
}


void MainWindow::ScrollPreview()
{
    DBG qDebug() << "in ScrollPreview called from FlowTab";
    QList<ElementIndex> location;
    HTMLResource *html_resource;

    ContentTab *tab = GetCurrentContentTab();
    if (tab != NULL) {
        html_resource = qobject_cast<HTMLResource *>(tab->GetLoadedResource());
        if (html_resource) {
            FlowTab *flow_tab = qobject_cast<FlowTab *>(tab);
            if (flow_tab) {
                // Make sure the document is loaded.  As soon as the views are created
                // signals are sent that it has changed which requests Preview to update
                // so these need to be ignored.  Once the document is loaded it signals again.
                if (!flow_tab->IsLoadingFinished()) {
                    return;
                }
                location = flow_tab->GetCaretLocation();
                m_PreviewWindow->ScrollTo(location);
            }
        }
    }
}


void MainWindow::UpdatePreview()
{

    if (m_IsClosing) return;

    m_PreviewTimer.stop();

    DBG qDebug() << "MW: UpdatePreview()";

    QString text;
    QList<ElementIndex> location;
    HTMLResource *html_resource;

    ContentTab *tab = GetCurrentContentTab();
    if (tab != NULL) {

        // Save CSS if update requested from CSS tab
        if (m_SaveCSS) {
            m_SaveCSS = false;
            tab->SaveTabContent();
        }

        html_resource = qobject_cast<HTMLResource *>(tab->GetLoadedResource());

        // handle any memory cache clearing inside Preview

        // handles all cases of non-html resource in front tab
        if (!html_resource) {
            DBG qDebug() << "MW: UpdatePreview to non-html resource, using Previous";
            // note: must handle case of m_PreviousHTMLResource being deleted by user
            // see RemoveResources()
            html_resource = m_PreviousHTMLResource;
        } else {
            m_PreviousHTMLResource = NULL;
        }

        if (html_resource) {
            FlowTab *flow_tab = qobject_cast<FlowTab *>(tab);
            if (flow_tab) {
                // Make sure the document is loaded.  As soon as the views are created
                // signals are sent that it has changed which requests Preview to update
                // so these need to be ignored.  Once the document is loaded it signals again.
                if (!flow_tab->IsLoadingFinished()) {
                    DBG qDebug() << "Flow Tab Is Loading Finished returned false";
                    return;
                }
                text = flow_tab->GetText();
                location = flow_tab->GetCaretLocation();
                DBG qDebug() << "MW: UpdatePreview using flow_tab Caret Location";
            } else {
                text = m_PreviousHTMLText;
                if (m_PreviousHTMLResource) {
                    location = m_PreviewWindow->GetCaretLocation();
                    DBG qDebug() << "MW: UpdatePreview using current PreviewWindow location";
                } else {
                    location = m_PreviousHTMLLocation;
                    DBG qDebug() << "MW: UpdatePreview using m_PreviousHTMLLocation location";
                }

            }
            m_PreviousHTMLResource = html_resource;
            m_PreviousHTMLText = text;
            m_PreviousHTMLLocation = location;

            bool res = m_PreviewWindow->UpdatePage(html_resource->GetFullPath(), text, location);
            if (!res) {
                m_PreviewTimer.start();
            }
        }
    }
}

void MainWindow::InspectHTML()
{
    m_PreviewWindow->show();
    m_PreviewWindow->raise();
    UpdatePreview();
}

void MainWindow::UpdateCursorPositionLabel(int line, int column)
{
    if (line > 0 && column > 0) {
        const QString l = QString::number(line);
        const QString c = QString::number(column);
        m_lbCursorPosition->setText(tr("Line: %1, Col: %2").arg(l).arg(c));
        m_lbCursorPosition->show();
    } else {
        m_lbCursorPosition->clear();
        m_lbCursorPosition->hide();
    }
}

void MainWindow::SliderZoom(int slider_value)
{
    ContentTab *tab = m_TabManager->GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    float new_zoom_factor     = SliderRangeToZoomFactor(slider_value);
    float current_zoom_factor = GetZoomFactor();

    // We try to prevent infinite loops...
    if (!qFuzzyCompare(new_zoom_factor, current_zoom_factor)) {
        ZoomByFactor(new_zoom_factor);
    }
}


void MainWindow::UpdateZoomSlider(float new_zoom_factor)
{
    m_slZoomSlider->setValue(ZoomFactorToSliderRange(new_zoom_factor));
}


void MainWindow::UpdateZoomLabel(int slider_value)
{
    float zoom_factor = SliderRangeToZoomFactor(slider_value);
    UpdateZoomLabel(zoom_factor);
}


void MainWindow::SetAutoSpellCheck(bool new_state)
{
    SettingsStore settings;
    settings.setSpellCheck(new_state);
    emit SettingsChanged();
}

bool MainWindow::MendPrettifyHTML()
{
    m_Book->ReformatAllHTML(false);
    return true;
}

bool MainWindow::MendHTML()
{
    m_Book->ReformatAllHTML(true);
    return true;
}

void MainWindow::ClearIgnoredWords()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    SpellCheck *sc = SpellCheck::instance();
    sc->clearIgnoredWords();
    // Need to reload any tabs to force spelling to be run again in CodeView
    RefreshSpellingHighlighting();
    QApplication::restoreOverrideCursor();
}

void MainWindow::RefreshSpellingHighlighting()
{
    QList<ContentTab *> content_tabs = m_TabManager->GetContentTabs();
    foreach(ContentTab *content_tab, content_tabs) {
        FlowTab *flow_tab = qobject_cast<FlowTab *>(content_tab);

        if (flow_tab) {
            flow_tab->RefreshSpellingHighlighting();
        }
    }
}

void MainWindow::UpdateZoomLabel(float new_zoom_factor)
{
    m_lbZoomLabel->setText(QString("%1% ").arg(qRound(new_zoom_factor * 100)));
}

void MainWindow::CreateSectionBreakOldTab(QString content, HTMLResource *originating_resource)
{
    if (content.isEmpty()) {
        QMessageBox::warning(this, tr("Sigil"), tr("File cannot be split at this position."));
        return;
    }

    // The test for nav is done in FlowTab::SplitSection via a signal OldTabRequest
    //  so it need not be repeated.
    // The xml file test can not be done here as it is already too late for the
    // originating tab. Since this comes from a FlowTab assume that it is 
    // an xhtml file even if called .xml to avoid doing damage.

#if 0
    // XXX: This should be using the mime type not the extension.
    if (!TEXT_EXTENSIONS.contains(QFileInfo(originating_resource->Filename()).suffix().toLower())) {
        QMessageBox::warning(this, tr("Sigil"), tr("Cannot split since it may not be an HTML file."));
        return;
    }

    HTMLResource * nav_resource = m_Book->GetConstOPF()->GetNavResource();
    if (nav_resource && nav_resource == originating_resource) {
        QMessageBox::warning(this, tr("Sigil"), tr("The Nav file cannot be split."));
        return;
    }
#endif

    HTMLResource *html_resource = m_Book->CreateSectionBreakOriginalResource(content, originating_resource);
    m_BookBrowser->Refresh();
    // Open the old shortened content in a new tab preceding the current one.
    // without grabbing focus
    OpenResource(html_resource, -1, -1, QString(), QUrl(), true);
    FlowTab *flow_tab = GetCurrentFlowTab();

    // We will reload the reduced content tab to ensure reflects updated resource.
    if (flow_tab) {
        flow_tab->LoadTabContent();
        flow_tab->ScrollToTop();
    }

    ShowMessageOnStatusBar(tr("Split completed."));
}

bool MainWindow::SplitOnSGFSectionMarkers()
{
    QList<Resource *> html_resources = GetAllHTMLResources();
    Resource * nav_resource = m_Book->GetConstOPF()->GetNavResource();
    if (nav_resource) {
        html_resources.removeOne(nav_resource);
    }

    SaveTabData();

    bool done_checking_frags = false;
    bool ignore_frags = false;
    foreach(Resource * resource, html_resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (!html_resource) {
            QMessageBox::warning(this, tr("Sigil"), tr("Cannot split since at least one file is not an HTML file."));
            return false;
        }

        // Check if data is well formed before splitting.
        if (!html_resource->FileIsWellFormed()) {
            QMessageBox::warning(this, tr("Sigil"), tr("Cannot split: %1 XML is not well formed").arg(html_resource->ShortPathName()));
            return false;
        }

        // XXX: This should be using the mime type not the extension.
        if (!TEXT_EXTENSIONS.contains(QFileInfo(html_resource->Filename()).suffix().toLower())) {
            QMessageBox::warning(this, tr("Sigil"), tr("Cannot split since at least one file may not be an HTML file."));
            return false;
        }

        // Handle warning the user about undefined url fragments.
        if (!done_checking_frags) {
            done_checking_frags = true;
            ignore_frags = ProceedWithUndefinedUrlFragments();
            if (!ignore_frags) {
                break;
            }
        }
    }

    if (!ignore_frags) {
        return false;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QList<Resource *> changed_resources;
    foreach(Resource * resource, html_resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        QStringList new_sections = html_resource->SplitOnSGFSectionMarkers();

        if (!new_sections.isEmpty()) {
            m_Book->CreateNewSections(new_sections, html_resource);
            changed_resources.append(resource);
        }
    }

    if (changed_resources.count() > 0) {
        m_TabManager->ReloadTabDataForResources(changed_resources);
        m_BookBrowser->Refresh();
        ShowMessageOnStatusBar(tr("Split completed. You may need to update the Table of Contents."));
    } else {
        ShowMessageOnStatusBar(tr("No split file markers found. Use Insert->Split Marker."));
    }

    QApplication::restoreOverrideCursor();
    return true;
}

void MainWindow::ShowPasteClipboardHistoryDialog()
{
    // We only want to show the dialog if focus is in a control that can accept its content.
    if (m_LastPasteTarget == NULL) {
        return;
    }

    m_ClipboardHistorySelector->exec();
}

// Change the selected/highlighted resource to match the current tab
void MainWindow::UpdateBrowserSelectionToTab()
{
    ContentTab *tab = m_TabManager->GetCurrentContentTab();

    if (tab != NULL) {
        m_BookBrowser->UpdateSelection(tab->GetLoadedResource());
    }
}


void MainWindow::ReadSettings()
{
    DWINGEO qDebug() << "------";
    DWINGEO qDebug() << "In ReadSettings";
    SettingsStore settings;
    ui.actionAutoSpellCheck->setChecked(settings.spellCheck());
    emit SettingsChanged();
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and its full screen status
    // Due to the 4.8 bug, we restore its "normal" window size and then maximize
    // it afterwards (if last state was maximized) to ensure on correct screen.
    bool MaximizedState = settings.value("maximized", false).toBool();
    bool FullScreenState = settings.value("fullscreen", false).toBool();

    m_LastWindowSize = settings.value("geometry",QByteArray()).toByteArray();

    // we should probably not restore geometry of a maximized window here
    // since it would restore the normal geometry

    if (!MaximizedState && !FullScreenState) {
        if (!m_LastWindowSize.isEmpty()) restoreGeometry(m_LastWindowSize);
    } 

    if (MaximizedState) {
        QRect maxsize = settings.value("max_mw_geometry", QApplication::desktop()->availableGeometry(this)).toRect();
        setGeometry(maxsize);
        setWindowState(windowState() | Qt::WindowMaximized);
    } else if (FullScreenState) {
        QRect maxsize = settings.value("max_mw_geometry", QApplication::desktop()->screenGeometry(this)).toRect();
        setGeometry(maxsize);
        setWindowState(windowState() | Qt::WindowFullScreen);
    }

    DWINGEO qDebug() << "------";
    DWINGEO qDebug() << "In ReadSettings before restoreState";

    DWINGEO DebugCurrentWidgetSizes();

    // The positions of all the toolbars and dock widgets
    // The dockwidgets  will only "restore" properly if the widget already
    // has the proper geometry to match what was saved and has been
    // properly resized (see QTBUG-46620 and QTBUG-16252)
    // So delay restore until the first time the widget is made active
    m_LastState = settings.value("toolbars",QByteArray()).toByteArray();

#ifdef Q_OS_MAC
    // Work around saved state restore bug with Find and Replace on macOS
    m_FRVisible = settings.value("frvisible", false).toBool();
#endif

    // The last folder used for saving and opening files
    m_LastFolderOpen  = settings.value("lastfolderopen", QDir::homePath()).toString();
    // The list of recent files
    s_RecentFiles    = settings.value("recentfiles").toStringList();
    m_preserveHeadingAttributes = settings.value("preserveheadingattributes", true).toBool();
    SetPreserveHeadingAttributes(m_preserveHeadingAttributes);
    const QStringList clipboardHistory = settings.value("clipboardringhistory").toStringList();
    m_ClipboardHistorySelector->LoadClipboardHistory(clipboardHistory);
    settings.endGroup();
    m_ClipboardHistoryLimit = settings.clipboardHistoryLimit();

    // Our default fonts for Preview
    SettingsStore::PreviewAppearance PVAppearance = settings.previewAppearance();
    QWebEngineSettings *web_settings = QWebEngineSettings::defaultSettings();

    // Default QWebEngine security settings to help prevent rogue epub3 javascripts
    // User preferences control if javascript is allowed (on) or not for Preview
    web_settings->setAttribute(QWebEngineSettings::AutoLoadImages, true);
    web_settings->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    web_settings->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    web_settings->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    web_settings->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    web_settings->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    web_settings->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, true);
    web_settings->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);
    web_settings->setAttribute(QWebEngineSettings::XSSAuditingEnabled, true);
    web_settings->setAttribute(QWebEngineSettings::AllowGeolocationOnInsecureOrigins, false);
    web_settings->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, false);
    web_settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    web_settings->setAttribute(QWebEngineSettings::AllowWindowActivationFromJavaScript, false);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    web_settings->setUnknownUrlSchemePolicy(QWebEngineSettings::DisallowUnknownUrlSchemes);
    web_settings->setAttribute(QWebEngineSettings::PlaybackRequiresUserGesture, true);
    web_settings->setAttribute(QWebEngineSettings::JavascriptCanPaste, false);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    web_settings->setAttribute(QWebEngineSettings::DnsPrefetchEnabled, false);
#endif

    web_settings->setFontSize(QWebEngineSettings::DefaultFontSize, PVAppearance.font_size);
    web_settings->setFontFamily(QWebEngineSettings::StandardFont, PVAppearance.font_family_standard);
    web_settings->setFontFamily(QWebEngineSettings::SerifFont, PVAppearance.font_family_serif);
    web_settings->setFontFamily(QWebEngineSettings::SansSerifFont, PVAppearance.font_family_sans_serif);

    // Check for existing custom Preview stylesheet in Prefs dir and tell Preview about it
    QFileInfo CustomPreviewStylesheetInfo(QDir(Utility::DefinePrefsDir()).filePath(CUSTOM_PREVIEW_STYLE_FILENAME));
    if (CustomPreviewStylesheetInfo.exists() && 
        CustomPreviewStylesheetInfo.isFile() && 
        CustomPreviewStylesheetInfo.isReadable()) {
        QString usercssurl = QUrl::fromLocalFile(CustomPreviewStylesheetInfo.absoluteFilePath()).toString();
        m_PreviewWindow->setUserCSSURL(usercssurl);
    }

    // Determine MathJax location and tell Preview about it
    // The path to MathJax.js is platform dependent
    QString mathjaxurl;
#ifdef Q_OS_MAC
    // On Mac OS X QCoreApplication::applicationDirPath() points to Sigil.app/Contents/MacOS/ 
    QDir execdir(QCoreApplication::applicationDirPath());
    execdir.cdUp();
    mathjaxurl = execdir.absolutePath() + "/polyfills/MJ/";
#elif defined(Q_OS_WIN32)
    mathjaxurl = QCoreApplication::applicationDirPath() + "/polyfills/MJ/";
#else
    // all flavours of linux / unix
    // First check if system MathJax was configured to be used at compile time
    if (!mathjax_dir.isEmpty()) {
        mathjaxurl = mathjax_dir;
        if (!mathjaxurl.endsWith('/')) {
            mathjaxurl.append('/');
        }
    } else {
        // otherwise user supplied environment variable to 'share/sigil'
        // takes precedence over Sigil's usual share location.
        if (!sigil_extra_root.isEmpty()) {
            mathjaxurl = sigil_extra_root + "/polyfills/MJ/";
        } else {
            mathjaxurl = sigil_share_root + "/polyfills/MJ/";
        }
    }
#endif
    m_mathjaxfolder = mathjaxurl;
    mathjaxurl = mathjaxurl + "MathJax.js";
    mathjaxurl = QUrl::fromLocalFile(mathjaxurl).toString();
    mathjaxurl = mathjaxurl + "?config=local/SIGIL_EBOOK_MML_SVG";
    m_PreviewWindow->setMathJaxURL(mathjaxurl);
}


void MainWindow::WriteSettings()
{
    DWINGEO qDebug() << "------";
    DWINGEO qDebug() << "In WriteSettings";

    // disable recording any last sizes as exiting
    m_SaveLastEnabled = false;

    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    // This is a workaround for this bug:
    // Maximizing Sigil and then closing will forget the previous window size
    // and open it maximized on the wrong screen.
    // https://bugreports.qt-project.org/browse/QTBUG-21371
    settings.setValue("maximized", isMaximized());
    settings.setValue("fullscreen",isFullScreen());

#ifdef Q_OS_MAC
    // work around Find Replace saved state restore bug on macOS
    settings.setValue("frvisible",m_FindReplace->isVisible());
    if (m_FindReplace->isVisible()) {
        // can not use just hide() and FR keeps its own internal state
        m_FindReplace->HideFindReplace();
    }
#endif

    DBG DebugCurrentWidgetSizes();

    // if currently not maximized and not full screen, just save what we have now
    if (!isMaximized() && !isFullScreen()) {
        settings.setValue("geometry", saveGeometry());
    } else {
        settings.setValue("geometry", m_LastWindowSize);
    }

    // in case this is needed for helping to determine when last normal sizes
    if (isMaximized() || isFullScreen() ) {
        settings.setValue("max_mw_geometry", geometry());
    }

    // The positions of all the toolbars and dock widgets
    settings.setValue("toolbars", saveState());

    // The last folders used for saving and opening files
    settings.setValue("lastfolderopen",  m_LastFolderOpen);
    // The list of recent files
    settings.setValue("recentfiles", s_RecentFiles);
    settings.setValue("preserveheadingattributes", m_preserveHeadingAttributes);
    settings.setValue("clipboardringhistory", m_ClipboardHistorySelector->GetClipboardHistory(m_ClipboardHistoryLimit));
    KeyboardShortcutManager::instance()->writeSettings();
    settings.endGroup();
    settings.setClipboardHistoryLimit(m_ClipboardHistoryLimit);
}

bool MainWindow::MaybeSaveDialogSaysProceed()
{
    // Make sure that any tabs currently about to be drawn etc get a chance to do so.
    // or else the process of closing/creating a new book will crash with Qt object errors.
    // Particularly a problem if open a large tab in Preview prior to the action
    // due to QWebInspector, QTimer timeouts etc
    qApp->processEvents();

    if (isWindowModified()) {
        QMessageBox::StandardButton button_pressed;
        button_pressed = QMessageBox::warning(this,
                                              tr("Sigil"),
                                              tr("The document has been modified.\n"
                                                      "Do you want to save your changes?"),
                                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
                                             );

        if (button_pressed == QMessageBox::Save) {
            return Save();
        } else if (button_pressed == QMessageBox::Cancel) {
            return false;
        }
    }
    return true;
}


bool MainWindow::ProceedToOverwrite(const QString& msg, const QString &filename)
{
    QMessageBox::StandardButton button_pressed;
    button_pressed = QMessageBox::warning(this,
                                          tr("Sigil"),
                                          msg + "\n" + filename + "\n\n" +
                                          tr("Should Sigil overwrite this file?"),
                                          QMessageBox::Yes | QMessageBox::No);
    if (button_pressed == QMessageBox::Yes) return true;
    return false;
}

void MainWindow::SetNewBook(QSharedPointer<Book> new_book)
{
    m_TabManager->CloseOtherTabs();
    m_TabManager->CloseAllTabs(true);
    m_Book = new_book;
    m_BookBrowser->SetBook(m_Book);
    m_TableOfContents->SetBook(m_Book);
    m_ValidationResultsView->SetBook(m_Book);
    m_IndexEditor->SetBook(m_Book);
    m_ClipEditor->SetBook(m_Book);
    m_SpellcheckEditor->SetBook(m_Book);
    SpellCheck *sc = SpellCheck::instance();
    sc->clearIgnoredWords();
    ResetLinkOrStyleBookmark();
    SettingsStore settings;
    settings.setRenameTemplate("");
    connect(m_Book.data(),     SIGNAL(ModifiedStateChanged(bool)), this, SLOT(setWindowModified(bool)));
    connect(m_Book.data(),     SIGNAL(ResourceUpdatedFromDiskRequest(Resource *)), this, SLOT(ResourceUpdatedFromDisk(Resource *)));
    connect(m_BookBrowser,     SIGNAL(ShowStatusMessageRequest(const QString &, int)), this, SLOT(ShowMessageOnStatusBar(const QString &, int)));
    connect(m_BookBrowser,     SIGNAL(ResourcesDeleted()), this, SLOT(ResourcesAddedOrDeletedOrMoved()));
    connect(m_BookBrowser,     SIGNAL(ResourcesAdded()), this, SLOT(ResourcesAddedOrDeletedOrMoved()));
    connect(m_BookBrowser,     SIGNAL(ResourcesMoved()), this, SLOT(ResourcesAddedOrDeletedOrMoved()));
}

void MainWindow::ResourcesAddedOrDeletedOrMoved()
{
    // MainWindow::clearMemoryCaches();
    m_Book->GetFolderKeeper()->RefreshGroupFolders();

    // Make sure currently visible tab is updated immediately
    FlowTab *flow_tab = GetCurrentFlowTab();
    if (flow_tab) {
        flow_tab->LoadTabContent();
    }
}


void MainWindow::CreateNewBook(const QString version, const QStringList &book_paths)
{
    QString epubversion = version;
    if (epubversion.isEmpty()) {
        SettingsStore ss;
        epubversion = ss.defaultVersion();
    }

    QStringList bookpaths(book_paths);

    if (bookpaths.isEmpty()) {
        // Check to see if a default empty epub layout already exists
        // and if so use that in place of the standard one
        QString empty_epub_ini_path = Utility::DefinePrefsDir() + "/" + "sigil_empty_epub.ini";
        if (QFile::exists(empty_epub_ini_path)) {
            SettingsStore ss(empty_epub_ini_path);
            const QString SETTINGS_GROUP = "bookpaths";
            const QString KEY_BOOKPATHS = SETTINGS_GROUP + "/" + "empty_epub_bookpaths";
            while (!ss.group().isEmpty()) {
                ss.endGroup();
            }
            bookpaths = ss.value(KEY_BOOKPATHS,QStringList()).toStringList();
        }
    }

    bool is_valid = false;

    if (!bookpaths.isEmpty()) {

        // Check if minimally valid
        bool hasOPF = false;   bool hasNCX = false;
        bool hasNAV = false;   bool hasTEXT = false;
        bool hasSTYLE = false;
        foreach(QString bkpath, bookpaths) {
            if (bkpath.endsWith(".opf")) hasOPF = true;
            if (bkpath.endsWith(".ncx")) hasNCX = true;
            if (bkpath.endsWith("marker.css")) hasSTYLE = true;
            if (bkpath.endsWith("marker.xhtml")) hasTEXT = true;
            if (bkpath.endsWith(".xhtml") && !bkpath.endsWith("marker.xhtml")) hasNAV = true;
        }
        if (epubversion.startsWith('3')) {
            is_valid = hasOPF && hasNAV && hasTEXT && hasSTYLE;
        } else {
            is_valid = hasOPF && hasNCX && hasTEXT;
        }
    }

    if (bookpaths.isEmpty() || !is_valid) {
        // these define the layout of a standard sigil epub
        bookpaths.clear();
        bookpaths << "OEBPS/content.opf"      << "OEBPS/Text/marker.xhtml" << "OEBPS/Styles/marker.css"
                  << "OEBPS/Fonts/marker.otf" << "OEBPS/Images/marker.jpg" << "OEBPS/Audio/marker.mp3"
                  << "OEBPS/Video/marker.mp4" << "OEBPS/Misc/marker.xml" << "OEBPS/toc.ncx";
        if (epubversion.startsWith('3')) {
            bookpaths << "OEBPS/Text/nav.xhtml" << "OEBPS/Misc/marker.js";
        }
    }

    // extract the information we need from the bookpaths
    QString opfbookpath;
    QString ncxbookpath;
    QString navfile;
    QString navdir;
    QStringList textdirs;
    QStringList mtypes;
    QStringList finalpaths;
    foreach(QString bkpath, bookpaths) {
        QString filename = bkpath.split("/").last();
        QString extension = filename.split(".").last();
        QString folder = Utility::startingDir(bkpath);
        QString mt = MediaTypes::instance()->GetMediaTypeFromExtension(extension);
        if (filename.endsWith(".opf")) opfbookpath = bkpath;
        if (filename.endsWith(".ncx")) ncxbookpath = bkpath;
        if (filename.endsWith("marker.xhtml")) textdirs << folder;
        if (filename.endsWith(".xhtml") && !filename.endsWith("marker.xhtml")) {
            navdir = folder;
            navfile = filename;
            continue;
        }
        mtypes << mt;
        finalpaths << bkpath;
    }
    QString first_textdir = textdirs.first();

    QSharedPointer<Book> new_book = QSharedPointer<Book>(new Book());
    // immediately after creating a new book, you must
    // add a proper OPF to it *before* doing anything else
    new_book->GetFolderKeeper()->AddOPFToFolder(epubversion, opfbookpath);

    // Set Group Folders from bookpaths
    new_book->GetFolderKeeper()->SetGroupFolders(finalpaths, mtypes);

    // create a single text file in each location
    foreach(QString textfolder, textdirs) {
        new_book->CreateEmptyHTMLFile(textfolder);
    }

    // handle nav / ncx
    if (epubversion.startsWith('3')) {
        HTMLResource * nav_resource = new_book->CreateEmptyNavFile(true, navdir, navfile, first_textdir);
        new_book->GetOPF()->SetNavResource(nav_resource);
        new_book->GetOPF()->SetItemRefLinear(nav_resource, false);
        // ncx is optional in epub3 so wait until user asks for it to be generated before creating it
        // if (!ncxbookpath.isEmpty()) {
        //     new_book->GetFolderKeeper()->AddNCXToFolder(version, ncxbookpath, first_textdir);
        //     NCXResource * ncxresource = new_book->GetNCX();
        //     QString NCXId = new_book->GetOPF()->AddNCXItem(ncxresource->GetFullPath(),"ncx");
        //     new_book->GetOPF()->UpdateNCXOnSpine(NCXId);
        // }
    } else {
        new_book->GetFolderKeeper()->AddNCXToFolder(epubversion, ncxbookpath, first_textdir);
        NCXResource * ncxresource = new_book->GetNCX();
        QString NCXId = new_book->GetOPF()->AddNCXItem(ncxresource->GetFullPath(),"ncx");
        new_book->GetOPF()->UpdateNCXOnSpine(NCXId);
    }
    SetNewBook(new_book);
    new_book->SetModified(false);
    m_SaveACopyFilename = "";
    UpdateUiWithCurrentFile("");
}


bool MainWindow::LoadFile(const QString &fullfilepath, bool is_internal)
{
    if (!Utility::IsFileReadable(fullfilepath)) {
        return false;
    }

    try {
        ImporterFactory importerFactory;
        // Create the new book, clean up the old one
        // (destructors take care of that)
        Importer *importer = importerFactory.GetImporter(fullfilepath);
        if (!importer) {
            throw tr("No importer for file type: %1").arg(QFileInfo(fullfilepath).suffix().toLower());
        }

        XhtmlDoc::WellFormedError error = importer->CheckValidToLoad();

        if (error.line != -1) {
            // Warn the user their content is invalid.
            Utility::DisplayStdErrorDialog(tr("The following file was not loaded due to invalid content or not well formed XML:\n\n%1 (line %2: %3)\n\nTry setting the Clean Source preference to Mend XHTML Source Code on Open and reloading the file.")
                                           .arg(QDir::toNativeSeparators(fullfilepath))
                                           .arg(error.line)
                                           .arg(error.message));
        } else {
            ShowMessageOnStatusBar(tr("Loading file..."), 0);
            m_Book->SetModified(false);
            SetNewBook(importer->GetBook());

            // The m_IsModified state variable is set in GetBook() to indicate whether the OPF
            // file was invalid and had to be recreated.
            // Since this happens before the connections have been established, it needs to be
            // tested and retoggled if true in order to indicate the actual state.
            if (m_Book->IsModified()) {
                m_Book->SetModified(false);
                m_Book->SetModified(true);
            }

            m_SaveACopyFilename = "";
            ShowMessageOnStatusBar(tr("File loaded."));

            // Get any warnings - if our main window is not currently visible they will be
            // shown when the window is displayed.
            m_LastOpenFileWarnings.append(importer->GetLoadWarnings());

            if (!m_IsInitialLoad) {
                ShowLastOpenFileWarnings();
            }

            if (!is_internal) {
                // Store the folder the user opened from
                m_LastFolderOpen = QFileInfo(fullfilepath).absolutePath();
                // Clear the last inserted file
                m_LastInsertedFile = "";
                UpdateUiWithCurrentFile(fullfilepath);
            } else {
                UpdateUiWithCurrentFile(QFileInfo(fullfilepath).fileName(), true);
                m_Book->SetModified();
            }

            return true;
        }
   } catch (FileEncryptedWithDrm&) {
       ShowMessageOnStatusBar();
       // ImportHTML/ImportEPUB use wait cursor and can throw exceptions caught here
       QApplication::restoreOverrideCursor();
       Utility::DisplayStdErrorDialog(
           tr("The creator of this file has encrypted it with DRM. "
              "Sigil cannot open such files."));
   } catch (EPUBLoadParseError& epub_load_error) {
       ShowMessageOnStatusBar();
       // ImportHTML/ImportEPUB use wait cursor and can throw exceptions caught here
       QApplication::restoreOverrideCursor();
       const QString errors = QString(epub_load_error.what());
       Utility::DisplayStdErrorDialog(
           tr("Cannot load EPUB: %1").arg(QDir::toNativeSeparators(fullfilepath)), errors);
   } catch (const std::runtime_error &e) {
       ShowMessageOnStatusBar();
       // ImportHTML/ImportEPUB use wait cursor and can throw exceptions caught here
       QApplication::restoreOverrideCursor();
       Utility::DisplayExceptionErrorDialog(tr("Cannot load file %1: %2")
                                             .arg(QDir::toNativeSeparators(fullfilepath))
                                             .arg(e.what()));
   } catch (QString& err) {
       ShowMessageOnStatusBar();
       // ImportHTML/ImportEPUB use wait cursor and can throw exceptions caught here
       QApplication::restoreOverrideCursor();
       Utility::DisplayStdErrorDialog(err);
   }

    // If we got to here some sort of error occurred while loading the file
    // and potentially has left the GUI in a nasty state (like on initial startup)
    // Fallback to displaying a new book instead so GUI integrity is maintained.
    CreateNewBook();
    return false;
}


void MainWindow::SetValidationResults(const QList<ValidationResult> &results)
{
    m_ValidationResultsView->LoadResults(results);
}


bool MainWindow::SaveFile(const QString &fullfilepath, bool update_current_filename)
{
    SettingsStore ss;

    try {
        ShowMessageOnStatusBar(tr("Saving EPUB..."), 0);
        SaveTabData();
        QString extension = QFileInfo(fullfilepath).suffix().toLower();

        // TODO: Move to ExporterFactory and throw exception
        // when the user tries to save an unsupported type
        if (!SUPPORTED_SAVE_TYPE.contains(extension)) {
            ShowMessageOnStatusBar();
            Utility::DisplayStdErrorDialog(
                tr("Sigil cannot save files of type \"%1\".\n"
                   "Please choose a different format.")
                .arg(extension)
            );
            return false;
        }

        QApplication::setOverrideCursor(Qt::WaitCursor);

        QList <HTMLResource *> broken_resources;
        bool not_well_formed = false;
        Q_FOREACH(Resource * r, GetAllHTMLResources()) {
            HTMLResource *t = qobject_cast<HTMLResource *>(r);
            if (t) {
                if (!XhtmlDoc::IsDataWellFormed(t->GetText())) {
                    not_well_formed = true;
                    broken_resources.append(t);
                }
            }
        }
        if (ss.cleanOn() & CLEANON_SAVE) {
            if (not_well_formed) {
                QApplication::restoreOverrideCursor();
                bool auto_fix = QMessageBox::Yes == QMessageBox::warning(this,
                                tr("Sigil"),
                                tr("This EPUB has HTML files that are not well formed and "
                                   "your current Clean Source preferences are set to automatically mend on Save. "
                                   "Saving a file that is not well formed will cause it to be automatically "
                                   "fixed, which very rarely may result in some data loss.\n\n"
                                   "Do you want to automatically mend the files before saving?"),
                                QMessageBox::Yes|QMessageBox::No);
                QApplication::setOverrideCursor(Qt::WaitCursor);
                if (auto_fix) {
                    CleanSource::ReformatAll(broken_resources, CleanSource::Mend);
                    not_well_formed = false;
                }
            }
        }

        ExporterFactory().GetExporter(fullfilepath, m_Book)->WriteBook();

        // Return the focus back to the current tab
        ContentTab *tab = GetCurrentContentTab();

        if (tab != NULL) {
            tab->setFocus();
        }

        if (update_current_filename) {
            m_Book->SetModified(false);
            UpdateUiWithCurrentFile(fullfilepath);
        }

        if (not_well_formed) {
            ShowMessageOnStatusBar(tr("EPUB saved, but not all HTML files are well formed."));
        } else {
            ShowMessageOnStatusBar(tr("EPUB saved."));
        }
        QApplication::restoreOverrideCursor();
    } catch (std::runtime_error &e) {
        ShowMessageOnStatusBar();
        QApplication::restoreOverrideCursor();
        Utility::DisplayExceptionErrorDialog(tr("Cannot save file %1: %2").arg(fullfilepath).arg(e.what()));
        return false;
    }

    return true;
}


void MainWindow::ZoomByStep(bool zoom_in)
{
    ContentTab *tab = m_TabManager->GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    // We use a negative zoom stepping if we are zooming *out*
    float zoom_stepping       = zoom_in ? ZOOM_STEP : - ZOOM_STEP;
    // If we are zooming in, we round UP;
    // on zoom out, we round DOWN.
    float rounding_helper     = zoom_in ? 0.05f : - 0.05f;
    float current_zoom_factor = GetZoomFactor();
    float rounded_zoom_factor = Utility::RoundToOneDecimal(current_zoom_factor + rounding_helper);

    // If the rounded value is nearly the same as the original value,
    // then the original was rounded to begin with and so we
    // add the zoom increment
    if (qAbs(current_zoom_factor - rounded_zoom_factor) < 0.01f) {
        ZoomByFactor(Utility::RoundToOneDecimal(current_zoom_factor + zoom_stepping));
    }
    // ...otherwise we first zoom to the rounded value
    else {
        ZoomByFactor(rounded_zoom_factor);
    }
}


void MainWindow::ZoomByFactor(float new_zoom_factor)
{
    ContentTab *tab = m_TabManager->GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    if (new_zoom_factor > ZOOM_MAX || new_zoom_factor < ZOOM_MIN) {
        return;
    }

    if (m_ZoomPreview && m_PreviewWindow->IsVisible()) {
        m_PreviewWindow->SetZoomFactor(new_zoom_factor);
    } else {
        tab->SetZoomFactor(new_zoom_factor);
    }
}

float MainWindow::GetZoomFactor()
{
    if (m_ZoomPreview && m_PreviewWindow->IsVisible()) {
        return m_PreviewWindow->GetZoomFactor();
    } else {
        ContentTab *tab = m_TabManager->GetCurrentContentTab();

        if (tab != NULL) {
            return tab->GetZoomFactor();
        }
    }

    return 1;
}


int MainWindow::ZoomFactorToSliderRange(float zoom_factor)
{
    // We want a precise value for the 100% zoom,
    // so we pick up all float values near it.
    if (qFuzzyCompare(zoom_factor, ZOOM_NORMAL)) {
        return ZOOM_SLIDER_MIDDLE;
    }

    // We actually use two ranges: one for the below 100% zoom,
    // and one for the above 100%. This is so that the 100% mark
    // rests in the middle of the slider.
    if (zoom_factor < ZOOM_NORMAL) {
        double range            = ZOOM_NORMAL - ZOOM_MIN;
        double normalized_value = zoom_factor - ZOOM_MIN;
        double range_proportion = normalized_value / range;
        return ZOOM_SLIDER_MIN + qRound(range_proportion * (ZOOM_SLIDER_MIDDLE - ZOOM_SLIDER_MIN));
    } else {
        double range            = ZOOM_MAX - ZOOM_NORMAL;
        double normalized_value = zoom_factor - ZOOM_NORMAL;
        double range_proportion = normalized_value / range;
        return ZOOM_SLIDER_MIDDLE + qRound(range_proportion * ZOOM_SLIDER_MIDDLE);
    }
}


float MainWindow::SliderRangeToZoomFactor(int slider_range_value)
{
    // We want a precise value for the 100% zoom
    if (slider_range_value == ZOOM_SLIDER_MIDDLE) {
        return ZOOM_NORMAL;
    }

    // We actually use two ranges: one for the below 100% zoom,
    // and one for the above 100%. This is so that the 100% mark
    // rests in the middle of the slider.
    if (slider_range_value < ZOOM_SLIDER_MIDDLE) {
        double range            = ZOOM_SLIDER_MIDDLE - ZOOM_SLIDER_MIN;
        double normalized_value = slider_range_value - ZOOM_SLIDER_MIN;
        double range_proportion = normalized_value / range;
        return ZOOM_MIN + range_proportion * (ZOOM_NORMAL - ZOOM_MIN);
    } else {
        double range            = ZOOM_SLIDER_MAX - ZOOM_SLIDER_MIDDLE;
        double normalized_value = slider_range_value - ZOOM_SLIDER_MIDDLE;
        double range_proportion = normalized_value / range;
        return ZOOM_NORMAL + range_proportion * (ZOOM_MAX - ZOOM_NORMAL);
    }
}

void MainWindow::SetInsertedFileWatchResourceFile(const QString &bookpath)
{
    try {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByBookPath(bookpath);
        m_Book->GetFolderKeeper()->WatchResourceFile(resource);
    } catch (ResourceDoesNotExist&) {
        // nothing
    }
}

const QMap<QString, QString> MainWindow::GetLoadFiltersMap()
{
    QMap<QString, QString> file_filters;
    file_filters[ "epub"  ] = tr("EPUB files (*.epub)");
    file_filters[ "htm"   ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "html"  ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "xhtml" ] = tr("HTML files (*.htm *.html *.xhtml)");
    file_filters[ "txt"   ] = tr("Text files (*.txt)");
    file_filters[ "*"     ] = tr("All files (*.*)");
    return file_filters;
}


const QMap<QString, QString> MainWindow::GetSaveFiltersMap()
{
    QMap<QString, QString> file_filters;
    file_filters[ "epub" ] = tr("EPUB file (*.epub)");
    return file_filters;
}


void MainWindow::UpdateUiWithCurrentFile(const QString &fullfilepath, bool just_name)
{
    if (just_name) {
        m_CurrentFilePath = "";
        m_CurrentFileName = fullfilepath;
    } else {
        m_CurrentFilePath = fullfilepath;
        m_CurrentFileName = m_CurrentFilePath.isEmpty() ? DEFAULT_FILENAME : QFileInfo(m_CurrentFilePath).fileName();
    }
    QString epubversion = m_Book->GetConstOPF()->GetEpubVersion();

    // Update the titlebar
    setWindowTitle(tr("%1[*] - epub%2 - %3").arg(m_CurrentFileName).arg(epubversion).arg(tr("Sigil")));

    if (m_CurrentFilePath.isEmpty()) {
        return;
    }

    // Update recent files actions
    const QString nativeFilePath = QDir::toNativeSeparators(m_CurrentFilePath);
    s_RecentFiles.removeAll(nativeFilePath);
    s_RecentFiles.prepend(nativeFilePath);

    while (s_RecentFiles.size() > MAX_RECENT_FILES) {
        s_RecentFiles.removeLast();
    }

    // Update the recent files actions on
    // ALL the main windows
    foreach(QWidget * window, QApplication::topLevelWidgets()) {
        if (MainWindow *mainWin = qobject_cast<MainWindow *>(window)) {
            mainWin->UpdateRecentFileActions();
        }
    }
}

void MainWindow::SelectEntryOnHeadingToolbar(const QString &element_name)
{
    ui.actionHeading1->setChecked(false);
    ui.actionHeading2->setChecked(false);
    ui.actionHeading3->setChecked(false);
    ui.actionHeading4->setChecked(false);
    ui.actionHeading5->setChecked(false);
    ui.actionHeading6->setChecked(false);
    ui.actionHeadingNormal->setChecked(false);

    if (!element_name.isEmpty()) {
        if ((element_name[ 0 ].toLower() == QChar('h')) && (element_name[ 1 ].isDigit())) {
            QString heading_name = QString(element_name[ 1 ]);

            if (heading_name == "1") {
                ui.actionHeading1->setChecked(true);
            } else if (heading_name == "2") {
                ui.actionHeading2->setChecked(true);
            } else if (heading_name == "3") {
                ui.actionHeading3->setChecked(true);
            } else if (heading_name == "4") {
                ui.actionHeading4->setChecked(true);
            } else if (heading_name == "5") {
                ui.actionHeading5->setChecked(true);
            } else if (heading_name == "6") {
                ui.actionHeading6->setChecked(true);
            }
        } else {
            ui.actionHeadingNormal->setChecked(true);
        }
    }
}

void MainWindow::ApplyHeadingStyleToTab(QAction* act)
{
    FlowTab *flow_tab = GetCurrentFlowTab();

    QString heading_type;
    QString name = act->objectName();
    if (name == "actionHeadingNormal") {
        heading_type = "Normal";
    } else {
        heading_type = name[ name.count() - 1 ];
    }

    if (flow_tab) {
        flow_tab->HeadingStyle(heading_type, m_preserveHeadingAttributes);
    }
}

void MainWindow::SetPreserveHeadingAttributes(bool new_state)
{
    m_preserveHeadingAttributes = new_state;
    ui.actionHeadingPreserveAttributes->setChecked(m_preserveHeadingAttributes);
    ShowMessageOnStatusBar(tr("Preserve existing heading attributes is now:") % " " %
                           (m_preserveHeadingAttributes ? tr("ON") : tr("OFF")));
}


void MainWindow::CreateRecentFilesActions()
{
    for (int i = 0; i < MAX_RECENT_FILES; ++i) {
        m_RecentFileActions[ i ] = new QAction(this);
        // The actions are not visible until we put a filename in them
        m_RecentFileActions[ i ]->setVisible(false);
        QList<QAction *> actlist = ui.menuFile->actions();
        // Add the new action just above the Quit action
        // and the separator behind it
        ui.menuFile->insertAction(actlist[ actlist.size() - 3 ], m_RecentFileActions[ i ]);
        connect(m_RecentFileActions[ i ], SIGNAL(triggered()), this, SLOT(OpenRecentFile()));
    }
}


void MainWindow::UpdateRecentFileActions()
{
    int num_recent_files = qMin(s_RecentFiles.size(), MAX_RECENT_FILES);

    // Store the filenames to the actions and display those actions
    for (int i = 0; i < num_recent_files; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(s_RecentFiles[ i ]).fileName());
        m_RecentFileActions[ i ]->setText(fontMetrics().elidedText(text, Qt::ElideRight, TEXT_ELIDE_WIDTH));
        m_RecentFileActions[ i ]->setData(s_RecentFiles[ i ]);
        m_RecentFileActions[ i ]->setVisible(true);
    }

    // If we have fewer files than actions, hide the other actions
    for (int j = num_recent_files; j < MAX_RECENT_FILES; ++j) {
        m_RecentFileActions[ j ]->setVisible(false);
    }

    QAction *separator = ui.menuFile->actions()[ ui.menuFile->actions().size() - 3 ];

    // If we have any actions with files shown,
    // display the separator; otherwise, don't
    if (num_recent_files > 0) {
        separator->setVisible(true);
    } else {
        separator->setVisible(false);
    }
}

void MainWindow::sizeMenuIcons() {
    // Size icons based on Qfont line-spacing and a
    // user-preference tweakable scale-factor.
    SettingsStore settings;
    double iconscalefactor = settings.mainMenuIconSize();
    int iconsize = QFontMetrics(QFont()).lineSpacing() * iconscalefactor;
    if (iconsize < 12) iconsize = 12;
    if (iconsize > 48) iconsize = 48;

    QList<QToolBar *> all_toolbars = findChildren<QToolBar *>();
    foreach(QToolBar * toolbar, all_toolbars) {
        toolbar->setIconSize(QSize(iconsize,iconsize));
    }
}

void MainWindow::PlatformSpecificTweaks()
{
    // We use the "close" action only on Macs,
    // because they need it for the multi-document interface
#ifndef Q_OS_MAC
    ui.actionClose->setEnabled(false);
    ui.actionClose->setVisible(false);
#else
    // Macs also use bigger icons but scale them automatically for high dpi displays
    //QList<QToolBar *> all_toolbars = findChildren<QToolBar *>();
    //foreach(QToolBar * toolbar, all_toolbars) {
    //    toolbar->setIconSize(QSize(32, 32));
    //}
    // Set the action because they are not automatically put in the right place as of Qt 5.1.
    // These can now be specified in the main.ui file
    // ui.actionAbout->setMenuRole(QAction::AboutRole);
    // ui.actionPreferences->setMenuRole(QAction::PreferencesRole);
#endif
    sizeMenuIcons();
}

void MainWindow::ExtendUI()
{
    // initialize list of quick launch plugin actions
    m_qlactions.append(ui.actionPlugin1);
    m_qlactions.append(ui.actionPlugin2);
    m_qlactions.append(ui.actionPlugin3);
    m_qlactions.append(ui.actionPlugin4);
    m_qlactions.append(ui.actionPlugin5);
    m_qlactions.append(ui.actionPlugin6);
    m_qlactions.append(ui.actionPlugin7);
    m_qlactions.append(ui.actionPlugin8);
    m_qlactions.append(ui.actionPlugin9);
    m_qlactions.append(ui.actionPlugin10);

    // initialize the first set of clip actions
    foreach(QAction * clipaction, ui.toolBarClips->actions()) {
        if (!clipaction->isSeparator()) {
            QString strIndex = clipaction->objectName();
            strIndex.replace(QString("actionClip"), QString(""));
            clipaction->setData(strIndex.toInt());
            m_clactions.append(clipaction);
        }
    }

    // initialize the second set of clip actions
    foreach(QAction * clipaction, ui.toolBarClips2->actions()) {
        if (!clipaction->isSeparator()) {
            QString strIndex = clipaction->objectName();
            strIndex.replace(QString("actionClip"), QString(""));
            clipaction->setData(strIndex.toInt());
            m_clactions.append(clipaction);
        }
    }

    // initialize action group from tbHeadings QToolButton actions
    foreach(QAction* ha, ui.tbHeadings->actions()) {
        if (!ha->isSeparator()) {
            m_headingActionGroup->addAction(ha);
        }
    }

    // initialize action group from tbCase QToolButton actions
    foreach(QAction* ca, ui.tbCase->actions()) {
        if (!ca->isSeparator()) {
            m_casingChangeGroup->addAction(ca);
        }
    }

    m_FindReplace->ShowHide();
    // We want a nice frame around the tab manager
    QFrame *frame = new QFrame(this);
    QLayout *layout = new QVBoxLayout(frame);
    frame->setLayout(layout);
    m_TabManager->setObjectName(TAB_MANAGER_NAME);
    m_FindReplace->setObjectName(FIND_REPLACE_NAME);
    layout->addWidget(m_TabManager);
    layout->addWidget(m_FindReplace);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);
    frame->setObjectName(FRAME_NAME);
    //frame->setStyleSheet(TAB_STYLE_SHEET);
    setCentralWidget(frame);
    m_BookBrowser = new BookBrowser(this);
    m_BookBrowser->setObjectName(BOOK_BROWSER_NAME);
    addDockWidget(Qt::LeftDockWidgetArea, m_BookBrowser);
    m_TableOfContents = new TableOfContents(this);
    m_TableOfContents->setObjectName(TABLE_OF_CONTENTS_NAME);
    addDockWidget(Qt::RightDockWidgetArea, m_TableOfContents);
    m_ValidationResultsView = new ValidationResultsView(this);
    m_ValidationResultsView->setObjectName(VALIDATION_RESULTS_VIEW_NAME);
    addDockWidget(Qt::BottomDockWidgetArea, m_ValidationResultsView);
    // By default, we want the validation results view to be hidden
    // *for first-time users*. That is, when a new user installs and opens Sigil,
    // the val. results view is hidden, but if he leaves it open before exiting,
    // then it will be open when he opens Sigil the next time.
    // Basically, restoreGeometry() in ReadSettings() overrules this command.
    m_ValidationResultsView->hide();

    m_PreviewWindow = new PreviewWindow(this);
    m_PreviewWindow->setObjectName(PREVIEW_WINDOW_NAME);
    m_PreviewWindow->setStyleSheet("QDockWidget {border: none;}");
    addDockWidget(Qt::RightDockWidgetArea, m_PreviewWindow);
    // Now that Book View is gone, show Preview by default on new installations
    // tabified with the TOC widget in the RightDockWidgetArea
    tabifyDockWidget(m_TableOfContents, m_PreviewWindow);

    m_Clips = new ClipsWindow(this);
    m_Clips->setObjectName(CLIPS_WINDOW_NAME);
    addDockWidget(Qt::LeftDockWidgetArea, m_Clips);
    m_Clips->hide();

    ui.menuView->addSeparator();
    ui.menuView->addAction(m_BookBrowser->toggleViewAction());
    m_BookBrowser->toggleViewAction()->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F1));
    ui.menuView->addAction(m_Clips->toggleViewAction());
    ui.menuView->addAction(m_PreviewWindow->toggleViewAction());
    m_PreviewWindow->toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F10));
    ui.menuView->addAction(m_TableOfContents->toggleViewAction());
    m_TableOfContents->toggleViewAction()->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F3));
    ui.menuView->addAction(m_ValidationResultsView->toggleViewAction());
    m_ValidationResultsView->toggleViewAction()->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F2));

    // Create the view menu to hide and show toolbars.
    ui.menuToolbars->addAction(ui.toolBarNewActions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarFileActions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarAddExisting->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarRepoActions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarUndoRedo->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextManip->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarXEditor->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarFind->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarInsertions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarBack->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarDonate->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTools->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarPlugins->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarPlugins2->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarHeadings->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextFormats->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextAlign->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarLists->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarIndents->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarChangeCase->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextDirection->toggleViewAction());
    ui.toolBarTextDirection->setVisible(false);
    ui.menuToolbars->addAction(ui.toolBarClips->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarClips2->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarIndexActions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarAutomate->toggleViewAction());
    ui.toolBarClips->setVisible(false);
    m_lbCursorPosition = new QLabel(QString(""), statusBar());
    statusBar()->addPermanentWidget(m_lbCursorPosition);
    UpdateCursorPositionLabel(0, 0);
    // Creating the zoom controls in the status bar
    m_slZoomSlider = new QSlider(Qt::Horizontal, statusBar());
    m_slZoomSlider->setTracking(false);
    m_slZoomSlider->setTickInterval(ZOOM_SLIDER_MIDDLE);
    m_slZoomSlider->setTickPosition(QSlider::TicksBelow);
    m_slZoomSlider->setFixedWidth(ZOOM_SLIDER_WIDTH);
    m_slZoomSlider->setMinimum(ZOOM_SLIDER_MIN);
    m_slZoomSlider->setMaximum(ZOOM_SLIDER_MAX);
    m_slZoomSlider->setValue(ZOOM_SLIDER_MIDDLE);
    QToolButton *zoom_out = new QToolButton(statusBar());
    zoom_out->setDefaultAction(ui.actionZoomOut);
    QToolButton *zoom_in = new QToolButton(statusBar());
    zoom_in->setDefaultAction(ui.actionZoomIn);
    m_lbZoomLabel = new QLabel(QString("100% "), statusBar());
    statusBar()->addPermanentWidget(m_lbZoomLabel);
    statusBar()->addPermanentWidget(zoom_out);
    statusBar()->addPermanentWidget(m_slZoomSlider);
    statusBar()->addPermanentWidget(zoom_in);
    // We override the default color for highlighted text
    // so we can actually *see* the text that the FindReplace
    // dialog finds in Book View... sadly, QWebView ignores a custom
    // palette set on it directly, so we have to do this globally.
    QPalette palette;
    palette.setColor(QPalette::Inactive, QPalette::Highlight, Qt::darkGreen);
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, Qt::white);
    qApp->setPalette(palette);
    // Setup userdefined keyboard shortcuts for actions.
    KeyboardShortcutManager *sm = KeyboardShortcutManager::instance();
    // Note: shortcut action Ids should not be translated.
    // File
    sm->registerAction(this, ui.actionNew, "MainWindow.NewDefault");
    sm->registerAction(this, ui.actionNewEpub2, "MainWindow.NewEpub2");
    sm->registerAction(this, ui.actionNewEpub3, "MainWindow.NewEpub3");
    sm->registerAction(this, ui.actionNewHTMLFile, "MainWindow.NewHTMLFile");
    sm->registerAction(this, ui.actionNewCSSFile, "MainWindow.NewCSSFile");
    sm->registerAction(this, ui.actionNewJSFile, "MainWindow.NewJSFile");
    sm->registerAction(this, ui.actionNewSVGFile, "MainWindow.NewSVGFile");
    sm->registerAction(this, ui.actionAddExistingFile, "MainWindow.AddExistingFile");
    sm->registerAction(this, ui.actionStandardize, "MainWindow.StandardizeEpub");
    sm->registerAction(this, ui.actionCustomLayout, "MainWindow.CustomLayout");
    sm->registerAction(this, ui.actionOpen, "MainWindow.Open");
#ifndef Q_OS_MAC
    sm->registerAction(this, ui.actionClose, "MainWindow.Close");
#endif
    sm->registerAction(this, ui.actionSave, "MainWindow.Save");
    sm->registerAction(this, ui.actionSaveAs, "MainWindow.SaveAs");
    sm->registerAction(this, ui.actionSaveACopy, "MainWindow.SaveACopy");
    sm->registerAction(this, ui.actionPrintPreview, "MainWindow.PrintPreview");
    sm->registerAction(this, ui.actionPrint, "MainWindow.Print");
    sm->registerAction(this, ui.actionExit, "MainWindow.Exit");
    // Edit
    sm->registerAction(this, ui.actionXEditor, "MainWindow.LaunchExternalXEditor");
    sm->registerAction(this, ui.actionUndo, "MainWindow.Undo");
    sm->registerAction(this, ui.actionRedo, "MainWindow.Redo");
    sm->registerAction(this, ui.actionCut, "MainWindow.Cut");
    sm->registerAction(this, ui.actionCopy, "MainWindow.Copy");
    sm->registerAction(this, ui.actionPaste, "MainWindow.Paste");
    sm->registerAction(this, ui.actionPasteClipboardHistory, "MainWindow.PasteClipboardHistory");
    sm->registerAction(this, ui.actionDeleteLine, "MainWindow.DeleteLine");
    sm->registerAction(this, ui.actionInsertFile, "MainWindow.InsertFile");
    sm->registerAction(this, ui.actionInsertSpecialCharacter, "MainWindow.InsertSpecialCharacter");
    sm->registerAction(this, ui.actionInsertId, "MainWindow.InsertId");
    sm->registerAction(this, ui.actionInsertHyperlink, "MainWindow.InsertHyperlink");
    sm->registerAction(this, ui.actionMarkForIndex, "MainWindow.MarkForIndex");
    sm->registerAction(this, ui.actionSplitSection, "MainWindow.SplitSection");
    sm->registerAction(this, ui.actionInsertSGFSectionMarker, "MainWindow.InsertSGFSectionMarker");
    sm->registerAction(this, ui.actionSplitOnSGFSectionMarkers, "MainWindow.SplitOnSGFSectionMarkers");
    sm->registerAction(this, ui.actionInsertClosingTag, "MainWindow.InsertClosingTag");
#ifndef Q_OS_MAC
    sm->registerAction(this, ui.actionPreferences, "MainWindow.Preferences");
#endif
    //Search
    sm->registerAction(this, ui.actionFind, "MainWindow.Find");
    sm->registerAction(this, ui.actionFindNext, "MainWindow.FindNext");
    sm->registerAction(this, ui.actionFindPrevious, "MainWindow.FindPrevious");
    sm->registerAction(this, ui.actionReplaceCurrent, "MainWindow.ReplaceCurrent");
    sm->registerAction(this, ui.actionReplaceNext, "MainWindow.ReplaceNext");
    sm->registerAction(this, ui.actionReplacePrevious, "MainWindow.ReplacePrevious");
    sm->registerAction(this, ui.actionReplaceAll, "MainWindow.ReplaceAll");
    sm->registerAction(this, ui.actionCount, "MainWindow.Count");
    sm->registerAction(this, ui.actionMarkSelection, "MainWindow.MarkSelection");
    sm->registerAction(this, ui.actionFindNextInFile, "MainWindow.FindNextInFile");
    sm->registerAction(this, ui.actionReplaceNextInFile, "MainWindow.ReplaceNextInFile");
    sm->registerAction(this, ui.actionReplaceAllInFile, "MainWindow.ReplaceAllInFile");
    sm->registerAction(this, ui.actionCountInFile, "MainWindow.CountInFile");
    sm->registerAction(this, ui.actionGoToLine, "MainWindow.GoToLine");
    sm->registerAction(this, ui.actionBookmarkLocation, "MainWindow.BookmarkLocation");
    sm->registerAction(this, ui.actionGoToLinkOrStyle, "MainWindow.GoToLinkOrStyle");
    sm->registerAction(this, ui.actionGoBackFromLinkOrStyle, "MainWindow.GoBackFromLinkOrStyle");
    // Format
    sm->registerAction(this, ui.actionBold, "MainWindow.Bold");
    sm->registerAction(this, ui.actionItalic, "MainWindow.Italic");
    sm->registerAction(this, ui.actionUnderline, "MainWindow.Underline");
    sm->registerAction(this, ui.actionStrikethrough, "MainWindow.Strikethrough");
    sm->registerAction(this, ui.actionSubscript, "MainWindow.Subscript");
    sm->registerAction(this, ui.actionSuperscript, "MainWindow.Superscript");
    sm->registerAction(this, ui.actionAlignLeft, "MainWindow.AlignLeft");
    sm->registerAction(this, ui.actionAlignCenter, "MainWindow.AlignCenter");
    sm->registerAction(this, ui.actionAlignRight, "MainWindow.AlignRight");
    sm->registerAction(this, ui.actionAlignJustify, "MainWindow.AlignJustify");
    sm->registerAction(this, ui.actionInsertNumberedList, "MainWindow.InsertNumberedList");
    sm->registerAction(this, ui.actionInsertBulletedList, "MainWindow.InsertBulletedList");
    sm->registerAction(this, ui.actionIncreaseIndent, "MainWindow.IncreaseIndent");
    sm->registerAction(this, ui.actionDecreaseIndent, "MainWindow.DecreaseIndent");
    sm->registerAction(this, ui.actionTextDirectionLTR, "MainWindow.TextDirectionLTR");
    sm->registerAction(this, ui.actionTextDirectionRTL, "MainWindow.TextDirectionRTL");
    sm->registerAction(this, ui.actionTextDirectionDefault, "MainWindow.TextDirectionDefault");
    sm->registerAction(this, ui.actionRemoveFormatting, "MainWindow.RemoveFormatting");
    sm->registerAction(this, ui.actionRemoveTagPair, "MainWindow.RemoveTagPair");
    sm->registerAction(this, ui.actionHeading1, "MainWindow.Heading1");
    sm->registerAction(this, ui.actionHeading2, "MainWindow.Heading2");
    sm->registerAction(this, ui.actionHeading3, "MainWindow.Heading3");
    sm->registerAction(this, ui.actionHeading4, "MainWindow.Heading4");
    sm->registerAction(this, ui.actionHeading5, "MainWindow.Heading5");
    sm->registerAction(this, ui.actionHeading6, "MainWindow.Heading6");
    sm->registerAction(this, ui.actionHeadingNormal, "MainWindow.HeadingNormal");
    sm->registerAction(this, ui.actionHeadingPreserveAttributes, "MainWindow.HeadingPreserveAttributes");
    sm->registerAction(this, ui.actionCasingLowercase, "MainWindow.CasingLowercase");
    sm->registerAction(this, ui.actionCasingUppercase, "MainWindow.CasingUppercase");
    sm->registerAction(this, ui.actionCasingTitlecase, "MainWindow.CasingTitlecase");
    sm->registerAction(this, ui.actionCasingCapitalize, "MainWindow.CasingCapitalize");
    // Tools
    sm->registerAction(this, ui.actionAddCover, "MainWindow.AddCover");
    sm->registerAction(this, ui.actionMetaEditor, "MainWindow.MetaEditor");
    sm->registerAction(this, ui.actionEditTOC, "MainWindow.EditTOC");
    sm->registerAction(this, ui.actionGenerateTOC, "MainWindow.GenerateTOC");
    sm->registerAction(this, ui.actionCreateHTMLTOC, "MainWindow.CreateHTMLTOC");
    sm->registerAction(this, ui.actionWellFormedCheckEpub, "MainWindow.WellFormedCheckEpub");
    sm->registerAction(this, ui.actionValidateStylesheetsWithW3C, "MainWindow.ValidateStylesheetsWithW3C");
    sm->registerAction(this, ui.actionAutoSpellCheck, "MainWindow.AutoSpellCheck");
    sm->registerAction(this, ui.actionMendPrettifyHTML, "MainWindow.MendPrettifyHTML");
    sm->registerAction(this, ui.actionMendHTML, "MainWindow.MendHTML");
    sm->registerAction(this, ui.actionUpdateManifestProperties, "MainWindow.UpdateManifestProperties");
    sm->registerAction(this, ui.actionNCXGuideFromNav, "MainWindow.NCXGuideFromNav");
    sm->registerAction(this, ui.actionRemoveNCXGuide, "MainWindow.RemoveNCXGuide");
    sm->registerAction(this, ui.actionSpellcheckEditor, "MainWindow.SpellcheckEditor");
    sm->registerAction(this, ui.actionSpellcheck, "MainWindow.Spellcheck");
    sm->registerAction(this, ui.actionAddMisspelledWord, "MainWindow.AddMispelledWord");
    sm->registerAction(this, ui.actionIgnoreMisspelledWord, "MainWindow.IgnoreMispelledWord");
    sm->registerAction(this, ui.actionClearIgnoredWords, "MainWindow.ClearIgnoredWords");
    sm->registerAction(this, ui.actionReports, "MainWindow.Reports");
    sm->registerAction(this, ui.actionSearchEditor, "MainWindow.SearchEditor");
    sm->registerAction(this, ui.actionClipEditor, "MainWindow.ClipEditor");
    sm->registerAction(this, ui.actionAddToIndex, "MainWindow.AddToIndex");
    sm->registerAction(this, ui.actionMarkForIndex, "MainWindow.MarkForIndex");
    sm->registerAction(this, ui.actionCreateIndex, "MainWindow.CreateIndex");
    sm->registerAction(this, ui.actionDeleteUnusedMedia, "MainWindow.DeleteUnusedMedia");
    sm->registerAction(this, ui.actionDeleteUnusedStyles, "MainWindow.DeleteUnusedStyles");
    // View
    sm->registerAction(this, ui.actionZoomIn, "MainWindow.ZoomIn");
    sm->registerAction(this, ui.actionZoomOut, "MainWindow.ZoomOut");
    sm->registerAction(this, ui.actionZoomReset, "MainWindow.ZoomReset");
    sm->registerAction(this, m_BookBrowser->toggleViewAction(), "MainWindow.BookBrowser");
    sm->registerAction(this, m_ValidationResultsView->toggleViewAction(), "MainWindow.ValidationResults");
    sm->registerAction(this, m_TableOfContents->toggleViewAction(), "MainWindow.TableOfContents");
    // Window
    sm->registerAction(this, ui.actionNextTab, "MainWindow.NextTab");
    sm->registerAction(this, ui.actionPreviousTab, "MainWindow.PreviousTab");
    sm->registerAction(this, ui.actionCloseTab, "MainWindow.CloseTab");
    sm->registerAction(this, ui.actionCloseOtherTabs, "MainWindow.CloseOtherTabs");
    sm->registerAction(this, ui.actionPreviousResource, "MainWindow.PreviousResource");
    sm->registerAction(this, ui.actionNextResource, "MainWindow.NextResource");
    // Checkpoints
    sm->registerAction(this, ui.actionCommit,     "MainWindow.CreateCheckpoint");
    sm->registerAction(this, ui.actionCheckout,   "MainWindow.RestoreFromCheckpoint");
    sm->registerAction(this, ui.actionDiff,       "MainWindow.CompareToCheckpoint");
    sm->registerAction(this, ui.actionManageRepo, "MainWindow.ManageCheckpointRepository");
    // Automation Lists
    sm->registerAction(this, ui.actionAutomate1,   "MainWindow.RunAutomate1");
    sm->registerAction(this, ui.actionAutomate2,   "MainWindow.RunAutomate2");
    sm->registerAction(this, ui.actionAutomate3,   "MainWindow.RunAutomate3");
    sm->registerAction(this, ui.actionAutomate1Editor,   "MainWindow.EditAutomate1");
    sm->registerAction(this, ui.actionAutomate2Editor,   "MainWindow.EditAutomate2");
    sm->registerAction(this, ui.actionAutomate3Editor,   "MainWindow.EditAutomate3");
    // Help
    sm->registerAction(this, ui.actionUserGuide, "MainWindow.UserGuide");
    sm->registerAction(this, ui.actionFAQ, "MainWindow.FAQ");
    sm->registerAction(this, ui.actionTutorials, "MainWindow.FAQ");
    sm->registerAction(this, ui.actionDonate, "MainWindow.Donate");
    sm->registerAction(this, ui.actionSigilWebsite, "MainWindow.SigilWebsite");
    sm->registerAction(this, ui.actionAbout, "MainWindow.About");

    // Clips
    foreach(QAction * clipaction, m_clactions) {
        QString clip_number = clipaction->data().toString();
        sm->registerAction(this, clipaction, "MainWindow.Clip" + clip_number);
    }

    // for plugins
    sm->registerAction(this, ui.actionPlugin1,  "MainWindow.Plugins.RunPlugin1");
    sm->registerAction(this, ui.actionPlugin2,  "MainWindow.Plugins.RunPlugin2");
    sm->registerAction(this, ui.actionPlugin3,  "MainWindow.Plugins.RunPlugin3");
    sm->registerAction(this, ui.actionPlugin4,  "MainWindow.Plugins.RunPlugin4");
    sm->registerAction(this, ui.actionPlugin5,  "MainWindow.Plugins.RunPlugin5");
    sm->registerAction(this, ui.actionPlugin6,  "MainWindow.Plugins.RunPlugin6");
    sm->registerAction(this, ui.actionPlugin7,  "MainWindow.Plugins.RunPlugin7");
    sm->registerAction(this, ui.actionPlugin8,  "MainWindow.Plugins.RunPlugin8");
    sm->registerAction(this, ui.actionPlugin9,  "MainWindow.Plugins.RunPlugin9");
    sm->registerAction(this, ui.actionPlugin10, "MainWindow.Plugins.RunPlugin10");

    // Headings QToolButton
    ui.tbHeadings->setPopupMode(QToolButton::InstantPopup);

    // Change Case QToolButton
    ui.tbCase->setPopupMode(QToolButton::InstantPopup);

    // Automate QToolButtons - set to run on click but delay for menu
    ui.tbAutomate1->setPopupMode(QToolButton::DelayedPopup);
    ui.tbAutomate2->setPopupMode(QToolButton::DelayedPopup);
    ui.tbAutomate3->setPopupMode(QToolButton::DelayedPopup);
    ui.tbAutomate1->setDefaultAction(ui.actionAutomate1);
    ui.tbAutomate2->setDefaultAction(ui.actionAutomate2);
    ui.tbAutomate3->setDefaultAction(ui.actionAutomate3);

    UpdateClipsUI();
}

void MainWindow::UpdateClipButton(QAction *ui_action)
{
    // clipEntry is a simple struct created by GetEntry with new,
    // no reference counting or smart pointers so they must be cleaned up appropriately
    int clip_number = ui_action->data().toInt();
    ClipEditorModel::clipEntry *clip_entry = ClipEditorModel::instance()->GetEntryFromNumber(clip_number);

    if (clip_entry) {
        ui_action->setText(clip_entry->name);
        QString clip_text = clip_entry->text;
        clip_text.replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;");
        ui_action->setToolTip(clip_text);
        ui_action->setVisible(true);
        // prevent memory leak
        delete clip_entry;
    } else {
        ui_action->setText("");
        ui_action->setToolTip("");
        ui_action->setVisible(false);
    }
}

void MainWindow::UpdateClipsUI()
{
    foreach(QAction * clipaction, m_clactions) {
        UpdateClipButton(clipaction);
    }
}


void MainWindow::LoadInitialFile(const QString &openfilepath, const QString version, bool is_internal)
{
    if (!openfilepath.isEmpty()) {
        LoadFile(QFileInfo(openfilepath).absoluteFilePath(), is_internal);
    } else {
        CreateNewBook(version);
    }
}

// Workaround for Long term Qt restore geometry bug - see WriteSettings() for details.
void MainWindow::UpdateLastSizes() {
    DWINGEO qDebug() << "------";
    DWINGEO qDebug() << "In UpdateLastSizes";
    DWINGEO qDebug() << "Pending: " << m_PendingLastSizeUpdate;
    DWINGEO qDebug() << "Enabled: " << m_SaveLastEnabled;

    if (!m_PendingLastSizeUpdate) return;

    if (m_SaveLastEnabled) {
        if (!isMaxOrFull()) {
            DWINGEO qDebug() << "recording last sizes";
            m_LastWindowSize = saveGeometry();
        }
    }

    DWINGEO DebugCurrentWidgetSizes();

    m_PendingLastSizeUpdate = false;

}

// This may still be needed on Windows and Linux
// so keep the code
void MainWindow::RestoreLastNormalGeometry()
{
    // record the current sizes before changing then as they
    // are updated in the resize event
    QByteArray WindowSize = m_LastWindowSize;
    DWINGEO qDebug() << "------";
    DWINGEO qDebug() << "In RestoreLastNormalGeometry";

    // prevent any resulting move or resize from being recorded here
    m_SaveLastEnabled = false;
    if (!WindowSize.isEmpty()) restoreGeometry(WindowSize);
    m_SaveLastEnabled=true;

    DWINGEO DebugCurrentWidgetSizes();
}

void MainWindow::changeEvent(QEvent *e) 
{
    DWINGEO qDebug() << "------";
    DWINGEO qDebug() << "In ChangeEvent: " << e;
    if(e->type() == QEvent::WindowStateChange) {
        const QWindowStateChangeEvent* wsevent = static_cast<QWindowStateChangeEvent*>(e);
        DWINGEO qDebug() << "old state" << wsevent->oldState();

        DWINGEO DebugCurrentWidgetSizes();

        if(isMinimized()) {
            // MINIMIZED
            DWINGEO qDebug() << "Main Window new state: minimized";
        } else if (isMaximized()) {
            DWINGEO qDebug() << "Main Window new state: maximized";
        } else if (isFullScreen()) {
            DWINGEO qDebug() << "Main Window new state: fullscreen";
            } else {
            // NORMAL
                DWINGEO qDebug() << "Main Window new state: normal";
            // This is still be needed for Windows and Linux to restore after maximize
                QTimer::singleShot(0, this, SLOT(RestoreLastNormalGeometry()));
        }
    }
    if (e->type() == QEvent::ActivationChange) {
        if(isActiveWindow()) {
            DWINGEO qDebug() << "------";
            DWINGEO qDebug() << "Main Window is transitioning from inactive to active: " << isMaxOrFull();

            if (m_FirstTime) {
                if (!m_LastState.isEmpty()) {
                    restoreState(m_LastState);
#ifdef Q_OS_MAC
                    // work around for macOS specific bug saved state restore with Find & Replace
                    if (m_FRVisible) {
                         QTimer::singleShot(0, this, SLOT(Find()));
                    }
#endif
                }

                DWINGEO {
                    int numscreens = qApp->desktop()->numScreens();
                    for (int i = 0; i < numscreens; i++) {
                        qDebug() << "Screen: " << i;
                        qDebug() << "    screen  geo: " << qApp->desktop()->screenGeometry(i);
                        QScreen *srn = QApplication::screens().at(i);
                        qDebug() << "    avail   geo: " << srn->availableGeometry();
                        qDebug() << "    geo        : " << srn->geometry();
                        qDebug() << "    devideRatio: " << srn->devicePixelRatio();
                        qDebug() << "    logical dpi: " << srn->logicalDotsPerInchX() << srn->logicalDotsPerInchY();
                        qDebug() << "    physic  dpi: " << srn->physicalDotsPerInchX() << srn->physicalDotsPerInchY();
                    }
                }

                // restoreState properly handles moving floating Preview Window
                // back to main screen if needed but keeps it hidden, only need to 
                // use View to display it, at least on macOSX

            }
            m_FirstTime = false;

            // moved here from showEvent to make sure it comes after state restoration
            if (!m_LastOpenFileWarnings.isEmpty()) {
                QTimer::singleShot(0, this, SLOT(ShowLastOpenFileWarnings()));
            }

            DBG DebugCurrentWidgetSizes();

            m_SaveLastEnabled = true;
            m_PendingLastSizeUpdate = true;
            UpdateLastSizes();

        } else {
            DWINGEO qDebug() << "Main Window is transitioning from active to inactive";
        }
    }

    QMainWindow::changeEvent(e);
    // e->accept();
}

void MainWindow::ConnectSignalsToSlots()
{
    connect(m_PreviewWindow, SIGNAL(Shown()), this, SLOT(UpdatePreview()));
    connect(m_PreviewWindow, SIGNAL(ZoomFactorChanged(float)),     this, SLOT(UpdateZoomLabel(float)));
    connect(m_PreviewWindow, SIGNAL(ZoomFactorChanged(float)),     this, SLOT(UpdateZoomSlider(float)));
    connect(m_PreviewWindow, SIGNAL(GoToPreviewLocationRequest()), this, SLOT(GoToPreviewLocation()));
    connect(m_PreviewWindow, SIGNAL(RequestPreviewReload()),       this, SLOT(UpdatePreview())); 
    connect(m_PreviewWindow, SIGNAL(OpenUrlRequest(const QUrl &)), this, SLOT(OpenUrl(const QUrl &)));
    connect(m_PreviewWindow, SIGNAL(ScrollToFragmentRequest(const QString &)), this, SLOT(ScrollCVToFragment(const QString &)));
    connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)), this, SLOT(ApplicationFocusChanged(QWidget *, QWidget *)));
    MainApplication *mainApplication = qobject_cast<MainApplication *>(qApp);
    connect(mainApplication, SIGNAL(applicationPaletteChanged()), this, SLOT(ApplicationPaletteChanged()));

    // File
    connect(ui.actionNew,           SIGNAL(triggered()), this, SLOT(NewDefault()));
    connect(ui.actionNewEpub2,      SIGNAL(triggered()), this, SLOT(NewEpub2()));
    connect(ui.actionNewEpub3,      SIGNAL(triggered()), this, SLOT(NewEpub3()));
    connect(ui.actionOpen,          SIGNAL(triggered()), this, SLOT(Open()));
    connect(ui.actionNewHTMLFile,   SIGNAL(triggered()), m_BookBrowser, SLOT(AddNewHTML()));
    connect(ui.actionNewCSSFile,    SIGNAL(triggered()), m_BookBrowser, SLOT(AddNewCSS()));
    connect(ui.actionNewJSFile,     SIGNAL(triggered()), m_BookBrowser, SLOT(AddNewJS()));
    connect(ui.actionNewSVGFile,    SIGNAL(triggered()), m_BookBrowser, SLOT(AddNewSVG()));
    connect(ui.actionAddExistingFile,   SIGNAL(triggered()), m_BookBrowser, SLOT(AddExisting()));
    connect(ui.actionSave,          SIGNAL(triggered()), this, SLOT(Save()));
    connect(ui.actionSaveAs,        SIGNAL(triggered()), this, SLOT(SaveAs()));
    connect(ui.actionSaveACopy,     SIGNAL(triggered()), this, SLOT(SaveACopy()));
    connect(ui.actionClose,         SIGNAL(triggered()), this, SLOT(close()));
    connect(ui.actionExit,          SIGNAL(triggered()), this, SLOT(Exit()));

    // Checkpoint Repo functions
    connect(ui.actionCommit,        SIGNAL(triggered()), this, SLOT(RepoCommit()));
    connect(ui.actionCheckout,      SIGNAL(triggered()), this, SLOT(RepoCheckout()));
    connect(ui.actionDiff,          SIGNAL(triggered()), this, SLOT(RepoDiff()));
    connect(ui.actionManageRepo,    SIGNAL(triggered()), this, SLOT(RepoManage()));

    // Automation
    connect(ui.actionAutomate1,        SIGNAL(triggered()), this, SLOT(RunAutomate1()));
    connect(ui.actionAutomate2,        SIGNAL(triggered()), this, SLOT(RunAutomate2()));
    connect(ui.actionAutomate3,        SIGNAL(triggered()), this, SLOT(RunAutomate3()));
    connect(ui.actionAutomate1Editor,  SIGNAL(triggered()), this, SLOT(EditAutomate1()));
    connect(ui.actionAutomate2Editor,  SIGNAL(triggered()), this, SLOT(EditAutomate2()));
    connect(ui.actionAutomate3Editor,  SIGNAL(triggered()), this, SLOT(EditAutomate3()));

    // Edit
    connect(ui.actionXEditor,         SIGNAL(triggered()), this, SLOT(launchExternalXEditor()));
    connect(ui.actionInsertFile,      SIGNAL(triggered()), this, SLOT(InsertFileDialog()));
    connect(ui.actionInsertSpecialCharacter, SIGNAL(triggered()), this, SLOT(InsertSpecialCharacter()));
    connect(ui.actionInsertId,        SIGNAL(triggered()),  this,   SLOT(InsertId()));
    connect(ui.actionInsertHyperlink, SIGNAL(triggered()),  this,   SLOT(InsertHyperlink()));
    connect(ui.actionPreferences,     SIGNAL(triggered()), this, SLOT(PreferencesDialog()));
    // Search
    connect(ui.actionFind,             SIGNAL(triggered()), this, SLOT(Find()));
    connect(ui.actionFindNext,         SIGNAL(triggered()), m_FindReplace, SLOT(FindNext()));
    connect(ui.actionFindPrevious,     SIGNAL(triggered()), m_FindReplace, SLOT(FindPrevious()));
    connect(ui.actionReplaceCurrent,   SIGNAL(triggered()), m_FindReplace, SLOT(ReplaceCurrent()));
    connect(ui.actionReplaceNext,      SIGNAL(triggered()), m_FindReplace, SLOT(ReplaceNext()));
    connect(ui.actionReplacePrevious,  SIGNAL(triggered()), m_FindReplace, SLOT(ReplacePrevious()));
    connect(ui.actionReplaceAll,       SIGNAL(triggered()), m_FindReplace, SLOT(ReplaceAll()));
    connect(ui.actionCount,            SIGNAL(triggered()), m_FindReplace, SLOT(Count()));
    connect(ui.actionFindNextInFile,   SIGNAL(triggered()), m_FindReplace, SLOT(FindNextInFile()));
    connect(ui.actionReplaceNextInFile, SIGNAL(triggered()), m_FindReplace, SLOT(ReplaceNextInFile()));
    connect(ui.actionReplaceAllInFile, SIGNAL(triggered()), m_FindReplace, SLOT(ReplaceAllInFile()));
    connect(ui.actionMarkSelection,    SIGNAL(triggered()), this, SLOT(MarkSelection()));
    connect(ui.actionCountInFile,      SIGNAL(triggered()), m_FindReplace, SLOT(CountInFile()));
    connect(ui.actionGoToLine,         SIGNAL(triggered()), this, SLOT(GoToLine()));
    // About
    connect(ui.actionUserGuide,     SIGNAL(triggered()), this, SLOT(UserGuide()));
    connect(ui.actionDonate,        SIGNAL(triggered()), this, SLOT(Donate()));
    connect(ui.actionSigilWebsite,  SIGNAL(triggered()), this, SLOT(SigilWebsite()));
    connect(ui.actionAbout,         SIGNAL(triggered()), this, SLOT(AboutDialog()));
    // Tools
    connect(ui.actionStandardize,   SIGNAL(triggered()), this, SLOT(StandardizeEpub()));
    connect(ui.actionCustomLayout,  SIGNAL(triggered()), this, SLOT(CreateEpubLayout()));
    connect(ui.actionAddCover,      SIGNAL(triggered()), this, SLOT(AddCover()));
    connect(ui.actionMetaEditor,    SIGNAL(triggered()), this, SLOT(MetaEditorDialog()));
    connect(ui.actionWellFormedCheckEpub,  SIGNAL(triggered()), this, SLOT(WellFormedCheckEpub()));
    connect(ui.actionValidateStylesheetsWithW3C,  SIGNAL(triggered()), this, SLOT(ValidateStylesheetsWithW3C()));
    connect(ui.actionSpellcheckEditor,   SIGNAL(triggered()), this, SLOT(SpellcheckEditorDialog()));
    connect(ui.actionAutoSpellCheck, SIGNAL(triggered(bool)), this, SLOT(SetAutoSpellCheck(bool)));
    connect(ui.actionSpellcheck,    SIGNAL(triggered()), m_FindReplace, SLOT(FindMisspelledWord()));
    connect(ui.actionMendPrettifyHTML,    SIGNAL(triggered()), this, SLOT(MendPrettifyHTML()));
    connect(ui.actionMendHTML,      SIGNAL(triggered()), this, SLOT(MendHTML()));
    connect(ui.actionUpdateManifestProperties,      SIGNAL(triggered()), this, SLOT(UpdateManifestProperties()));
    connect(ui.actionNCXGuideFromNav, SIGNAL(triggered()), this, SLOT(GenerateNCXGuideFromNav()));
    connect(ui.actionRemoveNCXGuide,  SIGNAL(triggered()), this, SLOT(RemoveNCXGuideFromEpub3()));
    connect(ui.actionClearIgnoredWords, SIGNAL(triggered()), this, SLOT(ClearIgnoredWords()));
    connect(ui.actionGenerateTOC,   SIGNAL(triggered()), this, SLOT(GenerateTOC()));
    connect(ui.actionEditTOC,       SIGNAL(triggered()), this, SLOT(EditTOCDialog()));
    connect(ui.actionCreateHTMLTOC, SIGNAL(triggered()), this, SLOT(CreateHTMLTOC()));
    connect(ui.actionReports,       SIGNAL(triggered()), this, SLOT(ReportsDialog()));
    connect(ui.actionClipEditor,    SIGNAL(triggered()), this, SLOT(ClipEditorDialog()));
    connect(ui.actionSearchEditor,  SIGNAL(triggered()), this, SLOT(SearchEditorDialog()));
    connect(ui.actionIndexEditor,   SIGNAL(triggered()), this, SLOT(IndexEditorDialog()));
    connect(ui.actionMarkForIndex,  SIGNAL(triggered()), this, SLOT(MarkForIndex()));
    connect(ui.actionCreateIndex,   SIGNAL(triggered()), this, SLOT(CreateIndex()));
    connect(ui.actionDeleteUnusedMedia,    SIGNAL(triggered()), this, SLOT(DeleteUnusedMedia()));
    connect(ui.actionDeleteUnusedStyles,    SIGNAL(triggered()), this, SLOT(DeleteUnusedStyles()));
    // Change case
    connect(m_casingChangeGroup,    SIGNAL(triggered(QAction*)), this, SLOT(ChangeCasing(QAction*)));
    // View
    connect(ui.actionZoomIn,        SIGNAL(triggered()), this, SLOT(ZoomIn()));
    connect(ui.actionZoomOut,       SIGNAL(triggered()), this, SLOT(ZoomOut()));
    connect(ui.actionZoomReset,     SIGNAL(triggered()), this, SLOT(ZoomReset()));
    connect(ui.actionHeadingPreserveAttributes, SIGNAL(triggered(bool)), this, SLOT(SetPreserveHeadingAttributes(bool)));
    connect(m_headingActionGroup,   SIGNAL(triggered(QAction*)), this, SLOT(ApplyHeadingStyleToTab(QAction*)));
    // Window
    connect(ui.actionNextTab,       SIGNAL(triggered()), m_TabManager, SLOT(NextTab()));
    connect(ui.actionPreviousTab,   SIGNAL(triggered()), m_TabManager, SLOT(PreviousTab()));
    connect(ui.actionCloseTab,      SIGNAL(triggered()), m_TabManager, SLOT(CloseTab()));
    connect(ui.actionCloseOtherTabs, SIGNAL(triggered()), m_TabManager, SLOT(CloseOtherTabs()));
    connect(ui.actionPreviousResource, SIGNAL(triggered()), m_BookBrowser, SLOT(PreviousResource()));
    connect(ui.actionNextResource,     SIGNAL(triggered()), m_BookBrowser, SLOT(NextResource()));
    connect(ui.actionBookmarkLocation,  SIGNAL(triggered()), this,   SLOT(BookmarkLocation()));
    connect(ui.actionGoBackFromLinkOrStyle,  SIGNAL(triggered()), this,   SLOT(GoBackFromLinkOrStyle()));
    connect(ui.actionSplitOnSGFSectionMarkers, SIGNAL(triggered()),  this,   SLOT(SplitOnSGFSectionMarkers()));
    connect(ui.actionPasteClipboardHistory,    SIGNAL(triggered()),  this,   SLOT(ShowPasteClipboardHistoryDialog()));

    // Clips
    foreach(QAction* clipaction, m_clactions) {
        // Use the new signal/slot syntax and use a lambda to
        // eliminate the need for the obsoleted QSignalMapper.
        // [captured variables]() {...anonymous processing to do...;}
        int i = clipaction->data().toInt();
        connect(clipaction, &QAction::triggered, this, [this,i]() {
                MainWindow::PasteClipIntoCurrentTarget(i);
        });
    }

    // Slider
    connect(m_slZoomSlider,         SIGNAL(valueChanged(int)), this, SLOT(SliderZoom(int)));
    // We also update the label when the slider moves... this is to show
    // the zoom value the slider will land on while it is being moved.
    connect(m_slZoomSlider,         SIGNAL(sliderMoved(int)),  this, SLOT(UpdateZoomLabel(int)));
    connect(m_TabManager,          SIGNAL(TabCountChanged()),
            this,                   SLOT(UpdateUIOnTabCountChange()));
    connect(m_TabManager,          SIGNAL(TabChanged(ContentTab *, ContentTab *)),
            this,                   SLOT(ChangeSignalsWhenTabChanges(ContentTab *, ContentTab *)));
    connect(m_TabManager,          SIGNAL(TabChanged(ContentTab *, ContentTab *)),
            this,                   SLOT(UpdateUIWhenTabsSwitch()));
    connect(m_TabManager,          SIGNAL(TabChanged(ContentTab *, ContentTab *)),
            this,                    SLOT(UpdateBrowserSelectionToTab()));
    connect(m_TabManager,          SIGNAL(UpdatePreviewAfterExistingTabSwitch()),
            this,                    SLOT(UpdatePreview()));
    connect(m_BookBrowser,          SIGNAL(UpdateBrowserSelection()),
            this,                    SLOT(UpdateBrowserSelectionToTab()));
    connect(m_BookBrowser, SIGNAL(RenumberTOCContentsRequest()),
            m_TableOfContents,     SLOT(RenumberTOCContents()));
    connect(m_BookBrowser, SIGNAL(RemoveTabRequest()),
            m_TabManager, SLOT(RemoveTab()));
    connect(m_BookBrowser, SIGNAL(ResourceActivated(Resource *)),
            this, SLOT(OpenResource(Resource *)));
    connect(m_BookBrowser, SIGNAL(MergeResourcesRequest(QList<Resource *>)), this, SLOT(MergeResources(QList<Resource *>)));
    connect(m_BookBrowser, SIGNAL(LinkStylesheetsToResourcesRequest(QList<Resource *>)), this, SLOT(LinkStylesheetsToResources(QList<Resource *>)));
    connect(m_BookBrowser, SIGNAL(LinkJavascriptsToResourcesRequest(QList<Resource *>)), this, SLOT(LinkJavascriptsToResources(QList<Resource *>)));
    connect(m_BookBrowser, SIGNAL(RemoveResourcesRequest()), this, SLOT(RemoveResources()));
    connect(m_BookBrowser, SIGNAL(OpenFileRequest(QString, int, int)), this, SLOT(OpenFile(QString, int, int)));
    connect(m_TableOfContents, SIGNAL(OpenResourceRequest(Resource *, int, int, const QString &, const QUrl &)),
            this,     SLOT(OpenResource(Resource *, int, int, const QString &, const QUrl &)));
    connect(m_ValidationResultsView, SIGNAL(OpenResourceRequest(Resource *, int, int, const QString &)),
            this,     SLOT(OpenResource(Resource *, int, int, const QString &)));
    connect(m_TabManager, SIGNAL(OpenUrlRequest(const QUrl &)),
            this, SLOT(OpenUrl(const QUrl &)));
    connect(m_TabManager, SIGNAL(OldTabRequest(QString, HTMLResource *)),
            this,          SLOT(CreateSectionBreakOldTab(QString, HTMLResource *)));
    connect(m_FindReplace, SIGNAL(OpenSearchEditorRequest(SearchEditorModel::searchEntry *)),
            this,          SLOT(SearchEditorDialog(SearchEditorModel::searchEntry *)));
    connect(m_TabManager, SIGNAL(ShowStatusMessageRequest(const QString &, int)), this, SLOT(ShowMessageOnStatusBar(const QString &, int)));
    connect(m_FindReplace, SIGNAL(ShowMessageRequest(const QString &)),
            m_SearchEditor, SLOT(ShowMessage(const QString &)));
    connect(m_FindReplace,   SIGNAL(ClipboardSaveRequest()),     m_ClipboardHistorySelector,  SLOT(SaveClipboardState()));
    connect(m_FindReplace,   SIGNAL(ClipboardRestoreRequest()),  m_ClipboardHistorySelector,  SLOT(RestoreClipboardState()));
    connect(m_SearchEditor, SIGNAL(LoadSelectedSearchRequest(SearchEditorModel::searchEntry *)),
            m_FindReplace,   SLOT(LoadSearch(SearchEditorModel::searchEntry *)));
    connect(m_SearchEditor, SIGNAL(FindSelectedSearchRequest(QList<SearchEditorModel::searchEntry *>)),
            m_FindReplace,   SLOT(FindSearch(QList<SearchEditorModel::searchEntry *>)));
    connect(m_SearchEditor, SIGNAL(ReplaceCurrentSelectedSearchRequest(QList<SearchEditorModel::searchEntry *>)),
            m_FindReplace,   SLOT(ReplaceCurrentSearch(QList<SearchEditorModel::searchEntry *>)));
    connect(m_SearchEditor, SIGNAL(ReplaceSelectedSearchRequest(QList<SearchEditorModel::searchEntry *>)),
            m_FindReplace,   SLOT(ReplaceSearch(QList<SearchEditorModel::searchEntry *>)));
    connect(m_SearchEditor, SIGNAL(CountAllSelectedSearchRequest(QList<SearchEditorModel::searchEntry *>)),
            m_FindReplace,   SLOT(CountAllSearch(QList<SearchEditorModel::searchEntry *>)));
    connect(m_SearchEditor, SIGNAL(ReplaceAllSelectedSearchRequest(QList<SearchEditorModel::searchEntry *>)),
            m_FindReplace,   SLOT(ReplaceAllSearch(QList<SearchEditorModel::searchEntry *>)));
    connect(m_ClipboardHistorySelector, SIGNAL(PasteRequest(const QString &)), this, SLOT(PasteTextIntoCurrentTarget(const QString &)));
    connect(m_SelectCharacter, SIGNAL(SelectedCharacter(const QString &)), this, SLOT(PasteTextIntoCurrentTarget(const QString &)));
    connect(m_ClipEditor, SIGNAL(PasteSelectedClipRequest(QList<ClipEditorModel::clipEntry *>)),
            this,           SLOT(PasteClipEntriesIntoCurrentTarget(QList<ClipEditorModel::clipEntry *>)));
    connect(m_Clips,        SIGNAL(PasteClips(QList<ClipEditorModel::clipEntry *>)),
            this,            SLOT(PasteClipEntriesIntoPreviousTarget(QList<ClipEditorModel::clipEntry *>)));
    connect(m_SearchEditor, SIGNAL(ShowStatusMessageRequest(const QString &)),
            this,            SLOT(ShowMessageOnStatusBar(const QString &)));
    connect(m_ClipEditor,   SIGNAL(ShowStatusMessageRequest(const QString &)),
            this,            SLOT(ShowMessageOnStatusBar(const QString &)));
    connect(m_ClipEditor,   SIGNAL(ClipsUpdated()),
            this,            SLOT(UpdateClipsUI()));
    connect(m_IndexEditor,  SIGNAL(ShowStatusMessageRequest(const QString &)),
            this,            SLOT(ShowMessageOnStatusBar(const QString &)));
    // connect(m_MetaEditor,  SIGNAL(ShowStatusMessageRequest(const QString &)),
    //         this,            SLOT(ShowMessageOnStatusBar(const QString &)));
    connect(m_SpellcheckEditor,  SIGNAL(ShowStatusMessageRequest(const QString &)),
            this,            SLOT(ShowMessageOnStatusBar(const QString &)));
    connect(m_SpellcheckEditor,   SIGNAL(SpellingHighlightRefreshRequest()), this,  SLOT(RefreshSpellingHighlighting()));
    connect(m_SpellcheckEditor,   SIGNAL(FindWordRequest(QString)), this,  SLOT(FindWord(QString)));
    connect(m_SpellcheckEditor,   SIGNAL(UpdateWordRequest(QString, QString)), this,  SLOT(UpdateWord(QString, QString)));
    connect(m_SpellcheckEditor,   SIGNAL(ShowStatusMessageRequest(const QString &)),
            this,  SLOT(ShowMessageOnStatusBar(const QString &)));
    connect(m_Reports,       SIGNAL(Refresh()), this, SLOT(ReportsDialog()));
    connect(m_Reports,       SIGNAL(OpenFileRequest(QString, int, int)), this, SLOT(OpenFile(QString, int, int)));
    connect(m_Reports,       SIGNAL(DeleteFilesRequest(QStringList)), this, SLOT(DeleteFilenames(QStringList)));
    connect(m_Reports,       SIGNAL(DeleteStylesRequest(QList<BookReports::StyleData *>)), this, SLOT(DeleteReportsStyles(QList<BookReports::StyleData *>)));
    connect(m_Reports,       SIGNAL(FindText(QString)), m_FindReplace, SLOT(FindAnyText(QString)));
    connect(m_Reports,       SIGNAL(FindTextInTags(QString)), m_FindReplace, SLOT(FindAnyTextInTags(QString)));

    // Plugins
    PluginDB *pdb = PluginDB::instance();
    connect(pdb, SIGNAL(plugins_changed()), this, SLOT(loadPluginsMenu()));
}

void MainWindow::MakeTabConnections(ContentTab *tab)
{
    Resource::ResourceType rType;

    if (tab == NULL) {
        return;
    }

    rType = tab->GetLoadedResource()->Type();

    // Triggered connections should be disconnected in BreakTabConnections
    if (rType != Resource::ImageResourceType && 
        rType != Resource::AudioResourceType && 
        rType != Resource::VideoResourceType && 
        rType != Resource::FontResourceType) {
        connect(ui.actionUndo,                     SIGNAL(triggered()),  tab,   SLOT(Undo()));
        connect(ui.actionRedo,                     SIGNAL(triggered()),  tab,   SLOT(Redo()));
        connect(ui.actionCut,                      SIGNAL(triggered()),  tab,   SLOT(Cut()));
        connect(ui.actionCopy,                     SIGNAL(triggered()),  tab,   SLOT(Copy()));
        connect(ui.actionPaste,                    SIGNAL(triggered()),  tab,   SLOT(Paste()));
        connect(ui.actionDeleteLine,               SIGNAL(triggered()),  tab,   SLOT(DeleteLine()));
        connect(tab,   SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)),
                this,  SLOT(ClipEditorDialog(ClipEditorModel::clipEntry *)));
    }

    // Allow Cut, Copy, Paste to work in other TextTabs to enable basic edit icons
    if (rType == Resource::MiscTextResourceType ||
        rType == Resource::OPFResourceType ||
        rType == Resource::NCXResourceType ||
        rType == Resource::SVGResourceType ||
        rType == Resource::TextResourceType ||
        rType == Resource::XMLResourceType) {
        connect(tab,   SIGNAL(SelectionChanged()),           this,          SLOT(UpdateUIOnTabChanges()));
    }

    if (rType == Resource::HTMLResourceType ||
        rType == Resource::ImageResourceType ||
        rType == Resource::SVGResourceType) {
        connect(tab, SIGNAL(InsertedFileOpenedExternally(const QString &)), this, SLOT(SetInsertedFileWatchResourceFile(const QString &)));
        connect(tab, SIGNAL(InsertedFileSaveAs(const QUrl &)), m_BookBrowser, SLOT(SaveAsUrl(const QUrl &)));
    }

    if (rType == Resource::CSSResourceType) {
        connect(tab,   SIGNAL(CSSUpdated()), this, SLOT(UpdatePreviewCSSRequest()));
    }

    if (rType == Resource::HTMLResourceType ||
        rType == Resource::CSSResourceType) {
        connect(ui.actionBold,                     SIGNAL(triggered()),  tab,   SLOT(Bold()));
        connect(ui.actionItalic,                   SIGNAL(triggered()),  tab,   SLOT(Italic()));
        connect(ui.actionUnderline,                SIGNAL(triggered()),  tab,   SLOT(Underline()));
        connect(ui.actionStrikethrough,            SIGNAL(triggered()),  tab,   SLOT(Strikethrough()));
        connect(ui.actionAlignLeft,                SIGNAL(triggered()),  tab,   SLOT(AlignLeft()));
        connect(ui.actionAlignCenter,              SIGNAL(triggered()),  tab,   SLOT(AlignCenter()));
        connect(ui.actionAlignRight,               SIGNAL(triggered()),  tab,   SLOT(AlignRight()));
        connect(ui.actionAlignJustify,             SIGNAL(triggered()),  tab,   SLOT(AlignJustify()));
        connect(ui.actionTextDirectionLTR,         SIGNAL(triggered()),  tab,   SLOT(TextDirectionLeftToRight()));
        connect(ui.actionTextDirectionRTL,         SIGNAL(triggered()),  tab,   SLOT(TextDirectionRightToLeft()));
        connect(ui.actionTextDirectionDefault,     SIGNAL(triggered()),  tab,   SLOT(TextDirectionDefault()));
        connect(tab,   SIGNAL(SelectionChanged()),           this,          SLOT(UpdateUIOnTabChanges()));
    }

    if (rType == Resource::HTMLResourceType) {
        connect(ui.actionSubscript,                SIGNAL(triggered()),  tab,   SLOT(Subscript()));
        connect(ui.actionSuperscript,              SIGNAL(triggered()),  tab,   SLOT(Superscript()));
        connect(ui.actionInsertBulletedList,       SIGNAL(triggered()),  tab,   SLOT(InsertBulletedList()));
        connect(ui.actionInsertNumberedList,       SIGNAL(triggered()),  tab,   SLOT(InsertNumberedList()));
        connect(ui.actionDecreaseIndent,           SIGNAL(triggered()),  tab,   SLOT(DecreaseIndent()));
        connect(ui.actionIncreaseIndent,           SIGNAL(triggered()),  tab,   SLOT(IncreaseIndent()));
        connect(ui.actionRemoveFormatting,         SIGNAL(triggered()),  tab,   SLOT(RemoveFormatting()));
        connect(ui.actionRemoveTagPair,            SIGNAL(triggered()),  tab,   SLOT(RemoveTagPair()));
        connect(ui.actionSplitSection,             SIGNAL(triggered()),  tab,   SLOT(SplitSection()));
        connect(ui.actionInsertSGFSectionMarker,   SIGNAL(triggered()),  tab,   SLOT(InsertSGFSectionMarker()));
        connect(ui.actionInsertClosingTag,         SIGNAL(triggered()),  tab,   SLOT(InsertClosingTag()));
        connect(ui.actionGoToLinkOrStyle,          SIGNAL(triggered()),  tab,   SLOT(GoToLinkOrStyle()));
        connect(ui.actionAddToIndex,               SIGNAL(triggered()),  tab,   SLOT(AddToIndex()));
        connect(ui.actionAddMisspelledWord,        SIGNAL(triggered()),  tab,   SLOT(AddMisspelledWord()));
        connect(ui.actionIgnoreMisspelledWord,     SIGNAL(triggered()),  tab,   SLOT(IgnoreMisspelledWord()));
        connect(this,                              SIGNAL(SettingsChanged()), tab, SLOT(LoadSettings()));
        connect(tab,   SIGNAL(OpenIndexEditorRequest(IndexEditorModel::indexEntry *)),
                this,  SLOT(IndexEditorDialog(IndexEditorModel::indexEntry *)));
        connect(tab,   SIGNAL(ViewImageRequest(const QUrl &)),
                this,  SLOT(ViewImageDialog(const QUrl &)));
        connect(tab,   SIGNAL(GoToLinkedStyleDefinitionRequest(const QString &, const QString &)),
                this,  SLOT(GoToLinkedStyleDefinition(const QString &, const QString &)));
        connect(tab,   SIGNAL(BookmarkLinkOrStyleLocationRequest()),
                this,  SLOT(BookmarkLinkOrStyleLocation()));
        connect(tab,   SIGNAL(ClipboardSaveRequest()),     m_ClipboardHistorySelector,  SLOT(SaveClipboardState()));
        connect(tab,   SIGNAL(ClipboardRestoreRequest()),  m_ClipboardHistorySelector,  SLOT(RestoreClipboardState()));
        connect(tab,   SIGNAL(SpellingHighlightRefreshRequest()), this,  SLOT(RefreshSpellingHighlighting()));
        connect(tab,   SIGNAL(InsertFileRequest()), this,  SLOT(InsertFileDialog()));
        connect(tab,   SIGNAL(UpdatePreview()), this, SLOT(UpdatePreviewRequest()));
        connect(tab,   SIGNAL(UpdatePreviewImmediately()), this, SLOT(UpdatePreview()));
        connect(tab,   SIGNAL(ScrollPreviewImmediately()), this, SLOT(ScrollPreview()));
    }

    if (rType != Resource::AudioResourceType && 
        rType != Resource::VideoResourceType &&
        rType != Resource::FontResourceType) {
        connect(ui.actionPrintPreview,             SIGNAL(triggered()),  tab,   SLOT(PrintPreview()));
        connect(ui.actionPrint,                    SIGNAL(triggered()),  tab,   SLOT(Print()));
        connect(tab,   SIGNAL(ContentChanged()),             m_Book.data(), SLOT(SetModified()));
        connect(tab,   SIGNAL(UpdateCursorPosition(int, int)), this,          SLOT(UpdateCursorPositionLabel(int, int)));
        connect(tab,   SIGNAL(ZoomFactorChanged(float)),   this,          SLOT(UpdateZoomLabel(float)));
        connect(tab,   SIGNAL(ZoomFactorChanged(float)),   this,          SLOT(UpdateZoomSlider(float)));
        connect(tab,   SIGNAL(ShowStatusMessageRequest(const QString &)), this, SLOT(ShowMessageOnStatusBar(const QString &)));
        connect(tab,   SIGNAL(MarkSelectionRequest()),             this, SLOT(MarkSelection()));
        connect(tab,   SIGNAL(ClearMarkedTextRequest()),           this, SLOT(ClearMarkedText()));
    }
}


void MainWindow::BreakTabConnections(ContentTab *tab)
{
    if (tab == NULL) {
        return;
    }

    // first disconnect tab from us
    disconnect(this, 0, tab, 0);
    if (tab) disconnect(tab, 0, this, 0);
    if (tab) disconnect(tab, 0, m_Book.data(), 0);
    if (tab) disconnect(tab, 0, m_BookBrowser, 0);
    if (tab) disconnect(tab, 0, m_ClipboardHistorySelector, 0);

    // next disconnect it from ui.actions
    disconnect(ui.actionUndo,                      0, tab, 0);
    disconnect(ui.actionRedo,                      0, tab, 0);
    disconnect(ui.actionCut,                       0, tab, 0);
    disconnect(ui.actionCopy,                      0, tab, 0);
    disconnect(ui.actionPaste,                     0, tab, 0);
    disconnect(ui.actionDeleteLine,                0, tab, 0);
    disconnect(ui.actionBold,                      0, tab, 0);
    disconnect(ui.actionItalic,                    0, tab, 0);
    disconnect(ui.actionUnderline,                 0, tab, 0);
    disconnect(ui.actionStrikethrough,             0, tab, 0);
    disconnect(ui.actionSubscript,                 0, tab, 0);
    disconnect(ui.actionSuperscript,               0, tab, 0);
    disconnect(ui.actionAlignLeft,                 0, tab, 0);
    disconnect(ui.actionAlignCenter,               0, tab, 0);
    disconnect(ui.actionAlignRight,                0, tab, 0);
    disconnect(ui.actionAlignJustify,              0, tab, 0);
    disconnect(ui.actionInsertBulletedList,        0, tab, 0);
    disconnect(ui.actionInsertNumberedList,        0, tab, 0);
    disconnect(ui.actionDecreaseIndent,            0, tab, 0);
    disconnect(ui.actionIncreaseIndent,            0, tab, 0);
    disconnect(ui.actionTextDirectionLTR,          0, tab, 0);
    disconnect(ui.actionTextDirectionRTL,          0, tab, 0);
    disconnect(ui.actionTextDirectionDefault,      0, tab, 0);
    disconnect(ui.actionRemoveFormatting,          0, tab, 0);
    disconnect(ui.actionRemoveTagPair,             0, tab, 0);
    disconnect(ui.actionSplitSection,              0, tab, 0);
    disconnect(ui.actionInsertSGFSectionMarker,    0, tab, 0);
    disconnect(ui.actionInsertClosingTag,          0, tab, 0);
    disconnect(ui.actionGoToLinkOrStyle,           0, tab, 0);
    disconnect(ui.actionAddMisspelledWord,         0, tab, 0);
    disconnect(ui.actionIgnoreMisspelledWord,      0, tab, 0);
    disconnect(ui.actionPrintPreview,              0, tab, 0);
    disconnect(ui.actionPrint,                     0, tab, 0);
    disconnect(ui.actionAddToIndex,                0, tab, 0);
    disconnect(ui.actionMarkForIndex,              0, tab, 0);
}
