/************************************************************************
**
**  Copyright (C) 2011, 2012  John Schember <john@nachtimwald.com>
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
#ifndef FINDREPLACE_H
#define FINDREPLACE_H

#include <QtCore/QTimer>

#include "ui_FindReplace.h"
#include "Misc/FindFields.h"
#include "BookManipulation/FolderKeeper.h"
#include "MainUI/MainWindow.h"
#include "Misc/SearchOperations.h"
#include "MiscEditors/SearchEditorModel.h"
#include "ViewEditors/Searchable.h"

class HTMLResource;
class Resource;
class MainWindow;

class FindReplace : public QWidget
{
    Q_OBJECT

public:
    FindReplace( MainWindow &main_window );
    ~FindReplace();

    /**
     * Sets up the default Find text during dialog creation.
     */
    void SetUpFindText();

    void WriteSettingsVisible(bool visible);

    void ShowHide();
    
    void SetRegexOptionDotAll(bool new_state);
    void SetRegexOptionMinimalMatch(bool new_state);
    void SetRegexOptionAutoTokenise(bool new_state);

public slots:
    void close();
    void show();

    void LoadSearchByName(QString name);

    void LoadSearch(SearchEditorModel::searchEntry *search_entry);
    void FindSearch(QList<SearchEditorModel::searchEntry *> search_entries);
    void ReplaceSearch(QList<SearchEditorModel::searchEntry *> search_entries);
    void CountAllSearch(QList<SearchEditorModel::searchEntry *> search_entries);
    void ReplaceAllSearch(QList<SearchEditorModel::searchEntry *>search_entries);

    bool FindMisspelledWord();

signals:

    void OpenSearchEditorRequest(SearchEditorModel::searchEntry *search_entry = NULL);

    void ShowMessageRequest(QString message);

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:

    // Shows a message in the main window.
    void ShowMessage( const QString &message );

    void FindClicked();
    void CountClicked();
    void ReplaceClicked();
    void ReplaceAllClicked();

    // Uses the find direction to determine if we should find next
    // or previous.
    bool Find();

    bool FindNext();

    bool FindPrevious();

    // Counts the number of occurrences of the user's
    // term in the document.
    int Count();

    // Uses the find direction to determine if we should replace next
    // or previous.
    bool Replace();

    // Replaces the user's search term with the user's
    // replacement text if a match is selected. If it's not,
    // calls FindNext() so it becomes selected.
    bool ReplaceNext();

    bool ReplacePrevious();

    // Replaces the user's search term with the user's
    // replacement text in the entire document. Shows a
    // dialog telling how many occurrences were replaced.
    int ReplaceAll();

    void clearMessage();

    void expireMessage();

    void SaveSearchAction();

    void TokeniseSelection();

    void HideFindReplace();

private:
    bool FindText( Searchable::Direction direction );
    bool ReplaceText( Searchable::Direction direction );

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

    void SetLookWhereFromModifier();

    void ResetLookWhereFromModifier();

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
    void UpdatePreviousFindStrings(const QString &text = QString());

    /**
     * Updates the replace combo box with the
     * currently typed-in string.
     */
    void UpdatePreviousReplaceStrings(const QString &text = QString());

    FindFields::LookWhere GetLookWhere();
    FindFields::SearchMode GetSearchMode();
    FindFields::SearchDirection GetSearchDirection();

    // Checks if Find is empty when not checking spelling
    bool IsValidFindText();

    // Reads all the stored dialog settings
    void ReadSettings();

    // Writes all the stored dialog settings
    void WriteSettings();

    void ExtendUI();

    /* 
     * Tokenisation helper functions for automating replacement
     * of elements of Find text in regular expressions
     */
    QString TokeniseSpacesForRegex(const QString &text);
	QString TokeniseNumericsForRegex(const QString &text);

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

    QTimer m_timer;

    Ui::FindReplace ui;

    bool m_RegexOptionDotAll;
    bool m_RegexOptionMinimalMatch;
    bool m_RegexOptionAutoTokenise;

    bool m_SpellCheck;

    bool m_LookWhereCurrentFile;
};


template< class T >
bool FindReplace::ResourceContainsCurrentRegex( T *resource )
{
    // For now, this must hold
    Q_ASSERT( GetLookWhere() == FindFields::LookWhere_AllHTMLFiles || GetLookWhere() == FindFields::LookWhere_SelectedHTMLFiles );

    Resource *generic_resource = resource;

    return SearchOperations::CountInFiles(
            GetSearchRegex(),
            QList< Resource* >() << generic_resource,
            SearchOperations::CodeViewSearch,
            m_SpellCheck ) > 0;
}

#endif // FINDREPLACE_H
