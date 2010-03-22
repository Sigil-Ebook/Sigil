/************************************************************************
**
**  Copyright (C) 2009  Strahinja Markovic
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
#include "BookViewEditor.h"
#include "../BookManipulation/Book.h"
#include "../BookManipulation/XHTMLDoc.h"
#include "../BookManipulation/CleanSource.h"
#include "../Misc/Utility.h"
#include <QDomDocument>

const int PROGRESS_BAR_MINIMUM_DURATION = 1500;

const QString BREAK_TAG_ID        = "sigilChBrMN364299QX";
const QString BREAK_TAG_INSERT    = "<hr id=\"" + BREAK_TAG_ID + "\" />";
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
    m_isLoadFinished( true ),
    m_PageUp(   *( new QShortcut( QKeySequence( QKeySequence::MoveToPreviousPage ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_PageDown( *( new QShortcut( QKeySequence( QKeySequence::MoveToNextPage     ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_ScrollOneLineUp(   *( new QShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_Up   ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_ScrollOneLineDown( *( new QShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_Down ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    c_GetCaretLocation( Utility::ReadUnicodeTextFile( ":/javascript/book_view_current_location.js" ) ),
    c_NewSelection(     Utility::ReadUnicodeTextFile( ":/javascript/new_selection.js"              ) ),
    c_GetRange(         Utility::ReadUnicodeTextFile( ":/javascript/get_range.js"                  ) ),
    c_ReplaceText(      Utility::ReadUnicodeTextFile( ":/javascript/replace_text.js"               ) ),
    c_GetSegmentHTML(   Utility::ReadUnicodeTextFile( ":/javascript/get_segment_html.js"           ) )

{
    connect( &m_PageUp,            SIGNAL( activated() ), this, SLOT( PageUp()            ) );
    connect( &m_PageDown,          SIGNAL( activated() ), this, SLOT( PageDown()          ) );
    connect( &m_ScrollOneLineUp,   SIGNAL( activated() ), this, SLOT( ScrollOneLineUp()   ) );
    connect( &m_ScrollOneLineDown, SIGNAL( activated() ), this, SLOT( ScrollOneLineDown() ) );
}


void BookViewEditor::CustomSetWebPage( QWebPage &webpage )
{
    connect( &webpage, SIGNAL( contentsChanged()    ), this, SIGNAL( textChanged()            ) );
    connect( &webpage, SIGNAL( selectionChanged()   ), this, SIGNAL( selectionChanged()       ) );
    connect( &webpage, SIGNAL( loadFinished( bool ) ), this, SLOT( JavascriptOnDocumentLoad() ) );
    connect( &webpage, SIGNAL( loadProgress( int )  ), this, SLOT( UpdateFinishedState( int ) ) );

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
    QString segment  = EvaluateJavascript( c_GetSegmentHTML  ).toString();

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
    QString javascript = QString( "document.execCommand( '%1', false, null)" ).arg( command );

    EvaluateJavascript( javascript );
}


void BookViewEditor::ExecCommand( const QString &command, const QString &parameter )
{       
    QString javascript = QString( "document.execCommand( '%1', false, '%2' )" ).arg( command ).arg( parameter );

    EvaluateJavascript( javascript );
}


bool BookViewEditor::QueryCommandState( const QString &command )
{
    QString javascript = QString( "document.queryCommandState( '%1', false, null)" ).arg( command );

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
    
    setFocus();
}


void BookViewEditor::ScrollToTop()
{
    page()->currentFrame()->setScrollBarValue( Qt::Vertical, 0 );
}


void BookViewEditor::ScrollToFragment( const QString &fragment )
{
    if ( fragment.isEmpty() )

        return;

    QString javascript = "window.location.hash = \""  + fragment + "\";";

    EvaluateJavascript( javascript );
}


void BookViewEditor::ScrollToFragmentAfterLoad( const QString &fragment )
{
    if ( fragment.isEmpty() )

        return;

    QString javascript = "window.addEventListener('load', GoToFragment, false);"
                         "function GoToFragment() { window.location.hash = \""  + fragment + "\"; }";

    EvaluateJavascript( javascript );
}


void BookViewEditor::FormatBlock( const QString &element_name )
{

    if ( element_name.isEmpty() )

        return;

    QString javascript =  "var node = document.getSelection().anchorNode;"
                          "var startNode = (node.nodeName == \"#text\" ? node.parentNode : node);"                          
                          "$(startNode).replaceWith( '<"+ element_name + ">' + $(startNode).html() + '</"+ element_name + ">' );";

    EvaluateJavascript( javascript );

    emit textChanged();
}


QString BookViewEditor::GetCaretElementName()
{
    QString javascript =  "var node = document.getSelection().anchorNode;"
                          "var startNode = (node.nodeName == \"#text\" ? node.parentNode : node);"
                          "startNode.nodeName;";

    return EvaluateJavascript( javascript ).toString();
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

    m_CaretLocationUpdate = caret_location + scroll;     
}


void BookViewEditor::SetZoomFactor( float factor )
{
    setZoomFactor( factor );

    emit ZoomFactorChanged( factor );
}



float BookViewEditor::GetZoomFactor() const
{
    return (float) zoomFactor();
}


bool BookViewEditor::FindNext( const QRegExp &search_regex, Searchable::Direction search_direction )
{
    SearchTools search_tools = GetSearchTools();
    int selection_offset     = GetSelectionOffset( search_tools.document, search_tools.node_offsets, search_direction ); 

    QRegExp result_regex = search_regex;
    RunSearchRegex( result_regex, search_tools.fulltext, selection_offset, search_direction ); 

    if ( result_regex.pos() != -1 )
    {
        SelectRangeInputs input = GetRangeInputs( search_tools.node_offsets, result_regex.pos(), result_regex.matchedLength() );
        SelectTextRange( input );
        ScrollToNode( input.start_node );  

        return true;
    } 

    return false;
}


int BookViewEditor::Count( const QRegExp &search_regex )
{
    SearchTools search_tools = GetSearchTools();

    return search_tools.fulltext.count( search_regex );
}


bool BookViewEditor::ReplaceSelected( const QRegExp &search_regex, const QString &replacement )
{
    SearchTools search_tools = GetSearchTools();

    // We ALWAYS say Direction_Up because we want
    // the "back" index of the selection range
    int selection_offset = GetSelectionOffset( search_tools.document, search_tools.node_offsets, Searchable::Direction_Up ); 

    QRegExp result_regex  = search_regex;
    QString selected_text = GetSelectedText();

    // If we are currently sitting at the start 
    // of a matching substring, we replace it.
    if ( result_regex.exactMatch( selected_text ) )
    {
        QString final_replacement = FillWithCapturedTexts( result_regex.capturedTexts(), replacement );
        QString replacing_js      = QString( c_ReplaceText ).replace( "$ESCAPED_TEXT_HERE", EscapeJSString( final_replacement ) );

        SelectRangeInputs input   = GetRangeInputs( search_tools.node_offsets, selection_offset, selected_text.length() );
        EvaluateJavascript( GetRangeJS( input ) + replacing_js + c_NewSelection ); 

        // Tell anyone who's interested that the document has been updated.
        emit textChanged();

        return true;
    }

    return false;
}


int BookViewEditor::ReplaceAll( const QRegExp &search_regex, const QString &replacement )
{    
    QRegExp result_regex = search_regex;
    int count = 0;
    
    QProgressDialog progress( tr( "Replacing search term..." ), QString(), 0, Count( search_regex ) );
    progress.setMinimumDuration( PROGRESS_BAR_MINIMUM_DURATION );
    
    // Slow as hell. Find a way to speed this up.
    // Something that does NOT require parsing the whole
    // document every single time we change something...
    while ( true )
    {
        // Update the progress bar
        progress.setValue( count );

        SearchTools search_tools = GetSearchTools();

        if ( search_tools.fulltext.indexOf( result_regex ) != -1 )
        {
            QString final_replacement = FillWithCapturedTexts( result_regex.capturedTexts(), replacement );
            QString replacing_js      = QString( c_ReplaceText ).replace( "$ESCAPED_TEXT_HERE", EscapeJSString( final_replacement ) );

            SelectRangeInputs input   = GetRangeInputs( search_tools.node_offsets, result_regex.pos(), result_regex.matchedLength() );
            EvaluateJavascript( GetRangeJS( input ) + replacing_js ); 

            ++count;
        }

        else
        {
            break;
        }
    }

    // Tell anyone who's interested that the document has been updated.
    emit textChanged();

    return count;
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


void BookViewEditor::JavascriptOnDocumentLoad()
{
    // The jQuery libs are loaded in 
    // HTMLResouce::WebPageJavascriptOnLoad

    // Run the caret update if it's pending
    ExecuteCaretUpdate();
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


int BookViewEditor::GetSelectionOffset( const QDomDocument &document,
                                        const QMap< int, QDomNode > &node_offsets, 
                                        Searchable::Direction search_direction )
{
    QDomNode caret_node = XHTMLDoc::GetNodeFromHierarchy( document, GetCaretLocation() );

    bool searching_down =   search_direction == Searchable::Direction_Down || 
                            search_direction == Searchable::Direction_All ? true : false;

    int local_offset    = GetLocalSelectionOffset( !searching_down );
    int search_start    = node_offsets.key( caret_node ) + local_offset;

    return search_start;
}


QString BookViewEditor::GetSelectedText()
{
    QString javascript = "window.getSelection().toString();";

    return EvaluateJavascript( javascript ).toString();
}


BookViewEditor::SearchTools BookViewEditor::GetSearchTools() const
{
    SearchTools search_tools;
    search_tools.fulltext = "";
    search_tools.document.setContent( page()->mainFrame()->toHtml() );

    QList< QDomNode > text_nodes = XHTMLDoc::GetVisibleTextNodes( search_tools.document.elementsByTagName( "body" ).at( 0 ) );

    QDomNode current_block_ancestor;    

    // We concatenate all text nodes that have the same 
    // block level ancestor element. A newline is added
    // when a new block element starts.
    // We also record the starting offset of every text node.
    for ( int i = 0; i < text_nodes.count(); ++i )
    {
        QDomNode new_block_ancestor = XHTMLDoc::GetAncestorBlockElement( text_nodes[ i ] );

        if ( new_block_ancestor == current_block_ancestor )
        {
            search_tools.node_offsets[ search_tools.fulltext.length() ] = text_nodes[ i ];
            search_tools.fulltext.append( text_nodes[ i ].nodeValue() );
        }

        else
        {
            current_block_ancestor = new_block_ancestor;
            search_tools.fulltext.append( "\n" );

            search_tools.node_offsets[ search_tools.fulltext.length() ] = text_nodes[ i ];
            search_tools.fulltext.append( text_nodes[ i ].nodeValue() );
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


QString BookViewEditor::EscapeJSString( const QString &string )
{
    QString new_string( string );

    // \ -> \\ and " -> \"
    return new_string.replace( "\\", "\\\\" ).replace( "\"", "\\\"" );
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



BookViewEditor::SelectRangeInputs BookViewEditor::GetRangeInputs( const QMap< int, QDomNode > &node_offsets,
                                                                  int string_start, 
                                                                  int string_length ) const
{
    SelectRangeInputs input;

    QList< int > offsets = node_offsets.keys();
    int last_offset = offsets.first();

    for ( int i = 0; i < offsets.length(); ++i )
    {
        int next_offset = 0;

        // If there is such a thing as the next offset, use it
        if ( i + 1 < offsets.length()  )

            next_offset = offsets[ i + 1 ];

        // Otherwise compute it
        else

            // + 1 because we are pretending there is another text node after this one
            next_offset = offsets[ i ] + node_offsets[ offsets[ i ] ].nodeValue().length() + 1;

        if ( next_offset > string_start && 
             input.start_node.isNull() 
           )
        {
            input.start_node_index = string_start - last_offset;
            input.start_node       = node_offsets.value( last_offset );
        }

        if ( next_offset > string_start + string_length &&
             input.end_node.isNull() 
           )
        {
            input.end_node_index = string_start + string_length - last_offset;
            input.end_node       = node_offsets.value( last_offset );
        }

        if ( !input.start_node.isNull() && !input.end_node.isNull() )

            break;

        last_offset = next_offset;
    }

    // TODO: throw an exception
    Q_ASSERT( !input.start_node.isNull() && !input.end_node.isNull() );

    return input;
}


QString BookViewEditor::GetRangeJS( const SelectRangeInputs &input ) const
{
    QString start_node_js = GetElementSelectingJS_WithTextNode( XHTMLDoc::GetHierarchyFromNode( input.start_node ) );
    QString end_node_js   = GetElementSelectingJS_WithTextNode( XHTMLDoc::GetHierarchyFromNode( input.end_node   ) );

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


void BookViewEditor::ScrollToNode( const QDomNode &node )
{
    QString element_selector = GetElementSelectingJS_NoTextNodes( XHTMLDoc::GetHierarchyFromNode( node ) );

    QString offset_js = "$(" + element_selector + ").offset().top;";
    QString height_js = "$(" + element_selector + ").height();";

    int elem_offset  = EvaluateJavascript( offset_js ).toInt();
    int elem_height  = EvaluateJavascript( height_js ).toInt();
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
        new_scroll_Y = elem_offset - frame_height + elem_height;
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










