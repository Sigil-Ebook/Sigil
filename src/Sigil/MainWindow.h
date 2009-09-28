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

const int MAX_RECENT_FILES = 5;

class QComboBox;
class QWebView;
class CodeViewEditor;
class BookViewEditor;
class ViewEditor;
class QLabel;
class QSlider;


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

    // Implements Undo action functionality
    void Undo();

    // Implements Redo action functionality
    void Redo();

    // Implements Cut action functionality
    void Cut();

    // Implements Copy action functionality
    void Copy();

    // Implements Paste action functionality
    void Paste();

    // Implements Bold action functionality
    void Bold();

    // Implements Italic action functionality
    void Italic();

    // Implements Underline action functionality
    void Underline();
    
    // Implements Strikethrough action functionality
    void Strikethrough();

    // Implements Align Left action functionality
    void AlignLeft();

    // Implements Center action functionality
    void Center();

    // Implements Align Right action functionality
    void AlignRight();

    // Implements Justify action functionality
    void Justify();

    // Implements Book View action functionality
    void BookView();

    // Implements Split View action functionality
    void SplitView();

    // Implements Code View action functionality
    void CodeView();

    // Implements Insert chapter break action functionality
    void InsertChapterBreak();

    // Implements Insert image action functionality
    void InsertImage();

    // Implements Insert bulleted list action functionality
    void InsertBulletedList();

    // Implements Insert numbered list action functionality
    void InsertNumberedList();

    // Implements the heading combo box functionality
    void HeadingStyle( const QString& heading_type );
    
    // Implements Print Preview action functionality
    void PrintPreview();

    // Implements Print action functionality
    void Print();

    // Implements Zoom In action functionality
    void ZoomIn();

    // Implements Zoom Out action functionality
    void ZoomOut();

    // Implements Meta Editor action functionality
    void MetaEditorDialog();

    // Implements TOC Editor action functionality
    void TOCEditorDialog();

    // Implements About action functionality
    void AboutDialog();

    // Used to catch the focus changeover from one widget
    // (code or book view) to the other; needed for source synchronization
    void FocusFilter( QWidget *old_widget, QWidget *new_widget );

    // Gets called every time the document is modified;
    // changes the UI to accordingly;
    // (star in titlebar on win and lin, different button colors on mac)
    void DocumentWasModified();

    // Updates user interface (action states etc.) based on 
    // the current selection in Book View
    void UpdateUIBookView(); 

    // Updates user interface (action states etc.) based on 
    // the current selection in Code View
    void UpdateUICodeView();

    // Set initial state for actions in Book View
    // (enable the actions the Code View disabled)
    void SetStateActionsBookView();

    // Set initial state for actions in Code View
    // (disable the actions used in Book View that
    // are not appropriate here)
    void SetStateActionsCodeView();

    // Updates the m_Book variable whenever
    // the user edits in book view
    void UpdateSourceFromBookView();

    // Updates the m_Book variable whenever
    // the user edits in code view
    void UpdateSourceFromCodeView();

    // On changeover, updates the code in code view
    void UpdateCodeViewFromSource();

    // On changeover, updates the code in book view
    void UpdateBookViewFromSource();

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
    bool IsSupportedSaveType( const QString &filename );

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

    // Returns the currently active View Editor
    ViewEditor* GetActiveViewEditor() const;

    // Runs HTML Tidy on sSource variable
    void TidyUp();

    // Removes every occurrence of class="Apple-style-span"
    // with which webkit litters our source code 
    void RemoveAppleClasses();

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


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // true if the last view the user edited in was book view
    bool m_isLastViewBook;

    // The filename of the current file loaded
    QString m_CurrentFile;

    // The book currently being worked on
    Book m_Book;

    // The value of the m_Book.sSource variable
    // after the last view change
    QString m_OldSource;

    // The last folder from which the user opened a file
    QString m_LastFolderOpen;

    // The last folder to which the user saved a file
    QString m_LastFolderSave;

    // The last folder to which the user imported an image
    QString m_LastFolderImage;

    // The list of full file names/paths
    // for the last MAX_RECENT_FILES files;
    // static because on Mac we have many MainWindows
    static QStringList m_RecentFiles;

    // Array of recent files actions that are in the File menu;
    QAction *m_RecentFileActions[ MAX_RECENT_FILES ];

    // The headings drop-down combo box
    QComboBox *m_cbHeadings;

    // The webview component that renders out HTML
    BookViewEditor *m_wBookView;

    // The plain text code editor 
    CodeViewEditor *m_wCodeView;

    // The slider which the user can use to zoom
    QSlider *m_slZoomSlider;

    // The label that displays the zoom factor
    QLabel *m_lbZoomLabel;

    // Holds all the widgets Qt Designer created for us
    Ui::MainWindow ui;
};

#endif // SIGIL_H


