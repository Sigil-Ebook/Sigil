/************************************************************************
**
**  Copyright (C) 2009, 2010, 2011  Strahinja Markovic  <strahinja.markovic@gmail.com>
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

#include <stdafx.h>
#include <QMessageBox>
#include "BookViewEditor.h"
#include "BookManipulation/Book.h"
#include "BookManipulation/XhtmlDoc.h"
#include "BookManipulation/CleanSource.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "PCRE/PCRECache.h"
#include "BookManipulation/XercesCppUse.h"


const int PROGRESS_BAR_MINIMUM_DURATION = 1500;

const QString BREAK_TAG_INSERT    = "<hr class=\"sigilChapterBreak\" />";
const QString XML_NAMESPACE_CRUFT = "xmlns=\"http://www.w3.org/1999/xhtml\"";

/**
 * The JavaScript source code for getting a string representation
 * of the "body" tag (without the children).
 */
static const QString GET_BODY_TAG_HTML = "new XMLSerializer().serializeToString( document.body.cloneNode(false) );";


BookViewEditor::BookViewEditor( QWidget *parent )
    : 
    QWebView( parent ),
    m_CaretLocationUpdate( QString() ),
    m_isLoadFinished( false ),
    m_PageUp(   *( new QShortcut( QKeySequence( QKeySequence::MoveToPreviousPage ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_PageDown( *( new QShortcut( QKeySequence( QKeySequence::MoveToNextPage     ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_ScrollOneLineUp(   *( new QShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_Up   ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_ScrollOneLineDown( *( new QShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_Down ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    c_GetCaretLocation( Utility::ReadUnicodeTextFile( ":/javascript/book_view_current_location.js" ) ),
    c_NewSelection(     Utility::ReadUnicodeTextFile( ":/javascript/new_selection.js"              ) ),
    c_GetRange(         Utility::ReadUnicodeTextFile( ":/javascript/get_range.js"                  ) ),
    c_ReplaceWrapped(   Utility::ReadUnicodeTextFile( ":/javascript/replace_wrapped.js"            ) ),
    c_ReplaceUndo(      Utility::ReadUnicodeTextFile( ":/javascript/replace_undo.js"               ) ),
    c_GetSegmentHTML(   Utility::ReadUnicodeTextFile( ":/javascript/get_segment_html.js"           ) ),
    c_GetBlock(         Utility::ReadUnicodeTextFile( ":/javascript/get_block.js"                  ) ),
    c_FormatBlock(      Utility::ReadUnicodeTextFile( ":/javascript/format_block.js"               ) ),
    c_SetCursor(        Utility::ReadUnicodeTextFile( ":/javascript/set_cursor.js"                 ) )

{
    connect( &m_PageUp,            SIGNAL( activated() ), this, SLOT( PageUp()            ) );
    connect( &m_PageDown,          SIGNAL( activated() ), this, SLOT( PageDown()          ) );
    connect( &m_ScrollOneLineUp,   SIGNAL( activated() ), this, SLOT( ScrollOneLineUp()   ) );
    connect( &m_ScrollOneLineDown, SIGNAL( activated() ), this, SLOT( ScrollOneLineDown() ) );

    // Set the Zoom factor but be sure no signals are set because of this.
    SettingsStore *ss = SettingsStore::instance();
    m_CurrentZoomFactor = ss->zoomWeb();
    Zoom();
}


QSize BookViewEditor::sizeHint() const
{
    return QSize( 16777215, 16777215 );
}


void BookViewEditor::CustomSetWebPage( QWebPage &webpage )
{
    m_isLoadFinished = true;

    connect( this,     SIGNAL( contentsChangedExtra() ), &webpage, SIGNAL( contentsChanged()        ) );
    connect( &webpage, SIGNAL( contentsChanged()      ), this,     SIGNAL( textChanged()            ) );

    connect( this,     SIGNAL( contentsChangedExtra() ), this,     SLOT( TextChangedFilter()        ) );
    connect( &webpage, SIGNAL( contentsChanged()      ), this,     SLOT( TextChangedFilter()        ) );

    connect( &webpage, SIGNAL( selectionChanged()     ), this,     SIGNAL( selectionChanged()       ) );
    connect( &webpage, SIGNAL( loadFinished( bool )   ), this,     SLOT( JavascriptOnDocumentLoad() ) );
    connect( &webpage, SIGNAL( loadProgress( int )    ), this,     SLOT( UpdateFinishedState( int ) ) );

    connect( &webpage, SIGNAL( linkClicked( const QUrl& ) ), this, SLOT( LinkClickedFilter( const QUrl&  ) ) );

    connect( this,                     SIGNAL( FilteredLinkClicked( const QUrl& ) ),
             this->parent()->parent(), SIGNAL( LinkClicked(         const QUrl& ) ) );

    // Needs to come after the signals connect;
    // we don't want race conditions.
    setPage( &webpage );
}


QString BookViewEditor::SplitChapter()
{
    QString head     = page()->mainFrame()->documentElement().findFirst( "head" ).toOuterXml();    
    QString body_tag = EvaluateJavascript( GET_BODY_TAG_HTML ).toString();
    QString segment  = EvaluateJavascript( c_GetBlock % c_GetSegmentHTML ).toString();

    emit contentsChangedExtra();

    return QString( "<html>" )
           .append( head )
           .append( body_tag )
           .append( segment )
           .append( "</body></html>" )
           // Webkit adds this xmlns attribute to *every*
           // element... for no reason. Tidy will add it back
           // to the <head> so we just remove it globally.
           .remove( XML_NAMESPACE_CRUFT );
}


void BookViewEditor::ExecCommand( const QString &command )
{
    if( m_isLoadFinished )
    {
        QString javascript = QString( "document.execCommand( '%1', false, null)" ).arg( EscapeJSString( command ) );

        EvaluateJavascript( javascript );
    }
}


void BookViewEditor::ExecCommand( const QString &command, const QString &parameter )
{       
    QString javascript = QString( "document.execCommand( '%1', false, '%2' )" )
                            .arg( EscapeJSString( command ) )
                            .arg( EscapeJSString( parameter ) );

    EvaluateJavascript( javascript );
}


bool BookViewEditor::QueryCommandState( const QString &command )
{
    QString javascript = QString( "document.queryCommandState( '%1', false, null)" ).arg( EscapeJSString( command ) );

    return EvaluateJavascript( javascript ).toBool();
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
    qobject_cast< QWidget *>( parent() )->setFocus();
    
    QWebView::setFocus();
}


// Overridden so we can emit the FocusGained() signal.
void BookViewEditor::focusInEvent( QFocusEvent *event )
{
    emit FocusGained( this );

    QWebView::focusInEvent( event );
}


// Overridden so we can emit the FocusLost() signal.
void BookViewEditor::focusOutEvent( QFocusEvent *event )
{
    emit FocusLost( this );

    QWebView::focusOutEvent( event );
}


void BookViewEditor::ScrollToTop()
{
    QString caret_location = "var elementList = document.getElementsByTagName(\"body\");"
                             "var element = elementList[0];";

    QString scroll = "var from_top = window.innerHeight / 2;"
        "$.scrollTo( element, 0, {offset: {top:-from_top, left:0 } } );";

    EvaluateJavascript( caret_location % scroll % c_SetCursor );
}


void BookViewEditor::ScrollToFragment( const QString &fragment )
{
    if( fragment.isEmpty() )
    {
        ScrollToTop();
        return;
    }

    QString caret_location = "var element = document.getElementById(\"" % fragment % "\");";

    QString scroll = "var from_top = window.innerHeight / 2;"
        "$.scrollTo( element, 0, {offset: {top:-from_top, left:0 } } );";

    EvaluateJavascript( caret_location % scroll % c_SetCursor );
}


void BookViewEditor::ScrollToFragmentAfterLoad( const QString &fragment )
{
    if ( fragment.isEmpty() )

        return;

    QString caret_location = "var element = document.getElementById(\"" % fragment % "\");";

    QString scroll = "var from_top = window.innerHeight / 2;"
        "$.scrollTo( element, 0, {offset: {top:-from_top, left:0 } } );";

    QString javascript = "window.addEventListener('load', GoToFragment, false);"
                         "function GoToFragment() { " % caret_location % scroll % c_SetCursor % "}";

    EvaluateJavascript( javascript );
}


void BookViewEditor::FormatBlock( const QString &element_name )
{

    if ( element_name.isEmpty() )

        return;

    QString javascript =  c_GetBlock % c_FormatBlock %
            "var node = document.getSelection().anchorNode;"
            "var startNode = get_block( node );"
            "var element = format_block( startNode, \""+element_name+"\" );"
            "startNode.parentNode.replaceChild( element, startNode );"
            % c_SetCursor;

    EvaluateJavascript( javascript );

    emit contentsChangedExtra();
}


QString BookViewEditor::GetCaretElementName()
{
    QString javascript =  "var node = document.getSelection().anchorNode;"
                          "var startNode = get_block( node );"
                          "startNode.nodeName;";

    return EvaluateJavascript( c_GetBlock % javascript ).toString();
}


QList< ViewEditor::ElementIndex > BookViewEditor::GetCaretLocation()
{
    // The location element hierarchy encoded in a string
    QString location_string = EvaluateJavascript( c_GetCaretLocation ).toString();
    QStringList elements    = location_string.split( ",", QString::SkipEmptyParts );

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


void BookViewEditor::StoreCaretLocationUpdate( const QList< ViewEditor::ElementIndex > &hierarchy )
{
    QString caret_location = "var element = " + GetElementSelectingJS_NoTextNodes( hierarchy ) + ";";

    // We scroll to the element and center the screen on it
    QString scroll = "var from_top = window.innerHeight / 2;"
                     "$.scrollTo( element, 0, {offset: {top:-from_top, left:0 } } );";

    m_CaretLocationUpdate = caret_location + scroll + c_SetCursor;
}


bool BookViewEditor::IsLoadingFinished()
{
    return m_isLoadFinished;
}


void BookViewEditor::SetZoomFactor( float factor )
{
    SettingsStore *ss = SettingsStore::instance();
    ss->setZoomWeb( factor );
    m_CurrentZoomFactor = factor;
    Zoom();
    emit ZoomFactorChanged( factor );
}


float BookViewEditor::GetZoomFactor() const
{
    SettingsStore *ss = SettingsStore::instance();
    return ss->zoomWeb();
}


void BookViewEditor::Zoom()
{
    setZoomFactor( m_CurrentZoomFactor );
}


void BookViewEditor::UpdateDisplay()
{
    SettingsStore *ss = SettingsStore::instance();
    float stored_factor = ss->zoomWeb();
    if ( stored_factor != m_CurrentZoomFactor )
    {
        m_CurrentZoomFactor = stored_factor;
        Zoom();
    }
}


bool BookViewEditor::FindNext( const QString &search_regex,
                               Searchable::Direction search_direction,
                               bool ignore_selection_offset,
                               bool wrap
                             )
                              

{
    SearchTools search_tools = GetSearchTools();
    return FindNext( search_tools, search_regex, search_direction, ignore_selection_offset, wrap );
}

bool BookViewEditor::FindNext( SearchTools &search_tools,
                               const QString &search_regex,
                               Searchable::Direction search_direction,
                               bool ignore_selection_offset,
                               bool wrap
                             )
{
    SPCRE *spcre = PCRECache::instance()->getObject(search_regex);
    SPCRE::MatchInfo match_info;
    int start_offset = 0;
    int selection_offset = -1;

    if ( ignore_selection_offset )
    {
        selection_offset = 0;
    }
    else
    {
        selection_offset = GetSelectionOffset( *search_tools.document, search_tools.node_offsets, search_direction ) - 1;
    }

    // Get the match info for the direction.
    if ( search_direction == Searchable::Direction_Up )
    {
        match_info = spcre->getLastMatchInfo( Utility::Substring( 0, selection_offset, search_tools.fulltext ) );
    }
    else
    {
        match_info = spcre->getFirstMatchInfo( Utility::Substring( selection_offset, search_tools.fulltext.count(), search_tools.fulltext ) );
        start_offset = selection_offset;
    }

    m_lastMatch = match_info;

    if ( match_info.offset.first != -1 )
    {
        m_lastMatch.offset.first += selection_offset;
        m_lastMatch.offset.second += selection_offset;

        SelectRangeInputs input = GetRangeInputs( search_tools.node_offsets, match_info.offset.first + start_offset, match_info.offset.second - match_info.offset.first );
        SelectTextRange( input );
        ScrollToNodeText( *input.start_node, input.start_node_index );

        return true;
    }
    else if ( wrap )
    {
        if ( FindNext( search_regex, search_direction, true, false ) )
        {
            ShowWrapIndicator(this);
            return true;
        }
    }

    return false;
}


int BookViewEditor::Count( const QString &search_regex )
{
    SPCRE *spcre = PCRECache::instance()->getObject( search_regex );
    return spcre->getEveryMatchInfo( GetSearchTools().fulltext ).count();
}

bool BookViewEditor::ReplaceSelected( const QString &search_regex, const QString &replacement, Searchable::Direction direction )
{
    QMessageBox::critical( this, tr( "Unsupported" ), tr( "Replace is not supported in Book View at this time.  Switch to Code View." ) );

    return false;

#if 0
    SearchTools search_tools = GetSearchTools();
    return ReplaceSelected( search_regex, replacement, search_tools, direction );
#endif
}

bool BookViewEditor::ReplaceSelected( const QString &search_regex, const QString &replacement, SearchTools search_tools, Searchable::Direction direction )
{
    return false;

#if 0
    SPCRE *spcre = PCRECache::instance()->getObject( search_regex );

    int selection_offset = GetSelectionOffset( *search_tools.document, search_tools.node_offsets, Searchable::Direction_Up );

    if ( m_lastMatch.offset.first == selection_offset )
    {
        QString replaced_text;
        bool replacement_made = spcre->replaceText( GetSelectedText(), m_lastMatch.capture_groups_offsets, replacement, replaced_text );

        if ( replacement_made )
        {
            SelectRangeInputs input = GetRangeInputs( search_tools.node_offsets, selection_offset, m_lastMatch.offset.second );

            SelectRangeJS inputJS;
            inputJS.start_node = GetElementSelectingJS_WithTextNode( XhtmlDoc::GetHierarchyFromNode( *input.start_node ) );
            inputJS.end_node =   GetElementSelectingJS_WithTextNode( XhtmlDoc::GetHierarchyFromNode( *input.end_node   ) );
            inputJS.start_node_index = input.start_node_index;
            inputJS.end_node_index   = input.end_node_index;

            BookViewReplaceCommand* replace_action = new BookViewReplaceCommand( this, inputJS, replaced_text );
            page()->undoStack()->push( replace_action );

            return true;
        }
    }

    return false;
#endif
}


int BookViewEditor::ReplaceAll( const QString &search_regex, const QString &replacement )
{
    QMessageBox::critical( this, tr( "Unsupported" ), tr( "Replace All for the current file is not supported in Book View at this time.  Switch to Code View." ) );

    return 0;

#if 0
    SearchTools search_tools = GetSearchTools();
    SPCRE *spcre = PCRECache::instance()->getObject( search_regex );
    QList<SPCRE::MatchInfo> match_info = spcre->getEveryMatchInfo( search_tools.fulltext );

    int count = 0;
    // We want this to be all one edit operation.
    page()->undoStack()->beginMacro("ReplaceAll");

    // Run though all match offsets making the replacment in reverse order.
    // This way changes in text lengh won't change the offsets as we make
    // our changes.
    for ( int i = match_info.count() - 1; i >= 0; i-- )
    {
        QString replaced_text;
        QString matched_text = Utility::Substring( match_info.at( i ).offset.first, match_info.at( i ).offset.second, search_tools.fulltext );
        bool replacement_made = spcre->replaceText( matched_text, match_info.at(i).capture_groups_offsets, replacement, replaced_text );

        if ( replacement_made )
        {
            SelectRangeInputs input = GetRangeInputs( search_tools.node_offsets, match_info.at( i ).offset.first, match_info.at( i ).offset.second - match_info.at( i ).offset.first );

            SelectRangeJS inputJS;
            inputJS.start_node = GetElementSelectingJS_WithTextNode( XhtmlDoc::GetHierarchyFromNode( *input.start_node ) );
            inputJS.end_node =   GetElementSelectingJS_WithTextNode( XhtmlDoc::GetHierarchyFromNode( *input.end_node   ) );
            inputJS.start_node_index = input.start_node_index;
            inputJS.end_node_index   = input.end_node_index;

            BookViewReplaceCommand* replace_action = new BookViewReplaceCommand( this, inputJS, replaced_text );
            page()->undoStack()->push( replace_action );

            count++;
        }
    }

    // End the edit operation.
    page()->undoStack()->endMacro();

    if ( count != 0 )
    {
        // Tell anyone who's interested that the document has been updated.
        emit contentsChangedExtra();
    }

    return count;
#endif
}


QString BookViewEditor::GetSelectedText()
{
    QString javascript = "window.getSelection().toString();";

    return EvaluateJavascript( javascript ).toString();
}


// Overridden because we need to update the cursor
// location if a cursor update (from CodeView) 
// is waiting to be processed.
// The update is pending only when we switch
// from Code View and there were no source changes
bool BookViewEditor::event( QEvent *event )
{
    // We just return whatever the "real" event handler returns
    bool real_return = QWebView::event( event );

    // Executing the caret update inside the paint event
    // handler causes artifacts on mac. So we do it after
    // the event is processed and accepted.
    if ( m_isLoadFinished && event->type() == QEvent::Paint )
    {
        ExecuteCaretUpdate();
    }

    return real_return;
}


void BookViewEditor::TextChangedFilter()
{
    m_lastMatch = SPCRE::MatchInfo();
}


void BookViewEditor::JavascriptOnDocumentLoad()
{
    // The jQuery libs are loaded in 
    // HTMLResouce::WebPageJavascriptOnLoad

    // Run the caret update if it's pending
    // ExecuteCaretUpdate();
}


void BookViewEditor::UpdateFinishedState( int progress )
{ 
    if ( progress == 100 )

        m_isLoadFinished = true;

    else

        m_isLoadFinished = false;
}


void BookViewEditor::LinkClickedFilter( const QUrl& url )
{
    // Urls in the document that have just "#fragmentID"
    // and no path (that is, "file local" urls), are returned
    // by QUrl.toString() as a path to the folder of this 
    // file with the fragment attached.
    if ( url.toString().contains( "/#" ) )

        ScrollToFragment( url.fragment() );

    else if ( url.scheme() == "file" )

        emit FilteredLinkClicked( url );

    // We kill all links to the internet
    else

        return;
}


void BookViewEditor::PageUp()
{
    ScrollByNumPixels( height(), false );
}


void BookViewEditor::PageDown()
{
    ScrollByNumPixels( height(), true );
}


void BookViewEditor::ScrollOneLineUp()
{
    ScrollByLine( false );
}


void BookViewEditor::ScrollOneLineDown()
{
    ScrollByLine( true );
}


QVariant BookViewEditor::EvaluateJavascript( const QString &javascript )
{
    return page()->mainFrame()->evaluateJavaScript( javascript );
}


int BookViewEditor::GetLocalSelectionOffset( bool start_of_selection )
{
    int anchor_offset = EvaluateJavascript( "document.getSelection().anchorOffset;" ).toInt();
    int focus_offset  = EvaluateJavascript( "document.getSelection().focusOffset;" ).toInt();

    QString javascript = "var anchor = document.getSelection().anchorNode;"
                         "var focus = document.getSelection().focusNode;"
                         "anchor.compareDocumentPosition( focus );";

    // The result of compareDocumentPosition is a bitmask
    int result_bitmask = EvaluateJavascript( javascript ).toInt();

    // If the result is 0, then the anchor and focus are the same node
    if ( result_bitmask == 0 )
    {
        // If they are the collapsed, then it's just the caret
        if ( anchor_offset == focus_offset )

            return anchor_offset;

        // This handles the situation with some text selected.
        // If we need the start of selection, we return the smaller
        // index; otherwise, the larger one.
        if ( start_of_selection )

            return anchor_offset < focus_offset ? anchor_offset : focus_offset;

        else

            return anchor_offset > focus_offset ? anchor_offset + 1 : focus_offset + 1;
    }

    // Otherwise, they are different nodes
    else
    {        
        // With 4 set, then node A precedes node B
        bool anchor_first = ( result_bitmask & 4 ) == 4;
       
        if ( start_of_selection )

            return anchor_first ? anchor_offset : focus_offset;

        else

            return anchor_first ? focus_offset : anchor_offset;
    }    
}


int BookViewEditor::GetSelectionOffset( const xc::DOMDocument &document,
                                        const QMap< int, xc::DOMNode* > &node_offsets, 
                                        Searchable::Direction search_direction )
{
    xc::DOMNode *caret_node = XhtmlDoc::GetNodeFromHierarchy( document, GetCaretLocation() );

    bool searching_down = search_direction == Searchable::Direction_Down ? true : false;

    int local_offset    = GetLocalSelectionOffset( !searching_down );
    int search_start    = node_offsets.key( caret_node ) + local_offset;

    return search_start;
}


BookViewEditor::SearchTools BookViewEditor::GetSearchTools() const
{
    SearchTools search_tools;
    search_tools.fulltext = "";

    search_tools.document = XhtmlDoc::LoadTextIntoDocument( page()->mainFrame()->toHtml() );

    QList< xc::DOMNode* > text_nodes = XhtmlDoc::GetVisibleTextNodes( 
        *( search_tools.document->getElementsByTagName( QtoX( "body" ) )->item( 0 ) ) );

    xc::DOMNode *current_block_ancestor = NULL;    

    // We concatenate all text nodes that have the same 
    // block level ancestor element. A newline is added
    // when a new block element starts.
    // We also record the starting offset of every text node.
    for ( int i = 0; i < text_nodes.count(); ++i )
    {
        xc::DOMNode *new_block_ancestor = &XhtmlDoc::GetAncestorBlockElement( *text_nodes[ i ] );

        if ( new_block_ancestor == current_block_ancestor )
        {
            search_tools.node_offsets[ search_tools.fulltext.length() ] = text_nodes[ i ];
            search_tools.fulltext.append( XtoQ( text_nodes[ i ]->getNodeValue() ) );
        }

        else
        {
            current_block_ancestor = new_block_ancestor;
            search_tools.fulltext.append( "\n" );

            search_tools.node_offsets[ search_tools.fulltext.length() ] = text_nodes[ i ];
            search_tools.fulltext.append( XtoQ( text_nodes[ i ]->getNodeValue() ) );
        }
    }

    return search_tools;
}


QString BookViewEditor::GetElementSelectingJS_NoTextNodes( const QList< ViewEditor::ElementIndex > &hierarchy ) const
{
    // TODO: see if replacing jQuery with pure JS will speed up
    // caret location scrolling... note the children()/contents() difference:
    // children() only considers element nodes, contents() considers text nodes too.

    QString element_selector = "$('html')";

    for ( int i = 0; i < hierarchy.count() - 1; ++i )
    {
        element_selector.append( QString( ".children().eq(%1)" ).arg( hierarchy[ i ].index ) );
    }

    element_selector.append( ".get(0)" );

    return element_selector;
}


QString BookViewEditor::GetElementSelectingJS_WithTextNode( const QList< ViewEditor::ElementIndex > &hierarchy ) const
{
    QString element_selector = "$('html')";

    for ( int i = 0; i < hierarchy.count() - 1; ++i )
    {
        element_selector.append( QString( ".children().eq(%1)" ).arg( hierarchy[ i ].index ) );
    }

    element_selector.append( QString( ".contents().eq(%1)" ).arg( hierarchy.last().index ) );
    element_selector.append( ".get(0)" );

    return element_selector;
}


QWebElement BookViewEditor::DomNodeToQWebElement( const xc::DOMNode &node )
{
    const QList< ViewEditor::ElementIndex > &hierarchy = XhtmlDoc::GetHierarchyFromNode( node );
    QWebElement element = page()->mainFrame()->documentElement();
    element.findFirst( "html" );

    for ( int i = 0; i < hierarchy.count() - 1; ++i )
    {
        element = XhtmlDoc::QWebElementChildren( element ).at( hierarchy[ i ].index );
    }

    return element;
}


QString BookViewEditor::EscapeJSString( const QString &string )
{
    QString new_string( string );

    /* \ -> \\ */ 
    // " -> \"
    // ' -> \'
    return new_string.replace( "\\", "\\\\" ).replace( "\"", "\\\"" ).replace( "'", "\\'" );
}


bool BookViewEditor::ExecuteCaretUpdate()
{
    // If there is no caret location update pending... 
    if ( m_CaretLocationUpdate.isEmpty() )
        
        return false;

    // run it...
    EvaluateJavascript( m_CaretLocationUpdate );

    // ...and clear the update.
    m_CaretLocationUpdate = ""; 

    return true;
}


BookViewEditor::SelectRangeInputs BookViewEditor::GetRangeInputs( const QMap< int, xc::DOMNode* > &node_offsets,
                                                                  int string_start, 
                                                                  int string_length ) const
{
    SelectRangeInputs input;

    QList< int > offsets = node_offsets.keys();
    int last_offset      = offsets.first();

    for ( int i = 0; i < offsets.length(); ++i )
    {
        int next_offset = 0;

        // If there is such a thing as the next offset, use it
        if ( i + 1 < offsets.length()  )

            next_offset = offsets[ i + 1 ];

        // Otherwise compute it
        else

            // + 1 because we are pretending there is another text node after this one
            next_offset = offsets[ i ] + XtoQ( node_offsets[ offsets[ i ] ]->getNodeValue() ).length() + 1;

        if ( next_offset > string_start && 
             input.start_node == NULL
           )
        {
            input.start_node_index = string_start - last_offset;
            input.start_node       = node_offsets.value( last_offset );
        }

        if ( next_offset > string_start + string_length &&
             input.end_node == NULL 
           )
        {
            input.end_node_index = string_start + string_length - last_offset;
            input.end_node       = node_offsets.value( last_offset );
        }

        if ( input.start_node != NULL && input.end_node != NULL )

            break;

        last_offset = next_offset;
    }

    // TODO: throw an exception
    Q_ASSERT( input.start_node != NULL && input.end_node != NULL );

    return input;
}


QString BookViewEditor::GetRangeJS( const SelectRangeInputs &input ) const
{
    QString start_node_js = GetElementSelectingJS_WithTextNode( XhtmlDoc::GetHierarchyFromNode( *input.start_node ) );
    QString end_node_js   = GetElementSelectingJS_WithTextNode( XhtmlDoc::GetHierarchyFromNode( *input.end_node   ) );

    QString start_node_index = QString::number( input.start_node_index );
    QString end_node_index   = QString::number( input.end_node_index );

    QString get_range_js = c_GetRange;

    get_range_js.replace( "$START_NODE",   start_node_js    );
    get_range_js.replace( "$END_NODE",     end_node_js      );
    get_range_js.replace( "$START_OFFSET", start_node_index );
    get_range_js.replace( "$END_OFFSET",   end_node_index   );

    return get_range_js;
}


void BookViewEditor::SelectTextRange( const SelectRangeInputs &input )
{
    EvaluateJavascript( GetRangeJS( input ) + c_NewSelection );
}


void BookViewEditor::ScrollToNodeText( const xc::DOMNode &node, int character_offset )
{
    const QWebElement element = DomNodeToQWebElement( node );
    QRect element_geometry    = element.geometry();

    int elem_offset  = element_geometry.top();
    int elem_height  = element_geometry.height();
    int frame_height = height();

    Q_ASSERT( frame_height != 0 );

    int current_scroll_offset = page()->mainFrame()->scrollBarValue( Qt::Vertical );
    int new_scroll_Y = 0;

    // If the element is visible, then we don't scroll
    if ( elem_offset >= current_scroll_offset && 
         elem_offset + elem_height <= current_scroll_offset + frame_height )
    {
        return;       
    }

    else if ( elem_height >= frame_height )
    {
        // The relative position of the text string start to the whole node
        float char_position = character_offset / (float) element.toPlainText().count();
        
        new_scroll_Y = elem_offset + elem_height * char_position - frame_height / 2;
    }

    // If it's "above" the currently displayed page section,
    // we scroll to the element, positioning the top of screen just above it
    else if ( elem_offset < current_scroll_offset )
    {
        new_scroll_Y = elem_offset;
    }

    // If it's "below" the currently displayed page section,
    // we scroll to the element, positioning the bottom of screen on the element
    else
    {
        new_scroll_Y = elem_offset + elem_height - frame_height;
    }   

    // If the element is very near the beginning of the document,
    // we can't position the bottom of the screen on the element
    if ( new_scroll_Y < 0 )

        new_scroll_Y = 0;

    page()->mainFrame()->setScrollBarValue( Qt::Vertical, new_scroll_Y );
}


void BookViewEditor::ScrollByLine( bool down )
{
    // This is an educated guess at best since QWebView is not
    // using the widget font but whatever font QWebView feels like using.
    int line_height = qRound( fontMetrics().height() * textSizeMultiplier() );

    ScrollByNumPixels( line_height, down );
}


void BookViewEditor::ScrollByNumPixels( int pixel_number, bool down )
{
    Q_ASSERT( pixel_number != 0 );

    int current_scroll_offset = page()->mainFrame()->scrollBarValue(   Qt::Vertical );
    int scroll_maximum        = page()->mainFrame()->scrollBarMaximum( Qt::Vertical );

    int new_scroll_Y = down ? current_scroll_offset + pixel_number : current_scroll_offset - pixel_number;    

    // qBound(min, ours, max) limits the value to the range
    new_scroll_Y     = qBound( 0, new_scroll_Y, scroll_maximum );

    page()->mainFrame()->setScrollBarValue( Qt::Vertical, new_scroll_Y );
}

QString BookViewEditor::BookViewReplaceCommand::GetRange( SelectRangeJS input )
{
    QString get_range_js = m_editor->c_GetRange;

    get_range_js.replace( "$START_NODE",   input.start_node );
    get_range_js.replace( "$END_NODE",     input.end_node      );
    get_range_js.replace( "$START_OFFSET", QString::number( input.start_node_index ) );
    get_range_js.replace( "$END_OFFSET",   QString::number( input.end_node_index )  );

    return get_range_js;
}

void BookViewEditor::BookViewReplaceCommand::undo()
{
    // Load in the identifier that indicates which replacement spans need to be undone.
    QString undoing_js = QString( m_editor->c_ReplaceUndo ).replace( "$ESCAPED_TEXT_HERE", m_elem_identifier );

    m_editor->EvaluateJavascript(  undoing_js );

    // Need to emit this even if the undo stack is cleared back to empty as the user may have switched to Code
    // View in between undos and the Text Document needs to be updated.
    emit m_editor->contentsChangedExtra();
}

void BookViewEditor::BookViewReplaceCommand::redo()
{
    QString replacing_js = GetRange( m_input ) % QString( m_editor->c_ReplaceWrapped ).replace( "$ESCAPED_TEXT_HERE", m_editor->EscapeJSString( m_replacement_text ) );

    m_elem_identifier = m_editor->EvaluateJavascript( replacing_js ).toString();

    // Tell anyone who's interested that the document has been updated.
    emit m_editor->contentsChangedExtra();
}
