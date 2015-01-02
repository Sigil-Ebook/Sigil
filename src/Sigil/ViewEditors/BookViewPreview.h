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

#include <boost/shared_ptr.hpp>

#include <QtCore/QMap>
#include <QtWebKitWidgets/QWebView>

#include "BookManipulation/XercesHUse.h"
#include "ViewEditors/ViewEditor.h"

using boost::shared_ptr;

class QSize;
class ViewWebPage;

class BookViewPreview : public QWebView, public ViewEditor
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param parent The object's parent.
     */
    BookViewPreview(QWidget *parent = 0);
    ~BookViewPreview();

    QSize sizeHint() const;

    void CustomSetDocument(const QString &path, const QString &html);

    bool IsLoadingFinished();

    void SetZoomFactor(float factor);

    void SetCurrentZoomFactor(float factor);

    float GetZoomFactor() const;

    void Zoom();

    void UpdateDisplay();

    /**
     * Scrolls the editor to the top.
     */
    void ScrollToTop();

    /**
     * Scrolls the editor to the specified fragment when the document is loaded.
     *
     * @param fragment The fragment ID to scroll to.
     *                 It should have the "#" character as the first character.
     */
    void ScrollToFragment(const QString &fragment);

    bool FindNext(const QString &search_regex,
                  Searchable::Direction search_direction,
                  bool check_spelling = false,
                  bool ignore_selection_offset = false,
                  bool wrap = true,
                  bool selected_text = false);

    int Count(const QString &search_regex, Searchable::Direction direction, bool wrap, bool selected_text = false);

    bool ReplaceSelected(const QString &search_regex, const QString &replacement, Searchable::Direction direction = Searchable::Direction_Down, bool keep_selection = false);

    int ReplaceAll(const QString &search_regex, const QString &replacement, Searchable::Direction direction, bool wrap, bool selected_text);

    QString GetSelectedText();

    void SetUpFindForSelectedText(const QString &search_regex);

    /**
     *  Workaround for a crappy setFocus implementation in QtWebKit.
     */
    void GrabFocus();

    // inherited
    QList<ViewEditor::ElementIndex> GetCaretLocation();

    // inherited
    void StoreCaretLocationUpdate(const QList<ViewEditor::ElementIndex> &hierarchy);

    // inherited
    bool ExecuteCaretUpdate();

    /**
     * Force a caret location update to the specified position.
     */
    bool ExecuteCaretUpdate(const QString &caret_location);

    QString GetCaretLocationUpdate();

    QString GetDisplayedCharacters();
    void ShowTag();

    void HighlightPosition();

    void StoreCurrentCaretLocation();

public slots:

    /**
     * Exposed for the Windows clipboard error workaround to
     * retry a clipboard copy operation.
     */
    void copy();

    void InspectElement();

signals:
    /**
     * Emitted whenever the zoom factor changes.
     *
     * @param new_zoom_factor The new zoom factor of the View.
     */
    void ZoomFactorChanged(float new_zoom_factor);

    void LinkClicked(const QUrl &url);

    void ShowStatusMessageRequest(const QString &message);

    void DocumentLoaded();

    void GoToPreviewLocationRequest();

protected:
    void mouseReleaseEvent(QMouseEvent *event);

    /**
     * Evaluates the provided javascript source code
     * and returns the result.
     *
     * @param javascript The JavaScript source code to execute.
     * @return The result from the last executed javascript statement.
     */
    QVariant EvaluateJavascript(const QString &javascript);

    /**
     * Javascript source that implements a function to find the
     * first block-level parent of a node in the source.
     */
    const QString c_GetBlock;

    bool m_isLoadFinished;

protected slots:

    void UpdateFinishedState(bool okay);

    /**
     * Opens the context menu at the requested point.
     *
     * @param point The point at which the menu should be opened.
     */
    virtual void OpenContextMenu(const QPoint &point);

protected:

    QMenu &m_ContextMenu;

private slots:

    /**
     * Loads the required JavaScript on web page loads.
     */
    void WebPageJavascriptOnLoad();

    void executeCaretUpdateInternal() {
        ExecuteCaretUpdate();
    }

private:

    /**
     * Actually performs the scrolling, will only be invoked after the document has loaded.
     */
    void ScrollToFragmentInternal(const QString &fragment);

    /**
     * Builds the element-selecting JavaScript code, ignoring the text nodes.
     * Always just chains children() jQuery calls.
     *
     * @return The element-selecting JavaScript code.
     */
    QString GetElementSelectingJS_NoTextNodes(const QList<ViewEditor::ElementIndex> &hierarchy) const;

    /**
     * Builds the element-selecting JavaScript code, ignoring all the
     * text nodes except the last one.
     * Chains children() jQuery calls, and then the contents() function
     * for the last element (the text node, naturally).
     *
     * @return The element-selecting JavaScript code.
     */
    QString GetElementSelectingJS_WithTextNode(const QList<ViewEditor::ElementIndex> &hierarchy) const;

    /**
     * Converts a DomNode from a Dom of the current page
     * into the QWebElement of that same element on tha page.
     *
     * @param node The node to covert.
     */
    QWebElement DomNodeToQWebElement(const xc::DOMNode &node);

    /**
     * Returns the local character offset of the selection.
     * The offset is calculated in the local text node.
     *
     * @param start_of_selection If \c true, then the offset is calculated from
     *                           the start of the selection. Otherwise, from its end.
     * @return The offset.
     */
    int GetLocalSelectionOffset(bool start_of_selection);

    /**
     * Returns the selection offset from the start of the document.
     *
     * @param document The loaded DOM document.
     * @param node_offsets The text node offset map from SearchTools.
     * @param search_direction The direction of the search.
     * @return The offset.
     */
    int GetSelectionOffset(const xc::DOMDocument &document,
                           const QMap<int, xc::DOMNode *> &node_offsets,
                           Searchable::Direction search_direction);

    /**
     * The necessary tools for searching.
     */
    struct SearchTools {
        /**
         * The full text of the document.
         */
        QString fulltext;

        /**
         * A map with text node starting offsets as keys,
         * and those text nodes as values.
         */
        QMap<int, xc::DOMNode *> node_offsets;

        /**
         *  A DOM document with the loaded text.
         */
        shared_ptr<xc::DOMDocument> document;
    };

    /**
     * Private overload for FindNext that allows it to return (by reference) the SearchTools object
     * it creates so that the DOM doesn't need to be parsed again when ReplaceSelected is called immediately
     * after.
     */
    bool FindNext(SearchTools &search_tools,
                  const QString &search_regex,
                  Searchable::Direction search_direction,
                  bool check_spelling = false,
                  bool ignore_selection_offset = false,
                  bool wrap = true,
                  bool selected_text = false);

    /**
     * The inputs for a new JavaScript \c range object.
     */
    struct SelectRangeInputs {
        SelectRangeInputs()
            :
            start_node(NULL),
            end_node(NULL),
            start_node_index(-1),
            end_node_index(-1) {}

        /**
         * The range start node.
         */
        xc::DOMNode *start_node;

        /**
         *  The range end node.
         */
        xc::DOMNode *end_node;

        /**
         * The char index inside the start node.
         */
        int start_node_index;

        /**
         * The char index inside the end node.
         */
        int end_node_index;
    };

    /**
     * Converts the parameters into JavaScript \c range object inputs.
     * The \c range object can then be used to select this particular string.
     *
     * @param The node offset map
     * @param string_start The index of the string in the full document text
     * @param string_length The string's length
     * @return The inputs for a \c range object.
     * @see SearchTools
     */
    SelectRangeInputs GetRangeInputs(const QMap<int, xc::DOMNode *> &node_offsets,
                                     int string_start,
                                     int string_length) const;

    /**
     * Converts the \c range input struct into the \c range creating JavaScript code.
     *
     * @param input The \c range object inputs.
     * @return The \c range creating JavaScript code.
     */
    QString GetRangeJS(const SelectRangeInputs &input) const;

    /**
     * Selects the string identified by the \c range inputs.
     *
     * @param input The \c range inputs.
     */
    void SelectTextRange(const SelectRangeInputs &input);

    /**
     * Scrolls the view to the specified node and text offset
     * within that node.
     *
     * @param node The node to scroll to.
     * @param character_offset The specific offset we're interested
     *                         in within the node.
     */
    void ScrollToNodeText(const xc::DOMNode &node, int character_offset);

    /**
     * Returns the all the necessary tools for searching.
     * Reads from the QWebPage source.
     *
     * @return The necessary tools for searching.
     */
    SearchTools GetSearchTools() const;

    void CreateContextMenuActions();

    /**
     * Tries to setup the context menu for the specified point,
     * and returns true on success.
     *
     * @param point The point at which the menu should be opened.
     * @return \c true if the menu could be set up.
     */
    bool SuccessfullySetupContextMenu(const QPoint &point);

    /**
     * Connects all the required signals to their respective slots.
     */
    void ConnectSignalsToSlots();

    ///////////////////////////////
    // PRIVATE MEMBER VARIABLES
    ///////////////////////////////

    ViewWebPage *m_ViewWebPage;

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

    /**
     * The JavaScript source code
     * for creating DOM ranges.
     */
    const QString c_GetRange;

    /**
     * The JavaScript source code that
     * removes all of the current selections
     * and adds the range in the "range"
     * variable to the current selection.
     */
    const QString c_NewSelection;

    /**
     * The JavaScript source code for getting the html
     * for parent tag at the caret location.
     */
    const QString c_GetParentTags;

    /**
     * Stores the JavaScript source code for the
     * caret location update. Used when switching from
     * CodeViewEditor to BookViewEditor.
     */
    QString m_CaretLocationUpdate;

    int m_pendingLoadCount;
    QString m_pendingScrollToFragment;

    QAction *m_InspectElement;

};

#endif // BOOKVIEWPREVIEW_H
