/************************************************************************
**
**  Copyright (C) 2011  John Schember <john@nachtimwald.com>
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
#ifndef FINDREPLACE_H
#define FINDREPLACE_H

#include <QtCore/QTimer>

#include "ui_FindReplace.h"
#include "BookManipulation/FolderKeeper.h"
#include "MainUI/MainWindow.h"
#include "Misc/SearchOperations.h"
#include "ViewEditors/Searchable.h"

class HTMLResource;
class Resource;

class FindReplace : public QWidget
{
    Q_OBJECT

public:
    FindReplace( MainWindow &main_window );
    ~FindReplace();

    // Capabilities for the F&R.
    enum CAPABILITY {
        CAPABILITY_NONE =                      0,
        CAPABILITY_ALL =                 (1 << 0),
        CAPABILITY_FIND =                (1 << 1),
        CAPABILITY_FIND_COUNT =          (1 << 2),
        CAPABILITY_REPLACE =             (1 << 3),
        CAPABILITY_REPLACE_ALL =         (1 << 4),
        CAPABILITY_MODE_NORMAL =         (1 << 5),
        CAPABILITY_MODE_CASE_SENSITIVE = (1 << 6),
        CAPABILITY_MODE_REGEX =          (1 << 7),
        CAPABILITY_MODE_SPELL_CHECK =    (1 << 8),
        CAPABILITY_LOOK_CURRENT =        (1 << 9),
        CAPABILITY_LOOK_ALL_HTML =       (1 << 10),
        CAPABILITY_LOOK_SELECTED_HTML =  (1 << 11)
    };
    typedef unsigned int FR_Capabilities;

    /**
     * Defines possible areas where the search can be performed.
     */
    enum LookWhere
    {
        LookWhere_CurrentFile = 0,
        LookWhere_AllHTMLFiles = 10,
        LookWhere_SelectedHTMLFiles = 20
    };

    enum SearchMode
    {
        // Case insensitive
        SearchMode_Normal = 0,
        SearchMode_Case_Sensitive = 10,
        SearchMode_Regex = 20,
        SearchMode_SpellCheck = 40
    };

    enum SearchDirection
    {
        SearchDirection_Down = 0,
        SearchDirection_Up = 10
    };

    /**
     * Sets up the default Find text during dialog creation.
     */
    void SetUpFindText();

    void SetCapabilities(FR_Capabilities caps);

public slots:
    void close();
    void show();

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:

    // Shows a message in the main window.
    void ShowMessage( const QString &message );

    // Uses the find direction to determine if we should find next
    // or previous.
    void Find();

    void FindNext();

    void FindPrevious();

    // Counts the number of occurrences of the user's
    // term in the document.
    void Count();

    // Uses the find direction to determine if we should replace next
    // or previous.
    void Replace();

    // Replaces the user's search term with the user's
    // replacement text if a match is selected. If it's not,
    // calls FindNext() so it becomes selected.
    void ReplaceNext();

    void ReplacePrevious();

    // Replaces the user's search term with the user's
    // replacement text in the entire document. Shows a
    // dialog telling how many occurrences were replaced.
    void ReplaceAll();

    void clearMessage();

    void expireMessage();

    void HideFindReplace();

private:
    void FindText( Searchable::Direction direction );
    void ReplaceText( Searchable::Direction direction );

    /**
     * Checks if book-wide searching is allowed for the current view.
     *
     * @return \c true if book-wide searching is allowed.
     */
    void SetCodeViewIfNeeded( bool force = false );

    // Displays a message to the user informing him
    // that his last search term could not be found.
    void CannotFindSearchTerm();

    Searchable* GetAvailableSearchable();

    // Constructs a searching regex from the selected
    // options and fields and then returns it.
    QString GetSearchRegex();

    QList <Resource *> GetHTMLFiles();

    bool IsCurrentFileInHTMLSelection();

    int CountInFiles();

    int ReplaceInAllFiles();

    bool FindInAllFiles( Searchable::Direction direction );

    HTMLResource* GetNextContainingHTMLResource( Searchable::Direction direction );

    HTMLResource* GetNextHTMLResource( HTMLResource *current_resource, Searchable::Direction direction );

    Resource* GetCurrentResource();

    template< class T >
    bool ResourceContainsCurrentRegex( T *resource );

    /**
     * Returns a list of all the strings
     * currently stored in the find combo box.
     *
     * @return The stored find strings.
     */
    QStringList GetPreviousFindStrings();

    /**
     * Returns a list of all the strings
     * currently stored in the replace combo box.
     *
     * @return The stored replace strings.
     */
    QStringList GetPreviousReplaceStrings();

    /**
     * Updates the find combo box with the
     * currently typed-in string.
     */
    void UpdatePreviousFindStrings();

    /**
     * Updates the replace combo box with the
     * currently typed-in string.
     */
    void UpdatePreviousReplaceStrings();

    LookWhere GetLookWhere();
    SearchMode GetSearchMode();
    SearchDirection GetSearchDirection();

    // Checks if Find is empty when not checking spelling
    bool IsValidFindText();

    // Reads all the stored dialog settings like
    void ReadSettings();
    void ReadUIMode();

    // Writes all the stored dialog settings like
    void WriteSettings();
    void WriteUIMode();

    void ExtendUI();

    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectSignalsToSlots();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // A const reference to the mainwindow that
    // spawned this widget. Needed for searching.
    MainWindow &m_MainWindow;

    FR_Capabilities m_capabilities;
    QTimer m_timer;

    Ui::FindReplace ui;

};


template< class T >
bool FindReplace::ResourceContainsCurrentRegex( T *resource )
{
    // For now, this must hold
    Q_ASSERT( GetLookWhere() == FindReplace::LookWhere_AllHTMLFiles || GetLookWhere() == FindReplace::LookWhere_SelectedHTMLFiles );

    Resource *generic_resource = resource;

    return SearchOperations::CountInFiles(
            GetSearchRegex(),
            QList< Resource* >() << generic_resource,
            SearchOperations::CodeViewSearch,
            GetSearchMode() == FindReplace::SearchMode_SpellCheck ) > 0;
}

#endif // FINDREPLACE_H
