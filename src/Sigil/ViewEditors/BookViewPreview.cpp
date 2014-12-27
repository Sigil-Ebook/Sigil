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
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWebKit/QWebSettings>
#include <QtWebKitWidgets/QWebFrame>

#include "BookManipulation/XercesCppUse.h"
#include "BookManipulation/XhtmlDoc.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "PCRE/PCRECache.h"
#include "sigil_constants.h"
#include "ViewEditors/BookViewPreview.h"
#include "ViewEditors/ViewWebPage.h"

const QString SET_CURSOR_JS =
    "var range = document.createRange();"
    "range.setStart(element, 0);"
    "range.setEnd(element, 0);"
    "var selection = window.getSelection();"
    "selection.removeAllRanges();"
    "selection.addRange(range);";


BookViewPreview::BookViewPreview(QWidget *parent)
    : QWebView(parent),
      c_GetBlock(Utility::ReadUnicodeTextFile(":/javascript/get_block.js")),
      m_isLoadFinished(false),
      m_ContextMenu(*new QMenu(this)),
      m_ViewWebPage(new ViewWebPage(this)),
      c_jQuery(Utility::ReadUnicodeTextFile(":/javascript/jquery-1.6.2.min.js")),
      c_jQueryScrollTo(Utility::ReadUnicodeTextFile(":/javascript/jquery.scrollTo-1.4.2-min.js")),
      c_jQueryWrapSelection(Utility::ReadUnicodeTextFile(":/javascript/jquery.wrapSelection.js")),
      c_GetCaretLocation(Utility::ReadUnicodeTextFile(":/javascript/book_view_current_location.js")),
      c_GetRange(Utility::ReadUnicodeTextFile(":/javascript/get_range.js")),
      c_NewSelection(Utility::ReadUnicodeTextFile(":/javascript/new_selection.js")),
      c_GetParentTags(Utility::ReadUnicodeTextFile(":/javascript/get_parent_tags.js")),
      m_CaretLocationUpdate(QString()),
      m_pendingLoadCount(0),
      m_pendingScrollToFragment(QString())
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    // Set the Zoom factor but be sure no signals are set because of this.
    SettingsStore settings;
    SetCurrentZoomFactor(settings.zoomPreview());
    // use our web page that can be used for debugging javascript
    setPage(m_ViewWebPage);
    // Enable our link filter.
    page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    CreateContextMenuActions();
    ConnectSignalsToSlots();
}

BookViewPreview::~BookViewPreview()
{
    if (m_ViewWebPage != NULL) {
        delete m_ViewWebPage;
        m_ViewWebPage = 0;
    }

    if (m_InspectElement) {
        delete m_InspectElement;
        m_InspectElement = 0;
    }

}

QString BookViewPreview::GetCaretLocationUpdate()
{
    StoreCaretLocationUpdate(GetCaretLocation());
    return m_CaretLocationUpdate;
}

QString BookViewPreview::GetDisplayedCharacters()
{
    page()->triggerAction(QWebPage::SelectAll);
    QString text = selectedText();
    page()->triggerAction(QWebPage::MoveToStartOfDocument);
    page()->triggerAction(QWebPage::SelectNextChar);
    return text;
}

void BookViewPreview::ShowTag()
{
    // Walk up the parent tag element hierarhcy at the caret location appending html
    // for all open tag nodes until we hit the body tag.
    // e.g. <p class='foo'><b>
    const QString &html = EvaluateJavascript(c_GetParentTags).toString();
    emit ShowStatusMessageRequest(html);
}

void BookViewPreview::copy()
{
    page()->triggerAction(QWebPage::Copy);
}

QSize BookViewPreview::sizeHint() const
{
    return QSize(16777215, 16777215);
}

void BookViewPreview::CustomSetDocument(const QString &path, const QString &html)
{
    m_pendingLoadCount += 1;

    if (html.isEmpty()) {
        return;
    }

    // If this is not the very first load of this document, store the caret location
    if (!url().isEmpty()) {
        StoreCurrentCaretLocation();
    }

    m_isLoadFinished = false;
    // If Tidy is turned off, then Sigil will explode if there is no xmlns
    // on the <html> element. So we will silently add it if needed to ensure
    // no errors occur, to allow loading of documents created outside of
    // Sigil as well as catering for section splits etc.
    QString replaced_html = html;
    replaced_html = replaced_html.replace("<html>", "<html xmlns=\"http://www.w3.org/1999/xhtml\">");
    setContent(replaced_html.toUtf8(), "application/xhtml+xml", QUrl::fromLocalFile(path));
}

bool BookViewPreview::IsLoadingFinished()
{
    return m_isLoadFinished;
}

void BookViewPreview::SetZoomFactor(float factor)
{
    SettingsStore settings;
    settings.setZoomPreview(factor);
    SetCurrentZoomFactor(factor);
    Zoom();
    emit ZoomFactorChanged(factor);
}

void BookViewPreview::SetCurrentZoomFactor(float factor)
{
    m_CurrentZoomFactor = factor;
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

void BookViewPreview::mouseReleaseEvent(QMouseEvent *event)
{
    // Propagate to base class
    QWebView::mouseReleaseEvent(event);

    emit GoToPreviewLocationRequest();
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
    if (IsLoadingFinished()) {
        ScrollToFragmentInternal(fragment);
    } else {
        m_pendingScrollToFragment = fragment;
    }
}

void BookViewPreview::ScrollToFragmentInternal(const QString &fragment)
{
    if (fragment.isEmpty()) {
        ScrollToTop();
        return;
    }

    QString caret_location = "var element = document.getElementById(\"" % fragment % "\");";
    QString scroll = "var from_top = window.innerHeight / 2.5;"
                     "$.scrollTo(element, 0, {offset: {top:-from_top, left:0 } });";
    EvaluateJavascript(caret_location % scroll % SET_CURSOR_JS);
}

bool BookViewPreview::FindNext(const QString &search_regex,
                               Searchable::Direction search_direction,
                               bool check_spelling,
                               bool ignore_selection_offset,
                               bool wrap,
                               bool selected_text)
{
    SearchTools search_tools = GetSearchTools();
    return FindNext(search_tools, search_regex, search_direction, check_spelling, ignore_selection_offset, wrap, selected_text);
}

bool BookViewPreview::FindNext(SearchTools &search_tools,
                               const QString &search_regex,
                               Searchable::Direction search_direction,
                               bool check_spelling,
                               bool ignore_selection_offset,
                               bool wrap,
                               bool selected_text
                              )
{
    if (check_spelling) {
        QMessageBox::critical(this, tr("Unsupported"), tr("Spellcheck mode is not supported in Book View at this time.  Switch to Code View."));
        return false;
    }

    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    SPCRE::MatchInfo match_info;
    int start_offset = 0;
    int selection_offset = -1;

    // Get the match info for the direction.
    if (search_direction == Searchable::Direction_Up) {
        if (ignore_selection_offset) {
            selection_offset = search_tools.fulltext.count();
        } else {
            selection_offset = GetSelectionOffset(*search_tools.document, search_tools.node_offsets, search_direction);
        }

        match_info = spcre->getLastMatchInfo(Utility::Substring(0, selection_offset, search_tools.fulltext));
    } else {
        if (ignore_selection_offset) {
            selection_offset = 0;
        } else {
            selection_offset = GetSelectionOffset(*search_tools.document, search_tools.node_offsets, search_direction) - 1;
        }

        match_info = spcre->getFirstMatchInfo(Utility::Substring(selection_offset, search_tools.fulltext.count(), search_tools.fulltext));
        start_offset = selection_offset;
    }

    SPCRE::MatchInfo last_match = match_info;

    if (match_info.offset.first != -1) {
        if (selection_offset < 0) {
            selection_offset = 0;
        }

        if (start_offset < 0) {
            start_offset = 0;
        }

        last_match.offset.first += selection_offset;
        last_match.offset.second += selection_offset;
        SelectRangeInputs input = GetRangeInputs(search_tools.node_offsets, match_info.offset.first + start_offset, match_info.offset.second - match_info.offset.first);
        SelectTextRange(input);
        ScrollToNodeText(*input.start_node, input.start_node_index);
        return true;
    } else if (wrap) {
        if (FindNext(search_regex, search_direction, check_spelling, true, false)) {
            ShowWrapIndicator(this);
            return true;
        }
    }

    return false;
}

int BookViewPreview::Count(const QString &search_regex, Searchable::Direction direction, bool wrap, bool selected_text)
{
    // Spell check not actually used
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    return spcre->getEveryMatchInfo(GetSearchTools().fulltext).count();
}

bool BookViewPreview::ReplaceSelected(const QString &search_regex, const QString &replacement, Searchable::Direction direction, bool keep_selection)
{
    QMessageBox::critical(this, tr("Unsupported"), tr("Replace is not supported in this view. Switch to Code View."));
    return false;
}

int BookViewPreview::ReplaceAll(const QString &search_regex, const QString &replacement, Searchable::Direction direction, bool wrap, bool selected_text)
{
    QMessageBox::critical(this, tr("Unsupported"), tr("Replace All for the current file is not supported in this view. Switch to Code View."));
    return 0;
}


QString BookViewPreview::GetSelectedText()
{
    QString javascript = "window.getSelection().toString();";
    return EvaluateJavascript(javascript).toString();
}

void BookViewPreview::SetUpFindForSelectedText(const QString &search_regex)
{
    // Nothing to do for Book Preview
}

void BookViewPreview::UpdateFinishedState(bool okay)
{
    if (okay) {
        m_isLoadFinished = true;
        emit DocumentLoaded();
    } else {
        m_isLoadFinished = false;
    }
}

void BookViewPreview::HighlightPosition()
{
    page()->setContentEditable(true);
    page()->triggerAction(QWebPage::SelectEndOfBlock);
    page()->setContentEditable(false);
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
    page()->mainFrame()->evaluateJavaScript(c_jQuery);
    page()->mainFrame()->evaluateJavaScript(c_jQueryScrollTo);
    page()->mainFrame()->evaluateJavaScript(c_jQueryWrapSelection);
    m_pendingLoadCount -= 1;

    if (m_pendingLoadCount == 0) {
        if (!m_pendingScrollToFragment.isEmpty()) {
            ScrollToFragment(m_pendingScrollToFragment);
            m_pendingScrollToFragment.clear();
        } else {
            executeCaretUpdateInternal();
        }
    }
}


int BookViewPreview::GetLocalSelectionOffset(bool start_of_selection)
{
    int anchor_offset = EvaluateJavascript("document.getSelection().anchorOffset;").toInt();
    int focus_offset  = EvaluateJavascript("document.getSelection().focusOffset;").toInt();
    QString javascript = "var anchor = document.getSelection().anchorNode;"
                         "var focus = document.getSelection().focusNode;"
                         "anchor.compareDocumentPosition( focus );";
    // The result of compareDocumentPosition is a bitmask
    int result_bitmask = EvaluateJavascript(javascript).toInt();

    // If the result is 0, then the anchor and focus are the same node
    if (result_bitmask == 0) {
        // If they are the collapsed, then it's just the caret
        if (anchor_offset == focus_offset) {
            return anchor_offset;
        }

        // This handles the situation with some text selected.
        // If we need the start of selection, we return the smaller
        // index; otherwise, the larger one.
        if (start_of_selection) {
            return anchor_offset < focus_offset ? anchor_offset : focus_offset;
        } else {
            return anchor_offset > focus_offset ? anchor_offset + 1 : focus_offset + 1;
        }
    }
    // Otherwise, they are different nodes
    else {
        // With 4 set, then node A precedes node B
        bool anchor_first = (result_bitmask & 4) == 4;

        if (start_of_selection) {
            return anchor_first ? anchor_offset : focus_offset;
        } else {
            return anchor_first ? focus_offset : anchor_offset;
        }
    }
}

int BookViewPreview::GetSelectionOffset(const xc::DOMDocument &document,
                                        const QMap<int, xc::DOMNode *> &node_offsets,
                                        Searchable::Direction search_direction)
{
    QList<ViewEditor::ElementIndex> cl = GetCaretLocation();
    xc::DOMNode *caret_node = XhtmlDoc::GetNodeFromHierarchy(document, cl);
    bool searching_down = search_direction == Searchable::Direction_Down ? true : false;
    int local_offset    = GetLocalSelectionOffset(!searching_down);
    int search_start    = node_offsets.key(caret_node) + local_offset;
    return search_start;
}


BookViewPreview::SearchTools BookViewPreview::GetSearchTools() const
{
    SearchTools search_tools;
    search_tools.fulltext = "";
    search_tools.document = XhtmlDoc::LoadTextIntoDocument(page()->mainFrame()->toHtml());
    QList<xc::DOMNode *> text_nodes = XhtmlDoc::GetVisibleTextNodes(
                                          *(search_tools.document->getElementsByTagName(QtoX("body"))->item(0)));
    xc::DOMNode *current_block_ancestor = NULL;

    // We concatenate all text nodes that have the same
    // block level ancestor element. A newline is added
    // when a new block element starts.
    // We also record the starting offset of every text node.
    for (int i = 0; i < text_nodes.count(); ++i) {
        xc::DOMNode *new_block_ancestor = &XhtmlDoc::GetAncestorBlockElement(*text_nodes[ i ]);

        if (new_block_ancestor == current_block_ancestor) {
            search_tools.node_offsets[ search_tools.fulltext.length() ] = text_nodes[ i ];
            search_tools.fulltext.append(XtoQ(text_nodes[ i ]->getNodeValue()));
        } else {
            current_block_ancestor = new_block_ancestor;
            search_tools.fulltext.append("\n");
            search_tools.node_offsets[ search_tools.fulltext.length() ] = text_nodes[ i ];
            search_tools.fulltext.append(XtoQ(text_nodes[ i ]->getNodeValue()));
        }
    }

    return search_tools;
}


QString BookViewPreview::GetElementSelectingJS_NoTextNodes(const QList<ViewEditor::ElementIndex> &hierarchy) const
{
    // TODO: see if replacing jQuery with pure JS will speed up
    // caret location scrolling... note the children()/contents() difference:
    // children() only considers element nodes, contents() considers text nodes too.
    QString element_selector = "$('html')";
    // We will have a different hierarchy depending on whether it was generated by CodeView or
    // generated by BV/PV. If the last element is '#text', strip it off and make sure the
    // element preceding it has a -1 index in order for the caret to move correctly.
    bool adjust_last_index = false;
    int hierarchy_length = hierarchy.count() - 1;

    if (hierarchy_length > 1 && hierarchy.last().name == "#text") {
        hierarchy_length--;
    }

    for (int i = 0; i < hierarchy_length; ++i) {
        int index = hierarchy[ i ].index;

        if ((i == hierarchy_length) && adjust_last_index) {
            index = -1;
        }

        element_selector.append(QString(".children().eq(%1)").arg(index));
    }

    element_selector.append(".get(0)");
    return element_selector;
}

QList<ViewEditor::ElementIndex> BookViewPreview::GetCaretLocation()
{
    // The location element hierarchy encoded in a string
    QString location_string = EvaluateJavascript(c_GetCaretLocation).toString();
    QStringList elements    = location_string.split(",", QString::SkipEmptyParts);
    QList<ElementIndex> caret_location;
    foreach(QString element, elements) {
        ElementIndex new_element;
        new_element.name  = element.split(" ")[ 0 ];
        new_element.index = element.split(" ")[ 1 ].toInt();
        caret_location.append(new_element);
    }
    return caret_location;
}

void BookViewPreview::StoreCurrentCaretLocation()
{
    // Only overwrite the current location stored if it is empty, in case we specifically
    // want a new location when switching to a new view
    if (m_CaretLocationUpdate.isEmpty()) {
        StoreCaretLocationUpdate(GetCaretLocation());
    }
}

void BookViewPreview::StoreCaretLocationUpdate(const QList<ViewEditor::ElementIndex> &hierarchy)
{
    QString caret_location = "var element = " + GetElementSelectingJS_NoTextNodes(hierarchy) + ";";
    // We scroll to the element and center the screen on it
    QString scroll = "var from_top = window.innerHeight / 2;"
                     "$.scrollTo( element, 0, {offset: {top:-from_top, left:0 } } );";
    m_CaretLocationUpdate = caret_location + scroll + SET_CURSOR_JS;
}

QString BookViewPreview::GetElementSelectingJS_WithTextNode(const QList<ViewEditor::ElementIndex> &hierarchy) const
{
    QString element_selector = "$('html')";

    for (int i = 0; i < hierarchy.count() - 1; ++i) {
        element_selector.append(QString(".children().eq(%1)").arg(hierarchy[ i ].index));
    }

    element_selector.append(QString(".contents().eq(%1)").arg(hierarchy.last().index));
    element_selector.append(".get(0)");
    return element_selector;
}

QWebElement BookViewPreview::DomNodeToQWebElement(const xc::DOMNode &node)
{
    const QList<ViewEditor::ElementIndex> &hierarchy = XhtmlDoc::GetHierarchyFromNode(node);
    QWebElement element = page()->mainFrame()->documentElement();
    element.findFirst("html");

    for (int i = 0; i < hierarchy.count() - 1; ++i) {
        element = XhtmlDoc::QWebElementChildren(element).at(hierarchy[ i ].index);
    }

    return element;
}

bool BookViewPreview::ExecuteCaretUpdate()
{
    // Currently certain actions in Sigil result in a document being loaded multiple times
    // in response to the signals. Only proceed with moving the caret if all loads are finished.
    if (m_pendingLoadCount > 0) {
        return false;
    }

    // If there is no caret location update pending...
    if (m_CaretLocationUpdate.isEmpty()) {
        return false;
    }

    // run it...
    EvaluateJavascript(m_CaretLocationUpdate);
    // ...and clear the update.
    m_CaretLocationUpdate.clear();
    return true;
}

bool BookViewPreview::ExecuteCaretUpdate(const QString &caret_update)
{
    // This overload is for use with the Back To Link type functionality, where we have
    // a specific caret location javascript we want to force when the tab is fully loaded.
    if (!caret_update.isEmpty()) {
        m_CaretLocationUpdate = caret_update;
        return ExecuteCaretUpdate();
    }

    return false;
}

BookViewPreview::SelectRangeInputs BookViewPreview::GetRangeInputs(const QMap<int, xc::DOMNode *> &node_offsets,
        int string_start,
        int string_length) const
{
    SelectRangeInputs input;
    QList<int> offsets = node_offsets.keys();
    int last_offset      = offsets.first();

    for (int i = 0; i < offsets.length(); ++i) {
        int next_offset = 0;

        // If there is such a thing as the next offset, use it
        if (i + 1 < offsets.length()) {
            next_offset = offsets[ i + 1 ];
        }
        // Otherwise compute it
        else
            // + 1 because we are pretending there is another text node after this one
        {
            next_offset = offsets[ i ] + XtoQ(node_offsets[ offsets[ i ] ]->getNodeValue()).length() + 1;
        }

        if (next_offset > string_start &&
            input.start_node == NULL
           ) {
            input.start_node_index = string_start - last_offset;
            input.start_node       = node_offsets.value(last_offset);
        }

        if (next_offset > string_start + string_length &&
            input.end_node == NULL
           ) {
            input.end_node_index = string_start + string_length - last_offset;
            input.end_node       = node_offsets.value(last_offset);
        }

        if (input.start_node != NULL && input.end_node != NULL) {
            break;
        }

        last_offset = next_offset;
    }

    // TODO: throw an exception
    Q_ASSERT(input.start_node != NULL && input.end_node != NULL);
    return input;
}

QString BookViewPreview::GetRangeJS(const SelectRangeInputs &input) const
{
    QString start_node_js = GetElementSelectingJS_WithTextNode(XhtmlDoc::GetHierarchyFromNode(*input.start_node));
    QString end_node_js   = GetElementSelectingJS_WithTextNode(XhtmlDoc::GetHierarchyFromNode(*input.end_node));
    QString start_node_index = QString::number(input.start_node_index);
    QString end_node_index   = QString::number(input.end_node_index);
    QString get_range_js = c_GetRange;
    get_range_js.replace("$START_NODE",   start_node_js);
    get_range_js.replace("$END_NODE",     end_node_js);
    get_range_js.replace("$START_OFFSET", start_node_index);
    get_range_js.replace("$END_OFFSET",   end_node_index);
    return get_range_js;
}

void BookViewPreview::SelectTextRange(const SelectRangeInputs &input)
{
    EvaluateJavascript(GetRangeJS(input) + c_NewSelection);
}


void BookViewPreview::ScrollToNodeText(const xc::DOMNode &node, int character_offset)
{
    const int MIN_MARGIN = 20;
    const QWebElement element = DomNodeToQWebElement(node);
    QRect element_geometry    = element.geometry();
    int elem_offset  = element_geometry.top();
    int elem_height  = element_geometry.height();
    int frame_height = height();
    Q_ASSERT(frame_height != 0);
    int current_scroll_offset = page()->mainFrame()->scrollBarValue(Qt::Vertical);
    int new_scroll_Y = 0;
    // We always want to make sure that the actual text within the block is visible at
    // the position we want it, in case the paragraph is very long.
    float char_position = character_offset / (float) element.toPlainText().count();
    int actual_offset = elem_offset + elem_height * char_position;

    // If the full element is visible with enough margin, then we don't scroll
    if ((elem_offset >= current_scroll_offset) &&
        (actual_offset - MIN_MARGIN >= current_scroll_offset) &&
        (elem_offset + elem_height <= current_scroll_offset + frame_height) &&
        (actual_offset + MIN_MARGIN <= current_scroll_offset + frame_height)) {
        // It is all visible, so no scrolling required.
        return;
    } else {
        // In all other circumstances (scrolling upwards or downwards) center on screen.
        new_scroll_Y = actual_offset - (frame_height / 2);
    }

    // If the element is very near the beginning of the document,
    // we can't position the bottom of the screen on the element
    if (new_scroll_Y < 0) {
        new_scroll_Y = 0;
    }

    page()->mainFrame()->setScrollBarValue(Qt::Vertical, new_scroll_Y);
}

void BookViewPreview::InspectElement()
{
    page()->triggerAction(QWebPage::InspectElement);
}

void BookViewPreview::CreateContextMenuActions()
{
    m_InspectElement = new QAction(tr("Inspect Element"), this);
}

void BookViewPreview::OpenContextMenu(const QPoint &point)
{
    if (!SuccessfullySetupContextMenu(point)) {
        return;
    }

    m_ContextMenu.exec(mapToGlobal(point));
    m_ContextMenu.clear();
}

bool BookViewPreview::SuccessfullySetupContextMenu(const QPoint &point)
{
    m_ContextMenu.addAction(m_InspectElement);
    m_InspectElement->setEnabled(page()->action(QWebPage::InspectElement)->isEnabled());
    return true;
}

void BookViewPreview::ConnectSignalsToSlots()
{
    connect(this,  SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(OpenContextMenu(const QPoint &)));
    connect(m_InspectElement,    SIGNAL(triggered()),  this, SLOT(InspectElement()));
    connect(page(), SIGNAL(loadFinished(bool)), this, SLOT(UpdateFinishedState(bool)));
    connect(page(), SIGNAL(linkClicked(const QUrl &)), SIGNAL(LinkClicked(const QUrl &)));
    connect(page(), SIGNAL(loadFinished(bool)), this, SLOT(WebPageJavascriptOnLoad()));
}
