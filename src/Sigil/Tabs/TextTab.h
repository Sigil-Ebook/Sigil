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
#ifndef TEXTTAB_H
#define TEXTTAB_H

#include "ContentTab.h"
#include "ViewEditors/CodeViewEditor.h"

class TextResource;
class QTextDocument;


class TextTab : public ContentTab
{
    Q_OBJECT

public:

    TextTab( TextResource& resource,
             CodeViewEditor::HighlighterType type,
             int line_to_scroll_to = -1,
             QWidget *parent = 0 );

    void ScrollToLine( int line );

    // Overrides inherited from ContentTab
    bool IsModified();

    bool CutEnabled();

    bool CopyEnabled();

    bool PasteEnabled();

    int GetCursorLine() const;
    int GetCursorColumn() const;

    float GetZoomFactor() const;

    void SetZoomFactor( float new_zoom_factor );

    void UpdateDisplay();

    Searchable* GetSearchableContent();

    ViewState GetViewState();

public slots:

    /**
     * Implements Undo action functionality.
     */
    void Undo();

    /**
     * Implements Redo action functionality.
     */
    void Redo();

    /**
     * Implements Cut action functionality.
     */
    void Cut();

    /**
     * void Cut();
     */
    void Copy();

    /**
     * Implements Paste action functionality.
     */
    void Paste();

signals:

    void SelectionChanged();

protected slots:

    void SaveTabContent();

    void SaveTabContent( QWidget *editor );

    void LoadTabContent();

    void LoadTabContent( QWidget *editor );

private slots:

    void EmitUpdateCursorPosition();

    void DelayedInitialization();

private:

    virtual void ConnectSignalsToSlots();

protected:

    // The plain text code editor 
    CodeViewEditor &m_wCodeView;

private:

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    TextResource &m_TextResource;

    int m_LineToScrollTo;

};

#endif // TEXTTAB_H
