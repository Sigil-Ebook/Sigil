/************************************************************************
**
**  Copyright (C) 2016 Kevin B. Hendricks, Stratford, Ontario Canada
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
#include <QtCore/QSignalMapper>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtConcurrent>
#include <QFuture>
#include <QtGui/QDesktopServices>
#include <QtGui/QImage>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QInputDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QToolBar>
#include <QtWebKit/QWebSettings>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QWebView>
#include <QWebPage>
#include <QString>
#include <QStringList>
#include <QFont>
#include <QFontMetrics>

#include "BookManipulation/CleanSource.h"
#include "BookManipulation/Index.h"
#include "BookManipulation/FolderKeeper.h"
#include "Dialogs/About.h"
#include "Dialogs/ClipEditor.h"
#include "Dialogs/ClipboardHistorySelector.h"
#include "Dialogs/DeleteStyles.h"
#include "Dialogs/EditTOC.h"
#include "Dialogs/HeadingSelector.h"
#include "Dialogs/LinkStylesheets.h"
#include "Dialogs/MetaEditor.h"
#include "Dialogs/PluginRunner.h"
#include "Dialogs/Preferences.h"
#include "Dialogs/SearchEditor.h"
#include "Dialogs/SelectCharacter.h"
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
#include "Misc/KeyboardShortcutManager.h"
#include "Misc/Plugin.h"
#include "Misc/PluginDB.h"
#include "Misc/PythonRoutines.h"
#include "Misc/SettingsStore.h"
#include "Misc/SleepFunctions.h"
#include "Misc/SpellCheck.h"
#include "Misc/TempFolder.h"
#include "Misc/TOCHTMLWriter.h"
#include "Misc/Utility.h"
#include "Misc/QuickSerialHtmlParser.h"
#include "MiscEditors/IndexHTMLWriter.h"
#include "ResourceObjects/HTMLResource.h"
#include "ResourceObjects/NCXResource.h"
#include "ResourceObjects/OPFResource.h"
#include "ResourceObjects/NavProcessor.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "SourceUpdates/LinkUpdates.h"
#include "SourceUpdates/WordUpdates.h"
#include "Tabs/FlowTab.h"
#include "Tabs/OPFTab.h"
#include "Tabs/TabManager.h"

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

static const QString DONATE         = "http://sigil-ebook.com/donate";
static const QString SIGIL_WEBSITE  = "http://sigil-ebook.com";
static const QString USER_GUIDE_URL = "http://sigil-ebook.com/documentation";

static const QString BOOK_BROWSER_NAME            = "bookbrowser";
static const QString FIND_REPLACE_NAME            = "findreplace";
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

static const QStringList SUPPORTED_SAVE_TYPE = QStringList() << "epub";

static const QString DEFAULT_FILENAME = "untitled.epub";

QStringList MainWindow::s_RecentFiles = QStringList();

MainWindow::MainWindow(const QString &openfilepath, bool is_internal, QWidget *parent, Qt::WindowFlags flags)
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
    m_ViewState(MainWindow::ViewState_BookView),
    m_headingMapper(new QSignalMapper(this)),
    m_casingChangeMapper(new QSignalMapper(this)),
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
    m_LastPasteTarget(NULL),
    m_ZoomPreview(false),
    m_LastWindowSize(QByteArray()),
    m_PreviousHTMLResource(NULL),
    m_PreviousHTMLText(QString()),
    m_PreviousHTMLLocation(QList<ViewEditor::ElementIndex>()),
    m_menuPluginsInput(NULL),
    m_menuPluginsOutput(NULL),
    m_menuPluginsEdit(NULL),
    m_menuPluginsValidation(NULL),
    m_actionPlugin1(NULL),
    m_actionPlugin2(NULL),
    m_actionPlugin3(NULL),
    m_pluginList(QStringList()),
    m_SaveCSS(false)
{
    ui.setupUi(this);

    // Telling Qt to delete this window
    // from memory when it is closed
    setAttribute(Qt::WA_DeleteOnClose);
    ExtendUI();
    PlatformSpecificTweaks();
    // Needs to come before signals connect
    // (avoiding side-effects)
    ReadSettings();
    // Ensure the UI is properly set to the saved view state.
    SetDefaultViewState();
    SetupPreviewTimer();
    ConnectSignalsToSlots();
    CreateRecentFilesActions();
    UpdateRecentFileActions();
    ChangeSignalsWhenTabChanges(NULL, m_TabManager->GetCurrentContentTab());
    LoadInitialFile(openfilepath, is_internal);
    loadPluginsMenu();
}

MainWindow::~MainWindow()
{
    //This is an ugly hack: as the time of SpellCheck's death, being singleton,
    //is unknown, it cannot write its settings on exit because, most offen than not,
    //the infrastructure allowing it is already dead - so we let MW, on exit, do it.
    //TODO: somethng better
    SpellCheck::instance()->WriteSettings();
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
}


// Note on Mac OS X you may only add a QMenu or SubMenu to the MenuBar Once!
// Actions can be removed
void MainWindow::loadPluginsMenu()
{
    PluginDB *pdb = PluginDB::instance();

    m_menuPlugins = ui.menuPlugins;
    m_actionManagePlugins = ui.actionManage_Plugins;
    m_actionPlugin1 = ui.actionPlugin1;
    m_actionPlugin2 = ui.actionPlugin2;
    m_actionPlugin3 = ui.actionPlugin3;
    
    unloadPluginsMenu();

    connect(m_actionManagePlugins, SIGNAL(triggered()), this, SLOT(ManagePluginsDialog()));
    connect(m_actionPlugin1, SIGNAL(triggered()), this, SLOT(RunPlugin1()));
    connect(m_actionPlugin2, SIGNAL(triggered()), this, SLOT(RunPlugin2()));
    connect(m_actionPlugin3, SIGNAL(triggered()), this, SLOT(RunPlugin3()));

    updateToolTipsOnPluginIcons();

    QHash<QString, Plugin *> plugins = pdb->all_plugins();
    QStringList keys = plugins.keys();
    keys.sort();
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
    disconnect(m_actionPlugin1, SIGNAL(triggered()), this, SLOT(RunPlugin1()));
    disconnect(m_actionPlugin2, SIGNAL(triggered()), this, SLOT(RunPlugin2()));
    disconnect(m_actionPlugin3, SIGNAL(triggered()), this, SLOT(RunPlugin3()));
}

void MainWindow::runPlugin(QAction *action)
{
    QString pname = action->text();
    PluginRunner prunner(m_TabManager, this);
    prunner.exec(pname);
}

void MainWindow::SelectResources(QList<Resource *> resources)
{
    return m_BookBrowser->SelectResources(resources);
}


QList <Resource *> MainWindow::GetValidSelectedHTMLResources()
{
    return m_BookBrowser->ValidSelectedHTMLResources();
}


QList <Resource *> MainWindow::GetAllHTMLResources()
{
    return m_BookBrowser->AllHTMLResources();
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

void MainWindow::ResetLinkOrStyleBookmark()
{
    ResetLocationBookmark(m_LinkOrStyleBookmark);
}

void MainWindow::ResetLocationBookmark(MainWindow::LocationBookmark *locationBookmark)
{
    locationBookmark->filename.clear();
    ui.actionGoBackFromLinkOrStyle->setEnabled(false);
}

void MainWindow::GoBackFromLinkOrStyle()
{
    GoToBookmark(m_LinkOrStyleBookmark);
    UpdateBrowserSelectionToTab();
}

void MainWindow::GoToBookmark(MainWindow::LocationBookmark *locationBookmark)
{
    if (locationBookmark->filename.isEmpty()) {
        return;
    }

    try {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByFilename(locationBookmark->filename);
        // The following OpenResource call will switch ViewState if required to be changed.
        // However in order to ensure the caret is moved to the bookmark which may not be where
        // the tab was last clicked on we must switch view state now.
        SetViewState(locationBookmark->view_state);
        OpenResource(resource, -1, locationBookmark->cv_cursor_position, locationBookmark->bv_caret_location_update, locationBookmark->view_state);
        ShowMessageOnStatusBar();
    } catch (ResourceDoesNotExist) {
        // Nothing. Old file must have been deleted.
        ShowMessageOnStatusBar(tr("Navigation cancelled as location no longer exists."));
        ResetLocationBookmark(locationBookmark);
    }
}

void MainWindow::GoToPreviewLocation()
{
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
    m_LinkOrStyleBookmark->filename = current_resource->Filename();
    m_LinkOrStyleBookmark->view_state = m_ViewState;
    m_LinkOrStyleBookmark->cv_cursor_position = tab->GetCursorPosition();
    m_LinkOrStyleBookmark->bv_caret_location_update = tab->GetCaretLocationUpdate();
    ui.actionGoBackFromLinkOrStyle->setEnabled(!m_LinkOrStyleBookmark->filename.isEmpty());
}

void MainWindow::OpenUrl(const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }

    BookmarkLinkOrStyleLocation();

    if (url.scheme().isEmpty() || url.scheme() == "file") {
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

        OpenResource(resource, line, -1, QString(), MainWindow::ViewState_Unknown, url.fragment());
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
                              MainWindow::ViewState view_state,
                              const QUrl &fragment,
                              bool precede_current_tab)
{
    if (view_state == MainWindow::ViewState_Unknown) {
        view_state = m_ViewState;
    }

    m_TabManager->OpenResource(resource, line_to_scroll_to, position_to_scroll_to, caret_location_to_scroll_to,
                              view_state, fragment, precede_current_tab);

    if (view_state != m_ViewState) {
        SetViewState(view_state);
    }
}

void MainWindow::OpenResourceAndWaitUntilLoaded(Resource *resource,
        int line_to_scroll_to,
        int position_to_scroll_to,
        const QString &caret_location_to_scroll_to,
        MainWindow::ViewState view_state,
        const QUrl &fragment,
        bool precede_current_tab)
{
    OpenResource(resource, line_to_scroll_to, position_to_scroll_to, caret_location_to_scroll_to, view_state, fragment, precede_current_tab);
    while (!GetCurrentContentTab()->IsLoadingFinished()) {
        qApp->processEvents();
        SleepFunctions::msleep(100);
    }
}

void MainWindow::ResourceUpdatedFromDisk(Resource *resource)
{
    SettingsStore ss;
    QString message = QString(tr("File")) + " " + resource->Filename() + " " + tr("was updated") + ".";
    int duration = 10000;

    if (resource->Type() == Resource::HTMLResourceType) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (!m_Book->IsDataOnDiskWellFormed(html_resource)) {
            OpenResource(resource, -1, -1, QString(), MainWindow::ViewState_CodeView);
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

    if (!m_LastOpenFileWarnings.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(ShowLastOpenFileWarnings()));
    }
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    // Workaround for Qt 4.8 bug - see WriteSettings() for details.
    if (!isMaximized()) {
        m_LastWindowSize = saveGeometry();
    }

    QMainWindow::moveEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    // Workaround for Qt 4.8 bug - see WriteSettings() for details.
    if (!isMaximized()) {
        m_LastWindowSize = saveGeometry();
    }

    QMainWindow::resizeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (MaybeSaveDialogSaysProceed()) {
        ShowMessageOnStatusBar(tr("Sigil is closing..."));
        WriteSettings();
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
            m_PreviewWindow->hide();
        }
        event->accept();
    } else {
        event->ignore();
    }
}


void MainWindow::New()
{
    // The nasty IFDEFs are here to enable the multi-document
    // interface on the Mac; Lin and Win just use multiple
    // instances of the Sigil application
#ifndef Q_OS_MAC
    if (MaybeSaveDialogSaysProceed())
#endif
    {
#ifdef Q_OS_MAC
        MainWindow *new_window = new MainWindow();
        new_window->show();
#else
        CreateNewBook();
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
        QString filename = QFileDialog::getOpenFileName(this,
                           tr("Open File"),
                           m_LastFolderOpen,
                           filter_string,
                           &default_filter
                                                       );

        if (!filename.isEmpty()) {
#ifdef Q_OS_MAC
            MainWindow *new_window = new MainWindow(filename);
            new_window->show();
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

    QString filename = QFileDialog::getSaveFileName(this,
                       tr("Save File"),
                       save_path,
                       filter_string,
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
                       & default_filter,
                       QFileDialog::DontUseNativeDialog
#else
                       & default_filter
#endif
                                                   );

    if (filename.isEmpty()) {
        m_CurrentFilePath.clear();
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
    QString filename = QFileDialog::getSaveFileName(this,
                       tr("Save a Copy"),
                       m_SaveACopyFilename,
                       filter_string,
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
                       & default_filter,
                       QFileDialog::DontUseNativeDialog
#else
                       & default_filter
#endif
                                                   );

    if (filename.isEmpty()) {
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

void MainWindow::Exit()
{
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
        OpenResource(tab->GetLoadedResource(), line, -1, QString(), MainWindow::ViewState_CodeView);
    }
}

void MainWindow::ViewImageDialog(const QUrl &url)
{
    // non-modal dialog
    m_ViewImage->show();
    m_ViewImage->raise();
    m_ViewImage->activateWindow();

    QString image_path = url.toString();
    QString filename = QFileInfo(image_path).fileName();

    try {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByFilename(filename);
        if (resource->Type() == Resource::ImageResourceType || resource->Type() == Resource::SVGResourceType) {
            m_ViewImage->ShowImage(resource->GetFullPath());
        }
    } catch (ResourceDoesNotExist) {
        QMessageBox::warning(this, tr("Sigil"), tr("Image does not exist: ") + image_path);
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
        foreach(QString pathname, stylesheets) {
            // Check whether the stylesheet contains this style
            CSSResource *css_resource = NULL;
            foreach(Resource * resource, css_resources) {
                if (pathname == QString("../" + resource->GetRelativePathToOEBPS())) {
                    // We have our resource matching this stylesheet.
                    css_resource = dynamic_cast<CSSResource *>(resource);

                    if (!first_css_resource) {
                        first_css_resource = css_resource;
                    }

                    break;
                }
            }
            if (css_resource) {
                CSSInfo css_info(css_resource->GetText(), true);
                CSSInfo::CSSSelector *selector = css_info.getCSSSelectorForElementClass(element_name, style_class_name);

                // If we fail to find a matching class, search again with just the element
                if (!style_class_name.isEmpty() && !selector) {
                    selector = css_info.getCSSSelectorForElementClass(element_name, "");
                }

                if (selector) {
                    m_TabManager->OpenResource(css_resource, selector->line);
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
                display_name = QString("\".%1\" or \"%2.%1\"").arg(style_class_name).arg(element_name);
            }

            // Open the first linked stylesheet if any
            if (first_css_resource) {
                OpenResource(first_css_resource, 1);
            }

            ShowMessageOnStatusBar(QString(tr("No CSS styles named") +  " " + display_name + " found, " + "or stylesheet not linked."), 7000);

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
    // See https://bugreports.qt-project.org/browse/QTBUG-4350
    // QWebSettinbgs::clearMemoryCaches();

    // replace the above with a similar sequence 
    // that does not invalidate the fontCache

    // toggle memory caches to disable and then re-enable
    QWebSettings::setObjectCacheCapacities(0,0,0);
    QWebSettings::setObjectCacheCapacities(0, 0, 100 * 1024 * 1024);

    // do the same to flush the page cache
    int numpages = QWebSettings::maximumPagesInCache();
    QWebSettings::setMaximumPagesInCache(0);
    QWebSettings::setMaximumPagesInCache(numpages);
}


bool MainWindow::ProceedWithUndefinedUrlFragments()
{
    std::tuple<bool, QString, QString, QString, QString> result = m_Book->HasUndefinedURLFragments();
    if (std::get<0>(result)) {
        QMessageBox::StandardButton button_pressed;
        const QString &msg = tr("<html><p>The url fragment <b>%1</b> in the href <b>%2</b> (found in <b>%3</b>) does not exist in the target file (<b>%4</b>) "
                                "(and there may be more). Splitting or merging under these conditions can result in broken links.</p>"
                                "<p>Do you still wish to continue?</p></html>");

        button_pressed = QMessageBox::warning(this, tr("Sigil"),
                                msg.arg(std::get<1>(result), std::get<2>(result), std::get<3>(result), std::get<4>(result)),
                                QMessageBox::Yes | QMessageBox::No);
        if (button_pressed != QMessageBox::Yes) {
            return false;
        }
        return true;
    }
    return true;
}


void MainWindow::AddCover()
{
    // Get the image to use.
    QStringList selected_files;
    // Get just images, not svg files.
    QList<Resource *> image_resources = m_Book->GetFolderKeeper()->GetResourceListByType(Resource::ImageResourceType);
    QString title = tr("Add Cover");
    SelectFiles select_files(title, image_resources, m_LastInsertedFile, this);

    if (select_files.exec() == QDialog::Accepted) {
        if (select_files.IsInsertFromDisk()) {
            QStringList filenames = m_BookBrowser->AddExisting(false, true);
            // Convert full path to filename.
            foreach(QString filename, filenames) {
                QString internal_filename = filename.right(filename.length() - filename.lastIndexOf("/") - 1);
                selected_files.append(internal_filename);
            }
        } else {
            selected_files = select_files.SelectedImages();
        }
    }
    if (selected_files.count() == 0) {
        return;
    }
    QString image_filename = selected_files.first();

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
            if (m_Book->GetOPF()->GetGuideSemanticCodeForResource(html_resource) == "cover") {
                html_cover_resource = html_resource;
            } else if (resource->Filename().toLower() == HTML_COVER_FILENAME && html_cover_resource == NULL) {
                html_cover_resource = html_resource;
            }
        }
    }

    // Populate the HTML cover file with the necessary text.
    // If a template file exists, use its text for the cover source.
    QString version = m_Book->GetOPF()->GetEpubVersion();
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
        Resource *image_resource = m_Book->GetFolderKeeper()->GetResourceByFilename(image_filename);

        // Set cover semantics
        if (version.startsWith('3')) {
            NavProcessor navproc(m_Book->GetOPF()->GetNavResource());
            navproc.AddLandmarkCode(html_cover_resource, "cover", false);
        }
        m_Book->GetOPF()->AddGuideSemanticCode(html_cover_resource, "cover", false);
        
        ImageResource *image_type_resource = qobject_cast<ImageResource *>(image_resource);
        if (image_type_resource) {
            if (!m_Book->GetOPF()->IsCoverImage(image_type_resource)) {
                m_Book->GetOPF()->SetResourceAsCoverImage(image_type_resource);
            }

            // Add the filename and dimensions of the image to the HTML source.
            QString image_relative_path = "../" + image_resource->GetRelativePathToOEBPS();
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
        }
    } catch (ResourceDoesNotExist) {
        //
    }

    m_BookBrowser->Refresh();
    m_Book->SetModified();
    clearMemoryCaches();
    OpenResourceAndWaitUntilLoaded(html_cover_resource);
    // Reload the tab to ensure it reflects updated image.
    FlowTab *flow_tab = GetCurrentFlowTab();
    if (flow_tab) {
        flow_tab->LoadTabContent();
        flow_tab->ScrollToTop();
    }

    ShowMessageOnStatusBar(tr("Cover added."));
    QApplication::restoreOverrideCursor();
}


void MainWindow::UpdateManifestProperties()
{
    QString version = m_Book->GetConstOPF()->GetEpubVersion();
    if (!version.startsWith('3')) {
        ShowMessageOnStatusBar(tr("Not Available for epub2."));
        return;
    }
    SaveTabData();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QList<Resource *> resources = GetAllHTMLResources();
    m_Book->GetOPF()->UpdateManifestProperties(resources);
    m_Book->SetModified();
    ShowMessageOnStatusBar(tr("OPF Manifest Properties Updated."));
    QApplication::restoreOverrideCursor();
}


void MainWindow::GenerateNCXFromNav()
{
    QString version = m_Book->GetConstOPF()->GetEpubVersion();
    if (!version.startsWith('3')) {
        ShowMessageOnStatusBar(tr("Not Available for epub2."));
        return;
    }

    SaveTabData();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // find existing nav document if there is one
    HTMLResource * nav_resource = m_Book->GetConstOPF()->GetNavResource();
    QString navname = "";
    QString navdata = "";
    if (nav_resource) {
        navdata = CleanSource::Mend(nav_resource->GetText(),"3.0");
        navname = nav_resource->Filename();
    }

    if ((!nav_resource) || navdata.isEmpty()) {
        ShowMessageOnStatusBar(tr("NCX generation failed."));
        QApplication::restoreOverrideCursor();
    }

    QList<QVariant> mvalues = m_Book->GetConstOPF()->GetDCMetadataValues("dc:title");
    QString doctitle = "UNKNOWN";
    if (!mvalues.isEmpty()) {
        doctitle = mvalues.at(0).toString();
    } 
    QString mainid = m_Book->GetConstOPF()->GetMainIdentifierValue();

    // Now build the ncx in python in a separate thread since may be an long job
    PythonRoutines pr;
    QFuture<QString> future = QtConcurrent::run(&pr, &PythonRoutines::GenerateNcxInPython, navdata, navname, doctitle, mainid);
    future.waitForFinished();
    QString ncxdata = future.result();

    if (!ncxdata.isEmpty()) {
        NCXResource * ncx_resource = m_Book->GetNCX();
        ncx_resource->SetText(ncxdata);
        ncx_resource->SaveToDisk();
        m_TableOfContents->Refresh();
        m_BookBrowser->BookContentModified();
        m_BookBrowser->Refresh();
        m_Book->SetModified();

        // now update the guide entries as well
        PythonRoutines pr;
        QList<QStringList> results = pr.UpdateGuideFromNavInPython(navdata, navname);
        foreach(QStringList entry, results) {
           QString gtype = entry.at(0);
           QString href  = entry.at(1);
           if (!gtype.isEmpty() && !href.isEmpty()) {
               // remove any fragment identifier and extract file name
               QStringList parts = href.split('#', QString::KeepEmptyParts);
               QString base = parts.at(0);
               QStringList path_pieces = base.split("/");
               int last = path_pieces.count() - 1;
               QString filename = path_pieces.at(last);
               Resource * resource = m_Book->GetFolderKeeper()->GetResourceByFilename(filename);
               HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
               if (html_resource) {
                   // The OPFResource AddSemantic code defaults to remove by adding again (ie. toggling)
                   // Setting the last parameter (toggle) to false disables toggling
                   m_Book->GetOPF()->AddGuideSemanticCode(html_resource, gtype, false); 
               }
           }
        }

        ShowMessageOnStatusBar(tr("NCX generated."));
        QApplication::restoreOverrideCursor();
        return;
    }

    ShowMessageOnStatusBar(tr("NCX generation failed."));
    QApplication::restoreOverrideCursor();
}


void MainWindow::CreateIndex()
{
    SaveTabData();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // If Index CSS file does not exist create one.
    bool found_css = false;
    foreach(Resource *resource, m_BookBrowser->AllCSSResources()) {
        if (resource->Filename() == SGC_INDEX_CSS_FILENAME) {
            found_css = true;
        }
    }

    // If Index CSS file does not exist look for a default file
    // in preferences directory and if none create one.
    if (!found_css) {
        QString css_path = Utility::DefinePrefsDir() + "/" + SGC_INDEX_CSS_FILENAME;
        if (QFile::exists(css_path)) {
            m_BookBrowser->AddFile(css_path);
        } else {
            m_BookBrowser->CreateIndexCSSFile();
        }
    }

    HTMLResource *index_resource = NULL;
    QList<HTMLResource *> html_resources;
    // Turn the list of Resources that are really HTMLResources to a real list
    // of HTMLResources.
    QList<Resource *> resources = GetAllHTMLResources();
    HTMLResource *toc_resource = NULL;
    foreach(Resource * resource, resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);

        if (html_resource) {
            html_resources.append(html_resource);

            // Check if this is an existing index file.
            QString guide_semantic = m_Book->GetOPF()->GetGuideSemanticCodeForResource(html_resource);

            if (guide_semantic == "index") {
                index_resource = html_resource;
            } else if (resource->Filename() == HTML_INDEX_FILE && html_resource == NULL) {
                index_resource = html_resource;
            }
            // determine any epub2 toc resource to skip it when indexing
            if (guide_semantic == "toc") {
                toc_resource = html_resource;
            }
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
        html_resources.append(index_resource);
        m_Book->GetOPF()->UpdateSpineOrder(html_resources);
    }

    // Skip indexing the index page itself
    html_resources.removeOne(index_resource);

    // Skip indexing any epub3 nav document
    HTMLResource * nav_resource = m_Book->GetConstOPF()->GetNavResource();
    if (nav_resource) {
        html_resources.removeOne(nav_resource);
    }

    // Skip indexing any epub2 toc document
    if (toc_resource) {
        html_resources.removeOne(toc_resource);
    }

    // Scan the book, add ids for any tag containing at least one index entry and store the
    // document index entry at the same time (including custom and from the index editor).
    if (!Index::BuildIndex(html_resources)) {
        return;
    }

    // Write out the HTML index file.
    QString version = index_resource->GetEpubVersion();
    IndexHTMLWriter index;
    index_resource->SetText(index.WriteXML(version));

    // Normally Setting a semantic on a resource that already has it set will remove the semantic.
    //  Pass along toggle as false to disable this default behaviour
    m_Book->GetOPF()->AddGuideSemanticCode(index_resource, "index", false);
    if (version.startsWith('3')) {
      NavProcessor navproc(m_Book->GetOPF()->GetNavResource());
        navproc.AddLandmarkCode(index_resource, "index", false);
    }

    m_Book->SetModified();
    m_BookBrowser->Refresh();
    OpenResource(index_resource);
    QApplication::restoreOverrideCursor();
}

void MainWindow::DeleteReportsStyles(QList<BookReports::StyleData *> reports_styles_to_delete)
{
    // Convert the styles to CSS Selectors
    QHash<QString, QList<CSSInfo::CSSSelector *>> css_styles_to_delete;
    foreach(BookReports::StyleData * report_style, reports_styles_to_delete) {
        CSSInfo::CSSSelector *selector = new CSSInfo::CSSSelector();
        selector->groupText = report_style->css_selector_text;
        selector->line = report_style->css_selector_line;
        QString css_short_filename = report_style->css_filename;
        css_short_filename = css_short_filename.right(css_short_filename.length() - css_short_filename.lastIndexOf('/') - 1);
        css_styles_to_delete[css_short_filename].append(selector);
    }
    // Confirm which styles to delete
    DeleteStyles delete_styles(css_styles_to_delete, this);
    connect(&delete_styles, SIGNAL(OpenFileRequest(QString, int)), this, SLOT(OpenFile(QString, int)));

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
    SaveTabData();
    if (!m_Book.data()->GetNonWellFormedHTMLFiles().isEmpty()) {
        QMessageBox::warning(this, tr("Sigil"), tr("Reports cancelled due to XML not well formed."));
        return;
    }
    // non-modal dialog
    m_Reports->CreateReports(m_Book);
    m_Reports->show();
    m_Reports->raise();
    m_Reports->activateWindow();
}

void MainWindow::OpenFile(QString filename, int line)
{
    if (filename.isEmpty()) {
        return;
    }

    if (line < 1) {
        line = 1;
    }

    try {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByFilename(filename);
        OpenResource(resource, line);
    } catch (ResourceDoesNotExist) {
        //
    }
}


void MainWindow::DeleteFilenames(QStringList files_to_delete)
{
    if (files_to_delete.count() <= 0) {
        return;
    }

    QList <Resource *> resources;
    foreach(QString filename, files_to_delete) {
        try {
            Resource *resource = m_Book->GetFolderKeeper()->GetResourceByFilename(filename);
            resources.append(resource);
        } catch (ResourceDoesNotExist) {
            continue;
        }
    }
    RemoveResources(resources);
}

bool MainWindow::DeleteCSSStyles(const QString &filename, QList<CSSInfo::CSSSelector *> css_selectors)
{
    // Save our tabs data as we will be modifying the underlying resources
    SaveTabData();
    bool is_modified = false;
    bool is_found = false;
    // Try our CSS resources first as most likely place for a style
    QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();
    foreach(Resource * resource, css_resources) {
        if (resource->Filename() == filename) {
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
            if (resource->Filename() == filename) {
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

void MainWindow::DeleteUnusedMedia()
{
    SaveTabData();
    if (!m_Book.data()->GetNonWellFormedHTMLFiles().isEmpty()) {
        QMessageBox::warning(this, tr("Sigil"), tr("Delete Unused Media Files cancelled due to XML not well formed."));
        return;
    }

    QList<Resource *> resources;
    QHash<QString, QStringList> html_files_hash = m_Book->GetHTMLFilesUsingMedia();

    // Get file urls from HTML inline styles
    QStringList style_urls = m_Book->GetStyleUrlsInHTMLFiles();

    // Get files urls from CSS files
    QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();
    foreach(Resource *resource, css_resources) {
        CSSResource *css_resource = dynamic_cast<CSSResource *>(resource);
        CSSInfo css_info(css_resource->GetText(), true);

        style_urls.append(css_info.getAllPropertyValues(""));
    }
    // Get file urls from HTML CSS files
    QList<Resource *> html_resources = GetAllHTMLResources();
    foreach(Resource *resource, html_resources) {
        HTMLResource *html_resource = dynamic_cast<HTMLResource *>(resource);
        CSSInfo css_info(html_resource->GetText(), false);

        style_urls.append(css_info.getAllPropertyValues(""));
    }

    style_urls.removeDuplicates();

    QStringList style_url_files;
    QRegularExpression url_file_search("url\\s*\\(\\s*['\"]?([^\\(\\)'\"]*)[\"']?\\)");
    foreach (QString url, style_urls) {
        QRegularExpressionMatch match = url_file_search.match(url);
        if (match.hasMatch()) {
            style_url_files.append(match.captured(1));
        }
    }

    foreach(Resource * resource, m_BookBrowser->AllMediaResources()) {
        QString filepath = "../" + resource->GetRelativePathToOEBPS();

        // Include the file in the list to delete if it was not referenced
        if (html_files_hash[filepath].count() == 0 && !style_url_files.contains(filepath)) {
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
        QMessageBox::information(this, tr("Sigil"), tr("There are no unused image, video or audio files to delete."));
    }
}

void MainWindow::DeleteUnusedStyles()
{
    SaveTabData();
    if (!m_Book.data()->GetNonWellFormedHTMLFiles().isEmpty()) {
        QMessageBox::warning(this, tr("Sigil"), tr("Delete Unused Styles cancelled due to XML not well formed."));
        return;
    }

    QList<BookReports::StyleData *> html_class_usage = BookReports::GetAllHTMLClassUsage(m_Book, true);
    QList<BookReports::StyleData *> css_selector_usage = BookReports::GetCSSSelectorUsage(m_Book, html_class_usage);
    qDeleteAll(html_class_usage);
    QList<BookReports::StyleData *> css_selectors_to_delete;
    foreach(BookReports::StyleData *selector, css_selector_usage) {
        if (selector->html_filename.isEmpty()) {
            css_selectors_to_delete.append(selector);
        }
    }

    if (css_selectors_to_delete.count() > 0) {
        DeleteReportsStyles(css_selectors_to_delete);
    } else {
        QMessageBox::information(this, tr("Sigil"), tr("There are no unused stylesheet classes to delete."));
    }
    qDeleteAll(css_selector_usage);
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

void MainWindow::InsertFiles(const QStringList &selected_files)
{
    if (!selected_files.isEmpty()) {

        FlowTab *flow_tab = GetCurrentFlowTab();
        if (!flow_tab) {
            return;
        }

        if (flow_tab->InsertFileEnabled()) {
            foreach(QString selected_file, selected_files) {
                try {
                    Resource *resource = m_Book->GetFolderKeeper()->GetResourceByFilename(selected_file);
                    QString relative_path = "../" + resource->GetRelativePathToOEBPS();

                    QString filename = relative_path.right(relative_path.length() - relative_path.lastIndexOf("/") - 1);
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
                } catch (ResourceDoesNotExist) {
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
    disconnect(m_BookBrowser, SIGNAL(ResourcesAdded()), this, SLOT(ResourcesAddedOrDeleted()));
    QStringList filenames = m_BookBrowser->AddExisting(true);
    connect(m_BookBrowser, SIGNAL(ResourcesAdded()), this, SLOT(ResourcesAddedOrDeleted()));
    // Since we disconnected the signal we will have missed forced clearing of cache
    clearMemoryCaches();
    QStringList internal_filenames;
    foreach(QString filename, filenames) {
        QString internal_filename = filename.right(filename.length() - filename.lastIndexOf("/") - 1);
        internal_filenames.append(internal_filename);
    }
    InsertFiles(internal_filenames);
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

    // Prevent adding a hidden anchor id in Book View.
    if (m_ViewState == MainWindow::ViewState_BookView && id.isEmpty() && flow_tab->GetSelectedText().isEmpty()) {
        QMessageBox::warning(this, tr("Sigil"), tr("You must select text before inserting a new id."));
        return;
    }

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

    // Prevent adding a hidden anchor link in Book View.
    if (m_ViewState == MainWindow::ViewState_BookView && href.isEmpty() && flow_tab->GetSelectedText().isEmpty()) {
        QMessageBox::warning(this, tr("Sigil"), tr("You must select text before inserting a new link."));
        return;
    }

    HTMLResource *html_resource = qobject_cast<HTMLResource *>(flow_tab->GetLoadedResource());
    QList<Resource *> resources = GetAllHTMLResources() + m_BookBrowser->AllMediaResources();
    SelectHyperlink select_hyperlink(href, html_resource, resources, m_Book, this);

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

void MainWindow::RunPlugin1()
{
    SettingsStore ss;
    QStringList namemap = ss.pluginMap();
    if (namemap.count() > 0) {
        QString pname = namemap.at(0);
        if (m_pluginList.contains(pname)) {
            PluginRunner prunner(m_TabManager, this);
            prunner.exec(pname);
        }
    }
}

void MainWindow::RunPlugin2()
{
    SettingsStore ss;
    QStringList namemap = ss.pluginMap();
    if (namemap.count() > 1) {
        QString pname = namemap.at(1);
        if (m_pluginList.contains(pname)) {
            PluginRunner prunner(m_TabManager, this);
            prunner.exec(pname);
        }
    }
}

void MainWindow::RunPlugin3()
{
    SettingsStore ss;
    QStringList namemap = ss.pluginMap();
    if (namemap.count() > 2) {
        QString pname = namemap.at(2);
        if (m_pluginList.contains(pname)) {
            PluginRunner prunner(m_TabManager, this);
            prunner.exec(pname);
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

void MainWindow::PasteClip1IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(1);
}

void MainWindow::PasteClip2IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(2);
}

void MainWindow::PasteClip3IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(3);
}

void MainWindow::PasteClip4IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(4);
}

void MainWindow::PasteClip5IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(5);
}

void MainWindow::PasteClip6IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(6);
}

void MainWindow::PasteClip7IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(7);
}

void MainWindow::PasteClip8IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(8);
}

void MainWindow::PasteClip9IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(9);
}

void MainWindow::PasteClip10IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(10);
}

void MainWindow::PasteClip11IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(11);
}

void MainWindow::PasteClip12IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(12);
}

void MainWindow::PasteClip13IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(13);
}

void MainWindow::PasteClip14IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(14);
}

void MainWindow::PasteClip15IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(15);
}

void MainWindow::PasteClip16IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(16);
}

void MainWindow::PasteClip17IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(17);
}

void MainWindow::PasteClip18IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(18);
}

void MainWindow::PasteClip19IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(19);
}

void MainWindow::PasteClip20IntoCurrentTarget()
{
    PasteClipIntoCurrentTarget(20);
}

void MainWindow::PasteClipEntriesIntoCurrentTarget(const QList<ClipEditorModel::clipEntry *> &clips)
{
    if (m_LastPasteTarget == NULL) {
        ShowMessageOnStatusBar(tr("Select the destination to paste into first."));
        return;
    }

    bool applied = m_LastPasteTarget->PasteClipEntries(clips);

    if (applied) {
        // Clear the statusbar afterwards but only if entries were pasted.
        ShowMessageOnStatusBar();
    }
}

void MainWindow::PasteClipEntriesIntoPreviousTarget(const QList<ClipEditorModel::clipEntry *> &clips)
{
    FlowTab *flow_tab = GetCurrentFlowTab();
    if (flow_tab && flow_tab->GetLoadedResource()->Type() == Resource::HTMLResourceType) {
        bool applied = flow_tab->PasteClipEntries(clips);
        if (applied) {
            flow_tab->setFocus();
            // Clear the statusbar afterwards but only if entries were pasted.
            ShowMessageOnStatusBar();
        }
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
        button_pressed = QMessageBox::warning(this, tr("Sigil"), tr("Are you sure you want to merge the selected files?\nThis action cannot be reversed."), QMessageBox::Ok | QMessageBox::Cancel);

        if (button_pressed != QMessageBox::Ok) {
            return;
        }
    }

    // Check if data is well formed
    foreach (Resource *r, resources) {
        HTMLResource *h = qobject_cast<HTMLResource *>(r);
        if (!h) {
            continue;
        }
        if (!h->FileIsWellFormed()) {
            QMessageBox::warning(this, tr("Sigil"), tr("Merge cancelled: %1, XML not well formed.").arg(h->Filename()));
            return;
        }
    }
    if (!m_TabManager->IsAllTabDataWellFormed()) {
        QMessageBox::warning(this, tr("Sigil"), tr("Merge cancelled due to XML not well formed."));
        return;
    }

    // Handle warning the user about undefined url fragments.
    if (!ProceedWithUndefinedUrlFragments()) {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    // Close all tabs being updated to prevent BV overwriting the new data
    foreach(Resource *resource, resources) {
        m_TabManager->CloseTabForResource(resource);
    }
    // Display progress dialog
    Resource *resource_to_open = resources.first();
    Resource *failed_resource = m_Book->MergeResources(resources);

    if (failed_resource != NULL) {
        QApplication::restoreOverrideCursor();
        QMessageBox::critical(this, tr("Sigil"), tr("Cannot merge file %1").arg(failed_resource->Filename()));
        QApplication::setOverrideCursor(Qt::WaitCursor);
        resource_to_open = failed_resource;
    } else {
        m_BookBrowser->Refresh();
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
            QMessageBox::warning(this, tr("Sigil"), tr("Link Stylesheets cancelled: %1, XML not well formed.").arg(h->Filename()));
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

    // Close all tabs being updated to prevent BV overwriting the new data
    foreach(Resource *resource, resources) {
        m_TabManager->CloseTabForResource(resource);
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

void MainWindow::FindWord(QString lword)
{
    QStringList lw{lword.split(',')};
    QString lang=lw.first();
    QString word=lw.last();

    SaveTabData();
    SetViewState(MainWindow::ViewState_CodeView);

    // Note the current tab if it is an HTML file.
    HTMLResource *current_html_resource = NULL;
    QString current_html_filename;
    FlowTab *flow_tab = GetCurrentFlowTab();
    if (flow_tab) {
        Resource *resource = flow_tab->GetLoadedResource();
        if (resource->Type() == Resource::HTMLResourceType) {
            current_html_resource = qobject_cast<HTMLResource *>(resource);
            current_html_filename = current_html_resource->Filename();
        }
    }

    // Get list of files from current to end followed
    // by start to just before current file.
    QList<Resource *> html_resources;
    QList<Resource *> resources = GetAllHTMLResources();
    int passed_current = false;
    foreach(Resource *resource, resources) {
        if (!passed_current && resource->Filename() != current_html_filename) {
            continue;
        }
        passed_current = true;
        html_resources.append(resource);
    }
    foreach(Resource * resource, resources) {
        html_resources.append(resource);
        if (resource->Filename() == current_html_filename) {
            break;
        }
    }

    // Search for the word.
    bool done_current = false;
    foreach (Resource *resource, html_resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (!html_resource) {
            continue;
        }
        int start_pos = 0;
        // Reset the start to current cursor position only if this is the
        // first time we are in the current file.
        if (resource->Filename() == current_html_filename) {
            if (!done_current) {
                FlowTab *flow_tab = GetCurrentFlowTab();
                if (flow_tab) {
                    start_pos = flow_tab->GetCursorPosition();
                }
            }
            done_current = true;
        }
        QString text = html_resource->GetText();

        int found_pos = HTMLSpellCheck::WordPosition(text, word, lang, start_pos);
        if (found_pos >= 0) {
            if (resource->Filename() != current_html_filename) {
                OpenResourceAndWaitUntilLoaded(resource, -1, found_pos);
            }
            FlowTab *flow_tab = GetCurrentFlowTab();
            if (flow_tab) {
                flow_tab->HighlightWord(word, found_pos);
                break;
            }
        }
    }
}

void MainWindow::UpdateWord(QString old_word, QString new_word)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    SaveTabData();
    SetViewState(MainWindow::ViewState_CodeView);

    QList<HTMLResource *> html_resources;
    QList<Resource *> resources = GetAllHTMLResources();
    foreach(Resource * resource, resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (html_resource) {
            html_resources.append(html_resource);
        }
    }

    WordUpdates::UpdateWordInAllFiles(html_resources, old_word, new_word);
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
    QStringList checked_linked_paths = GetStylesheetsAlreadyLinked(resources.at(0));
    // Then only consider them included if every selected resource includes
    // the same stylesheets in the same order.
    foreach(Resource * valid_resource, resources) {
        QStringList linked_paths = GetStylesheetsAlreadyLinked(valid_resource);
        foreach(QString path, checked_linked_paths) {
            if (!linked_paths.contains(path)) {
                checked_linked_paths.removeOne(path);
            }
        }
    }
    // Save the paths included in all resources in order
    foreach(QString path, checked_linked_paths) {
        stylesheet_map.append(std::make_pair(path, true));
    }
    // Save all the remaining paths and mark them not included
    foreach(Resource * resource, css_resources) {
        QString pathname = "../" + resource->GetRelativePathToOEBPS();

        if (!checked_linked_paths.contains(pathname)) {
            stylesheet_map.append(std::make_pair(pathname, false));
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
        //existing_stylesheets.append( css_resource->Filename() );
        existing_stylesheets.append("../" + css_resource->GetRelativePathToOEBPS());
    }
    foreach(QString pathname, html_resource->GetLinkedStylesheets()) {
        pathname = Utility::URLDecodePath(pathname);
        // Only list the stylesheet if it exists in the book
        if (existing_stylesheets.contains(pathname)) {
            linked_stylesheets.append(pathname);
        }
    }
    return linked_stylesheets;
}

void MainWindow::RemoveResources(QList<Resource *> resources)
{
    // work around Qt bug when deleting png images on page shown by both
    // BookView and Preview by temporarily hiding the PreviewWindow
    bool pw_showing = m_PreviewWindow->IsVisible();
    if ((pw_showing) && (m_ViewState == MainWindow::ViewState_BookView)) {
        m_PreviewWindow->hide();
    }
    // Provide the open tab list to ensure one tab stays open
    if (resources.count() > 0) {
        m_BookBrowser->RemoveResources(m_TabManager->GetTabResources(), resources);
    } else {
        m_BookBrowser->RemoveSelection(m_TabManager->GetTabResources());
    }
    if ((pw_showing) && !m_PreviewWindow->IsVisible()) {
        m_PreviewWindow->show();
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

// For epub2 this set the NCX for epub3 this sets the Nac TOC section
void MainWindow::GenerateToc()
{
    SaveTabData();
    QList<Resource *> resources = GetAllHTMLResources();

    if (resources.isEmpty()) {
        return;
    }

    bool is_headings_changed = false;
    {
        HeadingSelector toc(m_Book, this);

        if (toc.exec() != QDialog::Accepted) {
            ShowMessageOnStatusBar(tr("Generate TOC cancelled."));
            return;
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
        is_toc_changed = m_Book->GetNCX()->GenerateNCXFromBookContents(m_Book.data());
    }
    if (is_headings_changed || is_toc_changed) {
        // Reload the current tab to see visual impact if user changed heading level(s)
        // It might not have been the current tab, but what the heck, possible user has the NCX open even.
        ResourcesAddedOrDeleted();
        m_Book.data()->SetModified();
        ShowMessageOnStatusBar(tr("Table Of Contents generated."));
    } else {
        ShowMessageOnStatusBar(tr("No Table Of Contents changes were necessary."));
    }

    QApplication::restoreOverrideCursor();
}


void MainWindow::CreateHTMLTOC()
{
    SaveTabData();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    bool found_css = false;
    foreach(Resource *resource, m_BookBrowser->AllCSSResources()) {
        if (resource->Filename() == SGC_TOC_CSS_FILENAME) {
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
            CSSResource *css_resource = qobject_cast<CSSResource *> (resource);
            css_resource->InitialLoad();
        } else {
            m_BookBrowser->CreateHTMLTOCCSSFile();
        }
    }

    HTMLResource *tocResource = NULL;
    QList<HTMLResource *> htmlResources;

    // Turn the list of Resources that are really HTMLResources to a real list
    // of HTMLResources.
    QList<Resource *> resources = GetAllHTMLResources();
    foreach(Resource * resource, resources) {
        HTMLResource *htmlResource = qobject_cast<HTMLResource *>(resource);

        if (htmlResource) {
            htmlResources.append(htmlResource);

            // Check if this is an existing toc file.
            if (m_Book->GetOPF()->GetGuideSemanticCodeForResource(htmlResource) == "toc") {
                tocResource = htmlResource;
            } else if (resource->Filename() == HTML_TOC_FILE && tocResource == NULL) {
                tocResource = htmlResource;
            }
        }
    }
    // Close the tab so the focus saving doesn't overwrite the text were
    // replacing in the resource.
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
    TOCHTMLWriter toc(m_TableOfContents->GetRootEntry());
    QString version = tocResource->GetEpubVersion();
    tocResource->SetText(toc.WriteXML(version));

    // Setting a semantic on a resource that already has it set will remove the semantic.
    // Unless you pass toggle as false as the final parameter
    // In epub3 only the nav should be marked as the toc so do not set this landmark in the nav
    m_Book->GetOPF()->AddGuideSemanticCode(tocResource, "toc", false);

    m_Book->SetModified();
    m_BookBrowser->Refresh();
    OpenResource(tocResource);
    QApplication::restoreOverrideCursor();
}


void MainWindow::ChangeCasing(int casing_mode)
{
    ContentTab *tab = GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    Utility::Casing casing;

    switch (casing_mode) {
        case Utility::Casing_Lowercase: {
            casing = Utility::Casing_Lowercase;
            break;
        }

        case Utility::Casing_Uppercase: {
            casing = Utility::Casing_Uppercase;
            break;
        }

        case Utility::Casing_Titlecase: {
            casing = Utility::Casing_Titlecase;
            break;
        }

        case Utility::Casing_Capitalize: {
            casing = Utility::Casing_Capitalize;
            break;
        }

        default:
            return;
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

void MainWindow::ToggleViewState()
{
    ContentTab *tab = GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    Resource::ResourceType type = tab->GetLoadedResource()->Type();

    if (type == Resource::HTMLResourceType) {
        if (m_ViewState == MainWindow::ViewState_CodeView) {
            SetViewState(MainWindow::ViewState_BookView);
        } else {
            SetViewState(MainWindow::ViewState_CodeView);
        }
    }

    UpdateBrowserSelectionToTab();
}

void MainWindow::BookView()
{
    SetViewState(MainWindow::ViewState_BookView);
    UpdateBrowserSelectionToTab();
}

void MainWindow::CodeView()
{
    SetViewState(MainWindow::ViewState_CodeView);
    UpdateBrowserSelectionToTab();
}


MainWindow::ViewState MainWindow::GetViewState()
{
    return m_ViewState;
}

void MainWindow::AnyCodeView()
{
    SetViewState(MainWindow::ViewState_CodeView);
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
    Preferences preferences(this);
    preferences.exec();

    if (preferences.isReloadTabsRequired()) {
        m_TabManager->ReopenTabs(m_ViewState);
    } else if (preferences.isRefreshSpellingHighlightingRequired()) {
        RefreshSpellingHighlighting();
        // Make sure menu state is set
        SettingsStore settings;
        ui.actionAutoSpellCheck->setChecked(settings.spellCheck());
    }

    if (m_SelectCharacter->isVisible()) {
        // To ensure any font size changes are immediately applied.
        m_SelectCharacter->show();
    }

    updateToolTipsOnPluginIcons();
}


void MainWindow::ManagePluginsDialog()
{
    Preferences preferences(this);
    preferences.makeActive(Preferences::PluginsPrefs);
    preferences.exec();

    // other preferences may have been changed as well
    if (preferences.isReloadTabsRequired()) {
        m_TabManager->ReopenTabs(m_ViewState);
    } else if (preferences.isRefreshSpellingHighlightingRequired()) {
        RefreshSpellingHighlighting();
        // Make sure menu state is set
        SettingsStore settings;
        ui.actionAutoSpellCheck->setChecked(settings.spellCheck());
    }

    if (m_SelectCharacter->isVisible()) {
        // To ensure any font size changes are immediately applied.
        m_SelectCharacter->show();
    }

    updateToolTipsOnPluginIcons();
}

void MainWindow::updateToolTipsOnPluginIcons()
{
    SettingsStore ss;
    QStringList namemap = ss.pluginMap();
    QString pname1 = tr("RunPlugin1");
    QString pname2 = tr("RunPlugin2");
    QString pname3 = tr("RunPlugin3");
    if (namemap.count() > 0) pname1 = namemap.at(0);
    if (namemap.count() > 1) pname2 = namemap.at(1);
    if (namemap.count() > 2) pname3 = namemap.at(2);
    m_actionPlugin1->setToolTip(pname1);
    m_actionPlugin2->setToolTip(pname2);
    m_actionPlugin3->setToolTip(pname3);
}

void MainWindow::WellFormedCheckEpub()
{
    m_ValidationResultsView->ValidateCurrentBook();
}


bool MainWindow::CharLessThan(const QChar &s1, const QChar &s2)
{
    return s1 < s2;
}


void MainWindow::ValidateStylesheetsWithW3C()
{
    SaveTabData();
    QList<Resource *> css_resources = m_BookBrowser->AllCSSResources();

    if (css_resources.isEmpty()) {
        ShowMessageOnStatusBar(tr("This EPUB does not contain any CSS stylesheets to validate."));
        return;
    }

    foreach(Resource * resource, css_resources) {
        CSSResource *css_resource = qobject_cast<CSSResource *>(resource);
        css_resource->ValidateStylesheetWithW3C();
    }
}


void MainWindow::ChangeSignalsWhenTabChanges(ContentTab *old_tab, ContentTab *new_tab)
{
    BreakTabConnections(old_tab);
    MakeTabConnections(new_tab);
    // Clear selection if the tab changed.
    ClearMarkedText(old_tab);
}


void MainWindow::SetViewState(MainWindow::ViewState view_state)
{
    // This function is called when:
    // 1. Invoked from OpenResource() to open a new tab or programmatically switch to existing one.
    // 2. When the user toggles view state, or uses the specific view state icons on toolbar
    // 3. To return to a bookmarked location
    // OpenResource() ensures that any attempt to open a tab with a viewstate of "Unknown"
    // will instead inherit the current MainWindow setting for the last FlowTab used.
    // The only other places that call this function directly all use the m_ViewState
    // value for view_state parameter, hence can never be "Unknown" in this function.
    Q_ASSERT(view_state != MainWindow::ViewState_Unknown);
    bool set_tab_state = m_ViewState != view_state;
    m_ViewState = view_state;
    UpdateViewState(set_tab_state);
}


void MainWindow::UpdateViewState(bool set_tab_state)
{
    ContentTab *tab = GetCurrentContentTab();

    if (tab == NULL) {
        return;
    }

    Resource::ResourceType type = tab->GetLoadedResource()->Type();

    if (type == Resource::HTMLResourceType) {
        if (set_tab_state) {
            FlowTab *ftab = dynamic_cast<FlowTab *>(tab);

            if (ftab) {
                bool view_state_changed = ftab->SetViewState(m_ViewState);
                // We cannot reliably use Qt focus events to determine whether or
                // not to reload the contents of a tab.
                ftab->ReloadTabIfPending();

                if (!view_state_changed) {
                    // Either we were already in this view, the data is not well formed or tab is still loading.
                    // We must force the UI to be in the state that this tab is now in.
                    m_ViewState = ftab->GetViewState();
                }
            }
            ClearMarkedText();
        }

        if (m_ViewState == MainWindow::ViewState_CodeView) {
            SetStateActionsCodeView();
        } else {
            m_ViewState = MainWindow::ViewState_BookView;
            SetStateActionsBookView();
        }
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
    UpdateViewState();
    ClearMarkedText();
}


void MainWindow::UpdateUIOnTabCountChange()
{
    ui.actionNextTab       ->setEnabled(m_TabManager->GetTabCount() > 1);
    ui.actionPreviousTab   ->setEnabled(m_TabManager->GetTabCount() > 1);
    ui.actionCloseTab      ->setEnabled(m_TabManager->GetTabCount() > 1);
    ui.actionCloseOtherTabs->setEnabled(m_TabManager->GetTabCount() > 1);
}


void MainWindow::SetStateActionsBookView()
{
    ui.actionBookView->setChecked(true);
    ui.actionCodeView->setChecked(false);
    ui.actionBookView->setEnabled(true);
    ui.actionCodeView->setEnabled(true);
    ui.actionPrintPreview->setEnabled(true);
    ui.actionPrint->setEnabled(true);
    ui.actionSplitSection->setEnabled(true);
    ui.actionInsertSGFSectionMarker->setEnabled(true);
    ui.actionInsertFile->setEnabled(true);
    ui.actionInsertSpecialCharacter->setEnabled(true);
    ui.actionInsertId->setEnabled(true);
    ui.actionInsertHyperlink->setEnabled(true);
    ui.actionInsertClosingTag->setEnabled(false);
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
    ui.actionInsertBulletedList->setEnabled(true);
    ui.actionInsertNumberedList->setEnabled(true);
    ui.actionShowTag->setEnabled(true);
    ui.actionRemoveFormatting->setEnabled(true);
    ui.menuHeadings->setEnabled(true);
    ui.actionHeading1->setEnabled(true);
    ui.actionHeading2->setEnabled(true);
    ui.actionHeading3->setEnabled(true);
    ui.actionHeading4->setEnabled(true);
    ui.actionHeading5->setEnabled(true);
    ui.actionHeading6->setEnabled(true);
    ui.actionHeadingNormal->setEnabled(true);
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
    ui.actionMarkSelection->setEnabled(false);
    ui.menuSearchCurrentFile->setEnabled(true);
    ui.actionFindNextInFile->setEnabled(true);
    ui.actionReplaceNextInFile->setEnabled(true);
    ui.actionReplaceAllInFile->setEnabled(true);
    ui.actionCountInFile->setEnabled(true);
    ui.actionGoToLine->setEnabled(false);
    ui.actionGoToLinkOrStyle->setEnabled(false);
    ui.actionAddMisspelledWord->setEnabled(false);
    ui.actionIgnoreMisspelledWord->setEnabled(false);
    ui.actionAutoSpellCheck->setEnabled(false);
    UpdateUIOnTabChanges();
    m_FindReplace->ShowHide();
}

void MainWindow::SetStateActionsCodeView()
{
    ui.actionBookView->setChecked(false);
    ui.actionCodeView->setChecked(true);
    ui.actionBookView->setEnabled(true);
    ui.actionCodeView->setEnabled(true);
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
    ui.actionDecreaseIndent->setEnabled(false);
    ui.actionIncreaseIndent->setEnabled(false);
    ui.actionTextDirectionLTR    ->setEnabled(true);
    ui.actionTextDirectionRTL    ->setEnabled(true);
    ui.actionTextDirectionDefault->setEnabled(true);
    ui.actionInsertBulletedList->setEnabled(false);
    ui.actionInsertNumberedList->setEnabled(false);
    ui.actionShowTag->setEnabled(false);
    ui.actionRemoveFormatting->setEnabled(true);
    ui.menuHeadings->setEnabled(true);
    ui.actionHeading1->setEnabled(true);
    ui.actionHeading2->setEnabled(true);
    ui.actionHeading3->setEnabled(true);
    ui.actionHeading4->setEnabled(true);
    ui.actionHeading5->setEnabled(true);
    ui.actionHeading6->setEnabled(true);
    ui.actionHeadingNormal->setEnabled(true);
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
    ui.actionBookView->setChecked(false);
    ui.actionCodeView->setChecked(false);
    ui.actionBookView->setEnabled(false);
    ui.actionCodeView->setEnabled(false);
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
    ui.actionShowTag->setEnabled(false);
    ui.actionRemoveFormatting->setEnabled(false);
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
    ui.actionBookView->setChecked(false);
    ui.actionCodeView->setChecked(false);
    ui.actionBookView->setEnabled(false);
    ui.actionCodeView->setEnabled(false);
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
    ui.actionShowTag->setEnabled(false);
    ui.actionRemoveFormatting->setEnabled(false);
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
}

void MainWindow::UpdatePreviewRequest()
{
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

void MainWindow::UpdatePreview()
{
    m_PreviewTimer.stop();

    QString text;
    QList<ViewEditor::ElementIndex> location;
    HTMLResource *html_resource;

    ContentTab *tab = GetCurrentContentTab();
    if (tab != NULL) {

        // Save CSS if update requested from CSS tab
        if (m_SaveCSS) {
            m_SaveCSS = false;
            tab->SaveTabContent();
        }

        html_resource = qobject_cast<HTMLResource *>(tab->GetLoadedResource());
        if (!html_resource) {
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
                    return;
                }
                text = flow_tab->GetText();
                location = flow_tab->GetCaretLocation();
            } else {
                text = m_PreviousHTMLText;
                if (m_PreviousHTMLResource) {
                    location = m_PreviewWindow->GetCaretLocation();
                } else {
                    location = m_PreviousHTMLLocation;
                }
            }
            m_PreviousHTMLResource = html_resource;
            m_PreviousHTMLText = text;
            m_PreviousHTMLLocation = location;

            m_PreviewWindow->UpdatePage(html_resource->GetFullPath(), text, location);
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


void MainWindow::SetDefaultViewState()
{
    // Default will be BookView unless there is a persisted value.
    SettingsStore settings;
    int view_state_value = settings.viewState();

    switch (view_state_value) {
        case MainWindow::ViewState_CodeView:
            m_ViewState = static_cast<MainWindow::ViewState>(view_state_value);
            break;
    }
}

void MainWindow::SetAutoSpellCheck(bool new_state)
{
    SettingsStore settings;
    settings.setSpellCheck(new_state);
    emit SettingsChanged();
}

void MainWindow::MendPrettifyHTML()
{
    m_Book->ReformatAllHTML(false);
}

void MainWindow::MendHTML()
{
    m_Book->ReformatAllHTML(true);
}

void MainWindow::ClearIgnoredWords()
{
    SpellCheck *sc = SpellCheck::instance();
    sc->clearAllMLIgnoredWords();
    // Need to reload any tabs to force spelling to be run again in CodeView
    QApplication::setOverrideCursor(Qt::WaitCursor);
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

    HTMLResource * nav_resource = m_Book->GetConstOPF()->GetNavResource();
    if (nav_resource && nav_resource == originating_resource) {
        QMessageBox::warning(this, tr("Sigil"), tr("The Nav file cannot be split."));
        return;
    }

    HTMLResource *html_resource = m_Book->CreateSectionBreakOriginalResource(content, originating_resource);
    m_BookBrowser->Refresh();
    // Open the old shortened content in a new tab preceding the current one.
    // without grabbing focus
    OpenResource(html_resource, -1, -1, QString(), m_ViewState, QUrl(), true);
    FlowTab *flow_tab = GetCurrentFlowTab();

    // We will reload the reduced content tab to ensure reflects updated resource.
    if (flow_tab) {
        flow_tab->LoadTabContent();
        flow_tab->ScrollToTop();
    }

    ShowMessageOnStatusBar(tr("Split completed."));
}

void MainWindow::SplitOnSGFSectionMarkers()
{
    QList<Resource *> html_resources = GetAllHTMLResources();
    Resource * nav_resource = m_Book->GetConstOPF()->GetNavResource();
    if (nav_resource) {
        html_resources.removeOne(nav_resource);
    }

    SaveTabData();

    // If have the current tab is open in BV, make sure it has its content saved so it won't later overwrite a split.
    FlowTab *flow_tab = GetCurrentFlowTab();

    if (flow_tab && (flow_tab->GetViewState() == MainWindow::ViewState_BookView)) {
        flow_tab->SaveTabContent();
    }

    bool done_checking_frags = false;
    bool ignore_frags = false;
    foreach(Resource * resource, html_resources) {
        HTMLResource *html_resource = qobject_cast<HTMLResource *>(resource);
        if (!html_resource) {
            QMessageBox::warning(this, tr("Sigil"), tr("Cannot split since at least one file is not an HTML file."));
            return;
        }

        // Check if data is well formed before splitting.
        if (!html_resource->FileIsWellFormed()) {
            QMessageBox::warning(this, tr("Sigil"), tr("Cannot split: %1 XML is not well formed").arg(html_resource->Filename()));
            return;
        }

        // XXX: This should be using the mime type not the extension.
        if (!TEXT_EXTENSIONS.contains(QFileInfo(html_resource->Filename()).suffix().toLower())) {
            QMessageBox::warning(this, tr("Sigil"), tr("Cannot split since at least one file may not be an HTML file."));
            return;
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
        return;
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

        if (flow_tab && (flow_tab->GetViewState() == MainWindow::ViewState_BookView)) {
            // Our focus will have been moved to the book browser. Set it there and back to do
            // an equivalent of "GrabFocus()" to workaround Qt setFocus() not always working.
            m_BookBrowser->setFocus();
            flow_tab->setFocus();
        }
    } else {
        ShowMessageOnStatusBar(tr("No split file markers found. Use Insert->Split Marker."));
    }

    QApplication::restoreOverrideCursor();
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
    SettingsStore settings;
    ui.actionAutoSpellCheck->setChecked(settings.spellCheck());
    emit SettingsChanged();
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and its full screen status
    // Due to the 4.8 bug, we restore its "normal" window size and then maximize
    // it afterwards (if last state was maximized) to ensure on correct screen.
    bool isMaximized = settings.value("maximized", false).toBool();
    m_LastWindowSize = settings.value("geometry").toByteArray();

    if (!m_LastWindowSize.isNull()) {
        restoreGeometry(m_LastWindowSize);

        if (isMaximized) {
            setWindowState(windowState() | Qt::WindowMaximized);
        }
    }

    // The positions of all the toolbars and dock widgets
    QByteArray toolbars = settings.value("toolbars").toByteArray();

    if (!toolbars.isNull()) {
        restoreState(toolbars);
    }

    // The last folder used for saving and opening files
    m_LastFolderOpen  = settings.value("lastfolderopen", QDir::homePath()).toString();
    // The list of recent files
    s_RecentFiles    = settings.value("recentfiles").toStringList();
    m_preserveHeadingAttributes = settings.value("preserveheadingattributes", true).toBool();
    SetPreserveHeadingAttributes(m_preserveHeadingAttributes);
    const QStringList clipboardHistory = settings.value("clipboardringhistory").toStringList();
    m_ClipboardHistorySelector->LoadClipboardHistory(clipboardHistory);
    settings.endGroup();
    // Our default fonts for book view/web preview
    SettingsStore::BookViewAppearance bookViewAppearance = settings.bookViewAppearance();
    QWebSettings *web_settings = QWebSettings::globalSettings();
    web_settings->setFontSize(QWebSettings::DefaultFontSize, bookViewAppearance.font_size);
    web_settings->setFontFamily(QWebSettings::StandardFont, bookViewAppearance.font_family_standard);
    web_settings->setFontFamily(QWebSettings::SerifFont, bookViewAppearance.font_family_serif);
    web_settings->setFontFamily(QWebSettings::SansSerifFont, bookViewAppearance.font_family_sans_serif);
}


void MainWindow::WriteSettings()
{
    SettingsStore settings;
    settings.beginGroup(SETTINGS_GROUP);
    // The size of the window and it's full screen status
    // This is a workaround for this bug:
    // Maximizing Sigil and then closing will forget the previous window size
    // and open it maximized on the wrong screen.
    // https://bugreports.qt-project.org/browse/QTBUG-21371
    settings.setValue("maximized", isMaximized());
    settings.setValue("geometry", m_LastWindowSize);
    // The positions of all the toolbars and dock widgets
    settings.setValue("toolbars", saveState());
    // The last folders used for saving and opening files
    settings.setValue("lastfolderopen",  m_LastFolderOpen);
    // The list of recent files
    settings.setValue("recentfiles", s_RecentFiles);
    settings.setValue("preserveheadingattributes", m_preserveHeadingAttributes);
    settings.setValue("clipboardringhistory", m_ClipboardHistorySelector->GetClipboardHistory());
    KeyboardShortcutManager::instance()->writeSettings();
    settings.endGroup();
    settings.setViewState(m_ViewState);
}

bool MainWindow::MaybeSaveDialogSaysProceed()
{
    // Make sure that any tabs currently about to be drawn etc get a chance to do so.
    // or else the process of closing/creating a new book will crash with Qt object errors.
    // Particularly a problem if open a large tab in Preview prior to the action
    // due to QWebInspector
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


void MainWindow::SetNewBook(QSharedPointer<Book> new_book)
{
    m_TabManager->CloseOtherTabs();
    m_TabManager->CloseAllTabs(true);
    m_Book = new_book;
    //main language must be set before m_BookBrowser is called
    //because it calls in the end QuickSerialHtmlParser
    SpellCheck *sc = SpellCheck::instance();
    sc->setDCLanguages(m_Book->getBookMainDCLanguageCodes());
    sc->clearAllMLIgnoredWords();
    m_BookBrowser->SetBook(m_Book);
    m_TableOfContents->SetBook(m_Book);
    m_ValidationResultsView->SetBook(m_Book);
    m_IndexEditor->SetBook(m_Book);
    m_ClipEditor->SetBook(m_Book);
    m_SpellcheckEditor->SetBook(m_Book);

    ResetLinkOrStyleBookmark();
    SettingsStore settings;
    settings.setRenameTemplate("");
    connect(m_Book.data(),     SIGNAL(ModifiedStateChanged(bool)), this, SLOT(setWindowModified(bool)));
    connect(m_Book.data(),     SIGNAL(ResourceUpdatedFromDiskRequest(Resource *)), this, SLOT(ResourceUpdatedFromDisk(Resource *)));
    connect(m_BookBrowser,     SIGNAL(ShowStatusMessageRequest(const QString &, int)), this, SLOT(ShowMessageOnStatusBar(const QString &, int)));
    connect(m_BookBrowser,     SIGNAL(ResourcesDeleted()), this, SLOT(ResourcesAddedOrDeleted()));
    connect(m_BookBrowser,     SIGNAL(ResourcesAdded()), this, SLOT(ResourcesAddedOrDeleted()));
}

void MainWindow::ResourcesAddedOrDeleted()
{
    clearMemoryCaches();

    // Make sure currently visible tab is updated immediately
    FlowTab *flow_tab = GetCurrentFlowTab();
    if (flow_tab) {
        flow_tab->LoadTabContent();
    }
}


void MainWindow::CreateNewBook()
{
    QSharedPointer<Book> new_book = QSharedPointer<Book>(new Book());
    new_book->CreateEmptyHTMLFile();
    QString version = new_book->GetConstOPF()->GetEpubVersion();
    if (version.startsWith('3')) {
        HTMLResource * nav_resource = new_book->CreateEmptyNavFile(true);
        new_book->GetOPF()->SetNavResource(nav_resource);
        new_book->GetOPF()->SetItemRefLinear(nav_resource, false);
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
                UpdateUiWithCurrentFile("");
                m_Book->SetModified();
            }

            return true;
        }
    } catch (FileEncryptedWithDrm) {
        ShowMessageOnStatusBar();
        QApplication::restoreOverrideCursor();
        Utility::DisplayStdErrorDialog(
            tr("The creator of this file has encrypted it with DRM. "
               "Sigil cannot open such files."));
    } catch (EPUBLoadParseError epub_load_error) {
        ShowMessageOnStatusBar();
        QApplication::restoreOverrideCursor();
        const QString errors = QString(epub_load_error.what());
        Utility::DisplayStdErrorDialog(
            tr("Cannot load EPUB: %1").arg(QDir::toNativeSeparators(fullfilepath)), errors);
    } catch (const std::runtime_error &e) {
        ShowMessageOnStatusBar();
        QApplication::restoreOverrideCursor();
        Utility::DisplayExceptionErrorDialog(tr("Cannot load file %1: %2")
                                             .arg(QDir::toNativeSeparators(fullfilepath))
                                             .arg(e.what()));
    } catch (QString err) {
        ShowMessageOnStatusBar();
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
    bool not_well_formed = false;

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

        QList <HTMLResource *> resources;
        Q_FOREACH(Resource * r, GetAllHTMLResources()) {
            HTMLResource *t = dynamic_cast<HTMLResource *>(r);
            if (t) {
                resources.append(t);
                if (!XhtmlDoc::IsDataWellFormed(t->GetText())) {
                    not_well_formed = true;
                    break;
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
                                   "fixed, which very rarely may result in data loss.\n\n"
                                   "Do you want to automatically mend the files before saving?"),
                                QMessageBox::Yes|QMessageBox::No);
                QApplication::setOverrideCursor(Qt::WaitCursor);
                if (auto_fix) {
                    CleanSource::ReformatAll(resources, CleanSource::Mend);
                    not_well_formed = false;
                }
            } else {
                CleanSource::ReformatAll(resources, CleanSource::Mend);
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
    } catch (std::runtime_error e) {
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

void MainWindow::SetInsertedFileWatchResourceFile(const QString &pathname)
{
    QString filename = QFileInfo(pathname).fileName();

    try {
        Resource *resource = m_Book->GetFolderKeeper()->GetResourceByFilename(filename);
        m_Book->GetFolderKeeper()->WatchResourceFile(resource);
    } catch (ResourceDoesNotExist) {
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


void MainWindow::UpdateUiWithCurrentFile(const QString &fullfilepath)
{
    m_CurrentFilePath = fullfilepath;
    m_CurrentFileName = m_CurrentFilePath.isEmpty() ? DEFAULT_FILENAME : QFileInfo(m_CurrentFilePath).fileName();
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

void MainWindow::ApplyHeadingStyleToTab(const QString &heading_type)
{
    FlowTab *flow_tab = GetCurrentFlowTab();

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


void MainWindow::PlatformSpecificTweaks()
{
    // We use the "close" action only on Macs,
    // because they need it for the multi-document interface
#ifndef Q_OS_MAC
    ui.actionClose->setEnabled(false);
    ui.actionClose->setVisible(false);
    // attempt to grow icons for high dpi displays on Windows and Linux
    int iconsize = QFontMetrics(QFont()).lineSpacing() * 2;
    if (iconsize < 22) iconsize = 22;
    QList<QToolBar *> all_toolbars = findChildren<QToolBar *>();
    foreach(QToolBar * toolbar, all_toolbars) {
        toolbar->setIconSize(QSize(iconsize,iconsize));
    }
#else
    // Macs also use bigger icons but scale them automatically for high dpi displays
    QList<QToolBar *> all_toolbars = findChildren<QToolBar *>();
    foreach(QToolBar * toolbar, all_toolbars) {
        toolbar->setIconSize(QSize(32, 32));
    }
    // Set the action because they are not automatically put in the right place as of Qt 5.1.
    ui.actionAbout->setMenuRole(QAction::AboutRole);
    ui.actionPreferences->setMenuRole(QAction::PreferencesRole);
#endif
}


void MainWindow::ExtendUI()
{
    m_FindReplace->ShowHide();
    // We want a nice frame around the tab manager
    QFrame *frame = new QFrame(this);
    QLayout *layout = new QVBoxLayout(frame);
    frame->setLayout(layout);
    layout->addWidget(m_TabManager);
    layout->addWidget(m_FindReplace);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);
    frame->setObjectName(FRAME_NAME);
    frame->setStyleSheet(TAB_STYLE_SHEET);
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
    addDockWidget(Qt::RightDockWidgetArea, m_PreviewWindow);
    m_PreviewWindow->hide();

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
    ui.menuToolbars->addAction(ui.toolBarFileActions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextManip->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarViews->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarInsertions->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarBack->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarDonate->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTools->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarHeadings->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextFormats->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextAlign->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarLists->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarIndents->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarChangeCase->toggleViewAction());
    ui.menuToolbars->addAction(ui.toolBarTextDirection->toggleViewAction());
    ui.toolBarTextDirection->setVisible(false);
    ui.menuToolbars->addAction(ui.toolBarClips->toggleViewAction());
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
    sm->registerAction(this, ui.actionNew, "MainWindow.New");
    sm->registerAction(this, ui.actionNewHTMLFile, "MainWindow.NewHTMLFile");
    sm->registerAction(this, ui.actionNewCSSFile, "MainWindow.NewCSSFile");
    sm->registerAction(this, ui.actionNewSVGFile, "MainWindow.NewSVGFile");
    sm->registerAction(this, ui.actionAddExistingFile, "MainWindow.AddExistingFile");
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
    sm->registerAction(this, ui.actionShowTag, "MainWindow.ShowTag");
    sm->registerAction(this, ui.actionRemoveFormatting, "MainWindow.RemoveFormatting");
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
    sm->registerAction(this, ui.actionNCXFromNav, "MainWindow.NCXFromNav");
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
    sm->registerAction(this, ui.actionBookView, "MainWindow.BookView");
    sm->registerAction(this, ui.actionCodeView, "MainWindow.CodeView");
    sm->registerAction(this, ui.actionToggleViewState, "MainWindow.ToggleViewState");
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
    // Help
    sm->registerAction(this, ui.actionUserGuide, "MainWindow.UserGuide");
    sm->registerAction(this, ui.actionFAQ, "MainWindow.FAQ");
    sm->registerAction(this, ui.actionTutorials, "MainWindow.FAQ");
    sm->registerAction(this, ui.actionDonate, "MainWindow.Donate");
    sm->registerAction(this, ui.actionSigilWebsite, "MainWindow.SigilWebsite");
    sm->registerAction(this, ui.actionAbout, "MainWindow.About");
    // Clips
    sm->registerAction(this, ui.actionClip1, "MainWindow.Clip1");
    sm->registerAction(this, ui.actionClip2, "MainWindow.Clip2");
    sm->registerAction(this, ui.actionClip3, "MainWindow.Clip3");
    sm->registerAction(this, ui.actionClip4, "MainWindow.Clip4");
    sm->registerAction(this, ui.actionClip5, "MainWindow.Clip5");
    sm->registerAction(this, ui.actionClip6, "MainWindow.Clip6");
    sm->registerAction(this, ui.actionClip7, "MainWindow.Clip7");
    sm->registerAction(this, ui.actionClip8, "MainWindow.Clip8");
    sm->registerAction(this, ui.actionClip9, "MainWindow.Clip9");
    sm->registerAction(this, ui.actionClip10, "MainWindow.Clip10");
    sm->registerAction(this, ui.actionClip11, "MainWindow.Clip11");
    sm->registerAction(this, ui.actionClip12, "MainWindow.Clip12");
    sm->registerAction(this, ui.actionClip13, "MainWindow.Clip13");
    sm->registerAction(this, ui.actionClip14, "MainWindow.Clip14");
    sm->registerAction(this, ui.actionClip15, "MainWindow.Clip15");
    sm->registerAction(this, ui.actionClip16, "MainWindow.Clip16");
    sm->registerAction(this, ui.actionClip17, "MainWindow.Clip17");
    sm->registerAction(this, ui.actionClip18, "MainWindow.Clip18");
    sm->registerAction(this, ui.actionClip19, "MainWindow.Clip19");
    sm->registerAction(this, ui.actionClip20, "MainWindow.Clip20");

    // for plugins
    sm->registerAction(this, ui.actionPlugin1, "MainWindow.Plugins.RunPlugin1");
    sm->registerAction(this, ui.actionPlugin2, "MainWindow.Plugins.RunPlugin2");
    sm->registerAction(this, ui.actionPlugin3, "MainWindow.Plugins.RunPlugin3");

    ExtendIconSizes();
    UpdateClipsUI();
}

void MainWindow::UpdateClipButton(int clip_number, QAction *ui_action)
{
    ClipEditorModel::clipEntry *clip_entry = ClipEditorModel::instance()->GetEntryFromNumber(clip_number);

    if (clip_entry) {
        ui_action->setText(clip_entry->name);
        QString clip_text = clip_entry->text;
        clip_text.replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;");
        ui_action->setToolTip(clip_text);
        ui_action->setVisible(true);
    } else {
        ui_action->setText("");
        ui_action->setToolTip("");
        ui_action->setVisible(false);
    }
}

void MainWindow::UpdateClipsUI()
{
    UpdateClipButton(1, ui.actionClip1);
    UpdateClipButton(2, ui.actionClip2);
    UpdateClipButton(3, ui.actionClip3);
    UpdateClipButton(4, ui.actionClip4);
    UpdateClipButton(5, ui.actionClip5);
    UpdateClipButton(6, ui.actionClip6);
    UpdateClipButton(7, ui.actionClip7);
    UpdateClipButton(8, ui.actionClip8);
    UpdateClipButton(9, ui.actionClip9);
    UpdateClipButton(10, ui.actionClip10);
    UpdateClipButton(11, ui.actionClip11);
    UpdateClipButton(12, ui.actionClip12);
    UpdateClipButton(13, ui.actionClip13);
    UpdateClipButton(14, ui.actionClip14);
    UpdateClipButton(15, ui.actionClip15);
    UpdateClipButton(16, ui.actionClip16);
    UpdateClipButton(17, ui.actionClip17);
    UpdateClipButton(18, ui.actionClip18);
    UpdateClipButton(19, ui.actionClip19);
    UpdateClipButton(20, ui.actionClip20);
}

void MainWindow::ExtendIconSizes()
{
    QIcon icon;
    icon = ui.actionNew->icon();
    icon.addFile(QString::fromUtf8(":/main/document-new_16px.png"));
    ui.actionNew->setIcon(icon);
    icon = ui.actionAddExistingFile->icon();
    icon.addFile(QString::fromUtf8(":/main/document-add_16px.png"));
    ui.actionAddExistingFile->setIcon(icon);
    icon = ui.actionSave->icon();
    icon.addFile(QString::fromUtf8(":/main/document-save_16px.png"));
    ui.actionSave->setIcon(icon);
    icon = ui.actionSpellcheckEditor->icon();
    icon.addFile(QString::fromUtf8(":/main/document-spellcheck_16px.png"));
    ui.actionSpellcheckEditor->setIcon(icon);
    icon = ui.actionCut->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-cut_16px.png"));
    ui.actionCut->setIcon(icon);
    icon = ui.actionPaste->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-paste_16px.png"));
    ui.actionPaste->setIcon(icon);
    icon = ui.actionUndo->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-undo_16px.png"));
    ui.actionUndo->setIcon(icon);
    icon = ui.actionRedo->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-redo_16px.png"));
    ui.actionRedo->setIcon(icon);
    icon = ui.actionCopy->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-copy_16px.png"));
    ui.actionCopy->setIcon(icon);
    icon = ui.actionAlignLeft->icon();
    icon.addFile(QString::fromUtf8(":/main/format-justify-left_16px.png"));
    ui.actionAlignLeft->setIcon(icon);
    icon = ui.actionAlignRight->icon();
    icon.addFile(QString::fromUtf8(":/main/format-justify-right_16px.png"));
    ui.actionAlignRight->setIcon(icon);
    icon = ui.actionAlignCenter->icon();
    icon.addFile(QString::fromUtf8(":/main/format-justify-center_16px.png"));
    ui.actionAlignCenter->setIcon(icon);
    icon = ui.actionAlignJustify->icon();
    icon.addFile(QString::fromUtf8(":/main/format-justify-fill_16px.png"));
    ui.actionAlignJustify->setIcon(icon);
    icon = ui.actionBold->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-bold_16px.png"));
    ui.actionBold->setIcon(icon);
    icon = ui.actionItalic->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-italic_16px.png"));
    ui.actionItalic->setIcon(icon);
    icon = ui.actionUnderline->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-underline_16px.png"));
    ui.actionUnderline->setIcon(icon);
    icon = ui.actionStrikethrough->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-strikethrough_16px.png"));
    ui.actionStrikethrough->setIcon(icon);
    icon = ui.actionSubscript->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-subscript_16px.png"));
    ui.actionSubscript->setIcon(icon);
    icon = ui.actionSuperscript->icon();
    icon.addFile(QString::fromUtf8(":/main/format-text-superscript_16px.png"));
    ui.actionSuperscript->setIcon(icon);
    icon = ui.actionInsertNumberedList->icon();
    icon.addFile(QString::fromUtf8(":/main/insert-numbered-list_16px.png"));
    ui.actionInsertNumberedList->setIcon(icon);
    icon = ui.actionInsertBulletedList->icon();
    icon.addFile(QString::fromUtf8(":/main/insert-bullet-list_16px.png"));
    ui.actionInsertBulletedList->setIcon(icon);
    icon = ui.actionIncreaseIndent->icon();
    icon.addFile(QString::fromUtf8(":/main/format-indent-more_16px.png"));
    ui.actionIncreaseIndent->setIcon(icon);
    icon = ui.actionDecreaseIndent->icon();
    icon.addFile(QString::fromUtf8(":/main/format-indent-less_16px.png"));
    ui.actionDecreaseIndent->setIcon(icon);
    icon = ui.actionCasingLowercase->icon();
    icon.addFile(QString::fromUtf8(":/main/format-case-lowercase_16px.png"));
    ui.actionCasingLowercase->setIcon(icon);
    icon = ui.actionCasingUppercase->icon();
    icon.addFile(QString::fromUtf8(":/main/format-case-uppercase_16px.png"));
    ui.actionCasingUppercase->setIcon(icon);
    icon = ui.actionCasingTitlecase->icon();
    icon.addFile(QString::fromUtf8(":/main/format-case-titlecase_16px.png"));
    ui.actionCasingTitlecase->setIcon(icon);
    icon = ui.actionCasingCapitalize->icon();
    icon.addFile(QString::fromUtf8(":/main/format-case-capitalize_16px.png"));
    ui.actionCasingCapitalize->setIcon(icon);
    icon = ui.actionTextDirectionLTR->icon();
    icon.addFile(QString::fromUtf8(":/main/format-direction-ltr_16px.png"));
    ui.actionTextDirectionLTR->setIcon(icon);
    icon = ui.actionTextDirectionRTL->icon();
    icon.addFile(QString::fromUtf8(":/main/format-direction-rtl_16px.png"));
    ui.actionTextDirectionRTL->setIcon(icon);
    icon = ui.actionTextDirectionDefault->icon();
    icon.addFile(QString::fromUtf8(":/main/format-direction-default_16px.png"));
    ui.actionTextDirectionDefault->setIcon(icon);
    icon = ui.actionHeading1->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-1_16px.png"));
    ui.actionHeading1->setIcon(icon);
    icon = ui.actionHeading2->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-2_16px.png"));
    ui.actionHeading2->setIcon(icon);
    icon = ui.actionHeading3->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-3_16px.png"));
    ui.actionHeading3->setIcon(icon);
    icon = ui.actionHeading4->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-4_16px.png"));
    ui.actionHeading4->setIcon(icon);
    icon = ui.actionHeading5->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-5_16px.png"));
    ui.actionHeading5->setIcon(icon);
    icon = ui.actionHeading6->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-6_16px.png"));
    ui.actionHeading6->setIcon(icon);
    icon = ui.actionHeadingNormal->icon();
    icon.addFile(QString::fromUtf8(":/main/heading-normal_16px.png"));
    ui.actionHeadingNormal->setIcon(icon);
    icon = ui.actionOpen->icon();
    icon.addFile(QString::fromUtf8(":/main/document-open_16px.png"));
    ui.actionOpen->setIcon(icon);
    icon = ui.actionExit->icon();
    icon.addFile(QString::fromUtf8(":/main/process-stop_16px.png"));
    ui.actionExit->setIcon(icon);
    icon = ui.actionAbout->icon();
    icon.addFile(QString::fromUtf8(":/main/help-browser_16px.png"));
    ui.actionAbout->setIcon(icon);
    icon = ui.actionBookView->icon();
    icon.addFile(QString::fromUtf8(":/main/view-book_16px.png"));
    ui.actionBookView->setIcon(icon);
    icon = ui.actionCodeView->icon();
    icon.addFile(QString::fromUtf8(":/main/view-code_16px.png"));
    ui.actionCodeView->setIcon(icon);
    icon = ui.actionSplitSection->icon();
    icon.addFile(QString::fromUtf8(":/main/insert-section-break_16px.png"));
    ui.actionSplitSection->setIcon(icon);
    icon = ui.actionInsertFile->icon();
    icon.addFile(QString::fromUtf8(":/main/insert-image_16px.png"));
    ui.actionInsertFile->setIcon(icon);
    icon = ui.actionPrint->icon();
    icon.addFile(QString::fromUtf8(":/main/document-print_16px.png"));
    ui.actionPrint->setIcon(icon);
    icon = ui.actionPrintPreview->icon();
    icon.addFile(QString::fromUtf8(":/main/document-print-preview_16px.png"));
    ui.actionPrintPreview->setIcon(icon);
    icon = ui.actionZoomIn->icon();
    icon.addFile(QString::fromUtf8(":/main/list-add_16px.png"));
    ui.actionZoomIn->setIcon(icon);
    icon = ui.actionZoomOut->icon();
    icon.addFile(QString::fromUtf8(":/main/list-remove_16px.png"));
    ui.actionZoomOut->setIcon(icon);
    icon = ui.actionFind->icon();
    icon.addFile(QString::fromUtf8(":/main/edit-find_16px.png"));
    ui.actionFind->setIcon(icon);
    icon = ui.actionDonate->icon();
    icon.addFile(QString::fromUtf8(":/main/emblem-favorite_16px.png"));
    ui.actionDonate->setIcon(icon);
    icon = ui.actionMetaEditor->icon();
    icon.addFile(QString::fromUtf8(":/main/metadata-editor_16px.png"));
    ui.actionMetaEditor->setIcon(icon);
    icon = ui.actionGenerateTOC->icon();
    icon.addFile(QString::fromUtf8(":/main/generate-toc_16px.png"));
    ui.actionGenerateTOC->setIcon(icon);
}

void MainWindow::LoadInitialFile(const QString &openfilepath, bool is_internal)
{
    if (!openfilepath.isEmpty()) {
        LoadFile(openfilepath, is_internal);
    } else {
        CreateNewBook();
    }
}


void MainWindow::ConnectSignalsToSlots()
{
    connect(m_PreviewWindow, SIGNAL(Shown()), this, SLOT(UpdatePreview()));
    connect(m_PreviewWindow, SIGNAL(GoToPreviewLocationRequest()), this, SLOT(GoToPreviewLocation()));
    connect(m_PreviewWindow, SIGNAL(ZoomFactorChanged(float)),     this, SLOT(UpdateZoomLabel(float)));
    connect(m_PreviewWindow, SIGNAL(ZoomFactorChanged(float)),     this, SLOT(UpdateZoomSlider(float)));
    connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)), this, SLOT(ApplicationFocusChanged(QWidget *, QWidget *)));
    // Setup signal mapping for heading actions.
    connect(ui.actionHeading1, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading1, "1");
    connect(ui.actionHeading2, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading2, "2");
    connect(ui.actionHeading3, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading3, "3");
    connect(ui.actionHeading4, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading4, "4");
    connect(ui.actionHeading5, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading5, "5");
    connect(ui.actionHeading6, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeading6, "6");
    connect(ui.actionHeadingNormal, SIGNAL(triggered()), m_headingMapper, SLOT(map()));
    m_headingMapper->setMapping(ui.actionHeadingNormal, "Normal");
    // File
    connect(ui.actionNew,           SIGNAL(triggered()), this, SLOT(New()));
    connect(ui.actionOpen,          SIGNAL(triggered()), this, SLOT(Open()));
    connect(ui.actionNewHTMLFile,   SIGNAL(triggered()), m_BookBrowser, SLOT(AddNewHTML()));
    connect(ui.actionNewCSSFile,    SIGNAL(triggered()), m_BookBrowser, SLOT(AddNewCSS()));
    connect(ui.actionNewSVGFile,    SIGNAL(triggered()), m_BookBrowser, SLOT(AddNewSVG()));
    connect(ui.actionAddExistingFile,   SIGNAL(triggered()), m_BookBrowser, SLOT(AddExisting()));
    connect(ui.actionSave,          SIGNAL(triggered()), this, SLOT(Save()));
    connect(ui.actionSaveAs,        SIGNAL(triggered()), this, SLOT(SaveAs()));
    connect(ui.actionSaveACopy,     SIGNAL(triggered()), this, SLOT(SaveACopy()));
    connect(ui.actionClose,         SIGNAL(triggered()), this, SLOT(close()));
    connect(ui.actionExit,          SIGNAL(triggered()), this, SLOT(Exit()));
    // Edit
    connect(ui.actionInsertFile,     SIGNAL(triggered()), this, SLOT(InsertFileDialog()));
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
    connect(ui.actionNCXFromNav,    SIGNAL(triggered()), this, SLOT(GenerateNCXFromNav()));
    connect(ui.actionClearIgnoredWords, SIGNAL(triggered()), this, SLOT(ClearIgnoredWords()));
    connect(ui.actionGenerateTOC,   SIGNAL(triggered()), this, SLOT(GenerateToc()));
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
    connect(ui.actionCasingLowercase,  SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    connect(ui.actionCasingUppercase,  SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    connect(ui.actionCasingTitlecase, SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    connect(ui.actionCasingCapitalize, SIGNAL(triggered()), m_casingChangeMapper, SLOT(map()));
    m_casingChangeMapper->setMapping(ui.actionCasingLowercase,  Utility::Casing_Lowercase);
    m_casingChangeMapper->setMapping(ui.actionCasingUppercase,  Utility::Casing_Uppercase);
    m_casingChangeMapper->setMapping(ui.actionCasingTitlecase, Utility::Casing_Titlecase);
    m_casingChangeMapper->setMapping(ui.actionCasingCapitalize, Utility::Casing_Capitalize);
    connect(m_casingChangeMapper, SIGNAL(mapped(int)), this, SLOT(ChangeCasing(int)));
    // View
    connect(ui.actionZoomIn,        SIGNAL(triggered()), this, SLOT(ZoomIn()));
    connect(ui.actionZoomOut,       SIGNAL(triggered()), this, SLOT(ZoomOut()));
    connect(ui.actionZoomReset,     SIGNAL(triggered()), this, SLOT(ZoomReset()));
    connect(ui.actionBookView,      SIGNAL(triggered()),  this,   SLOT(BookView()));
    connect(ui.actionCodeView,      SIGNAL(triggered()),  this,   SLOT(CodeView()));
    connect(ui.actionToggleViewState, SIGNAL(triggered()),  this,   SLOT(ToggleViewState()));
    connect(ui.actionHeadingPreserveAttributes, SIGNAL(triggered(bool)), this, SLOT(SetPreserveHeadingAttributes(bool)));
    connect(m_headingMapper,      SIGNAL(mapped(const QString &)),  this,   SLOT(ApplyHeadingStyleToTab(const QString &)));
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
    connect(ui.actionClip1,       SIGNAL(triggered()), this, SLOT(PasteClip1IntoCurrentTarget()));
    connect(ui.actionClip2,       SIGNAL(triggered()), this, SLOT(PasteClip2IntoCurrentTarget()));
    connect(ui.actionClip3,       SIGNAL(triggered()), this, SLOT(PasteClip3IntoCurrentTarget()));
    connect(ui.actionClip4,       SIGNAL(triggered()), this, SLOT(PasteClip4IntoCurrentTarget()));
    connect(ui.actionClip5,       SIGNAL(triggered()), this, SLOT(PasteClip5IntoCurrentTarget()));
    connect(ui.actionClip6,       SIGNAL(triggered()), this, SLOT(PasteClip6IntoCurrentTarget()));
    connect(ui.actionClip7,       SIGNAL(triggered()), this, SLOT(PasteClip7IntoCurrentTarget()));
    connect(ui.actionClip8,       SIGNAL(triggered()), this, SLOT(PasteClip8IntoCurrentTarget()));
    connect(ui.actionClip9,       SIGNAL(triggered()), this, SLOT(PasteClip9IntoCurrentTarget()));
    connect(ui.actionClip10,      SIGNAL(triggered()), this, SLOT(PasteClip10IntoCurrentTarget()));
    connect(ui.actionClip11,      SIGNAL(triggered()), this, SLOT(PasteClip11IntoCurrentTarget()));
    connect(ui.actionClip12,      SIGNAL(triggered()), this, SLOT(PasteClip12IntoCurrentTarget()));
    connect(ui.actionClip13,      SIGNAL(triggered()), this, SLOT(PasteClip13IntoCurrentTarget()));
    connect(ui.actionClip14,      SIGNAL(triggered()), this, SLOT(PasteClip14IntoCurrentTarget()));
    connect(ui.actionClip15,      SIGNAL(triggered()), this, SLOT(PasteClip15IntoCurrentTarget()));
    connect(ui.actionClip16,      SIGNAL(triggered()), this, SLOT(PasteClip16IntoCurrentTarget()));
    connect(ui.actionClip17,      SIGNAL(triggered()), this, SLOT(PasteClip17IntoCurrentTarget()));
    connect(ui.actionClip18,      SIGNAL(triggered()), this, SLOT(PasteClip18IntoCurrentTarget()));
    connect(ui.actionClip19,      SIGNAL(triggered()), this, SLOT(PasteClip19IntoCurrentTarget()));
    connect(ui.actionClip20,      SIGNAL(triggered()), this, SLOT(PasteClip20IntoCurrentTarget()));
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
    connect(m_TabManager,          SIGNAL(TabChanged(ContentTab *, ContentTab *)),
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
    connect(m_BookBrowser, SIGNAL(RemoveResourcesRequest()), this, SLOT(RemoveResources()));
    connect(m_BookBrowser, SIGNAL(OpenFileRequest(QString, int)), this, SLOT(OpenFile(QString, int)));
    connect(m_TableOfContents, SIGNAL(OpenResourceRequest(Resource *, int, int, const QString &, MainWindow::ViewState, const QUrl &)),
            this,     SLOT(OpenResource(Resource *, int, int, const QString &, MainWindow::ViewState, const QUrl &)));
    connect(m_ValidationResultsView, SIGNAL(OpenResourceRequest(Resource *, int, int, const QString &, MainWindow::ViewState)),
            this,     SLOT(OpenResource(Resource *, int, int, const QString &, MainWindow::ViewState)));
    connect(m_TabManager, SIGNAL(OpenUrlRequest(const QUrl &)),
            this, SLOT(OpenUrl(const QUrl &)));
    connect(m_TabManager, SIGNAL(OldTabRequest(QString, HTMLResource *)),
            this,          SLOT(CreateSectionBreakOldTab(QString, HTMLResource *)));
    connect(m_TabManager, SIGNAL(ToggleViewStateRequest()),
            this,          SLOT(ToggleViewState()));
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
    connect(m_Reports,       SIGNAL(OpenFileRequest(QString, int)), this, SLOT(OpenFile(QString, int)));
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
    if (rType != Resource::ImageResourceType && rType != Resource::AudioResourceType && rType != Resource::VideoResourceType) {
        connect(ui.actionUndo,                     SIGNAL(triggered()),  tab,   SLOT(Undo()));
        connect(ui.actionRedo,                     SIGNAL(triggered()),  tab,   SLOT(Redo()));
        connect(ui.actionCut,                      SIGNAL(triggered()),  tab,   SLOT(Cut()));
        connect(ui.actionCopy,                     SIGNAL(triggered()),  tab,   SLOT(Copy()));
        connect(ui.actionPaste,                    SIGNAL(triggered()),  tab,   SLOT(Paste()));
        connect(ui.actionDeleteLine,               SIGNAL(triggered()),  tab,   SLOT(DeleteLine()));
        connect(tab,   SIGNAL(OpenClipEditorRequest(ClipEditorModel::clipEntry *)),
                this,  SLOT(ClipEditorDialog(ClipEditorModel::clipEntry *)));
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
        connect(ui.actionShowTag,                  SIGNAL(triggered()),  tab,   SLOT(ShowTag()));
        connect(ui.actionRemoveFormatting,         SIGNAL(triggered()),  tab,   SLOT(RemoveFormatting()));
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
        connect(tab,   SIGNAL(InspectElement()), this, SLOT(InspectHTML()));
    }

    if (rType != Resource::AudioResourceType && rType != Resource::VideoResourceType) {
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
    disconnect(ui.actionShowTag,                   0, tab, 0);
    disconnect(ui.actionRemoveFormatting,          0, tab, 0);
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
    disconnect(tab,                                0, this, 0);
    disconnect(tab,                                0, m_Book.data(), 0);
    disconnect(tab,                                0, m_BookBrowser, 0);
    disconnect(tab,                                0, m_ClipboardHistorySelector, 0);
}

