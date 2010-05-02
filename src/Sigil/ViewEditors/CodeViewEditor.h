/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic, Nokia Corporation
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

#include <QPlainTextEdit>
#include "ViewEditor.h"
#include <QStack>
#include <QList>
#include <boost/tuple/tuple.hpp>

class QResizeEvent;
class QSize;
class QWidget;
class QPrinter;
class QShortcut;
class LineNumberArea;
class QSyntaxHighlighter;


class CodeViewEditor : public QPlainTextEdit, public ViewEditor
{
    Q_OBJECT

public: 

    enum HighlighterType
    {
        Highlight_XHTML, 
        Highlight_CSS
    };

    // Constructor;
    // the first parameter says which syn. highlighter to use;
    // the second parameter is the object's parent
    CodeViewEditor( HighlighterType high_type, QWidget *parent = 0 );

    void CustomSetDocument( QTextDocument &document );

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

    // Paints the line number area;
    // receives the event directly 
    // from the area's paintEvent() handler
    void LineNumberAreaPaintEvent( QPaintEvent *event );

    // Returns the width the LinuNumberArea
    // should take (in pixels)
    int CalculateLineNumberAreaWidth();

    // Returns a list of elements representing a "chain"
    // or "walk" through the XHTML document with which one
    // can identify a single element in the document.
    // This list identifies the element in which the 
    // keyboard caret is currently located.
    QList< ViewEditor::ElementIndex > GetCaretLocation(); 

    // Accepts a list returned by a view's GetCaretLocation
    // and creates and stores an update that sends the caret
    // in this view to the specified element.
    // The CodeView implementation initiates the update in
    // the main event handler.
    void StoreCaretLocationUpdate( const QList< ViewEditor::ElementIndex > &hierarchy );

    bool IsLoadingFinished();

    void ScrollToTop();

    // Sets a zoom factor for the view,
    // thus zooming in (factor > 1.0) or out (factor < 1.0). 
    void SetZoomFactor( float factor );

    // Returns the View's current zoom factor
    float GetZoomFactor() const;

    // Finds the next occurrence of the search term in the document,
    // and selects the matched string. The first argument is the matching
    // regex, the second is the direction of the search.
    bool FindNext( const QRegExp &search_regex, 
                   Searchable::Direction search_direction,
                   bool ignore_selection_offset = false );

    // Returns the number of times that the specified
    // regex matches in the document.
    int Count( const QRegExp &search_regex );

    // If the currently selected text matches the specified regex, 
    // it is replaced by the specified replacement string.
    bool ReplaceSelected( const QRegExp &search_regex, const QString &replacement );

    // Replaces all occurrences of the specified regex in 
    // the document with the specified replacement string.
    int ReplaceAll( const QRegExp &search_regex, const QString &replacement );


signals:
    
    // Emitted whenever the zoom factor changes
    void ZoomFactorChanged( float new_zoom_factor );

    void FocusLost();

    void FocusGained();

    /**
     * A filtered version of the QPlainTextEdit::textChnaged signal.
     * We use it to prevent our syntax highlighter from emitting that signal.
     */
    void FilteredTextChanged();

public slots:

    // The base class implementation of the print()
    // method is not a slot, and we need it as a slot
    // for print preview support; so this is just
    // a slot wrapper around that function 
    void print( QPrinter* printer );

protected:
    
    // Overridden because we need to update the cursor
    // location if a cursor update (from BookView) 
    // is waiting to be processed
    bool event( QEvent *event );

    // Overridden because after updating itself on a resize event,
    // the editor needs to update its line number area too
    void resizeEvent( QResizeEvent *event );

    // Overridden because we want the ExecuteCaretUpdate()
    // to be called from here when the user clicks inside
    // this widget in SplitView. Leaving it up to our event()
    // override causes graphical artifacts for SplitView.
    // So in those conditions, this handler takes over.
    void mousePressEvent( QMouseEvent *event );

    void focusInEvent( QFocusEvent *event );

    void focusOutEvent( QFocusEvent *event );

private slots:

    /**
     * Filters the textChanged signal.
     * Does this based on the availability of undo.
     */
    void TextChangedFilter();

    /**
     * Used solely to update the m_isUndoAvailable variable
     * on undo availability change.
     *
     * @param available The current availability of the undo action.
     */
    void UpdateUndoAvailable( bool available );

    // Called whenever the number of lines changes;
    // sets a margin where the line number area can be displayed
    void UpdateLineNumberAreaMargin();

    // The first parameter represents the area 
    // that the editor needs an update of, and the second
    // is the amount of pixels the viewport is vertically scrolled
    void UpdateLineNumberArea( const QRect &rectangle, int vertical_delta );

    // Highlights the line the user is editing
    void HighlightCurrentLine(); 

    // Wrapper slot for the Scroll One Line Up shortcut
    void ScrollOneLineUp();

    // Wrapper slot for the Scroll One Line Down shortcut
    void ScrollOneLineDown();

private:

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

    // An element on the stack when searching for
    // the current caret location. 
    struct StackElement
    {
        // The tag name
        QString name;

        // The number of child elements
        // detected for the element, so far.
        int num_children;
    };

    // Returns a stack of elements representing the
    // current location of the caret in the document.
    // Accepts the number of characters to the end of
    // the start tag of the element the caret is residing in. 
    QStack< StackElement > GetCaretLocationStack( int offset ) const;

    // Converts the stack provided by GetCaretLocationStack()
    // and converts it into the element location hierarchy
    QList< ElementIndex > ConvertStackToHierarchy( const QStack< StackElement > stack ) const;

    // Converts a ViewEditor element hierarchy to a "CaretMove" tuple:
    // the tuple contains the vertical lines and horizontal chars move deltas
    boost::tuple< int, int > ConvertHierarchyToCaretMove( const QList< ViewEditor::ElementIndex > &hierarchy ) const;

    // Executes the caret updating code
    // if such an update is pending;
    // returns true if update was performed
    bool ExecuteCaretUpdate();

    // Returns the selection offset from the start of the 
    // document depending on the search direction specified
    int GetSelectionOffset( Searchable::Direction search_direction, bool ignore_selection_offset ) const;

    // Scrolls the whole screen by one line.
    // The parameter specifies are we scrolling up or down.
    // Used for ScrollOneLineUp and ScrollOneLineDown shortcuts.
    // It will also move the cursor position if the
    // scroll would make it "fall of the screen".
    void ScrollByLine( bool down );
    
    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectSignalsToSlots();


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    bool m_isUndoAvailable;

    /**
     * Keeps track of the last block count.
     * Needed because QTextDocument::setPlainText sets
     * this back to 1 before updating it.
     */
    int m_LastBlockCount;

    // The line number area widget of the code view
    LineNumberArea *m_LineNumberArea;

    // The syntax highlighter
    QSyntaxHighlighter *m_Highlighter;

    // The view's current zoom factor
    float m_CurrentZoomFactor;

    // Stores the update for the caret location
    // when switching from BookView to CodeVIew
    QList< ViewEditor::ElementIndex >  m_CaretUpdate;

    // These shortcuts catch when the user
    // wants to scroll the view by one line up/down.
    QShortcut &m_ScrollOneLineUp;
    QShortcut &m_ScrollOneLineDown;

    /**
     * Set to \c false whenever the page is loading content.
     */
    bool m_isLoadFinished;
};

#endif // CODEVIEWEDITOR_H

