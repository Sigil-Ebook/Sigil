/************************************************************************
**
**  Copyright (C) 2015 Kevin B. Hendricks Stratford, ON Canada 
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
#ifndef CODEVIEWEDITOR_H
#define CODEVIEWEDITOR_H

#include <QtCore/QList>
#include <QtCore/QStack>
#include <QtWidgets/QPlainTextEdit>
#include <QtGui/QStandardItem>
#include <QtCore/QUrl>

#include "Misc/CSSInfo.h"
#include "Misc/PasteTarget.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "MiscEditors/ClipEditorModel.h"
#include "MiscEditors/IndexEditorModel.h"
#include "ViewEditors/ViewEditor.h"

class QResizeEvent;
class QSize;
class QWidget;
class QPrinter;
class QShortcut;
class LineNumberArea;
class QSyntaxHighlighter;
class QContextMenuEvent;
class QSignalMapper;
class QuickSerialHtmlParser;

/**
 * A text editor for source code.
 * Also called the "Code View" because it shows
 * the code of a section of the book. Provides syntax highlighting.
 */
class CodeViewEditor : public QPlainTextEdit, public ViewEditor, public PasteTarget
{
    Q_OBJECT

public:

    /**
     * What type of syntax highlighting to use.
     */
    enum HighlighterType {
        Highlight_NONE,  /**< No source code highlighting */
        Highlight_XHTML, /**< XHTML source code highlighting */
        Highlight_CSS    /**< CSS source code highlighting */
    };

    /**
     * Constructor.
     *
     * @param highlighter_type Which syntax highlighter to use.
     * @param parent The object's parent.
     */
    CodeViewEditor(HighlighterType highlighter_type, bool check_spelling = false, QWidget *parent = 0);
    ~CodeViewEditor();

    QSize sizeHint() const;

    /**
     * A custom implementation of QPlainTextEdit::setDocument()
     * since that doesn't do everything we want it to.
     *
     * @param document The new text document.
     */
    void CustomSetDocument(QTextDocument &document);

    void DeleteLine();

    void HighlightMarkedText();

    bool MoveToMarkedText(Searchable::Direction direction, bool wrap);

    /**
     * Routines to handle cutting code tags from selected text
     */
    void CutCodeTags();
    bool IsCutCodeTagsAllowed();

    bool TextIsSelected();
    bool TextIsSelectedAndNotInStartOrEndTag();
    bool TextIsSelectedAndNotContainingTag();

    QString StripCodeTags(QString text);

    bool IsSelectionOK();

    void InsertClosingTag();
    bool IsInsertClosingTagAllowed();

    void GoToStyleDefinition();

    void AddMisspelledWord();
    void IgnoreMisspelledWord();

    void AddToIndex();
    bool IsAddToIndexAllowed();

    bool MarkForIndex(const QString &title);

    bool IsInsertFileAllowed();

    bool InsertId(const QString &attribute_value);
    bool InsertHyperlink(const QString &attribute_value);
    bool IsInsertIdAllowed();
    bool IsInsertHyperlinkAllowed();
    bool InsertTagAttribute(const QString &element_name, const QString &attribute_name, const QString &attribute_value, const QStringList &tag_list, bool ignore_seletion = false);

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

    /**
     * Inserts the SGF section marker code at the current caret location.
     */
    void InsertSGFSectionMarker();

    /**
     * Paints the line number area.
     * Receives the event directly from the area's paintEvent() handler.
     *
     * @param event The paint event to process.
     */
    void LineNumberAreaPaintEvent(QPaintEvent *event);

    /**
     * Selects the line that was clicked on.
     * Receives the event directly from the area's mouseEvent() handler.
     *
     * @param event The mouse event to process.
     */
    void LineNumberAreaMouseEvent(QMouseEvent *event);

    /**
     * Returns the width the LinuNumberArea should take (in pixels).
     *
     * @return The width in pixels.
     */
    int CalculateLineNumberAreaWidth();

    /**
     * Replaces the text of the entire document with the new text.
     * Records the replacement as one action for the undo stack.
     *
     * @param new_text The new text of the document.
     */
    void ReplaceDocumentText(const QString &new_text);

    /**
     * Scrolls the entire view to the top.
     */
    void ScrollToTop();

    void ScrollToPosition(int cursor_position, bool center_screen = true);

    /**
     * Scrolls the view to the specified line.
     *
     * @param line The line to scroll to.
     */
    void ScrollToLine(int line);

    void ScrollToFragment(const QString &fragment);

    // inherited
    bool IsLoadingFinished();

    int GetCursorPosition() const;
    int GetCursorLine() const;
    int GetCursorColumn() const;

    void SetZoomFactor(float factor);

    float GetZoomFactor() const;

    void Zoom();

    void UpdateDisplay();

    SPCRE::MatchInfo GetMisspelledWord(const QString &text,
                                       int start_offset,
                                       int end_offset,
                                       const QString &search_regex,
                                       Searchable::Direction search_direction);

    bool FindNext(const QString &search_regex,
                  Searchable::Direction search_direction,
                  bool misspelled_words = false,
                  bool ignore_selection_offset = false,
                  bool wrap = true,
                  bool selected_text = false);

    int Count(const QString &search_regex, Searchable::Direction direction, bool wrap, bool selected_text = false);

    bool ReplaceSelected(const QString &search_regex,
                         const QString &replacement,
                         Searchable::Direction direction = Searchable::Direction_Down,
                         bool replace_current = false);

    int ReplaceAll(const QString &search_regex,
                   const QString &replacement,
                   Searchable::Direction direction,
                   bool wrap,
                   bool selected_text = false);

    QString GetSelectedText();

    void SetUpFindForSelectedText(const QString &search_regex);

    /**
     * Sets flag to execute a centerCursor() call later
     * with m_DelayedCursorScreenCenteringRequired.
     */
    void SetDelayedCursorScreenCenteringRequired();

    // inherited
    QList<ViewEditor::ElementIndex> GetCaretLocation();

    // inherited
    void StoreCaretLocationUpdate(const QList<ViewEditor::ElementIndex> &hierarchy);

    // inherited
    bool ExecuteCaretUpdate(bool default_to_top = false);

    /**
     * Find the containing block for the cursor location and apply a new
     * element tag name to it.
     *
     * @param element_name The name of the element to format the block to.
     * @param preserve_attributes Whether to keep any existing attributes on the previous block tag.
     */
    void FormatBlock(const QString &element_name, bool preserve_attributes);

    /**
     * Given the current cursor position/selection, look to toggle a format style tag
     * around it. The rule on whether to insert a new tag or remove an existing one is
     * attempting to emulate what would happen in BookView. If there is the same tag
     * immediately adjacent to the selection (inside or outside it) then it is removed.
     * Otherwise a new tag is inserted around the selection.
     *
     * @param element_name The name of the element to toggle the format of the selection.
     * @param property_name If caret is in an inline CSS style instead of the body, property to change
     * @param property_value If caret is in an inline CSS style instead of the body, value of property to change
     */
    void ToggleFormatSelection(const QString &element_name, const QString property_name = "", const QString property_value = "");

    /**
     * Based on the cursor location (in html file) add/replace as
     * appropriate a style="property_name: property_value" attribute.
     *
     * @param property_name The name of the style property to be inserted/replaced.
     * @param property_value The new value to be assigned to this property.
     */
    void FormatStyle(const QString &property_name, const QString &property_value);

    /**
     * Based on the cursor location (in CSS file or inlined in HTML file)
     * add/replace as appropriate a property_name: property_value property in
     * the currently selected CSS style if any.
     *
     * @param property_name The name of the style property to be inserted/replaced.
     * @param property_value The new value to be assigned to this property.
     */
    void FormatCSSStyle(const QString &property_name, const QString &property_value);

    void ApplyCaseChangeToSelection(const Utility::Casing &casing);

    QString GetAttributeId();

    /**
     * Based on the cursor location (in html file)
     * get appropriate attribute_value for attribute_name.
     *
     */
    QString GetAttribute(const QString &attribute_name, QStringList tag_list = QStringList(), bool must_be_in_attribute = false, bool skip_paired_tags = false);

    QString SetAttribute(const QString &attribute_name, QStringList tag_list = QStringList(), const QString &attribute_value = QString(), bool must_be_in_attribute = false, bool skip_paired_tags = false);

    /**
     * Based on the cursor location (in html file) add/replace as
     * appropriate an attribute_name="attribute_value".
     *
     * @param attribute_name The name of the attribute to be inserted/replaced.
     * @param attribute_value The new value to be assigned to this attribute.
     */
    QString ProcessAttribute(const QString &attribute_name, QStringList tag_list = QStringList(), const QString &attribute_value = QString(), bool set_attribute = false , bool must_be_in_attribute = false, bool skip_paired_tags = false);

    /**
     * Control whether the Reformat CSS submenu is available on the context menu.
     */
    bool ReformatCSSEnabled();
    void SetReformatCSSEnabled(bool value);

    /**
     * Control wheter the Reformat (clean) HTML submenu is avaliable on the context menu.
     */
    bool ReformatHTMLEnabled();
    void SetReformatHTMLEnabled(bool value);

    bool PasteClipNumber(int clip_number);

    void HighlightWord(const QString &word, int pos);

signals:

    /**
     * Emitted whenever the zoom factor changes.
     *
     * @param new_zoom_factor The new zoom factor of the View.
     */
    void ZoomFactorChanged(float new_zoom_factor);

    /**
     * Emitted when the focus is lost.
     */
    void FocusLost(QWidget *editor);

    /**
     * Emitted when the focus is gained.
     */
    void FocusGained(QWidget *editor);

    void FilteredCursorMoved();
    void PageClicked();
    void PageUpdated();

    /**
     * A filtered version of the QPlainTextEdit::textChnaged signal.
     * We use it to prevent our syntax highlighter from emitting that signal.
     */
    void FilteredTextChanged();

    void OpenClipEditorRequest(ClipEditorModel::clipEntry *);

    void OpenIndexEditorRequest(IndexEditorModel::indexEntry *);

    void LinkClicked(const QUrl &url);

    void ViewImage(const QUrl &url);

    void GoToLinkedStyleDefinitionRequest(const QString &element_name, const QString &style_class_name);

    void BookmarkLinkOrStyleLocationRequest();

    void SpellingHighlightRefreshRequest();

    void ShowStatusMessageRequest(const QString &message);

    void DocumentSet();

    void MarkSelectionRequest();
    void ClearMarkedTextRequest();

public slots:

    /**
     * A slot wrapper around the base class print() function.
     *
     * @param printer The printer interface to use for printing.
     */
    void print(QPagedPaintDevice *printer);

    // Implementations for PasteTarget.h
    void PasteText(const QString &text);
    bool PasteClipEntries(const QList<ClipEditorModel::clipEntry *> &clips);

    void RefreshSpellingHighlighting();

    void OpenImageAction();

    void GoToLinkOrStyle();

    bool MarkSelection();
    bool ClearMarkedText();

protected:

    /**
     * The global event processing function.
     *
     * @param event The event to process.
     */
    bool event(QEvent *event);

    /**
     * Handles the resize event for the editor.
     *
     * @param event The event to process.
     */
    void resizeEvent(QResizeEvent *event);

    /**
     * Handles the mouse press event for the editor.
     *
     * @param event The event to process.
     */
    void mousePressEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    /**
     * Handles the content menu event for the editor.
     *
     * @param event The event to process.
     */
    void contextMenuEvent(QContextMenuEvent *event);

    /**
     * Handles the focus in event for the editor.
     *
     * @param event The event to process.
     */
    void focusInEvent(QFocusEvent *event);

    /**
     * Handles the focus out event for the editor.
     *
     * @param event The event to process.
     */
    void focusOutEvent(QFocusEvent *event);

private slots:
    void ResetLastFindMatch();

    void EmitFilteredCursorMoved();

    /**
     * Filters the textChanged signal.
     * It does this based on the availability of undo.
     */
    void TextChangedFilter();

    void RehighlightDocument();

    void PasteClipEntryFromName(const QString &name);

    /**
     * Used solely to update the m_isUndoAvailable variable
     * on undo availability change.
     *
     * @param available The current availability of the undo action.
     */
    void UpdateUndoAvailable(bool available);

    /**
     * Creates a margin where the line number are can sit.
     * Called whenever the number of lines changes.
     */
    void UpdateLineNumberAreaMargin();

    /**
     * Repaints a part of the line number area as needed.
     *
     * @param rectangle Represents the area that the editor needs an update of.
     * @param vertical_delta The amount of pixels the viewport has been vertically scrolled.
     */
    void UpdateLineNumberArea(const QRect &rectangle, int vertical_delta);

    /**
     * Highlights the line the user is editing.
     */
    void HighlightCurrentLine();

    /**
     * Wrapper slot for the Scroll One Line Up shortcut.
     */
    void ScrollOneLineUp();

    /**
      * Wrapper slot for the Scroll One Line Down shortcut.
     */
    void ScrollOneLineDown();

    void InsertText(const QString &text);

    void addToUserDictionary(const QString &text);
    void addToDefaultDictionary(const QString &text);
    void ignoreWord(const QString &text);

    void SaveClipAction();

    void GoToLinkOrStyleAction();

    void ReformatCSSMultiLineAction();
    void ReformatCSSSingleLineAction();

    void ReformatHTMLCleanAction();
    void ReformatHTMLCleanAllAction();
    void ReformatHTMLToValidAction();
    void ReformatHTMLToValidAllAction();
    void updateLangMap();

private:
    bool IsMarkedText();

    QString GetCurrentWordAtCaret(bool select_word);

    bool PasteClipEntry(ClipEditorModel::clipEntry *clip);

    /**
     * Returns the text inside < > if cursor is in < >
     */
    QString GetTagText();

    /**
     * Resets the currently used font.
     */
    void ResetFont();

    /**
     * Updates the font used in the line number area
     * and also repaints it.
     *
     * @param font The new font to use.
     */
    void UpdateLineNumberAreaFont(const QFont &font);

    void SetAppearanceColors();

    /**
     * Executes a centerCursor() call if requested
     * with m_DelayedCursorScreenCenteringRequired.
     */
    void DelayedCursorScreenCentering();

    /**
     * Returns the selection offset from the start of the
     * document depending on the search direction specified
     *
     * @param search_direction Depending on this, the anchor or the focus position is returned.
     * @param ignore_selection_offset Should the selection offset be ignored.
     */
    int GetSelectionOffset(Searchable::Direction search_direction, bool ignore_selection_offset, bool marked_text) const;

    /**
     * Scrolls the whole screen by one line.
     * Used for ScrollOneLineUp and ScrollOneLineDown shortcuts.
     * It will also move the cursor position if the
     * scroll would make it "fall of the screen".
     *
     * @param down If \c true, we scroll down. Otherwise, we scroll up.
     */
    void ScrollByLine(bool down);

    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectSignalsToSlots();

    void AddReformatCSSContextMenu(QMenu *menu);

    void AddReformatHTMLContextMenu(QMenu *menu);

    void AddGoToLinkOrStyleContextMenu(QMenu *menu);

    void AddClipContextMenu(QMenu *menu);

    void AddMarkSelectionMenu(QMenu *menu);

    bool AddSpellCheckContextMenu(QMenu *menu);

    void AddViewImageContextMenu(QMenu *menu);

    bool CreateMenuEntries(QMenu *parent_menu, QAction *topAction, QStandardItem *item);

    bool InViewableImage();

    /**
     * An element on the stack when searching for
     * the current caret location.
     */
    struct StackElement {
        /**
         * The tag name.
         */
        QString name;

        /**
         * The number of child elements
         * detected for the element, so far.
         */
        int num_children;
    };

    /**
     * Returns a stack of elements representing the
     * current location of the caret in the document.
     *
     * @param offset The number of characters from document start to the end of
     *               the start tag of the element the caret is residing in.
     * @return The element location stack.
     */
    QStack<StackElement> GetCaretLocationStack(int offset) const;

    /**
     * Takes the stack provided by GetCaretLocationStack()
     * and converts it into the element location hierarchy
     * used by other ViewEditors.
     *
     * @param stack The StackElement stack.
     * @return The converted ElementIndex hierarchy.
     */
    QList<ElementIndex> ConvertStackToHierarchy(const QStack<StackElement> stack) const;


    // Used to convert Hierarchy to QWedPath used by BV and Gumbo
    QString ConvertHierarchyToQWebPath(const QList<ViewEditor::ElementIndex>& hierarchy) const;

    /**
     * Converts a ViewEditor element hierarchy to a tuple describing necessary caret moves.
     * The tuple contains the vertical lines and horizontal chars move deltas
     *
     * @param hierarchy The caret location as ElementIndex hierarchy.
     * @return The info needed to move the caret to the new location.
     */
    std::tuple<int, int> ConvertHierarchyToCaretMove(const QList<ViewEditor::ElementIndex> &hierarchy) const;

    /**
     * Insert HTML tags around the current selection.
     */
    void InsertHTMLTagAroundSelection(const QString &left_element_name, const QString &right_element_name, const QString &attributes = QString());

    void InsertHTMLTagAroundText(const QString &left_element_name, const QString &right_element_name, const QString &attributes, const QString &text);

    /**
     * Is this position within the <body> tag of this text.
     */
    bool IsPositionInBody(const int &pos = -1, const QString &text = QString());
    bool IsPositionInTag(const int &pos = -1, const QString &text = QString());
    bool IsPositionInOpeningTag(const int &pos = -1, const QString &text = QString());
    bool IsPositionInClosingTag(const int &pos = -1, const QString &text = QString());
    QString GetOpeningTagName(const int &pos, const QString &text);
    QString GetClosingTagName(const int &pos, const QString &text);

    void FormatSelectionWithinElement(const QString &element_name, const int &previous_tag_index, const QString &text);

    void ReplaceTags(const int &opening_tag_start, const int &opening_tag_end, const QString &opening_tag_text,
                     const int &closing_tag_start, const int &closing_tag_end, const QString &closing_tag_text);

    /**
     * An element on the stack when searching for
     * the current caret location.
     */
    struct StyleTagElement {
        StyleTagElement() {
            name = QString();
            classStyle = QString();
        }
        /**
         * The tag name.
         */
        QString name;

        /**
         * The class style under the caret location if cursor in the class section.
         * If cursor is on the tag element, is the first style class of this element if any.
         */
        QString classStyle;
    };

    StyleTagElement GetSelectedStyleTagElement();

    /**
     * Given a list of CSS properties perform any pruning/replacing/adding as necessary to
     * ensure that property_name:property_value is added (or removed if it already exists).
     */
    void ApplyChangeToProperties(QList<CSSInfo::CSSProperty *> &css_properties, const QString &property_name, const QString &property_value);

    void ReformatCSS(bool multiple_line_format);

    void ReformatHTML(bool all, bool to_valid);

    QStringList GetUnmatchedTagsForBlock(const int &pos, const QString &text) const;

    void SelectAndScrollIntoView(int start_position, int end_position, Searchable::Direction direction, bool wrapped);

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    /**
     * \c true when an undo action can be performed.
     */
    bool m_isUndoAvailable;

    /**
     * Keeps track of the last block count.
     * Needed because QTextDocument::setPlainText sets
     * this back to 1 before updating it.
     */
    int m_LastBlockCount;

    /**
     * Keep tack of the currenlt selected line number when selected
     * by by clicking on the LineNumberArea.
     */
    int m_LineNumberAreaBlockNumber;

    /**
     * The line number area widget of the code view.
     */
    LineNumberArea *m_LineNumberArea;

    /**
     * The syntax highlighter.
     */
    QSyntaxHighlighter *m_Highlighter;

    /**
     * The view's current zoom factor.
     */
    float m_CurrentZoomFactor;

    /**
     * Catches when the user wants to scroll the view by one line up.
     */
    QShortcut *m_ScrollOneLineUp;

    /**
     * Catches when the user wants to scroll the view by one line down.
     */
    QShortcut *m_ScrollOneLineDown;

    /**
     * Set to \c false whenever the page is loading content.
     */
    bool m_isLoadFinished;

    /**
     * When \c true, a centerCursor() call will be executed
     * once after the View is repainted.
     */
    bool m_DelayedCursorScreenCenteringRequired;

    int m_caretLocation;

    /**
     * Stores the update for the caret location
     * when switching from BookView to CodeView.
     */
    QList<ViewEditor::ElementIndex> m_CaretUpdate;

    /**
     * Whether spell checking is enabled on this view.
     * Misspellings are marked by the QSyntaxHighlighter used.
     */
    bool m_checkSpelling;

    /**
     * Whether reformat CSS context menu option is enabled on this view.
     */
    bool m_reformatCSSEnabled;

    /**
     * Whether reformat (clean) HTML context menu option is enabled on this view.
     */
    bool m_reformatHTMLEnabled;

    /**
     * Store the last match when doing a find so we can determine if
     * found text is selected for doing a replace. We also need to store the
     * match because we can't run the selected text though the PCRE engine
     * (we don't want to because it's slower than caching) because it will fail
     * if a look ahead or behind expression is in use.
     */
    SPCRE::MatchInfo m_lastMatch;
    QString m_lastFindRegex;

    /**
     * Map spelling suggestion actions from the context menu to the
     * ReplaceSelected slot.
     */
    QSignalMapper *m_spellingMapper;
    QSignalMapper *m_addSpellingMapper;
    QSignalMapper *m_addDictMapper;
    QSignalMapper *m_ignoreSpellingMapper;
    QSignalMapper *m_clipMapper;

    int m_MarkedTextStart;
    int m_MarkedTextEnd;
    bool m_ReplacingInMarkedText;

    /**
     * The fonts and colors for appearance of xhtml and text.
     */
    SettingsStore::CodeViewAppearance m_codeViewAppearance;

    /**
     * Whether spelling highlighting should be reapplied when this tab is next given focus.
     */
    bool m_pendingSpellingHighlighting;

    QuickSerialHtmlParser *m_QSHParser;

    //DEBUG
    bool m_DEBUG;
    void AddPosInTxtContextMenu(QMenu *menu);
};

#endif // CODEVIEWEDITOR_H

