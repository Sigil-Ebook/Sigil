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

#ifndef BOOKVIEWPREVIEW_H
#define BOOKVIEWPREVIEW_H

#include <QtWebKit/QWebView>

#include "ViewEditors/ViewEditor.h"

class QSize;

class BookViewPreview : public QWebView, public ViewEditor
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param parent The object's parent.
     */
    BookViewPreview(QWidget *parent=0);

    QSize sizeHint() const;

    void CustomSetDocument(const QString &path, const QString &html);

    bool IsLoadingFinished();

    void SetZoomFactor(float factor);

    float GetZoomFactor() const;

    void Zoom();

    void UpdateDisplay();

    /**
     * Scrolls the editor to the top.
     */
    void ScrollToTop();

    /**
     * Scrolls the editor to the specified fragment.
     *
     * @param fragment The fragment ID to scroll to.
     *                 It should have the "#" character as the first character.
     */
    void ScrollToFragment(const QString &fragment);

    /**
     * Scrolls the editor to the specified fragment after the document is loaded.
     *
     * @param fragment The fragment ID to scroll to.
     *                 It should have the "#" character as the first character.
     */
    void ScrollToFragmentAfterLoad(const QString &fragment);

    bool FindNext(const QString &search_regex,
                  Searchable::Direction search_direction,
                  bool check_spelling=false,
                  bool ignore_selection_offset=false,
                  bool wrap=true);

    int Count(const QString &search_regex, bool check_spelling);

    bool ReplaceSelected(const QString &search_regex, const QString &replacement, Searchable::Direction direction=Searchable::Direction_Down, bool check_spelling=false);

    int ReplaceAll(const QString &search_regex, const QString &replacement, bool check_spelling);

    QString GetSelectedText();

    /**
     *  Workaround for a crappy setFocus implementation in QtWebKit.
     */
    void GrabFocus();

signals:
    /**
     * Emitted whenever the zoom factor changes.
     *
     * @param new_zoom_factor The new zoom factor of the View.
     */
    void ZoomFactorChanged(float new_zoom_factor);

    /**
     * A filtered version of the QWebPage::linkClicked signal.
     * Should be used in place of that one, since this one
     * performs custom logic on the QUrl.
     *
     * @param url The URL of the clicked link.
     */
    void FilteredLinkClicked(const QUrl& url);

    /**
     * Emitted when the focus is gained.
     */
    void FocusGained(QWidget* editor);

protected:
    /**
     * Handles the focus in event for the editor.
     *
     * @param event The event to process.
     */
    void focusInEvent(QFocusEvent *event);

    /**
     * Evaluates the provided javascript source code
     * and returns the result.
     *
     * @param javascript The JavaScript source code to execute.
     * @return The result from the last executed javascript statement.
     */
    QVariant EvaluateJavascript(const QString &javascript);

    bool m_isLoadFinished;

protected slots:
    /**
     * Tracks the loading progress.
     * Updates the state of the m_isLoadFinished variable
     * depending on the received loading progress; if the
     * progress equals 100, the state is true, otherwise false.
     *
     * @param progress The value of the loading progress (0-100).
     */
    void UpdateFinishedState(int progress);

    /**
     * Filters the linkCLicked signal.
     *
     * @param url The URL of the clicked link.
     */
    void LinkClickedFilter(const QUrl& url);

private slots:
    
    /**
     * Loads the required JavaScript on web page loads.
     */
    void WebPageJavascriptOnLoad();

private:

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    float m_CurrentZoomFactor;

    /**
     * The javascript source code of the jQuery library.
     */
    const QString c_jQuery;
    
    /**
     * The javascript source code of the jQuery 
     * ScrollTo extension library.
     */
    const QString c_jQueryScrollTo;

    /**
     * Javascript source for the wrap selection plugin.
     */
    const QString c_jQueryWrapSelection;

    /** 
     * The JavaScript source code used 
     * to get a hierarchy of elements from
     * the caret element to the top of the document.
     */
    const QString c_GetCaretLocation;
};

#endif // BOOKVIEWPREVIEW_H
