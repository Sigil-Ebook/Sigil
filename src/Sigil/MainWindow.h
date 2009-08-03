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
#include "Book.h"

const int MAX_RECENT_FILES = 5;

class XHTMLHighlighter;
class QComboBox;
class QWebView;
class CodeViewEditor;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    // Constructor
    MainWindow( QWidget *parent = 0, Qt::WFlags flags = 0 );

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

    // Loads from the file specified
    void LoadFile( const QString &filename );

    // Saves to the file specified
    bool SaveFile( const QString &filename );

    // Returns true if the provided extension is supported as a save type
    bool IsSupportedSaveType( const QString &filename );

    // Runs HTML Tidy on sSource variable
    void TidyUp();

    // Sets the current file in window title;
    // updates the recent files list
    void SetCurrentFile( const QString &filename );

    // Removes every occurrence of class="Apple-style-span"
    // with which webkit litters our source code 
    void RemoveAppleClasses();

    // Executes the specified command on the document with javascript
    void ExecCommand( const QString &command );

    // Executes the specified command with the specified parameter
    // on the document with javascript
    void ExecCommand( const QString &command, const QString &parameter );

    // Returns the state of the JavaScript command provided
    bool QueryCommandState( const QString &command );

    // Returns the name of the element the caret is located in;
    // if text is selected, returns the name of the element
    // where the selection *starts*
    QString GetCursorElementName();

    // Selects the appropriate entry in the heading combo box
    // based on the provided name of the element
    void SelectEntryInHeadingCombo( const QString &element_name );

    // Initializes the code view
    void SetUpCodeView();	

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

    // The list of full file names/paths
    // for the last MAX_RECENT_FILES files
    QStringList m_RecentFiles;

    // Array of recent files actions that are in the File menu
    QAction *m_RecentFileActions[ MAX_RECENT_FILES ];

    // The highlighter for the Code View
    XHTMLHighlighter *m_Highlighter;

    // The headings drop-down combo box
    QComboBox *m_cbHeadings;

    // The webview component that renders out HTML
    QWebView *m_wBookView;

    // The plain text code editor 
    CodeViewEditor *m_wCodeView;

    // Holds all the widgets Qt Designer created for us
    Ui::MainWindow ui;
};

#endif // SIGIL_H
