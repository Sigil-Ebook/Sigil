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
#include <QtGui/QKeyEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QShortcut>
#include <QtGui/QTextDocument>
#include <QtWebKit/QWebFrame>

#include "BookManipulation/Book.h"
#include "Misc/SettingsStore.h"
#include "Misc/Utility.h"
#include "PCRE/PCRECache.h"
#include "sigil_constants.h"
#include "sigil_exception.h"
#include "ViewEditors/BookViewEditor.h"

const int PROGRESS_BAR_MINIMUM_DURATION = 1500;

const QString BREAK_TAG_INSERT    = "<hr class=\"sigilChapterBreak\" />";
const QString XML_NAMESPACE_CRUFT = "xmlns=\"http://www.w3.org/1999/xhtml\"";
const QString REPLACE_SPANS = "<span class=\"SigilReplace_\\d*\"( id=\"SigilReplace_\\d*\")*>";

const QString XML_TAG = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"no\"?>";

/**
 * The JavaScript source code for getting a string representation
 * of the "body" tag (without the children).
 */
static const QString GET_BODY_TAG_HTML = "new XMLSerializer().serializeToString( document.body.cloneNode(false) );";


BookViewEditor::BookViewEditor(QWidget *parent)
    :
    BookViewPreview(parent),
    m_WebPageModified( false ),
    m_PageUp(   *( new QShortcut( QKeySequence( QKeySequence::MoveToPreviousPage ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_PageDown( *( new QShortcut( QKeySequence( QKeySequence::MoveToNextPage     ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_ScrollOneLineUp(   *( new QShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_Up   ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    m_ScrollOneLineDown( *( new QShortcut( QKeySequence( Qt::ControlModifier + Qt::Key_Down ), this, 0, 0, Qt::WidgetShortcut ) ) ),
    c_GetSegmentHTML(   Utility::ReadUnicodeTextFile( ":/javascript/get_segment_html.js"           ) ),
    c_GetBlock(         Utility::ReadUnicodeTextFile( ":/javascript/get_block.js"                  ) ),
    c_FormatBlock(      Utility::ReadUnicodeTextFile( ":/javascript/format_block.js"               ) )
{
    setContextMenuPolicy(Qt::DefaultContextMenu);
    page()->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, false);
    page()->settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    page()->settings()->setAttribute( QWebSettings::LocalContentCanAccessRemoteUrls, false );
    page()->settings()->setAttribute( QWebSettings::ZoomTextOnly, true );

    ConnectSignalsToSlots();
}

void BookViewEditor::CustomSetDocument(const QString &path, const QString &html)
{
    m_isLoadFinished = false;
    m_path = path;

    CustomUpdateDocument(html, false);
}

void BookViewEditor::CustomUpdateDocument(const QString &html, bool saveSelection)
{
    m_isLoadFinished = false;

    BookViewPreview::CustomSetDocument(m_path, html);
    page()->setContentEditable(true);
    SetWebPageModified( false );
}

void BookViewEditor::ScrollToFragment(const QString &fragment)
{
    if (fragment.isEmpty()) {
        ScrollToTop();
        return;
    }

    QString caret_location = "var element = document.getElementById(\"" % fragment % "\");";

    QString scroll = "var from_top = window.innerHeight / 2;"
        "$.scrollTo( element, 0, {offset: {top:-from_top, left:0 } } );";

    EvaluateJavascript(caret_location % scroll % SET_CURSOR_JS);
}

void BookViewEditor::ScrollToFragmentAfterLoad(const QString &fragment)
{
    if (fragment.isEmpty()) {
        return;
    }

    QString caret_location = "var element = document.getElementById(\"" % fragment % "\");";

    QString scroll = "var from_top = window.innerHeight / 2;"
        "$.scrollTo( element, 0, {offset: {top:-from_top, left:0 } } );";

    QString javascript = "window.addEventListener('load', GoToFragment, false);"
        "function GoToFragment() { " % caret_location % scroll % SET_CURSOR_JS % "}";

    EvaluateJavascript(javascript);
}

QString BookViewEditor::GetHtml()
{
    RemoveWebkitCruft();

    // Set the xml tag here rather than let Tidy do it.
    // This prevents false mismatches with the cache later on.
    QString html_from_Qt = page()->mainFrame()->toHtml();

    html_from_Qt = RemoveBookViewReplaceSpans( html_from_Qt );
    html_from_Qt = html_from_Qt.remove( XML_NAMESPACE_CRUFT );
    return XML_TAG % html_from_Qt;
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

void BookViewEditor::InsertHtml(const QString &html)
{
    ExecCommand( "insertHTML", html );
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
    page()->triggerAction( QWebPage::Undo );
}

void BookViewEditor::Redo()
{
    page()->triggerAction( QWebPage::Redo );
}

// Overridden so we can emit the FocusLost() signal.
void BookViewEditor::focusOutEvent(QFocusEvent *event)
{
    emit FocusLost(this);
    QWebView::focusOutEvent(event);
}


QString BookViewEditor::GetSelectedText()
{
    QString javascript = "window.getSelection().toString();";

    return EvaluateJavascript( javascript ).toString();
}

void BookViewEditor::TextChangedFilter()
{
    emit textChanged();
}

void BookViewEditor::ExecCommand( const QString &command )
{
    if( m_isLoadFinished ) {
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

QString BookViewEditor::EscapeJSString( const QString &string )
{
    QString new_string( string );

    /* \ -> \\ */ 
    // " -> \"
    // ' -> \'
    return new_string.replace( "\\", "\\\\" ).replace( "\"", "\\\"" ).replace( "'", "\\'" );
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

void BookViewEditor::RemoveWebkitCruft()
{
    QWebElementCollection collection = page()->mainFrame()->findAllElements( ".Apple-style-span" );

    foreach( QWebElement element, collection )
    {
        element.toggleClass( "Apple-style-span" );
    }

    collection = page()->mainFrame()->findAllElements( ".webkit-indent-blockquote" );

    foreach( QWebElement element, collection )
    {
        element.toggleClass( "webkit-indent-blockquote" );
    }

    QWebElement body_tag =  page()->mainFrame()->findFirstElement( "body" );

    // Removing junk webkit styles
    body_tag.setStyleProperty( "word-wrap", "" );
    body_tag.setStyleProperty( "-webkit-nbsp-mode", "" );
    body_tag.setStyleProperty( "-webkit-line-break", "" );

    // Banish the irritating <body style=""> tags
    if( body_tag.attribute( "style", "none" ) == "" )
    {
        body_tag.removeAttribute( "style" );
    }
}

QString BookViewEditor::RemoveBookViewReplaceSpans( const QString &source )
{
    QRegExp replace_spans( REPLACE_SPANS );
    replace_spans.setMinimal( true );
    QRegExp span_open_or_close( "<\\s*(/)*\\s*span\\s*>");
    span_open_or_close.setMinimal( true );

    QString newsource = "";
    int left_pos = 0;
    int index = source.indexOf( replace_spans );
    while( index != -1 )
    {
        // Append the text between the last capture and this one.
        newsource.append( source.mid( left_pos, index - left_pos ) );

        // Advance past the captured opening tag.
        index += replace_spans.cap(0).length();
        left_pos = index;

        // Check for nested spans.
        int nest_count = 1; // set to 1 as we already have an open span
        int next_span_tag = index;
        do 
        {
            next_span_tag = source.indexOf( span_open_or_close, index );
            if( next_span_tag == -1 )
            {
                // Content is not well-formed, which should never happen here.
                boost_throw( ErrorParsingXml()
                             << errinfo_XML_parsing_error_string( "GetWebPageHTML() has returned invalid xhtml" ) );
            }

            if( !span_open_or_close.cap(0).contains( "/" ) )
            {
                // Opening tag, so increment the counter.
                nest_count++;
            }
            else
            {
                // Closing tag, so decrement the counter
                nest_count--;
            }
        } while( nest_count > 0 );

        // next_span_tag now points to the start of the closing tag of the span we're removing.
        // Append the source from the end of the span tag to the start of the closing tag
        newsource.append( source.mid( index, next_span_tag - index ) );

        // Move left_pos past the closing tag and search for another span to remove.
        left_pos = next_span_tag + span_open_or_close.cap(0).length(); 
        index = source.indexOf( replace_spans, left_pos );
    }

    // Append the rest of the source after all the spans have been removed.
    newsource.append( source.mid( left_pos ) );

    // It's possible that we might have replace spans nested within each other,
    // so go back to the start and check again, and recurse if found.
    if( newsource.indexOf( replace_spans ) != -1 )
    {
        newsource = RemoveBookViewReplaceSpans( newsource );
    }

    return newsource;
}

void BookViewEditor::SetWebPageModified( bool modified )
{
    m_WebPageModified = modified;
}

void BookViewEditor::FormatBlock( const QString &element_name, bool preserve_attributes )
{
    if ( element_name.isEmpty() ) {
        return;
    }

    QString preserve = preserve_attributes ? "true" : "false";
    QString javascript =  c_GetBlock % c_FormatBlock %
            "var node = document.getSelection().anchorNode;"
            "var startNode = get_block( node );"
            "var element = format_block( startNode, \""+element_name+"\", "+preserve+" );"
            "startNode.parentNode.replaceChild( element, startNode );"
            % SET_CURSOR_JS;

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

void BookViewEditor::ConnectSignalsToSlots()
{
    connect( &m_PageUp,            SIGNAL( activated() ), this, SLOT( PageUp()            ) );
    connect( &m_PageDown,          SIGNAL( activated() ), this, SLOT( PageDown()          ) );
    connect( &m_ScrollOneLineUp,   SIGNAL( activated() ), this, SLOT( ScrollOneLineUp()   ) );
    connect( &m_ScrollOneLineDown, SIGNAL( activated() ), this, SLOT( ScrollOneLineDown() ) );

    connect( this,   SIGNAL( contentsChangedExtra() ),  page(), SIGNAL( contentsChanged()        ) );
    connect( page(), SIGNAL( contentsChanged()  ),      this,   SIGNAL( textChanged()            ) );
    connect( page(), SIGNAL( selectionChanged() ),      this,   SIGNAL( selectionChanged()       ) );

    connect( page(), SIGNAL( contentsChanged()  ),      this,   SLOT( SetWebPageModified()       ) );
}