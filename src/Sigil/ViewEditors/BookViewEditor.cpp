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

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QMessageBox>
#include <QtGui/QTextDocument>
#include <QtWebKit/QWebFrame>

#include "BookManipulation/Book.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "PCRE/PCRECache.h"
#include "sigil_constants.h"
#include "ViewEditors/BookViewEditor.h"

const int PROGRESS_BAR_MINIMUM_DURATION = 1500;

const QString BREAK_TAG_INSERT    = "<hr class=\"sigilChapterBreak\" />";
// %1 = CKE path.
// %2 = Text to load.
// %3 = language
// %4 = customConfig : '/custom/ckeditor_config.js'.
const QString CKE_BASE =
    "<html>"
    "<head>"
    "    <script type=\"text/javascript\" src=\"%1/ckeditor.js\"></script>"
    "</head>"
    "<body>"
    "    <form action=\"sample_posteddata.php\" method=\"post\">"
    "        <textarea id=\"editor1\" name=\"editor1\">%2</textarea>"
    "        <script type=\"text/javascript\">"
    "            CKEDITOR.replace('editor1', { fullPage: true, startupFocus: true, extraPlugins: 'onchange,docprops', language: '%3', %4 });"
    "            CKEDITOR.on('instanceReady', function(e) { e.editor.execCommand('maximize'); });"
    "            CKEDITOR.instances[\"editor1\"].on('change', function() { BookViewEditor.TextChangedFilter(); });"
    "        </script>"
    "    </form>"
    "</body>"
    "</html>";

BookViewEditor::BookViewEditor(QWidget *parent)
    : 
    BookViewPreview(parent)
{
}

void BookViewEditor::CustomSetDocument(const QString &path, const QString &html)
{
    m_isLoadFinished = true;

    // Enable our link filter.
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    page()->mainFrame()->addToJavaScriptWindowObject("BookViewEditor", this);
    page()->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);

    connect(page(), SIGNAL(loadProgress(int)), this, SLOT(UpdateFinishedState(int)));
    connect(page(), SIGNAL(linkClicked(const QUrl&)), this, SLOT( LinkClickedFilter(const QUrl&)));
    connect(this, SIGNAL(FilteredLinkClicked(const QUrl&)), this->parent()->parent(), SIGNAL(LinkClicked(const QUrl&)));

    QString cke_path;
#ifdef Q_WS_MAC
    cke_path = QCoreApplication::applicationDirPath() + "/../ckeditor";
#else
    cke_path = QCoreApplication::applicationDirPath() + "/ckeditor";
#endif
// On *nix we have an installation path we need to check too.
#ifdef Q_WS_X11
    if (cke_path.isEmpty() || !QDir(cke_path).exists()) {
        cke_path = QCoreApplication::applicationDirPath() + "/../share/" + QCoreApplication::applicationName().toLower() + "/ckeditor";
    }
#endif

    QString cke_settings;
    QString cke_setting_path = QDesktopServices::storageLocation(QDesktopServices::DataLocation) + "/ckeditor_config.js";
    if (QFile::exists(cke_setting_path)) {
        cke_settings = QString("customConfig: '%1'").arg(cke_setting_path);
    }

    SettingsStore settings;
    QString base = CKE_BASE.arg(QDir::fromNativeSeparators(cke_path)).arg(Qt::escape(html)).arg(settings.uiLanguage()).arg(cke_settings);
    setHtml(base, QUrl::fromLocalFile(path));
}

void BookViewEditor::ScrollToFragment(const QString &fragment)
{
    if (fragment.isEmpty()) {
        ScrollToTop();
        return;
    }

    QString scroll = "var documentWrapper = document.getElementById(\"editor1\").document;"
        "var documentNode = documentWrapper['$'];"
        "var element = documentNode.getElementById(\"" % fragment % "\");"
        "var from_top = window.innerHeight / 2;"
        "documentNode.scrollTo(element, 0, {offset: {top:-from_top, left:0 } });";

    EvaluateJavascript(scroll % SET_CURSOR_JS);
}

void BookViewEditor::ScrollToFragmentAfterLoad(const QString &fragment)
{
    if (fragment.isEmpty()) {
        return;
    }

    QString scroll = "var documentWrapper = document.getElementById(\"editor1\").document;"
        "var documentNode = documentWrapper['$'];"
        "var element = documentNode.getElementById(\"" % fragment % "\");"
        "var from_top = window.innerHeight / 2;"
        "documentNode.scrollTo(element, 0, {offset: {top:-from_top, left:0 } });";
    QString javascript = "window.addEventListener('load', GoToFragment, false);"
        "function GoToFragment() { " % scroll % SET_CURSOR_JS % "}";

    EvaluateJavascript(javascript);
}

QString BookViewEditor::GetHtml()
{
    QString command = "var objEditor = CKEDITOR.instances[\"editor1\"]; objEditor.getData();";
    return EvaluateJavascript(command).toString();
}

QString BookViewEditor::GetXHtml11()
{
    return GetHtml();
}

QString BookViewEditor::GetHtml5()
{
    return GetHtml();
}

QString BookViewEditor::SplitChapter()
{
    return QString();
}

//   We need to make sure that the Book View has focus,
// but just calling setFocus isn't enough because Nokia
// did a terrible job integrating Webkit. So we first
// have to steal focus away, and then give it back.
//   If we don't steal focus first, then the QWebView
// can have focus (and its QWebFrame) and still not
// really have it (no blinking cursor).
void BookViewEditor::GrabFocus()
{
    qobject_cast<QWidget *>(parent())->setFocus();
    QWebView::setFocus();
}

// Overridden so we can emit the FocusLost() signal.
void BookViewEditor::focusOutEvent(QFocusEvent *event)
{
    emit FocusLost(this);
    QWebView::focusOutEvent(event);
}

bool BookViewEditor::FindNext(const QString &search_regex,
                              Searchable::Direction search_direction,
                              bool check_spelling,
                              bool ignore_selection_offset,
                              bool wrap
                             )
                              

{
    return false;
}

int BookViewEditor::Count(const QString &search_regex, bool check_spelling)
{
    return 0;
}

bool BookViewEditor::ReplaceSelected(const QString &search_regex, const QString &replacement, Searchable::Direction direction, bool check_spelling)
{
    QMessageBox::critical(this, tr("Unsupported"), tr("Replace is not supported in Book View at this time.  Switch to Code View."));
    return false;
}

int BookViewEditor::ReplaceAll(const QString &search_regex, const QString &replacement, bool check_spelling)
{
    QMessageBox::critical(this, tr("Unsupported"), tr("Replace All for the current file is not supported in Book View at this time.  Switch to Code View."));
    return 0;
}

QString BookViewEditor::GetSelectedText()
{
    QString javascript = "CKEDITOR.instances[\"editor1\"].getSelection().getSelectedText();";
    return EvaluateJavascript(javascript).toString();
}

void BookViewEditor::SaveCaret()
{
    QString javascript = "CKEDITOR.instances[\"editor1\"].getSelection().getRanges();";
    m_caret = EvaluateJavascript(javascript);
}

void BookViewEditor::RestoreCaret()
{
    QString javascript = QString("CKEDITOR.instances[\"editor1\"].getSelection().selectRanges(%1)").arg(m_caret.toString());
    EvaluateJavascript(javascript);
}

void BookViewEditor::TextChangedFilter()
{
    //m_lastMatch = SPCRE::MatchInfo();
    emit textChanged();
}

