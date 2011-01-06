/************************************************************************
**
**  Copyright (C) 2009, 2010  Strahinja Markovic
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

#include <QtGui/QMainWindow>
#include "ui_main.h"
#include "BookManipulation/Book.h"
#include "Tabs/ContentTab.h"
#include <QSharedPointer>
#include <QMutex>

const int MAX_RECENT_FILES = 5;
const int STATUSBAR_MSG_DISPLAY_TIME = 2000;

class QComboBox;
class QLabel;
class QSlider;
class FindReplace;
class TabManager;
class BookBrowser;
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
    ContentTab& GetCurrentContentTab(); 

    /**
     * Opens the specified resource in the specified view state.
     * 
     * @param resource The resource to open.
     * @param view_state The state the resource should be opened in.
     */
    void OpenResource( Resource &resource, ContentTab::ViewState view_state );

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
     * Implements Replace action functionality.
     */
    void Replace();
   
    /**
     * Implements Zoom In action functionality.
     */
    void ZoomIn();

    /**
     * Implements Zoom Out action functionality.
     */
    void ZoomOut();

    /**
     * Implements Insert image action functionality.
     */
    void InsertImage();

    /**
     * Implements Meta Editor action functionality.
     */
    void MetaEditorDialog();

    /**
     * Implements TOC Editor action functionality.
     */
    void TOCEditorDialog();

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
     * Creates new chapters/XHTML documents.
     * 
     * @param new_chapters The contents of the new chapters.
     * @see Book::CreateNewChapters
     */
    void CreateNewChapters( QStringList new_chapters );

    /**
     * Sets the new state of the option that controls 
     * whether to clean with Tidy or not.
     *
     * @param new_state The new state of the option.
     */
    void SetTidyCleanOption( bool new_state );

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
     * @param update_ui If \c true, then the UI will be notified
     *                  that the file was saved. Otherwise, it's a 
     *                  "silent" save.
     */    
    bool SaveFile( const QString &fullfilepath, bool update_ui = true );

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
     * Selects the appropriate entry in the heading combo box 
     * based on the provided name of the element.
     *
     * @param element_name The name of the currently selected element.
     */
    void SelectEntryInHeadingCombo( const QString &element_name );

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
    QString m_LastFolderImage;

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
     * The headings drop-down combo box.
     */
    QComboBox *m_cbHeadings;

    /**
     * The tab managing object.
     */
    TabManager &m_TabManager;

    /**
     * The Book Browser pane that lists all the files in the book.
     */
    BookBrowser *m_BookBrowser;

    /**
     * The Validation Results pane that lists all the validation problems.
     */
    ValidationResultsView *m_ValidationResultsView;

    /**
     * The slider which the user can use to zoom.
     */
    QSlider *m_slZoomSlider;

    /**
     *  The label that displays the current zoom factor.
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
     * A guarded pointer to the FindReplace dialog. 
     * Used to make sure we always have one FindReplace dialog.
     */
    QWeakPointer< FindReplace > m_FindReplace;

    /**
     * Holds all the widgets Qt Designer created for us.
     */
    Ui::MainWindow ui;
};

#endif // SIGIL_H


