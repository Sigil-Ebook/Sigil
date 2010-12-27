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

/**
 * A tab widget used for displaying XHTML chapters.
 * It can display the chapter in both rendered view (Book View)
 * and raw code view (Code View).
 */
class FlowTab : public ContentTab
{
    Q_OBJECT

public:

    /**
     * Constructor.
     * 
     * @param resource The resource this tab will be displaying.
     * @param fragment The URL fragment ID to which the tab should scroll.
     * @param view_state In which View should the resource open or switch to.
     * @param line_to_scroll_to To which line should the resource scroll.
     * @param parent The parent of this QObject.
     */
    FlowTab( Resource& resource, 
             const QUrl &fragment, 
             ContentTab::ViewState view_state, 
             int line_to_scroll_to = -1,
             QWidget *parent = 0 );

    // Overrides inherited from ContentTab

    int GetReadingOrder();

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

    void SetViewState( ViewState new_view_state );

    bool IsLoadingFinished();

    /**
     * Scrolls the tab to the specified fragment (if in Book View).
     *
     * @param fragment The URL fragment ID to which the tab should scroll.
     */
    void ScrollToFragment( const QString &fragment );

    /**
     * Scrolls the tab to the specified line (if in Code View).
     *
     * @param line The line to scroll to.
     */
    void ScrollToLine( int line );

    /**
     * Scrolls the tab to the top.
     */
    void ScrollToTop();    

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
     * Implements Copy action functionality.
     */
    void Copy();

    /**
     * Implements Paste action functionality.
     */
    void Paste();

    /**
     * Implements Bold action functionality.
     */
    void Bold();

    /**
     * Implements Italic action functionality.
     */
    void Italic();

    /**
     * Implements Underline action functionality.
     */
    void Underline();

    /**
     * Implements Strikethrough action functionality.
     */
    void Strikethrough();

    /**
     * Implements Align Left action functionality.
     */
    void AlignLeft();

    /**
     * Implements Center action functionality.
     */
    void Center();

    /**
     * Implements Align Right action functionality.
     */
    void AlignRight();

    /**
     * Implements Justify action functionality.
     */
    void Justify();

    /**
     * Implements Split chapter action functionality.
     */
    void SplitChapter();

    /**
     * Implements Insert SGF chapter marker action functionality.
     */
    void InsertSGFChapterMarker();

    /**
     * Implements Split on SGF chapter markers action functionality.
     */
    void SplitOnSGFChapterMarkers();

    /**
     * Implements \em a \em part of Insert image action functionality. 
     * The rest is in MainWindow. It has to be, FlowTabs don't
     * have a reference to the Book object.
     *
     * @param image_path The full path to the image that should be inserted.
     */
    void InsertImage( const QString &image_path );

    /**
     * Implements Insert bulleted list action functionality.
     */
    void InsertBulletedList();

    /**
     * Implements Insert numbered list action functionality.
     */
    void InsertNumberedList();

    /**
     * Implements Decrease indent action functionality.
     */
    void DecreaseIndent();

    /**
     * Implements Increase indent action functionality.
     */
    void IncreaseIndent();

    /**
     * Implements Remove Formatting action functionality.
     */
    void RemoveFormatting();

    /**
     * Implements the heading combo box functionality.
     */
    void HeadingStyle( const QString& heading_type );

    /**
     * Implements Print Preview action functionality.
     */
    void PrintPreview();

    /**
     * Implements Print action functionality.
     */
    void Print();

    /**
     * Implements Book View action functionality.
     */
    void BookView();

    /**
     * Implements Split View action functionality.
     */
    void SplitView();

    /**
     *  Implements Code View action functionality.
     */
    void CodeView();
    
    // inherited 

    void SaveTabContent();

    void LoadTabContent();

signals:

    /**
     * Emitted when the tab enters the Book View.
     */
    void EnteringBookView();

    /**
     * Emitted when the tab enters the Code View.
     */
    void EnteringCodeView();

    /**
     * Emitted when the View changes.
     */
    void ViewChanged();    

    /**
     * Emitted when the selection in the view has changed.
     */
    void SelectionChanged();

    /**
     * Emitted when a linked is clicked in the Book View.
     *
     * @param url The URL of the clicked link.
     */
    void LinkClicked( const QUrl &url );

    /**
     * Emitted when an "old" tab should be created.
     * Emitted as part of the chapter break operation.
     *
     * @param content The content of the "old" tab/resource.
     * @param originating_resource  The original resource from which the content
     *                              was extracted to create the "old" tab/resource.
     */
    void OldTabRequest( QString content, HTMLResource& originating_resource );

    /**
     * Emitted when the user wants to create several new chapters.
     *
     * @param chapters The content of the new chapters to be created.
     */
    void NewChaptersRequest( QStringList chapters );

private slots:

    /**
     * A global focus switch filter used to catch tab enter and
     * tab leave events.
     * 
     * @param old_widget The widget that just lost focus.
     * @param new_widget The widget that just gained focus. 
     * @note Since we use two child widgets (the View Editors) that cover 
     *       the whole tab, we cannot just reimplement focusIn and focusOut
     *       event handlers (they won't get called). So we use this function
     *       to implement our own focusIn and focusOut handling. 
     */
    void TabFocusChange( QWidget *old_widget, QWidget *new_widget );

    /**
     * A global focus switch filter used to catch focus switches
     * inside the Split View.
     * 
     * @param old_widget The widget that just lost focus.
     * @param new_widget The widget that just gained focus. 
     */
    void SplitViewFocusSwitch( QWidget *old_widget, QWidget *new_widget );

    /**
     * Performs the delayed initialization of the tab.
     * We perform delayed initialization after the widget is on
     * the screen. This way, the user perceives less load time.
     */
    void DelayedInitialization();

    /**
     * Emits the ContentChanged signal.
     */
    void EmitContentChanged();  

private:

    /**
     * Makes the Book View the current View.
     */
    void EnterBookView();

    /**
     * Makes the Code View the current View.
     */
    void EnterCodeView();

    /**
     * Reads all the stored application settings like
     * window position, geometry etc.
     */
    void ReadSettings();
    
    /**
     * Writes all the stored application settings like
     * window position, geometry etc.
     */
    void WriteSettings();

    /**
     * Returns the active View Editor.
     *
     * @return The active View Editor.
     */
    ViewEditor& GetActiveViewEditor() const;

    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectSignalsToSlots();

    /**
     * Connects all the required signals to their respective slots,
     * after some initial setup work has been done.
     */
    void DelayedConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * The fragment to scroll to after the tab is initialized.
     */
    const QUrl m_FragmentToScroll;

    /**
     * The line to scroll to after the tab is initialized.
     */
    int m_LineToScrollTo;

    /**
     * The HTML resource the tab is currently displaying.
     */
    HTMLResource &m_HTMLResource;

    /**
     * The splitter widget that separates the two Views.
     */
    QSplitter &m_Splitter;

    /**
     * The Book View Editor.
     * Displays and edits the rendered state of the HTML.
     */
    BookViewEditor &m_wBookView;

    /**
     * The Code View Editor.
     * Displays and edits the raw code.
     */ 
    CodeViewEditor &m_wCodeView;

    /**
     * Specifies which view was used last.
     * \c True if the last view the user edited in was Book View. 
     */
    bool m_IsLastViewBook; 

    /**
     * \c True when the user is using the Split View.
     *
     * @note We need this variable because for some reason, 
     *       checking for isVisible on both views doesn't work.
     */
    bool m_InSplitView;

    ContentTab::ViewState m_StartingViewState;
};

#endif // FLOWTAB_H


