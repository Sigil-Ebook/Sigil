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
#ifndef FLOWTAB_H
#define FLOWTAB_H

#include "ContentTab.h"
#include <QUrl>

class QSplitter;
class BookViewEditor;
class CodeViewEditor;
class ViewEditor;
class Resource;
class HTMLResource;
class QUrl;

class FlowTab : public ContentTab
{
    Q_OBJECT

public:

    FlowTab( Resource& resource, const QUrl &fragment, QWidget *parent = 0 );

    // Overrides inherited from ContentTab
    bool IsModified();

    bool CutEnabled();

    bool CopyEnabled();

    bool PasteEnabled();

    bool BoldChecked();

    bool ItalicChecked();

    bool UnderlineChecked();

    bool StrikethroughChecked();

    bool BulletListChecked();

    bool NumberListChecked();

    bool BookViewChecked();

    bool SplitViewChecked();

    bool CodeViewChecked();

    QString GetCaretElementName();

    float GetZoomFactor() const;

    void SetZoomFactor( float new_zoom_factor );

    Searchable* GetSearchableContent();

    ViewState GetViewState();

    void ScrollToFragment( const QString &fragment );

    void ScrollToTop();

public slots:

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

    // Implements Insert chapter break action functionality
    void InsertChapterBreak();

    // Implements *a part* of Insert image action functionality
    // The rest is in MainWindow. (it has to be, FlowTabs don't
    // have a reference to the Book object)
    void InsertImage( const QString &image_path );

    // Implements Insert bulleted list action functionality
    void InsertBulletedList();

    // Implements Insert numbered list action functionality
    void InsertNumberedList();

    // Implements Decrease indent action functionality
    void DecreaseIndent();

    // Implements Increase indent action functionality
    void IncreaseIndent();

    // Implements Remove Formatting action functionality
    void RemoveFormatting();

    // Implements the heading combo box functionality
    void HeadingStyle( const QString& heading_type );

    // Implements Print Preview action functionality
    void PrintPreview();

    // Implements Print action functionality
    void Print();

    void BookView();

    void SplitView();

    void CodeView();

signals:

    void EnteringBookView();

    void EnteringCodeView();

    void ViewChanged();    

    void SelectionChanged();

    // Emitted by Book View (wired)
    void LinkClicked( const QUrl &url );

    void OldTabRequest( QString content, HTMLResource& originating_resource );

protected slots:

    void SaveContentOnTabLeave();
    
    void LoadContentOnTabEnter();

private slots:

    // Since we use two child widgets (the View Editors) that cover 
    // the whole tab, we cannot just reimplement focusIn and focusOut
    // event handlers (they won't get called). So we use this function
    // to implement our own focusIn and focusOut handling. 
    void TabFocusChange( QWidget *old_widget, QWidget *new_widget );

    // Used to catch the focus changeover from one widget
    // (code or book view) to the other in Split View;
    // needed for source synchronization.
    void SplitViewFocusSwitch( QWidget *old_widget, QWidget *new_widget );

    void DelayedInitialization();

    void EmitContentChanged();  

private:

    void EnterBookView();

    void EnterCodeView();

    void ReadSettings();
    
    void WriteSettings();

    ViewEditor& GetActiveViewEditor() const;

    void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    const QUrl m_FragmentToScroll;

    HTMLResource &m_HTMLResource;

    QSplitter &m_Splitter;

    // The webview component that renders out HTML
    BookViewEditor &m_wBookView;

    // The plain text code editor 
    CodeViewEditor &m_wCodeView;

    // true if the last view the user edited in was book view
    bool m_IsLastViewBook; 

    // We need this variable because for some reason,
    // checking for isVisible on both views doesn't work.
    bool m_InSplitView;
};

#endif // FLOWTAB_H


