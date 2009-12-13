/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#include <QWeakPointer>

const int MAX_RECENT_FILES = 5;

class QComboBox;
class QLabel;
class QSlider;
class FindReplace;
class TabManager;
class BookBrowser;
class ContentTab;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    // Constructor.
    // The first argument is the path to the file that the window
    // should load (new file loaded if empty); the second is the
    // windows parent; the third specifies the flags used to modify window behaviour
    MainWindow( const QString &openfilepath = QString(), QWidget *parent = 0, Qt::WFlags flags = 0 );

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

    // Implements Meta Editor action functionality
    void MetaEditorDialog();

    // Implements TOC Editor action functionality
    void TOCEditorDialog();

    // Implements Report An Issue action functionality
    void ReportAnIssue();

    // Implements About action functionality
    void AboutDialog();

    // Gets called every time the document is modified;
    // changes the UI to accordingly;
    // (star in titlebar on win and lin, different button colors on mac)
    void DocumentWasModified();

    void TabChanged( ContentTab* old_tab, ContentTab* new_tab ); 

    void UpdateUI();

    // Set initial state for actions in Book View
    // (enable the actions the Code View disabled)
    void SetStateActionsBookView();

    // Set initial state for actions in Code View
    // (disable the actions used in Book View that
    // are not appropriate here)
    void SetStateActionsCodeView();

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
    
    // Creates a new, empty book and replaces
    // the current one with it
    void CreateNew();

    // Loads from the file specified
    void LoadFile( const QString &filename );

    // Saves to the file specified
    bool SaveFile( const QString &filename );

    // Returns true if the provided extension is supported as a save type
    bool IsSupportedSaveType( const QString &filename ) const;

    // Accepts the path to saved epub and a reference
    // to the main book. Calls calibre (in a special way)
    // with a path to a temp copy of the saved epub.
    // Needs to be called in a separate thread.
    static void CalibreInterop( QString filepath, Book &book );

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
    const QMap< QString, QString > GetLoadFiltersMap() const;

    // Returns a map with keys being extensions of file types
    // we can save, and the values being filters for use in file dialogs
    const QMap< QString, QString > GetSaveFiltersMap() const;

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
    Book m_Book;

    // The last folder from which the user opened a file
    QString m_LastFolderOpen;

    // The last folder to which the user saved a file
    QString m_LastFolderSave;

    // The list of full file names/paths
    // for the last MAX_RECENT_FILES files;
    // static because on Mac we have many MainWindows
    static QStringList s_RecentFiles;

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
    QWeakPointer<FindReplace> m_FindReplace;

    // Holds all the widgets Qt Designer created for us
    Ui::MainWindow ui;
};

#endif // SIGIL_H


