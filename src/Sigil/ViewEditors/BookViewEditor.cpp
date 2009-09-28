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
#include "../Misc/Utility.h"

// Constructor;
// the parameters is the object's parent
BookViewEditor::BookViewEditor( QWidget *parent )
    : 
    QWebView( parent ),
    c_JQuery( Utility::ReadUnicodeTextFile( ":/javascript/jquery-1.3.2.min.js" ) ),
    c_JQueryScrollTo( Utility::ReadUnicodeTextFile( ":/javascript/jquery.scrollTo-1.4.2-min.js" ) ),
    c_GetCaretLocation( Utility::ReadUnicodeTextFile( ":/javascript/book_view_current_location.js" ) ),
    m_CaretLocationUpdate( QString() ),
    m_isLoadFinished( false )
{
    connect(    page(),
                SIGNAL( loadFinished( bool ) ), 
                this,
                SLOT( JavascriptOnDocumentLoad() )
           );

    connect(    page(),
                SIGNAL( loadProgress( int ) ), 
                this,
                SLOT( UpdateFinishedState( int ) )
           );

    connect(    page(),
                SIGNAL( contentsChanged() ), 
                this,
                SIGNAL( textChanged() )
           );
}


// Sets the content of the View to the specified book
void BookViewEditor::SetBook( const Book &book )
{
    setHtml( book.source, book.GetBaseUrl() );

    page()->setContentEditable( true );

    // TODO: we kill external links; a dialog should be used
    // that asks the user if he wants to open this external link in a browser
    page()->setLinkDelegationPolicy( QWebPage::DelegateAllLinks );
}


// Executes the specified command on the document with javascript
void BookViewEditor::ExecCommand( const QString &command )
{       
    QString javascript = QString( "document.execCommand( '%1', false, null)" ).arg( command );

    EvaluateJavascript( javascript );
}


// Executes the specified command with the specified parameter
// on the document with javascript
void BookViewEditor::ExecCommand( const QString &command, const QString &parameter )
{       
    QString javascript = QString( "document.execCommand( '%1', false, '%2' )" ).arg( command ).arg( parameter );

    EvaluateJavascript( javascript );
}


// Returns the state of the JavaScript command provided
bool BookViewEditor::QueryCommandState( const QString &command )
{
    QString javascript = QString( "document.queryCommandState( '%1', false, null)" ).arg( command );

    return EvaluateJavascript( javascript ).toBool();
}


// Implements the "formatBlock" execCommand because
// WebKit's default one has bugs.
// It takes an element name as an argument (e.g. "p"),
// and replaces the element the cursor is located in with it.
void BookViewEditor::FormatBlock( const QString &element_name )
{
    QString javascript =  "var node = document.getSelection().anchorNode;"
                          "var startNode = (node.nodeName == \"#text\" ? node.parentNode : node);"                          
                          "$(startNode).replaceWith( '<"+ element_name + ">' + $(startNode).html() + '</"+ element_name + ">' );";

    EvaluateJavascript( javascript );

    emit textChanged();
}


// Returns the name of the element the caret is located in;
// if text is selected, returns the name of the element
// where the selection *starts*
QString BookViewEditor::GetCaretElementName()
{
    QString javascript =  "var node = document.getSelection().anchorNode;"
                          "var startNode = (node.nodeName == \"#text\" ? node.parentNode : node);"
                          "startNode.nodeName;";

    return EvaluateJavascript( javascript ).toString();
}


// Returns a list of elements representing a "chain"
// or "walk" through the XHTML document with which one
// can identify a single element in the document.
// This list identifies the element in which the 
// keyboard caret is currently located.
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


// Accepts a list returned by a view's GetCaretLocation
// and creates and stores an update that sends the caret
// in this view to the specified element.
// The BookView implementation initiates the update in
// the JavascriptOnDocumentLoad() function.
void BookViewEditor::StoreCaretLocationUpdate( const QList< ViewEditor::ElementIndex > &hierarchy )
{
    QString caret_location = "var element = $('html')";

    for ( int i = 0; i < hierarchy.count() - 1; i++ )
    {
        caret_location.append( QString( ".children().eq(%1)" ).arg( hierarchy[ i ].index ) );
    }

    caret_location += ".get(0);";
    
    // We scroll to the element and center the screen on it
    QString scroll = "var from_top = $(window).height() / 2;"
                     "$.scrollTo( element, 0, {offset: {top:-from_top, left:0 } } );";

    m_CaretLocationUpdate = caret_location + scroll;

    // If we have focus, then we run the update right now;
    // otherwise, we defer the update until later
    if ( hasFocus() && m_isLoadFinished )
        
        ExecuteCaretUpdate();        
}

// Sets a zoom factor for the view,
// thus zooming in (factor > 1.0) or out (factor < 1.0). 
void BookViewEditor::SetZoomFactor( float factor )
{
    setZoomFactor( factor );

    emit ZoomFactorChanged( factor );
}


// Returns the View's current zoom factor
float BookViewEditor::GetZoomFactor() const
{
    return (float) zoomFactor();
}


// Executes javascript that needs to be run when
// the document has finished loading
void BookViewEditor::JavascriptOnDocumentLoad()
{
    // Javascript libraries needed
    EvaluateJavascript( c_JQuery );
    EvaluateJavascript( c_JQueryScrollTo );  

    // Run the caret update if it's pending
    ExecuteCaretUpdate();
}

// Updates the state of the m_isLoadFinished variable
// depending on the received loading progress; if the 
// progress equals 100, the state is true, otherwise false.
void BookViewEditor::UpdateFinishedState( int progress )
{ 
    if ( progress == 100 )

        m_isLoadFinished = true;

    else

        m_isLoadFinished = false;
}


// Evaluates the provided javascript source code 
// and returns the result of the last executed javascript statement
QVariant BookViewEditor::EvaluateJavascript( const QString &javascript )
{
    return page()->mainFrame()->evaluateJavaScript( javascript );
}


// Executes the caret updating code
// if an update is pending;
// returns true if update was performed
bool BookViewEditor::ExecuteCaretUpdate()
{
    // If there is no caret location update
    // pending, 
    if ( m_CaretLocationUpdate.isEmpty() )
        
        return false;

    // ... run it...
    EvaluateJavascript( m_CaretLocationUpdate );

    // ... and clear the update.
    m_CaretLocationUpdate = ""; 

    return true;
}





