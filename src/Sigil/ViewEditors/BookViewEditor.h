/************************************************************************
**
**  Copyright (C) 2012 John Schember <john@nachtimwald.com>
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
#ifndef BOOKVIEWEDITOR_H
#define BOOKVIEWEDITOR_H

#include <QtCore/QVariant>
#include <QtWebKit/QWebElement>

#include "Misc/PasteTarget.h"
#include "Misc/Utility.h"
#include "MiscEditors/ClipEditorModel.h"
#include "MiscEditors/IndexEditorModel.h"
#include "ViewEditors/BookViewPreview.h"
#include "ViewEditors/ViewEditor.h"

class QAction;
class QEvent;
class QMenu;
class QSize;
class QShortcut;
class QSignalMapper;

/**
 * A WYSIWYG editor for XHTML flows.
 * Also called the "Book View", because it shows a
 * section of a book in its final, rendered state
 * (the way it will look like in epub Reading Systems).
 */
class BookViewEditor : public BookViewPreview, public PasteTarget
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param parent The object's parent.
     */
    BookViewEditor(QWidget *parent = 0);

    /**
     * Destructor.
     */
    ~BookViewEditor();

    /**
     * Sets a custom webpage for the editor.
     */
    void CustomSetDocument(const QString &path, const QString &html);

    QString GetHtml();
    //QString GetXHtml11();
    //QString GetHtml5();

    bool InsertHtml(const QString &html);

    /**
     * Splits the section and returns the "upper" content.
     * The current flow is split at the caret point.
     *
     * @return The content of the section up to the section break point.
     *
     * @note What we actually do when the user wants to split the loaded section
     * is create a new tab with the XHTML content \em above the split point.
     * The new tab is actually the "old" section, and this tab becomes the
     * "new" section.
     * \par
     * Why? Because we can only avoid a tab render in the tab from which
     * we remove elements. Since the users move from the top of a large HTML
     * file down, the new section will be the one with the most content.
     * So this way we \em try to avoid the painful render time on the biggest
     * section, but there is still some render time left...
     */
    QString SplitSection();

    bool IsModified();
    void ResetModified();

    void SetZoomFactor(float factor);

    // Even though the BookViewPreview implements these they are pure virtual
    // in ViewEditor so they have to be implemented here.
    void SetCurrentZoomFactor(float factor) {
        BookViewPreview::SetCurrentZoomFactor(factor);
    }
    float GetZoomFactor() const {
        return BookViewPreview::GetZoomFactor();
    }

    bool IsLoadingFinished() {
        return BookViewPreview::IsLoadingFinished();
    }

    bool FindNext(const QString &search_regex,
                  Searchable::Direction search_direction,
                  bool check_spelling = false,
                  bool ignore_selection_offset = false,
                  bool wrap = true,
                  bool selected_text = false) {
        return BookViewPreview::FindNext(search_regex, search_direction, check_spelling, ignore_selection_offset, wrap, selected_text);
    }

    int Count(const QString &search_regex, Searchable::Direction direction, bool wrap, bool selected_text = false) {
        return BookViewPreview::Count(search_regex, direction, wrap, selected_text);
    }

    bool ReplaceSelected(const QString &search_regex, const QString &replacement, Searchable::Direction direction = Searchable::Direction_Down, bool keep_selection = false) {
        return BookViewPreview::ReplaceSelected(search_regex, replacement, direction, keep_selection);
    }

    int ReplaceAll(const QString &search_regex, const QString &replacement,                                Searchable::Direction direction, bool wrap, bool selected_text) {
        return BookViewPreview::ReplaceAll(search_regex, replacement, direction, wrap, selected_text);
    }

    QString GetSelectedText();

    void SetUpFindForSelectedText(const QString &search_regex);

    /**
     * Executes a contentEditable command.
     * The command is executed through JavaScript.
     *
     * @param command The command to execute.
     */
    bool ExecCommand(const QString &command);

    /**
     * Executes a contentEditable command.
     * The command is executed through JavaScript.
     *
     * @param command The command to execute.
     * @param parameter The parameter that should be passed to the command.
     */
    bool ExecCommand(const QString &command, const QString &parameter);

    /**
     * Returns the state of the contentEditable command.
     * The query is performed through JavaScript.
     */
    bool QueryCommandState(const QString &command);

    /**
     * Implements the "formatBlock" execCommand because
     * WebKit's default one has bugs.
     * It takes an element name as an argument (e.g. "p"),
     * and replaces the element the cursor is located in with it.
     *
     * @param element_name The name of the element to format the block to.
     * @param preserve_attributes Whether to keep any existing attributes on the previous block tag.
     */
    void FormatBlock(const QString &element_name, bool preserve_attributes);

    /**
     * Returns the name of the element the caret is located in.
     * If text is selected, returns the name of the element
     * where the selection \em starts.
     *
     * @return The name of the caret element.
     */
    QString GetCaretElementName();

    void ApplyCaseChangeToSelection(const Utility::Casing &casing);

    bool InsertId(const QString &id);
    bool InsertHyperlink(const QString &href);

    void AddToIndex();
    bool MarkForIndex(const QString &title);

    /**
      * From the current cursor position, search for a parent tag element named in the tag list.
      * When first if any matching tag found, return the value of the named attribute if exists.
      */
    QString GetAncestorTagAttributeValue(const QString &attribute_name, const QStringList &tag_list);

    bool PasteClipNumber(int clip_number);

public slots:
    /**
     * Filters the text changed signals by the BVEditor inside of the page
     * and then takes the appropriate action.
     */
    void TextChangedFilter();

    void Undo();
    void Redo();

    void cut();
    void paste();
    void selectAll();

    void insertFile();
    void openImage();
    void copyImage();

    void openWith();
    void openWithEditor();

    void saveAs();

    void EmitInspectElement();

    // Implementations for PasteTarget.h
    void PasteText(const QString &text);
    bool PasteClipEntries(const QList<ClipEditorModel::clipEntry *> &clips);

signals:
    void PageUpdated();
    void PageClicked();
    void PageOpened();

    /**
     * Emitted when the text changes.
     * The contentsChanged QWebPage signal is wired to this one,
     * and contentsChangedExtra is wired to contentsChanged.
     */
    void textChanged();

    /**
     * Extends the QWebPage contentsChanged signal.
     * Use textChanged to know when the BookView has been modified.
     *
     * The QWebPage contentsChanged signal is not emitted on every
     * occasion we want it to, so we emit this when necessary.
     * This signal is in turn wired to contentsChanged. Why?
     * Because we want others connected to our QWebPage but not to
     * the Book View textChanged signal to be aware of these changes.
     * Thus, the wired extension.
     */
    void contentsChangedExtra();

    /**
     * Emitted when the focus is lost.
     */
    void FocusLost(QWidget *editor);

    void InsertFile();

    void InsertedFileOpenedExternally(const QString &pathname);

    void InsertedFileSaveAs(const QUrl &url);

    /**
     * Emitted when we want to do some operations with the clipboard
     * to paste things, but restoring state afterwards so that the
     * Clipboard History and current clipboard is left unaffected.
     */
    void ClipboardSaveRequest();
    void ClipboardRestoreRequest();

    void OpenClipEditorRequest(ClipEditorModel::clipEntry *);

    void OpenIndexEditorRequest(IndexEditorModel::indexEntry *);

    void BVInspectElement();

protected:
    /**
     * Handles the focus out event for the editor.
     *
     * @param event The event to process.
     */
    void focusOutEvent(QFocusEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

private slots:

    void EmitPageUpdated();

    /**
     * Wrapper slot for the Page Up shortcut.
     */
    void PageUp();

    /**
     * Wrapper slot for the Page Down shortcut.
     */
    void PageDown();

    void ClickAtTopLeft();

    /**
     * Wrapper slot for the Scroll One Line Up shortcut.
     */
    void ScrollOneLineUp();

    /**
     * Wrapper slot for the Scroll One Line Down shortcut.
     */
    void ScrollOneLineDown();

    /**
     * Sets the web page modified state.
     *
     * @param modified The new modified state.
     */
    void SetWebPageModified(bool modified = true);

    /**
     * Opens the context menu at the requested point.
     *
     * @param point The point at which the menu should be opened.
     */
    void OpenContextMenu(const QPoint &point);

    void PasteClipEntryFromName(const QString &name);

    void SaveClipAction();

private:
    bool PasteClipEntry(ClipEditorModel::clipEntry *clip);

    void AddClipContextMenu(QMenu *menu);
    bool CreateMenuEntries(QMenu *parent_menu, QAction *topAction, QStandardItem *item);

    /**
     * Escapes JavaScript string special characters.
     *
     * @return The escaped string.
     */
    QString EscapeJSString(const QString &string);

    /**
     * Scrolls the whole screen by one line.
     * Used for ScrollOneLineUp and ScrollOneLineDown shortcuts.
     *
     * @param down Specifies are we scrolling up or down.
     */
    void ScrollByLine(bool down);

    /**
     * Scrolls the whole screen a number of pixels.
     *
     * @param pixel_number The number of pixels to scroll
     * @param down Specifies are we scrolling up or down.
     */
    void ScrollByNumPixels(int pixel_number, bool down);

    /**
     * Removes all the cruft with which WebKit litters our source code.
     * The cruft is removed from the QWebPage cache, and includes
     * superfluous CSS styles and classes.
     */
    void RemoveWebkitCruft();

    /**
     * Removes the spans created by the replace mechanism in Book View.
     *
     * @param The source html from the web page.
     * @return The html cleaned of spans with 'class="SigilReplace_..."'.
     */
    QString RemoveBookViewReplaceSpans(const QString &source);

    bool InsertTagAttribute(const QString &element_name, const QString &attribute_name, const QString &attribute_value, const QStringList &tag_list, bool ignore_selection = false);

    bool SetAncestorTagAttributeValue(const QString &attribute_name, const QString &attribute_value, const QStringList &tag_list);

    /**
     * Creates all the context menu actions.
     */
    void CreateContextMenuActions();

    /**
     * Tries to setup the context menu for the specified point,
     * and returns true on success.
     *
     * @param point The point at which the menu should be opened.
     * @return \c true if the menu could be set up.
     */
    bool SuccessfullySetupContextMenu(const QPoint &point);

    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectSignalsToSlots();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * Store the last match when doing a find so we can determine if
     * found text is selected for doing a replace. We also need to store the
     * match because we can't run the selected text though the PCRE engine
     * (we don't want to because it's slower than caching) because it will fail
     * if a look ahead or behind expression is in use.
     */
    SPCRE::MatchInfo m_lastMatch;

    QVariant m_caret;
    QString m_path;

    /**
     * \c true if the WebPage was modified by the user.
     */
    bool m_WebPageModified;

    QSignalMapper *m_clipMapper;

    /**
     * The context menu actions.
     */
    QAction *m_Undo;
    QAction *m_Redo;

    QAction *m_Cut;
    QAction *m_Copy;
    QAction *m_CopyImage;
    QAction *m_Paste;
    QAction *m_SelectAll;

    QAction *m_InsertFile;
    QAction *m_Open;

    QMenu &m_OpenWithContextMenu;

    QAction *m_OpenWith;
    QAction *m_OpenWithEditor;

    QAction *m_SaveAs;
    QAction *m_InspectElement;

    /**
     * Paste keyboard shortcuts - CTRL+V (Command-V - MacOS) and SHIFT-Insert (Old - Windows).
     */
    QShortcut &m_Paste1;
    QShortcut &m_Paste2;

    /**
     * PageUp keyboard shortcut.
     */
    QShortcut &m_PageUp;

    /**
     * PageDown keyboard shortcut.
     */
    QShortcut &m_PageDown;

    /**
     * Keyboard shortcut for scrolling one line up.
     */
    QShortcut &m_ScrollOneLineUp;

    /**
     * Keyboard shortcut for scrolling one line down.
     */
    QShortcut &m_ScrollOneLineDown;

    /**
     * The JavaScript source code that returns the XHTML source
     * from the caret to the top of the file. This code is also
     * removed from the current section.
     */
    const QString c_GetSegmentHTML;

    /**
     * Javascript source that implements a function to format the
     * first block-level parent of a node in the source.
     */
    const QString c_FormatBlock;

    /**
     * Javascript source that implements a function to find the
     * ancestor node that has the matching element node name.
     */
    const QString c_GetAncestor;

    /**
     * Javascript source that implements a function to find the
     * ancestor node that has the matching element node name
     * and return the named attribute value if any.
     */
    const QString c_GetAncestorAttribute;

    /**
     * Javascript source that implements a function to find the
     * ancestor node that has the matching element node name
     * and set the named attribute value.
     */
    const QString c_SetAncestorAttribute;
};


#endif // BOOKVIEWEDITOR_H

