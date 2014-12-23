/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012 Dave Heiland
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#pragma once
#ifndef SIGIL_H
#define SIGIL_H

#include <QtCore/QSharedPointer>
#include <QtWidgets/QMainWindow>

#include "ui_main.h"
#include "BookManipulation/Book.h"
#include "BookManipulation/BookReports.h"
#include "Dialogs/ClipboardHistorySelector.h"
#include "Dialogs/IndexEditor.h"
#include "Dialogs/Reports.h"
#include "Dialogs/SpellcheckEditor.h"
#include "Dialogs/ViewImage.h"
#include "MainUI/FindReplace.h"
#include "MainUI/NCXModel.h"
#include "Misc/CSSInfo.h"
#include "Misc/PasteTarget.h"
#include "Misc/SettingsStore.h"
#include "Misc/ValidationResult.h"
#include "MiscEditors/ClipEditorModel.h"
#include "MiscEditors/IndexEditorModel.h"
#include "MiscEditors/SearchEditorModel.h"
#include "Tabs/ContentTab.h"

const int MAX_RECENT_FILES = 5;
const int STATUSBAR_MSG_DISPLAY_TIME = 7000;

class QComboBox;
class QLabel;
class QSignalMapper;
class QSlider;
class QTimer;
class FindReplace;
class MetaEditor;
class TabManager;
class BookBrowser;
class TableOfContents;
class ValidationResultsView;
class PreviewWindow;
class SearchEditor;
class ClipEditor;
class ClipsWindow;
class SelectCharacter;
class ViewImage;
class FlowTab;


/**
 * @mainpage
 * The conversion of all source comments to Doxygen format
 * is in progress. Some files have been converted, others have not.
 *
 * Be patient.
 */


/**
 * The main window of the application.
 * Presents the main user interface with menus, toolbars, editing panes
 * and side panes like the Book Browser.
 *
 * This window is the main entry point to all functionality.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /**
     * Constructor.
     *
     * @param openfilepath The path to the file that the window
     *                     should load (new file loaded if empty).
     * @param parent The window's parent object.
     * @param flags The flags used to modify window behavior.
     */
    MainWindow(const QString &openfilepath = QString(), bool is_internal = false, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~MainWindow();

    /**
     * The book currently being edited.
     *
     * @return A shared pointer to the book.
     */
    QSharedPointer<Book> GetCurrentBook();


    /**
     * get BookBrowser for this book
     *
     * @return A pointer to the BookBrowser
     */
    BookBrowser *GetBookBrowser();


    /**
     * Returns a reference to the current content tab.
     *
     * @return A reference to the current content tab.
     */
    ContentTab &GetCurrentContentTab();

    FlowTab *GetCurrentFlowTab();

    /**
     * Returns a list of valid selected HTML resources
     *
     * @return List of valid selected HTML resources
     */
    QList <Resource *> GetValidSelectedHTMLResources();

    /**
     * Returns a list of all HTML resources in book browser order
     *
     * @return List of all HTML resources in book browser order
     */
    QList <Resource *> GetAllHTMLResources();


    /**
     * Select resources in the Book Browser
     *
     */
    void SelectResources(QList<Resource *> resources);

    /**
     * Describes the type of the View mode
     * currently used in FlowTab.
     */
    enum ViewState {
        ViewState_Unknown = 0,     /**< Default non view that we don't know or care what it is */
        ViewState_BookView = 10,   /**< The WYSIWYG view. */
        ViewState_CodeView = 30    /**< The XHTML code editing view. */
    };

    /**
     * The location of the last bookmark.
     */
    struct LocationBookmark {
        QString filename;
        MainWindow::ViewState view_state;
        QString bv_caret_location_update;
        int cv_cursor_position;
    };

    /**
     * Returns the current view state.
     *
     * @return The current view state.
     */
    MainWindow::ViewState GetViewState();

    void CloseAllTabs();

    void SaveTabData();

    SearchEditorModel *GetSearchEditorModel();

    /**
     * Returns a map with keys being extensions of file types
     * we can load, and the values being filters for use in file dialogs.
     *
     * @return The load dialog filters.
     */
    static const QMap<QString, QString> GetLoadFiltersMap();

    /**
     * Loads a book from the file specified.
     *
     * @param fullfilepath The path to the file to load.
     */
    bool LoadFile(const QString &fullfilepath, bool is_internal = false);

    void SetValidationResults(const QList<ValidationResult> &results);

    static void clearMemoryCaches();

public slots:
    void AnyCodeView();

    void OpenUrl(const QUrl &url);

    /**
     * Opens the specified resource in the specified view state.
     */
    void OpenResource(Resource &resource,
                      int line_to_scroll_to = -1,
                      int position_to_scroll_to = -1,
                      const QString &caret_location_to_scroll_to = QString(),
                      MainWindow::ViewState view_state = MainWindow::ViewState_Unknown,
                      const QUrl &fragment = QUrl(),
                      bool precede_current_tab = false);

    void OpenResourceAndWaitUntilLoaded(Resource &resource,
                                        int line_to_scroll_to = -1,
                                        int position_to_scroll_to = -1,
                                        const QString &caret_location_to_scroll_to = QString(),
                                        MainWindow::ViewState view_state = MainWindow::ViewState_Unknown,
                                        const QUrl &fragment = QUrl(),
                                        bool precede_current_tab = false);

    void CreateIndex();

    void runPlugin(QAction *action);

    void ResourcesAddedOrDeleted();


signals:
    void SettingsChanged();

protected:
    void showEvent(QShowEvent *event);
    /**
     * Workaround for Qt 4.8 bug, which does not save/restore window state
     * correctly if maximized at the time of calling saveGeometry().
     */
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);

    /**
     * Overrides the closeEvent handler so we can check
     * for saved status before actually closing.
     *
     * @param event The close event.
     */
    void closeEvent(QCloseEvent *event);

private slots:

    void AddCover();

    /**
     * Implements New action functionality.
     */
    void New();

    /**
     * Implements Open action functionality.
     */
    void Open();

    /**
     * Implements Open recent file action functionality.
     */
    void OpenRecentFile();

    /**
     * Implements Save action functionality.
     */
    bool Save();

    /**
     * Implements Save As action functionality.
     */
    bool SaveAs();

    /**
     * Implements Save A Copy action functionality.
     */
    bool SaveACopy();

    void Exit();

    void ShowMessageOnStatusBar(const QString &message = "", int millisecond_duration = STATUSBAR_MSG_DISPLAY_TIME);

    void ShowLastOpenFileWarnings();

    /**
     * Implements Find action functionality.
     */
    void Find();

    /**
     * Implements Go To Line action functionality.
     */
    void GoToLine();

    /**
     * Implements Zoom In action functionality.
     */
    void ZoomIn();

    /**
     * Implements Zoom Out action functionality.
     */
    void ZoomOut();

    /**
     * Implements Zoom Reset action functionality.
     */
    void ZoomReset();

    void IndexEditorDialog(IndexEditorModel::indexEntry *index_entry = NULL);
    void SpellcheckEditorDialog();

    void ReportsDialog();

    bool DeleteCSSStyles(const QString &filename, QList<CSSInfo::CSSSelector *> css_selectors);

    void DeleteUnusedMedia();
    void DeleteUnusedStyles();

    void InsertFileDialog();

    void InsertSpecialCharacter();

    void InsertId();

    void InsertHyperlink();

    void MarkForIndex();

    /**
     * Track the last active control that had focus in the MainWindow that
     * is a valid PasteTarget.
     */
    void ApplicationFocusChanged(QWidget *old, QWidget *now);

    /**
     * Some controls (CodeView, BookView and combo boxes in F&R) inherit PasteTarget
     * to allow various modeless/popup dialogs like Clipboard History, Clip Editor and
     * Insert Special Characters to insert text into the focused "PasteTarget" control.
     * These two slots will delegate the relevant signal to the current target if any.
     */
    void PasteTextIntoCurrentTarget(const QString &text);
    void PasteClipEntriesIntoCurrentTarget(const QList<ClipEditorModel::clipEntry *> &clips);
    void PasteClipEntriesIntoPreviousTarget(const QList<ClipEditorModel::clipEntry *> &clips);
    void PasteClipIntoCurrentTarget(int clip_number);
    void PasteClip1IntoCurrentTarget();
    void PasteClip2IntoCurrentTarget();
    void PasteClip3IntoCurrentTarget();
    void PasteClip4IntoCurrentTarget();
    void PasteClip5IntoCurrentTarget();
    void PasteClip6IntoCurrentTarget();
    void PasteClip7IntoCurrentTarget();
    void PasteClip8IntoCurrentTarget();
    void PasteClip9IntoCurrentTarget();
    void PasteClip10IntoCurrentTarget();
    void PasteClip11IntoCurrentTarget();
    void PasteClip12IntoCurrentTarget();
    void PasteClip13IntoCurrentTarget();
    void PasteClip14IntoCurrentTarget();
    void PasteClip15IntoCurrentTarget();
    void PasteClip16IntoCurrentTarget();
    void PasteClip17IntoCurrentTarget();
    void PasteClip18IntoCurrentTarget();
    void PasteClip19IntoCurrentTarget();
    void PasteClip20IntoCurrentTarget();

    /**
     * Implements the set BookView functionality.
     */
    void BookView();

    /**
     * Implements the set CodeView functionality.
     */
    void CodeView();

    /**
     * Implements Meta Editor action functionality.
     */
    void MetaEditorDialog();

    /**
     * Implements Search Editor Dialog functionality.
     */
    void SearchEditorDialog(SearchEditorModel::searchEntry *search_entry = NULL);

    void ClipEditorDialog(ClipEditorModel::clipEntry *clip_entry = NULL);

    /**
     * Implements Tutorials action functionality.
     */
    void Tutorials();

    /**
     * Implements User Guide action functionality.
     */
    void UserGuide();

    /**
     * Implements Frequently Asked Questions action functionality.
     */
    void FrequentlyAskedQuestions();

    /**
     * Implements Donate action functionality.
     */
    void Donate();

    /**
     * Implements Sigil Dev Blog action functionality.
     */
    void SigilDevBlog();

    /**
     * Implements About action functionality.
     */
    void AboutDialog();

    /**
     * Implements Preferences action functionality.
     */
    void PreferencesDialog();


    /**
     * Implements Preferences action functionality.
     */
    void ManagePluginsDialog();

    /**
     * Implements Validate Epub with FlightCrew action functionality.
     */
    void ValidateEpubWithFlightCrew();

    void ValidateStylesheetsWithW3C();

    bool CharLessThan(const QChar &s1, const QChar &s2);

    /**
     * Disconnects all signals to the old tab
     * and reconnects them to the new tab when the
     * current tab is changed.
     *
     * @old_tab The tab that was previously in use.
     * @new_tab The tab that is becoming current.
     */
    void ChangeSignalsWhenTabChanges(ContentTab *old_tab, ContentTab *new_tab);

    /**
     * Updates the toolbars/menus based on current state
     * and updates the tab state if requested
     */
    void UpdateViewState(bool set_tab_state = true);

    /**
     * Updates the toolbars based on current tab state and changes.
     */
    void UpdateUIOnTabChanges();

    /**
     * Updates the menus based on the number of tabs open.
     */
    void UpdateUIOnTabCountChange();

    /**
     * Performs needed changes when the user switches tabs.
     */
    void UpdateUIWhenTabsSwitch();

    /**
     * Set initial state for actions in Book View
     */
    void SetStateActionsBookView();

    /**
     * Set initial state for actions in Code View
     */
    void SetStateActionsCodeView();

    /**
     * Set initial state for actions in CSS files
     */
    void SetStateActionsCSSView();

    /**
     * Set initial state for actions in Raw View
     */
    void SetStateActionsRawView();

    /**
     * Set initial state for actions in Static View
     * (everything dead, used for viewing images etc.)
     */
    void SetStateActionsStaticView();

    void UpdatePreviewRequest();
    void UpdatePreviewCSSRequest();
    void UpdatePreview();
    void InspectHTML();

    /**
     * Updates the cursor postion label to refelect the position of the
     * cursor within the text.
     *
     * Use a negative value to to denote an unknown or invalid value.
     *
     * @param line The line the currsor is currently at.
     * @param column The column within the line that the cursor is currently at.
     */
    void UpdateCursorPositionLabel(int line, int column);

    /**
     * Zooms the current view with the new zoom slider value.
     *
     * @param slider_value The new value from the zoom slider.
     */
    void SliderZoom(int slider_value);

    /**
     * Updates the zoom slider to reflect the new zoom factor.
     *
     * @new_zoom_factor The new zoom factor.
     */
    void UpdateZoomSlider(float new_zoom_factor);

    /**
     * Updates the zoom label to reflect the state of the zoom slider.
     * This is needed so the user can see to what zoom value the slider
     * is being dragged to.
     *
     * @param slider_value The new value from the zoom slider.
     */
    void UpdateZoomLabel(int slider_value);

    /**
     * Updates the zoom label to reflect the new zoom factor.
     */
    void UpdateZoomLabel(float new_zoom_factor);

    /**
     * Creates a new tab from the section splitting operation.
     *
     * @param content The content of the "old" tab/resource.
     * @param originating_resource  The original resource from which the content
     *                              was extracted to create the "old" tab/resource.
     * @see FlowTab::SplitSection, FlowTab::OldTabRequest,
     *      BookViewEditor::SplitSection, Book::CreateSectionBreakOriginalResource
     */
    void CreateSectionBreakOldTab(QString content, HTMLResource &originating_resource);


    /**
     * Updates the selection/highlight in the Book Browser to the resource in the current tab
     *
     * @see BookBrowser::UpdateSelection
     */
    void UpdateBrowserSelectionToTab();

    /**
     * Creates new section/XHTML documents.
     */
    void SplitOnSGFSectionMarkers();

    void SetAutoSpellCheck(bool new_state);

    void ClearIgnoredWords();

    void RefreshSpellingHighlighting();

    void MergeResources(QList <Resource *> resources);

    void LinkStylesheetsToResources(QList <Resource *> resources);

    void ResourceUpdatedFromDisk(Resource &resource);

    void UpdateWord(QString old_word, QString new_word);
    void FindWord(QString word);

    /**
     * Return a map of stylesheets included/excluded for all given resources
     */
    QList<std::pair<QString, bool>> GetStylesheetsMap(QList<Resource *> resources);

    /**
     * Return the list of stylesheets linked to the given resource
     */
    QStringList GetStylesheetsAlreadyLinked(Resource *resource);

    void RemoveResources(QList<Resource *> resources = QList<Resource *>());

    void GenerateToc();
    void EditTOCDialog();
    void CreateHTMLTOC();

    void ChangeCasing(int casing_mode);

    void MarkSelection();
    void ClearMarkedText(ContentTab *old_tab = NULL);

    void ToggleViewState();

    void ApplyHeadingStyleToTab(const QString &heading_type);
    void SetPreserveHeadingAttributes(bool new_state);

    void GoBackFromLinkOrStyle();
    void GoToBookmark(LocationBookmark *locationBookmark);

    void GoToLinkedStyleDefinition(const QString &element_name, const QString &style_class_name);

    void ViewImageDialog(const QUrl &url);

    void BookmarkLocation();
    void BookmarkLinkOrStyleLocation();

    void GoToPreviewLocation();

    void ShowPasteClipboardHistoryDialog();

    void SetInsertedFileWatchResourceFile(const QString &pathname);

    void DeleteReportsStyles(QList<BookReports::StyleData *> reports_styles_to_delete);

    void DeleteFilenames(QStringList files_to_delete);
    void OpenFile(QString filename, int line = -1);

    void UpdateClipsUI();

    /**
     * support for plugins
     */
    void loadPluginsMenu();
    void unloadPluginsMenu();

private:
    void UpdateClipButton(int clip_number, QAction *ui_action);
    void InsertFiles(const QStringList &selected_images);
    void InsertFilesFromDisk();

    void ResetLinkOrStyleBookmark();
    void ResetLocationBookmark(LocationBookmark *locationBookmark);

    /**
     * Reads all the stored application settings like
     * window position, geometry etc.
     */
    void ReadSettings();

    /**
     * Writes all the stored application settings like
     * window position, geometry etc.
     */
    void WriteSettings();

    void SetDefaultViewState();

    /**
     * Gets called on possible saves and asks the user
     * does he want to save.
     * If the user chooses SAVE, we save and continue
     * If the user chooses DISCARD, we don't save and continue
     * If the user chooses CANCEL, we don't save and stop what we were doing
     *
     * @return \c true if we are allowed to proceed with the current operation.
     */
    bool MaybeSaveDialogSaysProceed();

    /**
     * Makes the provided book the current one.
     *
     * @param new_book The new book for editing.
     */
    void SetNewBook(QSharedPointer<Book> new_book);

    /**
     * Creates a new, empty book and replaces
     * the current one with it.
     */
    void CreateNewBook();

    /**
     * Saves the current book to the file specified.
     *
     * @param fullfilepath The path to save to.
     */
    bool SaveFile(const QString &fullfilepath, bool update_current_filename = true);

    /**
     * Performs zoom operations in the views using the default
     * zoom step. Setting zoom_in to \c true zooms the views *in*,
     * and a setting of \c false zooms them *out*. The zoom value
     * is first wrapped to the nearest zoom step (relative to the zoom direction).
     *
     * @param zoom_in If \c true, zooming in. Otherwise zooming out.
     */
    void ZoomByStep(bool zoom_in);

    /**
     * Sets the provided zoom factor on the active view editor.
     * Valid values are between ZOOM_MAX and ZOOM_MIN, others are ignored.
     *
     * @param new_zoom_factor The new zoom factor for the view.
     */
    void ZoomByFactor(float new_zoom_factor);

    /**
     * Converts a zoom factor to a value in the zoom slider range.
     *
     * @param zoom_factor The zoom factor being converted.
     * @return The converted slider range value.
     */
    static int ZoomFactorToSliderRange(float zoom_factor);

    /**
     * Converts a value in the zoom slider range to a zoom factor.
     *
     * @param slider_range_value The slider range value being converted.
     * @return The converted zoom factor value.
     */
    static float SliderRangeToZoomFactor(int slider_range_value);

    float GetZoomFactor();

    /**
     * Returns a map with keys being extensions of file types
     * we can save, and the values being filters for use in file dialogs.
     *
     * @return The save dialog filters.
     */
    static const QMap<QString, QString> GetSaveFiltersMap();

    /**
     * Sets the current file in the window title and also
     * updates the recent files list.
     *
     * @param fullfilepath The path to the currently edited file.
     */
    void UpdateUiWithCurrentFile(const QString &fullfilepath);

    /**
     * Selects the appropriate entry in the heading combo box
     * based on the provided name of the element.
     *
     * @param element_name The name of the currently selected element.
     */
    void SelectEntryOnHeadingToolbar(const QString &element_name);

    /**
     * Creates and adds the recent files actions
     * to the File menu.
     */
    void CreateRecentFilesActions();

    /**
     * Updates the recent files actions when the
     * list of files to be listed has changed.
     */
    void UpdateRecentFileActions();

    /**
     * Performs specific changes based on the OS platform.
     */
    void PlatformSpecificTweaks();

    /**
     * Extends the UI with extra widgets and tweaks.
     * Qt Designer is not able to create all the widgets
     * we want in the MainWindow, so we use this function
     * to extend the UI created by the Designer.
     */
    void ExtendUI();

    /**
     * Extends all the icons with 16px versions.
     * The prevents the use of automatic, blurry, scaled
     * down versions that Qt creates.
     */
    void ExtendIconSizes();

    /**
     * Loads the initial file provided to the MainWindow on creation.
     * If a file was provided to be loaded with this main window instance,
     * that file is loaded; if not, or it can't be opened, an empty file
     * is loaded.
     *
     * @param openfilepath The path to the file to load. Can be empty.
     */
    void LoadInitialFile(const QString &openfilepath, bool is_internal = false);

    /**
     * Connects all the required signals to their slots.
     */
    void ConnectSignalsToSlots();

    /**
     * Connects all the UI signals to the provided tab.
     *
     * @param tab The tab to connect the signals.
     */
    void MakeTabConnections(ContentTab *tab);

    /**
     * Disconnects all the UI signals from the provided tab.
     *
     * @param tab The tab from which to disconnect the signals.
     */
    void BreakTabConnections(ContentTab *tab);

    /**
     * Sets the view state of the current tab to view_state
     *
     * @param view_state - The view state to set.
     */
    void SetViewState(MainWindow::ViewState view_state);

    void SetupPreviewTimer();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * Warning messages to be displayed to the user after opening a book
     * if non-fatal errors occurred during loading. When Sigil is started
     * with a filename on command line, we must store these for display
     * after the UI main window is visible.
     */
    QStringList m_LastOpenFileWarnings;
    bool m_IsInitialLoad;

    /**
     * The path to the current file loaded.
     */
    QString m_CurrentFilePath;

    /**
     * The name of the current file loaded.
     */
    QString m_CurrentFileName;

    /**
     * The book currently being worked on.
     */
    QSharedPointer<Book> m_Book;

    /**
     * The last folder from which the user opened or saved a file.
     */
    QString m_LastFolderOpen;

    /**
     * The last filename used for Save As Copy
     */
    QString m_SaveACopyFilename;

    /**
     * The last file selected from Insert File, per book
     */
    QString m_LastInsertedFile;

    /**
     * The list of full filepaths
     * for the last MAX_RECENT_FILES files.
     * \c static because on Mac we have many MainWindows
     */
    static QStringList s_RecentFiles;

    /**
     * Array of recent files actions that are in the File menu.
     */
    QAction *m_RecentFileActions[ MAX_RECENT_FILES ];

    /**
     * The tab managing object.
     */
    TabManager &m_TabManager;

    /**
     * The Book Browser pane that lists all the files in the book.
     */
    BookBrowser *m_BookBrowser;

    ClipsWindow *m_Clips;

    /**
     * The find / replace widget.
     */

    FindReplace *m_FindReplace;

    MetaEditor *m_MetaEditor;

    /**
     * The Table of Contents pane that displays a rendered view of the NCX.
     */
    TableOfContents *m_TableOfContents;

    /**
     * The Validation Results pane that lists all the validation problems.
     */
    ValidationResultsView *m_ValidationResultsView;

    PreviewWindow *m_PreviewWindow;

    /**
     * The lable that displays the cursor position.
     * Line and column.
     */
    QLabel *m_lbCursorPosition;

    /**
     * The slider which the user can use to zoom.
     */
    QSlider *m_slZoomSlider;

    /**
     * The label that displays the current zoom factor.
     */
    QLabel *m_lbZoomLabel;

    /**
     * A map with keys being extensions of file types
     * we can load, and the values being filters for use in file dialogs.
     */
    const QMap<QString, QString> c_SaveFilters;

    /**
     * A map with keys being extensions of file types
     * we can save, and the values being filters for use in file dialogs.
     */
    const QMap<QString, QString> c_LoadFilters;

    /**
     * Holds the view state for new/switched tabs
     */
    MainWindow::ViewState m_ViewState;

    /**
     * Collects signals and sends specific parameters to the connected slots.
     */
    QSignalMapper *m_headingMapper;
    QSignalMapper *m_casingChangeMapper;

    /**
     * The Search Manager dialog
     */
    SearchEditor *m_SearchEditor;

    /**
     * The storage for SearchEditor
     */
    SearchEditorModel *m_SearchEditorModel;

    ClipEditor *m_ClipEditor;

    IndexEditor *m_IndexEditor;
    SpellcheckEditor *m_SpellcheckEditor;

    SelectCharacter *m_SelectCharacter;

    ViewImage *m_ViewImage;

    Reports *m_Reports;

    bool m_preserveHeadingAttributes;

    LocationBookmark *m_LinkOrStyleBookmark;

    ClipboardHistorySelector *m_ClipboardHistorySelector;

    /**
     * The last widget in this window that had focus that inherited PasteTarget.
     */
    PasteTarget *m_LastPasteTarget;

    bool m_ZoomPreview;

    /**
     * Workaround for Qt 4.8 bug, to track the last known window size when not maximized.
     */
    QByteArray m_LastWindowSize;

    QTimer &m_PreviewTimer;

    HTMLResource *m_PreviousHTMLResource;
    QString m_PreviousHTMLText;
    QList<ViewEditor::ElementIndex> m_PreviousHTMLLocation;

    /**
     * dynamically updated plugin menus and actions
     */
    QMenu *m_menuPlugins;
    QMenu *m_menuPluginsInput;
    QMenu *m_menuPluginsOutput;
    QMenu *m_menuPluginsEdit;
    QMenu *m_menuPluginsValidation;
    QAction *m_actionManagePlugins;
    bool m_SaveCSS;

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::MainWindow ui;
};

#endif // SIGIL_H


