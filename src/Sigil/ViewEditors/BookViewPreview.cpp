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

#include <QtCore/QSize>
#include <QtCore/QUrl>
#include <QtWebKit/QWebFrame>

#include "Misc/SettingsStore.h"
#include "sigil_constants.h"
#include "ViewEditors/BookViewPreview.h"

const QString SET_CURSOR_JS =
    "var range = document.createRange();"
    "range.setStart(element, 0);"
    "range.setEnd(element, 0);"
    "var selection = window.getSelection();"
    "selection.removeAllRanges();"
    "selection.addRange(range);";


BookViewPreview::BookViewPreview(QWidget *parent)
    : QWebView(parent),
      m_isLoadFinished(false)
{
    setContextMenuPolicy(Qt::NoContextMenu);

    // Set the Zoom factor but be sure no signals are set because of this.
    SettingsStore settings;
    m_CurrentZoomFactor = settings.zoomWeb();
    Zoom();

    // Enable our link filter.
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

    connect(page(), SIGNAL(loadProgress(int)), this, SLOT(UpdateFinishedState(int)));
    connect(page(), SIGNAL(linkClicked(const QUrl&)), this, SLOT(LinkClickedFilter(const QUrl&)));
}

QSize BookViewPreview::sizeHint() const
{
    return QSize(16777215, 16777215);
}

void BookViewPreview::CustomSetDocument(const QString &path, const QString &html)
{
    m_isLoadFinished = false;
    setHtml(html, QUrl::fromLocalFile(path));
}

bool BookViewPreview::IsLoadingFinished()
{
    return m_isLoadFinished;
}

void BookViewPreview::SetZoomFactor(float factor)
{
    SettingsStore settings;
    settings.setZoomWeb(factor);
    m_CurrentZoomFactor = factor;
    Zoom();
    emit ZoomFactorChanged(factor);
}

float BookViewPreview::GetZoomFactor() const
{
    return m_CurrentZoomFactor;
}

void BookViewPreview::Zoom()
{
    setZoomFactor(m_CurrentZoomFactor);
}

void BookViewPreview::UpdateDisplay()
{
    SettingsStore settings;
    float stored_factor = settings.zoomWeb();
    if (stored_factor != m_CurrentZoomFactor) {
        m_CurrentZoomFactor = stored_factor;
        Zoom();
    }
}

void BookViewPreview::ScrollToTop()
{
    QString caret_location = "var elementList = document.getElementsByTagName(\"body\");"
        "var element = elementList[0];";
    QString scroll = "var from_top = window.innerHeight / 2;"
        "$.scrollTo(element, 0, {offset: {top:-from_top, left:0} });";

    EvaluateJavascript(caret_location % scroll % SET_CURSOR_JS);
}

void BookViewPreview::ScrollToFragment(const QString &fragment)
{
    if (fragment.isEmpty()) {
        ScrollToTop();
        return;
    }

    QString caret_location = "var element = document.getElementById(\"" % fragment % "\");";
    QString scroll = "var from_top = window.innerHeight / 2;"
        "$.scrollTo(element, 0, {offset: {top:-from_top, left:0 } });";

    EvaluateJavascript(caret_location % scroll % SET_CURSOR_JS);
}

void BookViewPreview::ScrollToFragmentAfterLoad(const QString &fragment)
{
    if (fragment.isEmpty()) {
        return;
    }

    QString caret_location = "var element = document.getElementById(\"" % fragment % "\");";
    QString scroll = "var from_top = window.innerHeight / 2;"
        "$.scrollTo(element, 0, {offset: {top:-from_top, left:0 } });";
    QString javascript = "window.addEventListener('load', GoToFragment, false);"
        "function GoToFragment() { " % caret_location % scroll % SET_CURSOR_JS % "}";

    EvaluateJavascript(javascript);
}

void BookViewPreview::UpdateFinishedState(int progress)
{
    if (progress == 100) {
        m_isLoadFinished = true;
    }
    else {
        m_isLoadFinished = false;
    }
}

void BookViewPreview::LinkClickedFilter(const QUrl& url)
{
    // Urls in the document that have just "#fragmentID"
    // and no path (that is, "file local" urls), are returned
    // by QUrl.toString() as a path to the folder of this
    // file with the fragment attached.
    if (url.toString().contains("/#")) {
        ScrollToFragment(url.fragment());
    }
    else if (url.scheme() == "file") {
        emit FilteredLinkClicked(url);
    }

    // We kill all links to the internet
}

// Overridden so we can emit the FocusGained() signal.
void BookViewPreview::focusInEvent( QFocusEvent *event )
{
    emit FocusGained(this);
    QWebView::focusInEvent(event);
}

QVariant BookViewPreview::EvaluateJavascript(const QString &javascript)
{
    return page()->mainFrame()->evaluateJavaScript(javascript);
}
