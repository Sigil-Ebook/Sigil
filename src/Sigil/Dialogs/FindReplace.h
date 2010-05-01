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
#ifndef FINDREPLACE_H
#define FINDREPLACE_H

#include <QtGui/QDialog>
#include "ui_FindReplace.h"
#include "../ViewEditors/Searchable.h"

class MainWindow;
class QRegExp;

class FindReplace : public QDialog
{
    Q_OBJECT

public:

    // Constructor;
    // the first argument specifies which tab to load first;
    // the second argument is the MainWindow that created the dialog;
    // the third argument is the widget's parent.
    FindReplace( bool find_tab, MainWindow &main_window, QWidget *parent = 0 );

    // Destructor
    ~FindReplace();

private slots:

    // Switches the display between the "more" version with 
    // the option controls and the "less" version without them
    void ToggleMoreLess();

    // Gets called whenever the user switches tabs,
    // so it moves all the controls to the other tab.
    void TabChanged();

    // Starts the search for the user's term.
    // Shows a dialog if the term cannot be found.
    void FindNext();

    // Counts the number of occurrences of the user's
    // term in the document. Shows a dialog with the number.
    void Count();

    // Replaces the user's search term with the user's
    // replacement text if a match is selected. If it's not,
    // calls FindNext() so it becomes selected.
    void Replace();

    // Replaces the user's search term with the user's
    // replacement text in the entire document. Shows a
    // dialog telling how many occurrences were replaced.
    void ReplaceAll();

    // Toggles the availability of options depending on
    // whether the normal search type is selected.
    void ToggleAvailableOptions( bool normal_search_checked );

    /**
     * Handles changes to the cbLookWhere combo box.
     * Connected to the activated() signal of cbLookWhere.
     *
     * @param text The newly selected text in the combo box.
     */
    void LookWhereChanged( int index );  

private:

    /**
     * Defines possible areas where the search can be performed.
     */
    enum LookWhere 
    {
        CurrentFile,
        AllHTMLFiles
    };

    // Displays a message to the user informing him
    // that his last search term could not be found.
    void CannotFindSearchTerm();

    Searchable* GetAvailableSearchable();

    // Constructs a searching regex from the selected 
    // options and fields and then returns it.
    QRegExp GetSearchRegex();

    // Returns the selected search direction.
    Searchable::Direction GetSearchDirection();

    LookWhere CurrentLookWhere();

    int CountInFiles();

    // Changes the layout of the controls to the Find tab style
    void ToFindTab();

    // Changes the layout of the controls to the Replace tab style
    void ToReplaceTab();

    // Reads all the stored dialog settings like
    // window position, geometry etc.
    void ReadSettings();

    // Writes all the stored dialog settings like
    // window position, geometry etc.
    void WriteSettings();

    // Qt Designer is not able to create all the widgets
    // we want in our dialog, so we use this function
    // to extend the UI created by the Designer
    void ExtendUI();

    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // Stores the current display 
    // state of the dialog
    bool m_isMore;

    // A const reference to the mainwindow that
    // spawned this dialog. Needed for searching.
    MainWindow &m_MainWindow;

    // Holds all the widgets Qt Designer created for us
    Ui::FindReplace ui;
};

#endif // FINDREPLACE_H


