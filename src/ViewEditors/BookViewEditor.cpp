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

#include <QApplication>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QEvent>
#include <QtCore/QTimer>
#include <QtCore/QSignalMapper>
#include <QtGui/QClipboard>
#include <QtCore/QMimeData>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QTextEdit>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QShortcut>
#include <QtGui/QTextDocument>
#include <QtWebKit/QWebElement>
#include <QtWebKit/QWebSettings>
#include <QtWebKitWidgets/QWebFrame>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "BookManipulation/Book.h"
#include "BookManipulation/CleanSource.h"
#include "Dialogs/ClipEditor.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "Misc/OpenExternally.h"
#include "PCRE/PCRECache.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "ViewEditors/BookViewEditor.h"

const int PROGRESS_BAR_MINIMUM_DURATION = 1000;

const QString BREAK_TAG_INSERT    = "<hr class=\"sigil_split_marker\" />";
const QString XML_NAMESPACE_CRUFT = " xmlns=\"http://www.w3.org/1999/xhtml\"";
const QString WEBKIT_BODY_STYLE_CRUFT = " style=\"word-wrap: break-word; -webkit-nbsp-mode: space; -webkit-line-break: after-white-space; \"";

const QString DOC_PREFIX = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>\n"
                           "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
                           "  \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n"
                           "<html xmlns=\"http://www.w3.org/1999/xhtml\">";

const QString REMOVE_XML_TAG = "<\\?xml.*>";
const QString REMOVE_DOCTYPE_TAG = "<!DOCTYPE.*>";
const QString REMOVE_HTML_TAG = "<html.*>";

const QString REPLACE_SPANS = "<span class=\"SigilReplace_\\d*\"( id=\"SigilReplace_\\d*\")*>";

const QString XLINK_NAMESPACE = "http://www.w3.org/1999/xlink";
const QString XHTML_NAMESPACE = "http://www.w3.org/1999/xhtml";

/**
 * The JavaScript source code for getting a string representation
 * of the "body" tag (without the children).
 */
static const QString GET_BODY_TAG_HTML = "new XMLSerializer().serializeToString( document.body.cloneNode(false) );";


BookViewEditor::BookViewEditor(QWidget *parent)
    :
    BookViewPreview(parent),
    m_WebPageModified(false),
    m_clipMapper(new QSignalMapper(this)),
    m_OpenWithContextMenu(*new QMenu(this)),
    m_Paste1(*(new QShortcut(QKeySequence(QKeySequence::Paste), this, 0, 0, Qt::WidgetShortcut))),
    // Old style windows paste.
    m_Paste2(*(new QShortcut(QKeySequence(Qt::ShiftModifier + Qt::Key_Insert), this, 0, 0, Qt::WidgetShortcut))),
    m_PageUp(*(new QShortcut(QKeySequence(QKeySequence::MoveToPreviousPage), this, 0, 0, Qt::WidgetShortcut))),
    m_PageDown(*(new QShortcut(QKeySequence(QKeySequence::MoveToNextPage), this, 0, 0, Qt::WidgetShortcut))),
    m_ScrollOneLineUp(*(new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Up), this, 0, 0, Qt::WidgetShortcut))),
    m_ScrollOneLineDown(*(new QShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Down), this, 0, 0, Qt::WidgetShortcut))),
    c_GetSegmentHTML(Utility::ReadUnicodeTextFile(":/javascript/get_segment_html.js")),
    c_FormatBlock(Utility::ReadUnicodeTextFile(":/javascript/format_block.js")),
    c_GetAncestor(Utility::ReadUnicodeTextFile(":/javascript/get_ancestor.js")),
    c_GetAncestorAttribute(Utility::ReadUnicodeTextFile(":/javascript/get_ancestor_attribute.js")),
    c_SetAncestorAttribute(Utility::ReadUnicodeTextFile(":/javascript/set_ancestor_attribute.js"))
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);
    page()->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    page()->settings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, false);
    page()->settings()->setAttribute(QWebSettings::ZoomTextOnly, true);
    SettingsStore settings;
    SetCurrentZoomFactor(settings.zoomWeb());
    installEventFilter(this);
    CreateContextMenuActions();
    ConnectSignalsToSlots();
}

BookViewEditor::~BookViewEditor()
{
    if (m_Undo) {
        delete m_Undo;
        m_Undo = 0;
    }

    if (m_Redo) {
        delete m_Redo;
        m_Redo = 0;
    }

    if (m_Cut) {
        delete m_Cut;
        m_Cut = 0;
    }

    if (m_Copy) {
        delete m_Copy;
        m_Copy = 0;
    }

    if (m_CopyImage) {
        delete m_CopyImage;
        m_CopyImage = 0;
    }

    if (m_Paste) {
        delete m_Paste;
        m_Paste = 0;
    }

    if (m_SelectAll) {
        delete m_SelectAll;
        m_SelectAll = 0;
    }

    if (m_OpenWith) {
        delete m_OpenWith;
        m_OpenWith = 0;
    }

    if (m_OpenWithEditor) {
        delete m_OpenWithEditor;
        m_OpenWithEditor = 0;
    }

    if (m_SaveAs) {
        delete m_SaveAs;
        m_SaveAs = 0;
    }

    if (m_InsertFile) {
        delete m_InsertFile;
        m_InsertFile = 0;
    }

    if (m_InspectElement) {
        delete m_InspectElement;
        m_InspectElement = 0;
    }
}


void BookViewEditor::CustomSetDocument(const QString &path, const QString &html)
{
    m_isLoadFinished = false;
    m_path = path;
    BookViewPreview::CustomSetDocument(m_path, html);
    page()->setContentEditable(true);
    SetWebPageModified(false);
    emit PageOpened();
}

QString BookViewEditor::GetHtml()
{
    RemoveWebkitCruft();
    // Set the xml tag here rather than let Tidy do it.
    // This prevents false mismatches with the cache later on.
    QString html_from_Qt = page()->mainFrame()->toHtml();
    html_from_Qt = RemoveBookViewReplaceSpans(html_from_Qt);
    // Convert nbsp to entity because it cannot be seen and there are issues
    // where CV will remove them if they are a single character.
    html_from_Qt = CleanSource::CharToEntity(html_from_Qt);
    // Make sure body always has text - primarily from Split Section.
    QRegularExpression empty_body_search("<body>\\s</body>");
    QRegularExpressionMatch empty_body_search_mo = empty_body_search.match(html_from_Qt);
    int empty_body_tag_start = empty_body_search_mo.capturedStart();
    if (empty_body_tag_start != -1) {
        int empty_body_tag_end = empty_body_tag_start + QString("<body>").length();
        html_from_Qt.insert(empty_body_tag_end, "\n  <p>&#160;</p>");
    };

    return html_from_Qt;
}

#if 0
QString BookViewEditor::GetXHtml11()
{
    return GetHtml();
}

QString BookViewEditor::GetHtml5()
{
    return GetHtml();
}
#endif

bool BookViewEditor::InsertHtml(const QString &html)
{
    return ExecCommand("insertHTML", html);
}

void BookViewEditor::SetZoomFactor(float factor)
{
    SettingsStore settings;
    settings.setZoomWeb(factor);
    SetCurrentZoomFactor(factor);
    Zoom();
    emit ZoomFactorChanged(factor);
}


QString BookViewEditor::SplitSection()
{
    QString head     = page()->mainFrame()->documentElement().findFirst("head").toOuterXml();
    QString body_tag = EvaluateJavascript(GET_BODY_TAG_HTML).toString();
    QString segment  = EvaluateJavascript(c_GetBlock % c_GetSegmentHTML).toString();
    emit contentsChangedExtra();
    // The <body> tag returned from above will have a closing </body>
    // element. With Tidy turned on, the old code got away with this, but
    // with Tidy turned off it goes horribly wrong. We also want to remove
    // some other webkit cruft that isn't required.
    body_tag = body_tag.remove(WEBKIT_BODY_STYLE_CRUFT).remove("</body>");
    // In addition we strip Webkit namespace cruft out of the above text.
    // Every element will have an xmlns element on it which we want removed.
    QString new_html = QString(DOC_PREFIX)
                       .append(head.remove(XML_NAMESPACE_CRUFT))
                       .append(body_tag.remove(XML_NAMESPACE_CRUFT))
                       .append(segment.remove(XML_NAMESPACE_CRUFT))
                       .append("</body></html>");

    return new_html;
}

bool BookViewEditor::IsModified()
{
    return m_WebPageModified;
}

void BookViewEditor::ResetModified()
{
    SetWebPageModified(false);
}

void BookViewEditor::Undo()
{
    page()->triggerAction(QWebPage::Undo);
}

void BookViewEditor::Redo()
{
    page()->triggerAction(QWebPage::Redo);
}

// Overridden so we can emit the FocusLost() signal.
void BookViewEditor::focusOutEvent(QFocusEvent *event)
{
    emit FocusLost(this);
    QWebView::focusOutEvent(event);
}

void BookViewEditor::EmitPageUpdated()
{
    emit PageUpdated();
}

QString BookViewEditor::GetSelectedText()
{
    QString javascript = "window.getSelection().toString();";
    return EvaluateJavascript(javascript).toString();
}

void BookViewEditor::SetUpFindForSelectedText(const QString &search_regex)
{
    // Nothing to do for Book View
}

void BookViewEditor::TextChangedFilter()
{
    emit textChanged();
}

bool BookViewEditor::ExecCommand(const QString &command)
{
    if (m_isLoadFinished) {
        QString javascript = QString("document.execCommand( '%1', false, null)").arg(EscapeJSString(command));
        return EvaluateJavascript(javascript).toBool();
    }

    return false;
}

bool BookViewEditor::ExecCommand(const QString &command, const QString &parameter)
{
    QString javascript = QString("document.execCommand( '%1', false, '%2' )")
                         .arg(EscapeJSString(command))
                         .arg(EscapeJSString(parameter));
    return EvaluateJavascript(javascript).toBool();
}

bool BookViewEditor::QueryCommandState(const QString &command)
{
    QString javascript = QString("document.queryCommandState( '%1', false, null)").arg(EscapeJSString(command));
    return EvaluateJavascript(javascript).toBool();
}

QString BookViewEditor::EscapeJSString(const QString &string)
{
    QString new_string(string);
    /* \ -> \\ */
    // " -> \"
    // ' -> \'
    return new_string.replace("\\", "\\\\").replace("\"", "\\\"").replace("'", "\\'");
}

void BookViewEditor::ScrollByLine(bool down)
{
    // This is an educated guess at best since QWebView is not
    // using the widget font but whatever font QWebView feels like using.
    int line_height = qRound(fontMetrics().height() * textSizeMultiplier());
    ScrollByNumPixels(line_height, down);
}

void BookViewEditor::ScrollByNumPixels(int pixel_number, bool down)
{
    Q_ASSERT(pixel_number != 0);
    int current_scroll_offset = page()->mainFrame()->scrollBarValue(Qt::Vertical);
    int scroll_maximum        = page()->mainFrame()->scrollBarMaximum(Qt::Vertical);
    int new_scroll_Y = down ? current_scroll_offset + pixel_number : current_scroll_offset - pixel_number;
    // qBound(min, ours, max) limits the value to the range
    new_scroll_Y     = qBound(0, new_scroll_Y, scroll_maximum);
    page()->mainFrame()->setScrollBarValue(Qt::Vertical, new_scroll_Y);
}

void BookViewEditor::PageUp()
{
    // Scroll by a bit less than the full height to be sure the user does not miss anything.
    ScrollByNumPixels(height() - 40, false);
    QTimer::singleShot(0, this, SLOT(ClickAtTopLeft()));
}

void BookViewEditor::PageDown()
{
    ScrollByNumPixels(height() - 40, true);
    QTimer::singleShot(0, this, SLOT(ClickAtTopLeft()));
}

void BookViewEditor::ClickAtTopLeft()
{
    // As Qt 4.8.3 is still buggy when it comes to page up/down when in edit mode, we simulate the scrolling
    // by hooking into the PageUp and PageDown keys above, and then send a mouse click event in the top left
    // to ensure the caret gets moved to the new position.
    QPoint topLeft = QPoint(0, 30);
    QMouseEvent *mouseDownEvent = new QMouseEvent(QEvent::MouseButtonPress, topLeft, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QMouseEvent *mouseUpEvent = new QMouseEvent(QEvent::MouseButtonRelease, topLeft, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QKeyEvent *homeEvent = new QKeyEvent(QEvent::KeyPress, Qt::Key_Home, Qt::NoModifier);
    // We need to disconnect our link clicked events in case our mouse position where we simulate a click
    // happens to be right on top of a link in the document.
    disconnect(page(), SIGNAL(linkClicked(const QUrl &)), this, SIGNAL(LinkClicked(const QUrl &)));
    QApplication::postEvent(this, mouseDownEvent);
    QApplication::postEvent(this, mouseUpEvent);
    QApplication::postEvent(this, homeEvent);
    QApplication::processEvents();
    connect(page(), SIGNAL(linkClicked(const QUrl &)), this, SIGNAL(LinkClicked(const QUrl &)));
}

void BookViewEditor::ScrollOneLineUp()
{
    ScrollByLine(false);
}

void BookViewEditor::ScrollOneLineDown()
{
    ScrollByLine(true);
}

void BookViewEditor::RemoveWebkitCruft()
{
    QWebElementCollection collection = page()->mainFrame()->findAllElements(".Apple-style-span");
    foreach(QWebElement element, collection) {
        element.toggleClass("Apple-style-span");
    }
    collection = page()->mainFrame()->findAllElements(".webkit-indent-blockquote");
    foreach(QWebElement element, collection) {
        element.toggleClass("webkit-indent-blockquote");
    }
    QWebElement body_tag =  page()->mainFrame()->findFirstElement("body");
    // Removing junk webkit styles
    body_tag.setStyleProperty("word-wrap", "");
    body_tag.setStyleProperty("-webkit-nbsp-mode", "");
    body_tag.setStyleProperty("-webkit-line-break", "");

    // Banish the irritating <body style=""> tags
    if (body_tag.attribute("style", "none") == "") {
        body_tag.removeAttribute("style");
    }
}

QString BookViewEditor::RemoveBookViewReplaceSpans(const QString &source)
{
    QRegularExpression replace_spans(REPLACE_SPANS, QRegularExpression::InvertedGreedinessOption);
    QRegularExpression span_open_or_close("<\\s*(/)*\\s*span\\s*>", QRegularExpression::InvertedGreedinessOption);
    QString newsource = "";
    int left_pos = 0;

    QRegularExpressionMatch replace_spans_mo = replace_spans.match(source);
    while (replace_spans_mo.hasMatch()) {
        int index = replace_spans_mo.capturedStart();
        // Append the text between the last capture and this one.
        newsource.append(source.mid(left_pos, index - left_pos));
        // Advance past the captured opening tag.
        index += replace_spans_mo.capturedLength();

        QRegularExpressionMatch span_open_or_close_mo = span_open_or_close.match(source, index);
        int next_span_tag = span_open_or_close_mo.capturedStart();

        // next_span_tag now points to the start of the closing tag of the span we're removing.
        // Append the source from the end of the span tag to the start of the closing tag
        newsource.append(source.mid(index, next_span_tag - index));
        // Move left_pos past the closing tag and search for another span to remove.
        left_pos = next_span_tag + span_open_or_close_mo.capturedLength();
        replace_spans_mo = replace_spans.match(source, left_pos);
    }

    // Append the rest of the source after all the spans have been removed.
    newsource.append(source.mid(left_pos));

    // It's possible that we might have replace spans nested within each other,
    // so go back to the start and check again, and recurse if found.
    if (newsource.indexOf(replace_spans) != -1) {
        newsource = RemoveBookViewReplaceSpans(newsource);
    }

    return newsource;
}

void BookViewEditor::SetWebPageModified(bool modified)
{
    m_WebPageModified = modified;
}

void BookViewEditor::FormatBlock(const QString &element_name, bool preserve_attributes)
{
    if (element_name.isEmpty()) {
        return;
    }

    QString preserve = preserve_attributes ? "true" : "false";
    QString javascript =  c_GetBlock % c_FormatBlock %
                          "var node = document.getSelection().anchorNode;"
                          "var startNode = get_block( node );"
                          "var element = format_block( startNode, \"" + element_name + "\", " + preserve + " );"
                          "startNode.parentNode.replaceChild( element, startNode );"
                          % SET_CURSOR_JS;
    EvaluateJavascript(javascript);
    emit contentsChangedExtra();
}


QString BookViewEditor::GetCaretElementName()
{
    QString javascript =  "var node = document.getSelection().anchorNode;"
                          "var startNode = get_block( node );"
                          "if (startNode != null) { startNode.nodeName; }";
    return EvaluateJavascript(c_GetBlock % javascript).toString();
}

void BookViewEditor::ApplyCaseChangeToSelection(const Utility::Casing &casing)
{
    // This is going to lose any styles that may have been applied within
    // that selected text.
    QString selected_text = GetSelectedText();
    QString new_text = Utility::ChangeCase(selected_text, casing);

    if (new_text == selected_text) {
        return;
    }

    // The "proper" way to do a replacement of the text (and allow undo) will
    // undoubtedly involve reams of javascript. However there is a quick and
    // dirty alternative, of putting the text on the clipboard and pasting it.
    // However since this will muck around the with clipboard history we need
    // to do a little extra work to disconnect and restore things afterwards.
    emit ClipboardSaveRequest();
    QApplication::clipboard()->setText(new_text);
    paste();
    emit ClipboardRestoreRequest();

    // We will have lost our selection from the paste operation.
    for (int i = 0; i < new_text.length(); i++) {
        page()->triggerAction(QWebPage::SelectPreviousChar);
    }
}

bool BookViewEditor::InsertId(const QString &id)
{
    const QString &element_name = "a";
    const QString &attribute_name = "id";
    return InsertTagAttribute(element_name, attribute_name, id, ANCHOR_TAGS);
}

bool BookViewEditor::InsertHyperlink(const QString &href)
{
    const QString &element_name = "a";
    const QString &attribute_name = "href";
    return InsertTagAttribute(element_name, attribute_name, href, ANCHOR_TAGS);
}

void BookViewEditor::AddToIndex()
{
    QString selected_text = GetSelectedText();

    if (selected_text.isEmpty()) {
        return;
    }

    IndexEditorModel::indexEntry *index = new IndexEditorModel::indexEntry();
    index->pattern = selected_text;
    emit OpenIndexEditorRequest(index);
}

bool BookViewEditor::MarkForIndex(const QString &title)
{
    bool ok = true;
    const QString &element_name = "a";
    const QString &attribute_name = "class";

    if (!InsertTagAttribute(element_name, attribute_name, SIGIL_INDEX_CLASS, ANCHOR_TAGS)) {
        ok = false;
    }

    const QString &second_attribute_name = "title";

    if (!InsertTagAttribute(element_name, second_attribute_name, title, ANCHOR_TAGS, true)) {
        ok = false;
    }

    return ok;
}

bool BookViewEditor::InsertTagAttribute(const QString &element_name, const QString &attribute_name, const QString &attribute_value, const QStringList &tag_list, bool ignore_selection)
{
    QString selected_text = GetSelectedText();

    if (SetAncestorTagAttributeValue(attribute_name, attribute_value, tag_list)) {
        return true;
    }

    // We need to insert a new tag element into the document - cannot insert an empty
    // element or else Qt will push it into a new block. So we will just insert the
    // attribute value as some placeholder text.
    if (selected_text.isEmpty()) {
        selected_text = attribute_value;
    }

    // Just prepend and append the tag pairs to the text
    const QString html = "<" % element_name % " " % attribute_name % "=\"" % attribute_value % "\">" % selected_text % "</" % element_name % ">";
    bool insert_ok = InsertHtml(html);

    // We will have lost our selection from the insert - viewable text hasn't changed
    for (int i = 0; i < selected_text.length(); i++) {
        page()->triggerAction(QWebPage::SelectPreviousChar);
    }

    return insert_ok;
}

QString BookViewEditor::GetAncestorTagAttributeValue(const QString &attribute_name, const QStringList &tag_list)
{
    if (tag_list.isEmpty() || attribute_name.isEmpty()) {
        return QString();
    }

    const QString js = c_GetAncestor % c_GetAncestorAttribute %
                       "var tagNames = [\"" % tag_list.join("\", \"") % "\"];"
                       "var attributeName = '" % attribute_name % "';"
                       "var startNode = document.getSelection().anchorNode;"
                       "get_ancestor_attribute(startNode, tagNames, attributeName);";
    return EvaluateJavascript(js).toString();
}

bool BookViewEditor::SetAncestorTagAttributeValue(const QString &attribute_name, const QString &attribute_value, const QStringList &tag_list)
{
    if (tag_list.isEmpty() || attribute_name.isEmpty()) {
        return false;
    }

    const QString js = c_GetAncestor % c_SetAncestorAttribute %
                       "var tagNames = [\"" % tag_list.join("\", \"") % "\"];"
                       "var attributeName = '" % attribute_name % "';"
                       "var attributeValue = '" % attribute_value % "';"
                       "var startNode = document.getSelection().anchorNode;"
                       "set_ancestor_attribute(startNode, tagNames, attributeName, attributeValue);";
    bool applied = EvaluateJavascript(js).toBool();

    if (applied) {
        emit contentsChangedExtra();
    }

    return applied;
}


void BookViewEditor::cut()
{
    page()->triggerAction(QWebPage::Cut);
}


void BookViewEditor::paste()
{
    QClipboard *clipboard = QApplication::clipboard();

    if (clipboard->mimeData()->hasHtml()) {
        QMessageBox msgBox(QMessageBox::Question,
                           tr("Clipboard contains HTML formatting"),
                           tr("Do you want to paste clipboard data as plain text?"),
                           QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Yes);

        // populate the detailed text window - by HTML not by the text
        msgBox.setDetailedText(clipboard->mimeData()->html());

        // show message box
        switch (msgBox.exec()) {
            case QMessageBox::Yes:
                page()->triggerAction(QWebPage::PasteAndMatchStyle);
                break;
            case QMessageBox::No:
                page()->triggerAction(QWebPage::Paste);
                break;
            default:
                // Cancel was clicked - do nothing
                break;
        }
    } else {
        page()->triggerAction(QWebPage::Paste);
    }
}

void BookViewEditor::copyImage()
{
    const QVariant &data = m_CopyImage->data();

    if (data.isValid()) {
        const QUrl &resourceUrl = data.toUrl();
        const QImage img(resourceUrl.toLocalFile());
        QApplication::clipboard()->setImage(img);
    }
}

void BookViewEditor::selectAll()
{
    page()->triggerAction(QWebPage::SelectAll);
}

void BookViewEditor::openImage()
{
    const QVariant &data = m_Open->data();

    if (data.isValid()) {
        const QUrl &url = data.toUrl();
        emit LinkClicked(url);
    }
}

void BookViewEditor::insertFile()
{
    emit InsertFile();
}

void BookViewEditor::openWith()
{
    const QVariant &data = m_OpenWith->data();

    if (data.isValid()) {
        const QUrl &resourceUrl = data.toUrl();
        const QString &editorPath = OpenExternally::selectEditorForResourceType((Resource::ResourceType) resourceUrl.port());

        if (!editorPath.isEmpty()) {
            if (OpenExternally::openFile(resourceUrl.toLocalFile(), editorPath)) {
                const QString &pathname = resourceUrl.toString();
                emit InsertedFileOpenedExternally(pathname);
            }
        }
    }
}

void BookViewEditor::saveAs()
{
    const QVariant &data = m_SaveAs->data();

    if (data.isValid()) {
        const QUrl &url = data.toUrl();
        emit InsertedFileSaveAs(url);
    }
}

void BookViewEditor::openWithEditor()
{
    const QVariant &data = m_OpenWithEditor->data();

    if (data.isValid()) {
        const QUrl &resourceUrl = data.toUrl();
        const QString &editorPath = OpenExternally::editorForResourceType((Resource::ResourceType) resourceUrl.port());

        if (OpenExternally::openFile(resourceUrl.toLocalFile(), editorPath)) {
            const QString &pathname = resourceUrl.toString();
            emit InsertedFileOpenedExternally(pathname);
        }
    }
}

void BookViewEditor::PasteText(const QString &text)
{
    InsertHtml(text);
}

bool BookViewEditor::PasteClipNumber(int clip_number)
{
    ClipEditorModel::clipEntry *clip = ClipEditorModel::instance()->GetEntryFromNumber(clip_number);
    if (!clip) {
        return false;
    }

    PasteClipEntry(clip);

    return true;
}

void BookViewEditor::PasteClipEntryFromName(const QString &name)
{
    ClipEditorModel::clipEntry *clip = ClipEditorModel::instance()->GetEntryFromName(name);
    PasteClipEntry(clip);
}

bool BookViewEditor::PasteClipEntries(const QList<ClipEditorModel::clipEntry *> &clips)
{
    bool applied = false;
    foreach(ClipEditorModel::clipEntry * clip, clips) {
        applied = applied || PasteClipEntry(clip);
    }
    return applied;
}

bool BookViewEditor::PasteClipEntry(ClipEditorModel::clipEntry *clip)
{
    if (!clip || clip->text.isEmpty()) {
        return false;
    }

    QString text = clip->text;

    if (text.contains("\\1")) {
        text.replace("\\1", GetSelectedText());
    }

    InsertHtml(text);

    return true;
}

void BookViewEditor::EmitInspectElement()
{
    StoreCurrentCaretLocation();
    emit BVInspectElement();
}

void BookViewEditor::OpenContextMenu(const QPoint &point)
{
    if (!SuccessfullySetupContextMenu(point)) {
        return;
    }

    m_ContextMenu.exec(mapToGlobal(point));
    m_ContextMenu.clear();
}

bool BookViewEditor::SuccessfullySetupContextMenu(const QPoint &point)
{
    const QWebFrame *frame = page()->frameAt(point);
    bool has_image = false;

    if (frame) {
        const QWebHitTestResult &hitTest = frame->hitTestContent(point);
        QUrl imageUrl = hitTest.imageUrl();
        QString tag = hitTest.element().tagName().toLower();
        if (!imageUrl.isValid() && (tag == "image" || tag == "video" || tag == "audio" )) {
            // It is "possible" that we are in an SVG image tag which the hitTest does
            // not return sufficient information for directly. Go spelunking for an attribute.
            const QWebElement element = hitTest.element();

            foreach(QString attrib, element.attributeNames(XLINK_NAMESPACE)) {
                if (attrib.toLower() == "href") {
                    QString path = element.attributeNS(XLINK_NAMESPACE, attrib);

                    if (!path.isEmpty()) {
                        imageUrl = QUrl::fromLocalFile(QDir::cleanPath(QFileInfo(m_path).absoluteDir().filePath(path)));
                    }

                    break;
                }
            }
        }

        if (imageUrl.isValid() && imageUrl.isLocalFile()) {
            has_image = true;
            // Open in image tab
            QString filename = imageUrl.toString();
            filename = filename.right(filename.length() - filename.lastIndexOf("/") - 1);
            m_Open->setData(imageUrl);
            m_Open->setText(tr("Open Tab For") + " \"" + filename + "\"");
            m_ContextMenu.addAction(m_Open);
            // Open With
            Resource::ResourceType imageType = Resource::ImageResourceType;

            if (imageUrl.path().toLower().endsWith(".svg")) {
                imageType = Resource::SVGResourceType;
            }

            imageUrl.setPort(imageType);   // "somewhat" ugly, but cheaper than using a QList<QVariant>
            const QString &editorPath = OpenExternally::editorForResourceType(imageType);

            if (editorPath.isEmpty()) {
                m_OpenWithEditor->setData(QVariant::Invalid);
                m_OpenWith->setText(tr("Open With") + "...");
                m_OpenWith->setData(imageUrl);
                m_ContextMenu.addAction(m_OpenWith);
            } else {
                const QString &editorDescription = OpenExternally::editorDescriptionForResourceType(imageType);
                m_OpenWithEditor->setText(editorDescription);
                m_OpenWithEditor->setData(imageUrl);
                m_OpenWith->setText(tr("Other Application") + "...");
                m_OpenWith->setData(imageUrl);
                m_ContextMenu.addMenu(&m_OpenWithContextMenu);
            }

            // Save As
            m_SaveAs->setData(imageUrl);
            m_CopyImage->setData(imageUrl);
            m_ContextMenu.addAction(m_SaveAs);
            m_ContextMenu.addSeparator();
        } else {
            // If not an image allow insert - otherwise cursor is not where you expect.
            AddClipContextMenu(&m_ContextMenu);
            m_ContextMenu.addSeparator();
            m_ContextMenu.addAction(m_InsertFile);
        }
    }

    m_ContextMenu.addSeparator();
    m_ContextMenu.addAction(m_InspectElement);

    m_ContextMenu.addSeparator();
    m_ContextMenu.addAction(m_Undo);
    m_ContextMenu.addAction(m_Redo);
    m_ContextMenu.addSeparator();
    m_ContextMenu.addAction(m_Cut);
    m_ContextMenu.addAction(m_Copy);
    m_ContextMenu.addAction(m_CopyImage);
    m_ContextMenu.addAction(m_Paste);
    m_ContextMenu.addSeparator();
    m_ContextMenu.addAction(m_SelectAll);
    bool has_selection = !selectedText().isEmpty();
    m_Cut->setEnabled(has_selection);
    m_Copy->setEnabled(has_selection);
    m_CopyImage->setEnabled(has_image);
    return true;
}


void BookViewEditor::AddClipContextMenu(QMenu *menu)
{
    QAction *topAction = 0;

    if (!menu->actions().isEmpty()) {
        topAction = menu->actions().at(0);
    }

    QMenu *clips_menu = new QMenu(this);
    clips_menu->setTitle(tr("Clips"));

    if (topAction) {
        menu->insertMenu(topAction, clips_menu);
    } else {
        menu->addMenu(clips_menu);
    }

    CreateMenuEntries(clips_menu, 0, ClipEditorModel::instance()->invisibleRootItem());

    QAction *saveClipAction = new QAction(tr("Add To Clips") + "...", menu);

    if (!topAction) {
        menu->addAction(saveClipAction);
    } else {
        menu->insertAction(topAction, saveClipAction);
    }

    connect(saveClipAction, SIGNAL(triggered()), this , SLOT(SaveClipAction()));
    saveClipAction->setEnabled(!GetSelectedText().isEmpty());

    if (topAction) {
        menu->insertSeparator(topAction);
    }
}

bool BookViewEditor::CreateMenuEntries(QMenu *parent_menu, QAction *topAction, QStandardItem *item)
{
    QAction *clipAction = 0;
    QMenu *group_menu = parent_menu;

    if (!item) {
        return false;
    }

    if (!item->text().isEmpty()) {
        // If item has no children, add entry to the menu, else create menu
        if (!item->data().toBool()) {
            clipAction = new QAction(item->text(), this);
            connect(clipAction, SIGNAL(triggered()), m_clipMapper, SLOT(map()));
            m_clipMapper->setMapping(clipAction, ClipEditorModel::instance()->GetFullName(item));

            if (!topAction) {
                parent_menu->addAction(clipAction);
            } else {
                parent_menu->insertAction(topAction, clipAction);
            }
        } else {
            group_menu = new QMenu(this);
            group_menu->setTitle(item->text());

            if (topAction) {
                parent_menu->insertMenu(topAction, group_menu);
            } else {
                parent_menu->addMenu(group_menu);
            }

            topAction = 0;
        }
    }

    // Recursively add entries for children
    for (int row = 0; row < item->rowCount(); row++) {
        CreateMenuEntries(group_menu, topAction, item->child(row, 0));
    }

    return item->rowCount() > 0;
}

void BookViewEditor::SaveClipAction()
{
    ClipEditorModel::clipEntry clip;
    clip.name = "Unnamed Entry";
    clip.is_group = false;
    clip.text = GetSelectedText();
    emit OpenClipEditorRequest(&clip);
}

void BookViewEditor::mouseReleaseEvent(QMouseEvent *event)
{
    // Propagate to base class
    QWebView::mouseReleaseEvent(event);
    emit PageClicked();
}

void BookViewEditor::keyReleaseEvent(QKeyEvent *event)
{
    // Propagate to base class
    BookViewPreview::keyReleaseEvent(event);
    emit PageUpdated();
}


void BookViewEditor::CreateContextMenuActions()
{
    m_InsertFile = new QAction(tr("Insert File") + "...", this);
    m_Undo      = new QAction(tr("Undo"),         this);
    m_Redo      = new QAction(tr("Redo"),         this);
    m_Cut       = new QAction(tr("Cut"),         this);
    m_Copy      = new QAction(tr("Copy"),        this);
    m_CopyImage = new QAction(tr("Copy Image"),  this);
    m_Paste     = new QAction(tr("Paste"),       this);
    m_SelectAll = new QAction(tr("Select All"),  this);
    m_Open           = new QAction(tr("Open"),  this);
    m_OpenWithEditor = new QAction("",          this);
    m_OpenWith       = new QAction(tr("Open With") + "...",  this);
    m_SaveAs         = new QAction(tr("Save As") + "...",  this);
    m_OpenWithContextMenu.setTitle(tr("Open With"));
    m_OpenWithContextMenu.addAction(m_OpenWithEditor);
    m_OpenWithContextMenu.addAction(m_OpenWith);
    m_InspectElement = new QAction(tr("Inspect Element") + "...", this);
}

void BookViewEditor::ConnectSignalsToSlots()
{
    connect(&m_Paste1,            SIGNAL(activated()), this, SLOT(paste()));
    connect(&m_Paste2,            SIGNAL(activated()), this, SLOT(paste()));
    connect(&m_PageUp,            SIGNAL(activated()), this, SLOT(PageUp()));
    connect(&m_PageDown,          SIGNAL(activated()), this, SLOT(PageDown()));
    connect(&m_ScrollOneLineUp,   SIGNAL(activated()), this, SLOT(ScrollOneLineUp()));
    connect(&m_ScrollOneLineDown, SIGNAL(activated()), this, SLOT(ScrollOneLineDown()));
    connect(this,   SIGNAL(contentsChangedExtra()),  page(), SIGNAL(contentsChanged()));
    connect(page(), SIGNAL(contentsChanged()),      this,   SIGNAL(textChanged()));
    connect(page(), SIGNAL(selectionChanged()),      this,   SIGNAL(selectionChanged()));
    connect(page(), SIGNAL(contentsChanged()),      this,   SLOT(SetWebPageModified()));
    connect(m_InsertFile,     SIGNAL(triggered()),  this, SLOT(insertFile()));
    connect(m_Undo,           SIGNAL(triggered()),  this, SLOT(Undo()));
    connect(m_Redo,           SIGNAL(triggered()),  this, SLOT(Redo()));
    connect(m_Cut,            SIGNAL(triggered()),  this, SLOT(cut()));
    connect(m_Copy,           SIGNAL(triggered()),  this, SLOT(copy()));
    connect(m_CopyImage,      SIGNAL(triggered()),  this, SLOT(copyImage()));
    connect(m_Paste,          SIGNAL(triggered()),  this, SLOT(paste()));
    connect(m_SelectAll,      SIGNAL(triggered()),  this, SLOT(selectAll()));
    connect(m_Open,           SIGNAL(triggered()),  this, SLOT(openImage()));
    connect(m_OpenWith,       SIGNAL(triggered()),  this, SLOT(openWith()));
    connect(m_OpenWithEditor, SIGNAL(triggered()),  this, SLOT(openWithEditor()));
    connect(m_SaveAs,         SIGNAL(triggered()),  this, SLOT(saveAs()));
    connect(m_clipMapper, SIGNAL(mapped(const QString &)), this, SLOT(PasteClipEntryFromName(const QString &)));
    connect(m_InspectElement,      SIGNAL(triggered()),  this, SLOT(EmitInspectElement()));
    connect(page(), SIGNAL(contentsChanged()), this, SLOT(EmitPageUpdated()));
}
