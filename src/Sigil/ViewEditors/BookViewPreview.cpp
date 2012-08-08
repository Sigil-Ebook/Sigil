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

#include <QtCore/QEvent>
#include <QtCore/QSize>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QMessageBox>
#include <QtWebKit/QWebFrame>

#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
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
      c_GetCaretLocation( Utility::ReadUnicodeTextFile( ":/javascript/book_view_current_location.js" ) ),
      m_CaretLocationUpdate( QString() ),
      c_jQuery(           Utility::ReadUnicodeTextFile( ":/javascript/jquery-1.6.2.min.js"           ) ),
      c_jQueryScrollTo(   Utility::ReadUnicodeTextFile( ":/javascript/jquery.scrollTo-1.4.2-min.js"  ) ),
      c_jQueryWrapSelection( Utility::ReadUnicodeTextFile( ":/javascript/jquery.wrapSelection.js"    ) ),
      m_isLoadFinished(false),
      m_pendingLoadCount(0)
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
    connect(page(), SIGNAL(loadFinished(bool)), this, SLOT(WebPageJavascriptOnLoad()));
}

QSize BookViewPreview::sizeHint() const
{
    return QSize(16777215, 16777215);
}

void BookViewPreview::CustomSetDocument(const QString &path, const QString &html)
{
    m_pendingLoadCount += 1;
    m_isLoadFinished = false;
    setContent(html.toUtf8(), "application/xhtml+xml", QUrl::fromLocalFile(path));
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

bool BookViewPreview::FindNext(const QString &search_regex,
                              Searchable::Direction search_direction,
                              bool check_spelling,
                              bool ignore_selection_offset,
                              bool wrap
                             )
{
    Q_UNUSED(check_spelling)

    bool found = false;
    if (ignore_selection_offset) {
        ScrollToTop();
    }

    // We can't handle a regex so remove the regex code.
    QString search_text = search_regex;
    // Remove the case insensitive parameter.
    search_text = search_text.remove(0, 4);
    search_text = search_text.replace(QRegExp("\\([^\\])"), "\1");
    search_text = search_text.replace("\\\\", "\\");

    QWebPage::FindFlags flags;
    if (search_direction == Searchable::Direction_Up) {
        flags  |= QWebPage::FindBackward;
    }

    found = findText(search_text, flags);

    if (!found && wrap) {
        flags |= QWebPage::FindWrapsAroundDocument;
        found = findText(search_text, flags);

        if (found) {
            ShowWrapIndicator(this);
        }
    }

    return found;
}

int BookViewPreview::Count(const QString &search_regex, bool check_spelling)
{
    return 0;
}

bool BookViewPreview::ReplaceSelected(const QString &search_regex, const QString &replacement, Searchable::Direction direction, bool check_spelling)
{
    QMessageBox::critical(this, tr("Unsupported"), tr("Replace is not supported in this view. Switch to Code View."));
    return false;
}

int BookViewPreview::ReplaceAll(const QString &search_regex, const QString &replacement, bool check_spelling)
{
    QMessageBox::critical(this, tr("Unsupported"), tr("Replace All for the current file is not supported in this view. Switch to Code View."));
    return 0;
}


QString BookViewPreview::GetSelectedText()
{
    QString javascript = "window.getSelection().toString();";
    return EvaluateJavascript(javascript).toString();
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

//   We need to make sure that the Book View has focus,
// but just calling setFocus isn't enough because Nokia
// did a terrible job integrating Webkit. So we first
// have to steal focus away, and then give it back.
//   If we don't steal focus first, then the QWebView
// can have focus (and its QWebFrame) and still not
// really have it (no blinking cursor).
//   We also still need to attempt to GrabFocus even
// when shown as a Preview page (even though no cursor
// is shown) or else the QStackWidget will explode on
// Windows when switching to another tab when it tries
// to determine where the previous focus was.
void BookViewPreview::GrabFocus()
{
    qobject_cast<QWidget *>(parent())->setFocus();
    QWebView::setFocus();
}

void BookViewPreview::WebPageJavascriptOnLoad()
{
    page()->mainFrame()->evaluateJavaScript( c_jQuery         );
    page()->mainFrame()->evaluateJavaScript( c_jQueryScrollTo );
    page()->mainFrame()->evaluateJavaScript( c_jQueryWrapSelection );
    
    m_pendingLoadCount -= 1;
    QTimer::singleShot(0, this, SLOT(executeCaretUpdateInternal()));
}

QString BookViewPreview::GetElementSelectingJS_NoTextNodes( const QList< ViewEditor::ElementIndex > &hierarchy ) const
{
    // TODO: see if replacing jQuery with pure JS will speed up
    // caret location scrolling... note the children()/contents() difference:
    // children() only considers element nodes, contents() considers text nodes too.

    QString element_selector = "$('html')";

    for ( int i = 0; i < hierarchy.count() - 1; ++i )
    {
        if ( hierarchy[ i ].index > 0 ) {
            element_selector.append( QString( ".children().eq(%1)" ).arg( hierarchy[ i ].index ) );
        }
    }

    element_selector.append( ".get(0)" );

    return element_selector;
}

QList< ViewEditor::ElementIndex > BookViewPreview::GetCaretLocation()
{
    // The location element hierarchy encoded in a string
    QString location_string = EvaluateJavascript( c_GetCaretLocation ).toString();
    QStringList elements    = location_string.split( ",", QString::SkipEmptyParts );
    // We remove the very last element because this is a zero location that causes
    // issues when switching to PV
    elements.removeLast();

    QList< ElementIndex > caret_location;

    foreach( QString element, elements )
    {
        ElementIndex new_element;

        new_element.name  = element.split( " " )[ 0 ];
        new_element.index = element.split( " " )[ 1 ].toInt();

        caret_location.append( new_element );
    }
    return caret_location;
}

void BookViewPreview::StoreCaretLocationUpdate( const QList< ViewEditor::ElementIndex > &hierarchy )
{
    QString caret_location = "var element = " + GetElementSelectingJS_NoTextNodes( hierarchy ) + ";";

    // We scroll to the element and center the screen on it
    QString scroll = "var from_top = window.innerHeight / 2;"
                     "$.scrollTo( element, 0, {offset: {top:-from_top, left:0 } } );";

    m_CaretLocationUpdate = caret_location + scroll + SET_CURSOR_JS;
}

bool BookViewPreview::ExecuteCaretUpdate()
{
    // Currently certain actions in Sigil result in a document being loaded multiple times
	// in response to the signals. Only proceed with moving the caret if all loads are finished. 
    if ( m_pendingLoadCount > 0 ) {
        return false;
    }
    // If there is no caret location update pending... 
    if ( m_CaretLocationUpdate.length() == 0 ) {
        return false;
    }

    // run it...
    EvaluateJavascript( m_CaretLocationUpdate );

    // ...and clear the update.
    m_CaretLocationUpdate = ""; 

    return true;
}

