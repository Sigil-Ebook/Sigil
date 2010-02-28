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
#ifndef BOOKVIEWEDITOR_H
#define BOOKVIEWEDITOR_H

#include <QWebView>
#include "ViewEditor.h"
#include <QDomNode>
#include <QMap>
#include <QWebElement>

class QShortcut;

class BookViewEditor : public QWebView, public ViewEditor
{
    Q_OBJECT

public:

    // Constructor;
    // the parameters is the object's parent
    BookViewEditor( QWidget *parent = 0 );

    void CustomSetWebPage( QWebPage &webpage );

    QString SplitChapter();

    // Executes the specified command on the document with javascript
    void ExecCommand( const QString &command );

    // Executes the specified command with the specified parameter
    // on the document with javascript
    void ExecCommand( const QString &command, const QString &parameter );

    // Returns the state of the JavaScript command provided
    bool QueryCommandState( const QString &command );

    // Workaround for a crappy setFocus implementation in Webkit
    void GrabFocus();

    void ScrollToTop();

    void ScrollToFragment( const QString &fragment );

    void ScrollToFragmentAfterLoad( const QString &fragment );

    // Implements the "formatBlock" execCommand because
    // WebKit's default one has bugs.
    // It takes an element name as an argument (e.g. "p"),
    // and replaces the element the cursor is located in with it.
    void FormatBlock( const QString &element_name );

    // Returns the name of the element the caret is located in;
    // if text is selected, returns the name of the element
    // where the selection *starts*
    QString GetCaretElementName();

    // Returns a list of elements representing a "chain"
    // or "walk" through the XHTML document with which one
    // can identify a single element in the document.
    // This list identifies the element in which the 
    // keyboard caret is currently located.
    QList< ViewEditor::ElementIndex > GetCaretLocation(); 

    // Accepts a list returned by a view's GetCaretLocation
    // and creates and stores an update that sends the caret
    // in this view to the specified element.
    // The BookView implementation initiates the update in
    // the JavascriptOnDocumentLoad() function.
    // This should *always* be called *before* the page is updated
    // to avoid loading race conditions.
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

    // The contentsChanged QWebPage signal is wired to this one,
    // and we also emit it ourselves when necessary.
    void textChanged();

    // The selectionChanged QWebPage signal is wired to this one,
    // and we also emit it ourselves when necessary.
    void selectionChanged();

    // Emitted whenever the zoom factor changes
    void ZoomFactorChanged( float new_zoom_factor );

    void FilteredLinkClicked( const QUrl& url );

protected:

    // Overridden because we need to update the cursor
    // location if a cursor update (from CodeView) 
    // is waiting to be processed.
    // The update is pending only when we switch
    // from Code View and there were no source changes
    bool event( QEvent *event );

private slots:

    // Executes javascript that needs to be run when
    // the document has finished loading
    void JavascriptOnDocumentLoad();

    // Updates the state of the m_isLoadFinished variable
    // depending on the received loading progress; if the 
    // progress equals 100, the state is true, otherwise false.
    void UpdateFinishedState( int progress );

    void LinkClickedFilter( const QUrl& url );

    // Wrapper slot for the Page Up shortcut
    void PageUp();

    // Wrapper slot for the Page Down shortcut
    void PageDown();

    // Wrapper slot for the Scroll One Line Up shortcut
    void ScrollOneLineUp();

    // Wrapper slot for the Scroll One Line Down shortcut
    void ScrollOneLineDown();

private:

    // Evaluates the provided javascript source code 
    // and returns the result of the last executed javascript statement
    QVariant EvaluateJavascript( const QString &javascript );

    // Returns the local character offset of the selection
    // (in the local text node). Depending on the argument,
    // it returns the offset of the start of the selection or the end.
    int GetLocalSelectionOffset( bool start_of_selection );

    // Returns the selection offset from the start of the document.
    // The first argument is the loaded DOM doc, the second is the
    // text node offset map and the third is the search direction.
    int GetSelectionOffset( const QDomDocument &document,
                            const QMap< int, QDomNode > &node_offsets, 
                            Searchable::Direction search_direction );

    // Returns the currently selected text string
    QString GetSelectedText();

    // The necessary tools for searching
    struct SearchTools
    {
        // The full text of the document
        QString fulltext;

        // A map with text node starting offsets as keys,
        // and those text nodes as values.
        QMap< int, QDomNode > node_offsets;

        // A DOM doc with the loaded text
        QDomDocument document;
    };

    // Returns the all the necessary tools for searching.
    // Reads from the QWebPage source.
    SearchTools GetSearchTools() const;    

    // Returns the element selecting javascript code that completely
    // ignores text nodes and always just chains children() jQuery calls
    QString GetElementSelectingJS_NoTextNodes( const QList< ViewEditor::ElementIndex > &hierarchy ) const;

    // Returns the element selecting javascript code that chains
    // text node ignoring children() jQuery calls, but that uses
    // contents() for the last element (the text node, naturally)
    QString GetElementSelectingJS_WithTextNodes( const QList< ViewEditor::ElementIndex > &hierarchy ) const;

    // Escapes a string so that it can be embedded
    // inside a javascript source code string
    QString EscapeJSString( const QString &string );

    // Executes the caret updating code
    // if an update is pending;
    // returns true if update was performed
    bool ExecuteCaretUpdate();

    // The inputs for a new javascript range object
    struct SelectRangeInputs
    {
        // The range start node
        QDomNode start_node;

        // The range end node
        QDomNode end_node;

        // The char index inside the start node
        int start_node_index;

        // The char index inside the end node
        int end_node_index;
    };

    // Accepts a node offset map, the index of the string in the full doc text
    // and the string's length. Converts this information into a struct
    // with which a JS range object can be created to select this particular string.
    SelectRangeInputs GetRangeInputs( const QMap< int, QDomNode > &node_offsets, int string_start, int string_length ) const;

    // Accepts the range input struct and returns  
    // the range creating javascript code
    QString GetRangeJS( const SelectRangeInputs &input ) const;

    // Selects the string identified by the range inputs
    void SelectTextRange( const SelectRangeInputs &input );

    // Scrolls the view to the specified node.
    // Does NOT center the node in view.
    void ScrollToNode( const QDomNode &node );

    // Scrolls the whole screen by one line.
    // The parameter specifies are we scrolling up or down.
    // Used for ScrollOneLineUp and ScrollOneLineDown shortcuts.
    void ScrollByLine( bool down );

    // Scrolls the whole screen by pixel_number.
    // "down" specifies are we scrolling up or down.
    void ScrollByNumPixels( int pixel_number, bool down );


    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    // The javascript source code of the jQuery library
    const QString c_JQuery;

    // The javascript source code of the jQuery
    // ScrollTo extension library
    const QString c_JQueryScrollTo;

    // The javascript source code used
    // to get a hierarchy of elements from
    // the caret element to the top of the document
    const QString c_GetCaretLocation;

    // The javascript source code that
    // removes all of the current selections
    // and adds the range in the "range"
    // variable to the current selection.
    const QString c_NewSelection;

    // The javascript source code
    // for creating DOM ranges
    const QString c_GetRange;

    // The javascript source code that deletes the
    // contents of the range in "range" and replaces
    // them with a new text node whose text should be inputted.
    const QString c_ReplaceText;

    const QString c_GetSegmentHTML;

    // The javascript source code for the 
    // caret update when switching from 
    // CodeView to BookView
    QString m_CaretLocationUpdate;

    // Is set to false whenever the page is loading content
    bool m_isLoadFinished;

    // Used for PageUp and PageDown keyboard shortcuts
    QShortcut &m_PageUp;
    QShortcut &m_PageDown; 

    // These shortcuts catch when the user
    // wants to scroll the view by one line up/down.
    QShortcut &m_ScrollOneLineUp;
    QShortcut &m_ScrollOneLineDown;
};


#endif // BOOKVIEWEDITOR_H

