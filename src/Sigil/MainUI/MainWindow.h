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
#include "../BookManipulation/Book.h"
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



/**
 * @mainpage 
 * The conversion of all source comments to Doxygen format
 * is in progress. Some files have been converted, others have not.
 * 
 * Be patient.
 */


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    // Constructor.
    // The first argument is the path to the file that the window
    // should load (new file loaded if empty); the second is the
    // windows parent; the third specifies the flags used to modify window behaviour
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

protected:

    // Overrides the closeEvent handler so we can check
    // for saved status before actually closing
    void closeEvent( QCloseEvent *event );

private slots:

    // Implements New action functionality
    void New();

    // Implements Open action functionality
    void Open();

    // Implements Open recent file action functionality
    void OpenRecentFile();

    // Implements Save action functionality
    bool Save();

    // Implements Save As action functionality
    bool SaveAs();

    // Implements Find action functionality
    void Find();

    // Implements Replace action functionality
    void Replace();
   
    // Implements Zoom In action functionality
    void ZoomIn();

    // Implements Zoom Out action functionality
    void ZoomOut();

    // Implements Insert image action functionality
    void InsertImage();

    // Implements Meta Editor action functionality
    void MetaEditorDialog();

    // Implements TOC Editor action functionality
    void TOCEditorDialog();

    // Implements User Manual action functionality
    void UserManual();

    // Implements Frequently Asked Questions action functionality
    void FrequentlyAskedQuestions();

    // Implements Report An Issue action functionality
    void ReportAnIssue();

    // Implements Sigil Dev Blog action functionality
    void SigilDevBlog();

    // Implements About action functionality
    void AboutDialog();

    void TabChanged( ContentTab* old_tab, ContentTab* new_tab ); 

    void UpdateUI();

    void TabSwitchChanges();

    // Set initial state for actions in Book View
    // (enable the actions the Code View disabled)
    void SetStateActionsBookView();

    // Set initial state for actions in Code View
    // (disable the actions used in Book View that
    // are not appropriate here)
    void SetStateActionsCodeView();

    void SetStateActionsRawView();

    void SetStateActionsStaticView();

    // Zooms the current view with the new zoom slider value
    void SliderZoom( int slider_value );

    // Updates the zoom controls by reading the current
    // zoom factor from the view. Needed on View changeover.
    void UpdateZoomControls();

    // Updates the zooming slider to reflect the new zoom factor
    void UpdateZoomSlider( float new_zoom_factor );

    // Updates the zoom label to reflect the state of the zoom slider.
    // This is needed so the user can see to what zoom value the slider
    // is being dragged to.
    void UpdateZoomLabel( int slider_value );

    // Updates the zoom label to reflect the new zoom factor
    void UpdateZoomLabel( float new_zoom_factor );

    void CreateChapterBreakOldTab( QString content, HTMLResource& originating_resource );

    void CreateNewChapters( QStringList new_chapters );

private:

    // Reads all the stored application settings like
    // window position, geometry etc.
    void ReadSettings();

    // Writes all the stored application settings like
    // window position, geometry etc.
    void WriteSettings();

    // Gets called on possible saves; asks the user
    // does he want to save; 
    // if the user chooses SAVE, we save and continue
    // if the user chooses DISCARD, we don't save and continue
    // if the user chooses CANCEL, we don't save and stop what we were doing
    bool MaybeSave();

    void SetNewBook( QSharedPointer< Book > new_book );
    
    // Creates a new, empty book and replaces
    // the current one with it
    void CreateNewBook();

    // Loads from the file specified
    void LoadFile( const QString &filename );

    // Saves to the file specified
    bool SaveFile( const QString &filename );

    // Performs zoom operations in the views using the default
    // zoom step. Setting zoom_in to true zooms the views *in*,
    // and a setting of false zooms them *out*. The zoom value
    // is first wrapped to the nearest zoom step (relative to the zoom direction).
    void ZoomByStep( bool zoom_in );

    // Sets the provided zoom factor on the active view editor.
    // Valid values are between ZOOM_MAX and ZOOM_MIN, others are ignored.
    void ZoomByFactor( float new_zoom_factor );

    // Converts a zoom factor to a value in the zoom slider range
    int ZoomFactorToSliderRange( float zoom_factor ) const;

    // Converts a value in the zoom slider range to a zoom factor
    float SliderRangeToZoomFactor( int slider_range_value ) const;    

    // Returns a map with keys being extensions of file types
    // we can load, and the values being filters for use in file dialogs
    static const QMap< QString, QString > GetLoadFiltersMap();

    // Returns a map with keys being extensions of file types
    // we can save, and the values being filters for use in file dialogs
    static const QMap< QString, QString > GetSaveFiltersMap();

    /**
     * Returns the currently active MainWindow. 
     *
     * @return The currently active MainWindow.
     */
    static MainWindow& GetCurrentMainWindow();

    // Sets the current file in window title;
    // updates the recent files list
    void SetCurrentFile( const QString &filename );

    // Selects the appropriate entry in the heading combo box
    // based on the provided name of the element
    void SelectEntryInHeadingCombo( const QString &element_name );

    // Creates and adds the recent files actions
    // to the File menu
    void CreateRecentFilesActions();

    // Updates the recent files actions when the
    // list of files to be listed has changed
    void UpdateRecentFileActions();

    // Qt Designer is not able to create all the widgets
    // we want in the MainWindow, so we use this function
    // to extend the UI created by the Designer
    void ExtendUI();

    // If a file was provided to be loaded
    // with this main window instance, that file is loaded;
    // if not, or it can't be opened, an empty file is loaded
    void LoadInitialFile( const QString &openfilepath );

    // Connects all the required signals to their slots
    void ConnectSignalsToSlots();

    void MakeTabConnections( ContentTab *tab );

    void BreakTabConnections( ContentTab *tab );


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // The filename of the current file loaded
    QString m_CurrentFile;

    // The book currently being worked on
    QSharedPointer< Book > m_Book;

    // The last folder from which the user opened a file
    QString m_LastFolderOpen;

    // The last folder to which the user saved a file
    QString m_LastFolderSave;

    // The last folder to which the user imported an image;
    QString m_LastFolderImage;

    // The list of full file names/paths
    // for the last MAX_RECENT_FILES files;
    // static because on Mac we have many MainWindows
    static QStringList s_RecentFiles;

    /**
     * Protects the status bar showMessage() func
     * from being called from several threads at once.
     */
    QMutex m_StatusBarMutex;

    // Array of recent files actions that are in the File menu;
    QAction *m_RecentFileActions[ MAX_RECENT_FILES ];

    // The headings drop-down combo box
    QComboBox *m_cbHeadings;

    TabManager &m_TabManager;

    BookBrowser *m_BookBrowser;

    // The slider which the user can use to zoom
    QSlider *m_slZoomSlider;

    // The label that displays the zoom factor
    QLabel *m_lbZoomLabel;

    // A map with keys being extensions of file types
    // we can load, and the values being filters for use in file dialogs
    const QMap< QString, QString > c_SaveFilters;

    // A map with keys being extensions of file types
    // we can save, and the values being filters for use in file dialogs
    const QMap< QString, QString > c_LoadFilters;

    // A guarded pointer to the FindReplace dialog;
    QWeakPointer< FindReplace > m_FindReplace;

    // Holds all the widgets Qt Designer created for us
    Ui::MainWindow ui;
};

#endif // SIGIL_H


