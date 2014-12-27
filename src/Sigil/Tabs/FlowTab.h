/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
**  Copyright (C) 2012  Dave Heiland
**  Copyright (C) 2012  Grant Drake
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
#ifndef FLOWTAB_H
#define FLOWTAB_H

#include <QtCore/QUrl>

#include "MainUI/MainWindow.h"
#include "Misc/Utility.h"
#include "MiscEditors/ClipEditorModel.h"
#include "MiscEditors/IndexEditorModel.h"
#include "Tabs/ContentTab.h"
#include "Tabs/WellFormedContent.h"

class QStackedWidget;
class QUrl;
class BookViewEditor;
class CodeViewEditor;
class HTMLResource;
class Resource;
class ViewEditor;
class WellFormedCheckComponent;

/**
 * A tab widget used for displaying XHTML section.
 * It can display the section in both rendered view (Book View)
 * and raw code view (Code View).
 */
class FlowTab : public ContentTab, public WellFormedContent
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
    FlowTab(HTMLResource &resource,
            const QUrl &fragment,
            MainWindow::ViewState view_state,
            int line_to_scroll_to = -1,
            int position_to_scroll_to = -1,
            QString caret_location_to_scroll_to = QString(),
            bool grab_focus = true,
            QWidget *parent = 0);

    ~FlowTab();

    // Overrides inherited from ContentTabs

    MainWindow::ViewState GetViewState();

    bool IsModified();

    bool CutEnabled();
    bool CopyEnabled();
    bool PasteEnabled();

    bool DeleteLineEnabled();

    bool RemoveFormattingEnabled();

    bool InsertClosingTagEnabled();

    bool GoToLinkOrStyleEnabled();

    bool AddToIndexEnabled();
    bool MarkForIndexEnabled();

    bool InsertIdEnabled();
    bool InsertHyperlinkEnabled();

    bool InsertSpecialCharacterEnabled();
    bool ToggleAutoSpellcheckEnabled();

    bool InsertFileEnabled();

    bool ViewStatesEnabled();

    QList<ViewEditor::ElementIndex> GetCaretLocation();
    QString GetCaretLocationUpdate() const;
    void GoToCaretLocation(QList<ViewEditor::ElementIndex> location);

    QString GetDisplayedCharacters();
    QString GetText();

    int GetCursorPosition() const;
    int GetCursorLine() const;
    int GetCursorColumn() const;

    float GetZoomFactor() const;

    void SetZoomFactor(float new_zoom_factor);

    void UpdateDisplay();

    Searchable *GetSearchableContent();

    bool SetViewState(MainWindow::ViewState new_view_state);

    bool IsLoadingFinished();

    /**
     * Scrolls the tab to the specified fragment (if in Book View).
     *
     * @param fragment The URL fragment ID to which the tab should scroll.
     */
    void ScrollToFragment(const QString &fragment);

    /**
     * Scrolls the tab to the specified line (if in Code View).
     *
     * @param line The line to scroll to.
     */
    void ScrollToLine(int line);
    void ScrollToPosition(int cursor_position);
    void ScrollToCaretLocation(QString caret_location_update);

    /**
     * Scrolls the tab to the top.
     */
    void ScrollToTop();

    // Overrides inherited from WellFormedContent

    void AutoFixWellFormedErrors();

    void TakeControlOfUI();

    QString GetFilename();

    bool BoldChecked();
    bool ItalicChecked();
    bool UnderlineChecked();
    bool StrikethroughChecked();
    bool SubscriptChecked();
    bool SuperscriptChecked();
    bool AlignLeftChecked();
    bool AlignRightChecked();
    bool AlignCenterChecked();
    bool AlignJustifyChecked();
    bool BulletListChecked();
    bool NumberListChecked();

    bool PasteClipNumber(int clip_number);
    bool PasteClipEntries(QList<ClipEditorModel::clipEntry *>clips);

    QString GetCaretElementName();

    void SuspendTabReloading();
    void ResumeTabReloading();

public slots:

    bool IsDataWellFormed();

    void Undo();
    void Redo();
    void Cut();
    void Copy();
    void Paste();

    void DeleteLine();

    bool MarkSelection();
    bool ClearMarkedText();

    void SplitSection();

    void InsertSGFSectionMarker();

    void InsertClosingTag();

    void InsertFile(QString html);

    void PrintPreview();
    void Print();

    /**
     * Qt has some nasty inconsistencies on when to focus is fired. In the situation
     * where we are switching tabs or switching views on a tab we want to ensure
     * that we consistently force a reload of the tab if it is pending.
     */
    void ReloadTabIfPending();

    // inherited
    void SaveTabContent();
    void LoadTabContent();

    void Bold();
    void Italic();
    void Underline();
    void Strikethrough();
    void Subscript();
    void Superscript();

    void AlignLeft();
    void AlignCenter();
    void AlignRight();
    void AlignJustify();

    void InsertBulletedList();
    void InsertNumberedList();

    void DecreaseIndent();
    void IncreaseIndent();

    void TextDirectionLeftToRight();
    void TextDirectionRightToLeft();
    void TextDirectionDefault();

    void ShowTag();
    void RemoveFormatting();

    void ChangeCasing(const Utility::Casing casing);

    void HeadingStyle(const QString &heading_type, bool preserve_attributes);

    void AddToIndex();
    bool MarkForIndex(const QString &title);

    QString GetAttributeId();
    QString GetAttributeHref();
    QString GetAttributeIndexTitle();

    QString GetSelectedText();
    bool InsertId(const QString &id);
    bool InsertHyperlink(const QString &url);

    void GoToLinkOrStyle();

    void AddMisspelledWord();
    void IgnoreMisspelledWord();

    void HighlightWord(QString word, int pos);
    void RefreshSpellingHighlighting();

signals:

    void SelectionChanged();

    /**
     * Emitted when a linked is clicked in the Book View.
     *
     * @param url The URL of the clicked link.
     */
    void LinkClicked(const QUrl &url);

    void ViewImageRequest(const QUrl &url);

    /**
     * Emitted when an "old" tab should be created.
     * Emitted as part of the section break operation.
     *
     * @param content The content of the "old" tab/resource.
     * @param originating_resource  The original resource from which the content
     *                              was extracted to create the "old" tab/resource.
     */
    void OldTabRequest(QString content, HTMLResource &originating_resource);

    void OpenClipEditorRequest(ClipEditorModel::clipEntry *clip);

    void OpenIndexEditorRequest(IndexEditorModel::indexEntry *index);

    void GoToLinkedStyleDefinitionRequest(const QString &element_name, const QString &style_class_name);

    void BookmarkLinkOrStyleLocationRequest();

    void InsertFileRequest();

    void UpdatePreview();
    void UpdatePreviewImmediately();

    void InspectElement();


private slots:

    /**
     * Performs the delayed initialization of the tab.
     * We perform delayed initialization after the widget is on
     * the screen. This way, the user perceives less load time.
     */
    void DelayedInitialization();

    /**
     * Any slots to do with changing of the underlying resource/content should
     * only be connected after the resource has loaded. Otherwise we do a
     * whole lot of unnecessary saving and loading.
     */
    void DelayedConnectSignalsToSlots();

    void EmitContentChanged();

    void EmitUpdatePreview();
    void EmitUpdatePreviewImmediately();

    void EmitUpdateCursorPosition();

    /**
     * Receives the signal emitted when an editor loses focus. Ensures that
     * the editor's content is well-formed and then saves it.
     *
     * @param A pointer to the editor.
     */
    void LeaveEditor(QWidget *editor);

    /**
     * Receives the signal emitted when user settings have changed.
     */
    void LoadSettings();

    // Called when the underlying resource is modified. It is only connected
    // when the view state is BV and used to know if BV should be reloaded
    // when the user enters the view. CV is linked to the resource in such a
    // way that this is unnecessary. The CV linking is not possible in BV.
    void ResourceModified();
    void LinkedResourceModified();

    // Called when the underlying text inside the control is being replaced
    // Store our caret location as required.
    void ResourceTextChanging();

private:
    void CreateBookViewIfRequired(bool is_delayed_load = true);
    void CreateCodeViewIfRequired(bool is_delayed_load = true);

    void BookView();
    void CodeView();

    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectBookViewSignalsToSlots();
    void ConnectCodeViewSignalsToSlots();


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

    int m_PositionToScrollTo;

    QString m_CaretLocationToScrollTo;

    /**
     * The HTML resource the tab is currently displaying.
     */
    HTMLResource &m_HTMLResource;

    /**
     * The splitter widget that separates the two Views.
     */
    QStackedWidget *m_views;

    /**
     * The Book View Editor.
     * Displays and edits the rendered state of the HTML.
     */
    BookViewEditor *m_wBookView;

    /**
     * The Code View Editor.
     * Displays and edits the raw code.
     */
    CodeViewEditor *m_wCodeView;

    /**
     * This is used in a few different ways.
     *
     * 1) We store the requested view state for loading the document.
     * 2) We store the current view state.
     * 3) We compare the state of the view when entering to this in order
     *    to determine if we have changed the view (BV, CV) in order to
     *    load the latest content into the view.
     */
    MainWindow::ViewState m_ViewState;
    MainWindow::ViewState m_previousViewState;

    /**
     * The component used to display a dialog about
     * well-formedness errors.
     */
    WellFormedCheckComponent &m_WellFormedCheckComponent;

    /**
     * A flag to be used in conjunction with the check for well-formedness which
     * indicates whether it's safe to reload the tab content.
     */
    bool m_safeToLoad;

    bool m_initialLoad;

    bool m_bookViewNeedsReload;

    bool m_grabFocus;

    bool m_suspendTabReloading;

    bool m_defaultCaretLocationToTop;
};

#endif // FLOWTAB_H


