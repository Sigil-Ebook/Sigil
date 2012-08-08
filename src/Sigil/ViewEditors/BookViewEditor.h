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

#include "ViewEditors/BookViewPreview.h"
#include "ViewEditors/ViewEditor.h"

class QEvent;
class QSize;
class QShortcut;

/**
 * A WYSIWYG editor for XHTML flows.
 * Also called the "Book View", because it shows a
 * chapter of a book in its final, rendered state
 * (the way it will look like in epub Reading Systems).
 */
class BookViewEditor : public BookViewPreview
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param parent The object's parent.
     */
    BookViewEditor(QWidget *parent=0);

    /**
     * Sets a custom webpage for the editor.
     */
    void CustomSetDocument(const QString &path, const QString &html);

    void CustomUpdateDocument(const QString &html, bool saveSelection = true);

    void ScrollToFragment(const QString &fragment);

    void ScrollToFragmentAfterLoad(const QString &fragment);

    QString GetHtml();
    //QString GetXHtml11();
    //QString GetHtml5();

    void InsertHtml(const QString &html);

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

    bool IsModified();
    void ResetModified();

    void Undo();
    void Redo();

    // Even though the BookViewPreview implements these they are pure virtual
    // in ViewEditor so they have to be implemented here.
    float GetZoomFactor() const { return BookViewPreview::GetZoomFactor(); }
    void SetZoomFactor(float factor) { BookViewPreview::SetZoomFactor(factor); }
    bool IsLoadingFinished() { return BookViewPreview::IsLoadingFinished(); }

    bool FindNext(const QString &search_regex,
                  Searchable::Direction search_direction,
                  bool check_spelling=false,
                  bool ignore_selection_offset=false,
                  bool wrap=true)
    {
        return BookViewPreview::FindNext(search_regex, search_direction, check_spelling, ignore_selection_offset, wrap);
    }

    int Count(const QString &search_regex, bool check_spelling)
    {
        return BookViewPreview::Count(search_regex, check_spelling);
    }

    bool ReplaceSelected(const QString &search_regex, const QString &replacement, Searchable::Direction direction=Searchable::Direction_Down, bool check_spelling=false)
    {
        return BookViewPreview::ReplaceSelected(search_regex, replacement, direction, check_spelling);
    }

    int ReplaceAll(const QString &search_regex, const QString &replacement, bool check_spelling)
    {
        return BookViewPreview::ReplaceAll(search_regex, replacement, check_spelling);
    }

    QString GetSelectedText();

    /**
     * Executes a contentEditable command.
     * The command is executed through JavaScript.
     *
     * @param command The command to execute.
     */
    void ExecCommand( const QString &command );

    /**
     * Executes a contentEditable command.
     * The command is executed through JavaScript.
     *
     * @param command The command to execute.
     * @param parameter The parameter that should be passed to the command.
     */
    void ExecCommand( const QString &command, const QString &parameter );

    /**
     * Returns the state of the contentEditable command.
     * The query is performed through JavaScript.
     */
    bool QueryCommandState( const QString &command );        
    
    /**
     * Implements the "formatBlock" execCommand because
     * WebKit's default one has bugs.
     * It takes an element name as an argument (e.g. "p"),
     * and replaces the element the cursor is located in with it.
     *
     * @param element_name The name of the element to format the block to.
     * @param preserve_attributes Whether to keep any existing attributes on the previous block tag.
     */
    void FormatBlock( const QString &element_name, bool preserve_attributes );

    /**
     * Returns the name of the element the caret is located in.
     * If text is selected, returns the name of the element
     * where the selection \em starts.
     *
     * @return The name of the caret element.
     */
    QString GetCaretElementName();

public slots:
    /**
     * Filters the text changed signals by the CKEditor inside of the page
     * and then takes the appropriate action.
     *
     * The CKEditor is written in Javascript and runs inside of the webpage.
     * Using the magic of QtWebKit we link the Javascript calls to this function
     * so CKEditor can communicate to this C++ instance of the BooViewEditor.
     *
     * This must be a public slot so it is visible and connectable to the
     * Javascript inside of the web page.
     */
    void TextChangedFilter();

signals:
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
    void FocusLost(QWidget* editor);

protected:
    /**
     * Handles the focus out event for the editor.
     *
     * @param event The event to process.
     */
    void focusOutEvent(QFocusEvent *event);
    
private slots:

    /**
     * Wrapper slot for the Page Up shortcut.
     */
    void PageUp();

    /**
     * Wrapper slot for the Page Down shortcut.
     */
    void PageDown();
    
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
    void SetWebPageModified( bool modified = true );

private:
    /**
     * Escapes JavaScript string special characters.
     *
     * @return The escaped string.
     */
    QString EscapeJSString( const QString &string );
    
    /**
     * Scrolls the whole screen by one line. 
     * Used for ScrollOneLineUp and ScrollOneLineDown shortcuts.
     * 
     * @param down Specifies are we scrolling up or down.
     */
    void ScrollByLine( bool down );

    /**
     * Scrolls the whole screen a number of pixels.
     *
     * @param pixel_number The number of pixels to scroll
     * @param down Specifies are we scrolling up or down.
     */
    void ScrollByNumPixels( int pixel_number, bool down );
    
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
    QString RemoveBookViewReplaceSpans( const QString &source );

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
     * removed from the current chapter.
     */
    const QString c_GetSegmentHTML;

    /**
     * Javascript source that implements a function to find the
     * first block-level parent of a node in the source.
     */
    const QString c_GetBlock;

    /**
     * Javascript source that implements a function to format the
     * first block-level parent of a node in the source.
     */
    const QString c_FormatBlock;
};


#endif // BOOKVIEWEDITOR_H

