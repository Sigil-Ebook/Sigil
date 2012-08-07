/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>, Nokia Corporation
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

#include <boost/tuple/tuple.hpp>

#include <QtCore/QList>
#include <QtCore/QStack>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QStandardItem>
#include <QtCore/QUrl>

#include "ViewEditors/ViewEditor.h"
#include "MiscEditors/IndexEditorModel.h"
#include "MiscEditors/ClipboardEditorModel.h"

class QResizeEvent;
class QSize;
class QWidget;
class QPrinter;
class QShortcut;
class LineNumberArea;
class QSyntaxHighlighter; class QContextMenuEvent;
class QSignalMapper;

/**
 * A text editor for source code.
 * Also called the "Code View" because it shows
 * the code of a chapter of the book. Provides syntax highlighting.
 */
class CodeViewEditor : public QPlainTextEdit, public ViewEditor
{
    Q_OBJECT

public:

    /**
     * What type of syntax highlighting to use.
     */
    enum HighlighterType
    {
        Highlight_XHTML, /**< XHTML source code highlighting */
        Highlight_CSS    /**< CSS source code highlighting */
    };

    /**
     * Constructor.
     *
     * @param highlighter_type Which syntax highlighter to use.
     * @param parent The object's parent.
     */
    CodeViewEditor( HighlighterType highlighter_type, bool check_spelling = false, QWidget *parent = 0 );

    QSize sizeHint() const;

    /**
     * A custom implementation of QPlainTextEdit::setDocument()
     * since that doesn't do everything we want it to.
     *
     * @param document The new text document.
     */
    void CustomSetDocument( QTextDocument &document );

    /**
     * Routines to handle cutting code tags from selected text
     */
    void CutCodeTags();
    bool IsCutCodeTagsAllowed();
    bool IsNotInTagTextSelected();
    QString StripCodeTags(QString text);

    bool IsPositionInTag(int pos);

    bool IsInsertClosingTagAllowed();

    bool IsOpenAllowed();

    /**
    * Splits the chapter and returns the "upper" content.
    * The current flow is split at the caret point.
    *
    * @return The content of the chapter up to the chapter break point.
    *
    * @note What we actually do when the user wants to split the loaded chapter
    * is create a new tab with the XHTML content \em above the split point.
    * The new tab is actually the "old" chapter, and this tab becomes the
    * "new" chapter.
    * \par
    * Why? Because we can only avoid a tab render in the tab from which
    * we remove elements. Since the users move from the top of a large HTML
    * file down, the new chapter will be the one with the most content.
    * So this way we \em try to avoid the painful render time on the biggest
    * chapter, but there is still some render time left...
    */
    QString SplitChapter();

    /**
     * Inserts the SGF chapter marker code at the current caret location.
     */
    void InsertSGFChapterMarker();

    void InsertClosingTag();

    /**
     * Paints the line number area.
     * Receives the event directly from the area's paintEvent() handler.
     *
     * @param event The paint event to process.
     */
    void LineNumberAreaPaintEvent( QPaintEvent *event );

    /**
     * Selects the line that was clicked on.
     * Receives the event directly from the area's mouseEvent() handler.
     *
     * @param event The mouse event to process.
     */
    void LineNumberAreaMouseEvent( QMouseEvent *event );

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
    void ReplaceDocumentText( const QString &new_text );

    /**
     * Scrolls the entire view to the top.
     */
    void ScrollToTop();

    void ScrollToPosition( int cursor_position );

    /**
     * Scrolls the view to the specified line.
     *
     * @param line The line to scroll to.
     */
    void ScrollToLine( int line );

    void ScrollToFragment( const QString &fragment );

    // inherited
    bool IsLoadingFinished();

    int GetCursorPosition() const;
    int GetCursorLine() const;
    int GetCursorColumn() const;

    void SetZoomFactor( float factor );

    float GetZoomFactor() const;

    void Zoom();

    void UpdateDisplay();

    SPCRE::MatchInfo GetMisspelledWord( const QString &text,
                                        int start_offset,
                                        int end_offset,
                                        const QString &search_regex,
                                        Searchable::Direction search_direction );

    bool FindNext( const QString &search_regex,
                   Searchable::Direction search_direction,
                   bool check_spelling = false,
                   bool ignore_selection_offset = false,
                   bool wrap = true );

    int Count( const QString &search_regex,
               bool check_spelling );

    bool ReplaceSelected( const QString &search_regex,
                          const QString &replacement,
                          Searchable::Direction direction = Searchable::Direction_Down,
                          bool check_spelling = false );

    int ReplaceAll( const QString &search_regex,
                    const QString &replacement,
                    bool check_spelling = false );

    QString GetSelectedText();

    /**
     * Sets flag to execute a centerCursor() call later
     * with m_DelayedCursorScreenCenteringRequired.
     */
    void SetDelayedCursorScreenCenteringRequired();

    void SetBackToLinkAllowed(bool allowed);

    // inherited
    QList< ViewEditor::ElementIndex > GetCaretLocation(); 

    // inherited
    void StoreCaretLocationUpdate( const QList< ViewEditor::ElementIndex > &hierarchy );

    // inherited
    bool ExecuteCaretUpdate();
    	
signals:

    /**
     * Emitted whenever the zoom factor changes.
     *
     * @param new_zoom_factor The new zoom factor of the View.
     */
    void ZoomFactorChanged( float new_zoom_factor );

    /**
     * Emitted when the focus is lost.
     */
    void FocusLost( QWidget *editor );

    /**
     * Emitted when the focus is gained.
     */
    void FocusGained( QWidget *editor );

    /**
     * A filtered version of the QPlainTextEdit::textChnaged signal.
     * We use it to prevent our syntax highlighter from emitting that signal.
     */
    void FilteredTextChanged();

    void OpenClipboardEditorRequest(ClipboardEditorModel::clipEntry *);

    void OpenIndexEditorRequest(IndexEditorModel::indexEntry *);

    void OpenCodeLinkRequest(const QUrl &url);

    void OpenExternalUrl(const QUrl &url);

    void OpenLastCodeLinkOpenedRequest();

public slots:

    /**
     * A slot wrapper around the base class print() function.
     *
     * @param printer The printer interface to use for printing.
     */
    void print( QPrinter* printer );

    void LoadSettings();

    void PasteClipboardEntryFromName(QString name);
    void PasteClipboardEntries(QList<ClipboardEditorModel::clipEntry *> clips);
    void PasteClipboardEntry(ClipboardEditorModel::clipEntry *clip);

protected:

    /**
     * The global event processing function.
     *
     * @param event The event to process.
     */
    bool event( QEvent *event );

    /**
     * Handles the resize event for the editor.
     *
     * @param event The event to process.
     */
    void resizeEvent( QResizeEvent *event );

    /**
     * Handles the mouse press event for the editor.
     *
     * @param event The event to process.
     */
    void mousePressEvent( QMouseEvent *event );

    /**
     * Handles the content menu event for the editor.
     *
     * @param event The event to process.
     */
    void contextMenuEvent( QContextMenuEvent *event );

    /**
     * Handles the focus in event for the editor.
     *
     * @param event The event to process.
     */
    void focusInEvent( QFocusEvent *event );

    /**
     * Handles the focus out event for the editor.
     *
     * @param event The event to process.
     */
    void focusOutEvent( QFocusEvent *event );

private slots:
    /**
     * Filters the textChanged signal.
     * It does this based on the availability of undo.
     */
    void TextChangedFilter();

    /**
     * Used solely to update the m_isUndoAvailable variable
     * on undo availability change.
     *
     * @param available The current availability of the undo action.
     */
    void UpdateUndoAvailable( bool available );

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
    void UpdateLineNumberArea( const QRect &rectangle, int vertical_delta );

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

    /**
     * Replace the selected text with the the given text.
     *
     * @param text The string to replace the selected text with.
     */
    void ReplaceSelected(const QString &text);

    void addToUserDictionary(const QString &text);

    void ignoreWordInDictionary(const QString &text);

    void OpenClipboardEditor();

    void SaveClipboardAction();

    void SaveIndexAction();

    void MarkIndexAction();

    QUrl GetInternalLinkInTag();
    
    void OpenLinkAction();

    void BackToLinkAction();

private:

    /**
     * Returns the text inside < > if cursor is in < >
     */
    QString GetTagText();

    QString GetAttributeText(QString text, QString attribute);

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
    void UpdateLineNumberAreaFont( const QFont &font );

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
    int GetSelectionOffset( Searchable::Direction search_direction, bool ignore_selection_offset ) const;

    /**
     * Scrolls the whole screen by one line.
     * Used for ScrollOneLineUp and ScrollOneLineDown shortcuts.
     * It will also move the cursor position if the
     * scroll would make it "fall of the screen".
     *
     * @param down If \c true, we scroll down. Otherwise, we scroll up.
     */
    void ScrollByLine( bool down );

    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectSignalsToSlots();

    void AddClipboardContextMenu(QMenu *menu);

    bool AddSpellCheckContextMenu(QMenu *menu);

    bool CreateMenuEntries(QMenu *parent_menu, QAction *topAction, QStandardItem *item);

    void AddOpenLinkContextMenu(QMenu *menu);

    void AddIndexContextMenu(QMenu *menu);

    /**
     * An element on the stack when searching for 
     * the current caret location. 
     */
    struct StackElement
    {
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
    QStack< StackElement > GetCaretLocationStack( int offset ) const;

    /**
     * Takes the stack provided by GetCaretLocationStack() 
     * and converts it into the element location hierarchy 
     * used by other ViewEditors.
     *
     * @param stack The StackElement stack.
     * @return The converted ElementIndex hierarchy.
     */
    QList< ElementIndex > ConvertStackToHierarchy( const QStack< StackElement > stack ) const;

    /**
     * Converts a ViewEditor element hierarchy to a tuple describing necessary caret moves. 
     * The tuple contains the vertical lines and horizontal chars move deltas
     *
     * @param hierarchy The caret location as ElementIndex hierarchy.
     * @return The info needed to move the caret to the new location.
     */
    boost::tuple< int, int > ConvertHierarchyToCaretMove( const QList< ViewEditor::ElementIndex > &hierarchy ) const;


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
    QShortcut &m_ScrollOneLineUp;

    /**
     * Catches when the user wants to scroll the view by one line down.
     */
    QShortcut &m_ScrollOneLineDown;

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
    QList< ViewEditor::ElementIndex > m_CaretUpdate;

    /**
     * Whether spell checking is enabled on this view.
     * Misspellings are marked by the QSyntaxHighlighter used.
     */
    bool m_checkSpelling;

    /**
     * Store the last match when doing a find so we can determine if
     * found text is selected for doing a replace. We also need to store the
     * match because we can't run the selected text though the PCRE engine
     * (we don't want to because it's slower than caching) because it will fail
     * if a look ahead or behind expression is in use.
     */
    SPCRE::MatchInfo m_lastMatch;

    /**
     * Map spelling suggestion actions from the context menu to the
     * ReplaceSelected slot.
     */
    QSignalMapper *m_spellingMapper;
    QSignalMapper *m_addSpellingMapper;
    QSignalMapper *m_ignoreSpellingMapper;
    QSignalMapper *m_clipboardMapper;

    bool m_BackToLinkAllowed;
};

#endif // CODEVIEWEDITOR_H

