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

#include "ui_FindReplace.h"
#include "ViewEditors/Searchable.h"
#include "Misc/SearchOperations.h"
#include "MainUI/MainWindow.h"
#include "BookManipulation/FolderKeeper.h"

class QRegExp;
class Resource;
class HTMLResource;

class FindReplace : public QDockWidget
{
    Q_OBJECT

public:

    FindReplace( MainWindow &main_window );
    ~FindReplace();

    /**
     * Sets up the default Find text during dialog creation.
     */
    void SetUpFindText();

    /**
     * Defines possible areas where the search can be performed.
     */
    enum LookWhere
    {
        LookWhere_CurrentFile,
        LookWhere_AllHTMLFiles
    };

    enum SearchMode
    {
        SearchMode_Normal,
        SearchMode_Wildcard,
        SearchMode_Regex
    };

private slots:

    void AdjustSize( bool toplevel );

    // Shows a message in the main window.
    void ShowMessage( const QString &message );

    void FindNext();

    void FindPrevious();

    // Counts the number of occurrences of the user's
    // term in the document.
    void Count();

    // Replaces the user's search term with the user's
    // replacement text if a match is selected. If it's not,
    // calls FindNext() so it becomes selected.
    void ReplaceNext();

    void ReplacePrevious();

    // Replaces the user's search term with the user's
    // replacement text in the entire document. Shows a
    // dialog telling how many occurrences were replaced.
    void ReplaceAll();

    void ShowAdvancedOptions();

private:

    void FindText( Searchable::Direction direction );
    void ReplaceText( Searchable::Direction direction );

    /**
     * Checks if book-wide searching is allowed for the current view.
     *
     * @return \c true if book-wide searching is allowed.
     */
    bool CheckBookWideSearchingAllowed();

    // Displays a message to the user informing him
    // that his last search term could not be found.
    void CannotFindSearchTerm();

    Searchable* GetAvailableSearchable();

    // Constructs a searching regex from the selected
    // options and fields and then returns it.
    QRegExp GetSearchRegex();

    int CountInFiles();

    int ReplaceInAllFiles();

    void FindInAllFiles( Searchable *searchable );

    HTMLResource* GetNextContainingHTMLResource();

    HTMLResource* GetNextHTMLResource( HTMLResource *current_resource );

    template< class T >
    T* GetStartingResource();

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

    // Reads all the stored dialog settings like
    // window position, geometry etc.
    void ReadSettings();

    // Writes all the stored dialog settings like
    // window position, geometry etc.
    void WriteSettings();

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

    LookWhere m_LookWhere;
    SearchMode m_SearchMode;
    bool m_MatchWholeWord;
    bool m_MatchCase;
    bool m_MatchMinimal;

    Ui::FindReplace ui;
};


template< class T >
bool FindReplace::ResourceContainsCurrentRegex( T *resource )
{
    // For now, this must hold
    Q_ASSERT( m_LookWhere == LookWhere_AllHTMLFiles );

    Resource *generic_resource = resource;

    return SearchOperations::CountInFiles(
            GetSearchRegex(),
            QList< Resource* >() << generic_resource,
            SearchOperations::CodeViewSearch ) > 0;
}


template< class T >
T* FindReplace::GetStartingResource()
{
    Resource* resource = GetCurrentResource();
    T* typed_resource  = qobject_cast< T* >( resource );

    if ( typed_resource )

        return typed_resource;

    // If the current one is not the correct type,
    // we return the first resource of that type from the book.
    QList< T* > resources = m_MainWindow.GetCurrentBook()->GetFolderKeeper().GetResourceTypeList< T >( true );

    if ( resources.count() > 0 )

        return resources[ 0 ];

    else

        return NULL;
}

#endif // FINDREPLACE_H
