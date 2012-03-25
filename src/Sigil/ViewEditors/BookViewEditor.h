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

#include "ViewEditors/BookViewPreview.h"
#include "ViewEditors/ViewEditor.h"

class QSize;

/**
 * A WYSIWYG editor for XHTML flows. 
 * Also called the "Book View", because it shows a
 * chapter of a book in its final, rendered state
 * (the way it will look like in epub Reading Systems).
 */
class BookViewEditor : public BookViewPreview, public ViewEditor
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

    void ScrollToFragment(const QString &fragment);

    void ScrollToFragmentAfterLoad(const QString &fragment);

    QString GetHtml();
    QString GetXHtml11();
    QString GetHtml5();

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

    /**
     *  Workaround for a crappy setFocus implementation in QtWebKit.
     */
    void GrabFocus();

    bool IsModified();
    void ResetModified();

    // Even though the BookViewPreview implements these they are pure virtual
    // in ViewEditor so they have to be implemented here.
    float GetZoomFactor() const { return BookViewPreview::GetZoomFactor(); }
    void SetZoomFactor(float factor) { BookViewPreview::SetZoomFactor(factor); }
    bool IsLoadingFinished() { return BookViewPreview::IsLoadingFinished(); }

    bool FindNext(const QString &search_regex,
                  Searchable::Direction search_direction,
                  bool check_spelling=false,
                  bool ignore_selection_offset=false,
                  bool wrap=true);

    int Count(const QString &search_regex, bool check_spelling);

    bool ReplaceSelected(const QString &search_regex, const QString &replacement, Searchable::Direction direction=Searchable::Direction_Down, bool check_spelling=false);

    int ReplaceAll(const QString &search_regex, const QString &replacement, bool check_spelling);

    QString GetSelectedText();

    void SaveCaret();
    void RestoreCaret();

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

private:
    /**
     * Store the last match when doing a find so we can determine if
     * found text is selected for doing a replace. We also need to store the
     * match because we can't run the selected text though the PCRE engine
     * (we don't want to because it's slower than caching) because it will fail
     * if a look ahead or behind expression is in use.
     */
    SPCRE::MatchInfo m_lastMatch;

    QVariant m_caret;
};


#endif // BOOKVIEWEDITOR_H

