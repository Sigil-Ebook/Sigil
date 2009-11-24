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

class QResizeEvent;
class QSize;
class QWidget;
class QPrinter;
class LineNumberArea;
class XHTMLHighlighter;


class CodeViewEditor : public QPlainTextEdit, public ViewEditor
{
    Q_OBJECT

public:

    // Constructor;
    // the parameters is the object's parent
    CodeViewEditor( QWidget *parent = 0 );

    // Sets the content of the View to the specified book
    void SetBook( const Book &book );

    // Paints the line number area;
    // receives the event directly 
    // from the area's paintEvent() handler
    void LineNumberAreaPaintEvent( QPaintEvent *event );

    // Returns the width the LinuNumberArea
    // should take (in pixels)
    int CalculateLineNumberAreaWidth() const;

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

    // Sets a zoom factor for the view,
    // thus zooming in (factor > 1.0) or out (factor < 1.0). 
    void SetZoomFactor( float factor );

    // Returns the View's current zoom factor
    float GetZoomFactor() const;

    // Finds the next occurrence of the search term in the document,
    // and selects the matched string. The first argument is the matching
    // regex, the second is the direction of the search.
    bool FindNext( const QRegExp &search_regex, Searchable::Direction search_direction );

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

private slots:

    // Called whenever the number of lines changes;
    // sets a margin where the line number area can be displayed
    void UpdateLineNumberAreaMargin();

    // The first parameter represents the area 
    // that the editor needs an update of, and the second
    // is the amount of pixels the viewport is vertically scrolled
    void UpdateLineNumberArea( const QRect &rectangle, int vertical_delta );

    // Highlights the line the user is editing
    void HighlightCurrentLine();    

private:

    // Specifies the lines and characters the caret will
    // need to move to get to the required position
    struct CaretMove
    {
        // The vertical lines from
        // the start of the document
        int vertical_lines;

        // The number of horizontal characters
        // on the destination line
        int horizontal_chars;

        CaretMove( int vertical, int horizontal ) 
            : vertical_lines( vertical ), 
              horizontal_chars( horizontal ) {}
    };

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
    QStack< StackElement > GetCaretLocationStack( int offset );

    // Converts the stack provided by GetCaretLocationStack()
    // and converts it into the element location hierarchy
    QList< ElementIndex > ConvertStackToHierarchy( const QStack< StackElement > stack );

    // Executes the caret updating code
    // if such an update is pending;
    // returns true if update was performed
    bool ExecuteCaretUpdate();

    // Returns the selection offset from the start of the 
    // document depending on the search direction specified
    int GetSelectionOffset( Searchable::Direction search_direction );


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // Stores the update for the caret location
    // when switching from BookView to CodeVIew
    CaretMove m_CaretLocationUpdate;

    // The line number area widget of the code view
    LineNumberArea *m_LineNumberArea;

    // The syntax highlighter
    XHTMLHighlighter *m_Highlighter;

    // The view's current zoom factor
    float m_CurrentZoomFactor;
};

#endif // CODEVIEWEDITOR_H

