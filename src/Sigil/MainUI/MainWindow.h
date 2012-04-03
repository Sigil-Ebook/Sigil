/************************************************************************
**
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

#include <QtCore/QMutex>
#include <QtCore/QSharedPointer>
#include <QtGui/QMainWindow>

#include "ui_main.h"
#include "BookManipulation/Book.h"
#include "MainUI/NCXModel.h"
#include "Tabs/ContentTab.h"

const int MAX_RECENT_FILES = 5;
const int STATUSBAR_MSG_DISPLAY_TIME = 20000;

class QComboBox;
class QLabel;
class QSignalMapper;
class QSlider;
class FindReplace;
class TabManager;
class BookBrowser;
class TableOfContents;
class ValidationResultsView;


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
    MainWindow( const QString &openfilepath = QString(), QWidget *parent = 0, Qt::WFlags flags = 0 );

    /**
     * The book currently being edited.
     *
     * @return A shared pointer to the book.
     */
    QSharedPointer< Book > GetCurrentBook();

    /**
     * Returns a reference to the current content tab.
     *
     * @return A reference to the current content tab.
     */
    ContentTab &GetCurrentContentTab();

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
     * Saves the current Book Browser selected entries
     *
     */
    void SaveBrowserSelection();

    /**
     * Restores the Book Browser selected entries
     *
     */
    void RestoreBrowserSelection();

    /**
     * Describes the type of the View mode
     * currently used in the tab.
     */
    enum ViewState
    {
        ViewState_Unknown,          /**< Default non view that we don't know what it is */
        ViewState_BookView,         /**< The WYSIWYG view. */
        ViewState_CodeView,         /**< The XHTML code editing view. */
        ViewState_SplitView,        /**< We have a split view type (HTML resources support this) */
        ViewState_RawView,          /**< The view for editing non-XHTML related resources. */
        ViewState_StaticView       /**< The static view for non-editable content. */
    };

    /**
     * Returns the status bar mutex used to protect 
     * write access to the MainWindow's status bar.
     * 
     * @return The status bar mutex.
     */
    QMutex& GetStatusBarMutex();

    /**
     * Shows a message on the status bar of the current MainWindow.
     *
     * @param message The message to display.
     * @param millisecond_duration The millisecond duration during
     *                             which the message should be displayed.
     *
     * @note This function is thread-safe.
     */
    static void ShowMessageOnCurrentStatusBar( const QString &message, 
                                               int millisecond_duration = STATUSBAR_MSG_DISPLAY_TIME );

    /**
     * Returns the current state of the Tidy clean option,
     * as specified by the user.
     */
    static bool ShouldUseTidyClean();

    /**
     * Returns the current view state.
     *
     * @return The current view state.
     */
    MainWindow::ViewState GetViewState();

    /**
     * Sets the current state to CodeView or SplitView CodeView
     * depending on whether view was split view already
     */
    void AnyCodeView();

    bool CloseAllTabs();

    void SaveTabData();

public slots:
    /**
     * Opens the specified resource in the specified view state.
     *
     * @param resource The resource to open.
     * @param view_state The state the resource should be opened in.
     */
    void OpenResource( Resource& resource,
                       bool precede_current_tab = false,
                       const QUrl &fragment = QUrl(),
                       MainWindow::ViewState view_state = MainWindow::ViewState_Unknown,
                       int line_to_scroll_to = -1);

signals:
    void SettingsChanged();

protected:

    /**
     * Overrides the closeEvent handler so we can check
     * for saved status before actually closing.
     *
     * @param event The close event.
     */
    void closeEvent( QCloseEvent *event );

private slots:
    
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

    /**
     * Implements Insert image action functionality.
     */
    void InsertImage();

    /**
     * Implements the set BookView functionality.
     */
    void BookView();

    /**
     * Implements the set SplitView functionality.
     */
    void SplitView();

    /**
     * Implements the set CodeView functionality.
     */
    void CodeView();

    /**
     * Implements Meta Editor action functionality.
     */
    void MetaEditorDialog();

    /**
     * Implements User Manual action functionality.
     */
    void UserManual();

    /**
     * Implements Frequently Asked Questions action functionality.
     */
    void FrequentlyAskedQuestions();

    /**
     * Implements Donate action functionality.
     */
    void Donate();

    /**
     * Implements Report An Issue action functionality.
     */
    void ReportAnIssue();

    /**
     * Implements Sigil Dev Blog action functionality.
     */
    void SigilDevBlog();

    /**
     * Implements About action functionality.
     */
    void AboutDialog();

    /**
     * Implementes Preferences action functionality.
     */
    void PreferencesDialog();

    /**
     * Implements Validate Epub action functionality.
     */
    void ValidateEpub();

    /**
     * Disconnects all signals to the old tab 
     * and reconnects them to the new tab when the
     * current tab is changed.
     *
     * @old_tab The tab that was previously in use.
     * @new_tab The tab that is becoming current.
     */
    void ChangeSignalsWhenTabChanges( ContentTab* old_tab, ContentTab* new_tab ); 

    void UpdateViewState();

    /**
     * Updates the toolbars based on current tab state and changes.
     */
    void UpdateUIOnTabChanges();

    /**
     * Performs needed changes when the user switches tabs.
     */
    void UpdateUiWhenTabsSwitch();

    /**
     * Set initial state for actions in Book View 
     * (enable the actions the Code View disabled).
     */
    void SetStateActionsBookView();

    void SetStateActionsSplitView();

    /**
     * Set initial state for actions in Code View 
     * (disable the actions used in Book View that
     * are not appropriate here).
     */
    void SetStateActionsCodeView();

    /**
     * Set initial state for actions in Raw View
     * (same as Code View, but, the actions for switching
     * views are off as well; Raw View is for CSS, XML ... editing).
     */
    void SetStateActionsRawView();

    /**
     * Set initial state for actions in Static View
     * (everything dead, used for viewing images etc.)
     */
    void SetStateActionsStaticView();

    /**
     * Updates the cursor postion label to refelect the position of the
     * cursor within the text.
     *
     * Use a negative value to to denote an unknown or invalid value.
     *
     * @param line The line the currsor is currently at.
     * @param column The column within the line that the cursor is currently at.
     */
    void UpdateCursorPositionLabel( int line, int column );

    /**
     * Zooms the current view with the new zoom slider value.
     * 
     * @param slider_value The new value from the zoom slider.
     */
    void SliderZoom( int slider_value );

    /**
     * Updates the zoom controls by reading the current 
     * zoom factor from the View. Needed on View changeover.
     */
    void UpdateZoomControls();

    /**
     * Updates the zoom slider to reflect the new zoom factor.
     *
     * @new_zoom_factor The new zoom factor.
     */
    void UpdateZoomSlider( float new_zoom_factor );

    /**
     * Updates the zoom label to reflect the state of the zoom slider. 
     * This is needed so the user can see to what zoom value the slider
     * is being dragged to.
     *
     * @param slider_value The new value from the zoom slider.
     */
    void UpdateZoomLabel( int slider_value );

    /**
     * Updates the zoom label to reflect the new zoom factor.
     */
    void UpdateZoomLabel( float new_zoom_factor );

    /**
     * Creates a new tab from the chapter splitting operation.
     *
     * @param content The content of the "old" tab/resource.
     * @param originating_resource  The original resource from which the content
     *                              was extracted to create the "old" tab/resource.
     * @see FlowTab::SplitChapter, FlowTab::OldTabRequest,
     *      BookViewEditor::SplitChapter, Book::CreateChapterBreakOriginalResource
     */
    void CreateChapterBreakOldTab( QString content, HTMLResource& originating_resource );


    /**
     * Updates the selection/highlight in the Book Browser to the resource in the current tab
     *
     * @see BookBrowser::UpdateSelection
     */
    void UpdateBrowserSelectionToTab();

    /**
     * Creates new chapters/XHTML documents.
     * 
     * @param new_chapters The contents of the new chapters.
     * @param originating_resource The original HTML chapter that chapters
     * will be created after.
     * @see Book::CreateNewChapters
     */
    void CreateNewChapters( QStringList new_chapters, HTMLResource &originalResource );

    /**
     * Sets the new state of the option that controls 
     * whether to clean with Tidy or not.
     *
     * @param new_state The new state of the option.
     */
    void SetTidyCleanOption( bool new_state );

    /**
     * Sets the new state of the option that controls
     * whether to check or auto fix well-formed wrrors.
     *
     * @param new_state The new state of the option.
     */
    void SetCheckWellFormedErrors( bool new_state );

    /**
     * Sets the view state of the current tab to the saved state
     */
    void SetTabViewState();

    void MergeResources(QList <Resource *> resources);
    void GenerateToc();
    void GenerateInlineToc(NCXModel::NCXEntry ncx_root_entry);

private:

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
    void SetNewBook( QSharedPointer< Book > new_book );
    
    /**
     * Creates a new, empty book and replaces 
     * the current one with it.
     */
    void CreateNewBook();

    /**
     * Loads a book from the file specified.
     *
     * @param fullfilepath The path to the file to load.
     */
    void LoadFile( const QString &fullfilepath );

    /**
     * Saves the current book to the file specified.
     *
     * @param fullfilepath The path to save to.
     */    
    bool SaveFile( const QString &fullfilepath );

    /**
     * Performs zoom operations in the views using the default 
     * zoom step. Setting zoom_in to \c true zooms the views *in*,
     * and a setting of \c false zooms them *out*. The zoom value
     * is first wrapped to the nearest zoom step (relative to the zoom direction).
     *
     * @param zoom_in If \c true, zooming in. Otherwise zooming out.
     */
    void ZoomByStep( bool zoom_in );

    /**
     * Sets the provided zoom factor on the active view editor. 
     * Valid values are between ZOOM_MAX and ZOOM_MIN, others are ignored.
     *
     * @param new_zoom_factor The new zoom factor for the view.
     */
    void ZoomByFactor( float new_zoom_factor );

    /**
     * Converts a zoom factor to a value in the zoom slider range.
     *
     * @param zoom_factor The zoom factor being converted.
     * @return The converted slider range value.
     */
    static int ZoomFactorToSliderRange( float zoom_factor );

    /**
     * Converts a value in the zoom slider range to a zoom factor.
     *
     * @param slider_range_value The slider range value being converted.
     * @return The converted zoom factor value.
     */
    static float SliderRangeToZoomFactor( int slider_range_value );    

    /**
     * Returns a map with keys being extensions of file types
     * we can load, and the values being filters for use in file dialogs.
     *
     * @return The load dialog filters.
     */
    static const QMap< QString, QString > GetLoadFiltersMap();

    /**
     * Returns a map with keys being extensions of file types 
     * we can save, and the values being filters for use in file dialogs.
     *
     * @return The save dialog filters.
     */
    static const QMap< QString, QString > GetSaveFiltersMap();

    /**
     * Returns the currently active MainWindow. 
     *
     * @return The currently active MainWindow.
     */
    static MainWindow& GetCurrentMainWindow();

    /**
     * Sets the current file in the window title and also 
     * updates the recent files list.
     *
     * @param fullfilepath The path to the currently edited file.
     */
    void UpdateUiWithCurrentFile( const QString &fullfilepath );

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
    void LoadInitialFile( const QString &openfilepath );

    /**
     * Connects all the required signals to their slots.
     */
    void ConnectSignalsToSlots();

    /**
     * Connects all the UI signals to the provided tab.
     *
     * @param tab The tab to connect the signals.
     */
    void MakeTabConnections( ContentTab *tab );

    /**
     * Disconnects all the UI signals from the provided tab.
     *
     * @param tab The tab from which to disconnect the signals.
     */
    void BreakTabConnections( ContentTab *tab );

    /**
     * Sets the view state of the current tab to view_state
     *
     * @param view_state - The view state to set.
     */
    void SetViewState( MainWindow::ViewState view_state );

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The path to the current file loaded.
     */
    QString m_CurrentFilePath;

    /**
     * The book currently being worked on.
     */
    QSharedPointer< Book > m_Book;

    /**
     * The last folder from which the user opened a file.
     */
    QString m_LastFolderOpen;

    /**
     * The last folder to which the user saved a file.
     */
    QString m_LastFolderSave;

    /**
     * The last folder to which the user imported an image.
     */
    QString m_LastFolderAdd;

    /**
     * The list of full filepaths 
     * for the last MAX_RECENT_FILES files.
     * \c static because on Mac we have many MainWindows
     */
    static QStringList s_RecentFiles;

    /**
     * Protects the status bar showMessage() func
     * from being called from several threads at once.
     */
    QMutex m_StatusBarMutex;

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

    /**
     * The find / replace widget.
     */
    FindReplace *m_FindReplace;

    /**
     * The Table of Contents pane that displays a rendered view of the NCX.
     */
    TableOfContents *m_TableOfContents;

    /**
     * The Validation Results pane that lists all the validation problems.
     */
    ValidationResultsView *m_ValidationResultsView;

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
    const QMap< QString, QString > c_SaveFilters;

    /**
     * A map with keys being extensions of file types 
     * we can save, and the values being filters for use in file dialogs.
     */
    const QMap< QString, QString > c_LoadFilters;

    /**
     * Holds the state of whether the user wants tidy to be used
     * when cleaning XHTML.
     */
    static bool m_ShouldUseTidy;

    /**
     * Holds the state of wheter the user wants to be informed
     * about well-formed errors or if they should be auto fixed.
     */
    bool m_CheckWellFormedErrors;

    /**
     * Holds the view state for new/switched tabs
     */
    MainWindow::ViewState m_ViewState;

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::MainWindow ui;
};

#endif // SIGIL_H


